#include <kcenon/messaging/task/scheduler.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <set>

namespace tsk = kcenon::messaging::task;
using tsk::task_scheduler;
using tsk::schedule_entry;
using tsk::task_builder;
using tsk::task_queue;
using tsk::task_client;
using tsk::memory_result_backend;
using task_t = tsk::task;

// ============================================================================
// Test fixture for scheduler tests
// ============================================================================

class SchedulerTest : public ::testing::Test {
protected:
	void SetUp() override {
		queue_ = std::make_shared<task_queue>();
		queue_->start();

		backend_ = std::make_shared<memory_result_backend>();
		client_ = std::make_shared<task_client>(queue_, backend_);
	}

	void TearDown() override {
		queue_->stop();
	}

	std::shared_ptr<task_queue> queue_;
	std::shared_ptr<memory_result_backend> backend_;
	std::shared_ptr<task_client> client_;
};

// ============================================================================
// schedule_entry tests
// ============================================================================

TEST(ScheduleEntryTest, DefaultValues) {
	schedule_entry entry;

	EXPECT_TRUE(entry.name.empty());
	EXPECT_TRUE(entry.enabled);
	EXPECT_FALSE(entry.last_run.has_value());
	EXPECT_FALSE(entry.next_run.has_value());
	EXPECT_EQ(entry.run_count, 0);
	EXPECT_EQ(entry.failure_count, 0);
}

TEST(ScheduleEntryTest, PeriodicSchedule) {
	schedule_entry entry;
	entry.name = "periodic-test";
	entry.schedule = std::chrono::seconds(60);

	EXPECT_TRUE(entry.is_periodic());
	EXPECT_FALSE(entry.is_cron());
	EXPECT_EQ(entry.interval(), std::chrono::seconds(60));
	EXPECT_TRUE(entry.cron_expression().empty());
}

TEST(ScheduleEntryTest, CronSchedule) {
	schedule_entry entry;
	entry.name = "cron-test";
	entry.schedule = std::string("0 3 * * *");

	EXPECT_FALSE(entry.is_periodic());
	EXPECT_TRUE(entry.is_cron());
	EXPECT_EQ(entry.cron_expression(), "0 3 * * *");
	EXPECT_EQ(entry.interval(), std::chrono::seconds{0});
}

// ============================================================================
// task_scheduler tests - Construction and Lifecycle
// ============================================================================

TEST_F(SchedulerTest, Construction) {
	task_scheduler scheduler(client_);

	EXPECT_FALSE(scheduler.is_running());
	EXPECT_EQ(scheduler.schedule_count(), 0);
}

TEST_F(SchedulerTest, StartStop) {
	task_scheduler scheduler(client_);

	auto start_result = scheduler.start();
	EXPECT_TRUE(start_result.is_ok());
	EXPECT_TRUE(scheduler.is_running());

	auto stop_result = scheduler.stop();
	EXPECT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(scheduler.is_running());
}

TEST_F(SchedulerTest, MultipleStartsAreIdempotent) {
	task_scheduler scheduler(client_);

	auto result1 = scheduler.start();
	EXPECT_TRUE(result1.is_ok());

	auto result2 = scheduler.start();
	EXPECT_TRUE(result2.is_ok());

	scheduler.stop();
}

TEST_F(SchedulerTest, DestructorStopsScheduler) {
	{
		task_scheduler scheduler(client_);
		scheduler.start();
		EXPECT_TRUE(scheduler.is_running());
	}
	// Destructor should have stopped the scheduler cleanly
}

TEST_F(SchedulerTest, MoveConstruction) {
	task_scheduler scheduler1(client_);

	// Add a schedule before moving
	auto t = task_builder("move.test").build().unwrap();
	scheduler1.add_periodic("test-schedule", t, std::chrono::hours(1));

	// Move before starting (thread cannot be moved while running)
	task_scheduler scheduler2(std::move(scheduler1));

	// Can start the moved scheduler
	auto result = scheduler2.start();
	EXPECT_TRUE(result.is_ok());
	EXPECT_TRUE(scheduler2.is_running());
	EXPECT_TRUE(scheduler2.has_schedule("test-schedule"));

	scheduler2.stop();
}

// ============================================================================
// task_scheduler tests - Periodic Schedules
// ============================================================================

TEST_F(SchedulerTest, AddPeriodicSchedule) {
	task_scheduler scheduler(client_);

	auto task_result = task_builder("periodic.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = scheduler.add_periodic(
		"test-periodic",
		task_result.unwrap(),
		std::chrono::seconds(60)
	);

	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(scheduler.schedule_count(), 1);
	EXPECT_TRUE(scheduler.has_schedule("test-periodic"));
}

TEST_F(SchedulerTest, AddDuplicatePeriodicSchedule) {
	task_scheduler scheduler(client_);

	auto task1 = task_builder("task1").build().unwrap();
	auto task2 = task_builder("task2").build().unwrap();

	scheduler.add_periodic("duplicate", task1, std::chrono::seconds(60));
	auto result = scheduler.add_periodic("duplicate", task2, std::chrono::seconds(120));

	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(scheduler.schedule_count(), 1);
}

TEST_F(SchedulerTest, PeriodicScheduleExecution) {
	task_scheduler scheduler(client_);

	std::atomic<int> execute_count{0};
	std::mutex mtx;
	std::condition_variable cv;

	scheduler.on_task_executed([&](const schedule_entry& /* entry */) {
		execute_count.fetch_add(1);
		cv.notify_one();
	});

	auto t = task_builder("periodic.exec").build().unwrap();
	scheduler.add_periodic("fast-periodic", t, std::chrono::seconds(1));

	scheduler.start();

	// Wait for at least one execution
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait_for(lock, std::chrono::seconds(3), [&] {
			return execute_count.load() >= 1;
		});
	}

	scheduler.stop();

	EXPECT_GE(execute_count.load(), 1);
}

// ============================================================================
// task_scheduler tests - Cron Schedules
// ============================================================================

TEST_F(SchedulerTest, AddCronSchedule) {
	task_scheduler scheduler(client_);

	auto task_result = task_builder("cron.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = scheduler.add_cron(
		"test-cron",
		task_result.unwrap(),
		"0 3 * * *"  // Daily at 3 AM
	);

	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(scheduler.schedule_count(), 1);
}

TEST_F(SchedulerTest, AddInvalidCronExpression) {
	task_scheduler scheduler(client_);

	auto t = task_builder("cron.task").build().unwrap();

	auto result = scheduler.add_cron("invalid-cron", t, "invalid cron");

	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(scheduler.schedule_count(), 0);
}

TEST_F(SchedulerTest, AddDuplicateCronSchedule) {
	task_scheduler scheduler(client_);

	auto task1 = task_builder("task1").build().unwrap();
	auto task2 = task_builder("task2").build().unwrap();

	scheduler.add_cron("duplicate", task1, "0 * * * *");
	auto result = scheduler.add_cron("duplicate", task2, "30 * * * *");

	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(scheduler.schedule_count(), 1);
}

// ============================================================================
// task_scheduler tests - Schedule Management
// ============================================================================

TEST_F(SchedulerTest, RemoveSchedule) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_periodic("to-remove", t, std::chrono::seconds(60));

	EXPECT_EQ(scheduler.schedule_count(), 1);

	auto result = scheduler.remove("to-remove");
	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(scheduler.schedule_count(), 0);
	EXPECT_FALSE(scheduler.has_schedule("to-remove"));
}

TEST_F(SchedulerTest, RemoveNonexistentSchedule) {
	task_scheduler scheduler(client_);

	auto result = scheduler.remove("nonexistent");
	EXPECT_TRUE(result.is_err());
}

TEST_F(SchedulerTest, EnableDisableSchedule) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_periodic("toggle", t, std::chrono::seconds(60));

	auto disable_result = scheduler.disable("toggle");
	EXPECT_TRUE(disable_result.is_ok());

	auto entry = scheduler.get_schedule("toggle");
	ASSERT_TRUE(entry.is_ok());
	EXPECT_FALSE(entry.unwrap().enabled);

	auto enable_result = scheduler.enable("toggle");
	EXPECT_TRUE(enable_result.is_ok());

	entry = scheduler.get_schedule("toggle");
	ASSERT_TRUE(entry.is_ok());
	EXPECT_TRUE(entry.unwrap().enabled);
}

TEST_F(SchedulerTest, EnableDisableNonexistentSchedule) {
	task_scheduler scheduler(client_);

	EXPECT_TRUE(scheduler.enable("nonexistent").is_err());
	EXPECT_TRUE(scheduler.disable("nonexistent").is_err());
}

TEST_F(SchedulerTest, TriggerNow) {
	task_scheduler scheduler(client_);

	std::atomic<bool> executed{false};
	std::mutex mtx;
	std::condition_variable cv;

	scheduler.on_task_executed([&](const schedule_entry& /* entry */) {
		executed.store(true);
		cv.notify_one();
	});

	// Create a schedule with a long interval
	auto t = task_builder("trigger.task").build().unwrap();
	scheduler.add_periodic("trigger-test", t, std::chrono::hours(24));

	scheduler.start();

	// Trigger immediately
	auto result = scheduler.trigger_now("trigger-test");
	EXPECT_TRUE(result.is_ok());

	// Wait for execution
	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait_for(lock, std::chrono::seconds(2), [&] {
			return executed.load();
		});
	}

	scheduler.stop();

	EXPECT_TRUE(executed.load());
}

TEST_F(SchedulerTest, TriggerNonexistent) {
	task_scheduler scheduler(client_);
	scheduler.start();

	auto result = scheduler.trigger_now("nonexistent");
	EXPECT_TRUE(result.is_err());

	scheduler.stop();
}

TEST_F(SchedulerTest, UpdateInterval) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_periodic("update-interval", t, std::chrono::seconds(60));

	auto result = scheduler.update_interval("update-interval", std::chrono::seconds(120));
	EXPECT_TRUE(result.is_ok());

	auto entry = scheduler.get_schedule("update-interval");
	ASSERT_TRUE(entry.is_ok());
	EXPECT_EQ(entry.unwrap().interval(), std::chrono::seconds(120));
}

TEST_F(SchedulerTest, UpdateIntervalOnCronSchedule) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_cron("cron-schedule", t, "0 * * * *");

	auto result = scheduler.update_interval("cron-schedule", std::chrono::seconds(60));
	EXPECT_TRUE(result.is_err());
}

TEST_F(SchedulerTest, UpdateCron) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_cron("update-cron", t, "0 * * * *");

	auto result = scheduler.update_cron("update-cron", "30 * * * *");
	EXPECT_TRUE(result.is_ok());

	auto entry = scheduler.get_schedule("update-cron");
	ASSERT_TRUE(entry.is_ok());
	EXPECT_EQ(entry.unwrap().cron_expression(), "30 * * * *");
}

TEST_F(SchedulerTest, UpdateCronOnPeriodicSchedule) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_periodic("periodic-schedule", t, std::chrono::seconds(60));

	auto result = scheduler.update_cron("periodic-schedule", "0 * * * *");
	EXPECT_TRUE(result.is_err());
}

TEST_F(SchedulerTest, UpdateCronInvalid) {
	task_scheduler scheduler(client_);

	auto t = task_builder("task").build().unwrap();
	scheduler.add_cron("cron-schedule", t, "0 * * * *");

	auto result = scheduler.update_cron("cron-schedule", "invalid");
	EXPECT_TRUE(result.is_err());

	// Original should be preserved
	auto entry = scheduler.get_schedule("cron-schedule");
	ASSERT_TRUE(entry.is_ok());
	EXPECT_EQ(entry.unwrap().cron_expression(), "0 * * * *");
}

// ============================================================================
// task_scheduler tests - Query Operations
// ============================================================================

TEST_F(SchedulerTest, ListSchedules) {
	task_scheduler scheduler(client_);

	auto t1 = task_builder("task1").build().unwrap();
	auto t2 = task_builder("task2").build().unwrap();
	auto t3 = task_builder("task3").build().unwrap();

	scheduler.add_periodic("schedule-1", t1, std::chrono::seconds(60));
	scheduler.add_periodic("schedule-2", t2, std::chrono::seconds(120));
	scheduler.add_cron("schedule-3", t3, "0 * * * *");

	auto schedules = scheduler.list_schedules();
	EXPECT_EQ(schedules.size(), 3);

	std::set<std::string> names;
	for (const auto& entry : schedules) {
		names.insert(entry.name);
	}

	EXPECT_TRUE(names.count("schedule-1") > 0);
	EXPECT_TRUE(names.count("schedule-2") > 0);
	EXPECT_TRUE(names.count("schedule-3") > 0);
}

TEST_F(SchedulerTest, GetSchedule) {
	task_scheduler scheduler(client_);

	auto t = task_builder("test.task").build().unwrap();
	scheduler.add_periodic("get-test", t, std::chrono::seconds(60));

	auto result = scheduler.get_schedule("get-test");
	ASSERT_TRUE(result.is_ok());

	const auto& entry = result.unwrap();
	EXPECT_EQ(entry.name, "get-test");
	EXPECT_TRUE(entry.is_periodic());
	EXPECT_EQ(entry.interval(), std::chrono::seconds(60));
	EXPECT_TRUE(entry.enabled);
}

TEST_F(SchedulerTest, GetNonexistentSchedule) {
	task_scheduler scheduler(client_);

	auto result = scheduler.get_schedule("nonexistent");
	EXPECT_TRUE(result.is_err());
}

TEST_F(SchedulerTest, HasSchedule) {
	task_scheduler scheduler(client_);

	EXPECT_FALSE(scheduler.has_schedule("test-schedule"));

	auto t = task_builder("task").build().unwrap();
	scheduler.add_periodic("test-schedule", t, std::chrono::seconds(60));

	EXPECT_TRUE(scheduler.has_schedule("test-schedule"));
}

TEST_F(SchedulerTest, ScheduleCount) {
	task_scheduler scheduler(client_);

	EXPECT_EQ(scheduler.schedule_count(), 0);

	auto t1 = task_builder("task1").build().unwrap();
	auto t2 = task_builder("task2").build().unwrap();

	scheduler.add_periodic("s1", t1, std::chrono::seconds(60));
	EXPECT_EQ(scheduler.schedule_count(), 1);

	scheduler.add_cron("s2", t2, "0 * * * *");
	EXPECT_EQ(scheduler.schedule_count(), 2);

	scheduler.remove("s1");
	EXPECT_EQ(scheduler.schedule_count(), 1);
}

// ============================================================================
// task_scheduler tests - Callbacks
// ============================================================================

TEST_F(SchedulerTest, OnTaskExecutedCallback) {
	task_scheduler scheduler(client_);

	std::atomic<int> callback_count{0};
	std::string executed_name;
	std::mutex mtx;
	std::condition_variable cv;

	scheduler.on_task_executed([&](const schedule_entry& entry) {
		std::lock_guard<std::mutex> lock(mtx);
		executed_name = entry.name;
		callback_count.fetch_add(1);
		cv.notify_one();
	});

	auto t = task_builder("callback.test").build().unwrap();
	scheduler.add_periodic("callback-schedule", t, std::chrono::seconds(1));

	scheduler.start();

	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait_for(lock, std::chrono::seconds(3), [&] {
			return callback_count.load() >= 1;
		});
	}

	scheduler.stop();

	EXPECT_GE(callback_count.load(), 1);
	EXPECT_EQ(executed_name, "callback-schedule");
}

TEST_F(SchedulerTest, RunCountIncrement) {
	task_scheduler scheduler(client_);

	std::mutex mtx;
	std::condition_variable cv;
	std::atomic<int> runs{0};

	scheduler.on_task_executed([&](const schedule_entry& /* entry */) {
		runs.fetch_add(1);
		cv.notify_one();
	});

	auto t = task_builder("count.test").build().unwrap();
	scheduler.add_periodic("count-schedule", t, std::chrono::seconds(1));

	scheduler.start();

	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait_for(lock, std::chrono::seconds(5), [&] {
			return runs.load() >= 2;
		});
	}

	scheduler.stop();

	auto entry = scheduler.get_schedule("count-schedule");
	ASSERT_TRUE(entry.is_ok());
	EXPECT_GE(entry.unwrap().run_count, 2);
}

// ============================================================================
// task_scheduler tests - Disabled Schedules
// ============================================================================

TEST_F(SchedulerTest, DisabledScheduleDoesNotExecute) {
	task_scheduler scheduler(client_);

	std::atomic<bool> executed{false};

	scheduler.on_task_executed([&](const schedule_entry& /* entry */) {
		executed.store(true);
	});

	auto t = task_builder("disabled.test").build().unwrap();
	scheduler.add_periodic("disabled-schedule", t, std::chrono::seconds(1));

	// Disable before starting
	scheduler.disable("disabled-schedule");

	scheduler.start();

	std::this_thread::sleep_for(std::chrono::seconds(2));

	scheduler.stop();

	EXPECT_FALSE(executed.load());
}

TEST_F(SchedulerTest, ReenableSchedule) {
	task_scheduler scheduler(client_);

	std::atomic<int> execute_count{0};
	std::mutex mtx;
	std::condition_variable cv;

	scheduler.on_task_executed([&](const schedule_entry& /* entry */) {
		execute_count.fetch_add(1);
		cv.notify_one();
	});

	auto t = task_builder("reenable.test").build().unwrap();
	scheduler.add_periodic("reenable-schedule", t, std::chrono::seconds(1));

	scheduler.disable("reenable-schedule");
	scheduler.start();

	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	EXPECT_EQ(execute_count.load(), 0);

	scheduler.enable("reenable-schedule");

	// Use trigger_now to force immediate execution after enabling
	scheduler.trigger_now("reenable-schedule");

	{
		std::unique_lock<std::mutex> lock(mtx);
		cv.wait_for(lock, std::chrono::seconds(3), [&] {
			return execute_count.load() >= 1;
		});
	}

	scheduler.stop();

	EXPECT_GE(execute_count.load(), 1);
}

// ============================================================================
// task_scheduler tests - Thread Safety
// ============================================================================

TEST_F(SchedulerTest, ConcurrentScheduleOperations) {
	task_scheduler scheduler(client_);
	scheduler.start();

	std::atomic<int> add_count{0};
	std::atomic<int> remove_count{0};

	std::vector<std::thread> threads;

	// Add schedules concurrently
	for (int i = 0; i < 5; ++i) {
		threads.emplace_back([&scheduler, &add_count, i]() {
			for (int j = 0; j < 10; ++j) {
				auto t = task_builder("concurrent.task").build().unwrap();
				std::string name = "concurrent-" + std::to_string(i) + "-" + std::to_string(j);
				if (scheduler.add_periodic(name, t, std::chrono::seconds(3600)).is_ok()) {
					add_count.fetch_add(1);
				}
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(add_count.load(), 50);
	EXPECT_EQ(scheduler.schedule_count(), 50);

	// Remove schedules concurrently
	threads.clear();
	for (int i = 0; i < 5; ++i) {
		threads.emplace_back([&scheduler, &remove_count, i]() {
			for (int j = 0; j < 10; ++j) {
				std::string name = "concurrent-" + std::to_string(i) + "-" + std::to_string(j);
				if (scheduler.remove(name).is_ok()) {
					remove_count.fetch_add(1);
				}
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	scheduler.stop();

	EXPECT_EQ(remove_count.load(), 50);
	EXPECT_EQ(scheduler.schedule_count(), 0);
}
