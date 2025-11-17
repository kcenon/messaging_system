#include "bench_common.h"
#include <kcenon/messaging/core/message_queue.h>
#include <kcenon/messaging/core/message.h>
#include <thread>
#include <iostream>

using namespace kcenon::messaging;
using namespace kcenon::messaging::benchmark;

/**
 * @brief Benchmark message queue performance
 */
int main() {
    std::cout << "\n========================================\n";
    std::cout << "Message Queue Benchmarks\n";
    std::cout << "========================================\n";

    // Benchmark 1: Enqueue performance
    {
        queue_config config;
        config.max_size = 100000;
        message_queue queue(config);

        auto msg = message_builder()
            .topic("test.topic")
            .build();

        if (msg.is_ok()) {
            run_benchmark("Queue Enqueue", 10000, [&queue, &msg]() {
                queue.enqueue(msg.unwrap());
            });
        }
    }

    // Benchmark 2: Dequeue performance
    {
        queue_config config;
        config.max_size = 100000;
        message_queue queue(config);

        // Pre-fill queue
        auto msg = message_builder().topic("test.topic").build();
        if (msg.is_ok()) {
            for (int i = 0; i < 10000; ++i) {
                queue.enqueue(msg.unwrap());
            }

            run_benchmark("Queue Dequeue", 10000, [&queue]() {
                auto result = queue.try_dequeue();
            });
        }
    }

    // Benchmark 3: Priority queue performance
    {
        queue_config config;
        config.max_size = 100000;
        config.enable_priority = true;
        message_queue queue(config);

        auto msg = message_builder()
            .topic("test.topic")
            .priority(message_priority::high)
            .build();

        if (msg.is_ok()) {
            run_benchmark("Priority Queue Enqueue", 10000, [&queue, &msg]() {
                queue.enqueue(msg.unwrap());
            });
        }
    }

    // Benchmark 4: Concurrent enqueue/dequeue
    {
        queue_config config;
        config.max_size = 100000;
        message_queue queue(config);

        const int operations = 50000;
        BenchmarkTimer timer;

        std::thread producer([&]() {
            auto msg = message_builder().topic("test").build();
            if (msg.is_ok()) {
                for (int i = 0; i < operations; ++i) {
                    queue.enqueue(msg.unwrap());
                }
            }
        });

        std::thread consumer([&]() {
            for (int i = 0; i < operations; ++i) {
                queue.dequeue(std::chrono::seconds{1});
            }
        });

        producer.join();
        consumer.join();

        double duration = timer.elapsed_seconds();
        print_throughput("Concurrent Enqueue/Dequeue", operations * 2, duration);
    }

    // Benchmark 5: Throughput test
    {
        queue_config config;
        config.max_size = 200000;
        message_queue queue(config);

        const int batch_size = 100000;
        auto msg = message_builder().topic("test").build();

        if (msg.is_ok()) {
            BenchmarkTimer timer;

            for (int i = 0; i < batch_size; ++i) {
                queue.enqueue(msg.unwrap());
            }

            double duration = timer.elapsed_seconds();
            print_throughput("Queue Enqueue Throughput", batch_size, duration);
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "Benchmark Complete\n";
    std::cout << "========================================\n\n";

    return 0;
}
