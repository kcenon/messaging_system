// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_worker_scenarios.cpp
 * @brief Integration tests for worker scenarios
 *
 * Tests multiple workers, concurrent execution, graceful shutdown,
 * and handler matching scenarios.
 */

#include "task_fixture.h"

#include <gtest/gtest.h>
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>
#include <kcenon/messaging/task/worker_pool.h>

#include <atomic>
#include <chrono>
#include <set>
#include <thread>
#include <vector>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;

using tsk::testing::TaskSystemFixture;
using tsk::testing::TaskCounter;
using tsk::testing::wait_for_condition;

class WorkerScenariosTest : public TaskSystemFixture {
protected:
	void SetUp() override {
		// Use more workers for concurrency tests
		config_.worker.concurrency = 4;
		config_.worker.queues = {"default", "high-priority"};
		config_.enable_scheduler = false;
		config_.enable_monitoring = false;

		system_ = std::make_unique<tsk::task_system>(config_);
	}
};

// ============================================================================
// Multiple Workers Concurrent Execution
// ============================================================================

TEST_F(WorkerScenariosTest, ConcurrentTaskExecution) {
	TaskCounter counter;
	std::atomic<int> concurrent_count{0};
	std::atomic<int> max_concurrent{0};

	system_->register_handler("worker.concurrent", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		// Track concurrent executions
		int current = ++concurrent_count;

		// Update max concurrent count
		int expected = max_concurrent.load();
		while (current > expected && !max_concurrent.compare_exchange_weak(expected, current)) {
			// retry
		}

		// Simulate work
		std::this_thread::sleep_for(std::chrono::milliseconds{50});

		--concurrent_count;
		counter.increment();
		counter.increment_success();

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit many tasks simultaneously
	const int task_count = 20;
	std::vector<tsk::async_result> results;

	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		results.push_back(system_->submit("worker.concurrent", payload));
	}

	// Wait for all tasks
	for (auto& r : results) {
		auto result = r.get(std::chrono::seconds(30));
		EXPECT_TRUE(result.is_ok()) << result.error().message;
	}

	// Verify all tasks completed
	EXPECT_EQ(counter.count(), task_count);

	// With 4 workers and 50ms tasks, we should see concurrent execution
	EXPECT_GT(max_concurrent.load(), 1) << "Expected concurrent execution with multiple workers";
}

TEST_F(WorkerScenariosTest, WorkerCountVerification) {
	start_system();

	// Verify worker count matches configuration
	EXPECT_EQ(system_->total_workers(), 4);

	// Initially no workers should be active
	// (they're waiting for tasks)
	EXPECT_GE(system_->active_workers(), 0);
}

TEST_F(WorkerScenariosTest, WorkDistribution) {
	std::mutex thread_ids_mutex;
	std::set<std::thread::id> thread_ids;
	TaskCounter counter;

	system_->register_handler("worker.distribution", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		{
			std::lock_guard lock(thread_ids_mutex);
			thread_ids.insert(std::this_thread::get_id());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds{20});
		counter.increment();

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit enough tasks to use all workers
	const int task_count = 40;
	std::vector<tsk::async_result> results;

	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		results.push_back(system_->submit("worker.distribution", payload));
	}

	for (auto& r : results) {
		r.get(std::chrono::seconds(30));
	}

	// Multiple worker threads should have been used
	std::lock_guard lock(thread_ids_mutex);
	EXPECT_GT(thread_ids.size(), 1) << "Expected work to be distributed across multiple threads";
}

// ============================================================================
// Worker Graceful Shutdown
// ============================================================================

TEST_F(WorkerScenariosTest, GracefulShutdownWaitsForActiveTasks) {
	std::atomic<int> started{0};
	std::atomic<int> completed{0};

	system_->register_handler("worker.graceful", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		started++;
		std::this_thread::sleep_for(std::chrono::milliseconds{200});
		completed++;

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit tasks
	for (int i = 0; i < 4; ++i) {
		container_module::value_container payload;
		system_->submit("worker.graceful", payload);
	}

	// Wait for tasks to start
	ASSERT_TRUE(wait_for_condition([&]() { return started.load() >= 2; }, std::chrono::seconds(5)));

	// Graceful shutdown should wait for running tasks
	auto shutdown_result = system_->shutdown_graceful(std::chrono::seconds(10));
	EXPECT_TRUE(shutdown_result.is_ok());

	// Started tasks should have completed
	EXPECT_GE(completed.load(), started.load());
}

TEST_F(WorkerScenariosTest, ImmediateStopInterruptsTasks) {
	std::atomic<int> started{0};
	std::atomic<int> completed{0};

	system_->register_handler("worker.immediate_stop", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		started++;
		std::this_thread::sleep_for(std::chrono::seconds(5));  // Long task
		completed++;

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit a long-running task
	container_module::value_container payload;
	system_->submit("worker.immediate_stop", payload);

	// Wait for task to start
	ASSERT_TRUE(wait_for_condition([&]() { return started.load() >= 1; }, std::chrono::seconds(5)));

	// Immediate stop
	auto stop_result = system_->stop();
	EXPECT_TRUE(stop_result.is_ok());

	// System should be stopped
	EXPECT_FALSE(system_->is_running());
}

// ============================================================================
// Handler Matching
// ============================================================================

TEST_F(WorkerScenariosTest, ExactHandlerMatching) {
	TaskCounter handler1_counter;
	TaskCounter handler2_counter;

	system_->register_handler("handler.one", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		handler1_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	system_->register_handler("handler.two", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		handler2_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit tasks for different handlers
	container_module::value_container payload;
	system_->submit("handler.one", payload).get(std::chrono::seconds(10));
	system_->submit("handler.one", payload).get(std::chrono::seconds(10));
	system_->submit("handler.two", payload).get(std::chrono::seconds(10));

	EXPECT_EQ(handler1_counter.count(), 2);
	EXPECT_EQ(handler2_counter.count(), 1);
}

TEST_F(WorkerScenariosTest, UnregisteredHandlerFailsTask) {
	start_system();

	// Submit task for non-existent handler
	container_module::value_container payload;
	auto async_result = system_->submit("nonexistent.handler", payload);

	auto result = async_result.get(std::chrono::seconds(10));

	// Should fail because no handler is registered
	EXPECT_FALSE(result.is_ok());
}

TEST_F(WorkerScenariosTest, HandlerRegistrationAfterStart) {
	start_system();

	// Register handler after system start
	TaskCounter counter;
	system_->register_handler("worker.late_register", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	// Should work
	container_module::value_container payload;
	auto result = system_->submit("worker.late_register", payload).get(std::chrono::seconds(10));

	EXPECT_TRUE(result.is_ok()) << result.error().message;
	EXPECT_EQ(counter.count(), 1);
}

TEST_F(WorkerScenariosTest, HandlerUnregistration) {
	TaskCounter counter;

	system_->register_handler("worker.unregister", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// First task should succeed
	container_module::value_container payload;
	auto result1 = system_->submit("worker.unregister", payload).get(std::chrono::seconds(10));
	EXPECT_TRUE(result1.is_ok());

	// Unregister handler
	EXPECT_TRUE(system_->unregister_handler("worker.unregister"));

	// Second task should fail (no handler)
	auto result2 = system_->submit("worker.unregister", payload).get(std::chrono::seconds(10));
	EXPECT_FALSE(result2.is_ok());
}

TEST_F(WorkerScenariosTest, ListRegisteredHandlers) {
	system_->register_handler("handler.alpha", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	system_->register_handler("handler.beta", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	auto handlers = system_->workers().list_handlers();

	EXPECT_EQ(handlers.size(), 2);
	EXPECT_TRUE(system_->workers().has_handler("handler.alpha"));
	EXPECT_TRUE(system_->workers().has_handler("handler.beta"));
	EXPECT_FALSE(system_->workers().has_handler("handler.gamma"));
}

// ============================================================================
// Worker Statistics
// ============================================================================

TEST_F(WorkerScenariosTest, StatisticsCollection) {
	TaskCounter counter;
	register_counting_handler("worker.stats", counter);

	start_system();

	// Submit and complete tasks
	const int task_count = 10;
	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		system_->submit("worker.stats", payload).get(std::chrono::seconds(10));
	}

	// Check statistics
	auto stats = system_->get_statistics();
	EXPECT_GE(stats.total_tasks_processed, static_cast<size_t>(task_count));
	EXPECT_GE(stats.total_tasks_succeeded, static_cast<size_t>(task_count));
	EXPECT_EQ(stats.total_tasks_failed, 0);
}

TEST_F(WorkerScenariosTest, StatisticsReset) {
	TaskCounter counter;
	register_counting_handler("worker.stats_reset", counter);

	start_system();

	// Execute some tasks
	for (int i = 0; i < 5; ++i) {
		container_module::value_container payload;
		system_->submit("worker.stats_reset", payload).get(std::chrono::seconds(10));
	}

	// Reset statistics
	system_->workers().reset_statistics();

	auto stats = system_->get_statistics();
	EXPECT_EQ(stats.total_tasks_processed, 0);
}

// ============================================================================
// Multiple Queues
// ============================================================================

TEST_F(WorkerScenariosTest, MultipleQueueProcessing) {
	TaskCounter default_counter;
	TaskCounter priority_counter;

	system_->register_handler("queue.default", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		default_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	system_->register_handler("queue.priority", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		priority_counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit tasks to different queues
	auto default_task = tsk::task_builder("queue.default")
		.queue("default")
		.build();
	ASSERT_TRUE(default_task.is_ok());

	auto priority_task = tsk::task_builder("queue.priority")
		.queue("high-priority")
		.build();
	ASSERT_TRUE(priority_task.is_ok());

	system_->submit(std::move(default_task).value()).get(std::chrono::seconds(10));
	system_->submit(std::move(priority_task).value()).get(std::chrono::seconds(10));

	EXPECT_EQ(default_counter.count(), 1);
	EXPECT_EQ(priority_counter.count(), 1);
}

// ============================================================================
// Worker Pool State
// ============================================================================

TEST_F(WorkerScenariosTest, WorkerPoolLifecycle) {
	auto& workers = system_->workers();

	// Before start
	EXPECT_FALSE(workers.is_running());
	EXPECT_EQ(workers.total_workers(), 0);

	start_system();

	// After start
	EXPECT_TRUE(workers.is_running());
	EXPECT_EQ(workers.total_workers(), 4);

	stop_system();

	// After stop
	EXPECT_FALSE(workers.is_running());
}

TEST_F(WorkerScenariosTest, ActiveWorkerCount) {
	std::atomic<bool> task_started{false};
	std::atomic<bool> task_can_complete{false};

	system_->register_handler("worker.active", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		task_started = true;

		// Wait until allowed to complete
		while (!task_can_complete.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit a task
	container_module::value_container payload;
	auto async_result = system_->submit("worker.active", payload);

	// Wait for task to start
	ASSERT_TRUE(wait_for_condition([&]() { return task_started.load(); }, std::chrono::seconds(5)));

	// At least one worker should be active
	EXPECT_GE(system_->active_workers(), 1);

	// Allow task to complete
	task_can_complete = true;

	auto result = async_result.get(std::chrono::seconds(10));
	EXPECT_TRUE(result.is_ok());
}
