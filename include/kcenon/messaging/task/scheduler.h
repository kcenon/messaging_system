// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file scheduler.h
 * @brief Task scheduler for periodic and cron-based task execution
 *
 * Provides scheduling capabilities for automatic task execution based on
 * fixed intervals or cron expressions. Schedules are managed and persisted
 * internally, allowing tasks to be executed according to their schedules.
 *
 * C++20 Concepts are used for type-safe callback registration, providing
 * clearer compile-time error messages for callback type mismatches.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/cron_parser.h>
#include <kcenon/common/patterns/result.h>
#include <kcenon/thread/core/thread_base.h>

#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace kcenon::messaging::task {

// Forward declaration for internal worker class
class scheduler_worker;

// Forward declaration for schedule_entry
struct schedule_entry;

// =============================================================================
// C++20 Concepts for Scheduler Callbacks
// =============================================================================

/**
 * @concept ScheduleEventCallable
 * @brief A callable type for schedule event callbacks.
 *
 * Types satisfying this concept can be invoked with a const schedule_entry&
 * reference. This replaces std::function-based constraints with clearer
 * compile-time error messages.
 *
 * @tparam F The callable type to validate
 *
 * Example usage:
 * @code
 * template<ScheduleEventCallable Callback>
 * void on_task_executed(Callback&& callback) {
 *     // Callback is guaranteed to be callable with (const schedule_entry&)
 * }
 * @endcode
 */
template<typename F>
concept ScheduleEventCallable = std::invocable<F, const schedule_entry&>;

/**
 * @struct schedule_entry
 * @brief Represents a scheduled task configuration
 *
 * Contains all information needed to schedule and execute a task
 * on a recurring basis, either at fixed intervals or using cron expressions.
 */
struct schedule_entry {
	std::string name;  ///< Unique schedule identifier
	task task_template;  ///< Task template to execute

	/**
	 * @brief Schedule definition: fixed interval or cron expression
	 *
	 * - std::chrono::seconds: Execute at fixed intervals
	 * - std::string: Cron expression (5-field format)
	 */
	std::variant<std::chrono::seconds, std::string> schedule;

	bool enabled = true;  ///< Whether the schedule is active

	std::optional<std::chrono::system_clock::time_point> last_run;  ///< Last execution time
	std::optional<std::chrono::system_clock::time_point> next_run;  ///< Next scheduled execution

	size_t run_count = 0;  ///< Number of times the task has been executed
	size_t failure_count = 0;  ///< Number of execution failures

	/**
	 * @brief Check if the schedule uses cron expression
	 * @return true if schedule is cron-based
	 */
	bool is_cron() const {
		return std::holds_alternative<std::string>(schedule);
	}

	/**
	 * @brief Check if the schedule uses fixed interval
	 * @return true if schedule is interval-based
	 */
	bool is_periodic() const {
		return std::holds_alternative<std::chrono::seconds>(schedule);
	}

	/**
	 * @brief Get the cron expression
	 * @return Cron expression string or empty if not cron-based
	 */
	std::string cron_expression() const {
		if (is_cron()) {
			return std::get<std::string>(schedule);
		}
		return "";
	}

	/**
	 * @brief Get the interval duration
	 * @return Interval duration or zero if not interval-based
	 */
	std::chrono::seconds interval() const {
		if (is_periodic()) {
			return std::get<std::chrono::seconds>(schedule);
		}
		return std::chrono::seconds{0};
	}
};

/**
 * @class task_scheduler
 * @brief Scheduler for periodic and cron-based task execution
 *
 * The task_scheduler manages scheduled tasks that execute at regular intervals
 * or according to cron expressions. It uses a background thread to monitor
 * schedules and submit tasks to the task_client when due.
 *
 * Thread Safety:
 * - All public methods are thread-safe
 * - The scheduler runs in its own background thread
 *
 * @example
 * task_scheduler scheduler(client);
 *
 * // Execute every 5 minutes
 * scheduler.add_periodic(
 *     "cleanup-temp",
 *     task_builder("cleanup.temp").build().value(),
 *     std::chrono::minutes(5)
 * );
 *
 * // Execute daily at 3 AM
 * scheduler.add_cron(
 *     "daily-report",
 *     task_builder("report.daily").build().value(),
 *     "0 3 * * *"
 * );
 *
 * scheduler.start();
 *
 * // Later...
 * scheduler.stop();
 */
class task_scheduler {
	friend class scheduler_worker;
public:
	/**
	 * @brief Construct a task scheduler
	 * @param client Shared pointer to task client for submitting tasks
	 */
	explicit task_scheduler(std::shared_ptr<task_client> client);

	/**
	 * @brief Destructor - stops the scheduler if running
	 */
	~task_scheduler();

	// Non-copyable
	task_scheduler(const task_scheduler&) = delete;
	task_scheduler& operator=(const task_scheduler&) = delete;

	// Move operations
	task_scheduler(task_scheduler&& other) noexcept;
	task_scheduler& operator=(task_scheduler&& other) noexcept;

	// ========================================================================
	// Schedule registration
	// ========================================================================

	/**
	 * @brief Add a periodic schedule (fixed interval)
	 *
	 * The task will be executed at fixed intervals. The first execution
	 * happens immediately after the scheduler starts (or after adding if
	 * already started).
	 *
	 * @param name Unique schedule identifier
	 * @param task_template Task to execute
	 * @param interval Time between executions
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult add_periodic(
		const std::string& name,
		task task_template,
		std::chrono::seconds interval
	);

	/**
	 * @brief Add a cron-based schedule
	 *
	 * The task will be executed according to the cron expression.
	 * Uses standard 5-field cron format: minute hour day month weekday
	 *
	 * @param name Unique schedule identifier
	 * @param task_template Task to execute
	 * @param cron_expression Cron expression (e.g., "0 3 * * *" for 3 AM daily)
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult add_cron(
		const std::string& name,
		task task_template,
		const std::string& cron_expression
	);

	// ========================================================================
	// Schedule management
	// ========================================================================

	/**
	 * @brief Remove a schedule
	 * @param name Schedule identifier
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult remove(const std::string& name);

	/**
	 * @brief Enable a disabled schedule
	 * @param name Schedule identifier
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult enable(const std::string& name);

	/**
	 * @brief Disable a schedule without removing it
	 * @param name Schedule identifier
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult disable(const std::string& name);

	/**
	 * @brief Trigger immediate execution of a schedule
	 *
	 * Executes the task immediately regardless of schedule timing.
	 * Does not affect the regular schedule.
	 *
	 * @param name Schedule identifier
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult trigger_now(const std::string& name);

	/**
	 * @brief Update the interval of a periodic schedule
	 * @param name Schedule identifier
	 * @param interval New interval
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult update_interval(
		const std::string& name,
		std::chrono::seconds interval
	);

	/**
	 * @brief Update the cron expression of a cron schedule
	 * @param name Schedule identifier
	 * @param cron_expression New cron expression
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult update_cron(
		const std::string& name,
		const std::string& cron_expression
	);

	// ========================================================================
	// Lifecycle management
	// ========================================================================

	/**
	 * @brief Start the scheduler
	 *
	 * Starts the background thread that monitors and executes schedules.
	 * Has no effect if already running.
	 *
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the scheduler
	 *
	 * Stops the background thread and waits for it to finish.
	 * Has no effect if not running.
	 *
	 * @return VoidResult indicating success or error
	 */
	common::VoidResult stop();

	/**
	 * @brief Check if the scheduler is running
	 * @return true if the scheduler is active
	 */
	bool is_running() const;

	// ========================================================================
	// Query
	// ========================================================================

	/**
	 * @brief List all registered schedules
	 * @return Vector of schedule entries
	 */
	std::vector<schedule_entry> list_schedules() const;

	/**
	 * @brief Get a specific schedule by name
	 * @param name Schedule identifier
	 * @return Result containing the schedule entry or error
	 */
	common::Result<schedule_entry> get_schedule(const std::string& name) const;

	/**
	 * @brief Get the number of registered schedules
	 * @return Number of schedules
	 */
	size_t schedule_count() const;

	/**
	 * @brief Check if a schedule exists
	 * @param name Schedule identifier
	 * @return true if the schedule exists
	 */
	bool has_schedule(const std::string& name) const;

	// ========================================================================
	// Event callbacks
	// ========================================================================

	/**
	 * @brief Callback type for schedule events
	 */
	using schedule_callback = std::function<void(const schedule_entry&)>;

	/**
	 * @brief Set callback for task execution events
	 * @param callback Function called when a scheduled task is executed
	 */
	void on_task_executed(schedule_callback callback);

	/**
	 * @brief Set callback for task execution events using C++20 concept constraint
	 *
	 * This overload accepts any callable that satisfies the ScheduleEventCallable
	 * concept, providing better compile-time error messages.
	 *
	 * @tparam Callback A type satisfying ScheduleEventCallable concept
	 * @param callback Any callable matching the schedule event signature
	 *
	 * @example
	 * scheduler.on_task_executed([](const schedule_entry& entry) {
	 *     std::cout << "Task executed: " << entry.name << std::endl;
	 * });
	 */
	template<ScheduleEventCallable Callback>
	void on_task_executed(Callback&& callback) {
		on_task_executed(schedule_callback(std::forward<Callback>(callback)));
	}

	/**
	 * @brief Set callback for task failure events
	 * @param callback Function called when a scheduled task fails to execute
	 */
	void on_task_failed(schedule_callback callback);

	/**
	 * @brief Set callback for task failure events using C++20 concept constraint
	 *
	 * This overload accepts any callable that satisfies the ScheduleEventCallable
	 * concept, providing better compile-time error messages.
	 *
	 * @tparam Callback A type satisfying ScheduleEventCallable concept
	 * @param callback Any callable matching the schedule event signature
	 *
	 * @example
	 * scheduler.on_task_failed([](const schedule_entry& entry) {
	 *     std::cerr << "Task failed: " << entry.name << std::endl;
	 * });
	 */
	template<ScheduleEventCallable Callback>
	void on_task_failed(Callback&& callback) {
		on_task_failed(schedule_callback(std::forward<Callback>(callback)));
	}

private:
	/**
	 * @brief Main scheduler loop running in background thread
	 */
	void scheduler_loop();

	/**
	 * @brief Calculate next run time for a schedule entry
	 * @param entry Schedule entry
	 * @return Next execution time point
	 */
	std::chrono::system_clock::time_point calculate_next_run(
		const schedule_entry& entry
	);

	/**
	 * @brief Execute a schedule entry
	 * @param entry Schedule entry to execute
	 */
	void execute_schedule(schedule_entry& entry);

	/**
	 * @brief Find the schedule with the earliest next run time
	 * @return Iterator to the next schedule to run, or end() if none
	 */
	std::unordered_map<std::string, schedule_entry>::iterator find_next_schedule();

	/**
	 * @brief Wake up the scheduler loop
	 */
	void wake_up();

	std::shared_ptr<task_client> client_;

	// Schedules storage
	mutable std::mutex mutex_;
	std::unordered_map<std::string, schedule_entry> schedules_;

	// Background thread management (using thread_system)
	std::atomic<bool> running_{false};
	std::atomic<bool> stop_requested_{false};
	std::unique_ptr<scheduler_worker> scheduler_worker_;
	std::condition_variable cv_;

	// Event callbacks
	schedule_callback on_executed_;
	schedule_callback on_failed_;
};

/**
 * @class scheduler_worker
 * @brief Worker thread that runs the scheduler loop using thread_system's thread_base
 *
 * This class inherits from kcenon::thread::thread_base to delegate thread
 * lifecycle management to thread_system. It monitors schedules and triggers
 * task execution when schedules are due.
 */
class scheduler_worker : public kcenon::thread::thread_base {
public:
	/**
	 * @brief Construct a scheduler worker
	 * @param scheduler Reference to the owning task_scheduler
	 */
	explicit scheduler_worker(task_scheduler& scheduler);

	~scheduler_worker() override = default;

	// Non-copyable, non-movable
	scheduler_worker(const scheduler_worker&) = delete;
	scheduler_worker& operator=(const scheduler_worker&) = delete;
	scheduler_worker(scheduler_worker&&) = delete;
	scheduler_worker& operator=(scheduler_worker&&) = delete;

protected:
	/**
	 * @brief Determines whether the worker should continue running
	 * @return true if scheduler is running and stop not requested
	 */
	[[nodiscard]] auto should_continue_work() const -> bool override;

	/**
	 * @brief Main work routine - checks and executes due schedules
	 * @return result_void indicating success or failure
	 */
	auto do_work() -> kcenon::thread::result_void override;

private:
	task_scheduler& scheduler_;
};

}  // namespace kcenon::messaging::task
