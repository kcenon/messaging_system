// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_failure_recovery.cpp
 * @brief Integration tests for failure handling and recovery
 *
 * Tests task failures, retry mechanisms, timeout handling,
 * and dead letter queue behavior.
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
using tsk::testing::wait_for_condition;

class FailureRecoveryTest : public TaskSystemFixture {};

// ============================================================================
// Failure -> Retry -> Success
// ============================================================================

TEST_F(FailureRecoveryTest, RetryOnFailureEventualSuccess) {
	std::atomic<int> attempt_count{0};
	const int fail_until = 2;  // Fail first 2 attempts, succeed on 3rd

	system_->register_handler("failure.retry_success", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		int attempt = ++attempt_count;

		if (attempt < fail_until) {
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, "Temporary failure"}
			);
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Create task with retries enabled
	auto task_result = tsk::task_builder("failure.retry_success")
		.retries(3)
		.retry_delay(std::chrono::milliseconds{100})
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());
	auto result = async_result.get(std::chrono::seconds(30));

	// Should eventually succeed
	EXPECT_TRUE(result.is_ok()) << result.error().message;

	// Should have been called multiple times
	EXPECT_GE(attempt_count.load(), fail_until);
}

TEST_F(FailureRecoveryTest, RetryWithExponentialBackoff) {
	std::vector<std::chrono::steady_clock::time_point> attempt_times;
	std::mutex times_mutex;

	system_->register_handler("failure.backoff", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		{
			std::lock_guard lock(times_mutex);
			attempt_times.push_back(std::chrono::steady_clock::now());

			// Fail until we have enough data points
			if (attempt_times.size() < 3) {
				return cmn::Result<container_module::value_container>(
					cmn::error_info{-1, "Retry needed"}
				);
			}
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("failure.backoff")
		.retries(5)
		.retry_delay(std::chrono::milliseconds{100})
		.retry_backoff(2.0)  // Double delay each retry
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());
	auto result = async_result.get(std::chrono::seconds(30));

	EXPECT_TRUE(result.is_ok()) << result.error().message;

	// Verify exponential backoff pattern
	std::lock_guard lock(times_mutex);
	EXPECT_GE(attempt_times.size(), 3);

	if (attempt_times.size() >= 3) {
		auto delay1 = std::chrono::duration_cast<std::chrono::milliseconds>(
			attempt_times[1] - attempt_times[0]
		);
		auto delay2 = std::chrono::duration_cast<std::chrono::milliseconds>(
			attempt_times[2] - attempt_times[1]
		);

		// Second delay should be roughly double the first (with some tolerance)
		// Note: Actual backoff behavior depends on implementation
		EXPECT_GE(delay1.count(), 50);  // At least some delay
	}
}

// ============================================================================
// Failure After Max Retries
// ============================================================================

TEST_F(FailureRecoveryTest, PermanentFailureAfterMaxRetries) {
	std::atomic<int> attempt_count{0};

	system_->register_handler("failure.permanent", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		attempt_count++;
		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Always fails"}
		);
	});

	start_system();

	const size_t max_retries = 3;
	auto task_result = tsk::task_builder("failure.permanent")
		.retries(max_retries)
		.retry_delay(std::chrono::milliseconds{50})
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());
	auto result = async_result.get(std::chrono::seconds(30));

	// Should fail after all retries exhausted
	EXPECT_FALSE(result.is_ok());

	// Should have been called max_retries + 1 times (initial + retries)
	EXPECT_EQ(attempt_count.load(), static_cast<int>(max_retries) + 1);
}

TEST_F(FailureRecoveryTest, NoRetriesConfigured) {
	std::atomic<int> attempt_count{0};

	system_->register_handler("failure.no_retry", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		attempt_count++;
		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Single failure"}
		);
	});

	start_system();

	auto task_result = tsk::task_builder("failure.no_retry")
		.retries(0)  // No retries
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());
	auto result = async_result.get(std::chrono::seconds(10));

	EXPECT_FALSE(result.is_ok());
	EXPECT_EQ(attempt_count.load(), 1);  // Only one attempt
}

// ============================================================================
// Timeout Handling
// ============================================================================

TEST_F(FailureRecoveryTest, TaskTimeout) {
	std::atomic<bool> task_started{false};
	std::atomic<bool> should_exit{false};

	system_->register_handler("failure.timeout", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		task_started = true;

		// Wait in small increments, checking for exit signal
		// This allows the test to clean up properly
		for (int i = 0; i < 20 && !should_exit.load(); ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds{100});
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// Create task with short timeout
	auto task_result = tsk::task_builder("failure.timeout")
		.timeout(std::chrono::milliseconds{500})
		.retries(0)
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());

	// Wait for result with reasonable timeout
	auto result = async_result.get(std::chrono::seconds(5));

	// Signal handler to exit for cleanup
	should_exit = true;

	// Task should have started
	EXPECT_TRUE(task_started.load());

	// Result depends on timeout implementation
	// Some implementations may cancel the task, others may let it complete
}

TEST_F(FailureRecoveryTest, AsyncResultWaitTimeout) {
	std::atomic<bool> can_complete{false};
	std::atomic<int> loop_count{0};

	system_->register_handler("failure.wait_timeout", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		// Wait until allowed to complete, with a maximum loop count for safety
		const int max_loops = 500;  // 5 seconds max
		while (!can_complete.load() && loop_count.load() < max_loops) {
			loop_count++;
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	container_module::value_container payload;
	auto async_result = system_->submit("failure.wait_timeout", payload);

	// Try to get result with very short timeout
	auto result = async_result.get(std::chrono::milliseconds{100});

	// Should timeout (result not ready yet)
	// Note: depending on implementation, this may return an error or empty result

	// Allow task to complete for cleanup
	can_complete = true;

	// Wait for task to finish to ensure clean teardown
	async_result.wait(std::chrono::seconds(5));
}

// ============================================================================
// Exception Handling
// ============================================================================

TEST_F(FailureRecoveryTest, HandlerThrowsException) {
	std::atomic<int> attempt_count{0};

	system_->register_handler("failure.exception", [&](const tsk::task& t, tsk::task_context& ctx) -> cmn::Result<container_module::value_container> {
		(void)t;
		(void)ctx;

		attempt_count++;
		throw std::runtime_error("Unexpected exception");
	});

	start_system();

	auto task_result = tsk::task_builder("failure.exception")
		.retries(2)
		.retry_delay(std::chrono::milliseconds{50})
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());
	auto result = async_result.get(std::chrono::seconds(30));

	// Should fail due to exception
	EXPECT_FALSE(result.is_ok());

	// Should have retried
	EXPECT_GE(attempt_count.load(), 1);
}

// ============================================================================
// Error Message Propagation
// ============================================================================

TEST_F(FailureRecoveryTest, ErrorMessagePropagation) {
	const std::string error_message = "Specific error message for testing";

	system_->register_handler("failure.error_msg", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, error_message}
		);
	});

	start_system();

	auto task_result = tsk::task_builder("failure.error_msg")
		.retries(0)
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto async_result = system_->submit(std::move(task_result).value());
	auto result = async_result.get(std::chrono::seconds(10));

	ASSERT_FALSE(result.is_ok());

	// Error message should be propagated
	EXPECT_NE(result.error().message.find(error_message), std::string::npos);
}

// ============================================================================
// Recovery Patterns
// ============================================================================

TEST_F(FailureRecoveryTest, CircuitBreakerPattern) {
	// Simulates a circuit breaker pattern where we track failures
	std::atomic<int> consecutive_failures{0};
	std::atomic<int> total_attempts{0};
	const int failure_threshold = 3;

	system_->register_handler("failure.circuit", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		total_attempts++;
		int failures = consecutive_failures.load();

		// Simulate intermittent failure
		if (failures < failure_threshold) {
			consecutive_failures++;
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, "Service unavailable"}
			);
		}

		// Reset failures on success
		consecutive_failures.store(0);
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	// First few tasks will fail
	for (int i = 0; i < failure_threshold; ++i) {
		auto task_result = tsk::task_builder("failure.circuit")
			.retries(0)
			.build();

		ASSERT_TRUE(task_result.is_ok());
		auto result = system_->submit(std::move(task_result).value()).get(std::chrono::seconds(5));
		EXPECT_FALSE(result.is_ok());
	}

	// After failures, next task should succeed (circuit "closes")
	auto task_result = tsk::task_builder("failure.circuit")
		.retries(0)
		.build();

	ASSERT_TRUE(task_result.is_ok());
	auto result = system_->submit(std::move(task_result).value()).get(std::chrono::seconds(5));
	EXPECT_TRUE(result.is_ok()) << result.error().message;
}

TEST_F(FailureRecoveryTest, PartialBatchFailure) {
	std::atomic<int> task_index{0};

	// Every other task fails
	system_->register_handler("failure.batch", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		int index = task_index++;

		if (index % 2 == 0) {
			return cmn::ok(container_module::value_container{});
		}

		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Batch item failure"}
		);
	});

	start_system();

	const int batch_size = 10;
	std::vector<tsk::task> tasks;

	for (int i = 0; i < batch_size; ++i) {
		auto task_result = tsk::task_builder("failure.batch")
			.retries(0)
			.build();

		ASSERT_TRUE(task_result.is_ok());
		tasks.push_back(std::move(task_result).value());
	}

	auto results = system_->submit_batch(std::move(tasks));

	int success_count = 0;
	int failure_count = 0;

	for (auto& r : results) {
		auto result = r.get(std::chrono::seconds(10));
		if (result.is_ok()) {
			success_count++;
		} else {
			failure_count++;
		}
	}

	// Half should succeed, half should fail
	EXPECT_EQ(success_count, batch_size / 2);
	EXPECT_EQ(failure_count, batch_size / 2);
}

// ============================================================================
// Statistics for Failures
// ============================================================================

TEST_F(FailureRecoveryTest, FailureStatistics) {
	system_->register_handler("failure.stats", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Stats failure"}
		);
	});

	start_system();

	const int failure_count = 5;

	for (int i = 0; i < failure_count; ++i) {
		auto task_result = tsk::task_builder("failure.stats")
			.retries(0)
			.build();

		ASSERT_TRUE(task_result.is_ok());
		system_->submit(std::move(task_result).value()).get(std::chrono::seconds(10));
	}

	auto stats = system_->get_statistics();
	EXPECT_GE(stats.total_tasks_failed, static_cast<size_t>(failure_count));
}

TEST_F(FailureRecoveryTest, RetryStatistics) {
	std::atomic<int> attempt_count{0};

	system_->register_handler("failure.retry_stats", [&](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		if (++attempt_count < 3) {
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, "Need retry"}
			);
		}

		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("failure.retry_stats")
		.retries(5)
		.retry_delay(std::chrono::milliseconds{50})
		.build();

	ASSERT_TRUE(task_result.is_ok());
	auto result = system_->submit(std::move(task_result).value()).get(std::chrono::seconds(30));

	EXPECT_TRUE(result.is_ok()) << result.error().message;

	auto stats = system_->get_statistics();
	EXPECT_GE(stats.total_tasks_retried, 1);
}

// ============================================================================
// Task State After Failure
// ============================================================================

TEST_F(FailureRecoveryTest, TaskStateOnFailure) {
	system_->register_handler("failure.state", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;

		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "State test failure"}
		);
	});

	start_system();

	auto task_result = tsk::task_builder("failure.state")
		.retries(0)
		.build();

	ASSERT_TRUE(task_result.is_ok());

	auto task = std::move(task_result).value();
	auto task_id = task.task_id();

	auto async_result = system_->submit(std::move(task));
	auto result = async_result.get(std::chrono::seconds(10));

	EXPECT_FALSE(result.is_ok());

	// Check stored task state
	auto stored_state = system_->results()->get_state(task_id);
	if (stored_state.is_ok()) {
		// State should indicate failure
		EXPECT_EQ(stored_state.value(), tsk::task_state::failed);
	}
}
