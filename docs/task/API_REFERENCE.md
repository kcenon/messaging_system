# Task Module API Reference

Complete API documentation for the Task module.

## Table of Contents

- [task](#task)
- [task_config](#task_config)
- [task_builder](#task_builder)
- [task_handler_interface](#task_handler_interface)
- [task_context](#task_context)
- [task_queue](#task_queue)
- [worker_pool](#worker_pool)
- [task_client](#task_client)
- [async_result](#async_result)
- [result_backend_interface](#result_backend_interface)
- [memory_result_backend](#memory_result_backend)
- [task_scheduler](#task_scheduler)
- [cron_parser](#cron_parser)
- [task_monitor](#task_monitor)
- [task_system](#task_system)

---

## task

Represents a unit of work to be executed asynchronously.

**Header:** `<kcenon/messaging/task/task.h>`

### Enumerations

#### task_state

```cpp
enum class task_state {
    pending,    // Waiting to be queued
    queued,     // Added to queue
    running,    // Currently executing
    succeeded,  // Completed successfully
    failed,     // Execution failed
    retrying,   // Retrying after failure
    cancelled,  // Cancelled by user
    expired     // Expired before execution
};
```

### Constructors

```cpp
task();
task(const std::string& task_name);
task(const std::string& task_name, const container_module::value_container& payload);
task(const std::string& task_name, std::shared_ptr<container_module::value_container> payload);
```

### Identification Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `task_id()` | `const std::string&` | Get the unique task identifier |
| `task_name()` | `const std::string&` | Get the handler name for this task |

### State Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `state()` | `task_state` | Get current task state |
| `set_state(task_state)` | `void` | Set task state |
| `is_terminal_state()` | `bool` | Check if state is terminal (succeeded, failed, cancelled, expired) |

### Configuration Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `config()` | `const task_config&` | Get task configuration (const) |
| `config()` | `task_config&` | Get task configuration (mutable) |

### Execution Tracking

| Method | Return Type | Description |
|--------|-------------|-------------|
| `attempt_count()` | `size_t` | Get number of execution attempts |
| `increment_attempt()` | `void` | Increment attempt counter |
| `started_at()` | `const time_point&` | Get execution start time |
| `completed_at()` | `const time_point&` | Get execution completion time |

### Progress Tracking

| Method | Return Type | Description |
|--------|-------------|-------------|
| `progress()` | `double` | Get progress (0.0 to 1.0), thread-safe |
| `set_progress(double)` | `void` | Set progress value |
| `progress_message()` | `const std::string&` | Get progress message |
| `set_progress_message(const std::string&)` | `void` | Set progress message |

### Result/Error Handling

| Method | Return Type | Description |
|--------|-------------|-------------|
| `has_result()` | `bool` | Check if result is available |
| `result()` | `const value_container&` | Get task result |
| `set_result(shared_ptr<value_container>)` | `void` | Set task result |
| `has_error()` | `bool` | Check if error occurred |
| `error_message()` | `const std::string&` | Get error message |
| `error_traceback()` | `const std::string&` | Get error traceback |
| `set_error(message, traceback)` | `void` | Set error information |

### Retry Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_expired()` | `bool` | Check if task has expired |
| `should_retry()` | `bool` | Check if task should be retried |
| `get_next_retry_delay()` | `milliseconds` | Calculate next retry delay with backoff |

### Serialization

| Method | Return Type | Description |
|--------|-------------|-------------|
| `serialize()` | `Result<vector<uint8_t>>` | Serialize task to bytes |
| `deserialize(data)` | `Result<task>` | Static: Deserialize task from bytes |

---

## task_config

Configuration options for task execution.

**Header:** `<kcenon/messaging/task/task.h>`

### Fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `timeout` | `milliseconds` | 300000 (5 min) | Maximum execution time |
| `max_retries` | `size_t` | 3 | Maximum retry attempts |
| `retry_delay` | `milliseconds` | 1000 | Initial retry delay |
| `retry_backoff_multiplier` | `double` | 2.0 | Backoff multiplier |
| `priority` | `message_priority` | normal | Task priority level |
| `eta` | `optional<time_point>` | nullopt | Scheduled execution time |
| `expires` | `optional<milliseconds>` | nullopt | Expiration duration |
| `queue_name` | `std::string` | "default" | Target queue name |
| `tags` | `vector<string>` | {} | Task tags for filtering |

---

## task_builder

Fluent builder for constructing tasks.

**Header:** `<kcenon/messaging/task/task.h>`

### Constructor

```cpp
explicit task_builder(const std::string& task_name);
```

### Builder Methods

All methods return `task_builder&` for chaining.

| Method | Parameters | Description |
|--------|------------|-------------|
| `payload(const value_container&)` | Payload data | Set task payload |
| `payload(shared_ptr<value_container>)` | Payload pointer | Set task payload |
| `priority(message_priority)` | Priority level | Set priority |
| `timeout(milliseconds)` | Timeout duration | Set timeout |
| `retries(size_t)` | Max retries | Set max retry count |
| `retry_delay(milliseconds)` | Delay duration | Set initial retry delay |
| `backoff(double)` | Multiplier | Set backoff multiplier |
| `queue(const string&)` | Queue name | Set target queue |
| `eta(time_point)` | Scheduled time | Set scheduled execution time |
| `delay(milliseconds)` | Delay duration | Set execution delay from now |
| `expires_in(milliseconds)` | Duration | Set expiration |
| `tag(const string&)` | Tag name | Add a tag |
| `tags(const vector<string>&)` | Tag list | Add multiple tags |

### Build Method

```cpp
common::Result<task> build();
```

Returns `Result<task>` containing the constructed task or an error.

### Example

```cpp
auto result = task_builder("email.send")
    .payload(email_data)
    .priority(message_priority::high)
    .timeout(std::chrono::minutes(2))
    .retries(5)
    .queue("email-queue")
    .tag("notifications")
    .build();

if (result.is_ok()) {
    auto task = result.value();
}
```

---

## task_handler_interface

Abstract interface for task handlers.

**Header:** `<kcenon/messaging/task/task_handler.h>`

### Pure Virtual Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `name()` | `std::string` | Get handler name |
| `execute(task, context)` | `Result<value_container>` | Execute the task |

### Virtual Lifecycle Hooks

| Method | Parameters | Description |
|--------|------------|-------------|
| `on_retry(task, attempt)` | Task and attempt number | Called before retry |
| `on_failure(task, error)` | Task and error message | Called on failure |
| `on_success(task, result)` | Task and result | Called on success |

### Example

```cpp
class email_handler : public task_handler_interface {
public:
    std::string name() const override {
        return "email.send";
    }

    common::Result<value_container> execute(
        const task& t,
        task_context& ctx) override
    {
        auto to = t.payload().get_string("to").value();
        // Send email...
        value_container result;
        result.add("status", "sent");
        return common::ok(result);
    }

    void on_failure(const task& t, const std::string& error) override {
        // Log failure, send alert, etc.
    }
};
```

### Helper Functions

```cpp
// Create handler from lambda
template<typename Func>
std::shared_ptr<task_handler_interface> make_handler(
    const std::string& name,
    Func&& func);
```

---

## task_context

Execution context provided to handlers.

**Header:** `<kcenon/messaging/task/task_context.h>`

### Structures

#### progress_info

```cpp
struct progress_info {
    double progress;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};
```

#### task_log_entry

```cpp
struct task_log_entry {
    enum class level { info, warning, error };
    level log_level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};
```

### Progress Tracking

| Method | Return Type | Description |
|--------|-------------|-------------|
| `update_progress(double, string)` | `void` | Update progress (0.0-1.0) with message |
| `progress()` | `double` | Get current progress |
| `progress_history()` | `vector<progress_info>` | Get progress history |

### Checkpoint Management

| Method | Return Type | Description |
|--------|-------------|-------------|
| `save_checkpoint(value_container)` | `void` | Save checkpoint state |
| `load_checkpoint()` | `value_container` | Load saved checkpoint |
| `has_checkpoint()` | `bool` | Check if checkpoint exists |
| `clear_checkpoint()` | `void` | Clear checkpoint |

### Subtask Management

| Method | Return Type | Description |
|--------|-------------|-------------|
| `set_subtask_spawner(spawner)` | `void` | Set spawner function |
| `spawn_subtask(task)` | `Result<string>` | Spawn a subtask, returns task ID |
| `spawned_subtask_ids()` | `vector<string>` | Get IDs of spawned subtasks |

### Cancellation

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_cancelled()` | `bool` | Check if cancellation requested |
| `request_cancellation()` | `void` | Request cancellation |

### Logging

| Method | Parameters | Description |
|--------|------------|-------------|
| `log_info(message)` | Log message | Log info level message |
| `log_warning(message)` | Log message | Log warning level message |
| `log_error(message)` | Log message | Log error level message |
| `logs()` | - | Get all log entries |

---

## task_queue

Manages task queuing with multiple named queues.

**Header:** `<kcenon/messaging/task/task_queue.h>`

### Configuration

```cpp
struct task_queue_config {
    size_t max_size = 100000;
    bool enable_persistence = false;
    std::string persistence_path;
    bool enable_delayed_queue = true;
    std::chrono::milliseconds delayed_poll_interval{1000};
};
```

### Constructor

```cpp
explicit task_queue(const task_queue_config& config = {});
```

### Lifecycle

| Method | Return Type | Description |
|--------|-------------|-------------|
| `start()` | `VoidResult` | Start the queue |
| `stop()` | `void` | Stop the queue |
| `is_running()` | `bool` | Check if running |

### Enqueue Operations

| Method | Return Type | Description |
|--------|-------------|-------------|
| `enqueue(task)` | `Result<string>` | Add task, returns task ID |
| `enqueue_bulk(vector<task>)` | `Result<vector<string>>` | Add multiple tasks |

### Dequeue Operations

| Method | Return Type | Description |
|--------|-------------|-------------|
| `dequeue(queues, timeout)` | `Result<task>` | Wait for task from queues |
| `try_dequeue(queues)` | `Result<task>` | Non-blocking dequeue |

### Cancellation

| Method | Return Type | Description |
|--------|-------------|-------------|
| `cancel(task_id)` | `VoidResult` | Cancel by task ID |
| `cancel_by_tag(tag)` | `VoidResult` | Cancel by tag |

### Query Operations

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_task(task_id)` | `Result<task>` | Get task by ID |
| `queue_size(queue_name)` | `size_t` | Get queue size |
| `total_size()` | `size_t` | Get total tasks |
| `delayed_size()` | `size_t` | Get delayed task count |
| `list_queues()` | `vector<string>` | List queue names |
| `has_queue(name)` | `bool` | Check queue exists |

---

## worker_pool

Thread pool for executing tasks.

**Header:** `<kcenon/messaging/task/worker_pool.h>`

### Configuration

```cpp
struct worker_config {
    size_t concurrency = std::thread::hardware_concurrency();
    std::vector<std::string> queues = {"default"};
    std::chrono::milliseconds poll_interval{100};
    bool prefetch = true;
    size_t prefetch_count = 10;
};
```

### Statistics

```cpp
struct worker_statistics {
    size_t total_tasks_processed = 0;
    size_t total_tasks_succeeded = 0;
    size_t total_tasks_failed = 0;
    size_t total_tasks_retried = 0;
    size_t total_tasks_timed_out = 0;
    std::chrono::milliseconds total_execution_time{0};
    std::chrono::milliseconds avg_execution_time{0};
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point last_task_at;
};
```

### Handler Registration

| Method | Return Type | Description |
|--------|-------------|-------------|
| `register_handler(shared_ptr<handler>)` | `void` | Register class-based handler |
| `register_handler(name, func)` | `void` | Register lambda handler |
| `unregister_handler(name)` | `bool` | Unregister handler |
| `has_handler(name)` | `bool` | Check handler exists |
| `list_handlers()` | `vector<string>` | List handler names |

### Lifecycle

| Method | Return Type | Description |
|--------|-------------|-------------|
| `start()` | `VoidResult` | Start worker pool |
| `stop()` | `VoidResult` | Stop immediately |
| `shutdown_graceful(timeout)` | `VoidResult` | Graceful shutdown |

### Status

| Method | Return Type | Description |
|--------|-------------|-------------|
| `is_running()` | `bool` | Check if running |
| `active_workers()` | `size_t` | Count of busy workers |
| `idle_workers()` | `size_t` | Count of idle workers |
| `total_workers()` | `size_t` | Total worker count |

### Statistics

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_statistics()` | `worker_statistics` | Get execution statistics |
| `reset_statistics()` | `void` | Reset statistics |

---

## task_client

High-level API for task submission.

**Header:** `<kcenon/messaging/task/task_client.h>`

### Constructor

```cpp
task_client(
    std::shared_ptr<task_queue> queue,
    std::shared_ptr<result_backend_interface> backend);
```

### Immediate Execution

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send(task)` | `async_result` | Submit task |
| `send(name, payload)` | `async_result` | Submit with name and payload |

### Delayed Execution

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send_later(task, delay)` | `async_result` | Submit with delay |
| `send_at(task, time_point)` | `async_result` | Submit for specific time |

### Batch Operations

| Method | Return Type | Description |
|--------|-------------|-------------|
| `send_batch(vector<task>)` | `vector<async_result>` | Submit multiple tasks |

### Workflow Patterns

| Method | Return Type | Description |
|--------|-------------|-------------|
| `chain(vector<task>)` | `async_result` | Sequential execution |
| `chord(tasks, callback)` | `async_result` | Parallel with callback |

### Result/Cancellation

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_result(task_id)` | `async_result` | Get result handle |
| `cancel(task_id)` | `VoidResult` | Cancel task |
| `cancel_by_tag(tag)` | `VoidResult` | Cancel by tag |

### Status

| Method | Return Type | Description |
|--------|-------------|-------------|
| `pending_count(queue)` | `size_t` | Count pending tasks |
| `is_connected()` | `bool` | Check connection status |

---

## async_result

Handle for asynchronous task results.

**Header:** `<kcenon/messaging/task/async_result.h>`

### Status Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `task_id()` | `const string&` | Get task ID |
| `is_valid()` | `bool` | Check if handle is valid |
| `state()` | `task_state` | Get current state |
| `is_ready()` | `bool` | Check if completed |
| `is_successful()` | `bool` | Check if succeeded |
| `is_failed()` | `bool` | Check if failed |
| `is_cancelled()` | `bool` | Check if cancelled |

### Progress Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `progress()` | `double` | Get progress (0.0-1.0) |
| `progress_message()` | `string` | Get progress message |

### Blocking Result Retrieval

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get(timeout)` | `Result<value_container>` | Wait for result |
| `wait(timeout)` | `bool` | Wait for completion |

### Callback-Based

```cpp
void then(
    std::function<void(const value_container&)> on_success,
    std::function<void(const std::string&)> on_failure = nullptr);
```

### Task Control

| Method | Return Type | Description |
|--------|-------------|-------------|
| `revoke()` | `VoidResult` | Cancel the task |
| `children()` | `vector<async_result>` | Get child task handles |
| `add_child(task_id)` | `void` | Add child task |
| `error_message()` | `string` | Get error message |
| `error_traceback()` | `string` | Get error traceback |

---

## result_backend_interface

Abstract interface for result storage.

**Header:** `<kcenon/messaging/task/result_backend.h>`

### Structures

```cpp
struct progress_data {
    double progress{0.0};
    std::string message;
    std::chrono::system_clock::time_point updated_at;
};

struct error_data {
    std::string message;
    std::string traceback;
    std::chrono::system_clock::time_point occurred_at;
};
```

### Pure Virtual Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `store_state(task_id, state)` | `VoidResult` | Store task state |
| `store_result(task_id, result)` | `VoidResult` | Store result |
| `store_error(task_id, error, traceback)` | `VoidResult` | Store error |
| `store_progress(task_id, progress, message)` | `VoidResult` | Store progress |
| `get_state(task_id)` | `Result<task_state>` | Get state |
| `get_result(task_id)` | `Result<value_container>` | Get result |
| `get_progress(task_id)` | `Result<progress_data>` | Get progress |
| `get_error(task_id)` | `Result<error_data>` | Get error |
| `wait_for_result(task_id, timeout)` | `Result<value_container>` | Blocking wait |
| `cleanup_expired(max_age)` | `VoidResult` | Cleanup old entries |

### Optional Virtual Methods

| Method | Return Type | Default | Description |
|--------|-------------|---------|-------------|
| `exists(task_id)` | `bool` | false | Check if exists |
| `remove(task_id)` | `VoidResult` | no-op | Remove entry |
| `size()` | `size_t` | 0 | Get entry count |

---

## memory_result_backend

In-memory implementation of result_backend_interface.

**Header:** `<kcenon/messaging/task/memory_result_backend.h>`

### Constructor

```cpp
memory_result_backend();
```

### Additional Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `clear()` | `void` | Clear all stored data |

All methods from `result_backend_interface` are implemented.

---

## task_scheduler

Manages periodic and cron-based task scheduling.

**Header:** `<kcenon/messaging/task/scheduler.h>`

### Structures

```cpp
struct schedule_entry {
    std::string name;
    task task_template;
    std::variant<std::chrono::seconds, std::string> schedule;
    bool enabled = true;
    std::optional<time_point> last_run;
    std::optional<time_point> next_run;
    size_t run_count = 0;
    size_t failure_count = 0;

    bool is_cron() const;
    bool is_periodic() const;
    std::string cron_expression() const;
    std::chrono::seconds interval() const;
};
```

### Constructor

```cpp
explicit task_scheduler(task_client& client);
```

### Schedule Registration

| Method | Return Type | Description |
|--------|-------------|-------------|
| `add_periodic(name, task, interval)` | `VoidResult` | Add periodic schedule |
| `add_cron(name, task, expression)` | `VoidResult` | Add cron schedule |

### Schedule Management

| Method | Return Type | Description |
|--------|-------------|-------------|
| `remove(name)` | `VoidResult` | Remove schedule |
| `enable(name)` | `VoidResult` | Enable schedule |
| `disable(name)` | `VoidResult` | Disable schedule |
| `trigger_now(name)` | `VoidResult` | Trigger immediate execution |
| `update_interval(name, interval)` | `VoidResult` | Update periodic interval |
| `update_cron(name, expression)` | `VoidResult` | Update cron expression |

### Lifecycle

| Method | Return Type | Description |
|--------|-------------|-------------|
| `start()` | `VoidResult` | Start scheduler |
| `stop()` | `VoidResult` | Stop scheduler |
| `is_running()` | `bool` | Check if running |

### Query

| Method | Return Type | Description |
|--------|-------------|-------------|
| `list_schedules()` | `vector<schedule_entry>` | List all schedules |
| `get_schedule(name)` | `Result<schedule_entry>` | Get specific schedule |
| `schedule_count()` | `size_t` | Count schedules |
| `has_schedule(name)` | `bool` | Check schedule exists |

### Event Callbacks

```cpp
using schedule_callback = std::function<void(const schedule_entry&)>;

void on_task_executed(schedule_callback callback);
void on_task_failed(schedule_callback callback);
```

---

## cron_parser

Utility for parsing and evaluating cron expressions.

**Header:** `<kcenon/messaging/task/cron_parser.h>`

### Cron Expression Format

```
* * * * *
│ │ │ │ │
│ │ │ │ └─ Day of week (0-6, 0=Sunday)
│ │ │ └─── Month (1-12)
│ │ └───── Day of month (1-31)
│ └─────── Hour (0-23)
└───────── Minute (0-59)
```

### Supported Syntax

- `*` - All values
- `5` - Specific value
- `*/15` - Every N units
- `1-5` - Range
- `1,3,5` - List

### Structure

```cpp
struct cron_expression {
    std::set<int> minutes;   // 0-59
    std::set<int> hours;     // 0-23
    std::set<int> days;      // 1-31
    std::set<int> months;    // 1-12
    std::set<int> weekdays;  // 0-6
};
```

### Static Methods

| Method | Return Type | Description |
|--------|-------------|-------------|
| `parse(expr)` | `Result<cron_expression>` | Parse cron string |
| `next_run_time(expr, from)` | `Result<time_point>` | Calculate next run |
| `is_valid(expr)` | `bool` | Validate expression |
| `to_string(expr)` | `string` | Convert to string |

### Examples

```cpp
// Every hour at minute 0
auto expr = cron_parser::parse("0 * * * *").value();

// Daily at 3:00 AM
auto expr = cron_parser::parse("0 3 * * *").value();

// Every 15 minutes
auto expr = cron_parser::parse("*/15 * * * *").value();

// Weekdays at 9 AM
auto expr = cron_parser::parse("0 9 * * 1-5").value();
```

---

## task_monitor

System monitoring and event subscriptions.

**Header:** `<kcenon/messaging/task/monitor.h>`

### Structures

```cpp
struct queue_stats {
    std::string name;
    size_t pending_count = 0;
    size_t running_count = 0;
    size_t delayed_count = 0;
};

struct worker_info {
    std::string worker_id;
    std::vector<std::string> queues;
    size_t active_tasks = 0;
    std::chrono::system_clock::time_point last_heartbeat;
    bool is_healthy = true;
};
```

### Constructor

```cpp
task_monitor(
    std::shared_ptr<task_queue> queue,
    worker_pool* workers);
```

### Queue Statistics

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_queue_stats()` | `vector<queue_stats>` | All queue stats |
| `get_queue_stats(name)` | `Result<queue_stats>` | Specific queue stats |

### Worker Status

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_workers()` | `vector<worker_info>` | All worker info |
| `get_worker_statistics()` | `optional<worker_statistics>` | Worker pool stats |

### Task Queries

| Method | Return Type | Description |
|--------|-------------|-------------|
| `list_active_tasks()` | `vector<task>` | Running tasks |
| `list_pending_tasks(queue)` | `vector<task>` | Pending tasks |
| `list_failed_tasks(limit)` | `vector<task>` | Failed tasks |

### Task Management

| Method | Return Type | Description |
|--------|-------------|-------------|
| `cancel_task(task_id)` | `VoidResult` | Cancel task |
| `retry_task(task_id)` | `VoidResult` | Retry failed task |
| `purge_queue(name)` | `VoidResult` | Clear queue |

### Event Subscriptions

```cpp
using task_started_handler = std::function<void(const task&)>;
using task_completed_handler = std::function<void(const task&, bool success)>;
using task_failed_handler = std::function<void(const task&, const std::string& error)>;
using worker_offline_handler = std::function<void(const std::string& worker_id)>;

void on_task_started(task_started_handler handler);
void on_task_completed(task_completed_handler handler);
void on_task_failed(task_failed_handler handler);
void on_worker_offline(worker_offline_handler handler);
```

---

## task_system

Unified facade integrating all task components.

**Header:** `<kcenon/messaging/task/task_system.h>`

### Configuration

```cpp
struct task_system_config {
    task_queue_config queue;
    worker_config worker;
    bool enable_scheduler = true;
    bool enable_monitoring = true;
    std::string result_backend_type = "memory";
};
```

### Constructor

```cpp
explicit task_system(const task_system_config& config = {});
```

### Lifecycle

| Method | Return Type | Description |
|--------|-------------|-------------|
| `start()` | `VoidResult` | Start all components |
| `stop()` | `VoidResult` | Stop all components |
| `shutdown_graceful(timeout)` | `VoidResult` | Graceful shutdown |
| `is_running()` | `bool` | Check if running |

### Component Access

| Method | Return Type | Description |
|--------|-------------|-------------|
| `client()` | `task_client&` | Get task client |
| `workers()` | `worker_pool&` | Get worker pool |
| `scheduler()` | `task_scheduler*` | Get scheduler (nullable) |
| `monitor()` | `task_monitor*` | Get monitor (nullable) |
| `queue()` | `shared_ptr<task_queue>` | Get queue |
| `results()` | `shared_ptr<result_backend>` | Get result backend |

### Handler Registration (Convenience)

| Method | Return Type | Description |
|--------|-------------|-------------|
| `register_handler(shared_ptr<handler>)` | `void` | Register handler |
| `register_handler(name, func)` | `void` | Register lambda |
| `unregister_handler(name)` | `bool` | Unregister handler |

### Task Submission (Convenience)

| Method | Return Type | Description |
|--------|-------------|-------------|
| `submit(name, payload)` | `async_result` | Submit task |
| `submit(task)` | `async_result` | Submit task |
| `submit_later(task, delay)` | `async_result` | Submit delayed |
| `submit_batch(tasks)` | `vector<async_result>` | Submit batch |

### Scheduling (Convenience)

| Method | Return Type | Description |
|--------|-------------|-------------|
| `schedule_periodic(name, task, interval)` | `VoidResult` | Add periodic |
| `schedule_cron(name, task, expression)` | `VoidResult` | Add cron |

### Statistics

| Method | Return Type | Description |
|--------|-------------|-------------|
| `get_statistics()` | `worker_statistics` | Get worker stats |
| `pending_count(queue)` | `size_t` | Pending task count |
| `active_workers()` | `size_t` | Active worker count |
| `total_workers()` | `size_t` | Total worker count |

---

## Related Documentation

- [Quick Start Guide](QUICK_START.md)
- [Architecture Guide](ARCHITECTURE.md)
- [Patterns Guide](PATTERNS.md)
- [Configuration Guide](CONFIGURATION.md)
- [Troubleshooting](TROUBLESHOOTING.md)
