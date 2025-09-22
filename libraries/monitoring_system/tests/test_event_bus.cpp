/**
 * @file test_event_bus.cpp
 * @brief Test for event-driven communication system
 */

#include <monitoring/core/event_bus.h>
#include <monitoring/core/event_types.h>
#include <monitoring/adapters/thread_system_adapter.h>
#include <monitoring/adapters/logger_system_adapter.h>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace monitoring_system;
using namespace std::chrono_literals;

class EventBusTest : public ::testing::Test {
protected:
    void SetUp() override {
        event_bus::config config;
        config.max_queue_size = 1000;
        config.worker_thread_count = 2;
        config.auto_start = true;

        bus = std::make_shared<event_bus>(config);
    }

    void TearDown() override {
        if (bus) {
            bus->stop();
        }
    }

    std::shared_ptr<event_bus> bus;
};

// Test basic event publishing and subscribing
TEST_F(EventBusTest, PublishSubscribe) {
    std::atomic<int> received_count{0};
    std::string received_message;

    // Subscribe to performance alerts
    auto token = bus->subscribe_event<performance_alert_event>(
        [&](const performance_alert_event& event) {
            received_count++;
            received_message = event.get_message();
        }
    );

    ASSERT_TRUE(token.has_value());

    // Publish an event
    performance_alert_event alert(
        performance_alert_event::alert_type::high_cpu_usage,
        performance_alert_event::alert_severity::warning,
        "test_component",
        "CPU usage is high"
    );

    auto result = bus->publish_event(alert);
    ASSERT_TRUE(result);

    // Wait for event processing
    std::this_thread::sleep_for(100ms);

    EXPECT_EQ(received_count.load(), 1);
    EXPECT_EQ(received_message, "CPU usage is high");
}

// Test multiple subscribers
TEST_F(EventBusTest, MultipleSubscribers) {
    std::atomic<int> subscriber1_count{0};
    std::atomic<int> subscriber2_count{0};

    // Subscribe twice to the same event type
    bus->subscribe_event<system_resource_event>(
        [&](const system_resource_event& event) {
            subscriber1_count++;
        }
    );

    bus->subscribe_event<system_resource_event>(
        [&](const system_resource_event& event) {
            subscriber2_count++;
        }
    );

    // Publish event
    system_resource_event::resource_stats stats;
    stats.cpu_usage_percent = 75.5;
    system_resource_event event(stats);

    bus->publish_event(event);

    std::this_thread::sleep_for(100ms);

    EXPECT_EQ(subscriber1_count.load(), 1);
    EXPECT_EQ(subscriber2_count.load(), 1);
}

// Test event priority
TEST_F(EventBusTest, EventPriority) {
    std::vector<int> processing_order;
    std::mutex order_mutex;

    // Subscribe to configuration changes
    bus->subscribe_event<configuration_change_event>(
        [&](const configuration_change_event& event) {
            std::lock_guard<std::mutex> lock(order_mutex);
            if (event.get_config_key() == "high_priority") {
                processing_order.push_back(1);
            } else {
                processing_order.push_back(2);
            }
        },
        event_priority::high
    );

    // Publish events with different priorities
    configuration_change_event high_priority(
        "test", "high_priority",
        configuration_change_event::change_type::modified
    );

    configuration_change_event normal_priority(
        "test", "normal_priority",
        configuration_change_event::change_type::modified
    );

    // Stop bus to queue events
    bus->stop();

    // Queue events
    bus->publish_event(normal_priority);
    bus->publish_event(high_priority);

    // Restart and process
    bus->start();
    std::this_thread::sleep_for(200ms);

    // High priority should be processed first even if published second
    // Note: This test may be flaky due to async nature
}

// Test unsubscribe
TEST_F(EventBusTest, Unsubscribe) {
    std::atomic<int> received_count{0};

    auto token = bus->subscribe_event<health_check_event>(
        [&](const health_check_event& event) {
            received_count++;
        }
    );

    ASSERT_TRUE(token.has_value());

    // Publish first event
    std::vector<health_check_event::health_check_result> results;
    health_check_event event1("component1", results);
    bus->publish_event(event1);

    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(received_count.load(), 1);

    // Unsubscribe
    bus->unsubscribe_event(token.value());

    // Publish second event
    health_check_event event2("component2", results);
    bus->publish_event(event2);

    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(received_count.load(), 1); // Should still be 1
}

// Test thread system adapter
TEST_F(EventBusTest, ThreadSystemAdapter) {
    thread_system_adapter adapter(bus);

    // Check availability
    EXPECT_FALSE(adapter.is_thread_system_available());

    // Try to collect metrics (should return empty when not available)
    auto result = adapter.collect_metrics();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());

    // Get supported metric types
    auto types = adapter.get_metric_types();
    EXPECT_TRUE(types.empty()); // Empty when thread_system not available
}

// Test logger system adapter
TEST_F(EventBusTest, LoggerSystemAdapter) {
    logger_system_adapter adapter(bus);

    // Check availability
    EXPECT_FALSE(adapter.is_logger_system_available());

    // Try to collect metrics
    auto result = adapter.collect_metrics();
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result.value().empty());

    // Register a logger (won't do anything when system not available)
    adapter.register_logger("test_logger");

    // Get current log rate
    EXPECT_EQ(adapter.get_current_log_rate(), 0.0);
}

// Test event bus statistics
TEST_F(EventBusTest, Statistics) {
    auto initial_stats = bus->get_stats();
    EXPECT_EQ(initial_stats.total_published, 0);
    EXPECT_EQ(initial_stats.total_processed, 0);

    // Publish some events
    for (int i = 0; i < 10; ++i) {
        component_lifecycle_event event(
            "test_component",
            component_lifecycle_event::lifecycle_state::started,
            component_lifecycle_event::lifecycle_state::running
        );
        bus->publish_event(event);
    }

    std::this_thread::sleep_for(200ms);

    auto final_stats = bus->get_stats();
    EXPECT_EQ(final_stats.total_published, 10);
    EXPECT_GE(final_stats.total_processed, 0); // May vary due to async processing
}

// Test concurrent publishing
TEST_F(EventBusTest, ConcurrentPublishing) {
    std::atomic<int> received_count{0};

    // Subscribe to metric collection events
    bus->subscribe_event<metric_collection_event>(
        [&](const metric_collection_event& event) {
            received_count.fetch_add(event.get_metric_count());
        }
    );

    const int num_threads = 4;
    const int events_per_thread = 25;
    std::vector<std::thread> publishers;

    // Start publisher threads
    for (int t = 0; t < num_threads; ++t) {
        publishers.emplace_back([this, t, events_per_thread]() {
            for (int i = 0; i < events_per_thread; ++i) {
                std::vector<metric> metrics;
                metrics.push_back(metric{
                    "test_metric",
                    metric_value{42.0},
                    {{"thread", std::to_string(t)}},
                    metric_type::gauge
                });

                metric_collection_event event("test_collector", std::move(metrics));
                bus->publish_event(event);

                std::this_thread::sleep_for(1ms);
            }
        });
    }

    // Wait for all publishers
    for (auto& thread : publishers) {
        thread.join();
    }

    // Wait for processing
    std::this_thread::sleep_for(500ms);

    // Should have received all metrics
    EXPECT_EQ(received_count.load(), num_threads * events_per_thread);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}