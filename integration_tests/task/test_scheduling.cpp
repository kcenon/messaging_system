// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_scheduling.cpp
 * @brief Integration tests for task scheduling
 *
 * Tests periodic task execution, cron-based scheduling,
 * schedule management (enable/disable), and scheduling edge cases.
 */

#include "task_fixture.h"

#include <gtest/gtest.h>
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>
#include <kcenon/messaging/task/scheduler.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;

using tsk::testing::SchedulerFixture;
using tsk::testing::TaskCounter;
using tsk::testing::wait_for_condition;

class SchedulingTest : public SchedulerFixture {};

// ============================================================================
// Periodic Task Execution
// ============================================================================

TEST_F(SchedulingTest, PeriodicTaskExecution) {
	TaskCounter counter;

	system_->register_handler("schedule.periodic", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.periodic").build();
	ASSERT_TRUE(task_result.is_ok());

	// Schedule to run every 500ms
	auto schedule_result = system_->schedule_periodic(
		"test-periodic",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	);

	ASSERT_TRUE(schedule_result.is_ok()) << schedule_result.error().message;

	// Wait for at least 2 executions
	ASSERT_TRUE(wait_for_condition(
		[&counter]() { return counter.count() >= 2; },
		std::chrono::seconds(5)
	)) << "Expected at least 2 executions, got " << counter.count();

	EXPECT_GE(counter.count(), 2);
}

TEST_F(SchedulingTest, PeriodicTaskWithShortInterval) {
	TaskCounter counter;

	system_->register_handler("schedule.short", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.short").build();
	ASSERT_TRUE(task_result.is_ok());

	// Very short interval (1 second is minimum for seconds precision)
	auto schedule_result = system_->schedule_periodic(
		"test-short",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	);

	ASSERT_TRUE(schedule_result.is_ok()) << schedule_result.error().message;

	// Wait for multiple executions
	std::this_thread::sleep_for(std::chrono::seconds(3));

	EXPECT_GE(counter.count(), 2) << "Expected at least 2 executions with 1s interval";
}

// ============================================================================
// Cron Task Execution
// ============================================================================

TEST_F(SchedulingTest, CronTaskRegistration) {
	system_->register_handler("schedule.cron", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.cron").build();
	ASSERT_TRUE(task_result.is_ok());

	// Schedule for every minute (for testing schedule registration)
	auto schedule_result = system_->schedule_cron(
		"test-cron",
		std::move(task_result).value(),
		"* * * * *"
	);

	ASSERT_TRUE(schedule_result.is_ok()) << schedule_result.error().message;

	// Verify schedule exists
	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);
	EXPECT_TRUE(scheduler->has_schedule("test-cron"));
}

TEST_F(SchedulingTest, CronExpressionValidation) {
	system_->register_handler("schedule.cron_invalid", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.cron_invalid").build();
	ASSERT_TRUE(task_result.is_ok());

	// Invalid cron expression
	auto schedule_result = system_->schedule_cron(
		"test-invalid-cron",
		std::move(task_result).value(),
		"invalid cron expression"
	);

	// Should fail validation
	EXPECT_FALSE(schedule_result.is_ok());
}

TEST_F(SchedulingTest, MultipleCronSchedules) {
	TaskCounter counter1;
	TaskCounter counter2;

	system_->register_handler("schedule.cron1", [&counter1](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter1.increment();
		return cmn::ok(container_module::value_container{});
	});

	system_->register_handler("schedule.cron2", [&counter2](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter2.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task1_result = tsk::task_builder("schedule.cron1").build();
	auto task2_result = tsk::task_builder("schedule.cron2").build();
	ASSERT_TRUE(task1_result.is_ok());
	ASSERT_TRUE(task2_result.is_ok());

	// Register both schedules
	ASSERT_TRUE(system_->schedule_cron(
		"cron-schedule-1",
		std::move(task1_result).value(),
		"* * * * *"
	).is_ok());

	ASSERT_TRUE(system_->schedule_cron(
		"cron-schedule-2",
		std::move(task2_result).value(),
		"* * * * *"
	).is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	EXPECT_EQ(scheduler->schedule_count(), 2);
	EXPECT_TRUE(scheduler->has_schedule("cron-schedule-1"));
	EXPECT_TRUE(scheduler->has_schedule("cron-schedule-2"));
}

// ============================================================================
// Schedule Enable/Disable
// ============================================================================

TEST_F(SchedulingTest, DisableSchedule) {
	TaskCounter counter;

	system_->register_handler("schedule.disable", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.disable").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	// Add periodic schedule
	ASSERT_TRUE(system_->schedule_periodic(
		"test-disable",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	).is_ok());

	// Wait for first execution
	ASSERT_TRUE(wait_for_condition(
		[&counter]() { return counter.count() >= 1; },
		std::chrono::seconds(3)
	));

	size_t count_before_disable = counter.count();

	// Disable the schedule
	auto disable_result = scheduler->disable("test-disable");
	ASSERT_TRUE(disable_result.is_ok()) << disable_result.error().message;

	// Wait and verify no more executions
	std::this_thread::sleep_for(std::chrono::seconds(2));
	size_t count_after_disable = counter.count();

	// Count should not have increased significantly (allow for one in-flight execution)
	EXPECT_LE(count_after_disable, count_before_disable + 1);
}

TEST_F(SchedulingTest, EnableSchedule) {
	TaskCounter counter;

	system_->register_handler("schedule.enable", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.enable").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	// Add and immediately disable
	ASSERT_TRUE(system_->schedule_periodic(
		"test-enable",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	).is_ok());

	ASSERT_TRUE(scheduler->disable("test-enable").is_ok());

	// Wait - should not execute
	std::this_thread::sleep_for(std::chrono::seconds(2));
	size_t count_while_disabled = counter.count();

	// Re-enable
	ASSERT_TRUE(scheduler->enable("test-enable").is_ok());

	// Wait for execution after re-enable
	ASSERT_TRUE(wait_for_condition(
		[&counter, count_while_disabled]() { return counter.count() > count_while_disabled; },
		std::chrono::seconds(3)
	));

	EXPECT_GT(counter.count(), count_while_disabled);
}

TEST_F(SchedulingTest, RemoveSchedule) {
	TaskCounter counter;

	system_->register_handler("schedule.remove", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.remove").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	ASSERT_TRUE(system_->schedule_periodic(
		"test-remove",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	).is_ok());

	EXPECT_TRUE(scheduler->has_schedule("test-remove"));

	// Remove the schedule
	auto remove_result = scheduler->remove("test-remove");
	ASSERT_TRUE(remove_result.is_ok()) << remove_result.error().message;

	EXPECT_FALSE(scheduler->has_schedule("test-remove"));
	EXPECT_EQ(scheduler->schedule_count(), 0);
}

// ============================================================================
// Schedule Query
// ============================================================================

TEST_F(SchedulingTest, ListSchedules) {
	system_->register_handler("schedule.list", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	// Add multiple schedules
	for (int i = 0; i < 3; ++i) {
		auto task_result = tsk::task_builder("schedule.list").build();
		ASSERT_TRUE(task_result.is_ok());

		ASSERT_TRUE(system_->schedule_periodic(
			"schedule-" + std::to_string(i),
			std::move(task_result).value(),
			std::chrono::seconds(60)
		).is_ok());
	}

	auto schedules = scheduler->list_schedules();
	EXPECT_EQ(schedules.size(), 3);
}

TEST_F(SchedulingTest, GetScheduleDetails) {
	system_->register_handler("schedule.details", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.details").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	ASSERT_TRUE(system_->schedule_periodic(
		"detail-schedule",
		std::move(task_result).value(),
		std::chrono::seconds(30)
	).is_ok());

	auto schedule_result = scheduler->get_schedule("detail-schedule");
	ASSERT_TRUE(schedule_result.is_ok());

	const auto& entry = schedule_result.value();
	EXPECT_EQ(entry.name, "detail-schedule");
	EXPECT_TRUE(entry.is_periodic());
	EXPECT_EQ(entry.interval(), std::chrono::seconds(30));
	EXPECT_TRUE(entry.enabled);
}

// ============================================================================
// Trigger Now
// ============================================================================

TEST_F(SchedulingTest, TriggerScheduleImmediately) {
	TaskCounter counter;

	system_->register_handler("schedule.trigger", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.trigger").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	// Schedule with long interval
	ASSERT_TRUE(system_->schedule_periodic(
		"trigger-schedule",
		std::move(task_result).value(),
		std::chrono::seconds(3600)  // 1 hour - won't execute naturally
	).is_ok());

	size_t initial_count = counter.count();

	// Trigger immediately
	auto trigger_result = scheduler->trigger_now("trigger-schedule");
	ASSERT_TRUE(trigger_result.is_ok()) << trigger_result.error().message;

	// Should execute soon
	ASSERT_TRUE(wait_for_condition(
		[&counter, initial_count]() { return counter.count() > initial_count; },
		std::chrono::seconds(5)
	));

	EXPECT_GT(counter.count(), initial_count);
}

// ============================================================================
// Update Schedule
// ============================================================================

TEST_F(SchedulingTest, UpdatePeriodicInterval) {
	system_->register_handler("schedule.update", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.update").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	ASSERT_TRUE(system_->schedule_periodic(
		"update-schedule",
		std::move(task_result).value(),
		std::chrono::seconds(60)
	).is_ok());

	// Update interval
	auto update_result = scheduler->update_interval(
		"update-schedule",
		std::chrono::seconds(30)
	);
	ASSERT_TRUE(update_result.is_ok()) << update_result.error().message;

	// Verify update
	auto schedule_result = scheduler->get_schedule("update-schedule");
	ASSERT_TRUE(schedule_result.is_ok());
	EXPECT_EQ(schedule_result.value().interval(), std::chrono::seconds(30));
}

TEST_F(SchedulingTest, UpdateCronExpression) {
	system_->register_handler("schedule.update_cron", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.update_cron").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	ASSERT_TRUE(system_->schedule_cron(
		"cron-update-schedule",
		std::move(task_result).value(),
		"0 * * * *"
	).is_ok());

	// Update cron expression
	auto update_result = scheduler->update_cron(
		"cron-update-schedule",
		"30 * * * *"
	);
	ASSERT_TRUE(update_result.is_ok()) << update_result.error().message;

	// Verify update
	auto schedule_result = scheduler->get_schedule("cron-update-schedule");
	ASSERT_TRUE(schedule_result.is_ok());
	EXPECT_TRUE(schedule_result.value().is_cron());
	EXPECT_EQ(schedule_result.value().cron_expression(), "30 * * * *");
}

// ============================================================================
// Scheduler Lifecycle
// ============================================================================

TEST_F(SchedulingTest, SchedulerStartStop) {
	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	// Before system start
	EXPECT_FALSE(scheduler->is_running());

	start_system();

	// After system start
	EXPECT_TRUE(scheduler->is_running());

	stop_system();

	// After system stop
	EXPECT_FALSE(scheduler->is_running());
}

TEST_F(SchedulingTest, SchedulerDisabledByConfig) {
	// Create new system with scheduler disabled
	tsk::task_system_config disabled_config;
	disabled_config.worker.concurrency = 2;
	disabled_config.enable_scheduler = false;

	auto no_scheduler_system = std::make_unique<tsk::task_system>(disabled_config);

	// Scheduler should be null
	EXPECT_EQ(no_scheduler_system->scheduler(), nullptr);

	// Schedule operations should fail gracefully
	auto task_result = tsk::task_builder("test.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto schedule_result = no_scheduler_system->schedule_periodic(
		"no-scheduler-test",
		std::move(task_result).value(),
		std::chrono::seconds(60)
	);

	EXPECT_FALSE(schedule_result.is_ok());
}

// ============================================================================
// Duplicate Schedule Names
// ============================================================================

TEST_F(SchedulingTest, DuplicateScheduleName) {
	system_->register_handler("schedule.dup", [](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task1_result = tsk::task_builder("schedule.dup").build();
	auto task2_result = tsk::task_builder("schedule.dup").build();
	ASSERT_TRUE(task1_result.is_ok());
	ASSERT_TRUE(task2_result.is_ok());

	// First registration should succeed
	ASSERT_TRUE(system_->schedule_periodic(
		"duplicate-name",
		std::move(task1_result).value(),
		std::chrono::seconds(60)
	).is_ok());

	// Second registration with same name should fail
	auto duplicate_result = system_->schedule_periodic(
		"duplicate-name",
		std::move(task2_result).value(),
		std::chrono::seconds(30)
	);

	EXPECT_FALSE(duplicate_result.is_ok());
}

// ============================================================================
// Schedule Run Count
// ============================================================================

TEST_F(SchedulingTest, ScheduleRunCount) {
	TaskCounter counter;

	system_->register_handler("schedule.runcount", [&counter](const tsk::task& t, tsk::task_context& ctx) {
		(void)t;
		(void)ctx;
		counter.increment();
		return cmn::ok(container_module::value_container{});
	});

	start_system();

	auto task_result = tsk::task_builder("schedule.runcount").build();
	ASSERT_TRUE(task_result.is_ok());

	auto* scheduler = system_->scheduler();
	ASSERT_NE(scheduler, nullptr);

	ASSERT_TRUE(system_->schedule_periodic(
		"runcount-schedule",
		std::move(task_result).value(),
		std::chrono::seconds(1)
	).is_ok());

	// Wait for several executions
	ASSERT_TRUE(wait_for_condition(
		[&counter]() { return counter.count() >= 3; },
		std::chrono::seconds(10)
	));

	// Check run count in schedule entry
	auto schedule_result = scheduler->get_schedule("runcount-schedule");
	ASSERT_TRUE(schedule_result.is_ok());

	EXPECT_GE(schedule_result.value().run_count, 3);
}
