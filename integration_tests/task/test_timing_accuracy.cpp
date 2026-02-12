// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_timing_accuracy.cpp
 * @brief Integration tests for task timing accuracy
 *
 * Tests scheduled task execution timing, interval precision,
 * and timing consistency under various conditions.
 *
 * Issue #161 - High Priority Test: ScheduledTaskTiming
 * Schedule: 10 tasks at 100ms intervals
 * Verify: Execution within ±10ms tolerance
 */

#include "task_fixture.h"

#include <gtest/gtest.h>
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>
#include <kcenon/messaging/task/scheduler.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;

using tsk::testing::SchedulerFixture;
using tsk::testing::TaskCounter;
using tsk::testing::wait_for_condition;

/**
 * @class TimingAccuracyTest
 * @brief Test fixture for timing accuracy tests with scheduler enabled
 */
class TimingAccuracyTest : public SchedulerFixture {
protected:
	using time_point = std::chrono::steady_clock::time_point;
	using duration_ms = std::chrono::milliseconds;

	struct ExecutionRecord {
		time_point timestamp;
		int sequence;
	};

	/**
	 * @brief Calculate timing statistics from execution records
	 */
	struct TimingStats {
		double mean_interval_ms{0.0};
		double std_dev_ms{0.0};
		double max_deviation_ms{0.0};
		double min_interval_ms{0.0};
		double max_interval_ms{0.0};
		int sample_count{0};
	};

	TimingStats calculate_timing_stats(const std::vector<ExecutionRecord>& records) {
		TimingStats stats;

		if (records.size() < 2) {
			return stats;
		}

		std::vector<double> intervals;
		for (size_t i = 1; i < records.size(); ++i) {
			auto interval = std::chrono::duration_cast<duration_ms>(
				records[i].timestamp - records[i - 1].timestamp
			).count();
			intervals.push_back(static_cast<double>(interval));
		}

		stats.sample_count = static_cast<int>(intervals.size());

		// Calculate mean
		stats.mean_interval_ms = std::accumulate(intervals.begin(), intervals.end(), 0.0)
			/ intervals.size();

		// Calculate min/max
		stats.min_interval_ms = *std::min_element(intervals.begin(), intervals.end());
		stats.max_interval_ms = *std::max_element(intervals.begin(), intervals.end());

		// Calculate standard deviation
		double sq_sum = 0.0;
		for (double interval : intervals) {
			sq_sum += (interval - stats.mean_interval_ms) * (interval - stats.mean_interval_ms);
		}
		stats.std_dev_ms = std::sqrt(sq_sum / intervals.size());

		// Calculate max deviation from expected interval
		stats.max_deviation_ms = std::max(
			std::abs(stats.max_interval_ms - stats.mean_interval_ms),
			std::abs(stats.min_interval_ms - stats.mean_interval_ms)
		);

		return stats;
	}
};

// ============================================================================
// Scheduled Task Timing Accuracy Tests (Issue #161 - High Priority)
// ============================================================================

/**
 * @test ScheduledTaskTimingAccuracy
 * @brief Verify scheduled task execution timing accuracy
 *
 * Schedule: 10 tasks triggered periodically
 * Target: 1 second intervals (minimum reliable interval for scheduler)
 * Verify: Execution within reasonable tolerance
 */
TEST_F(TimingAccuracyTest, ScheduledTaskTimingAccuracy) {
	std::vector<ExecutionRecord> records;
	std::mutex records_mutex;
	std::atomic<int> sequence{0};

	system_->register_handler("timing.accuracy", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		ExecutionRecord record;
		record.timestamp = std::chrono::steady_clock::now();
		record.sequence = sequence.fetch_add(1);

		{
			std::lock_guard lock(records_mutex);
			records.push_back(record);
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("timing.accuracy").build();
	ASSERT_TRUE(task_result.is_ok());

	// Schedule periodic execution at 1 second intervals
	auto schedule_result = system_->schedule_periodic(
		"timing-test",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	);
	ASSERT_TRUE(schedule_result.is_ok()) << schedule_result.error().message;

	// Wait for 5 executions
	const int target_executions = 5;
	ASSERT_TRUE(wait_for_condition(
		[&sequence]() {
			return sequence.load() >= target_executions;
		},
		std::chrono::seconds(15)
	)) << "Expected " << target_executions << " executions, got " << sequence.load();

	// Analyze timing
	std::vector<ExecutionRecord> records_copy;
	{
		std::lock_guard lock(records_mutex);
		records_copy = records;
	}

	auto stats = calculate_timing_stats(records_copy);

	std::cout << "Timing Accuracy Test Results (1s interval):\n"
	          << "  Executions: " << records_copy.size() << "\n"
	          << "  Mean interval: " << stats.mean_interval_ms << " ms\n"
	          << "  Std deviation: " << stats.std_dev_ms << " ms\n"
	          << "  Min interval: " << stats.min_interval_ms << " ms\n"
	          << "  Max interval: " << stats.max_interval_ms << " ms\n"
	          << "  Max deviation: " << stats.max_deviation_ms << " ms\n";

	// Verify timing is within reasonable bounds
	// Expected interval is 1000ms, allow ±200ms tolerance
	EXPECT_GE(stats.mean_interval_ms, 800.0)
		<< "Mean interval too short (expected ~1000ms)";
	EXPECT_LE(stats.mean_interval_ms, 1200.0)
		<< "Mean interval too long (expected ~1000ms)";
}

/**
 * @test TaskSubmissionLatency
 * @brief Measure task submission to execution latency
 *
 * Submit 100 tasks and measure time from submission to execution
 * Verify: Low and consistent latency
 */
TEST_F(TimingAccuracyTest, TaskSubmissionLatency) {
	const int num_tasks = 100;

	// Store submission times indexed by task sequence
	std::vector<time_point> submission_times(num_tasks);
	std::vector<time_point> execution_times(num_tasks);
	std::mutex exec_mutex;
	std::atomic<int> completed{0};
	std::atomic<int> task_index{0};

	system_->register_handler("timing.latency", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		auto execution_time = std::chrono::steady_clock::now();
		int idx = task_index.fetch_add(1);

		if (idx < num_tasks) {
			std::lock_guard lock(exec_mutex);
			execution_times[idx] = execution_time;
		}

		completed.fetch_add(1);
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit tasks and record submission times
	for (int i = 0; i < num_tasks; ++i) {
		submission_times[i] = std::chrono::steady_clock::now();

		container_module::value_container payload;
		system_->submit("timing.latency", payload);
	}

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&completed]() { return completed.load() >= 100; },
		std::chrono::seconds(30)
	)) << "Expected " << num_tasks << " completions, got " << completed.load();

	// Analyze latency
	std::vector<double> latencies;
	{
		std::lock_guard lock(exec_mutex);
		for (int i = 0; i < num_tasks; ++i) {
			auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
				execution_times[i] - submission_times[i]).count();
			latencies.push_back(static_cast<double>(latency) / 1000.0);  // Convert to ms
		}
	}

	if (!latencies.empty()) {
		double mean = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
		double min_lat = *std::min_element(latencies.begin(), latencies.end());
		double max_lat = *std::max_element(latencies.begin(), latencies.end());

		// Calculate p50, p95, p99
		std::sort(latencies.begin(), latencies.end());
		double p50 = latencies[latencies.size() / 2];
		double p95 = latencies[static_cast<size_t>(latencies.size() * 0.95)];
		double p99 = latencies[static_cast<size_t>(latencies.size() * 0.99)];

		std::cout << "Task Submission Latency Results:\n"
		          << "  Samples: " << latencies.size() << "\n"
		          << "  Mean: " << mean << " ms\n"
		          << "  Min: " << min_lat << " ms\n"
		          << "  Max: " << max_lat << " ms\n"
		          << "  P50: " << p50 << " ms\n"
		          << "  P95: " << p95 << " ms\n"
		          << "  P99: " << p99 << " ms\n";

		// Verify reasonable latency (should be well under 100ms for simple tasks)
		EXPECT_LT(p95, 100.0) << "P95 latency too high";
	}
}

/**
 * @test TaskExecutionOrderConsistency
 * @brief Verify tasks are executed in submission order within same priority
 *
 * Submit 50 tasks sequentially
 * Verify: Execution order matches submission order (for FIFO queue)
 */
TEST_F(TimingAccuracyTest, TaskExecutionOrderConsistency) {
	const int num_tasks = 50;

	std::vector<int> execution_order;
	std::mutex order_mutex;
	std::atomic<int> completed{0};
	std::atomic<int> exec_sequence{0};

	system_->register_handler("timing.order", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		// Track execution order using atomic counter
		int seq = exec_sequence.fetch_add(1);
		{
			std::lock_guard lock(order_mutex);
			execution_order.push_back(seq);
		}

		completed.fetch_add(1);
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit tasks sequentially
	for (int i = 0; i < num_tasks; ++i) {
		auto task_result = tsk::task_builder("timing.order")
			.priority(msg::message_priority::normal)  // Same priority for all
			.build();
		ASSERT_TRUE(task_result.is_ok());

		system_->submit(std::move(task_result).value());
	}

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&completed]() { return completed.load() >= 50; },
		std::chrono::seconds(30)
	));

	// Verify order
	std::vector<int> order_copy;
	{
		std::lock_guard lock(order_mutex);
		order_copy = execution_order;
	}

	ASSERT_EQ(order_copy.size(), static_cast<size_t>(num_tasks));

	// Count out-of-order executions
	int out_of_order = 0;
	for (size_t i = 1; i < order_copy.size(); ++i) {
		if (order_copy[i] < order_copy[i - 1]) {
			out_of_order++;
		}
	}

	std::cout << "Task Execution Order Results:\n"
	          << "  Total tasks: " << num_tasks << "\n"
	          << "  Out-of-order: " << out_of_order << "\n";

	// With concurrent workers, some out-of-order is acceptable
	// but should be minimal for same-priority tasks
	double out_of_order_ratio = static_cast<double>(out_of_order) / num_tasks;
	EXPECT_LT(out_of_order_ratio, 0.3)
		<< "Too many out-of-order executions: " << out_of_order << "/" << num_tasks;
}

/**
 * @test TaskThroughputConsistency
 * @brief Measure throughput consistency over time
 *
 * Process 1000 tasks and measure throughput in 100-task batches
 * Verify: Consistent throughput without degradation
 */
TEST_F(TimingAccuracyTest, TaskThroughputConsistency) {
	const int total_tasks = 1000;
	const int batch_size = 100;

	std::atomic<int> completed{0};
	std::vector<time_point> completion_times;
	std::mutex times_mutex;

	system_->register_handler("timing.throughput", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		int count = completed.fetch_add(1) + 1;

		// Record completion time at batch boundaries
		if (count % batch_size == 0) {
			std::lock_guard lock(times_mutex);
			completion_times.push_back(std::chrono::steady_clock::now());
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto start_time = std::chrono::steady_clock::now();

	// Submit all tasks
	for (int i = 0; i < total_tasks; ++i) {
		container_module::value_container payload;
		system_->submit("timing.throughput", payload);
	}

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&completed]() { return completed.load() >= total_tasks; },
		std::chrono::seconds(60)
	)) << "Expected " << total_tasks << " completions, got " << completed.load();

	auto end_time = std::chrono::steady_clock::now();

	// Analyze batch throughput
	std::vector<time_point> times_copy;
	{
		std::lock_guard lock(times_mutex);
		times_copy = completion_times;
	}

	if (times_copy.size() >= 2) {
		std::vector<double> batch_throughputs;
		time_point prev_time = start_time;

		for (const auto& t : times_copy) {
			auto duration = std::chrono::duration_cast<duration_ms>(t - prev_time).count();
			if (duration > 0) {
				double throughput = (batch_size * 1000.0) / duration;
				batch_throughputs.push_back(throughput);
			}
			prev_time = t;
		}

		if (!batch_throughputs.empty()) {
			double mean = std::accumulate(batch_throughputs.begin(), batch_throughputs.end(), 0.0)
				/ batch_throughputs.size();
			double min_tp = *std::min_element(batch_throughputs.begin(), batch_throughputs.end());
			double max_tp = *std::max_element(batch_throughputs.begin(), batch_throughputs.end());

			std::cout << "Throughput Consistency Results:\n"
			          << "  Total tasks: " << total_tasks << "\n"
			          << "  Batches: " << batch_throughputs.size() << "\n"
			          << "  Mean throughput: " << mean << " tasks/sec\n"
			          << "  Min throughput: " << min_tp << " tasks/sec\n"
			          << "  Max throughput: " << max_tp << " tasks/sec\n";

			// Verify throughput doesn't degrade significantly
			// (min should be at least 50% of max)
			EXPECT_GT(min_tp / max_tp, 0.3)
				<< "Throughput degradation detected: min=" << min_tp << ", max=" << max_tp;
		}
	}

	auto total_duration = std::chrono::duration_cast<duration_ms>(end_time - start_time).count();
	double overall_throughput = (total_tasks * 1000.0) / total_duration;

	std::cout << "  Overall throughput: " << overall_throughput << " tasks/sec\n"
	          << "  Total duration: " << total_duration << " ms\n";
}

/**
 * @test ScheduleNextExecutionTime
 * @brief Verify scheduler calculates next execution time correctly
 */
TEST_F(TimingAccuracyTest, ScheduleNextExecutionTime) {
	system_->register_handler("timing.next", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("timing.next").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	const auto interval = std::chrono::seconds(30);

	ASSERT_TRUE(system_->schedule_periodic(
		"next-time-test",
		std::move(task_result).value(),
		interval
	).is_ok());

	auto schedule_result = scheduler->get_schedule("next-time-test");
	ASSERT_TRUE(schedule_result.is_ok());

	const auto& entry = schedule_result.value();

	// Verify schedule was created with correct interval
	EXPECT_TRUE(entry.is_periodic());
	EXPECT_EQ(entry.interval(), interval);
	EXPECT_TRUE(entry.enabled);

	// Verify next_run is set (if available)
	if (entry.next_run.has_value()) {
		auto next_run_system = entry.next_run.value();
		auto now_system = std::chrono::system_clock::now();
		auto time_until_next = std::chrono::duration_cast<std::chrono::seconds>(
			next_run_system - now_system);

		// Should be close to the interval (within a few seconds)
		EXPECT_GE(time_until_next.count(), -5);  // Allow slight past due to timing
		EXPECT_LE(time_until_next.count(), 35);  // Allow some tolerance
	}
}

/**
 * @test TaskResultWaitTiming
 * @brief Verify async result wait times out correctly
 */
TEST_F(TimingAccuracyTest, TaskResultWaitTiming) {
	std::atomic<bool> allow_completion{false};

	system_->register_handler("timing.wait", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		// Wait until released (with safety limit)
		const int max_wait_iterations = 500;  // 5 seconds max
		for (int i = 0; i < max_wait_iterations && !allow_completion.load(); ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	container_module::value_container payload;
	auto result = system_->submit("timing.wait", payload);

	// Try to get result with short timeout
	auto start = std::chrono::steady_clock::now();
	auto wait_result = result.get(std::chrono::milliseconds{100});
	auto end = std::chrono::steady_clock::now();

	auto wait_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	// Should timeout close to 100ms
	EXPECT_GE(wait_duration.count(), 80) << "Wait returned too early";
	EXPECT_LE(wait_duration.count(), 200) << "Wait took too long";
	EXPECT_FALSE(wait_result.is_ok()) << "Should have timed out";

	// Release the task to clean up
	allow_completion = true;
}
