#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message_types.h>
#include <kcenon/messaging/integrations/system_integrator.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <random>
#include <iomanip>
#include <iostream>
#include <functional>
#include <array>

using namespace kcenon::messaging::core;
using namespace kcenon::messaging::integrations;

namespace {
    constexpr std::chrono::milliseconds kPerformanceWaitTimeout{10000};
    constexpr std::chrono::milliseconds kPerformancePollInterval{1};

    bool wait_for_condition(const std::function<bool()>& condition,
                            std::chrono::milliseconds timeout = kPerformanceWaitTimeout) {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (condition()) {
                return true;
            }
            std::this_thread::sleep_for(kPerformancePollInterval);
        }
        return condition();
    }
}

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.worker_threads = 8;
        config_.max_queue_size = 50000;
        config_.enable_priority_queue = true;
        config_.enable_metrics = true;

        message_bus_ = std::make_unique<message_bus>(config_);
        ASSERT_TRUE(message_bus_->initialize());
    }

    void TearDown() override {
        if (message_bus_) {
            message_bus_->shutdown();
        }
    }

    message_bus_config config_;
    std::unique_ptr<message_bus> message_bus_;
};

TEST_F(PerformanceTest, MessageThroughputBenchmark) {
    constexpr int total_messages = 100000;
    std::atomic<int> processed_count{0};

    // Subscribe to test topic
    message_bus_->subscribe("performance.throughput", [&](const message& msg) {
        processed_count++;
    });

    auto start_time = std::chrono::high_resolution_clock::now();

    // Publish messages as fast as possible
    for (int i = 0; i < total_messages; ++i) {
        message_payload payload;
        payload.topic = "performance.throughput";
        payload.data["sequence"] = int64_t(i);
        payload.data["data"] = std::string("performance_test_data");

        message_bus_->publish("performance.throughput", payload);
    }

    auto publish_end_time = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(wait_for_condition([&] {
        return processed_count.load() >= total_messages;
    })) << "Timeout waiting for throughput benchmark messages to be processed.";

    auto process_end_time = std::chrono::high_resolution_clock::now();

    // Calculate metrics
    auto publish_duration = std::chrono::duration_cast<std::chrono::milliseconds>(publish_end_time - start_time);
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(process_end_time - start_time);

    double publish_rate = (total_messages * 1000.0) / publish_duration.count();
    double process_rate = (total_messages * 1000.0) / total_duration.count();

    std::cout << "Performance Results:\n";
    std::cout << "Total messages: " << total_messages << "\n";
    std::cout << "Publish time: " << publish_duration.count() << " ms\n";
    std::cout << "Total processing time: " << total_duration.count() << " ms\n";
    std::cout << "Publish rate: " << static_cast<int>(publish_rate) << " msg/sec\n";
    std::cout << "Processing rate: " << static_cast<int>(process_rate) << " msg/sec\n";

    // Performance assertions (adjust thresholds based on system capabilities)
    EXPECT_GE(publish_rate, 10000); // At least 10k messages/sec publish rate
    EXPECT_GE(process_rate, 5000);  // At least 5k messages/sec processing rate
    EXPECT_EQ(processed_count.load(), total_messages);
}

TEST_F(PerformanceTest, ConcurrentPublisherPerformance) {
    constexpr int num_publishers = 8;
    constexpr int messages_per_publisher = 5000;
    constexpr int total_messages = num_publishers * messages_per_publisher;

    std::atomic<int> total_processed{0};

    message_bus_->subscribe("performance.concurrent", [&](const message& msg) {
        total_processed++;
    });

    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch concurrent publishers
    std::vector<std::thread> publishers;
    for (int p = 0; p < num_publishers; ++p) {
        publishers.emplace_back([this, p, messages_per_publisher]() {
            for (int i = 0; i < messages_per_publisher; ++i) {
                message_payload payload;
                payload.topic = "performance.concurrent";
                payload.data["publisher_id"] = int64_t(p);
                payload.data["message_id"] = int64_t(i);
                payload.data["data"] = std::string("concurrent_test_data");

                message_bus_->publish("performance.concurrent", payload);
            }
        });
    }

    // Wait for all publishers to complete
    for (auto& thread : publishers) {
        thread.join();
    }

    auto publish_end_time = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(wait_for_condition([&] {
        return total_processed.load() >= total_messages;
    })) << "Timeout waiting for concurrent publish benchmark messages to be processed.";

    auto process_end_time = std::chrono::high_resolution_clock::now();

    auto publish_duration = std::chrono::duration_cast<std::chrono::milliseconds>(publish_end_time - start_time);
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(process_end_time - start_time);

    double concurrent_publish_rate = (total_messages * 1000.0) / publish_duration.count();
    double concurrent_process_rate = (total_messages * 1000.0) / total_duration.count();

    std::cout << "Concurrent Performance Results:\n";
    std::cout << "Publishers: " << num_publishers << "\n";
    std::cout << "Messages per publisher: " << messages_per_publisher << "\n";
    std::cout << "Total messages: " << total_messages << "\n";
    std::cout << "Concurrent publish rate: " << static_cast<int>(concurrent_publish_rate) << " msg/sec\n";
    std::cout << "Concurrent processing rate: " << static_cast<int>(concurrent_process_rate) << " msg/sec\n";

    EXPECT_GE(concurrent_publish_rate, 8000);
    EXPECT_GE(concurrent_process_rate, 4000);
    EXPECT_EQ(total_processed.load(), total_messages);
}

TEST_F(PerformanceTest, MessageSizeImpact) {
    constexpr int messages_per_size = 1000;

    std::vector<size_t> message_sizes = {64, 256, 1024, 4096, 16384}; // bytes

    for (size_t size : message_sizes) {
        std::atomic<int> processed{0};
        std::string topic = "performance.size." + std::to_string(size);

        message_bus_->subscribe(topic, [&](const message& msg) {
            processed++;
        });

        // Create data of specified size
        std::string large_data(size, 'X');

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < messages_per_size; ++i) {
            message_payload payload;
            payload.topic = topic;
            payload.data["large_data"] = large_data;
            payload.data["sequence"] = int64_t(i);

            message_bus_->publish(topic, payload);
        }

        ASSERT_TRUE(wait_for_condition([&] {
            return processed.load() >= messages_per_size;
        })) << "Timeout waiting for message size test (" << size << " bytes) to finish.";

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        double rate = (messages_per_size * 1000.0) / duration.count();
        double throughput_mb = (messages_per_size * size * 1000.0) / (duration.count() * 1024 * 1024);

        std::cout << "Message size " << size << " bytes: "
                  << static_cast<int>(rate) << " msg/sec, "
                  << std::fixed << std::setprecision(2) << throughput_mb << " MB/sec\n";

        EXPECT_GT(rate, 100); // At least 100 msg/sec for any size
    }
}

TEST_F(PerformanceTest, PriorityQueuePerformance) {
    constexpr int messages_per_priority = 2500;
    constexpr int total_messages = messages_per_priority * 4;
    std::vector<message_priority> received_priorities;
    std::mutex priorities_mutex;

    message_bus_->subscribe("performance.priority", [&](const message& msg) {
        std::lock_guard<std::mutex> lock(priorities_mutex);
        received_priorities.push_back(msg.metadata.priority);
    });

    auto start_time = std::chrono::high_resolution_clock::now();

    // Publish messages starting from lowest priority to highest to stress reordering
    const std::array<message_priority, 4> publish_order = {
        message_priority::low,
        message_priority::normal,
        message_priority::high,
        message_priority::critical
    };

    for (auto priority : publish_order) {
        for (int count = 0; count < messages_per_priority; ++count) {
            message msg;
            msg.payload.topic = "performance.priority";
            msg.payload.data["sequence"] = int64_t(priority) * messages_per_priority + count;
            msg.metadata.priority = priority;
            message_bus_->publish(msg);
        }
    }

    // Wait for all messages to be processed
    ASSERT_TRUE(wait_for_condition([&] {
        std::lock_guard<std::mutex> lock(priorities_mutex);
        return received_priorities.size() >= total_messages;
    })) << "Timeout waiting for priority queue benchmark messages to be processed.";

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double rate = (total_messages * 1000.0) / duration.count();

    std::cout << "Priority queue performance: " << static_cast<int>(rate) << " msg/sec\n";

    // Verify priority ordering is generally maintained
    std::lock_guard<std::mutex> lock(priorities_mutex);
    int priority_violations = 0;
    for (size_t i = 1; i < received_priorities.size(); ++i) {
        if (static_cast<int>(received_priorities[i]) > static_cast<int>(received_priorities[i-1])) {
            priority_violations++;
        }
    }

    double violation_rate = received_priorities.empty()
        ? 0.0
        : (priority_violations * 100.0) / received_priorities.size();
    std::cout << "Priority violation rate: " << std::fixed << std::setprecision(2) << violation_rate << "%\n";

    EXPECT_GT(rate, 1000); // Keep baseline throughput requirement modest for multi-platform stability
    EXPECT_LT(violation_rate, 25.0); // Allow small fraction of out-of-order deliveries on slower schedulers
}

TEST_F(PerformanceTest, MemoryUsageStability) {
    constexpr int iterations = 5;
    constexpr int messages_per_iteration = 10000;

    std::atomic<int> total_processed{0};

    message_bus_->subscribe("performance.memory", [&](const message& msg) {
        total_processed++;
    });

    // Run multiple iterations to check for memory leaks
    for (int iter = 0; iter < iterations; ++iter) {
        int iteration_processed = 0;

        auto start_stats = message_bus_->get_statistics();

        // Publish messages for this iteration
        for (int i = 0; i < messages_per_iteration; ++i) {
            message_payload payload;
            payload.topic = "performance.memory";
            payload.data["iteration"] = int64_t(iter);
            payload.data["sequence"] = int64_t(i);
            payload.data["data"] = std::string("memory_test_data_iteration_") + std::to_string(iter);

            message_bus_->publish("performance.memory", payload);
        }

        ASSERT_TRUE(wait_for_condition([&] {
            return total_processed.load() >= (iter + 1) * messages_per_iteration;
        })) << "Timeout during memory usage iteration " << iter + 1 << ".";

        auto end_stats = message_bus_->get_statistics();

        std::cout << "Iteration " << iter + 1 << ": "
                  << "Published: " << (end_stats.messages_published - start_stats.messages_published)
                  << ", Processed: " << (end_stats.messages_processed - start_stats.messages_processed) << "\n";

        // Give system time to cleanup
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_EQ(total_processed.load(), iterations * messages_per_iteration);

    auto final_stats = message_bus_->get_statistics();
    std::cout << "Final stats - Published: " << final_stats.messages_published
              << ", Processed: " << final_stats.messages_processed
              << ", Failed: " << final_stats.messages_failed << "\n";

    // No messages should fail in normal conditions
    EXPECT_EQ(final_stats.messages_failed, 0);
}

// Integration performance test
class SystemIntegratorPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto integrator = system_integrator::create_default();
        integrator_ = std::move(integrator);
        ASSERT_TRUE(integrator_->initialize());
    }

    void TearDown() override {
        if (integrator_) {
            integrator_->shutdown();
        }
    }

    std::unique_ptr<system_integrator> integrator_;
};

TEST_F(SystemIntegratorPerformanceTest, SystemIntegratorThroughput) {
    constexpr int total_messages = 50000;
    std::atomic<int> processed{0};

    integrator_->subscribe("system.performance", [&](const message& msg) {
        processed++;
    });

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < total_messages; ++i) {
        message_payload payload;
        payload.topic = "system.performance";
        payload.data["sequence"] = int64_t(i);
        payload.data["component"] = std::string("system_integrator");

        integrator_->publish("system.performance", payload, "performance_test");
    }

    ASSERT_TRUE(wait_for_condition([&] {
        return processed.load() >= total_messages;
    })) << "Timeout waiting for system integrator throughput test to complete.";

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double rate = (total_messages * 1000.0) / duration.count();

    std::cout << "System Integrator Performance: " << static_cast<int>(rate) << " msg/sec\n";

    EXPECT_GT(rate, 5000); // System integrator should maintain good performance
    EXPECT_EQ(processed.load(), total_messages);

    // Check system health after performance test
    auto health = integrator_->check_system_health();
    EXPECT_TRUE(health.message_bus_healthy);
    EXPECT_GE(health.total_messages_processed, total_messages);
}
