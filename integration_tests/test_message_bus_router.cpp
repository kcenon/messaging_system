#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;

/**
 * @brief Integration tests for Message Bus + Topic Router
 */
class MessageBusRouterTest : public MessagingFixture {};

TEST_F(MessageBusRouterTest, PublishSubscribeFlow) {
    MessageCounter counter;

    // Subscribe to topic
    auto sub_result = bus_->subscribe("test.topic", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish message
    auto msg = create_test_message("test.topic");
    ASSERT_TRUE(bus_->publish(msg).is_ok());

    // Wait for message delivery
    ASSERT_TRUE(wait_for_condition([&]() { return counter.count() >= 1; }));
    EXPECT_EQ(counter.count(), 1);
}

TEST_F(MessageBusRouterTest, WildcardPatternMatching) {
    MessageCounter counter;

    // Subscribe with wildcard pattern
    auto sub_result = bus_->subscribe("test.*", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish to matching topics
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic1")).is_ok());
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic2")).is_ok());
    ASSERT_TRUE(bus_->publish(create_test_message("other.topic")).is_ok());

    // Wait and verify only matching messages received
    ASSERT_TRUE(wait_for_condition([&]() { return counter.count() >= 2; }));
    EXPECT_EQ(counter.count(), 2);
}

TEST_F(MessageBusRouterTest, MultiLevelWildcard) {
    MessageCounter counter;

    // Subscribe with multi-level wildcard
    auto sub_result = bus_->subscribe("test.#", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish to various depths
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic")).is_ok());
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic.deep")).is_ok());
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic.very.deep")).is_ok());
    ASSERT_TRUE(bus_->publish(create_test_message("other.topic")).is_ok());

    // Wait and verify
    ASSERT_TRUE(wait_for_condition([&]() { return counter.count() >= 3; }));
    EXPECT_EQ(counter.count(), 3);
}

TEST_F(MessageBusRouterTest, MultipleSubscribers) {
    MessageCounter counter1, counter2, counter3;

    // Multiple subscribers to same topic
    auto sub1 = bus_->subscribe("test.topic", create_counting_callback(counter1));
    auto sub2 = bus_->subscribe("test.topic", create_counting_callback(counter2));
    auto sub3 = bus_->subscribe("test.topic", create_counting_callback(counter3));

    ASSERT_TRUE(sub1.is_ok() && sub2.is_ok() && sub3.is_ok());

    // Publish one message
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic")).is_ok());

    // All subscribers should receive it
    ASSERT_TRUE(wait_for_condition([&]() {
        return counter1.count() >= 1 && counter2.count() >= 1 && counter3.count() >= 1;
    }));

    EXPECT_EQ(counter1.count(), 1);
    EXPECT_EQ(counter2.count(), 1);
    EXPECT_EQ(counter3.count(), 1);
}

TEST_F(MessageBusRouterTest, UnsubscribeStopsDelivery) {
    MessageCounter counter;

    // Subscribe
    auto sub_result = bus_->subscribe("test.topic", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());
    auto sub_id = sub_result.unwrap();

    // Publish first message
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic")).is_ok());
    ASSERT_TRUE(wait_for_condition([&]() { return counter.count() >= 1; }));

    // Unsubscribe
    ASSERT_TRUE(bus_->unsubscribe(sub_id).is_ok());

    // Reset counter and publish again
    counter.reset();
    ASSERT_TRUE(bus_->publish(create_test_message("test.topic")).is_ok());

    // Wait a bit and verify no delivery
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    EXPECT_EQ(counter.count(), 0);
}

TEST_F(MessageBusRouterTest, HighThroughput) {
    MessageCounter counter;

    auto sub_result = bus_->subscribe("test.topic", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish many messages
    const int message_count = 1000;
    for (int i = 0; i < message_count; ++i) {
        ASSERT_TRUE(bus_->publish(create_test_message("test.topic")).is_ok());
    }

    // Wait for all messages
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= message_count; },
        std::chrono::seconds{5}
    ));

    EXPECT_EQ(counter.count(), message_count);
}

TEST_F(MessageBusRouterTest, MessageOrdering) {
    std::vector<message> received_messages;
    std::mutex mutex;

    auto sub_result = bus_->subscribe(
        "test.topic",
        create_storing_callback(received_messages, mutex)
    );
    ASSERT_TRUE(sub_result.is_ok());

    // Publish messages with metadata
    const int count = 10;
    for (int i = 0; i < count; ++i) {
        auto msg = create_test_message("test.topic");
        msg.metadata().id = "msg_" + std::to_string(i);
        ASSERT_TRUE(bus_->publish(msg).is_ok());
    }

    // Wait for all messages
    ASSERT_TRUE(wait_for_condition(
        [&]() {
            std::lock_guard lock(mutex);
            return received_messages.size() >= count;
        },
        std::chrono::seconds{2}
    ));

    // Verify order (FIFO)
    std::lock_guard lock(mutex);
    for (int i = 0; i < count; ++i) {
        EXPECT_EQ(received_messages[i].metadata().id, "msg_" + std::to_string(i));
    }
}
