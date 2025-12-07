#include <kcenon/messaging/task/worker_pool.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <kcenon/messaging/task/task_context.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>
#include <latch>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;
using tsk::worker_pool;
using tsk::worker_config;
using tsk::worker_statistics;
using tsk::task_queue;
using tsk::task_queue_config;
using tsk::memory_result_backend;
using tsk::task_builder;
using tsk::task_context;
using tsk::task_state;
using task_t = tsk::task;

// ============================================================================
// worker_pool construction tests
// ============================================================================

TEST(WorkerPoolTest, Construction) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 2;
	config.queues = {"default"};

	worker_pool pool(queue, results, config);

	EXPECT_FALSE(pool.is_running());
	EXPECT_EQ(pool.active_workers(), 0);
	EXPECT_EQ(pool.idle_workers(), 0);
}

// ============================================================================
// Handler registration tests
// ============================================================================

TEST(WorkerPoolTest, RegisterHandler) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	worker_pool pool(queue, results);

	pool.register_handler("test.handler", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	EXPECT_TRUE(pool.has_handler("test.handler"));
	EXPECT_FALSE(pool.has_handler("nonexistent"));
}

TEST(WorkerPoolTest, UnregisterHandler) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	worker_pool pool(queue, results);

	pool.register_handler("test.handler", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	EXPECT_TRUE(pool.has_handler("test.handler"));
	EXPECT_TRUE(pool.unregister_handler("test.handler"));
	EXPECT_FALSE(pool.has_handler("test.handler"));
	EXPECT_FALSE(pool.unregister_handler("test.handler"));
}

TEST(WorkerPoolTest, ListHandlers) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	worker_pool pool(queue, results);

	pool.register_handler("handler.a", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});
	pool.register_handler("handler.b", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	auto handlers = pool.list_handlers();
	EXPECT_EQ(handlers.size(), 2);
}

// ============================================================================
// Lifecycle tests
// ============================================================================

TEST(WorkerPoolTest, StartAndStop) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 2;

	worker_pool pool(queue, results, config);

	auto start_result = pool.start();
	EXPECT_TRUE(start_result.is_ok());
	EXPECT_TRUE(pool.is_running());
	EXPECT_EQ(pool.total_workers(), 2);

	// Starting again should fail
	auto start_again = pool.start();
	EXPECT_TRUE(start_again.is_err());

	auto stop_result = pool.stop();
	EXPECT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(pool.is_running());

	queue->stop();
}

TEST(WorkerPoolTest, StartFailsWithoutQueue) {
	worker_pool pool(nullptr, std::make_shared<memory_result_backend>());

	auto result = pool.start();
	EXPECT_TRUE(result.is_err());
}

TEST(WorkerPoolTest, StartFailsWithoutResultBackend) {
	auto queue = std::make_shared<task_queue>();
	worker_pool pool(queue, nullptr);

	auto result = pool.start();
	EXPECT_TRUE(result.is_err());
}

// ============================================================================
// Task execution tests
// ============================================================================

TEST(WorkerPoolTest, ExecuteSimpleTask) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<bool> executed{false};

	pool.register_handler("simple.task", [&executed](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		executed = true;
		container_module::value_container result;
		result.set_value("status", std::string("done"));
		return cmn::ok(result);
	});

	pool.start();

	// Submit a task
	auto task = task_builder("simple.task").build().unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for execution
	auto result = results->wait_for_result(task_id, std::chrono::seconds(5));

	EXPECT_TRUE(executed);
	EXPECT_TRUE(result.is_ok());

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, ExecuteTaskWithPayload) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<bool> payload_received{false};

	pool.register_handler("payload.task", [&payload_received](const task_t& t, task_context& ctx) {
		(void)ctx;
		// Check if payload exists
		auto& payload = t.payload();
		if (!payload.empty()) {
			payload_received = true;
		}
		container_module::value_container result;
		result.set_value("processed", true);
		return cmn::ok(result);
	});

	pool.start();

	// Submit a task with payload
	container_module::value_container payload;
	payload.set_value("data", std::string("test"));

	auto task = task_builder("payload.task")
		.payload(payload)
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for result
	auto result = results->wait_for_result(task_id, std::chrono::seconds(5));
	EXPECT_TRUE(result.is_ok());
	EXPECT_TRUE(payload_received);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, HandleMissingHandler) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);
	pool.start();

	// Submit a task without registering handler
	auto task = task_builder("unknown.task").build().unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for the task to be processed with polling
	task_state final_state = task_state::pending;
	for (int i = 0; i < 50; ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto state = results->get_state(task_id);
		if (state.is_ok()) {
			final_state = state.unwrap();
			if (final_state == task_state::failed) {
				break;
			}
		}
	}

	EXPECT_EQ(final_state, task_state::failed);

	pool.stop();
	queue->stop();
}

// ============================================================================
// Statistics tests
// ============================================================================

TEST(WorkerPoolTest, Statistics) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	pool.register_handler("stat.task", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Submit multiple tasks
	for (int i = 0; i < 3; ++i) {
		auto task = task_builder("stat.task").build().unwrap();
		queue->enqueue(std::move(task));
	}

	// Wait for processing
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	auto stats = pool.get_statistics();
	EXPECT_GE(stats.total_tasks_processed, 3);
	EXPECT_EQ(stats.total_tasks_succeeded, stats.total_tasks_processed);
	EXPECT_EQ(stats.total_tasks_failed, 0);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, ResetStatistics) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	worker_pool pool(queue, results);

	auto stats_before = pool.get_statistics();

	pool.reset_statistics();

	auto stats_after = pool.get_statistics();
	EXPECT_EQ(stats_after.total_tasks_processed, 0);
	EXPECT_EQ(stats_after.total_tasks_succeeded, 0);
	EXPECT_EQ(stats_after.total_tasks_failed, 0);
}

// ============================================================================
// Graceful shutdown tests
// ============================================================================

TEST(WorkerPoolTest, GracefulShutdown) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<bool> task_started{false};
	std::atomic<bool> task_finished{false};

	pool.register_handler("slow.task", [&](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		task_started = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		task_finished = true;
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Submit a slow task
	auto task = task_builder("slow.task").build().unwrap();
	queue->enqueue(std::move(task));

	// Wait for task to start
	while (!task_started) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Graceful shutdown should wait for task to complete
	auto result = pool.shutdown_graceful(std::chrono::seconds(5));
	EXPECT_TRUE(result.is_ok());
	EXPECT_TRUE(task_finished);

	queue->stop();
}

// ============================================================================
// Multiple workers tests
// ============================================================================

TEST(WorkerPoolTest, MultipleWorkers) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 4;

	worker_pool pool(queue, results, config);

	std::atomic<int> concurrent_count{0};
	std::atomic<int> max_concurrent{0};

	pool.register_handler("concurrent.task", [&](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		int current = ++concurrent_count;
		int expected = max_concurrent.load();
		while (current > expected && !max_concurrent.compare_exchange_weak(expected, current)) {}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		--concurrent_count;
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Submit multiple tasks
	for (int i = 0; i < 8; ++i) {
		auto task = task_builder("concurrent.task").build().unwrap();
		queue->enqueue(std::move(task));
	}

	// Wait for all tasks
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));

	// Should have had some concurrent execution
	EXPECT_GE(max_concurrent.load(), 2);

	pool.stop();
	queue->stop();
}

// ============================================================================
// Progress tracking tests
// ============================================================================

TEST(WorkerPoolTest, ProgressTracking) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	pool.register_handler("progress.task", [](const task_t& t, task_context& ctx) {
		(void)t;
		ctx.update_progress(0.25, "Step 1");
		ctx.update_progress(0.50, "Step 2");
		ctx.update_progress(0.75, "Step 3");
		ctx.update_progress(1.0, "Done");
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	auto task = task_builder("progress.task").build().unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for completion
	auto result = results->wait_for_result(task_id, std::chrono::seconds(5));
	EXPECT_TRUE(result.is_ok());

	// Check final progress
	auto progress = results->get_progress(task_id);
	EXPECT_TRUE(progress.is_ok());
	if (progress.is_ok()) {
		EXPECT_DOUBLE_EQ(progress.unwrap().progress, 1.0);
	}

	pool.stop();
	queue->stop();
}

// ============================================================================
// Retry mechanism tests
// ============================================================================

TEST(WorkerPoolTest, RetryOnFailure) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<int> attempt_count{0};
	constexpr int max_retries = 3;

	pool.register_handler("retry.task", [&attempt_count](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		++attempt_count;
		// Always fail to trigger retries
		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Intentional failure"});
	});

	pool.start();

	auto task = task_builder("retry.task")
		.retries(max_retries)
		.retry_delay(std::chrono::milliseconds(50))
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for all retry attempts plus some buffer time
	// Initial attempt + max_retries = max_retries + 1 total attempts
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	// Should have attempted max_retries + 1 times (initial + retries)
	EXPECT_EQ(attempt_count.load(), max_retries + 1);

	// Final state should be failed
	auto state = results->get_state(task_id);
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.unwrap(), task_state::failed);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, RetrySucceedsOnSecondAttempt) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<int> attempt_count{0};

	pool.register_handler("retry.success", [&attempt_count](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		int current_attempt = ++attempt_count;

		if (current_attempt < 2) {
			// Fail on first attempt
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, "First attempt failure"});
		}

		// Succeed on second attempt
		container_module::value_container result;
		result.set_value("status", std::string("success"));
		return cmn::ok(result);
	});

	pool.start();

	auto task = task_builder("retry.success")
		.retries(3)
		.retry_delay(std::chrono::milliseconds(50))
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for result
	auto result = results->wait_for_result(task_id, std::chrono::seconds(5));

	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(attempt_count.load(), 2);

	// Final state should be succeeded
	auto state = results->get_state(task_id);
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.unwrap(), task_state::succeeded);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, RetryExponentialBackoff) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::vector<std::chrono::steady_clock::time_point> attempt_times;
	std::mutex times_mutex;

	pool.register_handler("backoff.task", [&](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		{
			std::lock_guard<std::mutex> lock(times_mutex);
			attempt_times.push_back(std::chrono::steady_clock::now());
		}
		// Always fail
		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Intentional failure"});
	});

	pool.start();

	// Use base delay of 100ms with 2.0 multiplier
	// Expected delays: 100ms, 200ms, 400ms
	auto task = task_builder("backoff.task")
		.retries(3)
		.retry_delay(std::chrono::milliseconds(100))
		.retry_backoff(2.0)
		.build()
		.unwrap();
	queue->enqueue(std::move(task));

	// Wait for all attempts
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	std::lock_guard<std::mutex> lock(times_mutex);
	ASSERT_GE(attempt_times.size(), 3);

	// Check that delays are increasing (exponential backoff)
	if (attempt_times.size() >= 3) {
		auto delay1 = std::chrono::duration_cast<std::chrono::milliseconds>(
			attempt_times[1] - attempt_times[0]);
		auto delay2 = std::chrono::duration_cast<std::chrono::milliseconds>(
			attempt_times[2] - attempt_times[1]);

		// Second delay should be roughly double the first (with some tolerance)
		// Note: actual delays include execution time, so we check relative increase
		EXPECT_GE(delay2.count(), delay1.count());
	}

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, OnRetryHookCalled) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<int> retry_hook_count{0};
	std::atomic<int> failure_hook_count{0};

	// Create a custom handler to track hooks
	class retry_tracking_handler : public tsk::task_handler_interface {
	public:
		retry_tracking_handler(std::atomic<int>& retry_count, std::atomic<int>& failure_count)
			: retry_count_(retry_count), failure_count_(failure_count) {}

		std::string name() const override { return "hook.test"; }

		cmn::Result<container_module::value_container> execute(
			const task_t& t,
			task_context& ctx) override {
			(void)t;
			(void)ctx;
			// Always fail
			return cmn::Result<container_module::value_container>(
				cmn::error_info{-1, "Intentional failure"});
		}

		void on_retry(const task_t& t, size_t attempt) override {
			(void)t;
			(void)attempt;
			++retry_count_;
		}

		void on_failure(const task_t& t, const std::string& error) override {
			(void)t;
			(void)error;
			++failure_count_;
		}

	private:
		std::atomic<int>& retry_count_;
		std::atomic<int>& failure_count_;
	};

	pool.register_handler(
		std::make_shared<retry_tracking_handler>(retry_hook_count, failure_hook_count));

	pool.start();

	auto task = task_builder("hook.test")
		.retries(2)
		.retry_delay(std::chrono::milliseconds(50))
		.build()
		.unwrap();
	queue->enqueue(std::move(task));

	// Wait for all attempts
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// on_retry should be called for each retry (2 times)
	EXPECT_EQ(retry_hook_count.load(), 2);
	// on_failure should be called once when all retries exhausted
	EXPECT_EQ(failure_hook_count.load(), 1);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, NoRetryWhenMaxRetriesZero) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<int> attempt_count{0};

	pool.register_handler("no.retry", [&attempt_count](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		++attempt_count;
		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Intentional failure"});
	});

	pool.start();

	auto task = task_builder("no.retry")
		.retries(0)  // No retries
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for task to fail
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	// Should only attempt once (no retries)
	EXPECT_EQ(attempt_count.load(), 1);

	// Final state should be failed
	auto state = results->get_state(task_id);
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.unwrap(), task_state::failed);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, RetryStatistics) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	pool.register_handler("stat.retry", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::Result<container_module::value_container>(
			cmn::error_info{-1, "Intentional failure"});
	});

	pool.start();

	auto task = task_builder("stat.retry")
		.retries(2)
		.retry_delay(std::chrono::milliseconds(50))
		.build()
		.unwrap();
	queue->enqueue(std::move(task));

	// Wait for all attempts
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	auto stats = pool.get_statistics();
	// Should have 2 retries recorded
	EXPECT_EQ(stats.total_tasks_retried, 2);
	// Should have 1 failed task
	EXPECT_EQ(stats.total_tasks_failed, 1);

	pool.stop();
	queue->stop();
}

// ============================================================================
// Timeout handling tests
// ============================================================================

TEST(WorkerPoolTest, TaskTimeoutSoftCancellation) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<bool> cancellation_detected{false};
	std::atomic<bool> task_started{false};

	pool.register_handler("timeout.task", [&](const task_t& t, task_context& ctx) {
		(void)t;
		task_started = true;

		// Simulate long-running task that checks for cancellation
		for (int i = 0; i < 100; ++i) {
			if (ctx.is_cancelled()) {
				cancellation_detected = true;
				return cmn::Result<container_module::value_container>(
					cmn::error_info{-1, "Task cancelled"});
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Create task with short timeout (200ms)
	auto task = task_builder("timeout.task")
		.timeout(std::chrono::milliseconds(200))
		.retries(0)  // No retries for this test
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for task to be processed
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	EXPECT_TRUE(task_started);
	EXPECT_TRUE(cancellation_detected);

	// Check final state is failed
	auto state = results->get_state(task_id);
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.unwrap(), task_state::failed);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, TaskTimeoutErrorMessage) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	pool.register_handler("long.task", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		// Sleep longer than timeout
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Create task with short timeout
	auto task = task_builder("long.task")
		.timeout(std::chrono::milliseconds(100))
		.retries(0)
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for task to timeout
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Check error message contains timeout info
	auto error = results->get_error(task_id);
	EXPECT_TRUE(error.is_ok());
	EXPECT_TRUE(error.unwrap().message.find("timed out") != std::string::npos);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, TaskTimeoutStatistics) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	pool.register_handler("slow.stat", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Submit tasks that will timeout
	for (int i = 0; i < 2; ++i) {
		auto task = task_builder("slow.stat")
			.timeout(std::chrono::milliseconds(100))
			.retries(0)
			.build()
			.unwrap();
		queue->enqueue(std::move(task));
	}

	// Wait for tasks to timeout
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	auto stats = pool.get_statistics();
	EXPECT_EQ(stats.total_tasks_timed_out, 2);
	EXPECT_EQ(stats.total_tasks_failed, 2);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, TaskCompletesBeforeTimeout) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	pool.register_handler("quick.task", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		// Complete quickly
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		container_module::value_container result;
		result.set_value("status", std::string("completed"));
		return cmn::ok(result);
	});

	pool.start();

	// Create task with generous timeout
	auto task = task_builder("quick.task")
		.timeout(std::chrono::milliseconds(5000))
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for result
	auto result = results->wait_for_result(task_id, std::chrono::seconds(5));
	EXPECT_TRUE(result.is_ok());

	// Check final state is succeeded
	auto state = results->get_state(task_id);
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.unwrap(), task_state::succeeded);

	// No timeouts should be recorded
	auto stats = pool.get_statistics();
	EXPECT_EQ(stats.total_tasks_timed_out, 0);

	pool.stop();
	queue->stop();
}

TEST(WorkerPoolTest, TaskTimeoutWithRetry) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;

	worker_pool pool(queue, results, config);

	std::atomic<int> attempt_count{0};

	pool.register_handler("timeout.retry", [&attempt_count](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		++attempt_count;
		// Always sleep longer than timeout
		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		return cmn::ok(container_module::value_container{});
	});

	pool.start();

	// Create task with short timeout and retries
	auto task = task_builder("timeout.retry")
		.timeout(std::chrono::milliseconds(100))
		.retries(2)
		.retry_delay(std::chrono::milliseconds(50))
		.build()
		.unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for all attempts
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	// Should have attempted 3 times (initial + 2 retries)
	EXPECT_EQ(attempt_count.load(), 3);

	// Final state should be failed
	auto state = results->get_state(task_id);
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.unwrap(), task_state::failed);

	pool.stop();
	queue->stop();
}
