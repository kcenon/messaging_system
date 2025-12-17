// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file bench_worker_throughput.cpp
 * @brief Performance benchmarks for worker_pool throughput
 *
 * Measures:
 * - Single worker throughput
 * - Multi-worker scalability
 * - Handler dispatch latency
 *
 * Performance Targets:
 * - Worker throughput (empty task): > 10,000 tasks/sec
 */

#include "../bench_common.h"
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/worker_pool.h>
#include <kcenon/messaging/task/memory_result_backend.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::messaging::benchmark;

namespace {

std::atomic<int> tasks_processed{0};
std::atomic<int64_t> total_latency_ns{0};

void reset_counters() {
	tasks_processed.store(0, std::memory_order_relaxed);
	total_latency_ns.store(0, std::memory_order_relaxed);
}

}  // namespace

int main() {
	std::cout << "\n========================================\n";
	std::cout << "Worker Throughput Benchmarks\n";
	std::cout << "========================================\n";

	// Benchmark 1: Single worker throughput (empty task)
	{
		std::cout << "\n--- Benchmark 1: Single Worker Throughput ---\n";

		reset_counters();

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();

		worker_config config;
		config.concurrency = 1;
		config.queues = {"benchmark"};
		config.poll_interval = std::chrono::milliseconds{1};

		worker_pool pool(queue, backend, config);

		// Register empty handler
		pool.register_handler("benchmark.empty",
			[](const task& /* t */, task_context& /* ctx */)
				-> kcenon::common::Result<container_module::value_container> {
				tasks_processed.fetch_add(1, std::memory_order_relaxed);
				return kcenon::common::ok(container_module::value_container{});
			});

		queue->start();
		pool.start();

		const int num_tasks = 10000;

		// Enqueue tasks
		for (int i = 0; i < num_tasks; ++i) {
			auto result = task_builder("benchmark.empty")
							  .queue("benchmark")
							  .build();
			if (result.is_ok()) {
				queue->enqueue(result.unwrap());
			}
		}

		BenchmarkTimer timer;

		// Wait for all tasks to complete
		while (tasks_processed.load(std::memory_order_relaxed) < num_tasks) {
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
			if (timer.elapsed_seconds() > 60.0) {
				std::cout << "  Timeout waiting for tasks\n";
				break;
			}
		}

		double duration = timer.elapsed_seconds();
		int processed = tasks_processed.load(std::memory_order_relaxed);

		pool.stop();
		queue->stop();

		print_throughput("Single Worker (empty task)", processed, duration);

		double ops_per_sec = processed / duration;
		std::cout << "  Target: > 10,000 tasks/sec\n";
		std::cout << "  Status: " << (ops_per_sec > 10000 ? "PASS" : "BELOW TARGET") << "\n";
	}

	// Benchmark 2: Multi-worker scalability
	{
		std::cout << "\n--- Benchmark 2: Multi-Worker Scalability ---\n";

		std::vector<size_t> worker_counts = {1, 2, 4, 8};
		std::vector<double> throughputs;

		for (size_t num_workers : worker_counts) {
			reset_counters();

			auto queue = std::make_shared<task_queue>();
			auto backend = std::make_shared<memory_result_backend>();

			worker_config config;
			config.concurrency = num_workers;
			config.queues = {"benchmark"};
			config.poll_interval = std::chrono::milliseconds{1};

			worker_pool pool(queue, backend, config);

			pool.register_handler("benchmark.scale",
				[](const task& /* t */, task_context& /* ctx */)
					-> kcenon::common::Result<container_module::value_container> {
					tasks_processed.fetch_add(1, std::memory_order_relaxed);
					return kcenon::common::ok(container_module::value_container{});
				});

			queue->start();
			pool.start();

			const int num_tasks = 10000;

			// Enqueue tasks
			for (int i = 0; i < num_tasks; ++i) {
				auto result = task_builder("benchmark.scale")
								  .queue("benchmark")
								  .build();
				if (result.is_ok()) {
					queue->enqueue(result.unwrap());
				}
			}

			BenchmarkTimer timer;

			while (tasks_processed.load(std::memory_order_relaxed) < num_tasks) {
				std::this_thread::sleep_for(std::chrono::milliseconds{10});
				if (timer.elapsed_seconds() > 60.0) {
					break;
				}
			}

			double duration = timer.elapsed_seconds();
			int processed = tasks_processed.load(std::memory_order_relaxed);
			double throughput = processed / duration;
			throughputs.push_back(throughput);

			pool.stop();
			queue->stop();
		}

		std::cout << "\n=== Multi-Worker Scalability ===\n";
		std::cout << std::fixed << std::setprecision(0);
		for (size_t i = 0; i < worker_counts.size(); ++i) {
			double scaling = i > 0 ? throughputs[i] / throughputs[0] : 1.0;
			std::cout << "  " << worker_counts[i] << " worker(s): "
					  << throughputs[i] << " tasks/sec (scaling: "
					  << std::setprecision(2) << scaling << "x)\n";
		}
	}

	// Benchmark 3: Handler dispatch latency
	{
		std::cout << "\n--- Benchmark 3: Handler Dispatch Latency ---\n";

		reset_counters();

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();

		worker_config config;
		config.concurrency = 1;
		config.queues = {"benchmark"};
		config.poll_interval = std::chrono::milliseconds{1};

		worker_pool pool(queue, backend, config);

		std::vector<int64_t> latencies;
		std::mutex latency_mutex;

		pool.register_handler("benchmark.latency",
			[](const task& /* t */, task_context& /* ctx */)
				-> kcenon::common::Result<container_module::value_container> {
				// Use task metadata to get enqueue time from payload if available
				// For now, just measure handler execution
				tasks_processed.fetch_add(1, std::memory_order_relaxed);

				return kcenon::common::ok(container_module::value_container{});
			});

		queue->start();
		pool.start();

		const int num_tasks = 1000;

		// Measure round-trip time
		BenchmarkResults results("Handler Dispatch");

		for (int i = 0; i < num_tasks; ++i) {
			auto start = std::chrono::steady_clock::now();

			auto result = task_builder("benchmark.latency")
							  .queue("benchmark")
							  .build();
			if (result.is_ok()) {
				auto task_result = queue->enqueue(result.unwrap());
				if (task_result.is_ok()) {
					std::string task_id = task_result.unwrap();

					// Wait for completion
					auto wait_result = backend->wait_for_result(
						task_id, std::chrono::seconds{5});

					if (wait_result.is_ok()) {
						auto end = std::chrono::steady_clock::now();
						double latency_ms =
							std::chrono::duration<double, std::milli>(end - start).count();
						results.add_duration(latency_ms);
					}
				}
			}
		}

		pool.stop();
		queue->stop();

		results.print();
	}

	// Benchmark 4: Mixed workload
	{
		std::cout << "\n--- Benchmark 4: Mixed Workload ---\n";

		reset_counters();

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();

		worker_config config;
		config.concurrency = 4;
		config.queues = {"fast", "slow", "default"};
		config.poll_interval = std::chrono::milliseconds{1};

		worker_pool pool(queue, backend, config);

		std::atomic<int> fast_count{0};
		std::atomic<int> slow_count{0};

		// Fast handler (no-op)
		pool.register_handler("benchmark.fast",
			[&fast_count](const task& /* t */, task_context& /* ctx */)
				-> kcenon::common::Result<container_module::value_container> {
				fast_count.fetch_add(1, std::memory_order_relaxed);
				tasks_processed.fetch_add(1, std::memory_order_relaxed);
				return kcenon::common::ok(container_module::value_container{});
			});

		// Slow handler (simulated work)
		pool.register_handler("benchmark.slow",
			[&slow_count](const task& /* t */, task_context& /* ctx */)
				-> kcenon::common::Result<container_module::value_container> {
				std::this_thread::sleep_for(std::chrono::microseconds{100});
				slow_count.fetch_add(1, std::memory_order_relaxed);
				tasks_processed.fetch_add(1, std::memory_order_relaxed);
				return kcenon::common::ok(container_module::value_container{});
			});

		queue->start();
		pool.start();

		const int fast_tasks = 5000;
		const int slow_tasks = 1000;
		const int total_tasks = fast_tasks + slow_tasks;

		// Enqueue mixed workload
		for (int i = 0; i < total_tasks; ++i) {
			bool is_fast = (i % 6) != 0;  // 5:1 ratio

			auto result = task_builder(is_fast ? "benchmark.fast" : "benchmark.slow")
							  .queue(is_fast ? "fast" : "slow")
							  .build();
			if (result.is_ok()) {
				queue->enqueue(result.unwrap());
			}
		}

		BenchmarkTimer timer;

		while (tasks_processed.load(std::memory_order_relaxed) < total_tasks) {
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
			if (timer.elapsed_seconds() > 120.0) {
				std::cout << "  Timeout waiting for tasks\n";
				break;
			}
		}

		double duration = timer.elapsed_seconds();

		pool.stop();
		queue->stop();

		std::cout << "\n=== Mixed Workload Results ===\n";
		std::cout << "  Fast tasks completed: " << fast_count.load() << "\n";
		std::cout << "  Slow tasks completed: " << slow_count.load() << "\n";
		std::cout << std::fixed << std::setprecision(3);
		std::cout << "  Total duration: " << duration << " seconds\n";
		std::cout << "  Overall throughput: " << static_cast<int>(total_tasks / duration)
				  << " tasks/sec\n";
	}

	// Benchmark 5: Handler registration overhead
	{
		std::cout << "\n--- Benchmark 5: Handler Registration ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();

		worker_config config;
		config.concurrency = 1;
		config.queues = {"benchmark"};

		worker_pool pool(queue, backend, config);

		const int num_handlers = 1000;

		BenchmarkTimer reg_timer;
		for (int i = 0; i < num_handlers; ++i) {
			std::string name = "handler." + std::to_string(i);
			pool.register_handler(name,
				[](const task& /* t */, task_context& /* ctx */)
					-> kcenon::common::Result<container_module::value_container> {
					return kcenon::common::ok(container_module::value_container{});
				});
		}
		double reg_duration = reg_timer.elapsed_seconds();

		std::cout << "\n=== Handler Registration ===\n";
		std::cout << std::fixed << std::setprecision(6);
		std::cout << "  Registered " << num_handlers << " handlers in "
				  << reg_duration << " seconds\n";
		std::cout << "  Average: "
				  << (reg_duration / num_handlers) * 1000000 << " microseconds/handler\n";
	}

	std::cout << "\n========================================\n";
	std::cout << "Worker Throughput Benchmarks Complete\n";
	std::cout << "========================================\n\n";

	return 0;
}
