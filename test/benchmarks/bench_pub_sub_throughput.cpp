#include "bench_common.h"
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/patterns/pub_sub.h>
#include <atomic>
#include <iostream>

using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;
using namespace kcenon::messaging::benchmark;
using namespace kcenon::common;

/**
 * @brief Benchmark pub/sub throughput
 */
int main() {
    std::cout << "\n========================================\n";
    std::cout << "Pub/Sub Throughput Benchmarks\n";
    std::cout << "========================================\n";

    // Setup
    auto backend = std::make_shared<standalone_backend>(4);
    if (!backend->initialize().is_ok()) {
        std::cerr << "Failed to initialize backend\n";
        return 1;
    }

    message_bus_config config;
    config.queue_capacity = 100000;
    config.worker_threads = 4;

    auto bus = std::make_shared<message_bus>(backend, config);
    if (!bus->start().is_ok()) {
        std::cerr << "Failed to start message bus\n";
        return 1;
    }

    // Benchmark 1: Single publisher, single subscriber
    {
        std::atomic<int> received{0};
        auto sub_result = bus->subscribe("bench.single", [&](const message&) {
            received.fetch_add(1);
            return common::ok();
        });

        if (!sub_result.is_ok()) {
            std::cerr << "Failed to subscribe\n";
            return 1;
        }

        const int count = 10000;
        auto msg = message_builder().topic("bench.single").build();

        if (msg.is_ok()) {
            BenchmarkTimer timer;

            for (int i = 0; i < count; ++i) {
                bus->publish(msg.unwrap());
            }

            // Wait for all messages
            while (received.load() < count) {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }

            double duration = timer.elapsed_seconds();
            print_throughput("Single Publisher/Subscriber", count, duration);
        }

        bus->unsubscribe(sub_result.unwrap());
    }

    // Benchmark 2: Single publisher, multiple subscribers
    {
        std::vector<std::atomic<int>> counters(5);
        std::vector<uint64_t> sub_ids;

        for (auto& counter : counters) {
            auto sub_result = bus->subscribe("bench.multi", [&counter](const message&) {
                counter.fetch_add(1);
                return common::ok();
            });

            if (sub_result.is_ok()) {
                sub_ids.push_back(sub_result.unwrap());
            }
        }

        const int count = 5000;
        auto msg = message_builder().topic("bench.multi").build();

        if (msg.is_ok()) {
            BenchmarkTimer timer;

            for (int i = 0; i < count; ++i) {
                bus->publish(msg.unwrap());
            }

            // Wait for all subscribers
            bool all_received = false;
            while (!all_received) {
                all_received = true;
                for (const auto& counter : counters) {
                    if (counter.load() < count) {
                        all_received = false;
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }

            double duration = timer.elapsed_seconds();
            print_throughput("1 Publisher / 5 Subscribers", count * 5, duration);
        }

        for (auto id : sub_ids) {
            bus->unsubscribe(id);
        }
    }

    // Benchmark 3: High throughput test
    {
        std::atomic<int> received{0};
        auto sub_result = bus->subscribe("bench.throughput", [&](const message&) {
            received.fetch_add(1);
            return common::ok();
        });

        if (sub_result.is_ok()) {
            const int count = 100000;
            auto msg = message_builder().topic("bench.throughput").build();

            if (msg.is_ok()) {
                BenchmarkTimer timer;

                for (int i = 0; i < count; ++i) {
                    bus->publish(msg.unwrap());
                }

                // Wait for completion
                while (received.load() < count) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{10});
                }

                double duration = timer.elapsed_seconds();
                print_throughput("High Throughput Test", count, duration);
            }

            bus->unsubscribe(sub_result.unwrap());
        }
    }

    // Benchmark 4: Pattern-based pub/sub
    {
        std::atomic<int> received{0};
        auto sub_result = bus->subscribe("events.#", [&](const message&) {
            received.fetch_add(1);
            return common::ok();
        });

        if (sub_result.is_ok()) {
            const int count = 10000;

            BenchmarkTimer timer;

            for (int i = 0; i < count; ++i) {
                auto topic = "events.type" + std::to_string(i % 10);
                auto msg = message_builder().topic(topic).build();

                if (msg.is_ok()) {
                    bus->publish(msg.unwrap());
                }
            }

            while (received.load() < count) {
                std::this_thread::sleep_for(std::chrono::milliseconds{1});
            }

            double duration = timer.elapsed_seconds();
            print_throughput("Pattern-Based Pub/Sub", count, duration);

            bus->unsubscribe(sub_result.unwrap());
        }
    }

    // Cleanup
    bus->stop();
    backend->shutdown();

    std::cout << "\n========================================\n";
    std::cout << "Benchmark Complete\n";
    std::cout << "========================================\n\n";

    return 0;
}
