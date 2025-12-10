// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#pragma once

#include <gtest/gtest.h>
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace kcenon::messaging::task::testing {

namespace cmn = kcenon::common;

/**
 * @brief Wait for a condition with timeout using condition variable
 *
 * Uses std::condition_variable for efficient waiting instead of polling with sleep_for.
 * The predicate is checked periodically to avoid missed wakeups when the condition
 * is satisfied by external events that don't notify the condition variable.
 */
template<typename Predicate>
bool wait_for_condition(
	Predicate&& pred,
	std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
) {
	if (pred()) {
		return true;
	}

	std::mutex mtx;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock(mtx);

	auto deadline = std::chrono::steady_clock::now() + timeout;

	// Use wait_until with periodic predicate checks for conditions
	// that may be satisfied without explicit notification
	while (!pred()) {
		auto remaining = deadline - std::chrono::steady_clock::now();
		if (remaining <= std::chrono::milliseconds::zero()) {
			return false;
		}

		// Wait for a short interval or until timeout, then recheck the predicate
		auto wait_time = std::min(remaining, std::chrono::milliseconds{50});
		cv.wait_for(lock, wait_time);
	}

	return true;
}

/**
 * @brief Counter for tracking task executions
 */
class TaskCounter {
	std::atomic<size_t> count_{0};
	std::atomic<size_t> success_count_{0};
	std::atomic<size_t> failure_count_{0};

public:
	void increment() {
		count_.fetch_add(1, std::memory_order_relaxed);
	}

	void increment_success() {
		success_count_.fetch_add(1, std::memory_order_relaxed);
	}

	void increment_failure() {
		failure_count_.fetch_add(1, std::memory_order_relaxed);
	}

	size_t count() const {
		return count_.load(std::memory_order_relaxed);
	}

	size_t success_count() const {
		return success_count_.load(std::memory_order_relaxed);
	}

	size_t failure_count() const {
		return failure_count_.load(std::memory_order_relaxed);
	}

	void reset() {
		count_.store(0, std::memory_order_relaxed);
		success_count_.store(0, std::memory_order_relaxed);
		failure_count_.store(0, std::memory_order_relaxed);
	}
};

/**
 * @brief Progress tracker for tasks
 */
class ProgressTracker {
	mutable std::mutex mutex_;
	std::vector<std::pair<double, std::string>> progress_updates_;

public:
	void record(double progress, const std::string& message) {
		std::lock_guard lock(mutex_);
		progress_updates_.emplace_back(progress, message);
	}

	std::vector<std::pair<double, std::string>> get_updates() const {
		std::lock_guard lock(mutex_);
		return progress_updates_;
	}

	size_t update_count() const {
		std::lock_guard lock(mutex_);
		return progress_updates_.size();
	}

	void reset() {
		std::lock_guard lock(mutex_);
		progress_updates_.clear();
	}
};

/**
 * @class TaskSystemFixture
 * @brief Base test fixture for task system integration tests
 */
class TaskSystemFixture : public ::testing::Test {
protected:
	std::unique_ptr<task_system> system_;
	task_system_config config_;

	void SetUp() override {
		config_.worker.concurrency = 2;
		config_.worker.queues = {"default"};
		config_.enable_scheduler = false;
		config_.enable_monitoring = false;

		system_ = std::make_unique<task_system>(config_);
	}

	void TearDown() override {
		if (system_ && system_->is_running()) {
			system_->stop();
		}
		system_.reset();
	}

	/**
	 * @brief Create and start the task system
	 */
	void start_system() {
		auto result = system_->start();
		ASSERT_TRUE(result.is_ok()) << result.error().message;
	}

	/**
	 * @brief Stop the task system
	 */
	void stop_system() {
		if (system_ && system_->is_running()) {
			auto result = system_->stop();
			ASSERT_TRUE(result.is_ok()) << result.error().message;
		}
	}

	/**
	 * @brief Register a simple counting handler
	 */
	void register_counting_handler(const std::string& name, TaskCounter& counter) {
		system_->register_handler(name, [&counter](const task& t, task_context& ctx) {
			(void)t;
			(void)ctx;
			counter.increment();
			counter.increment_success();
			return cmn::ok(container_module::value_container{});
		});
	}

	/**
	 * @brief Register a handler that fails
	 */
	void register_failing_handler(
		const std::string& name,
		TaskCounter& counter,
		const std::string& error_msg = "Intentional failure"
	) {
		system_->register_handler(name, [&counter, error_msg](const task& t, task_context& ctx) {
			(void)t;
			(void)ctx;
			counter.increment();
			counter.increment_failure();
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, error_msg}
			);
		});
	}

	/**
	 * @brief Register a handler that tracks progress
	 */
	void register_progress_handler(
		const std::string& name,
		TaskCounter& counter,
		ProgressTracker& tracker
	) {
		system_->register_handler(name, [&counter, &tracker](const task& t, task_context& ctx) {
			(void)t;
			counter.increment();

			ctx.update_progress(0.25, "Starting...");
			tracker.record(0.25, "Starting...");
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			ctx.update_progress(0.50, "Processing...");
			tracker.record(0.50, "Processing...");
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			ctx.update_progress(0.75, "Finishing...");
			tracker.record(0.75, "Finishing...");
			std::this_thread::sleep_for(std::chrono::milliseconds{10});

			ctx.update_progress(1.0, "Complete");
			tracker.record(1.0, "Complete");

			counter.increment_success();
			return cmn::ok(container_module::value_container{});
		});
	}

	/**
	 * @brief Register a handler with simulated work duration
	 */
	void register_slow_handler(
		const std::string& name,
		TaskCounter& counter,
		std::chrono::milliseconds duration
	) {
		system_->register_handler(name, [&counter, duration](const task& t, task_context& ctx) {
			(void)t;
			(void)ctx;
			counter.increment();
			std::this_thread::sleep_for(duration);
			counter.increment_success();
			return cmn::ok(container_module::value_container{});
		});
	}
};

/**
 * @class SchedulerFixture
 * @brief Test fixture for scheduler integration tests
 */
class SchedulerFixture : public TaskSystemFixture {
protected:
	void SetUp() override {
		config_.worker.concurrency = 2;
		config_.worker.queues = {"default"};
		config_.enable_scheduler = true;
		config_.enable_monitoring = false;

		system_ = std::make_unique<task_system>(config_);
	}
};

/**
 * @class MonitoringFixture
 * @brief Test fixture for monitoring integration tests
 */
class MonitoringFixture : public TaskSystemFixture {
protected:
	void SetUp() override {
		config_.worker.concurrency = 2;
		config_.worker.queues = {"default"};
		config_.enable_scheduler = false;
		config_.enable_monitoring = true;

		system_ = std::make_unique<task_system>(config_);
	}
};

}  // namespace kcenon::messaging::task::testing
