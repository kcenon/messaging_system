/**
 * @file test_task_event_bridge.cpp
 * @brief Unit tests for task event bridge
 */

#include <gtest/gtest.h>

#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/integration/task_event_bridge.h>
#include <kcenon/messaging/integration/task_events.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace kcenon::messaging::task;
using namespace kcenon::common;

class TaskEventBridgeTest : public ::testing::Test {
protected:
	simple_event_bus& event_bus_ = get_event_bus();

	void SetUp() override { event_bus_.start(); }

	void TearDown() override { event_bus_.stop(); }
};

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST_F(TaskEventBridgeTest, DefaultConstruction) {
	task_event_bridge bridge;
	EXPECT_FALSE(bridge.is_running());
}

TEST_F(TaskEventBridgeTest, StartAndStop) {
	task_event_bridge bridge;

	auto result = bridge.start();
	EXPECT_TRUE(result.is_ok());
	EXPECT_TRUE(bridge.is_running());

	bridge.stop();
	EXPECT_FALSE(bridge.is_running());
}

TEST_F(TaskEventBridgeTest, DoubleStartReturnsError) {
	task_event_bridge bridge;

	auto result1 = bridge.start();
	EXPECT_TRUE(result1.is_ok());

	auto result2 = bridge.start();
	EXPECT_TRUE(result2.is_err());
}

TEST_F(TaskEventBridgeTest, StopWithoutStartIsNoOp) {
	task_event_bridge bridge;
	bridge.stop();  // Should not crash
	EXPECT_FALSE(bridge.is_running());
}

TEST_F(TaskEventBridgeTest, ConfigConstruction) {
	task_event_bridge_config config;
	config.queue_high_watermark_threshold = 500;
	config.enable_progress_events = false;
	config.enable_heartbeat_events = false;

	task_event_bridge bridge(config);

	EXPECT_EQ(bridge.config().queue_high_watermark_threshold, 500);
	EXPECT_FALSE(bridge.config().enable_progress_events);
	EXPECT_FALSE(bridge.config().enable_heartbeat_events);
}

// ============================================================================
// Task Lifecycle Event Tests
// ============================================================================

TEST_F(TaskEventBridgeTest, PublishTaskQueuedEvent) {
	std::atomic<int> event_count{0};
	std::string received_task_id;
	std::string received_task_name;
	std::string received_queue;

	auto sub = event_bus_.subscribe<task_queued_event>(
		[&](const task_queued_event& evt) {
			event_count++;
			received_task_id = evt.task_id;
			received_task_name = evt.task_name;
			received_queue = evt.queue;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_queued("task-123", "email.send", "default");

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_task_id, "task-123");
	EXPECT_EQ(received_task_name, "email.send");
	EXPECT_EQ(received_queue, "default");

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishTaskStartedEvent) {
	std::atomic<int> event_count{0};
	std::string received_worker_id;

	auto sub = event_bus_.subscribe<task_started_event>(
		[&](const task_started_event& evt) {
			event_count++;
			received_worker_id = evt.worker_id;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_started("task-123", "email.send", "default", "worker-1");

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_worker_id, "worker-1");

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishTaskProgressEvent) {
	std::atomic<int> event_count{0};
	double received_progress = 0.0;
	std::string received_message;

	auto sub = event_bus_.subscribe<task_progress_event>(
		[&](const task_progress_event& evt) {
			event_count++;
			received_progress = evt.progress;
			received_message = evt.message;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_progress("task-123", "email.send", 0.5, "Processing...");

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_DOUBLE_EQ(received_progress, 0.5);
	EXPECT_EQ(received_message, "Processing...");

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishTaskSucceededEvent) {
	std::atomic<int> event_count{0};
	std::chrono::milliseconds received_duration{0};

	auto sub = event_bus_.subscribe<task_succeeded_event>(
		[&](const task_succeeded_event& evt) {
			event_count++;
			received_duration = evt.duration;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_succeeded("task-123", "email.send", "default", "worker-1",
							 std::chrono::milliseconds(150));

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_duration.count(), 150);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishTaskFailedEvent) {
	std::atomic<int> event_count{0};
	std::string received_error;
	size_t received_attempt = 0;

	auto sub = event_bus_.subscribe<task_failed_event>(
		[&](const task_failed_event& evt) {
			event_count++;
			received_error = evt.error_message;
			received_attempt = evt.attempt;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_failed("task-123", "email.send", "default", "worker-1",
						  "Connection timeout", "stack trace", 2);

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_error, "Connection timeout");
	EXPECT_EQ(received_attempt, 2);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishTaskRetryingEvent) {
	std::atomic<int> event_count{0};
	size_t received_attempt = 0;
	size_t received_max_retries = 0;

	auto sub = event_bus_.subscribe<task_retrying_event>(
		[&](const task_retrying_event& evt) {
			event_count++;
			received_attempt = evt.attempt;
			received_max_retries = evt.max_retries;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_retrying("task-123", "email.send", "default", 1, 3,
							std::chrono::milliseconds(1000));

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_attempt, 1);
	EXPECT_EQ(received_max_retries, 3);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishTaskCancelledEvent) {
	std::atomic<int> event_count{0};
	std::string received_reason;

	auto sub = event_bus_.subscribe<task_cancelled_event>(
		[&](const task_cancelled_event& evt) {
			event_count++;
			received_reason = evt.reason;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_cancelled("task-123", "email.send", "default", "User requested");

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_reason, "User requested");

	event_bus_.unsubscribe(sub);
}

// ============================================================================
// Worker Event Tests
// ============================================================================

TEST_F(TaskEventBridgeTest, PublishWorkerOnlineEvent) {
	std::atomic<int> event_count{0};
	std::string received_worker_id;
	std::vector<std::string> received_queues;
	size_t received_concurrency = 0;

	auto sub = event_bus_.subscribe<worker_online_event>(
		[&](const worker_online_event& evt) {
			event_count++;
			received_worker_id = evt.worker_id;
			received_queues = evt.queues;
			received_concurrency = evt.concurrency;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_worker_online("worker-1", {"default", "high-priority"}, 4);

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_worker_id, "worker-1");
	ASSERT_EQ(received_queues.size(), 2);
	EXPECT_EQ(received_queues[0], "default");
	EXPECT_EQ(received_queues[1], "high-priority");
	EXPECT_EQ(received_concurrency, 4);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishWorkerOfflineEvent) {
	std::atomic<int> event_count{0};
	std::string received_reason;

	auto sub = event_bus_.subscribe<worker_offline_event>(
		[&](const worker_offline_event& evt) {
			event_count++;
			received_reason = evt.reason;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_worker_offline("worker-1", "Shutdown requested");

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_reason, "Shutdown requested");

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishWorkerHeartbeatEvent) {
	std::atomic<int> event_count{0};
	size_t received_active = 0;
	size_t received_completed = 0;
	size_t received_failed = 0;

	auto sub = event_bus_.subscribe<worker_heartbeat_event>(
		[&](const worker_heartbeat_event& evt) {
			event_count++;
			received_active = evt.active_tasks;
			received_completed = evt.completed_tasks;
			received_failed = evt.failed_tasks;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_worker_heartbeat("worker-1", 2, 100, 5);

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_active, 2);
	EXPECT_EQ(received_completed, 100);
	EXPECT_EQ(received_failed, 5);

	event_bus_.unsubscribe(sub);
}

// ============================================================================
// Queue Event Tests
// ============================================================================

TEST_F(TaskEventBridgeTest, PublishQueueHighWatermarkEvent) {
	std::atomic<int> event_count{0};
	std::string received_queue;
	size_t received_size = 0;
	size_t received_threshold = 0;

	auto sub = event_bus_.subscribe<queue_high_watermark_event>(
		[&](const queue_high_watermark_event& evt) {
			event_count++;
			received_queue = evt.queue;
			received_size = evt.current_size;
			received_threshold = evt.threshold;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_queue_high_watermark("default", 950, 1000);

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_queue, "default");
	EXPECT_EQ(received_size, 950);
	EXPECT_EQ(received_threshold, 1000);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, PublishQueueEmptyEvent) {
	std::atomic<int> event_count{0};
	std::string received_queue;

	auto sub = event_bus_.subscribe<queue_empty_event>(
		[&](const queue_empty_event& evt) {
			event_count++;
			received_queue = evt.queue;
		});

	task_event_bridge bridge;
	bridge.start();

	bridge.on_queue_empty("default");

	EXPECT_EQ(event_count.load(), 1);
	EXPECT_EQ(received_queue, "default");

	event_bus_.unsubscribe(sub);
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(TaskEventBridgeTest, ProgressEventsCanBeDisabled) {
	std::atomic<int> event_count{0};

	auto sub = event_bus_.subscribe<task_progress_event>(
		[&](const task_progress_event&) { event_count++; });

	task_event_bridge_config config;
	config.enable_progress_events = false;

	task_event_bridge bridge(config);
	bridge.start();

	bridge.on_task_progress("task-123", "email.send", 0.5, "Processing...");

	EXPECT_EQ(event_count.load(), 0);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, HeartbeatEventsCanBeDisabled) {
	std::atomic<int> event_count{0};

	auto sub = event_bus_.subscribe<worker_heartbeat_event>(
		[&](const worker_heartbeat_event&) { event_count++; });

	task_event_bridge_config config;
	config.enable_heartbeat_events = false;

	task_event_bridge bridge(config);
	bridge.start();

	bridge.on_worker_heartbeat("worker-1", 1, 10, 0);

	EXPECT_EQ(event_count.load(), 0);

	event_bus_.unsubscribe(sub);
}

// ============================================================================
// Running State Tests
// ============================================================================

TEST_F(TaskEventBridgeTest, EventsNotPublishedWhenNotRunning) {
	std::atomic<int> event_count{0};

	auto sub = event_bus_.subscribe<task_queued_event>(
		[&](const task_queued_event&) { event_count++; });

	task_event_bridge bridge;
	// Not started

	bridge.on_task_queued("task-123", "email.send", "default");

	EXPECT_EQ(event_count.load(), 0);

	event_bus_.unsubscribe(sub);
}

TEST_F(TaskEventBridgeTest, EventsNotPublishedAfterStop) {
	std::atomic<int> event_count{0};

	auto sub = event_bus_.subscribe<task_queued_event>(
		[&](const task_queued_event&) { event_count++; });

	task_event_bridge bridge;
	bridge.start();

	bridge.on_task_queued("task-123", "email.send", "default");
	EXPECT_EQ(event_count.load(), 1);

	bridge.stop();

	bridge.on_task_queued("task-456", "email.send", "default");
	EXPECT_EQ(event_count.load(), 1);  // Still 1

	event_bus_.unsubscribe(sub);
}

// ============================================================================
// Task Event Types Constants Tests
// ============================================================================

TEST(TaskEventTypesTest, EventTypeConstants) {
	EXPECT_STREQ(task_event_types::task_queued, "task.queued");
	EXPECT_STREQ(task_event_types::task_started, "task.started");
	EXPECT_STREQ(task_event_types::task_progress, "task.progress");
	EXPECT_STREQ(task_event_types::task_succeeded, "task.succeeded");
	EXPECT_STREQ(task_event_types::task_failed, "task.failed");
	EXPECT_STREQ(task_event_types::task_retrying, "task.retrying");
	EXPECT_STREQ(task_event_types::task_cancelled, "task.cancelled");
	EXPECT_STREQ(task_event_types::worker_online, "worker.online");
	EXPECT_STREQ(task_event_types::worker_offline, "worker.offline");
	EXPECT_STREQ(task_event_types::worker_heartbeat, "worker.heartbeat");
	EXPECT_STREQ(task_event_types::queue_high_watermark, "queue.high_watermark");
	EXPECT_STREQ(task_event_types::queue_empty, "queue.empty");
}

// ============================================================================
// Event Construction Tests
// ============================================================================

TEST(TaskEventsTest, TaskQueuedEventConstruction) {
	auto now = std::chrono::system_clock::now();
	auto eta = now + std::chrono::hours(1);

	task_queued_event evt("task-123", "email.send", "default", eta);

	EXPECT_EQ(evt.task_id, "task-123");
	EXPECT_EQ(evt.task_name, "email.send");
	EXPECT_EQ(evt.queue, "default");
	EXPECT_TRUE(evt.eta.has_value());
	EXPECT_GE(evt.timestamp, now);
}

TEST(TaskEventsTest, TaskSucceededEventConstruction) {
	task_succeeded_event evt("task-123", "email.send", "default", "worker-1",
							 std::chrono::milliseconds(150));

	EXPECT_EQ(evt.task_id, "task-123");
	EXPECT_EQ(evt.duration.count(), 150);
}

TEST(TaskEventsTest, WorkerOnlineEventConstruction) {
	worker_online_event evt("worker-1", {"default", "priority"}, 4);

	EXPECT_EQ(evt.worker_id, "worker-1");
	EXPECT_EQ(evt.queues.size(), 2);
	EXPECT_EQ(evt.concurrency, 4);
}
