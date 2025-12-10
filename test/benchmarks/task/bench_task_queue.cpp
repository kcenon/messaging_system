// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file bench_task_queue.cpp
 * @brief Performance benchmarks for task_queue
 *
 * Measures:
 * - Enqueue throughput (ops/sec)
 * - Dequeue throughput (ops/sec)
 * - Priority queue overhead
 * - Concurrent producer/consumer performance
 *
 * Performance Targets:
 * - Queue enqueue: > 100,000 ops/sec
 * - Queue dequeue: > 50,000 ops/sec
 */

#include "../bench_common.h"
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/task.h>

#include <iostream>
#include <thread>
#include <vector>

using namespace kcenon::messaging::task;
using namespace kcenon::messaging::benchmark;

namespace {

task create_test_task(const std::string& name = "benchmark.task") {
	auto result = task_builder(name)
					  .queue("benchmark")
					  .build();
	if (result.is_ok()) {
		return result.unwrap();
	}
	return task(name);
}

}  // namespace

int main() {
	std::cout << "\n========================================\n";
	std::cout << "Task Queue Benchmarks\n";
	std::cout << "========================================\n";

	// Benchmark 1: Enqueue throughput
	{
		std::cout << "\n--- Benchmark 1: Enqueue Throughput ---\n";

		task_queue_config config;
		config.max_size = 200000;
		config.enable_delayed_queue = false;
		task_queue queue(config);
		queue.start();

		const int batch_size = 100000;
		BenchmarkTimer timer;

		for (int i = 0; i < batch_size; ++i) {
			auto t = create_test_task();
			queue.enqueue(std::move(t));
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Task Queue Enqueue", batch_size, duration);

		double ops_per_sec = batch_size / duration;
		std::cout << "  Target: > 100,000 ops/sec\n";
		std::cout << "  Status: " << (ops_per_sec > 100000 ? "PASS" : "BELOW TARGET") << "\n";

		queue.stop();
	}

	// Benchmark 2: Dequeue throughput
	{
		std::cout << "\n--- Benchmark 2: Dequeue Throughput ---\n";

		task_queue_config config;
		config.max_size = 200000;
		config.enable_delayed_queue = false;
		task_queue queue(config);
		queue.start();

		const int batch_size = 100000;

		// Pre-fill queue
		for (int i = 0; i < batch_size; ++i) {
			auto t = create_test_task();
			queue.enqueue(std::move(t));
		}

		std::vector<std::string> queue_names = {"benchmark"};

		BenchmarkTimer timer;
		int dequeued = 0;

		while (dequeued < batch_size) {
			auto result = queue.try_dequeue(queue_names);
			if (result.is_ok()) {
				++dequeued;
			}
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Task Queue Dequeue", batch_size, duration);

		double ops_per_sec = batch_size / duration;
		std::cout << "  Target: > 50,000 ops/sec\n";
		std::cout << "  Status: " << (ops_per_sec > 50000 ? "PASS" : "BELOW TARGET") << "\n";

		queue.stop();
	}

	// Benchmark 3: Priority queue overhead
	{
		std::cout << "\n--- Benchmark 3: Priority Queue Overhead ---\n";

		task_queue_config config;
		config.max_size = 100000;
		config.enable_delayed_queue = false;
		task_queue queue(config);
		queue.start();

		const int operations = 50000;

		// High priority tasks
		BenchmarkTimer high_timer;
		for (int i = 0; i < operations; ++i) {
			auto result = task_builder("benchmark.high")
							  .queue("priority-test")
							  .priority(kcenon::messaging::message_priority::high)
							  .build();
			if (result.is_ok()) {
				queue.enqueue(result.unwrap());
			}
		}
		double high_duration = high_timer.elapsed_seconds();

		queue.stop();
		queue = task_queue(config);
		queue.start();

		// Normal priority tasks
		BenchmarkTimer normal_timer;
		for (int i = 0; i < operations; ++i) {
			auto result = task_builder("benchmark.normal")
							  .queue("priority-test")
							  .priority(kcenon::messaging::message_priority::normal)
							  .build();
			if (result.is_ok()) {
				queue.enqueue(result.unwrap());
			}
		}
		double normal_duration = normal_timer.elapsed_seconds();

		std::cout << "\n=== Priority Queue Overhead ===\n";
		std::cout << std::fixed << std::setprecision(3);
		std::cout << "  High priority enqueue:   " << high_duration << " seconds ("
				  << static_cast<int>(operations / high_duration) << " ops/sec)\n";
		std::cout << "  Normal priority enqueue: " << normal_duration << " seconds ("
				  << static_cast<int>(operations / normal_duration) << " ops/sec)\n";

		double overhead = ((high_duration / normal_duration) - 1.0) * 100.0;
		std::cout << "  Overhead: " << std::setprecision(1) << overhead << "%\n";

		queue.stop();
	}

	// Benchmark 4: Concurrent producer/consumer
	{
		std::cout << "\n--- Benchmark 4: Concurrent Producer/Consumer ---\n";

		task_queue_config config;
		config.max_size = 200000;
		config.enable_delayed_queue = false;
		task_queue queue(config);
		queue.start();

		const int operations = 50000;
		std::atomic<int> produced{0};
		std::atomic<int> consumed{0};
		std::vector<std::string> queue_names = {"concurrent-test"};

		BenchmarkTimer timer;

		std::thread producer([&]() {
			for (int i = 0; i < operations; ++i) {
				auto result = task_builder("benchmark.concurrent")
								  .queue("concurrent-test")
								  .build();
				if (result.is_ok()) {
					queue.enqueue(result.unwrap());
					++produced;
				}
			}
		});

		std::thread consumer([&]() {
			while (consumed < operations) {
				auto result = queue.dequeue(queue_names, std::chrono::milliseconds{100});
				if (result.is_ok()) {
					++consumed;
				}
			}
		});

		producer.join();
		consumer.join();

		double duration = timer.elapsed_seconds();
		print_throughput("Concurrent Enqueue/Dequeue", operations * 2, duration);

		queue.stop();
	}

	// Benchmark 5: Multi-queue performance
	{
		std::cout << "\n--- Benchmark 5: Multi-Queue Performance ---\n";

		task_queue_config config;
		config.max_size = 200000;
		config.enable_delayed_queue = false;
		task_queue queue(config);
		queue.start();

		const int operations_per_queue = 20000;
		const int num_queues = 5;
		std::vector<std::string> queue_names;
		for (int i = 0; i < num_queues; ++i) {
			queue_names.push_back("multi-queue-" + std::to_string(i));
		}

		BenchmarkTimer timer;

		// Enqueue to multiple queues
		for (int i = 0; i < operations_per_queue * num_queues; ++i) {
			auto result = task_builder("benchmark.multi")
							  .queue(queue_names[i % num_queues])
							  .build();
			if (result.is_ok()) {
				queue.enqueue(result.unwrap());
			}
		}

		double enqueue_duration = timer.elapsed_seconds();

		// Dequeue from all queues
		timer.reset();
		int dequeued = 0;
		while (dequeued < operations_per_queue * num_queues) {
			auto result = queue.try_dequeue(queue_names);
			if (result.is_ok()) {
				++dequeued;
			}
		}
		double dequeue_duration = timer.elapsed_seconds();

		std::cout << "\n=== Multi-Queue Performance (" << num_queues << " queues) ===\n";
		std::cout << std::fixed << std::setprecision(3);
		std::cout << "  Total operations: " << operations_per_queue * num_queues << "\n";
		std::cout << "  Enqueue: " << enqueue_duration << " seconds ("
				  << static_cast<int>((operations_per_queue * num_queues) / enqueue_duration)
				  << " ops/sec)\n";
		std::cout << "  Dequeue: " << dequeue_duration << " seconds ("
				  << static_cast<int>((operations_per_queue * num_queues) / dequeue_duration)
				  << " ops/sec)\n";

		queue.stop();
	}

	// Benchmark 6: Task cancellation overhead
	{
		std::cout << "\n--- Benchmark 6: Task Cancellation ---\n";

		task_queue_config config;
		config.max_size = 100000;
		config.enable_delayed_queue = false;
		task_queue queue(config);
		queue.start();

		const int operations = 10000;
		std::vector<std::string> task_ids;
		task_ids.reserve(operations);

		// Enqueue tasks
		for (int i = 0; i < operations; ++i) {
			auto result = task_builder("benchmark.cancel")
							  .queue("cancel-test")
							  .tag("batch-cancel")
							  .build();
			if (result.is_ok()) {
				auto enqueue_result = queue.enqueue(result.unwrap());
				if (enqueue_result.is_ok()) {
					task_ids.push_back(enqueue_result.unwrap());
				}
			}
		}

		// Benchmark individual cancellation
		BenchmarkTimer cancel_timer;
		for (size_t i = 0; i < task_ids.size() / 2; ++i) {
			queue.cancel(task_ids[i]);
		}
		double individual_duration = cancel_timer.elapsed_seconds();

		// Benchmark tag-based cancellation
		BenchmarkTimer tag_cancel_timer;
		queue.cancel_by_tag("batch-cancel");
		double tag_duration = tag_cancel_timer.elapsed_seconds();

		std::cout << "\n=== Task Cancellation Performance ===\n";
		std::cout << std::fixed << std::setprecision(6);
		std::cout << "  Individual cancel (" << task_ids.size() / 2 << " tasks): "
				  << individual_duration << " seconds\n";
		std::cout << "  Tag-based cancel: " << tag_duration << " seconds\n";

		queue.stop();
	}

	std::cout << "\n========================================\n";
	std::cout << "Task Queue Benchmarks Complete\n";
	std::cout << "========================================\n\n";

	return 0;
}
