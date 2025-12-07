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
