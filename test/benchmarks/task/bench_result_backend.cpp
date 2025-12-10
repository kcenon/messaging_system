// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file bench_result_backend.cpp
 * @brief Performance benchmarks for result_backend
 *
 * Measures:
 * - Store throughput
 * - Get throughput
 * - wait_for_result latency
 *
 * Performance Targets:
 * - Result store: > 50,000 ops/sec
 */

#include "../bench_common.h"
#include <kcenon/messaging/task/memory_result_backend.h>
#include <kcenon/messaging/task/task.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace kcenon::messaging::task;
using namespace kcenon::messaging::benchmark;

namespace {

std::string generate_task_id(int i) {
	return "task-" + std::to_string(i);
}

container_module::value_container create_test_result() {
	container_module::value_container result;
	result.add("status", "completed");
	result.add("value", 42);
	return result;
}

}  // namespace

int main() {
	std::cout << "\n========================================\n";
	std::cout << "Result Backend Benchmarks\n";
	std::cout << "========================================\n";

	// Benchmark 1: Store state throughput
	{
		std::cout << "\n--- Benchmark 1: Store State Throughput ---\n";

		memory_result_backend backend;
		const int operations = 100000;

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			backend.store_state(generate_task_id(i), task_state::pending);
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Store State", operations, duration);
	}

	// Benchmark 2: Store result throughput
	{
		std::cout << "\n--- Benchmark 2: Store Result Throughput ---\n";

		memory_result_backend backend;
		const int operations = 100000;
		auto result = create_test_result();

		// Pre-create task entries
		for (int i = 0; i < operations; ++i) {
			backend.store_state(generate_task_id(i), task_state::running);
		}

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			backend.store_result(generate_task_id(i), result);
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Store Result", operations, duration);

		double ops_per_sec = operations / duration;
		std::cout << "  Target: > 50,000 ops/sec\n";
		std::cout << "  Status: " << (ops_per_sec > 50000 ? "PASS" : "BELOW TARGET") << "\n";
	}

	// Benchmark 3: Get state throughput
	{
		std::cout << "\n--- Benchmark 3: Get State Throughput ---\n";

		memory_result_backend backend;
		const int operations = 100000;

		// Pre-populate
		for (int i = 0; i < operations; ++i) {
			backend.store_state(generate_task_id(i), task_state::succeeded);
		}

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			auto state = backend.get_state(generate_task_id(i));
			(void)state;  // Prevent optimization
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Get State", operations, duration);
	}

	// Benchmark 4: Get result throughput
	{
		std::cout << "\n--- Benchmark 4: Get Result Throughput ---\n";

		memory_result_backend backend;
		const int operations = 100000;
		auto result = create_test_result();

		// Pre-populate
		for (int i = 0; i < operations; ++i) {
			backend.store_state(generate_task_id(i), task_state::succeeded);
			backend.store_result(generate_task_id(i), result);
		}

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			auto res = backend.get_result(generate_task_id(i));
			(void)res;
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Get Result", operations, duration);
	}

	// Benchmark 5: Progress update throughput
	{
		std::cout << "\n--- Benchmark 5: Progress Update Throughput ---\n";

		memory_result_backend backend;
		const int operations = 100000;

		// Pre-create entries
		for (int i = 0; i < 1000; ++i) {
			backend.store_state(generate_task_id(i), task_state::running);
		}

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			std::string task_id = generate_task_id(i % 1000);
			double progress = static_cast<double>(i % 100) / 100.0;
			backend.store_progress(task_id, progress, "Processing...");
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Store Progress", operations, duration);
	}

	// Benchmark 6: wait_for_result latency
	{
		std::cout << "\n--- Benchmark 6: Wait For Result Latency ---\n";

		memory_result_backend backend;
		const int operations = 1000;
		auto result = create_test_result();

		BenchmarkResults results("Wait For Result");

		for (int i = 0; i < operations; ++i) {
			std::string task_id = generate_task_id(i);

			// Store state as pending first
			backend.store_state(task_id, task_state::pending);

			// Start a thread that will complete the task after a short delay
			std::thread completer([&backend, task_id, &result]() {
				std::this_thread::sleep_for(std::chrono::microseconds{100});
				backend.store_state(task_id, task_state::succeeded);
				backend.store_result(task_id, result);
			});

			BenchmarkTimer timer;
			auto wait_result = backend.wait_for_result(task_id, std::chrono::seconds{1});
			double latency_ms = timer.elapsed<std::chrono::nanoseconds>() / 1'000'000.0;

			if (wait_result.is_ok()) {
				results.add_duration(latency_ms);
			}

			completer.join();
		}

		results.print();
	}

	// Benchmark 7: Concurrent read/write
	{
		std::cout << "\n--- Benchmark 7: Concurrent Read/Write ---\n";

		memory_result_backend backend;
		const int operations = 10000;
		auto result = create_test_result();
		std::atomic<int> writes_done{0};
		std::atomic<int> reads_done{0};

		// Pre-create entries
		for (int i = 0; i < 1000; ++i) {
			backend.store_state(generate_task_id(i), task_state::running);
		}

		BenchmarkTimer timer;

		std::thread writer([&]() {
			for (int i = 0; i < operations; ++i) {
				std::string task_id = generate_task_id(i % 1000);
				backend.store_result(task_id, result);
				writes_done.fetch_add(1, std::memory_order_relaxed);
			}
		});

		std::thread reader([&]() {
			for (int i = 0; i < operations; ++i) {
				std::string task_id = generate_task_id(i % 1000);
				auto res = backend.get_result(task_id);
				(void)res;
				reads_done.fetch_add(1, std::memory_order_relaxed);
			}
		});

		writer.join();
		reader.join();

		double duration = timer.elapsed_seconds();
		int total_ops = writes_done.load() + reads_done.load();

		std::cout << "\n=== Concurrent Read/Write ===\n";
		std::cout << std::fixed << std::setprecision(3);
		std::cout << "  Writes: " << writes_done.load() << "\n";
		std::cout << "  Reads:  " << reads_done.load() << "\n";
		std::cout << "  Duration: " << duration << " seconds\n";
		std::cout << "  Combined throughput: " << static_cast<int>(total_ops / duration)
				  << " ops/sec\n";
	}

	// Benchmark 8: Multi-threaded writes
	{
		std::cout << "\n--- Benchmark 8: Multi-Threaded Writes ---\n";

		memory_result_backend backend;
		const int threads = 4;
		const int ops_per_thread = 25000;
		auto result = create_test_result();
		std::atomic<int> total_ops{0};

		BenchmarkTimer timer;

		std::vector<std::thread> writers;
		for (int t = 0; t < threads; ++t) {
			writers.emplace_back([&, t]() {
				for (int i = 0; i < ops_per_thread; ++i) {
					int task_num = t * ops_per_thread + i;
					std::string task_id = generate_task_id(task_num);
					backend.store_state(task_id, task_state::succeeded);
					backend.store_result(task_id, result);
					total_ops.fetch_add(1, std::memory_order_relaxed);
				}
			});
		}

		for (auto& w : writers) {
			w.join();
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Multi-Threaded Writes (4 threads)", total_ops.load(), duration);
	}

	// Benchmark 9: Cleanup performance
	{
		std::cout << "\n--- Benchmark 9: Cleanup Performance ---\n";

		memory_result_backend backend;
		const int num_tasks = 50000;
		auto result = create_test_result();

		// Populate with tasks
		for (int i = 0; i < num_tasks; ++i) {
			backend.store_state(generate_task_id(i), task_state::succeeded);
			backend.store_result(generate_task_id(i), result);
		}

		std::cout << "  Initial size: " << backend.size() << "\n";

		BenchmarkTimer timer;
		backend.cleanup_expired(std::chrono::milliseconds{0});  // Remove all
		double duration = timer.elapsed_seconds();

		std::cout << "\n=== Cleanup Performance ===\n";
		std::cout << std::fixed << std::setprecision(6);
		std::cout << "  Cleaned " << num_tasks << " entries in " << duration << " seconds\n";
		std::cout << "  Rate: " << static_cast<int>(num_tasks / duration) << " entries/sec\n";
		std::cout << "  Final size: " << backend.size() << "\n";
	}

	// Benchmark 10: Exists check performance
	{
		std::cout << "\n--- Benchmark 10: Exists Check Performance ---\n";

		memory_result_backend backend;
		const int operations = 100000;

		// Pre-populate half
		for (int i = 0; i < operations / 2; ++i) {
			backend.store_state(generate_task_id(i), task_state::pending);
		}

		int found = 0;
		int not_found = 0;

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			if (backend.exists(generate_task_id(i))) {
				++found;
			} else {
				++not_found;
			}
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Exists Check", operations, duration);

		std::cout << "  Found: " << found << ", Not found: " << not_found << "\n";
	}

	std::cout << "\n========================================\n";
	std::cout << "Result Backend Benchmarks Complete\n";
	std::cout << "========================================\n\n";

	return 0;
}
