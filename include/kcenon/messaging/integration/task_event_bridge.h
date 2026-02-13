// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_event_bridge.h
 * @brief Bridge between task system and common_system event bus
 *
 * This header provides integration between the messaging_system's task module
 * and common_system's event bus, enabling cross-module task event communication.
 */

#pragma once

#include <kcenon/common/patterns/event_bus.h>
#include <kcenon/messaging/error/messaging_error_category.h>
#include <kcenon/messaging/integration/task_events.h>
#include <kcenon/messaging/task/task.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace kcenon::messaging::task {

/**
 * @struct task_event_bridge_config
 * @brief Configuration for task event bridge
 */
struct task_event_bridge_config {
	size_t queue_high_watermark_threshold = 1000;
	bool enable_progress_events = true;
	bool enable_heartbeat_events = true;
	std::chrono::milliseconds heartbeat_interval{30000};
};

/**
 * @class task_event_bridge
 * @brief Bridge between task system and common_system event bus
 *
 * This class publishes task lifecycle events to the common_system event bus,
 * enabling other modules to react to task events such as task completion,
 * failure, worker status changes, and queue state transitions.
 *
 * Example usage:
 * @code
 * task_event_bridge bridge;
 * bridge.start();
 *
 * // Subscribe to task events via common event bus
 * auto& event_bus = common::get_event_bus();
 * event_bus.subscribe<task_succeeded_event>([](const auto& evt) {
 *     std::cout << "Task " << evt.task_id << " succeeded in "
 *               << evt.duration.count() << "ms" << std::endl;
 * });
 *
 * // Publish events from worker
 * bridge.on_task_started("task-123", "email.send", "default", "worker-1");
 * // ... task executes ...
 * bridge.on_task_succeeded("task-123", "email.send", "default", "worker-1",
 *                          std::chrono::milliseconds(150));
 * @endcode
 */
class task_event_bridge {
public:
	/**
	 * @brief Construct task event bridge with default configuration
	 */
	task_event_bridge();

	/**
	 * @brief Construct task event bridge with configuration
	 * @param config Bridge configuration
	 */
	explicit task_event_bridge(task_event_bridge_config config);

	/**
	 * @brief Destructor - stops the bridge
	 */
	~task_event_bridge();

	// Non-copyable, non-movable (contains reference and atomic)
	task_event_bridge(const task_event_bridge&) = delete;
	task_event_bridge& operator=(const task_event_bridge&) = delete;
	task_event_bridge(task_event_bridge&&) = delete;
	task_event_bridge& operator=(task_event_bridge&&) = delete;

	// ========================================================================
	// Lifecycle
	// ========================================================================

	/**
	 * @brief Start the event bridge
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the event bridge
	 */
	void stop();

	/**
	 * @brief Check if bridge is running
	 * @return true if running
	 */
	bool is_running() const;

	// ========================================================================
	// Task Lifecycle Events
	// ========================================================================

	/**
	 * @brief Notify that a task was queued
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param queue Queue name
	 * @param eta Optional scheduled execution time
	 */
	void on_task_queued(const std::string& task_id,
						const std::string& task_name,
						const std::string& queue,
						std::optional<std::chrono::system_clock::time_point> eta = std::nullopt);

	/**
	 * @brief Notify that a task started execution
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param queue Queue name
	 * @param worker_id Worker identifier
	 */
	void on_task_started(const std::string& task_id,
						 const std::string& task_name,
						 const std::string& queue,
						 const std::string& worker_id);

	/**
	 * @brief Notify task progress update
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param progress Progress value (0.0 to 1.0)
	 * @param message Optional progress message
	 */
	void on_task_progress(const std::string& task_id,
						  const std::string& task_name,
						  double progress,
						  const std::string& message = "");

	/**
	 * @brief Notify that a task completed successfully
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param queue Queue name
	 * @param worker_id Worker identifier
	 * @param duration Task execution duration
	 */
	void on_task_succeeded(const std::string& task_id,
						   const std::string& task_name,
						   const std::string& queue,
						   const std::string& worker_id,
						   std::chrono::milliseconds duration);

	/**
	 * @brief Notify that a task failed
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param queue Queue name
	 * @param worker_id Worker identifier
	 * @param error_message Error description
	 * @param traceback Stack trace or error details
	 * @param attempt Current attempt number
	 */
	void on_task_failed(const std::string& task_id,
						const std::string& task_name,
						const std::string& queue,
						const std::string& worker_id,
						const std::string& error_message,
						const std::string& traceback,
						size_t attempt);

	/**
	 * @brief Notify that a task is being retried
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param queue Queue name
	 * @param attempt Current attempt number
	 * @param max_retries Maximum retry count
	 * @param retry_delay Delay before next retry
	 */
	void on_task_retrying(const std::string& task_id,
						  const std::string& task_name,
						  const std::string& queue,
						  size_t attempt,
						  size_t max_retries,
						  std::chrono::milliseconds retry_delay);

	/**
	 * @brief Notify that a task was cancelled
	 * @param task_id Task identifier
	 * @param task_name Task handler name
	 * @param queue Queue name
	 * @param reason Cancellation reason
	 */
	void on_task_cancelled(const std::string& task_id,
						   const std::string& task_name,
						   const std::string& queue,
						   const std::string& reason = "");

	// ========================================================================
	// Worker Events
	// ========================================================================

	/**
	 * @brief Notify that a worker came online
	 * @param worker_id Worker identifier
	 * @param queues List of queues the worker handles
	 * @param concurrency Number of concurrent tasks
	 */
	void on_worker_online(const std::string& worker_id,
						  const std::vector<std::string>& queues,
						  size_t concurrency);

	/**
	 * @brief Notify that a worker went offline
	 * @param worker_id Worker identifier
	 * @param reason Reason for going offline
	 */
	void on_worker_offline(const std::string& worker_id,
						   const std::string& reason = "");

	/**
	 * @brief Publish worker heartbeat
	 * @param worker_id Worker identifier
	 * @param active_tasks Number of currently executing tasks
	 * @param completed_tasks Total completed tasks
	 * @param failed_tasks Total failed tasks
	 */
	void on_worker_heartbeat(const std::string& worker_id,
							 size_t active_tasks,
							 size_t completed_tasks,
							 size_t failed_tasks);

	// ========================================================================
	// Queue Events
	// ========================================================================

	/**
	 * @brief Notify queue high watermark reached
	 * @param queue Queue name
	 * @param current_size Current queue size
	 * @param threshold Watermark threshold
	 */
	void on_queue_high_watermark(const std::string& queue,
								 size_t current_size,
								 size_t threshold);

	/**
	 * @brief Notify that a queue became empty
	 * @param queue Queue name
	 */
	void on_queue_empty(const std::string& queue);

	// ========================================================================
	// Utilities
	// ========================================================================

	/**
	 * @brief Get the event bus reference
	 * @return Reference to the common event bus
	 */
	common::simple_event_bus& get_event_bus();

	/**
	 * @brief Get current configuration
	 * @return Configuration reference
	 */
	const task_event_bridge_config& config() const;

private:
	task_event_bridge_config config_;
	common::simple_event_bus& event_bus_;
	std::atomic<bool> running_{false};
};

// ============================================================================
// Inline Implementation
// ============================================================================

inline task_event_bridge::task_event_bridge()
	: config_(),
	  event_bus_(common::get_event_bus()) {}

inline task_event_bridge::task_event_bridge(task_event_bridge_config config)
	: config_(std::move(config)),
	  event_bus_(common::get_event_bus()) {}

inline task_event_bridge::~task_event_bridge() {
	stop();
}

inline common::VoidResult task_event_bridge::start() {
	if (running_) {
		return common::VoidResult::err(
			make_typed_error_code(messaging_error_category::already_running));
	}
	running_ = true;
	return common::ok();
}

inline void task_event_bridge::stop() {
	running_ = false;
}

inline bool task_event_bridge::is_running() const {
	return running_.load();
}

// Task Lifecycle Events
inline void task_event_bridge::on_task_queued(
	const std::string& task_id,
	const std::string& task_name,
	const std::string& queue,
	std::optional<std::chrono::system_clock::time_point> eta) {
	if (running_) {
		event_bus_.publish(task_queued_event{task_id, task_name, queue, eta});
	}
}

inline void task_event_bridge::on_task_started(
	const std::string& task_id,
	const std::string& task_name,
	const std::string& queue,
	const std::string& worker_id) {
	if (running_) {
		event_bus_.publish(task_started_event{task_id, task_name, queue, worker_id});
	}
}

inline void task_event_bridge::on_task_progress(
	const std::string& task_id,
	const std::string& task_name,
	double progress,
	const std::string& message) {
	if (running_ && config_.enable_progress_events) {
		event_bus_.publish(task_progress_event{task_id, task_name, progress, message});
	}
}

inline void task_event_bridge::on_task_succeeded(
	const std::string& task_id,
	const std::string& task_name,
	const std::string& queue,
	const std::string& worker_id,
	std::chrono::milliseconds duration) {
	if (running_) {
		event_bus_.publish(task_succeeded_event{task_id, task_name, queue, worker_id, duration});
	}
}

inline void task_event_bridge::on_task_failed(
	const std::string& task_id,
	const std::string& task_name,
	const std::string& queue,
	const std::string& worker_id,
	const std::string& error_message,
	const std::string& traceback,
	size_t attempt) {
	if (running_) {
		event_bus_.publish(task_failed_event{task_id, task_name, queue, worker_id,
											 error_message, traceback, attempt});
	}
}

inline void task_event_bridge::on_task_retrying(
	const std::string& task_id,
	const std::string& task_name,
	const std::string& queue,
	size_t attempt,
	size_t max_retries,
	std::chrono::milliseconds retry_delay) {
	if (running_) {
		event_bus_.publish(task_retrying_event{task_id, task_name, queue,
											   attempt, max_retries, retry_delay});
	}
}

inline void task_event_bridge::on_task_cancelled(
	const std::string& task_id,
	const std::string& task_name,
	const std::string& queue,
	const std::string& reason) {
	if (running_) {
		event_bus_.publish(task_cancelled_event{task_id, task_name, queue, reason});
	}
}

// Worker Events
inline void task_event_bridge::on_worker_online(
	const std::string& worker_id,
	const std::vector<std::string>& queues,
	size_t concurrency) {
	if (running_) {
		event_bus_.publish(worker_online_event{worker_id, queues, concurrency});
	}
}

inline void task_event_bridge::on_worker_offline(
	const std::string& worker_id,
	const std::string& reason) {
	if (running_) {
		event_bus_.publish(worker_offline_event{worker_id, reason});
	}
}

inline void task_event_bridge::on_worker_heartbeat(
	const std::string& worker_id,
	size_t active_tasks,
	size_t completed_tasks,
	size_t failed_tasks) {
	if (running_ && config_.enable_heartbeat_events) {
		event_bus_.publish(worker_heartbeat_event{worker_id, active_tasks,
												  completed_tasks, failed_tasks});
	}
}

// Queue Events
inline void task_event_bridge::on_queue_high_watermark(
	const std::string& queue,
	size_t current_size,
	size_t threshold) {
	if (running_) {
		event_bus_.publish(queue_high_watermark_event{queue, current_size, threshold});
	}
}

inline void task_event_bridge::on_queue_empty(const std::string& queue) {
	if (running_) {
		event_bus_.publish(queue_empty_event{queue});
	}
}

// Utilities
inline common::simple_event_bus& task_event_bridge::get_event_bus() {
	return event_bus_;
}

inline const task_event_bridge_config& task_event_bridge::config() const {
	return config_;
}

}  // namespace kcenon::messaging::task
