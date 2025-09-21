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

using namespace kcenon::messaging::core;
using namespace kcenon::messaging::integrations;

class BenchmarkRunner {
public:
    void run_all_benchmarks() {
        std::cout << "=== Messaging System Performance Benchmarks ===\n\n";

        run_throughput_benchmark();
        run_concurrent_benchmark();
        run_priority_benchmark();
        run_message_size_benchmark();
        run_system_integrator_benchmark();

        std::cout << "\n=== Benchmark Complete ===\n";
    }

private:
    void run_throughput_benchmark() {
        std::cout << "1. Throughput Benchmark\n";
        std::cout << "   Testing message processing throughput...\n";

        message_bus_config config;
        config.worker_threads = 8;
        config.max_queue_size = 100000;
        config.enable_priority_queue = true;

        auto bus = std::make_unique<message_bus>(config);
        bus->initialize();

        constexpr int total_messages = 100000;
        std::atomic<int> processed{0};

        bus->subscribe("benchmark.throughput", [&](const message& msg) {
            processed++;
        });

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < total_messages; ++i) {
            message_payload payload;
            payload.topic = "benchmark.throughput";
            payload.data["sequence"] = int64_t(i);
            payload.data["data"] = std::string("benchmark_data");

            bus->publish("benchmark.throughput", payload);
        }

        auto publish_end = std::chrono::high_resolution_clock::now();

        while (processed.load() < total_messages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto process_end = std::chrono::high_resolution_clock::now();

        auto publish_duration = std::chrono::duration_cast<std::chrono::milliseconds>(publish_end - start_time);
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(process_end - start_time);

        double publish_rate = (total_messages * 1000.0) / publish_duration.count();
        double process_rate = (total_messages * 1000.0) / total_duration.count();

        std::cout << "   Results:\n";
        std::cout << "   - Messages: " << total_messages << "\n";
        std::cout << "   - Publish rate: " << std::fixed << std::setprecision(0) << publish_rate << " msg/sec\n";
        std::cout << "   - Processing rate: " << std::fixed << std::setprecision(0) << process_rate << " msg/sec\n";
        std::cout << "   - Total time: " << total_duration.count() << " ms\n\n";

        bus->shutdown();
    }

    void run_concurrent_benchmark() {
        std::cout << "2. Concurrent Publishers Benchmark\n";
        std::cout << "   Testing concurrent publishing performance...\n";

        message_bus_config config;
        config.worker_threads = 8;
        config.max_queue_size = 200000;

        auto bus = std::make_unique<message_bus>(config);
        bus->initialize();

        constexpr int num_publishers = 8;
        constexpr int messages_per_publisher = 10000;
        constexpr int total_messages = num_publishers * messages_per_publisher;

        std::atomic<int> total_processed{0};

        bus->subscribe("benchmark.concurrent", [&](const message& msg) {
            total_processed++;
        });

        auto start_time = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> publishers;
        for (int p = 0; p < num_publishers; ++p) {
            publishers.emplace_back([&bus, p, messages_per_publisher]() {
                for (int i = 0; i < messages_per_publisher; ++i) {
                    message_payload payload;
                    payload.topic = "benchmark.concurrent";
                    payload.data["publisher_id"] = int64_t(p);
                    payload.data["message_id"] = int64_t(i);

                    bus->publish("benchmark.concurrent", payload);
                }
            });
        }

        for (auto& thread : publishers) {
            thread.join();
        }

        auto publish_end = std::chrono::high_resolution_clock::now();

        while (total_processed.load() < total_messages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto process_end = std::chrono::high_resolution_clock::now();

        auto publish_duration = std::chrono::duration_cast<std::chrono::milliseconds>(publish_end - start_time);
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(process_end - start_time);

        double concurrent_rate = (total_messages * 1000.0) / total_duration.count();

        std::cout << "   Results:\n";
        std::cout << "   - Publishers: " << num_publishers << "\n";
        std::cout << "   - Messages per publisher: " << messages_per_publisher << "\n";
        std::cout << "   - Total messages: " << total_messages << "\n";
        std::cout << "   - Concurrent rate: " << std::fixed << std::setprecision(0) << concurrent_rate << " msg/sec\n";
        std::cout << "   - Publish time: " << publish_duration.count() << " ms\n\n";

        bus->shutdown();
    }

    void run_priority_benchmark() {
        std::cout << "3. Priority Queue Benchmark\n";
        std::cout << "   Testing priority queue performance...\n";

        message_bus_config config;
        config.worker_threads = 4;
        config.max_queue_size = 50000;
        config.enable_priority_queue = true;

        auto bus = std::make_unique<message_bus>(config);
        bus->initialize();

        constexpr int total_messages = 20000;
        std::atomic<int> processed{0};

        bus->subscribe("benchmark.priority", [&](const message& msg) {
            processed++;
        });

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> priority_dist(0, 3);

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < total_messages; ++i) {
            message msg;
            msg.payload.topic = "benchmark.priority";
            msg.payload.data["sequence"] = int64_t(i);
            msg.metadata.priority = static_cast<message_priority>(priority_dist(gen));

            bus->publish(msg);
        }

        while (processed.load() < total_messages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        double rate = (total_messages * 1000.0) / duration.count();

        std::cout << "   Results:\n";
        std::cout << "   - Messages: " << total_messages << "\n";
        std::cout << "   - Priority queue rate: " << std::fixed << std::setprecision(0) << rate << " msg/sec\n";
        std::cout << "   - Total time: " << duration.count() << " ms\n\n";

        bus->shutdown();
    }

    void run_message_size_benchmark() {
        std::cout << "4. Message Size Impact Benchmark\n";
        std::cout << "   Testing performance with different message sizes...\n";

        message_bus_config config;
        config.worker_threads = 4;
        config.max_queue_size = 10000;

        auto bus = std::make_unique<message_bus>(config);
        bus->initialize();

        std::vector<size_t> message_sizes = {64, 256, 1024, 4096, 16384};
        constexpr int messages_per_size = 2000;

        for (size_t size : message_sizes) {
            std::atomic<int> processed{0};
            std::string topic = "benchmark.size." + std::to_string(size);

            bus->subscribe(topic, [&](const message& msg) {
                processed++;
            });

            std::string large_data(size, 'X');

            auto start_time = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < messages_per_size; ++i) {
                message_payload payload;
                payload.topic = topic;
                payload.data["large_data"] = large_data;
                payload.data["sequence"] = int64_t(i);

                bus->publish(topic, payload);
            }

            while (processed.load() < messages_per_size) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

            double rate = (messages_per_size * 1000.0) / duration.count();
            double throughput_mb = (messages_per_size * size * 1000.0) / (duration.count() * 1024 * 1024);

            std::cout << "   Size " << std::setw(6) << size << " bytes: "
                      << std::setw(6) << std::fixed << std::setprecision(0) << rate << " msg/sec, "
                      << std::setw(6) << std::fixed << std::setprecision(2) << throughput_mb << " MB/sec\n";
        }

        std::cout << "\n";
        bus->shutdown();
    }

    void run_system_integrator_benchmark() {
        std::cout << "5. System Integrator Benchmark\n";
        std::cout << "   Testing full system integration performance...\n";

        auto integrator = system_integrator::create_default();
        integrator->initialize();

        constexpr int total_messages = 50000;
        std::atomic<int> processed{0};

        integrator->subscribe("benchmark.system", [&](const message& msg) {
            processed++;
        });

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < total_messages; ++i) {
            message_payload payload;
            payload.topic = "benchmark.system";
            payload.data["sequence"] = int64_t(i);
            payload.data["component"] = std::string("system_integrator");

            integrator->publish("benchmark.system", payload, "benchmark");
        }

        while (processed.load() < total_messages) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        double rate = (total_messages * 1000.0) / duration.count();

        std::cout << "   Results:\n";
        std::cout << "   - Messages: " << total_messages << "\n";
        std::cout << "   - System integration rate: " << std::fixed << std::setprecision(0) << rate << " msg/sec\n";
        std::cout << "   - Total time: " << duration.count() << " ms\n\n";

        // Check system health
        auto health = integrator->check_system_health();
        std::cout << "   System Health:\n";
        std::cout << "   - Message bus healthy: " << (health.message_bus_healthy ? "Yes" : "No") << "\n";
        std::cout << "   - Active services: " << health.active_services << "\n";
        std::cout << "   - Total messages processed: " << health.total_messages_processed << "\n\n";

        integrator->shutdown();
    }
};

int main(int argc, char* argv[]) {
    std::cout << "Messaging System Performance Benchmark\n";
    std::cout << "=======================================\n\n";

    try {
        BenchmarkRunner runner;
        runner.run_all_benchmarks();
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}