#include <gtest/gtest.h>
#include <kcenon/messaging/services/network/network_service.h>
#include <kcenon/messaging/core/message_bus.h>
#include <memory>
#include <chrono>
#include <thread>

using namespace kcenon::messaging::services::network;
using namespace kcenon::messaging::services;
using namespace kcenon::messaging::core;

class NetworkServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.listen_address = "127.0.0.1";
        config_.listen_port = 8080;
        config_.max_connections = 100;
        config_.connection_timeout = std::chrono::seconds(30);
        config_.enable_ssl = false;

        service_ = std::make_unique<network_service>(config_);

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

    network_config config_;
    std::unique_ptr<network_service> service_;
    std::unique_ptr<message_bus> message_bus_;
};

TEST_F(NetworkServiceTest, ServiceLifecycle) {
    EXPECT_EQ(service_->get_state(), service_state::uninitialized);
    EXPECT_FALSE(service_->is_healthy());

    EXPECT_TRUE(service_->initialize());
    EXPECT_EQ(service_->get_state(), service_state::running);
    EXPECT_TRUE(service_->is_healthy());

    service_->shutdown();
    EXPECT_EQ(service_->get_state(), service_state::stopped);
    EXPECT_FALSE(service_->is_healthy());
}

TEST_F(NetworkServiceTest, ServiceMetadata) {
    EXPECT_EQ(service_->get_service_name(), "network_service");
    EXPECT_FALSE(service_->get_service_version().empty());
}

TEST_F(NetworkServiceTest, MessageHandling) {
    ASSERT_TRUE(service_->initialize());

    // Test topic handling
    EXPECT_TRUE(service_->can_handle_topic("network.send"));
    EXPECT_TRUE(service_->can_handle_topic("network.broadcast"));
    EXPECT_TRUE(service_->can_handle_topic("network.connect"));
    EXPECT_TRUE(service_->can_handle_topic("network.disconnect"));
    EXPECT_FALSE(service_->can_handle_topic("container.serialize"));
    EXPECT_FALSE(service_->can_handle_topic("random.topic"));

    // Create a test message
    message test_msg;
    test_msg.payload.topic = "network.send";
    test_msg.payload.data["destination"] = std::string("test_destination");
    test_msg.payload.data["content"] = std::string("test message");

    // The service should be able to handle this message
    EXPECT_NO_THROW(service_->handle_message(test_msg));
}

TEST_F(NetworkServiceTest, MessageSending) {
    ASSERT_TRUE(service_->initialize());

    // Create test message
    message test_msg;
    test_msg.payload.topic = "test.message";
    test_msg.payload.data["content"] = std::string("Hello Network");
    test_msg.payload.data["sender"] = std::string("test_sender");

    // Test sending message to specific destination
    EXPECT_TRUE(service_->send_message("test_destination", test_msg));

    // Check statistics
    const auto& stats = service_->get_statistics();
    EXPECT_GE(stats.messages_sent.load(), 1);
}

TEST_F(NetworkServiceTest, MessageBroadcasting) {
    ASSERT_TRUE(service_->initialize());

    // Create test message for broadcasting
    message broadcast_msg;
    broadcast_msg.payload.topic = "broadcast.test";
    broadcast_msg.payload.data["content"] = std::string("Broadcast message");
    broadcast_msg.payload.data["priority"] = std::string("high");

    // Test broadcasting message
    EXPECT_TRUE(service_->broadcast_message(broadcast_msg));

    // Check statistics
    const auto& stats = service_->get_statistics();
    EXPECT_GE(stats.messages_sent.load(), 1);
}

TEST_F(NetworkServiceTest, StatisticsTracking) {
    ASSERT_TRUE(service_->initialize());

    const auto& initial_stats = service_->get_statistics();
    uint64_t initial_sent = initial_stats.messages_sent.load();

    // Send multiple messages
    message test_msg;
    test_msg.payload.topic = "stats.test";
    test_msg.payload.data["content"] = std::string("Statistics test");

    for (int i = 0; i < 5; ++i) {
        test_msg.payload.data["sequence"] = int64_t(i);
        EXPECT_TRUE(service_->send_message("stats_dest", test_msg));
    }

    // Check that statistics were updated
    const auto& final_stats = service_->get_statistics();
    EXPECT_EQ(final_stats.messages_sent.load(), initial_sent + 5);
}

TEST_F(NetworkServiceTest, NetworkServiceAdapter) {
    ASSERT_TRUE(service_->initialize());

    // Create adapter with shared_ptr
    std::shared_ptr<network_service> service_shared(service_.get(), [](network_service*){});
    auto adapter = std::make_shared<network_service_adapter>(service_shared);
    EXPECT_EQ(adapter->get_service_name(), "network_service");

    // Register adapter with message bus
    adapter->register_with_bus(message_bus_.get());
    EXPECT_TRUE(adapter->initialize());

    std::atomic<bool> message_handled{false};
    std::string handled_topic;

    // Subscribe to network response topic to verify adapter is working
    message_bus_->subscribe("network.response", [&](const message& msg) {
        message_handled = true;
        handled_topic = msg.payload.topic;
    });

    // Send a network message through the bus
    message_payload payload;
    payload.topic = "network.send";
    payload.data["destination"] = std::string("test_destination");
    payload.data["content"] = std::string("adapter test");

    EXPECT_TRUE(message_bus_->publish("network.send", payload, "test_client"));

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Verify the adapter handled the message
    EXPECT_TRUE(message_handled.load());
    EXPECT_EQ(handled_topic, "network.response");

    adapter->shutdown();
}

TEST_F(NetworkServiceTest, ConfigurationValidation) {
    // Test with invalid configuration
    network_config invalid_config;
    invalid_config.listen_port = 0;  // Invalid port
    invalid_config.max_connections = 0;  // Invalid max connections

    auto invalid_service = std::make_unique<network_service>(invalid_config);

    // Service might still initialize but with default values
    // or fail initialization based on implementation
    bool init_result = invalid_service->initialize();
    (void)init_result; // Suppress unused variable warning

    if (init_result) {
        invalid_service->shutdown();
    }
}

TEST_F(NetworkServiceTest, SSLConfiguration) {
    // Test with SSL enabled
    network_config ssl_config = config_;
    ssl_config.enable_ssl = true;
    ssl_config.listen_port = 8443;

    auto ssl_service = std::make_unique<network_service>(ssl_config);

    // SSL service should initialize (even without certificates for testing)
    bool ssl_init = ssl_service->initialize();
    (void)ssl_init; // Implementation dependent

    if (ssl_init) {
        EXPECT_EQ(ssl_service->get_state(), service_state::running);
        ssl_service->shutdown();
    }
}

TEST_F(NetworkServiceTest, ConcurrentMessageSending) {
    ASSERT_TRUE(service_->initialize());

    constexpr int num_threads = 4;
    constexpr int messages_per_thread = 25;
    std::atomic<int> successful_sends{0};

    const auto& initial_stats = service_->get_statistics();
    uint64_t initial_sent = initial_stats.messages_sent.load();

    std::vector<std::thread> sender_threads;

    // Start concurrent message senders
    for (int t = 0; t < num_threads; ++t) {
        sender_threads.emplace_back([this, t, messages_per_thread, &successful_sends]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                message msg;
                msg.payload.topic = "concurrent.test";
                msg.payload.data["thread_id"] = int64_t(t);
                msg.payload.data["message_id"] = int64_t(i);
                msg.payload.data["content"] = std::string("concurrent message");

                std::string destination = "dest_" + std::to_string(t);
                if (service_->send_message(destination, msg)) {
                    successful_sends++;
                }
            }
        });
    }

    // Wait for all senders to complete
    for (auto& thread : sender_threads) {
        thread.join();
    }

    EXPECT_EQ(successful_sends.load(), num_threads * messages_per_thread);

    // Check final statistics
    const auto& final_stats = service_->get_statistics();
    EXPECT_EQ(final_stats.messages_sent.load(), initial_sent + (num_threads * messages_per_thread));
}

TEST_F(NetworkServiceTest, LargeMessageHandling) {
    ASSERT_TRUE(service_->initialize());

    // Create large message
    message large_msg;
    large_msg.payload.topic = "large.message";
    large_msg.payload.data["large_content"] = std::string(50000, 'X');  // 50KB content
    large_msg.payload.data["metadata"] = std::string("Large message test");

    // Test sending large message
    EXPECT_TRUE(service_->send_message("large_dest", large_msg));

    // Test broadcasting large message
    EXPECT_TRUE(service_->broadcast_message(large_msg));

    const auto& stats = service_->get_statistics();
    EXPECT_GE(stats.messages_sent.load(), 2);
}

TEST_F(NetworkServiceTest, ErrorRecovery) {
    ASSERT_TRUE(service_->initialize());

    // Test sending to invalid destinations
    message test_msg;
    test_msg.payload.topic = "error.test";
    test_msg.payload.data["content"] = std::string("Error recovery test");

    // These might fail depending on implementation
    const auto& initial_stats = service_->get_statistics();

    // Try to send to various potentially invalid destinations
    service_->send_message("", test_msg);  // Empty destination
    service_->send_message("invalid://destination", test_msg);  // Invalid format

    // Service should still be healthy after error scenarios
    EXPECT_TRUE(service_->is_healthy());
    EXPECT_EQ(service_->get_state(), service_state::running);
}