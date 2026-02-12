/**
 * @file test_event_bus_integration.cpp
 * @brief Integration tests for event_bus and messaging_system
 *
 * Tests the integration between common_system's event_bus and
 * messaging_system's message_bus for cross-module communication.
 */

#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>
#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/patterns/pub_sub.h>
#include <atomic>
#include <thread>
#include <chrono>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;
using namespace kcenon::messaging::patterns;
using namespace kcenon::common;

namespace {

// Custom event types for testing
struct message_received_event {
    std::string topic;
    std::string source;
    std::chrono::steady_clock::time_point timestamp;

    message_received_event(const std::string& t, const std::string& s)
        : topic(t), source(s), timestamp(std::chrono::steady_clock::now()) {}
};

struct message_published_event {
    std::string topic;
    int message_count;
    std::chrono::steady_clock::time_point timestamp;

    message_published_event(const std::string& t, int count)
        : topic(t), message_count(count), timestamp(std::chrono::steady_clock::now()) {}
};

struct message_error_event {
    std::string topic;
    std::string error_message;
    std::chrono::steady_clock::time_point timestamp;

    message_error_event(const std::string& t, const std::string& err)
        : topic(t), error_message(err), timestamp(std::chrono::steady_clock::now()) {}
};

} // anonymous namespace

class EventBusIntegrationTest : public MessagingFixture {
protected:
    simple_event_bus event_bus_;

    void SetUp() override {
        MessagingFixture::SetUp();
        event_bus_.start();
    }

    void TearDown() override {
        event_bus_.stop();
        MessagingFixture::TearDown();
    }
};

/**
 * @test Basic event bus and message bus interaction
 *
 * Verifies that events can be published through event_bus when
 * messages are received through message_bus.
 */
TEST_F(EventBusIntegrationTest, MessageBusTrigersEventBus) {
    std::atomic<int> event_count{0};
    std::string received_topic;

    // Subscribe to event bus
    auto event_sub = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event& evt) {
            event_count++;
            received_topic = evt.topic;
        }
    );

    // Subscribe to message bus and publish event on receive
    auto msg_sub = bus_->subscribe("events.test", [&]([[maybe_unused]] const message& msg) -> VoidResult {
        event_bus_.publish(message_received_event{"events.test", "test_source"});
        return ok();
    });
    ASSERT_TRUE(msg_sub.is_ok());

    // Publish message
    auto msg = create_test_message("events.test");
    ASSERT_TRUE(bus_->publish(msg).is_ok());

    // Wait for processing
    ASSERT_TRUE(wait_for_condition(
        [&]() { return event_count.load() >= 1; },
        std::chrono::seconds{2}
    ));

    EXPECT_EQ(event_count.load(), 1);
    EXPECT_EQ(received_topic, "events.test");

    event_bus_.unsubscribe(event_sub);
}

/**
 * @test Multiple event handlers
 *
 * Verifies that multiple handlers can subscribe to the same event type.
 */
TEST_F(EventBusIntegrationTest, MultipleEventHandlers) {
    std::atomic<int> handler1_count{0};
    std::atomic<int> handler2_count{0};
    std::atomic<int> handler3_count{0};

    // Multiple handlers for the same event
    auto sub1 = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) { handler1_count++; }
    );
    auto sub2 = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) { handler2_count++; }
    );
    auto sub3 = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) { handler3_count++; }
    );

    // Setup message bus subscriber
    auto msg_sub = bus_->subscribe("multi.test", [&](const message&) -> VoidResult {
        event_bus_.publish(message_received_event{"multi.test", "source"});
        return ok();
    });
    ASSERT_TRUE(msg_sub.is_ok());

    // Publish messages
    for (int i = 0; i < 5; ++i) {
        bus_->publish(create_test_message("multi.test"));
    }

    // Wait for processing
    ASSERT_TRUE(wait_for_condition(
        [&]() {
            return handler1_count.load() >= 5 &&
                   handler2_count.load() >= 5 &&
                   handler3_count.load() >= 5;
        },
        std::chrono::seconds{3}
    ));

    EXPECT_EQ(handler1_count.load(), 5);
    EXPECT_EQ(handler2_count.load(), 5);
    EXPECT_EQ(handler3_count.load(), 5);

    event_bus_.unsubscribe(sub1);
    event_bus_.unsubscribe(sub2);
    event_bus_.unsubscribe(sub3);
}

/**
 * @test Event filtering
 *
 * Verifies that events can be filtered before handler invocation.
 */
TEST_F(EventBusIntegrationTest, EventFiltering) {
    std::atomic<int> filtered_count{0};
    std::atomic<int> all_count{0};

    // Handler with filter (only "important" topics)
    auto filtered_sub = event_bus_.subscribe_filtered<message_received_event>(
        [&](const message_received_event&) { filtered_count++; },
        [](const message_received_event& evt) {
            return evt.topic.find("important") != std::string::npos;
        }
    );

    // Handler without filter
    auto all_sub = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) { all_count++; }
    );

    // Publish different events
    event_bus_.publish(message_received_event{"important.event", "source"});
    event_bus_.publish(message_received_event{"normal.event", "source"});
    event_bus_.publish(message_received_event{"important.alert", "source"});
    event_bus_.publish(message_received_event{"debug.log", "source"});

    EXPECT_EQ(filtered_count.load(), 2);  // Only "important" events
    EXPECT_EQ(all_count.load(), 4);       // All events

    event_bus_.unsubscribe(filtered_sub);
    event_bus_.unsubscribe(all_sub);
}

/**
 * @test Different event types
 *
 * Verifies that different event types are handled independently.
 */
TEST_F(EventBusIntegrationTest, DifferentEventTypes) {
    std::atomic<int> received_count{0};
    std::atomic<int> published_count{0};
    std::atomic<int> error_count{0};

    auto sub1 = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) { received_count++; }
    );
    auto sub2 = event_bus_.subscribe<message_published_event>(
        [&](const message_published_event&) { published_count++; }
    );
    auto sub3 = event_bus_.subscribe<message_error_event>(
        [&](const message_error_event&) { error_count++; }
    );

    // Publish different event types
    event_bus_.publish(message_received_event{"topic1", "source"});
    event_bus_.publish(message_published_event{"topic2", 10});
    event_bus_.publish(message_error_event{"topic3", "error occurred"});
    event_bus_.publish(message_received_event{"topic4", "source"});

    EXPECT_EQ(received_count.load(), 2);
    EXPECT_EQ(published_count.load(), 1);
    EXPECT_EQ(error_count.load(), 1);

    event_bus_.unsubscribe(sub1);
    event_bus_.unsubscribe(sub2);
    event_bus_.unsubscribe(sub3);
}

/**
 * @test Unsubscription
 *
 * Verifies that unsubscribed handlers no longer receive events.
 */
TEST_F(EventBusIntegrationTest, Unsubscription) {
    std::atomic<int> count{0};

    auto sub = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) { count++; }
    );

    // Publish and verify
    event_bus_.publish(message_received_event{"topic", "source"});
    EXPECT_EQ(count.load(), 1);

    // Unsubscribe
    event_bus_.unsubscribe(sub);

    // Publish again - should not increment
    event_bus_.publish(message_received_event{"topic", "source"});
    EXPECT_EQ(count.load(), 1);  // Still 1
}

/**
 * @test Concurrent event publishing
 *
 * Verifies thread-safety of concurrent event publishing.
 */
TEST_F(EventBusIntegrationTest, ConcurrentEventPublishing) {
    std::atomic<int> event_count{0};

    auto sub = event_bus_.subscribe<message_received_event>(
        [&](const message_received_event&) {
            event_count.fetch_add(1, std::memory_order_relaxed);
        }
    );

    const int num_threads = 4;
    const int events_per_thread = 100;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < events_per_thread; ++i) {
                event_bus_.publish(message_received_event{
                    "thread" + std::to_string(t),
                    "source"
                });
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(event_count.load(), num_threads * events_per_thread);

    event_bus_.unsubscribe(sub);
}

/**
 * @test Error callback handling
 *
 * Verifies that exception in handlers are caught and reported.
 */
TEST_F(EventBusIntegrationTest, ErrorCallbackHandling) {
    std::atomic<int> error_callback_count{0};
    std::string last_error_message;
    std::mutex error_mutex;

    // Set error callback
    event_bus_.set_error_callback([&](const std::string& msg, size_t, uint64_t) {
        std::lock_guard lock(error_mutex);
        error_callback_count++;
        last_error_message = msg;
    });

    // Subscribe with a throwing handler
    auto sub = event_bus_.subscribe<message_received_event>(
        [](const message_received_event&) {
            throw std::runtime_error("Test exception");
        }
    );

    // Publish event - should not crash
    event_bus_.publish(message_received_event{"topic", "source"});

    EXPECT_EQ(error_callback_count.load(), 1);
    EXPECT_TRUE(last_error_message.find("Test exception") != std::string::npos);

    event_bus_.unsubscribe(sub);
    event_bus_.clear_error_callback();
}

/**
 * @test Integration with common events
 *
 * Verifies integration with common_system's predefined events.
 */
TEST_F(EventBusIntegrationTest, CommonSystemEvents) {
    std::atomic<int> module_started_count{0};
    std::atomic<int> module_stopped_count{0};
    std::atomic<int> error_event_count{0};
    std::atomic<int> metric_event_count{0};

    auto sub1 = event_bus_.subscribe<events::module_started_event>(
        [&](const events::module_started_event&) { module_started_count++; }
    );
    auto sub2 = event_bus_.subscribe<events::module_stopped_event>(
        [&](const events::module_stopped_event&) { module_stopped_count++; }
    );
    auto sub3 = event_bus_.subscribe<events::error_event>(
        [&](const events::error_event&) { error_event_count++; }
    );
    auto sub4 = event_bus_.subscribe<events::metric_event>(
        [&](const events::metric_event&) { metric_event_count++; }
    );

    // Publish common events
    event_bus_.publish(events::module_started_event{"messaging_system"});
    event_bus_.publish(events::metric_event{"messages_processed", 100.0, "count"});
    event_bus_.publish(events::error_event{"messaging_system", "Test error", 500});
    event_bus_.publish(events::module_stopped_event{"messaging_system"});

    EXPECT_EQ(module_started_count.load(), 1);
    EXPECT_EQ(module_stopped_count.load(), 1);
    EXPECT_EQ(error_event_count.load(), 1);
    EXPECT_EQ(metric_event_count.load(), 1);

    event_bus_.unsubscribe(sub1);
    event_bus_.unsubscribe(sub2);
    event_bus_.unsubscribe(sub3);
    event_bus_.unsubscribe(sub4);
}
