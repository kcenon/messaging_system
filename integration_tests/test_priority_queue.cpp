#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;
using namespace kcenon::common;

/**
 * @brief Integration tests for Message Queue + Priority
 */
class PriorityQueueTest : public ::testing::Test {
protected:
    std::shared_ptr<backend_interface> backend_;
    std::shared_ptr<message_bus> bus_;

    void SetUp() override {
        // Create standalone backend with 2 worker threads
        backend_ = std::make_shared<standalone_backend>(2);

        // Create message bus with priority queue enabled
        message_bus_config config;
        config.queue_capacity = 1000;
        config.worker_threads = 2;
        config.enable_priority_queue = true;  // Enable for priority tests

        bus_ = std::make_shared<message_bus>(backend_, config);
        ASSERT_TRUE(bus_->start().is_ok());
    }

    void TearDown() override {
        if (bus_) {
            bus_->stop();
        }
    }
};

TEST_F(PriorityQueueTest, DISABLED_PriorityOrdering) {
    std::vector<message> received_messages;
    std::mutex mutex;

    auto sub_result = bus_->subscribe(
        "test.priority",
        create_storing_callback(received_messages, mutex)
    );
    ASSERT_TRUE(sub_result.is_ok());

    // Publish messages with different priorities
    auto low_msg = message_builder()
        .topic("test.priority")
        .priority(message_priority::low)
        .build();
    ASSERT_TRUE(low_msg.is_ok());

    auto high_msg = message_builder()
        .topic("test.priority")
        .priority(message_priority::high)
        .build();
    ASSERT_TRUE(high_msg.is_ok());

    auto normal_msg = message_builder()
        .topic("test.priority")
        .priority(message_priority::normal)
        .build();
    ASSERT_TRUE(normal_msg.is_ok());

    // Publish in mixed order
    ASSERT_TRUE(bus_->publish(low_msg.unwrap()).is_ok());
    ASSERT_TRUE(bus_->publish(high_msg.unwrap()).is_ok());
    ASSERT_TRUE(bus_->publish(normal_msg.unwrap()).is_ok());

    // Wait for all messages
    ASSERT_TRUE(wait_for_condition(
        [&]() {
            std::lock_guard lock(mutex);
            return received_messages.size() >= 3;
        },
        std::chrono::seconds{2}
    ));

    // High priority should be processed first
    std::lock_guard lock(mutex);
    EXPECT_EQ(received_messages[0].metadata().priority, message_priority::high);
}

TEST_F(PriorityQueueTest, DISABLED_CriticalPriorityFirst) {
    std::vector<message_priority> received_priorities;
    std::mutex mutex;

    auto sub_result = bus_->subscribe(
        "test.critical",
        [&](const message& msg) -> kcenon::common::VoidResult {
            std::lock_guard lock(mutex);
            received_priorities.push_back(msg.metadata().priority);
            return kcenon::common::ok();
        }
    );
    ASSERT_TRUE(sub_result.is_ok());

    // Publish multiple messages with different priorities
    std::vector<message_priority> priorities = {
        message_priority::low,
        message_priority::critical,
        message_priority::normal,
        message_priority::high,
        message_priority::lowest
    };

    for (auto priority : priorities) {
        auto msg = message_builder()
            .topic("test.critical")
            .priority(priority)
            .build();
        ASSERT_TRUE(msg.is_ok());
        ASSERT_TRUE(bus_->publish(msg.unwrap()).is_ok());
    }

    // Wait for all messages
    ASSERT_TRUE(wait_for_condition(
        [&]() {
            std::lock_guard lock(mutex);
            return received_priorities.size() >= priorities.size();
        },
        std::chrono::seconds{2}
    ));

    // Critical should be first
    std::lock_guard lock(mutex);
    EXPECT_EQ(received_priorities[0], message_priority::critical);
}

TEST_F(PriorityQueueTest, QueueCapacity) {
    // Get initial statistics
    auto initial_stats = bus_->get_statistics();

    // Try to overflow the queue by publishing many messages rapidly
    const int overflow_count = 2000; // More than queue capacity (1000)

    for (int i = 0; i < overflow_count; ++i) {
        auto msg = create_test_message("test.overflow");
        bus_->publish(msg); // Ignore result, some may be dropped
    }

    // Wait a bit for processing
    std::this_thread::sleep_for(std::chrono::milliseconds{500});

    auto final_stats = bus_->get_statistics();

    // Either messages were queued or dropped, but system should remain stable
    EXPECT_GT(final_stats.messages_published, initial_stats.messages_published);
}

TEST_F(PriorityQueueTest, MixedPriorityHighThroughput) {
    MessageCounter counter;

    auto sub_result = bus_->subscribe("test.mixed", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish many messages with random priorities
    const int count = 500;
    for (int i = 0; i < count; ++i) {
        auto priority = static_cast<message_priority>(i % 6);
        auto msg = message_builder()
            .topic("test.mixed")
            .priority(priority)
            .build();
        ASSERT_TRUE(msg.is_ok());
        ASSERT_TRUE(bus_->publish(msg.unwrap()).is_ok());
    }

    // All messages should eventually be delivered
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= count; },
        std::chrono::seconds{5}
    ));

    EXPECT_EQ(counter.count(), count);
}
