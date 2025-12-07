#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task_context.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;
using tsk::task_system;
using tsk::task_system_config;
using tsk::task_builder;
using tsk::task_context;
using tsk::task_state;
using task_t = tsk::task;

// ============================================================================
// task_system construction tests
// ============================================================================

TEST(TaskSystemTest, DefaultConstruction) {
	task_system system;

	EXPECT_FALSE(system.is_running());
	EXPECT_EQ(system.pending_count(), 0);
}

TEST(TaskSystemTest, ConstructionWithConfig) {
	task_system_config config;
	config.worker.concurrency = 2;
	config.worker.queues = {"default", "high-priority"};
	config.enable_scheduler = true;
	config.enable_monitoring = true;

	task_system system(config);

	EXPECT_FALSE(system.is_running());
}

// ============================================================================
// Lifecycle tests
// ============================================================================

TEST(TaskSystemTest, StartAndStop) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	auto start_result = system.start();
	ASSERT_TRUE(start_result.is_ok()) << start_result.error().message;
	EXPECT_TRUE(system.is_running());

	// Starting again should fail
	auto start_again = system.start();
	EXPECT_FALSE(start_again.is_ok());

	auto stop_result = system.stop();
	ASSERT_TRUE(stop_result.is_ok()) << stop_result.error().message;
	EXPECT_FALSE(system.is_running());
}

TEST(TaskSystemTest, GracefulShutdown) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	system.start();
	EXPECT_TRUE(system.is_running());

	auto result = system.shutdown_graceful(std::chrono::seconds(5));
	ASSERT_TRUE(result.is_ok()) << result.error().message;
	EXPECT_FALSE(system.is_running());
}

TEST(TaskSystemTest, DestructorStopsSystem) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	{
		task_system system(config);
		system.start();
		EXPECT_TRUE(system.is_running());
	}
	// Destructor should stop the system without errors
}

// ============================================================================
// Component access tests
// ============================================================================

TEST(TaskSystemTest, AccessClient) {
	task_system system;

	auto& client = system.client();
	EXPECT_FALSE(client.is_connected());  // Not started yet
}

TEST(TaskSystemTest, AccessWorkers) {
	task_system_config config;
	config.worker.concurrency = 2;

	task_system system(config);

	auto& workers = system.workers();
	EXPECT_FALSE(workers.is_running());
	EXPECT_EQ(workers.total_workers(), 0);  // Not started
}

TEST(TaskSystemTest, AccessSchedulerWhenEnabled) {
	task_system_config config;
	config.enable_scheduler = true;

	task_system system(config);

	auto* scheduler = system.scheduler();
	EXPECT_NE(scheduler, nullptr);
	EXPECT_FALSE(scheduler->is_running());
}

TEST(TaskSystemTest, AccessSchedulerWhenDisabled) {
	task_system_config config;
	config.enable_scheduler = false;

	task_system system(config);

	auto* scheduler = system.scheduler();
	EXPECT_EQ(scheduler, nullptr);
}

TEST(TaskSystemTest, AccessMonitorWhenEnabled) {
	task_system_config config;
	config.enable_monitoring = true;

	task_system system(config);

	auto* monitor = system.monitor();
	EXPECT_NE(monitor, nullptr);
}

TEST(TaskSystemTest, AccessMonitorWhenDisabled) {
	task_system_config config;
	config.enable_monitoring = false;

	task_system system(config);

	auto* monitor = system.monitor();
	EXPECT_EQ(monitor, nullptr);
}

TEST(TaskSystemTest, AccessQueue) {
	task_system system;

	auto queue = system.queue();
	EXPECT_NE(queue, nullptr);
}

TEST(TaskSystemTest, AccessResults) {
	task_system system;

	auto results = system.results();
	EXPECT_NE(results, nullptr);
}

// ============================================================================
// Handler registration tests
// ============================================================================

TEST(TaskSystemTest, RegisterHandler) {
	task_system system;

	system.register_handler("test.handler", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	EXPECT_TRUE(system.workers().has_handler("test.handler"));
}

TEST(TaskSystemTest, UnregisterHandler) {
	task_system system;

	system.register_handler("test.handler", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	EXPECT_TRUE(system.unregister_handler("test.handler"));
	EXPECT_FALSE(system.workers().has_handler("test.handler"));
}

// ============================================================================
// Task submission tests
// ============================================================================

TEST(TaskSystemTest, SubmitTask) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	std::atomic<bool> executed{false};

	system.register_handler("test.task", [&executed](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		executed = true;
		return cmn::ok(container_module::value_container{});
	});

	system.start();

	container_module::value_container payload;
	auto result = system.submit("test.task", payload);

	// Wait for task to complete
	auto outcome = result.get(std::chrono::seconds(5));
	EXPECT_TRUE(outcome.is_ok()) << outcome.error().message;
	EXPECT_TRUE(executed);

	system.stop();
}

TEST(TaskSystemTest, SubmitTaskObject) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	std::atomic<bool> executed{false};

	system.register_handler("test.task", [&executed](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		executed = true;
		return cmn::ok(container_module::value_container{});
	});

	system.start();

	auto task_result = task_builder("test.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = system.submit(std::move(task_result).value());

	auto outcome = result.get(std::chrono::seconds(5));
	EXPECT_TRUE(outcome.is_ok()) << outcome.error().message;
	EXPECT_TRUE(executed);

	system.stop();
}

TEST(TaskSystemTest, SubmitBatch) {
	task_system_config config;
	config.worker.concurrency = 2;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	std::atomic<int> count{0};

	system.register_handler("batch.task", [&count](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		++count;
		return cmn::ok(container_module::value_container{});
	});

	system.start();

	std::vector<task_t> tasks;
	for (int i = 0; i < 5; ++i) {
		auto task_result = task_builder("batch.task").build();
		ASSERT_TRUE(task_result.is_ok());
		tasks.push_back(std::move(task_result).value());
	}

	auto results = system.submit_batch(std::move(tasks));
	EXPECT_EQ(results.size(), 5);

	// Wait for all tasks to complete
	for (auto& r : results) {
		auto outcome = r.get(std::chrono::seconds(10));
		EXPECT_TRUE(outcome.is_ok()) << outcome.error().message;
	}

	EXPECT_EQ(count, 5);

	system.stop();
}

// ============================================================================
// Scheduling tests (when enabled)
// ============================================================================

TEST(TaskSystemTest, SchedulePeriodicWhenDisabled) {
	task_system_config config;
	config.enable_scheduler = false;

	task_system system(config);

	auto task_result = task_builder("periodic.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = system.schedule_periodic(
		"test-schedule",
		std::move(task_result).value(),
		std::chrono::seconds(60)
	);

	EXPECT_FALSE(result.is_ok());
}

TEST(TaskSystemTest, ScheduleCronWhenDisabled) {
	task_system_config config;
	config.enable_scheduler = false;

	task_system system(config);

	auto task_result = task_builder("cron.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = system.schedule_cron(
		"test-cron",
		std::move(task_result).value(),
		"0 * * * *"
	);

	EXPECT_FALSE(result.is_ok());
}

TEST(TaskSystemTest, SchedulePeriodicWhenEnabled) {
	task_system_config config;
	config.enable_scheduler = true;
	config.worker.concurrency = 1;

	task_system system(config);

	auto task_result = task_builder("periodic.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = system.schedule_periodic(
		"test-schedule",
		std::move(task_result).value(),
		std::chrono::seconds(60)
	);

	EXPECT_TRUE(result.is_ok()) << result.error().message;
	EXPECT_TRUE(system.scheduler()->has_schedule("test-schedule"));
}

// ============================================================================
// Statistics tests
// ============================================================================

TEST(TaskSystemTest, GetStatisticsBeforeStart) {
	task_system system;

	auto stats = system.get_statistics();
	EXPECT_EQ(stats.total_tasks_processed, 0);
}

TEST(TaskSystemTest, GetStatisticsAfterTasks) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	system.register_handler("stats.task", [](const task_t& t, task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	system.start();

	container_module::value_container payload;
	auto result = system.submit("stats.task", payload);
	result.get(std::chrono::seconds(5));

	auto stats = system.get_statistics();
	EXPECT_GE(stats.total_tasks_processed, 1);
	EXPECT_GE(stats.total_tasks_succeeded, 1);

	system.stop();
}

TEST(TaskSystemTest, PendingCount) {
	task_system_config config;
	config.worker.concurrency = 1;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	EXPECT_EQ(system.pending_count(), 0);
}

TEST(TaskSystemTest, WorkerCounts) {
	task_system_config config;
	config.worker.concurrency = 4;
	config.enable_scheduler = false;
	config.enable_monitoring = false;

	task_system system(config);

	// Before start
	EXPECT_EQ(system.total_workers(), 0);
	EXPECT_EQ(system.active_workers(), 0);

	system.start();

	// After start
	EXPECT_EQ(system.total_workers(), 4);

	system.stop();
}

// ============================================================================
// Integration test
// ============================================================================

TEST(TaskSystemTest, FullWorkflow) {
	task_system_config config;
	config.worker.concurrency = 2;
	config.worker.queues = {"default"};
	config.enable_scheduler = true;
	config.enable_monitoring = true;

	task_system system(config);

	// Register handlers
	system.register_handler("process.data", [](const task_t&, task_context& ctx) {
		ctx.update_progress(0.5, "Processing...");
		return cmn::ok(container_module::value_container{});
	});

	// Start system
	auto start_result = system.start();
	ASSERT_TRUE(start_result.is_ok()) << start_result.error().message;

	// Verify components are accessible
	EXPECT_TRUE(system.is_running());
	EXPECT_NE(system.scheduler(), nullptr);
	EXPECT_NE(system.monitor(), nullptr);

	// Submit task
	container_module::value_container payload;
	auto result = system.submit("process.data", payload);

	// Wait for result
	auto outcome = result.get(std::chrono::seconds(10));
	EXPECT_TRUE(outcome.is_ok()) << outcome.error().message;

	// Check statistics
	auto stats = system.get_statistics();
	EXPECT_GE(stats.total_tasks_processed, 1);

	// Graceful shutdown
	auto shutdown_result = system.shutdown_graceful(std::chrono::seconds(5));
	EXPECT_TRUE(shutdown_result.is_ok());
	EXPECT_FALSE(system.is_running());
}
