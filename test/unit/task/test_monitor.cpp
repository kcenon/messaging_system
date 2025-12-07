#include <kcenon/messaging/task/monitor.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <kcenon/messaging/task/worker_pool.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;
using tsk::task_monitor;
using tsk::task_queue;
using tsk::task_queue_config;
using tsk::memory_result_backend;
using tsk::worker_pool;
using tsk::worker_config;
using tsk::task_builder;
using tsk::task_context;
using tsk::task_state;
using tsk::queue_stats;
using tsk::worker_info;
using task_t = tsk::task;

// ============================================================================
// task_monitor construction tests
// ============================================================================

TEST(TaskMonitorTest, Construction) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Should not throw
	EXPECT_TRUE(true);
}

TEST(TaskMonitorTest, ConstructionWithWorkerPool) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();
	auto workers = std::make_shared<worker_pool>(queue, results);

	task_monitor monitor(queue, results, workers);

	// Should not throw
	EXPECT_TRUE(true);
}

TEST(TaskMonitorTest, ConstructionWithNullComponents) {
	task_monitor monitor(nullptr, nullptr, nullptr);

	// Should handle null gracefully
	auto stats = monitor.get_queue_stats();
	EXPECT_TRUE(stats.empty());

	auto workers = monitor.get_workers();
	EXPECT_TRUE(workers.empty());
}

// ============================================================================
// Queue statistics tests
// ============================================================================

TEST(TaskMonitorTest, GetQueueStats) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Enqueue some tasks
	auto task1 = task_builder("test.task").queue("default").build().unwrap();
	auto task2 = task_builder("test.task").queue("high").build().unwrap();
	queue->enqueue(std::move(task1));
	queue->enqueue(std::move(task2));

	auto stats = monitor.get_queue_stats();

	// Should have at least 2 queues (default and high)
	EXPECT_GE(stats.size(), 2);

	queue->stop();
}

TEST(TaskMonitorTest, GetQueueStatsByName) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Enqueue tasks to specific queue
	for (int i = 0; i < 3; ++i) {
		auto task = task_builder("test.task").queue("test-queue").build().unwrap();
		queue->enqueue(std::move(task));
	}

	auto stats_result = monitor.get_queue_stats("test-queue");
	EXPECT_TRUE(stats_result.is_ok());

	auto stats = stats_result.unwrap();
	EXPECT_EQ(stats.name, "test-queue");
	EXPECT_EQ(stats.pending_count, 3);

	queue->stop();
}

TEST(TaskMonitorTest, GetQueueStatsNonExistent) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	auto stats_result = monitor.get_queue_stats("nonexistent-queue");
	EXPECT_TRUE(stats_result.is_err());
}

// ============================================================================
// Worker status tests
// ============================================================================

TEST(TaskMonitorTest, GetWorkersWithoutPool) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	auto workers = monitor.get_workers();
	EXPECT_TRUE(workers.empty());
}

TEST(TaskMonitorTest, GetWorkersWithPool) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 2;
	auto workers = std::make_shared<worker_pool>(queue, results, config);
	workers->start();

	task_monitor monitor(queue, results, workers);

	auto worker_infos = monitor.get_workers();
	EXPECT_EQ(worker_infos.size(), 2);

	for (const auto& info : worker_infos) {
		EXPECT_TRUE(info.is_healthy);
	}

	workers->stop();
	queue->stop();
}

TEST(TaskMonitorTest, GetWorkerStatistics) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;
	auto workers = std::make_shared<worker_pool>(queue, results, config);

	task_monitor monitor(queue, results, workers);

	auto stats = monitor.get_worker_statistics();
	EXPECT_TRUE(stats.has_value());

	queue->stop();
}

TEST(TaskMonitorTest, GetWorkerStatisticsWithoutPool) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	auto stats = monitor.get_worker_statistics();
	EXPECT_FALSE(stats.has_value());
}

// ============================================================================
// Task query tests
// ============================================================================

TEST(TaskMonitorTest, ListActiveTasks) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Initially empty
	auto active = monitor.list_active_tasks();
	EXPECT_TRUE(active.empty());

	// Notify task started
	auto task = task_builder("test.task").build().unwrap();
	monitor.notify_task_started(task);

	active = monitor.list_active_tasks();
	EXPECT_EQ(active.size(), 1);
	EXPECT_EQ(active[0].task_id(), task.task_id());
}

TEST(TaskMonitorTest, ListPendingTasks) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// This returns empty since task_queue doesn't expose pending task iteration
	auto pending = monitor.list_pending_tasks();
	EXPECT_TRUE(pending.empty());
}

TEST(TaskMonitorTest, ListFailedTasks) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Initially empty
	auto failed = monitor.list_failed_tasks();
	EXPECT_TRUE(failed.empty());

	// Notify task failed
	auto task = task_builder("test.task").build().unwrap();
	monitor.notify_task_failed(task, "Test error");

	failed = monitor.list_failed_tasks();
	EXPECT_EQ(failed.size(), 1);
	EXPECT_EQ(failed[0].task_id(), task.task_id());
}

TEST(TaskMonitorTest, ListFailedTasksWithLimit) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Add multiple failed tasks
	for (int i = 0; i < 10; ++i) {
		auto task = task_builder("test.task").build().unwrap();
		monitor.notify_task_failed(task, "Error " + std::to_string(i));
	}

	// Request limited number
	auto failed = monitor.list_failed_tasks(5);
	EXPECT_EQ(failed.size(), 5);
}

// ============================================================================
// Task management tests
// ============================================================================

TEST(TaskMonitorTest, CancelTask) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Enqueue a task
	auto task = task_builder("test.task").build().unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Cancel it
	auto result = monitor.cancel_task(task_id);
	EXPECT_TRUE(result.is_ok());

	queue->stop();
}

TEST(TaskMonitorTest, CancelTaskWithoutQueue) {
	task_monitor monitor(nullptr, nullptr);

	auto result = monitor.cancel_task("any-id");
	EXPECT_TRUE(result.is_err());
}

TEST(TaskMonitorTest, PurgeQueue) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	// Enqueue tasks
	for (int i = 0; i < 5; ++i) {
		auto task = task_builder("test.task").queue("purge-test").build().unwrap();
		queue->enqueue(std::move(task));
	}

	// Purge the queue
	auto result = monitor.purge_queue("purge-test");
	EXPECT_TRUE(result.is_ok());

	queue->stop();
}

TEST(TaskMonitorTest, PurgeNonExistentQueue) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	auto result = monitor.purge_queue("nonexistent");
	EXPECT_TRUE(result.is_err());
}

// ============================================================================
// Event subscription tests
// ============================================================================

TEST(TaskMonitorTest, OnTaskStarted) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	std::atomic<bool> handler_called{false};
	std::string received_task_id;

	monitor.on_task_started([&](const task_t& t) {
		handler_called = true;
		received_task_id = t.task_id();
	});

	auto task = task_builder("test.task").build().unwrap();
	auto expected_id = task.task_id();
	monitor.notify_task_started(task);

	EXPECT_TRUE(handler_called);
	EXPECT_EQ(received_task_id, expected_id);
}

TEST(TaskMonitorTest, OnTaskCompleted) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	std::atomic<bool> handler_called{false};
	bool received_success = false;

	monitor.on_task_completed([&](const task_t& t, bool success) {
		(void)t;
		handler_called = true;
		received_success = success;
	});

	auto task = task_builder("test.task").build().unwrap();
	monitor.notify_task_completed(task, true);

	EXPECT_TRUE(handler_called);
	EXPECT_TRUE(received_success);
}

TEST(TaskMonitorTest, OnTaskFailed) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	std::atomic<bool> handler_called{false};
	std::string received_error;

	monitor.on_task_failed([&](const task_t& t, const std::string& error) {
		(void)t;
		handler_called = true;
		received_error = error;
	});

	auto task = task_builder("test.task").build().unwrap();
	monitor.notify_task_failed(task, "Test error message");

	EXPECT_TRUE(handler_called);
	EXPECT_EQ(received_error, "Test error message");
}

TEST(TaskMonitorTest, OnWorkerOffline) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	std::atomic<bool> handler_called{false};
	std::string received_worker_id;

	monitor.on_worker_offline([&](const std::string& worker_id) {
		handler_called = true;
		received_worker_id = worker_id;
	});

	monitor.notify_worker_offline("worker-1");

	EXPECT_TRUE(handler_called);
	EXPECT_EQ(received_worker_id, "worker-1");
}

TEST(TaskMonitorTest, MultipleHandlers) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	std::atomic<int> call_count{0};

	monitor.on_task_started([&](const task_t& t) {
		(void)t;
		++call_count;
	});

	monitor.on_task_started([&](const task_t& t) {
		(void)t;
		++call_count;
	});

	auto task = task_builder("test.task").build().unwrap();
	monitor.notify_task_started(task);

	EXPECT_EQ(call_count, 2);
}

// ============================================================================
// Event notification tests
// ============================================================================

TEST(TaskMonitorTest, NotifyTaskStartedUpdatesActiveList) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	auto task = task_builder("test.task").build().unwrap();
	auto task_id = task.task_id();

	EXPECT_TRUE(monitor.list_active_tasks().empty());

	monitor.notify_task_started(task);
	EXPECT_EQ(monitor.list_active_tasks().size(), 1);

	monitor.notify_task_completed(task, true);
	EXPECT_TRUE(monitor.list_active_tasks().empty());
}

TEST(TaskMonitorTest, NotifyTaskFailedUpdatesLists) {
	auto queue = std::make_shared<task_queue>();
	auto results = std::make_shared<memory_result_backend>();

	task_monitor monitor(queue, results);

	auto task = task_builder("test.task").build().unwrap();
	auto task_id = task.task_id();

	// Start task
	monitor.notify_task_started(task);
	EXPECT_EQ(monitor.list_active_tasks().size(), 1);
	EXPECT_TRUE(monitor.list_failed_tasks().empty());

	// Fail task
	monitor.notify_task_failed(task, "Error");
	EXPECT_TRUE(monitor.list_active_tasks().empty());
	EXPECT_EQ(monitor.list_failed_tasks().size(), 1);
}

// ============================================================================
// Integration tests
// ============================================================================

TEST(TaskMonitorTest, IntegrationWithWorkerPool) {
	auto queue = std::make_shared<task_queue>();
	queue->start();
	auto results = std::make_shared<memory_result_backend>();

	worker_config config;
	config.concurrency = 1;
	auto workers = std::make_shared<worker_pool>(queue, results, config);

	task_monitor monitor(queue, results, workers);

	workers->register_handler("monitor.test", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return cmn::ok(container_module::value_container{});
	});

	workers->start();

	// Submit a task
	auto task = task_builder("monitor.test").build().unwrap();
	auto task_id = task.task_id();
	queue->enqueue(std::move(task));

	// Wait for completion
	results->wait_for_result(task_id, std::chrono::seconds(5));

	// Wait for statistics to update (may be delayed on Windows)
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
	std::optional<tsk::worker_statistics> stats;
	while (std::chrono::steady_clock::now() < deadline) {
		stats = monitor.get_worker_statistics();
		if (stats.has_value() && stats->total_tasks_processed >= 1) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	// Check statistics
	EXPECT_TRUE(stats.has_value());
	EXPECT_GE(stats->total_tasks_processed, 1);

	workers->stop();
	queue->stop();
}
