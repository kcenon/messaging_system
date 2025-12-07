// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_events.h
 * @brief Task event definitions for distributed task queue system
 *
 * Defines event types for task lifecycle, worker status, and queue state
 * that can be published to the common_system event bus.
 */

#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace kcenon::messaging::task {

// ============================================================================
// Task Event Type Constants
// ============================================================================

/**
 * @namespace task_event_types
 * @brief String constants for task event type identification
 */
namespace task_event_types {
	// Task lifecycle events
	static constexpr auto task_queued = "task.queued";
	static constexpr auto task_started = "task.started";
	static constexpr auto task_progress = "task.progress";
	static constexpr auto task_succeeded = "task.succeeded";
	static constexpr auto task_failed = "task.failed";
	static constexpr auto task_retrying = "task.retrying";
	static constexpr auto task_cancelled = "task.cancelled";

	// Worker events
	static constexpr auto worker_online = "worker.online";
	static constexpr auto worker_offline = "worker.offline";
	static constexpr auto worker_heartbeat = "worker.heartbeat";

	// Queue events
	static constexpr auto queue_high_watermark = "queue.high_watermark";
	static constexpr auto queue_empty = "queue.empty";
}  // namespace task_event_types

// ============================================================================
// Task Lifecycle Events
// ============================================================================

/**
 * @struct task_queued_event
 * @brief Event published when a task is added to the queue
 */
struct task_queued_event {
	std::string task_id;
	std::string task_name;
	std::string queue;
	std::chrono::system_clock::time_point timestamp;
	std::optional<std::chrono::system_clock::time_point> eta;

	task_queued_event(const std::string& id, const std::string& name,
					  const std::string& queue_name,
					  std::optional<std::chrono::system_clock::time_point> scheduled_eta = std::nullopt)
		: task_id(id),
		  task_name(name),
		  queue(queue_name),
		  timestamp(std::chrono::system_clock::now()),
		  eta(scheduled_eta) {}
};

/**
 * @struct task_started_event
 * @brief Event published when a task begins execution
 */
struct task_started_event {
	std::string task_id;
	std::string task_name;
	std::string queue;
	std::string worker_id;
	std::chrono::system_clock::time_point timestamp;

	task_started_event(const std::string& id, const std::string& name,
					   const std::string& queue_name, const std::string& worker)
		: task_id(id),
		  task_name(name),
		  queue(queue_name),
		  worker_id(worker),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct task_progress_event
 * @brief Event published when task progress is updated
 */
struct task_progress_event {
	std::string task_id;
	std::string task_name;
	double progress;  // 0.0 to 1.0
	std::string message;
	std::chrono::system_clock::time_point timestamp;

	task_progress_event(const std::string& id, const std::string& name,
						double prog, const std::string& msg = "")
		: task_id(id),
		  task_name(name),
		  progress(prog),
		  message(msg),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct task_succeeded_event
 * @brief Event published when a task completes successfully
 */
struct task_succeeded_event {
	std::string task_id;
	std::string task_name;
	std::string queue;
	std::string worker_id;
	std::chrono::milliseconds duration;
	std::chrono::system_clock::time_point timestamp;

	task_succeeded_event(const std::string& id, const std::string& name,
						 const std::string& queue_name, const std::string& worker,
						 std::chrono::milliseconds dur)
		: task_id(id),
		  task_name(name),
		  queue(queue_name),
		  worker_id(worker),
		  duration(dur),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct task_failed_event
 * @brief Event published when a task fails
 */
struct task_failed_event {
	std::string task_id;
	std::string task_name;
	std::string queue;
	std::string worker_id;
	std::string error_message;
	std::string traceback;
	size_t attempt;
	std::chrono::system_clock::time_point timestamp;

	task_failed_event(const std::string& id, const std::string& name,
					  const std::string& queue_name, const std::string& worker,
					  const std::string& error, const std::string& trace,
					  size_t attempt_count)
		: task_id(id),
		  task_name(name),
		  queue(queue_name),
		  worker_id(worker),
		  error_message(error),
		  traceback(trace),
		  attempt(attempt_count),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct task_retrying_event
 * @brief Event published when a task is scheduled for retry
 */
struct task_retrying_event {
	std::string task_id;
	std::string task_name;
	std::string queue;
	size_t attempt;
	size_t max_retries;
	std::chrono::milliseconds retry_delay;
	std::chrono::system_clock::time_point timestamp;

	task_retrying_event(const std::string& id, const std::string& name,
						const std::string& queue_name, size_t attempt_count,
						size_t max, std::chrono::milliseconds delay)
		: task_id(id),
		  task_name(name),
		  queue(queue_name),
		  attempt(attempt_count),
		  max_retries(max),
		  retry_delay(delay),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct task_cancelled_event
 * @brief Event published when a task is cancelled
 */
struct task_cancelled_event {
	std::string task_id;
	std::string task_name;
	std::string queue;
	std::string reason;
	std::chrono::system_clock::time_point timestamp;

	task_cancelled_event(const std::string& id, const std::string& name,
						 const std::string& queue_name, const std::string& cancel_reason = "")
		: task_id(id),
		  task_name(name),
		  queue(queue_name),
		  reason(cancel_reason),
		  timestamp(std::chrono::system_clock::now()) {}
};

// ============================================================================
// Worker Events
// ============================================================================

/**
 * @struct worker_online_event
 * @brief Event published when a worker comes online
 */
struct worker_online_event {
	std::string worker_id;
	std::vector<std::string> queues;
	size_t concurrency;
	std::chrono::system_clock::time_point timestamp;

	worker_online_event(const std::string& id, const std::vector<std::string>& queue_list,
						size_t conc)
		: worker_id(id),
		  queues(queue_list),
		  concurrency(conc),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct worker_offline_event
 * @brief Event published when a worker goes offline
 */
struct worker_offline_event {
	std::string worker_id;
	std::string reason;
	std::chrono::system_clock::time_point timestamp;

	worker_offline_event(const std::string& id, const std::string& offline_reason = "")
		: worker_id(id),
		  reason(offline_reason),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct worker_heartbeat_event
 * @brief Event published periodically by active workers
 */
struct worker_heartbeat_event {
	std::string worker_id;
	size_t active_tasks;
	size_t completed_tasks;
	size_t failed_tasks;
	std::chrono::system_clock::time_point timestamp;

	worker_heartbeat_event(const std::string& id, size_t active, size_t completed, size_t failed)
		: worker_id(id),
		  active_tasks(active),
		  completed_tasks(completed),
		  failed_tasks(failed),
		  timestamp(std::chrono::system_clock::now()) {}
};

// ============================================================================
// Queue Events
// ============================================================================

/**
 * @struct queue_high_watermark_event
 * @brief Event published when queue size exceeds a threshold
 */
struct queue_high_watermark_event {
	std::string queue;
	size_t current_size;
	size_t threshold;
	std::chrono::system_clock::time_point timestamp;

	queue_high_watermark_event(const std::string& queue_name, size_t size, size_t thresh)
		: queue(queue_name),
		  current_size(size),
		  threshold(thresh),
		  timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @struct queue_empty_event
 * @brief Event published when a queue becomes empty
 */
struct queue_empty_event {
	std::string queue;
	std::chrono::system_clock::time_point timestamp;

	explicit queue_empty_event(const std::string& queue_name)
		: queue(queue_name),
		  timestamp(std::chrono::system_clock::now()) {}
};

}  // namespace kcenon::messaging::task
