#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message_types.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace kcenon::messaging::core;

class MessageBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.worker_threads = 2;
        config_.max_queue_size = 1000;
        config_.enable_priority_queue = true;
        config_.enable_metrics = true;

        message_bus_ = std::make_unique<message_bus>(config_);
    }

    void TearDown() override {
        if (message_bus_ && message_bus_->is_running()) {
            message_bus_->shutdown();
        }
    }

    message_bus_config config_;
    std::unique_ptr<message_bus> message_bus_;
};

TEST_F(MessageBusTest, InitializationAndShutdown) {
    EXPECT_FALSE(message_bus_->is_running());

    EXPECT_TRUE(message_bus_->initialize());
    EXPECT_TRUE(message_bus_->is_running());

    message_bus_->shutdown();
    EXPECT_FALSE(message_bus_->is_running());
}

TEST_F(MessageBusTest, BasicPublishSubscribe) {
    ASSERT_TRUE(message_bus_->initialize());

    std::atomic<int> message_count{0};
    std::string received_topic;
    std::string received_data;

    // Subscribe to a topic
    message_bus_->subscribe("test_topic", [&](const message& msg) {
        message_count++;
        received_topic = msg.payload.topic;
        if (!msg.payload.data.empty()) {
            auto it = msg.payload.data.find("content");
            if (it != msg.payload.data.end()) {
                if (std::holds_alternative<std::string>(it->second)) {
                    received_data = std::get<std::string>(it->second);
                }
            }
        }
    });

    // Publish a message
    message_payload payload;
    payload.topic = "test_topic";
    payload.data["content"] = std::string("Hello, World!");

    EXPECT_TRUE(message_bus_->publish("test_topic", payload, "test_sender"));

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(message_count.load(), 1);
    EXPECT_EQ(received_topic, "test_topic");
    EXPECT_EQ(received_data, "Hello, World!");
}

TEST_F(MessageBusTest, MultipleSubscribers) {
    ASSERT_TRUE(message_bus_->initialize());

    std::atomic<int> subscriber1_count{0};
    std::atomic<int> subscriber2_count{0};

    // Multiple subscribers to the same topic
    message_bus_->subscribe("broadcast_topic", [&](const message& msg) {
        subscriber1_count++;
    });

    message_bus_->subscribe("broadcast_topic", [&](const message& msg) {
        subscriber2_count++;
    });

    // Publish a message
    message_payload payload;
    payload.topic = "broadcast_topic";
    payload.data["test"] = std::string("broadcast");

    EXPECT_TRUE(message_bus_->publish("broadcast_topic", payload));

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_EQ(subscriber1_count.load(), 1);
    EXPECT_EQ(subscriber2_count.load(), 1);
}

TEST_F(MessageBusTest, MessagePriority) {
    ASSERT_TRUE(message_bus_->initialize());

    std::vector<message_priority> received_priorities;
    std::mutex received_mutex;

    message_bus_->subscribe("priority_topic", [&](const message& msg) {
        std::lock_guard<std::mutex> lock(received_mutex);
        received_priorities.push_back(msg.metadata.priority);
    });

    // Publish messages with different priorities
    message msg_low;
    msg_low.payload.topic = "priority_topic";
    msg_low.metadata.priority = message_priority::low;
    msg_low.payload.data["priority"] = std::string("low");

    message msg_high;
    msg_high.payload.topic = "priority_topic";
    msg_high.metadata.priority = message_priority::high;
    msg_high.payload.data["priority"] = std::string("high");

    message msg_critical;
    msg_critical.payload.topic = "priority_topic";
    msg_critical.metadata.priority = message_priority::critical;
    msg_critical.payload.data["priority"] = std::string("critical");

    // Publish in reverse priority order
    EXPECT_TRUE(message_bus_->publish(msg_low));
    EXPECT_TRUE(message_bus_->publish(msg_high));
    EXPECT_TRUE(message_bus_->publish(msg_critical));

    // Wait for message processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Verify messages were processed in priority order
    ASSERT_EQ(received_priorities.size(), 3);
    EXPECT_EQ(received_priorities[0], message_priority::critical);
    EXPECT_EQ(received_priorities[1], message_priority::high);
    EXPECT_EQ(received_priorities[2], message_priority::low);
}

TEST_F(MessageBusTest, Statistics) {
    ASSERT_TRUE(message_bus_->initialize());

    auto initial_stats = message_bus_->get_statistics();
    EXPECT_EQ(initial_stats.messages_published, 0);
    EXPECT_EQ(initial_stats.messages_processed, 0);

    // Subscribe to capture messages
    std::atomic<int> processed_count{0};
    message_bus_->subscribe("stats_topic", [&](const message& msg) {
        processed_count++;
    });

    // Publish several messages
    message_payload payload;
    payload.topic = "stats_topic";

    for (int i = 0; i < 5; ++i) {
        payload.data["message_id"] = int64_t(i);
        EXPECT_TRUE(message_bus_->publish("stats_topic", payload));
    }

    // Wait for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto final_stats = message_bus_->get_statistics();
    EXPECT_EQ(final_stats.messages_published, 5);
    EXPECT_EQ(processed_count.load(), 5);
}

TEST_F(MessageBusTest, TopicManagement) {
    ASSERT_TRUE(message_bus_->initialize());

    // Initially no topics
    auto topics = message_bus_->get_topics();
    EXPECT_TRUE(topics.empty());

    // Subscribe to topics
    message_bus_->subscribe("topic1", [](const message& msg) {});
    message_bus_->subscribe("topic2", [](const message& msg) {});
    message_bus_->subscribe("topic1", [](const message& msg) {}); // Duplicate topic

    topics = message_bus_->get_topics();
    EXPECT_EQ(topics.size(), 2);
    EXPECT_TRUE(std::find(topics.begin(), topics.end(), "topic1") != topics.end());
    EXPECT_TRUE(std::find(topics.begin(), topics.end(), "topic2") != topics.end());

    // Check subscriber counts
    EXPECT_EQ(message_bus_->get_subscriber_count("topic1"), 2);
    EXPECT_EQ(message_bus_->get_subscriber_count("topic2"), 1);
    EXPECT_EQ(message_bus_->get_subscriber_count("nonexistent"), 0);
}

TEST_F(MessageBusTest, ConcurrentPublishing) {
    ASSERT_TRUE(message_bus_->initialize());

    std::atomic<int> total_received{0};
    constexpr int num_threads = 4;
    constexpr int messages_per_thread = 25;

    message_bus_->subscribe("concurrent_topic", [&](const message& msg) {
        total_received++;
    });

    std::vector<std::thread> publishers;
    for (int t = 0; t < num_threads; ++t) {
        publishers.emplace_back([this, t, messages_per_thread]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                message_payload payload;
                payload.topic = "concurrent_topic";
                payload.data["thread_id"] = int64_t(t);
                payload.data["message_id"] = int64_t(i);

                message_bus_->publish("concurrent_topic", payload);
            }
        });
    }

    for (auto& thread : publishers) {
        thread.join();
    }

    // Wait for all messages to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_EQ(total_received.load(), num_threads * messages_per_thread);
}