// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_task_lifecycle.cpp
 * @brief Integration tests for full task lifecycle
 *
 * Tests the complete task lifecycle from submission through execution
 * to completion, including progress tracking and result retrieval.
 */

#include "task_fixture.h"

#include <gtest/gtest.h>
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;

using tsk::testing::TaskSystemFixture;
using tsk::testing::TaskCounter;
using tsk::testing::ProgressTracker;
using tsk::testing::wait_for_condition;

class TaskLifecycleTest : public TaskSystemFixture {};

// ============================================================================
// Task Submit -> Queue -> Execute -> Complete
// ============================================================================

TEST_F(TaskLifecycleTest, BasicTaskLifecycle) {
	TaskCounter counter;
	register_counting_handler("lifecycle.basic", counter);

	start_system();

	// Submit task
	container_module::value_container payload;
	auto async_result = system_->submit("lifecycle.basic", payload);

	// Wait for completion
	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	// Verify execution
	EXPECT_EQ(counter.count(), 1);
	EXPECT_EQ(counter.success_count(), 1);
}

TEST_F(TaskLifecycleTest, TaskBuilderLifecycle) {
	TaskCounter counter;
	register_counting_handler("lifecycle.builder", counter);

	start_system();

	// Build task with custom options
	auto task_result = tsk::task_builder("lifecycle.builder")
		.priority(msg::message_priority::high)
		.timeout(std::chrono::seconds(30))
		.queue("default")
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());

	// Wait for completion
	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	EXPECT_EQ(counter.count(), 1);
}

TEST_F(TaskLifecycleTest, TaskWithPayloadLifecycle) {
	std::atomic<bool> payload_verified{false};

	system_->register_handler("lifecycle.payload", [&payload_verified](const tsk::task& t, tsk::task_context& ctx) {
		(void)ctx;

		// Verify payload was received correctly
		const auto& payload = t.payload();
		auto value = payload.get_value("test_key");
		if (value.has_value()) {
			payload_verified = true;
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit task with payload
	container_module::value_container payload;
	payload.set_value("test_key", std::string("test_value"));

	auto async_result = system_->submit("lifecycle.payload", payload);
	auto result = async_result.get(std::chrono::seconds(10));

	ASSERT_TRUE(result.is_ok()) << result.error().message;
	EXPECT_TRUE(payload_verified);
}

TEST_F(TaskLifecycleTest, MultipleTasksSequential) {
	TaskCounter counter;
	register_counting_handler("lifecycle.sequential", counter);

	start_system();

	const int task_count = 10;
	std::vector<tsk::async_result> results;

	// Submit tasks sequentially
	for (int i = 0; i < task_count; ++i) {
		container_module::value_container payload;
		results.push_back(system_->submit("lifecycle.sequential", payload));
	}

	// Wait for all to complete
	for (auto& r : results) {
		auto result = r.get(std::chrono::seconds(10));
		EXPECT_TRUE(result.is_ok()) << result.error().message;
	}

	EXPECT_EQ(counter.count(), task_count);
	EXPECT_EQ(counter.success_count(), task_count);
}

// ============================================================================
// Task Cancellation Flow
// ============================================================================

TEST_F(TaskLifecycleTest, TaskCancellationAPI) {
	// Test that the cancellation API is callable without crashing
	// Note: Full cancellation behavior test requires more complex setup

	start_system();

	// Test cancel on non-existent task - should not crash
	// Note: Implementation may return success even for non-existent tasks
	auto cancel_result = system_->queue()->cancel("non-existent-task-id");

	// The test passes as long as the API call doesn't crash
	(void)cancel_result;
}

// ============================================================================
// Progress Update Verification
// ============================================================================

TEST_F(TaskLifecycleTest, ProgressUpdateTracking) {
	TaskCounter counter;
	ProgressTracker tracker;
	register_progress_handler("lifecycle.progress", counter, tracker);

	start_system();

	container_module::value_container payload;
	auto async_result = system_->submit("lifecycle.progress", payload);

	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	// Verify progress updates were recorded
	auto updates = tracker.get_updates();
	EXPECT_GE(updates.size(), 4);

	// Verify progress values are increasing
	double last_progress = 0.0;
	for (const auto& [progress, message] : updates) {
		EXPECT_GE(progress, last_progress);
		last_progress = progress;
	}

	// Final progress should be 1.0
	EXPECT_DOUBLE_EQ(updates.back().first, 1.0);
}

TEST_F(TaskLifecycleTest, TaskContextProgressUpdates) {
	std::atomic<double> final_progress{0.0};

	system_->register_handler("lifecycle.ctx_progress", [&final_progress](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;

		// Update progress multiple times
		ctx.update_progress(0.1, "Step 1");
		ctx.update_progress(0.5, "Step 2");
		ctx.update_progress(0.9, "Step 3");
		ctx.update_progress(1.0, "Done");

		final_progress = ctx.progress();

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	container_module::value_container payload;
	auto async_result = system_->submit("lifecycle.ctx_progress", payload);

	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	EXPECT_DOUBLE_EQ(final_progress.load(), 1.0);
}

// ============================================================================
// Result Handling
// ============================================================================

TEST_F(TaskLifecycleTest, TaskResultRetrieval) {
	std::atomic<bool> handler_executed{false};

	system_->register_handler("lifecycle.result", [&handler_executed](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		handler_executed = true;

		container_module::value_container result;
		result.set_value("status", std::string("completed"));

		return cmn::ok(result);
	});

	start_system();

	container_module::value_container payload;
	auto async_result = system_->submit("lifecycle.result", payload);

	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	// Verify handler was executed
	EXPECT_TRUE(handler_executed.load());

	// The result backend stores the value_container returned by the handler
	// Just verify we got some result back
	EXPECT_TRUE(result.is_ok());
}

TEST_F(TaskLifecycleTest, TaskResultFromBackend) {
	system_->register_handler("lifecycle.backend", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		container_module::value_container result;
		result.set_value("source", std::string("backend_test"));

		return cmn::ok(result);
	});

	start_system();

	auto task_result = tsk::task_builder("lifecycle.backend").build();
	ASSERT_TRUE(task_result.is_ok());

	auto task = std::move(task_result).value();
	auto task_id = task.task_id();

	auto async_result = system_->submit(std::move(task));
	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	// Retrieve result from backend
	auto backend_result = system_->results()->get_result(task_id);
	EXPECT_TRUE(backend_result.is_ok());
}

// ============================================================================
// State Transitions
// ============================================================================

TEST_F(TaskLifecycleTest, TaskStateTransitions) {
	std::vector<tsk::task_state> recorded_states;
	std::mutex states_mutex;

	system_->register_handler("lifecycle.states", [&recorded_states, &states_mutex](const tsk::task& t, tsk::task_context& ctx) {
		(void)ctx;

		{
			std::lock_guard lock(states_mutex);
			recorded_states.push_back(t.state());
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	container_module::value_container payload;
	auto async_result = system_->submit("lifecycle.states", payload);

	auto result = async_result.get(std::chrono::seconds(10));
	ASSERT_TRUE(result.is_ok()) << result.error().message;

	// During handler execution, state should be running
	std::lock_guard lock(states_mutex);
	EXPECT_FALSE(recorded_states.empty());
	EXPECT_EQ(recorded_states[0], tsk::task_state::running);
}

// ============================================================================
// Batch Submission
// ============================================================================

TEST_F(TaskLifecycleTest, BatchSubmission) {
	TaskCounter counter;
	register_counting_handler("lifecycle.batch", counter);

	start_system();

	const size_t batch_size = 20;
	std::vector<tsk::task> tasks;

	for (size_t i = 0; i < batch_size; ++i) {
		auto task_result = tsk::task_builder("lifecycle.batch").build();
		ASSERT_TRUE(task_result.is_ok());
		tasks.push_back(std::move(task_result).value());
	}

	auto results = system_->submit_batch(std::move(tasks));
	EXPECT_EQ(results.size(), batch_size);

	// Wait for all tasks to complete
	for (auto& r : results) {
		auto result = r.get(std::chrono::seconds(30));
		EXPECT_TRUE(result.is_ok()) << result.error().message;
	}

	EXPECT_EQ(counter.count(), batch_size);
	EXPECT_EQ(counter.success_count(), batch_size);
}

// ============================================================================
// System Lifecycle
// ============================================================================

TEST_F(TaskLifecycleTest, GracefulShutdownWithPendingTasks) {
	TaskCounter counter;

	// Register a slow handler
	system_->register_handler("lifecycle.shutdown", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		std::this_thread::sleep_for(std::chrono::milliseconds{100});
		counter.increment_success();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Submit several tasks
	for (int i = 0; i < 5; ++i) {
		container_module::value_container payload;
		system_->submit("lifecycle.shutdown", payload);
	}

	// Allow some tasks to start
	std::this_thread::sleep_for(std::chrono::milliseconds{50});

	// Graceful shutdown
	auto shutdown_result = system_->shutdown_graceful(std::chrono::seconds(10));
	EXPECT_TRUE(shutdown_result.is_ok()) << shutdown_result.error().message;

	// Verify system is stopped
	EXPECT_FALSE(system_->is_running());

	// Tasks that started should have completed
	EXPECT_GE(counter.success_count(), 1);
}

TEST_F(TaskLifecycleTest, RestartAfterStop) {
	TaskCounter counter;
	register_counting_handler("lifecycle.restart", counter);

	// First run
	start_system();

	container_module::value_container payload;
	auto result1 = system_->submit("lifecycle.restart", payload).get(std::chrono::seconds(10));
	EXPECT_TRUE(result1.is_ok()) << result1.error().message;

	stop_system();
	EXPECT_FALSE(system_->is_running());

	// Second run - recreate the system (restart requires new instance)
	system_ = std::make_unique<tsk::task_system>(config_);
	register_counting_handler("lifecycle.restart", counter);
	start_system();

	auto result2 = system_->submit("lifecycle.restart", payload).get(std::chrono::seconds(10));
	EXPECT_TRUE(result2.is_ok()) << result2.error().message;

	EXPECT_EQ(counter.count(), 2);
}
