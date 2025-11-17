#include "bench_common.h"
#include <kcenon/messaging/core/message.h>
#include <iostream>

using namespace kcenon::messaging;
using namespace kcenon::messaging::benchmark;

/**
 * @brief Benchmark message creation performance
 */
int main() {
    std::cout << "\n========================================\n";
    std::cout << "Message Creation Benchmarks\n";
    std::cout << "========================================\n";

    // Benchmark 1: Basic message creation
    run_benchmark("Basic Message Creation", 10000, []() {
        auto msg = message_builder()
            .topic("test.topic")
            .build();
    });

    // Benchmark 2: Message with metadata
    run_benchmark("Message with Metadata", 10000, []() {
        auto msg = message_builder()
            .topic("test.topic")
            .type(message_type::event)
            .priority(message_priority::normal)
            .source("benchmark")
            .build();
    });

    // Benchmark 3: Message with full metadata
    run_benchmark("Message with Full Metadata", 10000, []() {
        auto msg = message_builder()
            .topic("test.topic.deep.nested")
            .type(message_type::command)
            .priority(message_priority::high)
            .source("benchmark_source")
            .target("benchmark_target")
            .correlation_id("corr_12345")
            .trace_id("trace_67890")
            .ttl(std::chrono::seconds{30})
            .header("custom_header", "custom_value")
            .build();
    });

    // Benchmark 4: Message copy
    auto original = message_builder()
        .topic("test.topic")
        .build();

    if (original.is_ok()) {
        auto msg = original.unwrap();
        run_benchmark("Message Copy", 10000, [&msg]() {
            auto copy = msg;
        });
    }

    // Benchmark 5: Throughput test - batch creation
    {
        const int batch_size = 100000;
        BenchmarkTimer timer;

        for (int i = 0; i < batch_size; ++i) {
            auto msg = message_builder()
                .topic("test.topic")
                .build();
        }

        double duration = timer.elapsed_seconds();
        print_throughput("Message Creation", batch_size, duration);
    }

    std::cout << "\n========================================\n";
    std::cout << "Benchmark Complete\n";
    std::cout << "========================================\n\n";

    return 0;
}
