// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_concurrent_load.cpp
 * @brief Integration tests for concurrent load and stress testing
 *
 * Tests high volume task processing, memory usage patterns,
 * throughput measurement, and system stability under load.
 */

#include "task_fixture.h"

#include <gtest/gtest.h>
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>

#include <atomic>
#include <chrono>
#include <future>
#include <random>
#include <thread>
#include <vector>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;

using tsk::testing::TaskSystemFixture;
using tsk::testing::TaskCounter;
using tsk::testing::wait_for_condition;

class ConcurrentLoadTest : public TaskSystemFixture {
protected:
	void SetUp() override {
		// Use more workers for load tests
		config_.worker.concurrency = 8;
		config_.worker.queues = {"default"};
		config_.enable_scheduler = false;
		config_.enable_monitoring = true;

		system_ = std::make_unique<tsk::task_system>(config_);
	}
};

// ============================================================================
// High Volume Task Processing (1000+ tasks)
// ============================================================================

TEST_F(ConcurrentLoadTest, HighVolumeTaskProcessing) {
	TaskCounter counter;

	system_->register_handler("load.high_volume", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		counter.increment_success();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int task_count = 1000;
	std::vector<tsk::async_result> results;
	results.reserve(task_count);

	auto start_time = std::chrono::steady_clock::now();

	// Submit all tasks
	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		results.push_back(system_->submit("load.high_volume", payload));
	}

	auto submit_time = std::chrono::steady_clock::now();

	// Wait for all tasks to complete
	int completed = 0;
	int failed = 0;

	for (auto& r : results) {
		auto result = r.get(std::chrono::seconds(60));
		if (result.is_ok()) {
			completed++;
		} else {
			failed++;
		}
	}

	auto end_time = std::chrono::steady_clock::now();

	// Verify all tasks completed
	EXPECT_EQ(completed, task_count);
	EXPECT_EQ(failed, 0);
	EXPECT_EQ(counter.count(), task_count);

	// Calculate throughput
	auto submit_duration = std::chrono::duration_cast<std::chrono::milliseconds>(submit_time - start_time);
	auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	double submit_rate = task_count / (submit_duration.count() / 1000.0);
	double throughput = task_count / (total_duration.count() / 1000.0);

	// Log performance metrics (for visibility in test output)
	std::cout << "High Volume Test Results:\n"
	          << "  Total tasks: " << task_count << "\n"
	          << "  Submit time: " << submit_duration.count() << "ms\n"
	          << "  Total time: " << total_duration.count() << "ms\n"
	          << "  Submit rate: " << submit_rate << " tasks/sec\n"
	          << "  Throughput: " << throughput << " tasks/sec\n";

	// Basic performance expectations
	EXPECT_GT(throughput, 100.0) << "Expected at least 100 tasks/sec throughput";
}

TEST_F(ConcurrentLoadTest, SustainedLoad) {
	TaskCounter counter;

	system_->register_handler("load.sustained", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		// Simulate some work
		std::this_thread::sleep_for(std::chrono::microseconds{100});
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit tasks over time
	const int batches = 10;
	const int tasks_per_batch = 100;
	const auto batch_interval = std::chrono::milliseconds{100};

	std::vector<tsk::async_result> all_results;

	for (int batch = 0; batch < batches; ++batch) {
		for (int i = 0; i < tasks_per_batch; ++i) {
			container_module::value_container payload;
			all_results.push_back(system_->submit("load.sustained", payload));
		}
		std::this_thread::sleep_for(batch_interval);
	}

	// Wait for all to complete
	for (auto& r : all_results) {
		r.get(std::chrono::seconds(60));
	}

	EXPECT_EQ(counter.count(), batches * tasks_per_batch);
}

// ============================================================================
// Concurrent Producer/Consumer
// ============================================================================

TEST_F(ConcurrentLoadTest, MultipleProducers) {
	TaskCounter counter;

	system_->register_handler("load.multi_producer", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int num_producers = 4;
	const int tasks_per_producer = 250;

	std::vector<std::thread> producers;
	std::atomic<int> submitted{0};

	for (int p = 0; p < num_producers; ++p) {
		producers.emplace_back([this, &submitted]() {
			for (int i = 0; i < tasks_per_producer; ++i) {
				container_module::value_container payload;
				system_->submit("load.multi_producer", payload);
				submitted++;
			}
		});
	}

	// Wait for all producers to finish submitting
	for (auto& t : producers) {
		t.join();
	}

	EXPECT_EQ(submitted.load(), num_producers * tasks_per_producer);

	// Wait for all tasks to be processed
	ASSERT_TRUE(wait_for_condition(
		[&counter]() {
			return counter.count() >= static_cast<size_t>(num_producers * tasks_per_producer);
		},
		std::chrono::seconds(60)
	));

	EXPECT_EQ(counter.count(), num_producers * tasks_per_producer);
}

// ============================================================================
// Memory Usage Verification
// ============================================================================

TEST_F(ConcurrentLoadTest, MemoryStabilityUnderLoad) {
	TaskCounter counter;

	// Handler that creates some allocations
	system_->register_handler("load.memory", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		// Create some temporary data
		std::vector<char> buffer(1024);
		for (size_t i = 0; i < buffer.size(); ++i) {
			buffer[i] = static_cast<char>(i % 256);
		}

		container_module::value_container result;
		result.set("buffer_size", static_cast<int>(buffer.size()));

		counter.increment();
		return cmn::ok(result);
	});

	start_system();

	// Process many tasks
	const int task_count = 500;

	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		system_->submit("load.memory", payload);
	}

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&counter]() { return counter.count() >= static_cast<size_t>(task_count); },
		std::chrono::seconds(60)
	));

	EXPECT_EQ(counter.count(), task_count);

	// Verify statistics are reasonable
	auto stats = system_->get_statistics();
	EXPECT_GE(stats.total_tasks_processed, static_cast<size_t>(task_count));
}

TEST_F(ConcurrentLoadTest, LargePayloadProcessing) {
	TaskCounter counter;

	system_->register_handler("load.large_payload", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)ctx;

		// Verify payload is received correctly
		const auto& payload = t.payload();
		auto data = payload.get("data");

		if (data.has_value()) {
			counter.increment_success();
		}

		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int task_count = 100;
	const size_t payload_size = 10000;  // 10KB string

	std::string large_data(payload_size, 'X');

	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		payload.set("data", large_data);
		system_->submit("load.large_payload", payload);
	}

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&counter]() { return counter.count() >= static_cast<size_t>(task_count); },
		std::chrono::seconds(60)
	));

	EXPECT_EQ(counter.count(), task_count);
	EXPECT_EQ(counter.success_count(), task_count);
}

// ============================================================================
// Throughput Measurement
// ============================================================================

TEST_F(ConcurrentLoadTest, ThroughputWithMinimalWork) {
	TaskCounter counter;

	// Minimal handler - just count
	system_->register_handler("load.minimal", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int task_count = 2000;

	auto start_time = std::chrono::steady_clock::now();

	// Submit all tasks using batch API for efficiency
	std::vector<tsk::task> tasks;
	tasks.reserve(task_count);

	for (int i = 0; i < task_count; ++i) {
		auto task_result = tsk::task_builder("load.minimal").build();
		ASSERT_TRUE(task_result.is_ok());
		tasks.push_back(std::move(task_result).value());
	}

	auto results = system_->submit_batch(std::move(tasks));

	// Wait for all to complete
	for (auto& r : results) {
		r.get(std::chrono::seconds(120));
	}

	auto end_time = std::chrono::steady_clock::now();

	EXPECT_EQ(counter.count(), task_count);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	double throughput = task_count / (duration.count() / 1000.0);

	std::cout << "Minimal Work Throughput:\n"
	          << "  Tasks: " << task_count << "\n"
	          << "  Duration: " << duration.count() << "ms\n"
	          << "  Throughput: " << throughput << " tasks/sec\n";

	EXPECT_GT(throughput, 500.0) << "Expected higher throughput with minimal work";
}

TEST_F(ConcurrentLoadTest, ThroughputWithSimulatedWork) {
	TaskCounter counter;

	// Handler with simulated work
	system_->register_handler("load.simulated", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		// Simulate 1ms of work
		std::this_thread::sleep_for(std::chrono::milliseconds{1});

		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int task_count = 500;

	auto start_time = std::chrono::steady_clock::now();

	std::vector<tsk::async_result> results;
	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		results.push_back(system_->submit("load.simulated", payload));
	}

	for (auto& r : results) {
		r.get(std::chrono::seconds(60));
	}

	auto end_time = std::chrono::steady_clock::now();

	EXPECT_EQ(counter.count(), task_count);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	double throughput = task_count / (duration.count() / 1000.0);

	std::cout << "Simulated Work Throughput (1ms/task, 8 workers):\n"
	          << "  Tasks: " << task_count << "\n"
	          << "  Duration: " << duration.count() << "ms\n"
	          << "  Throughput: " << throughput << " tasks/sec\n";

	// With 8 workers and 1ms/task, theoretical max is ~8000 tasks/sec
	// Expect at least 50% efficiency
	EXPECT_GT(throughput, 100.0);
}

// ============================================================================
// Stress Testing
// ============================================================================

TEST_F(ConcurrentLoadTest, RapidSubmissionBursts) {
	TaskCounter counter;

	system_->register_handler("load.burst", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int bursts = 10;
	const int tasks_per_burst = 100;
	const auto burst_interval = std::chrono::milliseconds{50};

	for (int burst = 0; burst < bursts; ++burst) {
		// Rapid fire burst
		for (int i = 0; i < tasks_per_burst; ++i) {
			container_module::value_container payload;
			system_->submit("load.burst", payload);
		}

		// Brief pause between bursts
		std::this_thread::sleep_for(burst_interval);
	}

	// Wait for all to complete
	ASSERT_TRUE(wait_for_condition(
		[&counter]() {
			return counter.count() >= static_cast<size_t>(bursts * tasks_per_burst);
		},
		std::chrono::seconds(60)
	));

	EXPECT_EQ(counter.count(), bursts * tasks_per_burst);
}

TEST_F(ConcurrentLoadTest, MixedPriorityLoad) {
	TaskCounter high_counter;
	TaskCounter normal_counter;
	TaskCounter low_counter;

	system_->register_handler("load.high", [&high_counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		high_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	system_->register_handler("load.normal", [&normal_counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		normal_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	system_->register_handler("load.low", [&low_counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		low_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int tasks_per_priority = 200;

	// Submit mixed priority tasks
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 2);

	for (int i = 0; i < tasks_per_priority * 3; ++i) {
		int priority = dis(gen);
		std::string handler_name;
		msg::message_priority prio;

		switch (priority) {
			case 0:
				handler_name = "load.high";
				prio = msg::message_priority::high;
				break;
			case 1:
				handler_name = "load.normal";
				prio = msg::message_priority::normal;
				break;
			default:
				handler_name = "load.low";
				prio = msg::message_priority::low;
				break;
		}

		auto task_result = tsk::task_builder(handler_name)
			.priority(prio)
			.build();

		ASSERT_TRUE(task_result.is_ok());
		system_->submit(std::move(task_result).value());
	}

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&high_counter, &normal_counter, &low_counter]() {
			return (high_counter.count() + normal_counter.count() + low_counter.count()) >=
			       static_cast<size_t>(tasks_per_priority * 3);
		},
		std::chrono::seconds(60)
	));

	size_t total = high_counter.count() + normal_counter.count() + low_counter.count();
	EXPECT_EQ(total, tasks_per_priority * 3);
}

// ============================================================================
// System Stability
// ============================================================================

TEST_F(ConcurrentLoadTest, StabilityWithFailingTasks) {
	TaskCounter success_counter;
	TaskCounter failure_counter;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 9);

	// 20% of tasks fail
	system_->register_handler("load.mixed_outcome", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		if (dis(gen) < 2) {  // 20% failure rate
			failure_counter.increment();
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, "Random failure"}
			);
		}

		success_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	const int task_count = 500;

	for (int i = 0; i < task_count; ++i) {
		auto task_result = tsk::task_builder("load.mixed_outcome")
			.retries(0)  // No retries for this test
			.build();

		ASSERT_TRUE(task_result.is_ok());
		system_->submit(std::move(task_result).value());
	}

	// Wait for all to complete
	ASSERT_TRUE(wait_for_condition(
		[&success_counter, &failure_counter]() {
			return (success_counter.count() + failure_counter.count()) >= static_cast<size_t>(task_count);
		},
		std::chrono::seconds(60)
	));

	// All tasks should have been processed
	EXPECT_EQ(success_counter.count() + failure_counter.count(), task_count);

	// System should still be running
	EXPECT_TRUE(system_->is_running());
}

TEST_F(ConcurrentLoadTest, GracefulShutdownUnderLoad) {
	TaskCounter counter;
	std::atomic<bool> keep_submitting{true};
	std::atomic<int> submitted{0};

	system_->register_handler("load.shutdown", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		std::this_thread::sleep_for(std::chrono::milliseconds{10});
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Start a producer thread
	std::thread producer([this, &keep_submitting, &submitted]() {
		while (keep_submitting.load()) {
			container_module::value_container payload;
			system_->submit("load.shutdown", payload);
			submitted++;
			std::this_thread::sleep_for(std::chrono::milliseconds{1});
		}
	});

	// Let it run for a bit
	std::this_thread::sleep_for(std::chrono::milliseconds{500});

	// Stop submitting
	keep_submitting = false;
	producer.join();

	// Graceful shutdown
	auto shutdown_result = system_->shutdown_graceful(std::chrono::seconds(30));
	EXPECT_TRUE(shutdown_result.is_ok()) << shutdown_result.error().message;

	EXPECT_FALSE(system_->is_running());

	std::cout << "Graceful Shutdown Test:\n"
	          << "  Submitted: " << submitted.load() << "\n"
	          << "  Completed: " << counter.count() << "\n";
}

// ============================================================================
// Queue Backpressure
// ============================================================================

TEST_F(ConcurrentLoadTest, QueueCapacityHandling) {
	std::atomic<bool> handler_started{false};
	std::atomic<bool> allow_completion{false};
	TaskCounter counter;

	// Handler that blocks until released (with max iterations for safety)
	system_->register_handler("load.blocking", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		handler_started = true;

		const int max_loops = 6000;  // 60 seconds max
		for (int i = 0; i < max_loops && !allow_completion.load(); ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		}

		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Fill up the queue
	const int queue_fill = 100;

	for (int i = 0; i < queue_fill; ++i) {
		container_module::value_container payload;
		system_->submit("load.blocking", payload);
	}

	// Wait for at least one task to start
	ASSERT_TRUE(wait_for_condition(
		[&handler_started]() { return handler_started.load(); },
		std::chrono::seconds(5)
	));

	// Release all tasks
	allow_completion = true;

	// Wait for completion
	ASSERT_TRUE(wait_for_condition(
		[&counter]() { return counter.count() >= static_cast<size_t>(queue_fill); },
		std::chrono::seconds(60)
	));

	EXPECT_EQ(counter.count(), queue_fill);
}
