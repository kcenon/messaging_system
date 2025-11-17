#include "bench_common.h"
#include <kcenon/messaging/core/topic_router.h>
#include <kcenon/messaging/core/message.h>
#include <iostream>

using namespace kcenon::messaging;
using namespace kcenon::messaging::benchmark;
using namespace kcenon::common;

/**
 * @brief Benchmark topic router performance
 */
int main() {
    std::cout << "\n========================================\n";
    std::cout << "Topic Router Benchmarks\n";
    std::cout << "========================================\n";

    // Benchmark 1: Subscribe performance
    {
        topic_router router;

        run_benchmark("Router Subscribe", 1000, [&router]() {
            router.subscribe("test.topic", [](const message&) {
                return common::ok();
            });
        });
    }

    // Benchmark 2: Route to single subscriber
    {
        topic_router router;
        auto msg = message_builder().topic("test.topic").build();

        if (msg.is_ok()) {
            router.subscribe("test.topic", [](const message&) {
                return common::ok();
            });

            run_benchmark("Route to Single Subscriber", 10000, [&router, &msg]() {
                router.route(msg.unwrap());
            });
        }
    }

    // Benchmark 3: Route with wildcards
    {
        topic_router router;
        auto msg = message_builder().topic("test.topic.deep").build();

        if (msg.is_ok()) {
            router.subscribe("test.*", [](const message&) {
                return common::ok();
            });
            router.subscribe("test.#", [](const message&) {
                return common::ok();
            });

            run_benchmark("Route with Wildcards", 10000, [&router, &msg]() {
                router.route(msg.unwrap());
            });
        }
    }

    // Benchmark 4: Route to multiple subscribers
    {
        topic_router router;
        auto msg = message_builder().topic("test.topic").build();

        if (msg.is_ok()) {
            for (int i = 0; i < 10; ++i) {
                router.subscribe("test.topic", [](const message&) {
                    return common::ok();
                });
            }

            run_benchmark("Route to 10 Subscribers", 10000, [&router, &msg]() {
                router.route(msg.unwrap());
            });
        }
    }

    // Benchmark 5: Complex routing scenario
    {
        topic_router router;

        // Setup complex subscription tree
        router.subscribe("orders.created", [](const message&) { return common::ok(); });
        router.subscribe("orders.updated", [](const message&) { return common::ok(); });
        router.subscribe("orders.deleted", [](const message&) { return common::ok(); });
        router.subscribe("orders.*", [](const message&) { return common::ok(); });
        router.subscribe("orders.#", [](const message&) { return common::ok(); });
        router.subscribe("*.created", [](const message&) { return common::ok(); });

        auto msg = message_builder().topic("orders.created").build();

        if (msg.is_ok()) {
            run_benchmark("Complex Routing", 10000, [&router, &msg]() {
                router.route(msg.unwrap());
            });
        }
    }

    // Benchmark 6: Throughput test
    {
        topic_router router;
        router.subscribe("test.topic", [](const message&) {
            return common::ok();
        });

        auto msg = message_builder().topic("test.topic").build();
        const int operations = 100000;

        if (msg.is_ok()) {
            BenchmarkTimer timer;

            for (int i = 0; i < operations; ++i) {
                router.route(msg.unwrap());
            }

            double duration = timer.elapsed_seconds();
            print_throughput("Router Throughput", operations, duration);
        }
    }

    std::cout << "\n========================================\n";
    std::cout << "Benchmark Complete\n";
    std::cout << "========================================\n\n";

    return 0;
}
