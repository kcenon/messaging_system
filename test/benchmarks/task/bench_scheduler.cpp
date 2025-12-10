// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file bench_scheduler.cpp
 * @brief Performance benchmarks for task_scheduler
 *
 * Measures:
 * - Schedule evaluation overhead
 * - Large schedule management
 * - Cron parsing performance
 */

#include "../bench_common.h"
#include <kcenon/messaging/task/scheduler.h>
#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <kcenon/messaging/task/cron_parser.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

using namespace kcenon::messaging::task;
using namespace kcenon::messaging::benchmark;

namespace {

task create_test_task(const std::string& name = "benchmark.scheduled") {
	auto result = task_builder(name)
					  .queue("scheduled")
					  .build();
	if (result.is_ok()) {
		return result.unwrap();
	}
	return task(name);
}

}  // namespace

int main() {
	std::cout << "\n========================================\n";
	std::cout << "Scheduler Benchmarks\n";
	std::cout << "========================================\n";

	// Benchmark 1: Schedule addition performance
	{
		std::cout << "\n--- Benchmark 1: Schedule Addition ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);
		const int num_schedules = 1000;

		BenchmarkTimer timer;

		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			auto t = create_test_task("benchmark.scheduled." + std::to_string(i));
			scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
		}

		double duration = timer.elapsed_seconds();

		std::cout << "\n=== Schedule Addition ===\n";
		std::cout << std::fixed << std::setprecision(6);
		std::cout << "  Added " << num_schedules << " schedules in "
				  << duration << " seconds\n";
		std::cout << "  Rate: " << static_cast<int>(num_schedules / duration)
				  << " schedules/sec\n";
		std::cout << "  Average: " << (duration / num_schedules) * 1000000
				  << " microseconds/schedule\n";
	}

	// Benchmark 2: Schedule lookup performance
	{
		std::cout << "\n--- Benchmark 2: Schedule Lookup ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);
		const int num_schedules = 1000;

		// Populate schedules
		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			auto t = create_test_task();
			scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
		}

		const int lookups = 10000;
		int found = 0;

		BenchmarkTimer timer;

		for (int i = 0; i < lookups; ++i) {
			std::string name = "schedule-" + std::to_string(i % num_schedules);
			auto result = scheduler.get_schedule(name);
			if (result.is_ok()) {
				++found;
			}
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Schedule Lookup", lookups, duration);

		std::cout << "  Found: " << found << "/" << lookups << "\n";
	}

	// Benchmark 3: Schedule removal performance
	{
		std::cout << "\n--- Benchmark 3: Schedule Removal ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);
		const int num_schedules = 1000;

		// Populate schedules
		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			auto t = create_test_task();
			scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
		}

		BenchmarkTimer timer;

		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			scheduler.remove(name);
		}

		double duration = timer.elapsed_seconds();

		std::cout << "\n=== Schedule Removal ===\n";
		std::cout << std::fixed << std::setprecision(6);
		std::cout << "  Removed " << num_schedules << " schedules in "
				  << duration << " seconds\n";
		std::cout << "  Rate: " << static_cast<int>(num_schedules / duration)
				  << " schedules/sec\n";
	}

	// Benchmark 4: Cron parsing performance
	{
		std::cout << "\n--- Benchmark 4: Cron Parsing ---\n";

		std::vector<std::string> cron_expressions = {
			"* * * * *",           // Every minute
			"0 * * * *",           // Every hour
			"0 0 * * *",           // Daily at midnight
			"0 0 * * 0",           // Weekly on Sunday
			"0 0 1 * *",           // Monthly on 1st
			"30 4 1 1 *",          // Yearly on Jan 1 at 4:30
			"*/5 * * * *",         // Every 5 minutes
			"0 */2 * * *",         // Every 2 hours
			"0 9-17 * * 1-5",      // Business hours weekdays
			"0,30 * * * *"         // Every half hour
		};

		const int iterations = 10000;

		BenchmarkResults results("Cron Parse");

		for (int i = 0; i < iterations; ++i) {
			const auto& expr = cron_expressions[i % cron_expressions.size()];

			BenchmarkTimer timer;
			auto result = cron_parser::parse(expr);
			double duration_ms = timer.elapsed<std::chrono::nanoseconds>() / 1'000'000.0;

			if (result.is_ok()) {
				results.add_duration(duration_ms);
			}
		}

		results.print();
	}

	// Benchmark 5: Next run time calculation
	{
		std::cout << "\n--- Benchmark 5: Next Run Time Calculation ---\n";

		std::vector<std::string> cron_expressions = {
			"* * * * *",
			"0 * * * *",
			"0 0 * * *",
			"*/5 * * * *",
			"0 9-17 * * 1-5"
		};

		const int iterations = 10000;

		for (const auto& expr : cron_expressions) {
			auto parser_result = cron_parser::parse(expr);
			if (!parser_result.is_ok()) {
				continue;
			}

			auto parser = parser_result.unwrap();
			auto now = std::chrono::system_clock::now();

			BenchmarkTimer timer;
			for (int i = 0; i < iterations; ++i) {
				auto next = parser.next_run(now);
				(void)next;
			}
			double duration = timer.elapsed_seconds();

			std::cout << "  \"" << expr << "\": " << static_cast<int>(iterations / duration)
					  << " calculations/sec\n";
		}
	}

	// Benchmark 6: Enable/disable performance
	{
		std::cout << "\n--- Benchmark 6: Enable/Disable Performance ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);
		const int num_schedules = 100;

		// Populate schedules
		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			auto t = create_test_task();
			scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
		}

		const int operations = 10000;

		BenchmarkTimer timer;

		for (int i = 0; i < operations; ++i) {
			std::string name = "schedule-" + std::to_string(i % num_schedules);
			if (i % 2 == 0) {
				scheduler.disable(name);
			} else {
				scheduler.enable(name);
			}
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Enable/Disable", operations, duration);
	}

	// Benchmark 7: Interval update performance
	{
		std::cout << "\n--- Benchmark 7: Interval Update ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);
		const int num_schedules = 100;

		// Populate schedules
		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			auto t = create_test_task();
			scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
		}

		const int updates = 10000;

		BenchmarkTimer timer;

		for (int i = 0; i < updates; ++i) {
			std::string name = "schedule-" + std::to_string(i % num_schedules);
			scheduler.update_interval(name, std::chrono::seconds{30 + (i % 60)});
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Interval Update", updates, duration);
	}

	// Benchmark 8: List schedules performance
	{
		std::cout << "\n--- Benchmark 8: List Schedules ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);

		std::vector<size_t> schedule_counts = {10, 100, 500, 1000};

		for (size_t count : schedule_counts) {
			// Clear and repopulate
			task_scheduler new_scheduler(client);

			for (size_t i = 0; i < count; ++i) {
				std::string name = "schedule-" + std::to_string(i);
				auto t = create_test_task();
				new_scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
			}

			const int iterations = 1000;

			BenchmarkTimer timer;
			for (int i = 0; i < iterations; ++i) {
				auto schedules = new_scheduler.list_schedules();
				(void)schedules;
			}
			double duration = timer.elapsed_seconds();

			std::cout << "  " << count << " schedules: "
					  << static_cast<int>(iterations / duration)
					  << " list ops/sec\n";
		}
	}

	// Benchmark 9: Cron vs periodic overhead
	{
		std::cout << "\n--- Benchmark 9: Cron vs Periodic Overhead ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		const int num_schedules = 500;

		// Periodic schedules
		{
			task_scheduler scheduler(client);

			BenchmarkTimer timer;
			for (int i = 0; i < num_schedules; ++i) {
				std::string name = "periodic-" + std::to_string(i);
				auto t = create_test_task();
				scheduler.add_periodic(name, std::move(t), std::chrono::seconds{60});
			}
			double periodic_duration = timer.elapsed_seconds();

			std::cout << "\n=== Periodic vs Cron ===\n";
			std::cout << std::fixed << std::setprecision(6);
			std::cout << "  Periodic add (" << num_schedules << "): "
					  << periodic_duration << " seconds\n";
		}

		// Cron schedules
		{
			task_scheduler scheduler(client);

			BenchmarkTimer timer;
			for (int i = 0; i < num_schedules; ++i) {
				std::string name = "cron-" + std::to_string(i);
				auto t = create_test_task();
				scheduler.add_cron(name, std::move(t), "*/5 * * * *");
			}
			double cron_duration = timer.elapsed_seconds();

			std::cout << "  Cron add (" << num_schedules << "): "
					  << cron_duration << " seconds\n";
		}
	}

	// Benchmark 10: Trigger now performance
	{
		std::cout << "\n--- Benchmark 10: Trigger Now Performance ---\n";

		auto queue = std::make_shared<task_queue>();
		auto backend = std::make_shared<memory_result_backend>();
		auto client = std::make_shared<task_client>(queue, backend);

		task_scheduler scheduler(client);
		const int num_schedules = 100;

		// Populate schedules
		for (int i = 0; i < num_schedules; ++i) {
			std::string name = "schedule-" + std::to_string(i);
			auto t = create_test_task();
			scheduler.add_periodic(name, std::move(t), std::chrono::seconds{3600});
		}

		queue->start();

		const int triggers = 1000;

		BenchmarkTimer timer;

		for (int i = 0; i < triggers; ++i) {
			std::string name = "schedule-" + std::to_string(i % num_schedules);
			scheduler.trigger_now(name);
		}

		double duration = timer.elapsed_seconds();
		print_throughput("Trigger Now", triggers, duration);

		queue->stop();
	}

	std::cout << "\n========================================\n";
	std::cout << "Scheduler Benchmarks Complete\n";
	std::cout << "========================================\n\n";

	return 0;
}
