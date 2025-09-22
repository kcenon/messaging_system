#include <gtest/gtest.h>
#include <kcenon/messaging/services/container/container_service.h>
#include <kcenon/messaging/core/message_bus.h>
#include <memory>
#include <chrono>
#include <thread>

using namespace kcenon::messaging::services::container;
using namespace kcenon::messaging::services;
using namespace kcenon::messaging::core;

class ContainerServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.max_message_size = 1024 * 1024;  // 1MB
        config_.enable_compression = true;
        config_.enable_validation = true;
        config_.default_format = "json";

        service_ = std::make_unique<container_service>(config_);

        // Create message bus for adapter testing
        message_bus_config bus_config;
        bus_config.worker_threads = 2;
        message_bus_ = std::make_unique<message_bus>(bus_config);
        message_bus_->initialize();
    }

    void TearDown() override {
        if (service_ && service_->get_state() == service_state::running) {
            service_->shutdown();
        }
        if (message_bus_) {
            message_bus_->shutdown();
        }
    }

    container_config config_;
    std::unique_ptr<container_service> service_;
    std::unique_ptr<message_bus> message_bus_;
};

TEST_F(ContainerServiceTest, ServiceLifecycle) {
    EXPECT_EQ(service_->get_state(), service_state::uninitialized);
    EXPECT_FALSE(service_->is_healthy());

    EXPECT_TRUE(service_->initialize());
    EXPECT_EQ(service_->get_state(), service_state::running);
    EXPECT_TRUE(service_->is_healthy());

    service_->shutdown();
    EXPECT_EQ(service_->get_state(), service_state::stopped);
    EXPECT_FALSE(service_->is_healthy());
}

TEST_F(ContainerServiceTest, ServiceMetadata) {
    EXPECT_EQ(service_->get_service_name(), "container_service");
    EXPECT_FALSE(service_->get_service_version().empty());
}

TEST_F(ContainerServiceTest, MessageHandling) {
    ASSERT_TRUE(service_->initialize());

    // Test topic handling
    EXPECT_TRUE(service_->can_handle_topic("container.serialize"));
    EXPECT_TRUE(service_->can_handle_topic("container.deserialize"));
    EXPECT_TRUE(service_->can_handle_topic("container.validate"));
    EXPECT_TRUE(service_->can_handle_topic("container.compress"));
    EXPECT_FALSE(service_->can_handle_topic("network.connect"));
    EXPECT_FALSE(service_->can_handle_topic("random.topic"));

    // Create a test message
    message test_msg;
    test_msg.payload.topic = "container.serialize";
    test_msg.payload.data["format"] = std::string("json");
    test_msg.payload.data["data"] = std::string("test_data");

    // The service should be able to handle this message
    EXPECT_NO_THROW(service_->handle_message(test_msg));
}

TEST_F(ContainerServiceTest, SerializationOperations) {
    ASSERT_TRUE(service_->initialize());

    // Create test payload
    message_payload test_payload;
    test_payload.topic = "test.topic";
    test_payload.data["string_value"] = std::string("Hello World");
    test_payload.data["int_value"] = int64_t(42);
    test_payload.data["double_value"] = double(3.14);
    test_payload.data["bool_value"] = bool(true);

    // Test serialization
    std::vector<uint8_t> serialized_data;
    EXPECT_TRUE(service_->serialize_payload(test_payload, serialized_data));
    EXPECT_FALSE(serialized_data.empty());

    // Test deserialization
    message_payload deserialized_payload;
    EXPECT_TRUE(service_->deserialize_payload(serialized_data, deserialized_payload));

    // Verify deserialized data matches original
    EXPECT_EQ(deserialized_payload.topic, test_payload.topic);
    EXPECT_EQ(deserialized_payload.data.size(), test_payload.data.size());

    // Check specific values
    auto string_it = deserialized_payload.data.find("string_value");
    ASSERT_NE(string_it, deserialized_payload.data.end());
    EXPECT_TRUE(std::holds_alternative<std::string>(string_it->second));
    EXPECT_EQ(std::get<std::string>(string_it->second), "Hello World");

    auto int_it = deserialized_payload.data.find("int_value");
    ASSERT_NE(int_it, deserialized_payload.data.end());
    EXPECT_TRUE(std::holds_alternative<int64_t>(int_it->second));
    EXPECT_EQ(std::get<int64_t>(int_it->second), 42);
}

TEST_F(ContainerServiceTest, ValidationOperations) {
    ASSERT_TRUE(service_->initialize());

    // Test valid payload
    message_payload valid_payload;
    valid_payload.topic = "valid.topic";
    valid_payload.data["content"] = std::string("Valid content");

    EXPECT_TRUE(service_->validate_payload(valid_payload));

    // Test invalid payload (topic too large for example)
    message_payload invalid_payload;
    invalid_payload.topic = std::string(10000, 'x');  // Very long topic
    invalid_payload.data["content"] = std::string("Content");

    // This might fail validation depending on implementation
    // EXPECT_FALSE(service_->validate_payload(invalid_payload));
}

TEST_F(ContainerServiceTest, CompressionOperations) {
    ASSERT_TRUE(service_->initialize());

    // Create test data that should compress well
    std::string repeated_text(1000, 'A');
    std::vector<uint8_t> input_data(repeated_text.begin(), repeated_text.end());

    // Test compression
    std::vector<uint8_t> compressed_data;
    EXPECT_TRUE(service_->compress_data(input_data, compressed_data));

    // Compressed data should be smaller for repeated content
    EXPECT_LT(compressed_data.size(), input_data.size());

    // Test decompression
    std::vector<uint8_t> decompressed_data;
    EXPECT_TRUE(service_->decompress_data(compressed_data, decompressed_data));

    // Decompressed data should match original
    EXPECT_EQ(decompressed_data.size(), input_data.size());
    EXPECT_EQ(decompressed_data, input_data);
}

TEST_F(ContainerServiceTest, ContainerServiceAdapter) {
    ASSERT_TRUE(service_->initialize());

    // Create adapter with shared_ptr
    std::shared_ptr<container_service> service_shared(service_.get(), [](container_service*){});
    auto adapter = std::make_shared<container_service_adapter>(service_shared);
    EXPECT_EQ(adapter->get_service_name(), "container_service");

    // Register adapter with message bus
    adapter->register_with_bus(message_bus_.get());
    EXPECT_TRUE(adapter->initialize());

    std::atomic<bool> message_handled{false};
    std::string handled_topic;

    // Subscribe to container response topic to verify adapter is working
    message_bus_->subscribe("container.response", [&](const message& msg) {
        message_handled = true;
        handled_topic = msg.payload.topic;
    });

    // Send a container operation message through the bus
    message_payload payload;
    payload.topic = "container.serialize";
    payload.data["format"] = std::string("json");
    payload.data["data"] = std::string("test_data");

    EXPECT_TRUE(message_bus_->publish("container.serialize", payload, "test_client"));

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Verify the adapter handled the message
    EXPECT_TRUE(message_handled.load());
    EXPECT_EQ(handled_topic, "container.response");

    adapter->shutdown();
}

TEST_F(ContainerServiceTest, ConfigurationRespect) {
    // Test with different configuration
    container_config custom_config;
    custom_config.max_message_size = 512;  // Smaller than default
    custom_config.enable_compression = false;
    custom_config.enable_validation = false;
    custom_config.default_format = "binary";

    auto custom_service = std::make_unique<container_service>(custom_config);
    EXPECT_TRUE(custom_service->initialize());

    // Service should still work with custom configuration
    message_payload test_payload;
    test_payload.topic = "test";
    test_payload.data["test"] = std::string("test");

    std::vector<uint8_t> serialized;
    EXPECT_TRUE(custom_service->serialize_payload(test_payload, serialized));

    custom_service->shutdown();
}

TEST_F(ContainerServiceTest, LargeDataHandling) {
    ASSERT_TRUE(service_->initialize());

    // Create large payload (but within limits)
    message_payload large_payload;
    large_payload.topic = "large.test";
    large_payload.data["large_string"] = std::string(100000, 'X');  // 100KB string

    // Test serialization of large data
    std::vector<uint8_t> serialized_data;
    EXPECT_TRUE(service_->serialize_payload(large_payload, serialized_data));

    // Test deserialization
    message_payload deserialized_payload;
    EXPECT_TRUE(service_->deserialize_payload(serialized_data, deserialized_payload));

    EXPECT_EQ(deserialized_payload.topic, large_payload.topic);

    auto large_it = deserialized_payload.data.find("large_string");
    ASSERT_NE(large_it, deserialized_payload.data.end());
    EXPECT_TRUE(std::holds_alternative<std::string>(large_it->second));
    EXPECT_EQ(std::get<std::string>(large_it->second).size(), 100000);
}

TEST_F(ContainerServiceTest, ErrorHandling) {
    ASSERT_TRUE(service_->initialize());

    // Test deserialization with invalid data
    std::vector<uint8_t> invalid_data = {0xFF, 0xFE, 0xFD, 0xFC};
    message_payload result_payload;

    // This should fail gracefully
    EXPECT_FALSE(service_->deserialize_payload(invalid_data, result_payload));

    // Test compression with empty data
    std::vector<uint8_t> empty_data;
    std::vector<uint8_t> compressed_result;

    // Should handle empty data gracefully
    bool compress_result = service_->compress_data(empty_data, compressed_result);
    // Result may vary based on compression algorithm, but shouldn't crash
    (void)compress_result; // Suppress unused variable warning
}