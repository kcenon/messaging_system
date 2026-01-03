// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task.cppm
 * @brief Distributed task queue partition for messaging_system module.
 *
 * This partition provides a distributed task queue system similar to
 * Celery (Python) or Sidekiq (Ruby). It includes:
 *
 * - task: Task definition with retry and scheduling support
 * - task_queue: Priority task queue
 * - worker_pool: Concurrent task execution
 * - scheduler: Cron-based task scheduling
 * - async_result: Async task result handling
 * - task_client: High-level task submission API
 *
 * @see kcenon.messaging for the primary module interface
 * @see kcenon.messaging:core for core messaging types
 */

module;

// =============================================================================
// Global Module Fragment - Standard Library Headers
// =============================================================================
#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// Third-party headers
#include <core/container.h>

export module kcenon.messaging:task;

export import kcenon.common;
export import kcenon.thread;
export import :core;

// =============================================================================
// Forward Declarations
// =============================================================================

export namespace kcenon::messaging::task {

class task;
class task_builder;
class task_queue;
class worker_pool;
class scheduler;
class task_monitor;
class task_client;

template<typename T>
class async_result;

class result_backend;
class memory_result_backend;

} // namespace kcenon::messaging::task

// =============================================================================
// Task State and Priority
// =============================================================================

export namespace kcenon::messaging::task {

// Import message_priority for convenience
using kcenon::messaging::message_priority;

/**
 * @enum task_state
 * @brief Represents the lifecycle state of a task
 */
enum class task_state {
    pending,    ///< Waiting to be queued
    queued,     ///< Added to queue
    running,    ///< Currently executing
    succeeded,  ///< Completed successfully
    failed,     ///< Execution failed
    retrying,   ///< Retrying after failure
    cancelled,  ///< Cancelled by user
    expired     ///< Expired before execution
};

/**
 * @brief Convert task_state to string representation
 */
constexpr std::string_view to_string(task_state state) noexcept {
    switch (state) {
        case task_state::pending: return "pending";
        case task_state::queued: return "queued";
        case task_state::running: return "running";
        case task_state::succeeded: return "succeeded";
        case task_state::failed: return "failed";
        case task_state::retrying: return "retrying";
        case task_state::cancelled: return "cancelled";
        case task_state::expired: return "expired";
        default: return "unknown";
    }
}

/**
 * @brief Parse string to task_state
 */
task_state task_state_from_string(std::string_view str) noexcept;

} // namespace kcenon::messaging::task

// =============================================================================
// Task Definition
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @struct task_options
 * @brief Options for task execution
 */
struct task_options {
    message_priority priority = message_priority::normal;
    std::optional<std::chrono::milliseconds> timeout;
    std::optional<std::chrono::milliseconds> ttl;
    uint32_t max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
    bool exponential_backoff = true;
    std::string queue_name = "default";
    std::unordered_map<std::string, std::string> headers;
};

/**
 * @struct task_result
 * @brief Result of task execution
 */
struct task_result {
    bool success = false;
    std::shared_ptr<container_module::value_container> data;
    std::string error_message;
    std::chrono::milliseconds execution_time{0};
    uint32_t retry_count = 0;
};

/**
 * @class task
 * @brief Task definition for distributed execution
 *
 * Represents a unit of work that can be queued, executed, and tracked.
 * Uses composition instead of inheritance from message.
 *
 * @code
 * auto task = task_builder()
 *     .name("process_order")
 *     .queue("orders")
 *     .priority(message_priority::high)
 *     .max_retries(5)
 *     .payload(order_data)
 *     .build()
 *     .value();
 *
 * auto result = task_client.submit(task);
 * @endcode
 */
class task {
    friend class task_builder;

public:
    task();
    explicit task(const std::string& name);

    // Task identification
    const std::string& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }

    // Queue and routing
    const std::string& queue_name() const noexcept { return options_.queue_name; }
    message_priority priority() const noexcept { return options_.priority; }

    // Options access
    const task_options& options() const noexcept { return options_; }
    task_options& options() noexcept { return options_; }

    // State management
    task_state state() const noexcept { return state_.load(); }
    void set_state(task_state state) noexcept { state_.store(state); }

    // Retry tracking
    uint32_t retry_count() const noexcept { return retry_count_.load(); }
    void increment_retry() noexcept { ++retry_count_; }
    bool can_retry() const noexcept;

    // Progress tracking
    float progress() const noexcept { return progress_.load(); }
    void set_progress(float progress) noexcept;

    // Payload access
    const container_module::value_container& payload() const;
    container_module::value_container& payload();

    // Timing
    std::chrono::system_clock::time_point created_at() const noexcept { return created_at_; }
    std::optional<std::chrono::system_clock::time_point> started_at() const;
    std::optional<std::chrono::system_clock::time_point> completed_at() const;

    // Expiration check
    bool is_expired() const noexcept;

    // Serialization
    kcenon::common::Result<std::vector<uint8_t>> serialize() const;
    static kcenon::common::Result<task> deserialize(const std::vector<uint8_t>& data);

private:
    std::string id_;
    std::string name_;
    task_options options_;
    std::shared_ptr<container_module::value_container> payload_;

    std::atomic<task_state> state_{task_state::pending};
    std::atomic<uint32_t> retry_count_{0};
    std::atomic<float> progress_{0.0f};

    std::chrono::system_clock::time_point created_at_;
    std::optional<std::chrono::system_clock::time_point> started_at_;
    std::optional<std::chrono::system_clock::time_point> completed_at_;
    mutable std::mutex timing_mutex_;
};

/**
 * @class task_builder
 * @brief Builder pattern for task construction
 */
class task_builder {
public:
    task_builder();

    task_builder& name(std::string name);
    task_builder& queue(std::string queue_name);
    task_builder& priority(message_priority priority);
    task_builder& timeout(std::chrono::milliseconds timeout);
    task_builder& ttl(std::chrono::milliseconds ttl);
    task_builder& max_retries(uint32_t retries);
    task_builder& retry_delay(std::chrono::milliseconds delay);
    task_builder& exponential_backoff(bool enabled);
    task_builder& header(std::string key, std::string value);
    task_builder& payload(std::shared_ptr<container_module::value_container> payload);

    kcenon::common::Result<task> build();

private:
    task task_;
};

} // namespace kcenon::messaging::task

// =============================================================================
// Task Queue
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @class task_queue
 * @brief Priority task queue for distributed execution
 *
 * Provides a thread-safe priority queue for tasks with:
 * - Priority ordering
 * - Delayed task support
 * - TTL enforcement
 * - Graceful shutdown
 */
class task_queue {
public:
    explicit task_queue(const std::string& name = "default", size_t max_size = 10000);
    ~task_queue();

    // Non-copyable
    task_queue(const task_queue&) = delete;
    task_queue& operator=(const task_queue&) = delete;

    /**
     * @brief Get queue name
     */
    const std::string& name() const noexcept { return name_; }

    /**
     * @brief Enqueue a task
     * @param t Task to enqueue
     * @return true if enqueued, false if queue is full
     */
    bool enqueue(task t);

    /**
     * @brief Enqueue a task with delay
     * @param t Task to enqueue
     * @param delay Delay before task becomes available
     * @return true if enqueued
     */
    bool enqueue_delayed(task t, std::chrono::milliseconds delay);

    /**
     * @brief Try to dequeue a task
     * @return Task if available, nullopt otherwise
     */
    std::optional<task> try_dequeue();

    /**
     * @brief Wait for and dequeue a task
     * @param timeout Maximum time to wait
     * @return Task if available within timeout, nullopt otherwise
     */
    std::optional<task> wait_dequeue(std::chrono::milliseconds timeout);

    /**
     * @brief Get current queue size
     */
    size_t size() const noexcept;

    /**
     * @brief Get number of delayed tasks
     */
    size_t delayed_count() const;

    /**
     * @brief Check if queue is empty
     */
    bool empty() const noexcept;

    /**
     * @brief Clear all tasks
     */
    void clear();

    /**
     * @brief Stop accepting new tasks and drain the queue
     */
    void shutdown();

    /**
     * @brief Check if shutdown was requested
     */
    bool is_shutdown() const noexcept { return shutdown_.load(); }

private:
    struct task_comparator {
        bool operator()(const task& a, const task& b) const;
    };

    struct delayed_task {
        std::chrono::steady_clock::time_point ready_at;
        task t;
    };

    std::string name_;
    std::priority_queue<task, std::vector<task>, task_comparator> queue_;
    std::vector<delayed_task> delayed_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    size_t max_size_;
    std::atomic<bool> shutdown_{false};
    std::thread delayed_worker_;
};

} // namespace kcenon::messaging::task

// =============================================================================
// Async Result
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @class async_result
 * @brief Async task result with polling and callback support
 *
 * Provides methods to wait for task completion and retrieve results.
 *
 * @tparam T Result type
 */
template<typename T>
class async_result {
public:
    explicit async_result(std::string task_id, std::shared_ptr<result_backend> backend);
    ~async_result() = default;

    // Non-copyable, movable
    async_result(const async_result&) = delete;
    async_result& operator=(const async_result&) = delete;
    async_result(async_result&&) noexcept = default;
    async_result& operator=(async_result&&) noexcept = default;

    /**
     * @brief Get the task ID
     */
    const std::string& task_id() const noexcept { return task_id_; }

    /**
     * @brief Check if the task is complete
     */
    bool is_ready() const;

    /**
     * @brief Get the current task state
     */
    task_state state() const;

    /**
     * @brief Wait for completion and get result
     * @param timeout Maximum time to wait
     * @return Result or error
     */
    kcenon::common::Result<T> get(
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

    /**
     * @brief Wait for completion without getting result
     * @param timeout Maximum time to wait
     * @return true if completed, false if timed out
     */
    bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) const;

    /**
     * @brief Register a callback for completion
     * @param callback Callback function
     */
    void on_complete(std::function<void(const kcenon::common::Result<T>&)> callback);

    /**
     * @brief Attempt to cancel the task
     * @return true if cancellation was requested
     */
    bool cancel();

    /**
     * @brief Revoke (forcefully cancel) the task
     * @return true if revoke was successful
     */
    bool revoke();

private:
    std::string task_id_;
    std::shared_ptr<result_backend> backend_;
    mutable std::optional<T> cached_result_;
    mutable std::mutex mutex_;
};

} // namespace kcenon::messaging::task

// =============================================================================
// Result Backend
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @class result_backend
 * @brief Interface for task result storage
 */
class result_backend {
public:
    virtual ~result_backend() = default;

    /**
     * @brief Store a task result
     * @param task_id Task identifier
     * @param result Result to store
     * @return true if stored successfully
     */
    virtual bool store(const std::string& task_id, const task_result& result) = 0;

    /**
     * @brief Get a task result
     * @param task_id Task identifier
     * @return Result if available
     */
    virtual std::optional<task_result> get(const std::string& task_id) const = 0;

    /**
     * @brief Get task state
     * @param task_id Task identifier
     * @return Task state if available
     */
    virtual std::optional<task_state> get_state(const std::string& task_id) const = 0;

    /**
     * @brief Update task state
     * @param task_id Task identifier
     * @param state New state
     * @return true if updated
     */
    virtual bool update_state(const std::string& task_id, task_state state) = 0;

    /**
     * @brief Delete a result
     * @param task_id Task identifier
     * @return true if deleted
     */
    virtual bool remove(const std::string& task_id) = 0;

    /**
     * @brief Check if a result exists
     * @param task_id Task identifier
     */
    virtual bool exists(const std::string& task_id) const = 0;

    /**
     * @brief Get all task IDs with a specific state
     * @param state State to filter by
     * @return List of task IDs
     */
    virtual std::vector<std::string> get_by_state(task_state state) const = 0;

    /**
     * @brief Cleanup expired results
     * @param max_age Maximum age of results to keep
     * @return Number of results removed
     */
    virtual size_t cleanup(std::chrono::seconds max_age) = 0;
};

/**
 * @class memory_result_backend
 * @brief In-memory result backend implementation
 */
class memory_result_backend : public result_backend {
public:
    memory_result_backend() = default;
    ~memory_result_backend() override = default;

    bool store(const std::string& task_id, const task_result& result) override;
    std::optional<task_result> get(const std::string& task_id) const override;
    std::optional<task_state> get_state(const std::string& task_id) const override;
    bool update_state(const std::string& task_id, task_state state) override;
    bool remove(const std::string& task_id) override;
    bool exists(const std::string& task_id) const override;
    std::vector<std::string> get_by_state(task_state state) const override;
    size_t cleanup(std::chrono::seconds max_age) override;

private:
    struct stored_result {
        task_result result;
        task_state state;
        std::chrono::steady_clock::time_point stored_at;
    };

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, stored_result> results_;
};

} // namespace kcenon::messaging::task

// =============================================================================
// Worker Pool
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @struct worker_options
 * @brief Options for worker pool configuration
 */
struct worker_options {
    size_t worker_count = std::thread::hardware_concurrency();
    size_t prefetch_count = 4;
    std::chrono::milliseconds poll_interval{100};
    bool auto_ack = true;
    std::function<void(const task&, const std::exception&)> error_handler;
};

/**
 * @class task_handler
 * @brief Interface for task handlers
 */
class task_handler {
public:
    virtual ~task_handler() = default;

    /**
     * @brief Handle a task
     * @param t Task to handle
     * @return Task result
     */
    virtual task_result handle(const task& t) = 0;

    /**
     * @brief Check if this handler can process the task
     * @param t Task to check
     * @return true if handler can process
     */
    virtual bool can_handle(const task& t) const = 0;
};

/**
 * @class worker_pool
 * @brief Concurrent task execution pool
 *
 * Manages a pool of workers that consume tasks from queues and execute them.
 */
class worker_pool {
public:
    explicit worker_pool(const worker_options& options = {});
    ~worker_pool();

    // Non-copyable
    worker_pool(const worker_pool&) = delete;
    worker_pool& operator=(const worker_pool&) = delete;

    /**
     * @brief Register a task queue
     * @param queue Queue to consume tasks from
     */
    void register_queue(std::shared_ptr<task_queue> queue);

    /**
     * @brief Unregister a task queue
     * @param queue_name Queue name
     * @return true if unregistered
     */
    bool unregister_queue(const std::string& queue_name);

    /**
     * @brief Register a task handler
     * @param name Handler name (matches task names)
     * @param handler Handler implementation
     */
    void register_handler(const std::string& name, std::shared_ptr<task_handler> handler);

    /**
     * @brief Register a function handler
     * @param name Handler name
     * @param fn Handler function
     */
    void register_handler(const std::string& name,
                          std::function<task_result(const task&)> fn);

    /**
     * @brief Start the worker pool
     */
    void start();

    /**
     * @brief Stop the worker pool
     * @param wait Wait for in-progress tasks to complete
     */
    void stop(bool wait = true);

    /**
     * @brief Check if pool is running
     */
    bool is_running() const noexcept { return running_.load(); }

    /**
     * @brief Get active worker count
     */
    size_t active_workers() const noexcept;

    /**
     * @brief Get processed task count
     */
    size_t processed_count() const noexcept { return processed_.load(); }

    /**
     * @brief Get failed task count
     */
    size_t failed_count() const noexcept { return failed_.load(); }

private:
    void worker_loop();
    task_result execute_task(const task& t);

    worker_options options_;
    std::vector<std::shared_ptr<task_queue>> queues_;
    std::unordered_map<std::string, std::shared_ptr<task_handler>> handlers_;
    std::unordered_map<std::string, std::function<task_result(const task&)>> fn_handlers_;
    std::vector<std::thread> workers_;
    mutable std::shared_mutex mutex_;
    std::atomic<bool> running_{false};
    std::atomic<size_t> active_count_{0};
    std::atomic<size_t> processed_{0};
    std::atomic<size_t> failed_{0};
};

} // namespace kcenon::messaging::task

// =============================================================================
// Scheduler
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @struct schedule_entry
 * @brief Scheduled task entry
 */
struct schedule_entry {
    std::string id;
    std::string name;
    std::string cron_expression;
    task_options options;
    std::shared_ptr<container_module::value_container> payload;
    bool enabled = true;
    std::optional<std::chrono::system_clock::time_point> last_run;
    std::optional<std::chrono::system_clock::time_point> next_run;
};

/**
 * @class cron_parser
 * @brief Cron expression parser
 */
class cron_parser {
public:
    /**
     * @brief Parse a cron expression
     * @param expression Cron expression (5 or 6 fields)
     * @return Result containing parser or error
     */
    static kcenon::common::Result<cron_parser> parse(const std::string& expression);

    /**
     * @brief Get next execution time
     * @param from Start time
     * @return Next execution time
     */
    std::chrono::system_clock::time_point next(
        std::chrono::system_clock::time_point from = std::chrono::system_clock::now()) const;

    /**
     * @brief Check if a time matches the schedule
     * @param time Time to check
     * @return true if matches
     */
    bool matches(std::chrono::system_clock::time_point time) const;

    /**
     * @brief Get the original expression
     */
    const std::string& expression() const noexcept { return expression_; }

private:
    explicit cron_parser(std::string expression);

    std::string expression_;
    std::vector<std::set<int>> fields_;  // minute, hour, day, month, weekday
};

/**
 * @class scheduler
 * @brief Cron-based task scheduler
 *
 * Schedules tasks based on cron expressions and submits them to queues.
 */
class scheduler {
public:
    explicit scheduler(std::shared_ptr<task_queue> default_queue = nullptr);
    ~scheduler();

    // Non-copyable
    scheduler(const scheduler&) = delete;
    scheduler& operator=(const scheduler&) = delete;

    /**
     * @brief Add a scheduled task
     * @param entry Schedule entry
     * @return Schedule ID
     */
    std::string add(schedule_entry entry);

    /**
     * @brief Add a task with cron expression
     * @param name Task name
     * @param cron Cron expression
     * @param options Task options
     * @param payload Task payload
     * @return Schedule ID
     */
    std::string add(const std::string& name,
                    const std::string& cron,
                    const task_options& options = {},
                    std::shared_ptr<container_module::value_container> payload = nullptr);

    /**
     * @brief Remove a scheduled task
     * @param schedule_id Schedule ID
     * @return true if removed
     */
    bool remove(const std::string& schedule_id);

    /**
     * @brief Enable/disable a scheduled task
     * @param schedule_id Schedule ID
     * @param enabled Enable state
     * @return true if updated
     */
    bool set_enabled(const std::string& schedule_id, bool enabled);

    /**
     * @brief Get schedule entry
     * @param schedule_id Schedule ID
     * @return Entry if found
     */
    std::optional<schedule_entry> get(const std::string& schedule_id) const;

    /**
     * @brief List all schedule entries
     */
    std::vector<schedule_entry> list() const;

    /**
     * @brief Start the scheduler
     */
    void start();

    /**
     * @brief Stop the scheduler
     */
    void stop();

    /**
     * @brief Check if scheduler is running
     */
    bool is_running() const noexcept { return running_.load(); }

    /**
     * @brief Set the default queue for scheduled tasks
     */
    void set_default_queue(std::shared_ptr<task_queue> queue);

private:
    void scheduler_loop();
    void check_and_submit();

    std::shared_ptr<task_queue> default_queue_;
    std::unordered_map<std::string, schedule_entry> entries_;
    std::unordered_map<std::string, cron_parser> parsers_;
    mutable std::shared_mutex mutex_;
    std::thread scheduler_thread_;
    std::condition_variable cv_;
    std::mutex cv_mutex_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> next_id_{0};
};

} // namespace kcenon::messaging::task

// =============================================================================
// Task Monitor
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @struct task_metrics
 * @brief Task execution metrics
 */
struct task_metrics {
    size_t total_submitted = 0;
    size_t total_completed = 0;
    size_t total_failed = 0;
    size_t total_retried = 0;
    size_t current_pending = 0;
    size_t current_running = 0;
    std::chrono::milliseconds avg_execution_time{0};
    std::chrono::milliseconds max_execution_time{0};
    std::chrono::system_clock::time_point last_updated;
};

/**
 * @class task_monitor
 * @brief Task execution monitoring
 */
class task_monitor {
public:
    task_monitor();
    ~task_monitor();

    /**
     * @brief Record task submission
     * @param task_id Task ID
     */
    void record_submit(const std::string& task_id);

    /**
     * @brief Record task start
     * @param task_id Task ID
     */
    void record_start(const std::string& task_id);

    /**
     * @brief Record task completion
     * @param task_id Task ID
     * @param execution_time Execution duration
     */
    void record_complete(const std::string& task_id, std::chrono::milliseconds execution_time);

    /**
     * @brief Record task failure
     * @param task_id Task ID
     * @param error Error message
     */
    void record_failure(const std::string& task_id, const std::string& error);

    /**
     * @brief Record task retry
     * @param task_id Task ID
     * @param attempt Retry attempt number
     */
    void record_retry(const std::string& task_id, uint32_t attempt);

    /**
     * @brief Get current metrics
     */
    task_metrics get_metrics() const;

    /**
     * @brief Get metrics for a specific queue
     * @param queue_name Queue name
     */
    task_metrics get_metrics(const std::string& queue_name) const;

    /**
     * @brief Reset all metrics
     */
    void reset();

private:
    task_metrics global_metrics_;
    std::unordered_map<std::string, task_metrics> queue_metrics_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> running_tasks_;
    mutable std::shared_mutex mutex_;
};

} // namespace kcenon::messaging::task

// =============================================================================
// Task Client
// =============================================================================

export namespace kcenon::messaging::task {

/**
 * @class task_client
 * @brief High-level API for task submission
 *
 * Provides a simple interface for submitting tasks and retrieving results.
 */
class task_client {
public:
    explicit task_client(std::shared_ptr<task_queue> default_queue = nullptr,
                         std::shared_ptr<result_backend> backend = nullptr);
    ~task_client();

    // Non-copyable
    task_client(const task_client&) = delete;
    task_client& operator=(const task_client&) = delete;

    /**
     * @brief Submit a task
     * @param t Task to submit
     * @return Async result for tracking
     */
    async_result<task_result> submit(task t);

    /**
     * @brief Submit a task to a specific queue
     * @param queue_name Queue name
     * @param t Task to submit
     * @return Async result
     */
    async_result<task_result> submit_to(const std::string& queue_name, task t);

    /**
     * @brief Submit a task with delay
     * @param t Task to submit
     * @param delay Delay before execution
     * @return Async result
     */
    async_result<task_result> submit_delayed(task t, std::chrono::milliseconds delay);

    /**
     * @brief Get result for a task
     * @param task_id Task ID
     * @return Async result
     */
    async_result<task_result> get_result(const std::string& task_id);

    /**
     * @brief Cancel a task
     * @param task_id Task ID
     * @return true if cancellation was requested
     */
    bool cancel(const std::string& task_id);

    /**
     * @brief Revoke (force cancel) a task
     * @param task_id Task ID
     * @return true if revoke was successful
     */
    bool revoke(const std::string& task_id);

    /**
     * @brief Register a queue
     * @param name Queue name
     * @param queue Queue instance
     */
    void register_queue(const std::string& name, std::shared_ptr<task_queue> queue);

    /**
     * @brief Get the result backend
     */
    std::shared_ptr<result_backend> backend() const { return backend_; }

private:
    std::shared_ptr<task_queue> default_queue_;
    std::shared_ptr<result_backend> backend_;
    std::unordered_map<std::string, std::shared_ptr<task_queue>> queues_;
    mutable std::shared_mutex mutex_;
};

} // namespace kcenon::messaging::task
