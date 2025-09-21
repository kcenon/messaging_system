#include <gtest/gtest.h>
#include <kcenon/messaging/integrations/system_integrator.h>
#include <kcenon/messaging/core/config.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>

using namespace kcenon::messaging::integrations;
using namespace kcenon::messaging::config;
using namespace kcenon::messaging::core;

class MessagingIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_builder builder;
        config_ = builder
            .set_environment("testing")
            .set_worker_threads(4)
            .set_queue_size(5000)
            .enable_compression(true)
            .build();

        integrator_ = std::make_unique<system_integrator>(config_);
        ASSERT_TRUE(integrator_->initialize());
    }

    void TearDown() override {
        if (integrator_) {
            integrator_->shutdown();
        }
    }

    messaging_config config_;
    std::unique_ptr<system_integrator> integrator_;
};

TEST_F(MessagingIntegrationTest, EndToEndMessaging) {
    std::atomic<int> messages_received{0};
    std::vector<std::string> received_messages;
    std::mutex received_mutex;

    // Subscribe to test topic
    integrator_->subscribe("integration.test", [&](const message& msg) {
        messages_received++;
        std::lock_guard<std::mutex> lock(received_mutex);

        auto it = msg.payload.data.find("content");
        if (it != msg.payload.data.end() && std::holds_alternative<std::string>(it->second)) {
            received_messages.push_back(std::get<std::string>(it->second));
        }
    });

    // Publish multiple messages
    std::vector<std::string> test_messages = {
        "Hello World",
        "Integration Test",
        "Message Bus",
        "System Integration"
    };

    for (const auto& content : test_messages) {
        message_payload payload;
        payload.topic = "integration.test";
        payload.data["content"] = content;
        payload.data["timestamp"] = int64_t(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

        EXPECT_TRUE(integrator_->publish("integration.test", payload, "integration_test"));
    }

    // Wait for all messages to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(messages_received.load(), test_messages.size());

    std::lock_guard<std::mutex> lock(received_mutex);
    EXPECT_EQ(received_messages.size(), test_messages.size());

    for (const auto& expected : test_messages) {
        EXPECT_TRUE(std::find(received_messages.begin(), received_messages.end(), expected) != received_messages.end());
    }
}

TEST_F(MessagingIntegrationTest, ServiceDiscoveryAndCommunication) {
    // Test that services can discover and communicate with each other
    auto& container = integrator_->get_container();

    // Verify core services are registered
    EXPECT_TRUE(container.is_registered("message_bus"));
    EXPECT_TRUE(container.is_registered("config"));

    // Get message bus instance through container
    auto message_bus_ptr = container.resolve<message_bus>("message_bus");
    ASSERT_NE(message_bus_ptr, nullptr);
    EXPECT_TRUE(message_bus_ptr->is_running());

    // Get config through container
    auto config = container.resolve<messaging_config>("config");
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->environment, "testing");
}

TEST_F(MessagingIntegrationTest, MultiTopicCommunication) {
    std::atomic<int> topic1_count{0};
    std::atomic<int> topic2_count{0};
    std::atomic<int> topic3_count{0};

    // Subscribe to multiple topics
    integrator_->subscribe("topic.1", [&](const message& msg) { topic1_count++; });
    integrator_->subscribe("topic.2", [&](const message& msg) { topic2_count++; });
    integrator_->subscribe("topic.3", [&](const message& msg) { topic3_count++; });

    // Publish to different topics
    message_payload payload1;
    payload1.topic = "topic.1";
    payload1.data["message"] = std::string("Topic 1 message");

    message_payload payload2;
    payload2.topic = "topic.2";
    payload2.data["message"] = std::string("Topic 2 message");

    message_payload payload3;
    payload3.topic = "topic.3";
    payload3.data["message"] = std::string("Topic 3 message");

    // Publish multiple messages to each topic
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(integrator_->publish("topic.1", payload1));
        EXPECT_TRUE(integrator_->publish("topic.2", payload2));
        EXPECT_TRUE(integrator_->publish("topic.3", payload3));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    EXPECT_EQ(topic1_count.load(), 3);
    EXPECT_EQ(topic2_count.load(), 3);
    EXPECT_EQ(topic3_count.load(), 3);
}

TEST_F(MessagingIntegrationTest, HighVolumeMessaging) {
    constexpr int total_messages = 1000;
    std::atomic<int> received_count{0};

    integrator_->subscribe("high.volume", [&](const message& msg) {
        received_count++;
    });

    // Publish high volume of messages
    for (int i = 0; i < total_messages; ++i) {
        message_payload payload;
        payload.topic = "high.volume";
        payload.data["sequence"] = int64_t(i);
        payload.data["batch"] = std::string("high_volume_test");

        EXPECT_TRUE(integrator_->publish("high.volume", payload));
    }

    // Wait for processing with generous timeout
    auto start_time = std::chrono::steady_clock::now();
    while (received_count.load() < total_messages) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > std::chrono::seconds(10)) {
            break; // Timeout after 10 seconds
        }
    }

    EXPECT_EQ(received_count.load(), total_messages);
}

TEST_F(MessagingIntegrationTest, SystemHealthMonitoring) {
    // Initial health check
    auto initial_health = integrator_->check_system_health();
    EXPECT_TRUE(initial_health.message_bus_healthy);
    EXPECT_GE(initial_health.active_services, 0);

    // Generate some activity
    std::atomic<int> activity_count{0};
    integrator_->subscribe("health.test", [&](const message& msg) {
        activity_count++;
    });

    // Send messages to generate activity
    for (int i = 0; i < 10; ++i) {
        message_payload payload;
        payload.topic = "health.test";
        payload.data["activity"] = int64_t(i);
        integrator_->publish("health.test", payload);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Check health after activity
    auto updated_health = integrator_->check_system_health();
    EXPECT_TRUE(updated_health.message_bus_healthy);
    EXPECT_GE(updated_health.total_messages_processed, initial_health.total_messages_processed);
    EXPECT_EQ(activity_count.load(), 10);
}

TEST_F(MessagingIntegrationTest, ConcurrentProducersConsumers) {
    constexpr int num_producers = 4;
    constexpr int num_consumers = 3;
    constexpr int messages_per_producer = 50;

    std::atomic<int> total_consumed{0};
    std::vector<std::atomic<int>> consumer_counts(num_consumers);

    // Start consumers
    std::vector<std::thread> consumer_threads;
    for (int c = 0; c < num_consumers; ++c) {
        consumer_threads.emplace_back([this, c, &consumer_counts, &total_consumed]() {
            integrator_->subscribe("concurrent.test", [&, c](const message& msg) {
                consumer_counts[c]++;
                total_consumed++;
            });
        });
    }

    // Let consumers register
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Start producers
    std::vector<std::thread> producer_threads;
    for (int p = 0; p < num_producers; ++p) {
        producer_threads.emplace_back([this, p, messages_per_producer]() {
            for (int i = 0; i < messages_per_producer; ++i) {
                message_payload payload;
                payload.topic = "concurrent.test";
                payload.data["producer_id"] = int64_t(p);
                payload.data["message_id"] = int64_t(i);

                integrator_->publish("concurrent.test", payload, "producer_" + std::to_string(p));
            }
        });
    }

    // Wait for all producers to finish
    for (auto& thread : producer_threads) {
        thread.join();
    }

    // Wait for message processing
    auto start_time = std::chrono::steady_clock::now();
    int expected_total = num_producers * messages_per_producer * num_consumers;

    while (total_consumed.load() < expected_total) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > std::chrono::seconds(5)) {
            break; // Timeout
        }
    }

    // Each consumer should receive all messages (fanout behavior)
    EXPECT_EQ(total_consumed.load(), expected_total);

    // Clean up consumer threads
    for (auto& thread : consumer_threads) {
        if (thread.joinable()) {
            thread.detach(); // Detach since subscribers are still active
        }
    }
}

TEST_F(MessagingIntegrationTest, MessagePriorityIntegration) {
    std::vector<message_priority> received_priorities;
    std::mutex priorities_mutex;

    integrator_->subscribe("priority.integration", [&](const message& msg) {
        std::lock_guard<std::mutex> lock(priorities_mutex);
        received_priorities.push_back(msg.metadata.priority);
    });

    // Create messages with different priorities
    std::vector<std::pair<message_priority, std::string>> test_messages = {
        {message_priority::low, "Low priority message"},
        {message_priority::high, "High priority message"},
        {message_priority::critical, "Critical priority message"},
        {message_priority::normal, "Normal priority message"}
    };

    // Publish in mixed order
    for (const auto& [priority, content] : test_messages) {
        message msg;
        msg.payload.topic = "priority.integration";
        msg.payload.data["content"] = content;
        msg.metadata.priority = priority;

        auto* bus = integrator_->get_message_bus();
        ASSERT_NE(bus, nullptr);
        EXPECT_TRUE(bus->publish(msg));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    std::lock_guard<std::mutex> lock(priorities_mutex);
    EXPECT_EQ(received_priorities.size(), test_messages.size());

    // Verify priority ordering (critical, high, normal, low)
    if (received_priorities.size() >= 4) {
        EXPECT_EQ(received_priorities[0], message_priority::critical);
        EXPECT_EQ(received_priorities[1], message_priority::high);
        EXPECT_EQ(received_priorities[2], message_priority::normal);
        EXPECT_EQ(received_priorities[3], message_priority::low);
    }
}