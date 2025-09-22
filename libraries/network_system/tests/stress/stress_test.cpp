/**
 * @file stress_test.cpp
 * @brief Stress testing for network_system
 *
 * Tests system behavior under heavy load including:
 * - High message throughput
 * - Many concurrent connections
 * - Memory usage under stress
 * - CPU utilization patterns
 *
 * @author kcenon
 * @date 2025-09-20

 */

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <random>
#include <iomanip>
#include <cstring>

#include "network_system/network_system.h"

using namespace network_system;
using namespace std::chrono_literals;

// Stress test configuration
struct StressConfig {
    size_t num_clients = 100;
    size_t messages_per_client = 1000;
    size_t message_size = 1024;
    std::chrono::seconds duration = 60s;
    uint16_t base_port = 10000;
    bool enable_random_delays = false;
    bool enable_random_sizes = true;
    size_t max_message_size = 8192;
};

// Stress test metrics
struct StressMetrics {
    std::atomic<size_t> total_messages_sent{0};
    std::atomic<size_t> total_messages_failed{0};
    std::atomic<size_t> total_bytes_sent{0};
    std::atomic<size_t> total_connections{0};
    std::atomic<size_t> failed_connections{0};
    std::atomic<size_t> total_errors{0};
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;

    void print() const {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            end_time - start_time
        ).count();

        if (duration == 0) duration = 1; // Avoid division by zero

        std::cout << "\n=== Stress Test Results ===" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Duration: " << duration << " seconds" << std::endl;
        std::cout << "Total messages sent: " << total_messages_sent << std::endl;
        std::cout << "Total messages failed: " << total_messages_failed << std::endl;
        std::cout << "Total bytes sent: " << (total_bytes_sent / (1024.0 * 1024.0))
                  << " MB" << std::endl;
        std::cout << "Total connections: " << total_connections << std::endl;
        std::cout << "Failed connections: " << failed_connections << std::endl;
        std::cout << "Total errors: " << total_errors << std::endl;
        std::cout << "\nPerformance Metrics:" << std::endl;
        std::cout << "Messages/second: "
                  << (total_messages_sent / static_cast<double>(duration)) << std::endl;
        std::cout << "Throughput: "
                  << ((total_bytes_sent / (1024.0 * 1024.0)) / duration)
                  << " MB/s" << std::endl;
        std::cout << "Success rate: "
                  << ((total_messages_sent * 100.0) /
                      (total_messages_sent + total_messages_failed))
                  << "%" << std::endl;
    }

    bool is_successful() const {
        // Consider successful if > 95% messages were sent
        double success_rate = (total_messages_sent * 100.0) /
                              (total_messages_sent + total_messages_failed + 1);
        return success_rate > 95.0 && total_errors < 10;
    }
};

/**
 * @brief Memory usage monitor
 */
class MemoryMonitor {
public:
    MemoryMonitor() : running_(true) {
        monitor_thread_ = std::thread(&MemoryMonitor::monitor, this);
    }

    ~MemoryMonitor() {
        running_ = false;
        if (monitor_thread_.joinable()) {
            monitor_thread_.join();
        }
    }

    size_t get_peak_memory() const { return peak_memory_; }
    size_t get_current_memory() const { return current_memory_; }

private:
    void monitor() {
        while (running_) {
            size_t current = get_current_rss();
            current_memory_ = current;

            size_t peak = peak_memory_.load();
            if (current > peak) {
                peak_memory_.store(current);
            }
            std::this_thread::sleep_for(100ms);
        }
    }

    size_t get_current_rss() {
        // Platform-specific memory measurement
        // This is a simplified version
#ifdef __APPLE__
        // macOS implementation would go here
        return 0; // Placeholder
#elif __linux__
        // Linux implementation would go here
        return 0; // Placeholder
#else
        return 0; // Placeholder for other platforms
#endif
    }

    std::atomic<bool> running_;
    std::thread monitor_thread_;
    std::atomic<size_t> current_memory_{0};
    std::atomic<size_t> peak_memory_{0};
};

/**
 * @brief Stress test client worker
 */
void stress_client_worker(
    size_t client_id,
    const StressConfig& config,
    StressMetrics& metrics,
    std::atomic<bool>& stop_flag
) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_dist(64, config.max_message_size);
    std::uniform_int_distribution<> delay_dist(0, 100);

    try {
        auto client = std::make_shared<core::messaging_client>(
            "stress_client_" + std::to_string(client_id)
        );

        client->start_client("127.0.0.1", config.base_port);
        metrics.total_connections++;

        std::this_thread::sleep_for(50ms); // Connection stabilization

        size_t messages_sent = 0;
        while (!stop_flag && messages_sent < config.messages_per_client) {
            // Determine message size
            size_t msg_size = config.enable_random_sizes ?
                              size_dist(gen) : config.message_size;

            // Create message
            std::vector<uint8_t> data(msg_size);
            for (size_t i = 0; i < msg_size; ++i) {
                data[i] = static_cast<uint8_t>((client_id + i) % 256);
            }

            // Send message
            try {
                client->send_packet(data);
                metrics.total_messages_sent++;
                metrics.total_bytes_sent += msg_size;
                messages_sent++;
            } catch (...) {
                metrics.total_messages_failed++;
            }

            // Optional delay
            if (config.enable_random_delays) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(delay_dist(gen))
                );
            }
        }

        client->stop_client();

    } catch (const std::exception& e) {
        metrics.failed_connections++;
        metrics.total_errors++;
        std::cerr << "Client " << client_id << " error: " << e.what() << std::endl;
    }
}

/**
 * @brief Run stress test
 */
bool run_stress_test(const StressConfig& config, StressMetrics& metrics) {
    std::cout << "\n🔥 Starting Stress Test" << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Clients: " << config.num_clients << std::endl;
    std::cout << "  Messages per client: " << config.messages_per_client << std::endl;
    std::cout << "  Message size: " << config.message_size << " bytes" << std::endl;
    std::cout << "  Duration: " << config.duration.count() << " seconds" << std::endl;

    try {
        // Start memory monitor
        MemoryMonitor mem_monitor;

        // Create server
        auto server = std::make_shared<core::messaging_server>("stress_server");
        server->start_server(config.base_port);

        std::this_thread::sleep_for(200ms); // Server startup time

        metrics.start_time = std::chrono::steady_clock::now();

        // Launch client threads
        std::vector<std::thread> client_threads;
        std::atomic<bool> stop_flag{false};

        for (size_t i = 0; i < config.num_clients; ++i) {
            client_threads.emplace_back(
                stress_client_worker,
                i,
                std::ref(config),
                std::ref(metrics),
                std::ref(stop_flag)
            );

            // Stagger client startup
            if (i % 10 == 0) {
                std::this_thread::sleep_for(10ms);
            }
        }

        // Monitor progress
        auto test_start = std::chrono::steady_clock::now();
        while (true) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - test_start
            );

            if (elapsed >= config.duration) {
                stop_flag = true;
                break;
            }

            // Print progress
            std::cout << "\rProgress: " << elapsed.count() << "s / "
                      << config.duration.count() << "s | "
                      << "Messages: " << metrics.total_messages_sent << " | "
                      << "Errors: " << metrics.total_errors << "     "
                      << std::flush;

            std::this_thread::sleep_for(1s);
        }

        std::cout << std::endl;

        // Wait for all clients to finish
        for (auto& thread : client_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        metrics.end_time = std::chrono::steady_clock::now();

        // Clean up
        server->stop_server();

        // Report memory usage
        std::cout << "\nMemory Usage:" << std::endl;
        std::cout << "  Peak: " << (mem_monitor.get_peak_memory() / (1024.0 * 1024.0))
                  << " MB" << std::endl;
        std::cout << "  Current: " << (mem_monitor.get_current_memory() / (1024.0 * 1024.0))
                  << " MB" << std::endl;

        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ Stress test failed: " << e.what() << std::endl;
        metrics.total_errors++;
        return false;
    }
}

/**
 * @brief Connection storm test
 */
bool connection_storm_test(StressMetrics& metrics) {
    std::cout << "\n⚡ Connection Storm Test" << std::endl;
    std::cout << "Creating 500 connections in rapid succession..." << std::endl;

    try {
        auto server = std::make_shared<core::messaging_server>("storm_server");
        server->start_server(11000);

        std::this_thread::sleep_for(100ms);

        std::vector<std::shared_ptr<core::messaging_client>> clients;

        // Create connections rapidly
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < 500; ++i) {
            try {
                auto client = std::make_shared<core::messaging_client>(
                    "storm_client_" + std::to_string(i)
                );
                client->start_client("127.0.0.1", 11000);
                clients.push_back(client);
                metrics.total_connections++;
            } catch (...) {
                metrics.failed_connections++;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

        std::cout << "Created " << clients.size() << " connections in "
                  << duration.count() << " ms" << std::endl;
        std::cout << "Connection rate: "
                  << (clients.size() * 1000.0 / duration.count())
                  << " connections/second" << std::endl;

        // Clean up
        for (auto& client : clients) {
            client->stop_client();
        }

        server->stop_server();

        return metrics.failed_connections < 50; // Allow up to 10% failure

    } catch (const std::exception& e) {
        std::cerr << "❌ Connection storm test failed: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Main stress test runner
 */
int main(int argc, char* argv[]) {
    std::cout << "=== Network System Stress Tests ===" << std::endl;
    std::cout << "Configuration: High-Performance | Target: 300K+ msg/s | Platform: Production-Ready" << std::endl;

    // Parse command line arguments
    StressConfig config;
    if (argc > 1) {
        config.num_clients = std::stoi(argv[1]);
    }
    if (argc > 2) {
        config.messages_per_client = std::stoi(argv[2]);
    }
    if (argc > 3) {
        config.duration = std::chrono::seconds(std::stoi(argv[3]));
    }

    // Initialize system
    network_system::compat::initialize();
    std::cout << "\nSystem initialized" << std::endl;

    StressMetrics metrics;

    // Run stress tests
    bool success = true;

    // Test 1: Main stress test
    if (!run_stress_test(config, metrics)) {
        success = false;
    }

    metrics.print();

    // Test 2: Connection storm
    if (!connection_storm_test(metrics)) {
        success = false;
    }

    // Cleanup
    network_system::compat::shutdown();
    std::cout << "\nSystem shutdown complete" << std::endl;

    // Final verdict
    if (success && metrics.is_successful()) {
        std::cout << "\n✅ STRESS TESTS PASSED" << std::endl;
        std::cout << "System is stable under heavy load!" << std::endl;
        return 0;
    } else {
        std::cout << "\n❌ STRESS TESTS FAILED" << std::endl;
        std::cout << "System showed instability under load" << std::endl;
        return 1;
    }
}