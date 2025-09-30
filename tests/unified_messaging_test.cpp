/**
 * @file unified_messaging_test.cpp
 * @brief Unit tests for the unified messaging system
 * @author kcenon
 * @date 2025
 *
 * Tests the integration of all 7 system modules through the unified interface
 */

#include <kcenon/messaging/unified_messaging_system.h>
#include <gtest/gtest.h>
#include <future>
#include <atomic>
#include <chrono>
#include <thread>
#include <random>

using namespace kcenon::messaging;
using namespace std::chrono_literals;

class UnifiedMessagingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Default config for tests
        config_.name = "TestSystem";
        config_.worker_threads = 2;
        config_.max_queue_size = 1000;
        config_.enable_console_logging = false;  // Disable logging noise in tests
        config_.enable_monitoring = true;
        config_.enable_metrics_collection = true;
    }

    void TearDown() override {
        // Cleanup
    }

    messaging_config config_;

    // Helper to create test messages
    message create_test_message(const std::string& topic = "test/topic",
                               const std::string& content = "test content",
                               message_priority priority = message_priority::normal) {
        message msg;
        msg.id = "test_" + std::to_string(rand());
        msg.type = message_type::notification;
        msg.priority = priority;
        msg.sender = "test";
        msg.recipient = "test";
        msg.topic = topic;
        msg.payload.assign(content.begin(), content.end());
        msg.timestamp = std::chrono::system_clock::now();
        return msg;
    }
};

// ============= Basic Functionality Tests =============

TEST_F(UnifiedMessagingTest, InitializationTest) {
    unified_messaging_system system(config_);
    auto result = system.initialize();
    EXPECT_TRUE(result.has_value() || result.is_success());
    EXPECT_TRUE(system.is_running());

    auto shutdown_result = system.shutdown();
    EXPECT_TRUE(shutdown_result.has_value() || shutdown_result.is_success());
    EXPECT_FALSE(system.is_running());
}

TEST_F(UnifiedMessagingTest, DefaultConstructorTest) {
    unified_messaging_system system;
    auto result = system.initialize();
    EXPECT_TRUE(result.has_value() || result.is_success());
    EXPECT_TRUE(system.is_running());
}

TEST_F(UnifiedMessagingTest, SendMessageTest) {
    unified_messaging_system system(config_);
    system.initialize();

    auto msg = create_test_message();
    auto future = system.send(msg);
    auto result = future.get();

    EXPECT_TRUE(result.has_value() || result.is_success());
}

TEST_F(UnifiedMessagingTest, SubscriptionTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> received_count(0);
    std::promise<void> received_promise;
    auto received_future = received_promise.get_future();

    // Subscribe to topic
    auto sub_result = system.subscribe("test/topic", [&](const message& msg) {
        received_count++;
        if (received_count == 1) {
            received_promise.set_value();
        }
    });

    EXPECT_TRUE(sub_result.has_value());
    std::string sub_id = sub_result.value();

    // Send message
    auto msg = create_test_message("test/topic", "Hello");
    system.send(msg);

    // Wait for reception
    ASSERT_EQ(received_future.wait_for(1s), std::future_status::ready);
    EXPECT_EQ(received_count.load(), 1);

    // Unsubscribe
    auto unsub_result = system.unsubscribe(sub_id);
    EXPECT_TRUE(unsub_result.has_value() || unsub_result.is_success());
}

TEST_F(UnifiedMessagingTest, WildcardSubscriptionTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> received_count(0);

    // Subscribe with wildcard
    system.on_message("test/*", [&](const message& msg) {
        received_count++;
    });

    // Send messages to different topics
    system.send(create_test_message("test/one", "Message 1"));
    system.send(create_test_message("test/two", "Message 2"));
    system.send(create_test_message("other/topic", "Message 3"));  // Should not match

    // Wait for processing
    std::this_thread::sleep_for(100ms);
    system.wait_for_completion();

    EXPECT_EQ(received_count.load(), 2);
}

// ============= Priority Queue Tests =============

TEST_F(UnifiedMessagingTest, PriorityQueueTest) {
    config_.use_priority_queue = true;
    unified_messaging_system system(config_);
    system.initialize();

    std::vector<message_priority> received_priorities;
    std::mutex mutex;

    system.on_message("priority/*", [&](const message& msg) {
        std::lock_guard<std::mutex> lock(mutex);
        received_priorities.push_back(msg.priority);
    });

    // Send messages with different priorities
    system.send(create_test_message("priority/test", "Low", message_priority::low));
    system.send(create_test_message("priority/test", "Critical", message_priority::critical));
    system.send(create_test_message("priority/test", "Normal", message_priority::normal));
    system.send(create_test_message("priority/test", "High", message_priority::high));

    // Wait for processing
    std::this_thread::sleep_for(200ms);
    system.wait_for_completion();

    // Check that higher priority messages were processed first
    ASSERT_EQ(received_priorities.size(), 4);
    // Critical should be processed early
    auto critical_pos = std::find(received_priorities.begin(), received_priorities.end(),
                                 message_priority::critical);
    auto low_pos = std::find(received_priorities.begin(), received_priorities.end(),
                            message_priority::low);

    if (critical_pos != received_priorities.end() && low_pos != received_priorities.end()) {
        EXPECT_LT(std::distance(received_priorities.begin(), critical_pos),
                 std::distance(received_priorities.begin(), low_pos));
    }
}

// ============= Batch Processing Tests =============

TEST_F(UnifiedMessagingTest, BatchSendTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> received_count(0);

    system.on_message("batch/*", [&](const message& msg) {
        received_count++;
    });

    // Create batch
    std::vector<message> batch;
    for (int i = 0; i < 10; ++i) {
        batch.push_back(create_test_message("batch/test", "Message " + std::to_string(i)));
    }

    // Send batch
    auto future = system.send_batch(batch);
    auto result = future.get();
    EXPECT_TRUE(result.has_value() || result.is_success());

    // Wait for processing
    std::this_thread::sleep_for(200ms);
    system.wait_for_completion();

    EXPECT_EQ(received_count.load(), 10);
}

TEST_F(UnifiedMessagingTest, ParallelProcessingTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::vector<message> messages;
    for (int i = 0; i < 5; ++i) {
        messages.push_back(create_test_message("parallel/test", std::to_string(i)));
    }

    auto futures = system.process_parallel(messages, [](const message& msg) {
        // Simulate work
        std::this_thread::sleep_for(10ms);
        return static_cast<int>(msg.payload.size());
    });

    int total_size = 0;
    for (auto& future : futures) {
        total_size += future.get();
    }

    EXPECT_GT(total_size, 0);
}

// ============= Filtering and Transformation Tests =============

TEST_F(UnifiedMessagingTest, MessageFilterTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> received_count(0);

    // Set filter - only high priority messages
    system.set_message_filter([](const message& msg) {
        return msg.priority >= message_priority::high;
    });

    system.on_message("filter/*", [&](const message& msg) {
        received_count++;
    });

    // Send messages with different priorities
    system.send(create_test_message("filter/test", "Low", message_priority::low));
    system.send(create_test_message("filter/test", "Normal", message_priority::normal));
    system.send(create_test_message("filter/test", "High", message_priority::high));
    system.send(create_test_message("filter/test", "Critical", message_priority::critical));

    // Wait for processing
    std::this_thread::sleep_for(100ms);
    system.wait_for_completion();

    // Only high and critical should pass
    EXPECT_EQ(received_count.load(), 2);
}

TEST_F(UnifiedMessagingTest, MessageTransformerTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::string received_content;

    // Set transformer - add prefix to payload
    system.set_message_transformer([](const message& msg) {
        message transformed = msg;
        std::string prefix = "[TRANSFORMED] ";
        transformed.payload.insert(transformed.payload.begin(), prefix.begin(), prefix.end());
        return transformed;
    });

    system.on_message("transform/*", [&](const message& msg) {
        received_content = std::string(msg.payload.begin(), msg.payload.end());
    });

    system.send(create_test_message("transform/test", "Original"));

    // Wait for processing
    std::this_thread::sleep_for(100ms);
    system.wait_for_completion();

    EXPECT_TRUE(received_content.find("[TRANSFORMED]") != std::string::npos);
    EXPECT_TRUE(received_content.find("Original") != std::string::npos);
}

// ============= Metrics and Monitoring Tests =============

TEST_F(UnifiedMessagingTest, MetricsCollectionTest) {
    config_.enable_metrics_collection = true;
    unified_messaging_system system(config_);
    system.initialize();

    // Send some messages
    for (int i = 0; i < 5; ++i) {
        system.send(create_test_message());
    }

    // Wait for processing
    std::this_thread::sleep_for(100ms);

    auto metrics = system.get_metrics();
    EXPECT_GE(metrics.messages_sent, 5);

    // Reset metrics
    system.reset_metrics();
    auto reset_metrics = system.get_metrics();
    EXPECT_EQ(reset_metrics.messages_sent, 0);
}

TEST_F(UnifiedMessagingTest, HealthCheckTest) {
    unified_messaging_system system(config_);
    system.initialize();

    auto health = system.get_health();
    EXPECT_TRUE(health.is_healthy);
    EXPECT_GT(health.overall_health_score, 0.0);
    EXPECT_LE(health.overall_health_score, 100.0);
}

TEST_F(UnifiedMessagingTest, MetricsToggleTest) {
    unified_messaging_system system(config_);
    system.initialize();

    // Disable metrics
    system.set_metrics_enabled(false);

    system.send(create_test_message());
    auto metrics1 = system.get_metrics();

    system.send(create_test_message());
    auto metrics2 = system.get_metrics();

    // Metrics should not change when disabled
    EXPECT_EQ(metrics1.messages_sent, metrics2.messages_sent);

    // Re-enable metrics
    system.set_metrics_enabled(true);
    system.send(create_test_message());
    auto metrics3 = system.get_metrics();

    // Now metrics should update
    EXPECT_GT(metrics3.messages_sent, metrics2.messages_sent);
}

// ============= Queue Management Tests =============

TEST_F(UnifiedMessagingTest, QueueSizeTest) {
    config_.max_queue_size = 10;
    unified_messaging_system system(config_);
    system.initialize();

    // Block processing with slow handler
    std::atomic<bool> process_messages(false);

    system.on_message("queue/*", [&](const message& msg) {
        while (!process_messages.load()) {
            std::this_thread::sleep_for(10ms);
        }
    });

    // Fill queue
    for (int i = 0; i < 5; ++i) {
        system.send(create_test_message("queue/test"));
    }

    // Check queue size
    std::this_thread::sleep_for(50ms);
    auto size = system.get_queue_size();
    EXPECT_GT(size, 0);
    EXPECT_LE(size, config_.max_queue_size);

    // Allow processing
    process_messages = true;
    system.wait_for_completion();

    // Queue should be empty now
    size = system.get_queue_size();
    EXPECT_EQ(size, 0);
}

// ============= Network Tests (if available) =============

TEST_F(UnifiedMessagingTest, ServerStartStopTest) {
    unified_messaging_system system(config_);
    system.initialize();

    // Try to start server
    auto start_result = system.start_server(9999, "127.0.0.1");

#ifdef HAS_NETWORK_SYSTEM
    if (start_result.has_value() || start_result.is_success()) {
        EXPECT_TRUE(system.is_server_running());

        auto stop_result = system.stop_server();
        EXPECT_TRUE(stop_result.has_value() || stop_result.is_success());
        EXPECT_FALSE(system.is_server_running());
    }
#else
    // Network system not available - should return error
    EXPECT_FALSE(start_result.has_value() || start_result.is_success());
#endif
}

TEST_F(UnifiedMessagingTest, ClientConnectionTest) {
    unified_messaging_system system(config_);
    system.initialize();

    connection_info conn_info;
    conn_info.address = "127.0.0.1";
    conn_info.port = 8888;

    auto connect_result = system.connect(conn_info);

#ifdef HAS_NETWORK_SYSTEM
    // Connection might fail if no server is running, but should not crash
    auto status = system.get_connection_status();
    EXPECT_TRUE(status == connection_status::connected ||
               status == connection_status::error ||
               status == connection_status::disconnected);

    system.disconnect();
    status = system.get_connection_status();
    EXPECT_EQ(status, connection_status::disconnected);
#else
    // Network system not available
    EXPECT_FALSE(connect_result.has_value() || connect_result.is_success());
#endif
}

// ============= Persistence Tests (if available) =============

TEST_F(UnifiedMessagingTest, PersistenceTest) {
    config_.enable_persistence = true;
    config_.db_connection_string = "sqlite://test.db";

    unified_messaging_system system(config_);
    system.initialize();

    auto msg = create_test_message("persist/test", "Persistent message");
    auto result = system.persist_message(msg);

#ifdef HAS_DATABASE_SYSTEM
    // If database is configured, should succeed
    if (result.has_value() || result.is_success()) {
        // Try to query
        auto query_result = system.query_messages("topic='persist/test'", 10);
        // Query might not be fully implemented
    }
#else
    // Database system not available
    EXPECT_FALSE(result.has_value() || result.is_success());
#endif
}

// ============= Concurrency Tests =============

TEST_F(UnifiedMessagingTest, ConcurrentSendTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> received_count(0);
    const int num_threads = 10;
    const int messages_per_thread = 100;

    system.on_message("concurrent/*", [&](const message& msg) {
        received_count++;
    });

    // Launch multiple threads sending messages
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                auto msg = create_test_message(
                    "concurrent/thread" + std::to_string(t),
                    "Message " + std::to_string(i)
                );
                system.send(msg);
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Wait for processing
    system.wait_for_completion();

    EXPECT_EQ(received_count.load(), num_threads * messages_per_thread);
}

TEST_F(UnifiedMessagingTest, ConcurrentSubscriptionTest) {
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> total_received(0);
    std::vector<std::string> subscription_ids;
    std::mutex mutex;

    // Multiple concurrent subscriptions
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, i]() {
            auto result = system.subscribe("multi/" + std::to_string(i), [&](const message& msg) {
                total_received++;
            });

            if (result.has_value()) {
                std::lock_guard<std::mutex> lock(mutex);
                subscription_ids.push_back(result.value());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Send messages to all topics
    for (int i = 0; i < 5; ++i) {
        system.send(create_test_message("multi/" + std::to_string(i)));
    }

    // Wait for processing
    std::this_thread::sleep_for(100ms);
    system.wait_for_completion();

    EXPECT_EQ(total_received.load(), 5);

    // Cleanup subscriptions
    for (const auto& id : subscription_ids) {
        system.unsubscribe(id);
    }
}

// ============= Stress Tests =============

TEST_F(UnifiedMessagingTest, StressTest) {
    config_.worker_threads = 4;
    config_.max_queue_size = 10000;
    unified_messaging_system system(config_);
    system.initialize();

    std::atomic<int> received_count(0);
    const int total_messages = 1000;

    system.on_message("stress/*", [&](const message& msg) {
        received_count++;
    });

    // Send many messages quickly
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < total_messages; ++i) {
        system.send(create_test_message("stress/test", std::to_string(i)));
    }

    // Wait for all to be processed
    system.wait_for_completion();

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_EQ(received_count.load(), total_messages);

    // Calculate throughput
    double throughput = (total_messages * 1000.0) / duration.count();
    std::cout << "Stress test throughput: " << throughput << " messages/second" << std::endl;
}

// ============= Edge Cases =============

TEST_F(UnifiedMessagingTest, EmptyMessageTest) {
    unified_messaging_system system(config_);
    system.initialize();

    message empty_msg;
    auto future = system.send(empty_msg);
    auto result = future.get();

    // Should handle empty message gracefully
    EXPECT_TRUE(result.has_value() || result.is_success());
}

TEST_F(UnifiedMessagingTest, LargePayloadTest) {
    unified_messaging_system system(config_);
    system.initialize();

    // Create message with large payload
    std::string large_content(1024 * 1024, 'X');  // 1MB
    auto msg = create_test_message("large/test", large_content);

    bool received = false;
    system.on_message("large/*", [&](const message& received_msg) {
        received = true;
        EXPECT_EQ(received_msg.payload.size(), large_content.size());
    });

    system.send(msg);
    system.wait_for_completion();

    EXPECT_TRUE(received);
}

TEST_F(UnifiedMessagingTest, InvalidSubscriptionIdTest) {
    unified_messaging_system system(config_);
    system.initialize();

    auto result = system.unsubscribe("invalid_subscription_id");
    EXPECT_FALSE(result.has_value() || result.is_success());
}

// Main function for running tests
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}