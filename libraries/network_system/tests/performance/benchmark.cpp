/**
 * @file benchmark.cpp
 * @brief Performance benchmarks for network_system
 *
 * Measures throughput, latency, and scalability of the network_system
 * implementation compared to legacy messaging_system behavior.
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <sstream>

#include "network_system/network_system.h"

using namespace network_system;
using namespace std::chrono_literals;

/**
 * @brief Benchmark result structure
 */
struct BenchmarkResult {
    std::string name;
    double throughput;  // messages per second
    double latency_avg;  // microseconds
    double latency_min;
    double latency_max;
    double latency_p50;
    double latency_p90;
    double latency_p99;
    size_t total_messages;
    std::chrono::milliseconds duration;

    void print() const {
        std::cout << "\n=== " << name << " ===" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Throughput: " << throughput << " msg/s" << std::endl;
        std::cout << "Total messages: " << total_messages << std::endl;
        std::cout << "Duration: " << duration.count() << " ms" << std::endl;
        std::cout << "\nLatency (μs):" << std::endl;
        std::cout << "  Average: " << latency_avg << std::endl;
        std::cout << "  Min: " << latency_min << std::endl;
        std::cout << "  Max: " << latency_max << std::endl;
        std::cout << "  P50: " << latency_p50 << std::endl;
        std::cout << "  P90: " << latency_p90 << std::endl;
        std::cout << "  P99: " << latency_p99 << std::endl;
    }
};

/**
 * @brief Calculate percentile from sorted vector
 */
double calculate_percentile(const std::vector<double>& sorted_data, double percentile) {
    if (sorted_data.empty()) return 0.0;

    size_t index = static_cast<size_t>((percentile / 100.0) * sorted_data.size());
    if (index >= sorted_data.size()) index = sorted_data.size() - 1;

    return sorted_data[index];
}

/**
 * @brief Benchmark message throughput
 */
BenchmarkResult benchmark_throughput(
    size_t num_messages,
    size_t message_size,
    uint16_t port
) {
    BenchmarkResult result;
    result.name = "Throughput Test (" + std::to_string(message_size) + " bytes)";
    result.total_messages = num_messages;

    std::atomic<size_t> messages_received{0};
    std::vector<double> latencies;
    latencies.reserve(num_messages);

    // Create server
    auto server = std::make_shared<core::messaging_server>("benchmark_server");
    server->start_server(port);

    // Wait for server to start
    std::this_thread::sleep_for(100ms);

    // Create client
    auto client = std::make_shared<core::messaging_client>("benchmark_client");
    client->start_client("127.0.0.1", port);

    // Wait for connection
    std::this_thread::sleep_for(100ms);

    // Create test message
    std::vector<uint8_t> message(message_size);
    std::fill(message.begin(), message.end(), 'X');

    // Start benchmark
    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < num_messages; ++i) {
        auto msg_start = std::chrono::high_resolution_clock::now();

        client->send_packet(message);

        auto msg_end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration<double, std::micro>(msg_end - msg_start).count();
        latencies.push_back(latency);

        messages_received++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Calculate metrics
    result.throughput = (static_cast<double>(num_messages) / result.duration.count()) * 1000.0;

    // Sort latencies for percentile calculation
    std::sort(latencies.begin(), latencies.end());

    if (!latencies.empty()) {
        result.latency_avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
        result.latency_min = latencies.front();
        result.latency_max = latencies.back();
        result.latency_p50 = calculate_percentile(latencies, 50);
        result.latency_p90 = calculate_percentile(latencies, 90);
        result.latency_p99 = calculate_percentile(latencies, 99);
    }

    // Cleanup
    client->stop_client();
    server->stop_server();

    return result;
}

/**
 * @brief Benchmark concurrent connections
 */
BenchmarkResult benchmark_concurrent_connections(
    size_t num_clients,
    size_t messages_per_client,
    uint16_t port
) {
    BenchmarkResult result;
    result.name = "Concurrent Connections (" + std::to_string(num_clients) + " clients)";
    result.total_messages = num_clients * messages_per_client;

    std::atomic<size_t> total_messages_sent{0};
    std::vector<double> all_latencies;
    std::mutex latency_mutex;

    // Create server
    auto server = std::make_shared<core::messaging_server>("concurrent_server");
    server->start_server(port);

    // Wait for server
    std::this_thread::sleep_for(200ms);

    auto start = std::chrono::high_resolution_clock::now();

    // Create and run clients in threads
    std::vector<std::thread> client_threads;

    for (size_t i = 0; i < num_clients; ++i) {
        client_threads.emplace_back([i, messages_per_client, port, &total_messages_sent, &all_latencies, &latency_mutex]() {
            auto client = std::make_shared<core::messaging_client>("client_" + std::to_string(i));
            client->start_client("127.0.0.1", port);

            // Wait for connection
            std::this_thread::sleep_for(50ms);

            std::vector<uint8_t> message(256);
            std::fill(message.begin(), message.end(), static_cast<uint8_t>(i % 256));

            std::vector<double> local_latencies;

            for (size_t j = 0; j < messages_per_client; ++j) {
                auto msg_start = std::chrono::high_resolution_clock::now();
                client->send_packet(message);
                auto msg_end = std::chrono::high_resolution_clock::now();

                auto latency = std::chrono::duration<double, std::micro>(msg_end - msg_start).count();
                local_latencies.push_back(latency);

                total_messages_sent++;
                std::this_thread::sleep_for(1ms);  // Small delay between messages
            }

            // Add local latencies to global list
            {
                std::lock_guard<std::mutex> lock(latency_mutex);
                all_latencies.insert(all_latencies.end(), local_latencies.begin(), local_latencies.end());
            }

            client->stop_client();
        });
    }

    // Wait for all clients to finish
    for (auto& thread : client_threads) {
        thread.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Calculate metrics
    result.throughput = (static_cast<double>(total_messages_sent) / result.duration.count()) * 1000.0;

    // Sort latencies for percentile calculation
    std::sort(all_latencies.begin(), all_latencies.end());

    if (!all_latencies.empty()) {
        result.latency_avg = std::accumulate(all_latencies.begin(), all_latencies.end(), 0.0) / all_latencies.size();
        result.latency_min = all_latencies.front();
        result.latency_max = all_latencies.back();
        result.latency_p50 = calculate_percentile(all_latencies, 50);
        result.latency_p90 = calculate_percentile(all_latencies, 90);
        result.latency_p99 = calculate_percentile(all_latencies, 99);
    }

    // Cleanup
    server->stop_server();

    return result;
}

/**
 * @brief Benchmark thread pool performance
 */
BenchmarkResult benchmark_thread_pool(size_t num_tasks) {
    BenchmarkResult result;
    result.name = "Thread Pool Performance (" + std::to_string(num_tasks) + " tasks)";
    result.total_messages = num_tasks;

    auto& thread_mgr = integration::thread_integration_manager::instance();
    std::vector<std::future<void>> futures;
    std::atomic<size_t> completed_tasks{0};
    std::vector<double> task_latencies;
    std::mutex latency_mutex;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < num_tasks; ++i) {
        auto task_start = std::chrono::high_resolution_clock::now();

        futures.push_back(thread_mgr.submit_task([&completed_tasks, task_start, &task_latencies, &latency_mutex]() {
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::microseconds(10));

            auto task_end = std::chrono::high_resolution_clock::now();
            auto latency = std::chrono::duration<double, std::micro>(task_end - task_start).count();

            {
                std::lock_guard<std::mutex> lock(latency_mutex);
                task_latencies.push_back(latency);
            }

            completed_tasks++;
        }));
    }

    // Wait for all tasks
    for (auto& future : futures) {
        future.wait();
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Calculate metrics
    result.throughput = (static_cast<double>(num_tasks) / result.duration.count()) * 1000.0;

    // Sort latencies
    std::sort(task_latencies.begin(), task_latencies.end());

    if (!task_latencies.empty()) {
        result.latency_avg = std::accumulate(task_latencies.begin(), task_latencies.end(), 0.0) / task_latencies.size();
        result.latency_min = task_latencies.front();
        result.latency_max = task_latencies.back();
        result.latency_p50 = calculate_percentile(task_latencies, 50);
        result.latency_p90 = calculate_percentile(task_latencies, 90);
        result.latency_p99 = calculate_percentile(task_latencies, 99);
    }

    return result;
}

/**
 * @brief Benchmark container serialization
 */
BenchmarkResult benchmark_container_serialization(size_t num_operations) {
    BenchmarkResult result;
    result.name = "Container Serialization (" + std::to_string(num_operations) + " operations)";
    result.total_messages = num_operations;

    auto& container_mgr = integration::container_manager::instance();
    std::vector<double> latencies;

    // Test data
    std::string test_data = "This is a test message for container serialization benchmark";

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < num_operations; ++i) {
        auto op_start = std::chrono::high_resolution_clock::now();

        // Serialize
        auto serialized = container_mgr.serialize(std::any(test_data));

        // Deserialize
        auto deserialized = container_mgr.deserialize(serialized);

        auto op_end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration<double, std::micro>(op_end - op_start).count();
        latencies.push_back(latency);
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Calculate metrics
    result.throughput = (static_cast<double>(num_operations) / result.duration.count()) * 1000.0;

    // Sort latencies
    std::sort(latencies.begin(), latencies.end());

    if (!latencies.empty()) {
        result.latency_avg = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
        result.latency_min = latencies.front();
        result.latency_max = latencies.back();
        result.latency_p50 = calculate_percentile(latencies, 50);
        result.latency_p90 = calculate_percentile(latencies, 90);
        result.latency_p99 = calculate_percentile(latencies, 99);
    }

    return result;
}

/**
 * @brief Main benchmark runner
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Network System Performance Benchmarks ===" << std::endl;
    std::cout << "Benchmark Suite: Network Performance Analysis | Standards: C++20 Async I/O" << std::endl;
    std::cout << "CPU Threads: " << std::thread::hardware_concurrency() << std::endl;

    // Initialize system
    network_system::compat::initialize();
    std::cout << "\nNetwork system initialized" << std::endl;

    std::vector<BenchmarkResult> results;

    // Run benchmarks
    std::cout << "\n🚀 Starting benchmarks..." << std::endl;

    // Throughput benchmarks with different message sizes
    std::cout << "\n[1/5] Running throughput benchmarks..." << std::endl;
    results.push_back(benchmark_throughput(10000, 64, 8081));    // Small messages
    results.push_back(benchmark_throughput(5000, 1024, 8082));   // Medium messages
    results.push_back(benchmark_throughput(1000, 8192, 8083));   // Large messages

    // Concurrent connections benchmark
    std::cout << "\n[2/5] Running concurrent connections benchmark..." << std::endl;
    results.push_back(benchmark_concurrent_connections(10, 100, 8084));   // 10 clients
    results.push_back(benchmark_concurrent_connections(50, 20, 8085));    // 50 clients

    // Thread pool benchmark
    std::cout << "\n[3/5] Running thread pool benchmark..." << std::endl;
    results.push_back(benchmark_thread_pool(1000));   // 1000 tasks
    results.push_back(benchmark_thread_pool(10000));  // 10000 tasks

    // Container serialization benchmark
    std::cout << "\n[4/5] Running container serialization benchmark..." << std::endl;
    results.push_back(benchmark_container_serialization(10000));   // 10000 operations
    results.push_back(benchmark_container_serialization(100000));  // 100000 operations

    // Print all results
    std::cout << "\n\n📊 BENCHMARK RESULTS" << std::endl;
    std::cout << "===========================================" << std::endl;

    for (const auto& result : results) {
        result.print();
    }

    // Summary statistics
    std::cout << "\n\n📈 SUMMARY STATISTICS" << std::endl;
    std::cout << "===========================================" << std::endl;

    double total_throughput = 0;
    double avg_latency = 0;
    size_t total_messages = 0;

    for (const auto& result : results) {
        total_throughput += result.throughput;
        avg_latency += result.latency_avg;
        total_messages += result.total_messages;
    }

    std::cout << "Total messages processed: " << total_messages << std::endl;
    std::cout << "Average throughput: " << std::fixed << std::setprecision(2)
              << (total_throughput / results.size()) << " msg/s" << std::endl;
    std::cout << "Average latency: " << (avg_latency / results.size()) << " μs" << std::endl;

    // Performance rating
    std::cout << "\n⭐ PERFORMANCE RATING" << std::endl;
    std::cout << "===========================================" << std::endl;

    double avg_throughput = total_throughput / results.size();
    if (avg_throughput > 10000) {
        std::cout << "🏆 EXCELLENT - Production ready!" << std::endl;
    } else if (avg_throughput > 5000) {
        std::cout << "✅ GOOD - Suitable for most applications" << std::endl;
    } else if (avg_throughput > 1000) {
        std::cout << "⚠️  FAIR - May need optimization for high-load scenarios" << std::endl;
    } else {
        std::cout << "❌ POOR - Requires performance improvements" << std::endl;
    }

    // Cleanup
    network_system::compat::shutdown();
    std::cout << "\nNetwork system shutdown complete" << std::endl;

    return 0;
}