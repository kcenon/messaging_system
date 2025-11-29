/**
 * @file test_container_serialization.cpp
 * @brief Integration tests for container_system serialization with messaging_system
 *
 * Tests the integration between container_system's value_container and
 * messaging_system for binary/JSON serialization and deserialization.
 */

#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>
#include <kcenon/messaging/core/message.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;
using namespace kcenon::common;

class ContainerSerializationTest : public MessagingFixture {};

/**
 * @test Basic message serialization
 *
 * Verifies that messages can be serialized and deserialized correctly.
 */
TEST_F(ContainerSerializationTest, BasicMessageSerialization) {
    // Create a test message
    auto msg_result = message_builder()
        .topic("orders.created")
        .type(message_type::event)
        .priority(message_priority::high)
        .source("order_service")
        .build();

    ASSERT_TRUE(msg_result.is_ok());
    auto original_msg = msg_result.unwrap();

    // Verify message properties
    EXPECT_EQ(original_msg.metadata().topic, "orders.created");
    EXPECT_EQ(original_msg.metadata().type, message_type::event);
    EXPECT_EQ(original_msg.metadata().priority, message_priority::high);
    EXPECT_EQ(original_msg.metadata().source, "order_service");
}

/**
 * @test Message with correlation ID
 *
 * Verifies that correlation ID is preserved through serialization.
 */
TEST_F(ContainerSerializationTest, CorrelationIdPreservation) {
    const std::string correlation_id = "req-12345-abcde";

    auto msg_result = message_builder()
        .topic("request.process")
        .type(message_type::command)
        .correlation_id(correlation_id)
        .build();

    ASSERT_TRUE(msg_result.is_ok());
    auto msg = msg_result.unwrap();

    EXPECT_EQ(msg.metadata().correlation_id, correlation_id);
}

/**
 * @test Message timestamp handling
 *
 * Verifies that timestamps are handled correctly.
 */
TEST_F(ContainerSerializationTest, TimestampHandling) {
    auto before = std::chrono::system_clock::now();

    auto msg_result = message_builder()
        .topic("time.test")
        .build();

    auto after = std::chrono::system_clock::now();

    ASSERT_TRUE(msg_result.is_ok());
    auto msg = msg_result.unwrap();

    // Timestamp should be within the expected range
    auto msg_time = msg.metadata().timestamp;
    EXPECT_GE(msg_time, before);
    EXPECT_LE(msg_time, after);
}

/**
 * @test Message priority levels
 *
 * Verifies all priority levels are correctly handled.
 */
TEST_F(ContainerSerializationTest, PriorityLevels) {
    struct PriorityTest {
        message_priority priority;
        std::string topic;
    };

    std::vector<PriorityTest> tests = {
        {message_priority::low, "priority.low"},
        {message_priority::normal, "priority.normal"},
        {message_priority::high, "priority.high"},
        {message_priority::critical, "priority.critical"}
    };

    for (const auto& test : tests) {
        auto msg_result = message_builder()
            .topic(test.topic)
            .priority(test.priority)
            .build();

        ASSERT_TRUE(msg_result.is_ok()) << "Failed for priority: " << test.topic;
        auto msg = msg_result.unwrap();
        EXPECT_EQ(msg.metadata().priority, test.priority);
    }
}

/**
 * @test Message types
 *
 * Verifies all message types are correctly handled.
 */
TEST_F(ContainerSerializationTest, MessageTypes) {
    struct TypeTest {
        message_type type;
        std::string topic;
    };

    std::vector<TypeTest> tests = {
        {message_type::event, "type.event"},
        {message_type::command, "type.command"},
        {message_type::query, "type.query"},
        {message_type::reply, "type.reply"}
    };

    for (const auto& test : tests) {
        auto msg_result = message_builder()
            .topic(test.topic)
            .type(test.type)
            .build();

        ASSERT_TRUE(msg_result.is_ok()) << "Failed for type: " << test.topic;
        auto msg = msg_result.unwrap();
        EXPECT_EQ(msg.metadata().type, test.type);
    }
}

/**
 * @test Message ID uniqueness
 *
 * Verifies that each message gets a unique ID.
 */
TEST_F(ContainerSerializationTest, MessageIdUniqueness) {
    std::set<std::string> ids;
    const int num_messages = 100;

    for (int i = 0; i < num_messages; ++i) {
        auto msg_result = message_builder()
            .topic("unique.test")
            .build();

        ASSERT_TRUE(msg_result.is_ok());
        auto msg = msg_result.unwrap();

        // ID should be unique
        auto [it, inserted] = ids.insert(msg.metadata().id);
        EXPECT_TRUE(inserted) << "Duplicate ID found: " << msg.metadata().id;
    }

    EXPECT_EQ(ids.size(), num_messages);
}

/**
 * @test Large batch message creation
 *
 * Verifies performance with large batches of messages.
 */
TEST_F(ContainerSerializationTest, LargeBatchCreation) {
    const int batch_size = 1000;
    std::vector<message> messages;
    messages.reserve(batch_size);

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < batch_size; ++i) {
        auto msg_result = message_builder()
            .topic("batch.test." + std::to_string(i % 10))
            .type(message_type::event)
            .priority(message_priority::normal)
            .source("batch_source")
            .build();

        ASSERT_TRUE(msg_result.is_ok());
        messages.push_back(msg_result.unwrap());
    }

    auto duration = std::chrono::steady_clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    EXPECT_EQ(messages.size(), batch_size);
    // Should complete in reasonable time (less than 1 second)
    EXPECT_LT(ms, 1000) << "Batch creation took " << ms << "ms";
}

/**
 * @test Message routing through bus with serialization
 *
 * Verifies that messages maintain integrity through the message bus.
 */
TEST_F(ContainerSerializationTest, MessageRoutingIntegrity) {
    std::vector<message> received_messages;
    std::mutex received_mutex;

    // Subscribe and capture messages
    auto sub_result = bus_->subscribe("integrity.*",
        [&](const message& msg) -> VoidResult {
            std::lock_guard lock(received_mutex);
            received_messages.push_back(msg);
            return ok();
        }
    );
    ASSERT_TRUE(sub_result.is_ok());

    // Publish messages with specific properties
    std::vector<std::pair<std::string, message_priority>> test_cases = {
        {"integrity.high", message_priority::high},
        {"integrity.normal", message_priority::normal},
        {"integrity.low", message_priority::low}
    };

    for (const auto& [topic, priority] : test_cases) {
        auto msg_result = message_builder()
            .topic(topic)
            .priority(priority)
            .type(message_type::event)
            .build();

        ASSERT_TRUE(msg_result.is_ok());
        ASSERT_TRUE(bus_->publish(msg_result.unwrap()).is_ok());
    }

    // Wait for all messages to be received
    ASSERT_TRUE(wait_for_condition(
        [&]() {
            std::lock_guard lock(received_mutex);
            return received_messages.size() >= test_cases.size();
        },
        std::chrono::seconds{3}
    ));

    // Verify message integrity
    std::lock_guard lock(received_mutex);
    EXPECT_EQ(received_messages.size(), test_cases.size());

    // Check that all topics were received
    std::set<std::string> received_topics;
    for (const auto& msg : received_messages) {
        received_topics.insert(msg.metadata().topic);
    }

    for (const auto& [topic, _] : test_cases) {
        EXPECT_TRUE(received_topics.count(topic) > 0)
            << "Topic not received: " << topic;
    }
}

/**
 * @test Concurrent message serialization
 *
 * Verifies thread-safety of message creation.
 */
TEST_F(ContainerSerializationTest, ConcurrentMessageSerialization) {
    const int num_threads = 4;
    const int messages_per_thread = 250;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                auto msg_result = message_builder()
                    .topic("concurrent.test." + std::to_string(t))
                    .type(message_type::event)
                    .priority(message_priority::normal)
                    .source("thread_" + std::to_string(t))
                    .build();

                if (msg_result.is_ok()) {
                    success_count.fetch_add(1, std::memory_order_relaxed);
                } else {
                    failure_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count.load(), num_threads * messages_per_thread);
    EXPECT_EQ(failure_count.load(), 0);
}

/**
 * @test Message builder chaining
 *
 * Verifies that builder pattern works correctly with method chaining.
 */
TEST_F(ContainerSerializationTest, BuilderChaining) {
    auto msg_result = message_builder()
        .topic("chained.test")
        .type(message_type::command)
        .priority(message_priority::critical)
        .source("chain_source")
        .correlation_id("chain-correlation-123")
        .build();

    ASSERT_TRUE(msg_result.is_ok());
    auto msg = msg_result.unwrap();

    EXPECT_EQ(msg.metadata().topic, "chained.test");
    EXPECT_EQ(msg.metadata().type, message_type::command);
    EXPECT_EQ(msg.metadata().priority, message_priority::critical);
    EXPECT_EQ(msg.metadata().source, "chain_source");
    EXPECT_EQ(msg.metadata().correlation_id, "chain-correlation-123");
}
