# Thread System: Patterns, Best Practices, and Troubleshooting Guide

This comprehensive guide covers patterns, best practices, antipatterns to avoid, and solutions to common concurrency issues when working with Thread System. Following these guidelines will help you write efficient, maintainable, and bug-free concurrent applications.

## Table of Contents

1. [Best Practices](#best-practices)
2. [Common Patterns](#common-patterns)
3. [Antipatterns to Avoid](#antipatterns-to-avoid)
4. [Troubleshooting Common Issues](#troubleshooting-common-issues)
5. [Advanced Concurrency Patterns](#advanced-concurrency-patterns)
6. [Debugging Concurrent Code](#debugging-concurrent-code)
7. [Performance Optimization](#performance-optimization)
8. [Integrating External Modules](#integrating-external-modules)

## Best Practices

### 1. Thread Base Usage

#### ✅ DO:
- Derive from `thread_base` when implementing custom worker threads
- Override `before_start()`, `do_work()`, and `after_stop()` to customize behavior
- Implement proper cleanup in your `after_stop()` method
- Use the `set_wake_interval()` method for periodic operations
- Check `should_continue_work()` regularly in long-running operations

#### ❌ DON'T:
- Call `start()` from within constructors
- Directly manipulate the underlying thread
- Implement tight loops without condition checks
- Ignore return values from `start()` and `stop()`
- Use thread-unsafe operations inside `do_work()` without proper synchronization

### 2. Thread Pool Usage

#### ✅ DO:
- Use `thread_pool` for CPU-bound tasks
- Create an appropriate number of worker threads (typically core count or core count + 1)
- Batch-submit jobs when possible using `enqueue_batch()`
- Use callback jobs for most operations
- Properly handle errors returned from job execution

#### ❌ DON'T:
- Create too many thread pools (one per application is often sufficient)
- Create an excessive number of worker threads (can lead to context switching overhead)
- Use thread pools for I/O-bound tasks without careful consideration
- Submit individual jobs in a tight loop (use batch submission instead)
- Block worker threads with long-running synchronous operations

### 3. Type Thread Pool Usage

#### ✅ DO:
- Use distinct type levels for different types of tasks
- Create dedicated workers for critical type levels
- Use lower type for background or maintenance tasks
- Consider custom type types for domain-specific scheduling
- Monitor queue sizes per type to ensure balanced execution

#### ❌ DON'T:
- Assign high type to all tasks (defeats the purpose)
- Create too many type levels (3-5 levels are typically sufficient)
- Ignore type inversion issues
- Use types inconsistently across the application
- Create type workers without assigning appropriate jobs

### 4. Error Handling

#### ✅ DO:
- Always check return values from Thread System functions
- Use the `result<T>` or `std::optional<std::string>` error patterns
- Implement custom error handlers when needed
- Provide meaningful error messages with context
- Handle errors appropriately at each layer

#### ❌ DON'T:
- Ignore error return values
- Let exceptions propagate from worker threads without handling
- Use generic error messages without context
- Assume operations will always succeed
- Mix different error handling patterns inconsistently

## Common Patterns

### 1. Worker Thread Pattern

```cpp
class MyWorker : public thread_module::thread_base {
protected:
    auto before_start() -> result_void override {
        // Initialize resources
        return {};
    }

    auto do_work() -> result_void override {
        // Perform work here
        return {};
    }

    auto should_continue_work() const -> bool override {
        // Logic to determine if more work is needed
        return !work_queue_.empty();
    }

    auto after_stop() -> result_void override {
        // Cleanup resources
        return {};
    }

private:
    // Worker-specific members
    std::queue<WorkItem> work_queue_;
};
```

### 2. Thread Pool Task Processing Pattern

```cpp
// Create a thread pool
auto [pool, error] = create_default(thread_counts_);
if (error.has_value()) {
    // Handle error
    return;
}

// Create a batch of jobs
std::vector<std::unique_ptr<thread_module::job>> jobs;
jobs.reserve(task_count);

for (auto i = 0; i < task_count; ++i) {
    jobs.push_back(std::make_unique<thread_module::callback_job>(
        [i]() -> std::optional<std::string> {
            // Process task
            return std::nullopt; // Success
        }
    ));
}

// Submit jobs as a batch for efficiency
error = pool->enqueue_batch(std::move(jobs));
if (error.has_value()) {
    // Handle error
    return;
}

// Start processing
error = pool->start();
if (error.has_value()) {
    // Handle error
    return;
}

// When done, stop the pool
pool->stop();
```

### 3. Type-Based Job Execution Pattern

```cpp
// Create a type thread pool with different type workers
auto [pool, error] = create_type_pool(
    high_type_workers_,
    normal_type_workers_,
    low_type_workers_
);

// Creating jobs with different types
std::vector<std::unique_ptr<typed_thread_pool_module::typed_job>> jobs;
jobs.reserve(job_count);

// High type critical tasks
jobs.push_back(std::make_unique<typed_thread_pool_module::callback_typed_job>(
    []() -> result_void {
        // Critical operation
        return {};
    },
    typed_thread_pool_module::job_types::High
));

// Normal type regular tasks
jobs.push_back(std::make_unique<typed_thread_pool_module::callback_typed_job>(
    []() -> result_void {
        // Regular operation
        return {};
    },
    typed_thread_pool_module::job_types::Normal
));

// Low type background tasks
jobs.push_back(std::make_unique<typed_thread_pool_module::callback_typed_job>(
    []() -> result_void {
        // Background operation
        return {};
    },
    typed_thread_pool_module::job_types::Low
));
```

### 4. Error Handler Pattern

```cpp
// Implement a custom error handler for thread operations
class ApplicationErrorHandler : public thread_module::error_handler {
public:
    void handle_error(const std::string& error_message) override {
        // Handle errors according to your application needs
        // Could write to a log file, send to monitoring system, etc.
        std::cerr << "Thread error: " << error_message << std::endl;
        
        // If using external logger module:
        // if (logger_) {
        //     logger_->write_error("Thread error: {}", error_message);
        // }
    }
    
private:
    // Optional: reference to external logger
    // std::shared_ptr<logger_interface> logger_;
};

// Use the error handler
auto error_handler = std::make_shared<ApplicationErrorHandler>();
thread_pool->set_error_handler(error_handler);
```

### 5. Producer-Consumer Pattern

```cpp
// Consumer thread worker
class ConsumerWorker : public thread_module::thread_base {
protected:
    auto do_work() -> result_void override {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        
        // Wait for data or stop signal
        data_condition_.wait(lock, [this]() {
            return !data_queue_.empty() || !should_continue_work();
        });
        
        // Check if should terminate
        if (!should_continue_work()) {
            return {};
        }
        
        // Process data
        auto data = std::move(data_queue_.front());
        data_queue_.pop();
        
        lock.unlock();
        process_data(data);
        
        return {};
    }
    
private:
    std::mutex queue_mutex_;
    std::condition_variable data_condition_;
    std::queue<Data> data_queue_;
    
    void process_data(const Data& data) {
        // Process the data
    }
};
```

### 6. Task Partitioning Pattern

```cpp
void process_large_dataset(const std::vector<Data>& dataset) {
    const size_t thread_count = std::thread::hardware_concurrency();
    const size_t chunk_size = (dataset.size() + thread_count - 1) / thread_count;
    
    // Create thread pool
    auto [pool, error] = create_default(thread_count);
    if (error.has_value()) {
        // Handle error appropriately - could use external logger if available
        std::cerr << "Failed to create thread pool: " << error.value() << std::endl;
        return;
    }
    
    // Submit chunks as separate jobs
    std::vector<std::unique_ptr<thread_module::job>> jobs;
    for (size_t i = 0; i < thread_count; ++i) {
        size_t start_idx = i * chunk_size;
        size_t end_idx = std::min(start_idx + chunk_size, dataset.size());
        
        jobs.push_back(std::make_unique<thread_module::callback_job>(
            [&dataset, start_idx, end_idx]() -> std::optional<std::string> {
                for (size_t j = start_idx; j < end_idx; ++j) {
                    process_item(dataset[j]);
                }
                return std::nullopt;
            }
        ));
    }
    
    pool->enqueue_batch(std::move(jobs));
    pool->start();
    pool->stop(); // Wait for all jobs to complete
}
```

## Antipatterns to Avoid

### 1. The Thread Explosion Antipattern

❌ **Problematic Approach**:
```cpp
// Creating a new thread for each small task
for (const auto& item : items) {
    auto thread = std::make_unique<thread_module::thread_base>();
    thread->start();
    // Process item in thread
}
```

✅ **Better Approach**:
```cpp
// Create a single thread pool
auto thread_pool = std::make_shared<thread_pool_module::thread_pool>();

// Submit all items as jobs
std::vector<std::unique_ptr<thread_module::job>> jobs;
for (const auto& item : items) {
    jobs.push_back(std::make_unique<thread_module::callback_job>(
        [item]() -> std::optional<std::string> {
            // Process item
            return std::nullopt;
        }
    ));
}

// Process all items with the thread pool
thread_pool->enqueue_batch(std::move(jobs));
thread_pool->start();
```

### 2. The Busy Waiting Antipattern

❌ **Problematic Approach**:
```cpp
auto do_work() -> result_void override {
    // Continuously check if work is available without yielding
    while (!work_available()) {
        // Tight loop consuming CPU
    }
    process_work();
    return {};
}
```

✅ **Better Approach**:
```cpp
auto do_work() -> result_void override {
    // Use condition variables and wake intervals
    if (!work_available()) {
        // Return and let the thread sleep until next wake interval
        return {};
    }
    process_work();
    return {};
}
```

### 3. The Type Abuse Antipattern

❌ **Problematic Approach**:
```cpp
// Marking all jobs as high type
for (auto i = 0; i < job_count; ++i) {
    jobs.push_back(std::make_unique<callback_typed_job>(
        [i]() -> result_void {
            // Regular task
            return {};
        },
        job_types::High // All jobs set to high type
    ));
}
```

✅ **Better Approach**:
```cpp
// Assign appropriate types based on task importance
for (auto i = 0; i < job_count; ++i) {
    // Determine appropriate type based on task characteristics
    auto type = determine_appropriate_type(i);
    
    jobs.push_back(std::make_unique<callback_typed_job>(
        [i]() -> result_void {
            // Regular task
            return {};
        },
        type
    ));
}
```

### 4. The Blocking Thread Pool Antipattern

❌ **Problematic Approach**:
```cpp
// Submitting I/O-bound or blocking operations to thread pool
pool->enqueue(std::make_unique<thread_module::callback_job>(
    []() -> std::optional<std::string> {
        // Perform long-running I/O operation that blocks
        std::this_thread::sleep_for(std::chrono::seconds(10));
        return std::nullopt;
    }
));
```

✅ **Better Approach**:
```cpp
// Use asynchronous I/O or dedicated threads for blocking operations
// For I/O-bound operations, consider async I/O or a dedicated thread pool
auto io_thread = std::make_unique<thread_module::thread_base>("io_thread");
io_thread->start();

// Use the main thread pool for CPU-bound work only
pool->enqueue(std::make_unique<thread_module::callback_job>(
    []() -> std::optional<std::string> {
        // CPU-bound computation
        return std::nullopt;
    }
));
```

### 5. The Performance Monitoring Antipattern

❌ **Problematic Approach**:
```cpp
for (int i = 0; i < 1000000; i++) {
    // Recording metrics for every single operation
    auto start = std::chrono::steady_clock::now();
    // Process item
    auto end = std::chrono::steady_clock::now();
    // Recording every single operation creates overhead
    record_metric("item_processed", end - start);
}
```

✅ **Better Approach**:
```cpp
// Record metrics at appropriate intervals or aggregated
std::atomic<size_t> processed_count{0};
auto batch_start = std::chrono::steady_clock::now();

for (int i = 0; i < 1000000; i++) {
    // Process item
    processed_count++;
    
    // Record metrics periodically
    if (i % 10000 == 0) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - batch_start;
        // Record aggregated metrics
        record_batch_metrics(10000, elapsed);
        batch_start = now;
    }
}
```

## Troubleshooting Common Issues

### 1. Race Conditions

#### Symptoms
- Inconsistent or unexpected results
- Program behavior varies between runs
- Results depend on timing or system load
- Intermittent crashes or data corruption

#### Solution Approaches
1. **Use mutex protection:**
   ```cpp
   std::mutex counter_mutex_;
   
   // In thread code
   std::lock_guard<std::mutex> lock(counter_mutex_);
   counter_++;
   ```

2. **Use atomic variables:**
   ```cpp
   std::atomic<int> counter_{0};
   
   // In thread code (no mutex needed)
   counter_++;
   ```

3. **Use job-based design:**
   ```cpp
   // Define a counter modification job
   auto increment_job = std::make_unique<thread_module::callback_job>(
       [this]() -> std::optional<std::string> {
           counter_++;
           return std::nullopt;
       }
   );
   
   // Submit to thread pool
   thread_pool->enqueue(std::move(increment_job));
   ```

### 2. Deadlocks

#### Symptoms
- Program freezes or hangs
- Multiple threads become unresponsive
- No CPU usage despite program appearing to run
- Deadlock detection tools report lock cycles

#### Solution Approaches
1. **Consistent lock ordering:**
   ```cpp
   // Always acquire locks in the same order
   std::lock_guard<std::mutex> lock1(mutex_a_); // Always first
   std::lock_guard<std::mutex> lock2(mutex_b_); // Always second
   ```

2. **Use std::lock for multiple locks:**
   ```cpp
   std::unique_lock<std::mutex> lock_a(mutex_a_, std::defer_lock);
   std::unique_lock<std::mutex> lock_b(mutex_b_, std::defer_lock);
   std::lock(lock_a, lock_b); // Atomic acquisition of both locks
   ```

3. **Avoid nested locks where possible:**
   ```cpp
   // Instead of nested locks, acquire all needed locks upfront
   {
       std::lock_guard<std::mutex> lock(mutex_);
       // Do all work requiring the lock here
   }
   // Then do work not requiring the lock
   ```

4. **Use lock timeouts to detect deadlocks:**
   ```cpp
   std::unique_lock<std::mutex> lock(mutex_, std::chrono::seconds(5));
   if (!lock) {
       // Handle potential deadlock - log error if logger is available
       handle_error("Potential deadlock detected - could not acquire lock within timeout");
       // Implement recovery strategy
   }
   ```

### 3. Type Inversion

#### Symptoms
- High-type tasks experience unexpected delays
- System responsiveness is inconsistent
- Critical jobs take longer than lower-type ones

#### Solution Approaches
1. **Minimize resource sharing across type boundaries:**
   ```cpp
   // Design your system to minimize cases where high and low type
   // threads need to share resources. Use separate resources when possible.
   auto high_type_resources = std::make_unique<ResourcePool>("high");
   auto low_type_resources = std::make_unique<ResourcePool>("low");
   ```

2. **Use type ceilings:**
   ```cpp
   // When a low-type thread acquires a critical resource,
   // temporarily boost its type
   auto original_type = get_current_type();
   set_current_type(high_type);
   
   // Critical section with shared resource
   {
       std::lock_guard<std::mutex> lock(shared_mutex_);
       // Work with shared resource
   }
   
   // Restore original type
   set_current_type(original_type);
   ```

### 4. Thread Starvation

#### Symptoms
- Certain tasks never complete or experience extreme delays
- Some threads never get CPU time
- System seems to focus on a subset of available work

#### Solution Approaches
1. **Dedicate workers to each type level:**
   ```cpp
   // Create workers specifically for low-type tasks
   auto low_worker = std::make_unique<typed_thread_worker>(
       std::vector<job_types>{job_types::Low},
       "low_type_worker"
   );
   
   type_pool->enqueue(std::move(low_worker));
   ```

2. **Implement aging for low-type tasks:**
   ```cpp
   class AgingJob : public typed_job {
   public:
       AgingJob(std::function<result_void()> func)
           : typed_job(job_types::Low), func_(func), 
             creation_time_(std::chrono::steady_clock::now()) {}
       
       auto get_type() const -> job_types override {
           auto age = std::chrono::steady_clock::now() - creation_time_;
           if (age > std::chrono::minutes(5)) {
               return job_types::High;
           } else if (age > std::chrono::minutes(1)) {
               return job_types::Normal;
           }
           return job_types::Low;
       }
       
   private:
       std::function<result_void()> func_;
       std::chrono::steady_clock::time_point creation_time_;
   };
   ```

### 5. False Sharing

#### Symptoms
- Unexpectedly poor performance in multi-threaded code
- Performance degrades as more cores are used
- CPU cache profiling shows high cache coherence traffic

#### Solution Approaches
1. **Pad data structures to avoid false sharing:**
   ```cpp
   struct alignas(64) PaddedCounter {
       std::atomic<int> value{0};
       char padding[64 - sizeof(std::atomic<int>)];
   };
   
   PaddedCounter counter1_;
   PaddedCounter counter2_;
   ```

2. **Use thread-local storage for counters:**
   ```cpp
   thread_local int local_counter_ = 0;
   
   // Each thread updates its own counter
   auto job = std::make_unique<thread_module::callback_job>(
       []() -> std::optional<std::string> {
           local_counter_++;
           return std::nullopt;
       }
   );
   ```

### 6. Memory Visibility Issues

#### Symptoms
- Threads don't "see" updates made by other threads
- Stale data is used in calculations
- Non-atomic operations on shared variables cause corruption

#### Solution Approaches
1. **Use atomic variables for flags:**
   ```cpp
   std::atomic<bool> done_{false};
   
   // Thread 1
   done_.store(true, std::memory_order_release);
   
   // Thread 2
   while (!done_.load(std::memory_order_acquire)) {
       std::this_thread::yield();
   }
   ```

2. **Use proper synchronization primitives:**
   ```cpp
   std::mutex mutex_;
   bool done_ = false;
   std::condition_variable cv_;
   
   // Thread 1
   {
       std::lock_guard<std::mutex> lock(mutex_);
       done_ = true;
   }
   cv_.notify_all();
   
   // Thread 2
   {
       std::unique_lock<std::mutex> lock(mutex_);
       cv_.wait(lock, [this]() { return done_; });
   }
   ```

## Advanced Concurrency Patterns

### 1. Event-Based Communication

```cpp
class EventSystem {
public:
    using EventHandler = std::function<void(const Event&)>;
    
    void subscribe(EventType type, EventHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_[type].push_back(handler);
    }
    
    void publish(const Event& event) {
        std::vector<EventHandler> handlers_to_call;
        
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handlers_.find(event.type);
            if (it != handlers_.end()) {
                handlers_to_call = it->second;
            }
        }
        
        // Submit each handler to the thread pool
        for (const auto& handler : handlers_to_call) {
            thread_pool_->enqueue(std::make_unique<thread_module::callback_job>(
                [handler, event]() -> std::optional<std::string> {
                    handler(event);
                    return std::nullopt;
                }
            ));
        }
    }
    
private:
    std::mutex mutex_;
    std::map<EventType, std::vector<EventHandler>> handlers_;
    std::shared_ptr<thread_pool_module::thread_pool> thread_pool_;
};
```

### 2. Work Stealing Pattern

```cpp
class WorkStealingPool {
public:
    WorkStealingPool(size_t worker_count) {
        // Create per-worker queues
        queues_.resize(worker_count);
        
        // Create workers
        for (size_t i = 0; i < worker_count; ++i) {
            workers_.push_back(std::make_unique<WorkStealingWorker>(
                i, queues_, *this
            ));
        }
    }
    
    auto enqueue(std::unique_ptr<thread_module::job> job, size_t preferred_worker) -> void {
        if (preferred_worker >= queues_.size()) {
            preferred_worker = 0;
        }
        
        queues_[preferred_worker].enqueue(std::move(job));
    }
    
private:
    std::vector<std::unique_ptr<WorkStealingWorker>> workers_;
    std::vector<JobQueue> queues_;
};

class WorkStealingWorker : public thread_module::thread_base {
protected:
    auto do_work() -> result_void override {
        // Try to get job from own queue
        auto job = queues_[worker_id_].dequeue();
        
        // If no job, try to steal from other queues
        if (!job) {
            for (size_t i = 0; i < queues_.size(); ++i) {
                if (i == worker_id_) continue;
                
                job = queues_[i].try_steal();
                if (job) break;
            }
        }
        
        // If we got a job, execute it
        if (job) {
            job->execute();
            return {};
        }
        
        // No job found, let the thread sleep
        return {};
    }
    
private:
    size_t worker_id_;
    std::vector<JobQueue>& queues_;
};
```

### 3. Read-Write Lock Pattern

```cpp
class ReadWriteLock {
public:
    void read_lock() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (write_count_ > 0 || write_waiting_ > 0) {
            read_cv_.wait(lock);
        }
        read_count_++;
    }
    
    void read_unlock() {
        std::unique_lock<std::mutex> lock(mutex_);
        read_count_--;
        if (read_count_ == 0 && write_waiting_ > 0) {
            write_cv_.notify_one();
        }
    }
    
    void write_lock() {
        std::unique_lock<std::mutex> lock(mutex_);
        write_waiting_++;
        while (read_count_ > 0 || write_count_ > 0) {
            write_cv_.wait(lock);
        }
        write_waiting_--;
        write_count_++;
    }
    
    void write_unlock() {
        std::unique_lock<std::mutex> lock(mutex_);
        write_count_--;
        if (write_waiting_ > 0) {
            write_cv_.notify_one();
        } else {
            read_cv_.notify_all();
        }
    }
    
private:
    std::mutex mutex_;
    std::condition_variable read_cv_;
    std::condition_variable write_cv_;
    int read_count_ = 0;
    int write_count_ = 0;
    int write_waiting_ = 0;
};
```

## Debugging Concurrent Code

### Using Diagnostics Effectively

1. **Track thread IDs for debugging:**
   ```cpp
   // Custom diagnostic helper
   class ThreadDiagnostics {
   public:
       static void record_event(const std::string& event) {
           auto thread_id = std::this_thread::get_id();
           auto timestamp = std::chrono::steady_clock::now();
           
           // Store or output diagnostic information
           std::ostringstream oss;
           oss << "[" << timestamp.time_since_epoch().count() 
               << "][Thread " << thread_id << "] " << event;
           
           // Output to console, file, or external logger if available
           std::cout << oss.str() << std::endl;
       }
   };
   ```

2. **Track state transitions:**
   ```cpp
   auto do_work() -> result_void override {
       ThreadDiagnostics::record_event("Worker state: entering critical section");
       {
           std::lock_guard<std::mutex> lock(mutex_);
           // Critical section
           ThreadDiagnostics::record_event(
               "Worker state: in critical section, count=" + std::to_string(count_)
           );
       }
       ThreadDiagnostics::record_event("Worker state: exited critical section");
       return {};
   }
   ```

3. **Use sequence numbers for ordering events:**
   ```cpp
   std::atomic<uint64_t> global_seq_{0};
   
   void record_sequenced_event(const std::string& event) {
       uint64_t seq = global_seq_++;
       std::ostringstream oss;
       oss << "[SEQ:" << seq << "] " << event;
       ThreadDiagnostics::record_event(oss.str());
   }
   ```

### Using Thread Sanitizers

1. **Enable ThreadSanitizer in your build:**
   ```bash
   # For GCC/Clang
   g++ -fsanitize=thread -g mycode.cpp
   
   # For MSVC
   # Use /fsanitize=address in recent versions
   ```

2. **Common issues detected by thread sanitizers:**
   - Data races
   - Deadlocks
   - Double-locking
   - Use-after-free in concurrent contexts

### Common Thread System Debugging Steps

1. **Verify thread pool startup:**
   ```cpp
   auto error = pool->start();
   if (error.has_value()) {
       // Handle startup error
       std::cerr << "Thread pool failed to start: " 
                 << error.value_or("unknown error") << std::endl;
       // Handle error
   } else {
       // Record successful startup
       std::cout << "Thread pool started successfully with " 
                 << pool->get_worker_count() << " workers" << std::endl;
   }
   ```

2. **Check job execution:**
   ```cpp
   // Add debugging to your job
   auto job = std::make_unique<thread_module::callback_job>(
       [](void) -> std::optional<std::string> {
           ThreadDiagnostics::record_event("Job started");
           
           // Your job logic here
           
           ThreadDiagnostics::record_event("Job completed");
           return std::nullopt;
       }
   );
   ```

## Performance Optimization

### Thread Pool Sizing Guidelines

1. **For CPU-bound tasks:**
   ```cpp
   // Use hardware concurrency as a baseline
   auto thread_count = std::thread::hardware_concurrency();
   ```

2. **For I/O-bound tasks:**
   ```cpp
   // Consider using more threads than cores
   auto thread_count = std::thread::hardware_concurrency() * 2;
   ```

3. **For mixed workloads:**
   ```cpp
   // Create separate pools for different workload types
   auto cpu_pool = create_default(std::thread::hardware_concurrency());
   auto io_pool = create_default(std::thread::hardware_concurrency() * 2);
   ```

### Batch Job Submission

Always prefer batch submission over individual job enqueueing:

```cpp
// ❌ Inefficient
for (const auto& task : tasks) {
    pool->enqueue(create_job(task));
}

// ✅ Efficient
std::vector<std::unique_ptr<thread_module::job>> jobs;
for (const auto& task : tasks) {
    jobs.push_back(create_job(task));
}
pool->enqueue_batch(std::move(jobs));
```

### Wake Interval Optimization

Configure wake intervals based on your workload:

```cpp
// For high-frequency tasks
worker.set_wake_interval(std::chrono::milliseconds(10));

// For periodic maintenance
worker.set_wake_interval(std::chrono::seconds(1));

// For rare events
worker.set_wake_interval(std::chrono::minutes(1));
```

## Integrating External Modules

### Logger Integration Pattern

Thread System is designed to work with external logging libraries. Here's how to integrate them:

```cpp
// Example: Integrating an external logger
class LoggerAdapter : public thread_module::error_handler {
public:
    LoggerAdapter(std::shared_ptr<external::Logger> logger) 
        : logger_(logger) {}
    
    void handle_error(const std::string& error_message) override {
        if (logger_) {
            logger_->error("Thread System: {}", error_message);
        }
    }
    
private:
    std::shared_ptr<external::Logger> logger_;
};

// Usage
auto external_logger = external::Logger::create("app.log");
auto logger_adapter = std::make_shared<LoggerAdapter>(external_logger);
thread_pool->set_error_handler(logger_adapter);
```

### Monitoring Integration Pattern

For performance monitoring and metrics collection:

```cpp
// Example: Integrating external monitoring
class MonitoringAdapter {
public:
    MonitoringAdapter(std::shared_ptr<external::MetricsCollector> collector)
        : collector_(collector) {}
    
    void record_job_execution(const std::string& job_type, 
                             std::chrono::nanoseconds duration) {
        if (collector_) {
            collector_->record_histogram("thread_system.job_duration", 
                                       duration.count(),
                                       {{"job_type", job_type}});
        }
    }
    
    void record_pool_size(size_t active_workers, size_t total_workers) {
        if (collector_) {
            collector_->record_gauge("thread_system.active_workers", 
                                   active_workers);
            collector_->record_gauge("thread_system.total_workers", 
                                   total_workers);
        }
    }
    
private:
    std::shared_ptr<external::MetricsCollector> collector_;
};
```

### Best Practices for Modular Architecture

1. **Use Dependency Injection**:
   ```cpp
   class Application {
   public:
       Application(std::shared_ptr<thread_module::error_handler> error_handler = nullptr)
           : error_handler_(error_handler) {
           // Create thread pool
           auto [pool, error] = create_default(4);
           if (!error.has_value() && error_handler_) {
               pool->set_error_handler(error_handler_);
           }
           thread_pool_ = std::move(pool);
       }
       
   private:
       std::shared_ptr<thread_module::error_handler> error_handler_;
       std::unique_ptr<thread_pool_module::thread_pool> thread_pool_;
   };
   ```

2. **Create Abstract Interfaces**:
   ```cpp
   // Define your own interfaces for optional dependencies
   class ILogger {
   public:
       virtual ~ILogger() = default;
       virtual void log(const std::string& message) = 0;
   };
   
   class IMonitor {
   public:
       virtual ~IMonitor() = default;
       virtual void record_metric(const std::string& name, double value) = 0;
   };
   ```

3. **Use Null Object Pattern for Optional Dependencies**:
   ```cpp
   class NullLogger : public ILogger {
   public:
       void log(const std::string&) override {
           // No-op implementation
       }
   };
   
   // Use null logger when no real logger is provided
   auto logger = external_logger ? external_logger : std::make_shared<NullLogger>();
   ```

## Conclusion

Following these patterns and avoiding the antipatterns will help you use Thread System effectively. The core principles to remember are:

1. **Use the right tool for the job**: Choose the appropriate component based on your requirements
2. **Design for concurrency**: Think about thread safety from the start
3. **Avoid overengineering**: Use the simplest concurrency pattern that meets your needs
4. **Monitor and measure**: Always validate the performance benefits of your threading design
5. **Handle errors**: Always check return values and handle errors properly
6. **Debug methodically**: Use diagnostics, debuggers, and thread sanitizers to identify issues
7. **Keep it modular**: Design your system to work with or without external logging/monitoring

By following these guidelines, you can create robust, efficient, and maintainable concurrent applications with Thread System.