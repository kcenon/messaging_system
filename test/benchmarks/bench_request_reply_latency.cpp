#include "bench_common.h"
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/patterns/request_reply.h>
#include <iostream>

using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;
using namespace kcenon::messaging::benchmark;
using namespace kcenon::common;

/**
 * @brief Benchmark request/reply latency
 */
int main() {
    std::cout << "\n========================================\n";
    std::cout << "Request/Reply Latency Benchmarks\n";
    std::cout << "========================================\n";

    // Setup
    auto backend = std::make_shared<standalone_backend>(4);
    if (!backend->initialize().is_ok()) {
        std::cerr << "Failed to initialize backend\n";
        return 1;
    }

    message_bus_config config;
    config.queue_capacity = 10000;
    config.worker_threads = 4;

    auto bus = std::make_shared<message_bus>(backend, config);
    if (!bus->start().is_ok()) {
        std::cerr << "Failed to start message bus\n";
        return 1;
    }

    // Setup request/reply handler
    auto handler = std::make_shared<request_reply_handler>(bus, "service.bench");

    // Register echo handler
    auto register_result = handler->register_handler(
        [](const message& req) -> Result<message> {
            auto reply = message_builder()
                .topic("service.bench.reply")
                .correlation_id(req.metadata().correlation_id)
                .build();

            return reply;
        }
    );

    if (!register_result.is_ok()) {
        std::cerr << "Failed to register handler\n";
        return 1;
    }

    // Benchmark 1: Basic request/reply latency
    {
        BenchmarkResults results("Request/Reply Latency");

        for (int i = 0; i < 1000; ++i) {
            auto request = message_builder()
                .topic("service.bench")
                .build();

            if (request.is_ok()) {
                BenchmarkTimer timer;
                auto reply_result = handler->request(
                    request.unwrap(),
                    std::chrono::seconds{2}
                );
                double latency = timer.elapsed<std::chrono::microseconds>() / 1000.0; // Convert to ms

                if (reply_result.is_ok()) {
                    results.add_duration(latency);
                }
            }

            if ((i + 1) % 100 == 0) {
                std::cout << "  Progress: " << (i + 1) << "/1000\n";
            }
        }

        results.print();
    }

    // Benchmark 2: Request/reply with payload
    {
        BenchmarkResults results("Request/Reply with Payload");

        for (int i = 0; i < 1000; ++i) {
            auto request = message_builder()
                .topic("service.bench")
                .type(message_type::query)
                .priority(message_priority::normal)
                .build();

            if (request.is_ok()) {
                BenchmarkTimer timer;
                auto reply_result = handler->request(
                    request.unwrap(),
                    std::chrono::seconds{2}
                );
                double latency = timer.elapsed<std::chrono::microseconds>() / 1000.0;

                if (reply_result.is_ok()) {
                    results.add_duration(latency);
                }
            }

            if ((i + 1) % 100 == 0) {
                std::cout << "  Progress: " << (i + 1) << "/1000\n";
            }
        }

        results.print();
    }

    // Benchmark 3: Concurrent requests
    {
        const int num_threads = 4;
        const int requests_per_thread = 250;

        BenchmarkResults results("Concurrent Request/Reply");
        std::mutex results_mutex;

        std::vector<std::thread> threads;
        BenchmarkTimer overall_timer;

        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < requests_per_thread; ++i) {
                    auto request = message_builder()
                        .topic("service.bench")
                        .build();

                    if (request.is_ok()) {
                        BenchmarkTimer timer;
                        auto reply_result = handler->request(
                            request.unwrap(),
                            std::chrono::seconds{2}
                        );
                        double latency = timer.elapsed<std::chrono::microseconds>() / 1000.0;

                        if (reply_result.is_ok()) {
                            std::lock_guard lock(results_mutex);
                            results.add_duration(latency);
                        }
                    }
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        double overall_duration = overall_timer.elapsed_seconds();

        results.print();
        print_throughput(
            "Concurrent Requests",
            num_threads * requests_per_thread,
            overall_duration
        );
    }

    // Benchmark 4: High priority requests
    {
        BenchmarkResults results("High Priority Request/Reply");

        for (int i = 0; i < 500; ++i) {
            auto request = message_builder()
                .topic("service.bench")
                .priority(message_priority::high)
                .build();

            if (request.is_ok()) {
                BenchmarkTimer timer;
                auto reply_result = handler->request(
                    request.unwrap(),
                    std::chrono::seconds{2}
                );
                double latency = timer.elapsed<std::chrono::microseconds>() / 1000.0;

                if (reply_result.is_ok()) {
                    results.add_duration(latency);
                }
            }

            if ((i + 1) % 100 == 0) {
                std::cout << "  Progress: " << (i + 1) << "/500\n";
            }
        }

        results.print();
    }

    // Cleanup
    bus->stop();
    backend->shutdown();

    std::cout << "\n========================================\n";
    std::cout << "Benchmark Complete\n";
    std::cout << "========================================\n\n";

    return 0;
}
