# Migration Guide: Transitioning to Thread System

This guide provides step-by-step instructions for transitioning existing multithreaded code to use the Thread System library. Whether you're migrating from raw threads, a different threading library, or just improving your existing concurrent code, this document will help you make the transition smoothly.

## Why Migrate to Thread System?

Before diving into the migration process, let's review the key benefits of using Thread System:

1. **Structured concurrency**: Clear thread lifecycle management and job-based design
2. **Error handling**: Consistent error reporting using `result_void` and optional return types
3. **Improved thread safety**: Thread-safe patterns and components
4. **Type scheduling**: Built-in support for type-based execution
5. **Logging integration**: Thread-safe, asynchronous logging system
6. **Modern C++**: Leverages C++20 features when available
7. **Cross-platform**: Works consistently across Windows, Linux, and macOS

## Migration Process Overview

The migration process typically follows these steps:

1. **Analysis**: Identify threading patterns in your existing code
2. **Component selection**: Choose appropriate Thread System components
3. **Basic refactoring**: Replace direct thread usage with Thread System
4. **Advanced refactoring**: Leverage Thread System features
5. **Testing and validation**: Verify correctness and performance

Let's explore each step in detail with examples.

## Step 1: Analyze Existing Code

Before migrating, identify your current threading patterns:

### Common Threading Patterns

#### Raw Thread Usage

```cpp
// Typical std::thread usage
std::thread worker_thread([data]() {
    // Worker function
    process_data(data);
});

// Join later
worker_thread.join();
```

#### Thread Pool Pattern

```cpp
// Simplified thread pool example
class SimpleThreadPool {
public:
    SimpleThreadPool(size_t thread_count) {
        for (size_t i = 0; i < thread_count; ++i) {
            threads_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this]() { 
                            return stop_ || !tasks_.empty(); 
                        });
                        
                        if (stop_ && tasks_.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        condition_.notify_one();
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        
        condition_.notify_all();
        
        for (std::thread& thread : threads_) {
            thread.join();
        }
    }
    
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_ = false;
};
```

#### Asynchronous Processing

```cpp
// Using std::async
auto future = std::async(std::launch::async, []() {
    // Perform operation
    return result;
});

// Get result later
auto result = future.get();
```

#### Producer-Consumer Pattern

```cpp
// Shared queue
template <typename T>
class ThreadSafeQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        condition_.notify_one();
    }
    
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }
    
    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]() { return !queue_.empty(); });
        value = std::move(queue_.front());
        queue_.pop();
    }
    
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable condition_;
};

// Usage
ThreadSafeQueue<Item> queue;

// Producer thread
std::thread producer([&queue]() {
    while (true) {
        Item item = produce_item();
        queue.push(std::move(item));
    }
});

// Consumer thread
std::thread consumer([&queue]() {
    while (true) {
        Item item;
        queue.wait_and_pop(item);
        consume_item(item);
    }
});
```

## Step 2: Select Thread System Components

Based on your existing patterns, select the appropriate Thread System components:

| Existing Pattern | Thread System Component | 
|------------------|--------------------------|
| Raw threads | `thread_base` |
| Thread pools | `thread_pool` or `typed_thread_pool` |
| Async processing | `callback_job` with thread pool |
| Producer-consumer | Job queues with thread pool |
| Worker threads | Custom `thread_base` subclass |
| Thread-safe logging | `log_module` |

## Step 3: Basic Refactoring

Let's refactor each common pattern to use Thread System:

### Raw Thread to thread_base

```cpp
// Before: Raw thread
std::thread worker_thread([data]() {
    // Worker function
    process_data(data);
});

// After: thread_base
class DataProcessor : public thread_module::thread_base {
public:
    DataProcessor(const Data& data) : data_(data) {}
    
protected:
    auto do_work() -> result_void override {
        try {
            process_data(data_);
            return {}; // Success
        } catch (const std::exception& e) {
            return make_error(e.what()); // Error
        }
    }
    
private:
    Data data_;
};

// Usage
auto processor = std::make_unique<DataProcessor>(data);
auto result = processor->start();
if (result.has_value()) {
    // Handle start error
}

// Later
processor->stop();
```

### Thread Pool to thread_pool

```cpp
// Before: Custom thread pool
SimpleThreadPool pool(std::thread::hardware_concurrency());
pool.enqueue([]() {
    process_data();
});

// After: Thread System thread_pool
auto [pool, error] = create_default(std::thread::hardware_concurrency());
if (error.has_value()) {
    // Handle error
    return;
}

auto job = std::make_unique<thread_module::callback_job>(
    []() -> std::optional<std::string> {
        try {
            process_data();
            return std::nullopt; // Success
        } catch (const std::exception& e) {
            return e.what(); // Error
        }
    }
);

error = pool->enqueue(std::move(job));
if (error.has_value()) {
    // Handle enqueue error
    return;
}

error = pool->start();
if (error.has_value()) {
    // Handle start error
    return;
}

// Later
pool->stop();
```

### Async Processing to Thread System

```cpp
// Before: std::async
auto future = std::async(std::launch::async, []() {
    return process_data();
});

// Later
auto result = future.get();

// After: Thread System with promise/future pattern
std::promise<Result> promise;
std::future<Result> future = promise.get_future();

auto job = std::make_unique<thread_module::callback_job>(
    [&promise]() -> std::optional<std::string> {
        try {
            Result result = process_data();
            promise.set_value(result);
            return std::nullopt; // Success
        } catch (const std::exception& e) {
            promise.set_exception(std::current_exception());
            return e.what(); // Error
        }
    }
);

thread_pool->enqueue(std::move(job));
thread_pool->start();

// Later
auto result = future.get(); // Will throw if exception was set
```

### Producer-Consumer to Thread System

```cpp
// Before: Custom producer-consumer with thread-safe queue
ThreadSafeQueue<Item> queue;
// Producer and consumer threads...

// After: Thread System job-based approach
class Producer : public thread_module::thread_base {
public:
    Producer(std::shared_ptr<thread_pool_module::thread_pool> consumer_pool)
        : consumer_pool_(consumer_pool) {}
    
protected:
    auto do_work() -> result_void override {
        Item item = produce_item();
        
        auto job = std::make_unique<thread_module::callback_job>(
            [item]() -> std::optional<std::string> {
                // Consumer logic
                consume_item(item);
                return std::nullopt;
            }
        );
        
        auto error = consumer_pool_->enqueue(std::move(job));
        if (error.has_value()) {
            return make_error(error.value());
        }
        
        return {};
    }
    
    auto should_continue_work() const -> bool override {
        return should_continue_producing_;
    }
    
private:
    std::shared_ptr<thread_pool_module::thread_pool> consumer_pool_;
    bool should_continue_producing_ = true;
    
    Item produce_item() {
        // Production logic
        return Item{};
    }
};

// Usage
auto consumer_pool = create_default(4).first;
consumer_pool->start();

auto producer = std::make_unique<Producer>(consumer_pool);
producer->start();

// Later
producer->stop();
consumer_pool->stop();
```

## Step 4: Advanced Refactoring

After the basic migration, enhance your code with Thread System's advanced features:

### Adding Type Support

```cpp
// Before: Regular thread pool jobs
pool->enqueue(std::make_unique<thread_module::callback_job>(
    []() -> std::optional<std::string> {
        process_data();
        return std::nullopt;
    }
));

// After: Type-based jobs
auto [type_pool, error] = create_type_pool(2, 2, 2); // High, normal, low

// Critical job
auto critical_job = std::make_unique<typed_thread_pool_module::callback_typed_job>(
    []() -> result_void {
        process_critical_data();
        return {};
    },
    typed_thread_pool_module::job_types::High
);

// Background job
auto background_job = std::make_unique<typed_thread_pool_module::callback_typed_job>(
    []() -> result_void {
        process_background_data();
        return {};
    },
    typed_thread_pool_module::job_types::Low
);

type_pool->enqueue(std::move(critical_job));
type_pool->enqueue(std::move(background_job));
type_pool->start();
```

### Integrating Logging

```cpp
// Before: Custom logging
void log_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    std::cout << get_timestamp() << ": " << message << std::endl;
}

// After: Thread System logging
auto initialize_logger() -> std::optional<std::string> {
    log_module::set_title("my_application");
    log_module::console_target(log_module::log_types::All);
    log_module::file_target(log_module::log_types::Warning | 
                           log_module::log_types::Error);
    
    return log_module::start();
}

// Initialize logger
auto error = initialize_logger();
if (error.has_value()) {
    std::cerr << "Logger initialization failed: " << error.value() << std::endl;
    return;
}

// Use logger in threads
auto job = std::make_unique<thread_module::callback_job>(
    []() -> std::optional<std::string> {
        log_module::write_information("Starting job processing");
        
        try {
            process_data();
            log_module::write_information("Job completed successfully");
        } catch (const std::exception& e) {
            log_module::write_error("Job failed: {}", e.what());
            return e.what();
        }
        
        return std::nullopt;
    }
);
```

### Implementing Periodic Tasks

```cpp
// Before: Custom timer thread
std::atomic<bool> running{true};
std::thread timer_thread([&running]() {
    while (running) {
        perform_periodic_task();
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
});

// Later
running = false;
timer_thread.join();

// After: Thread System with wake interval
class PeriodicTask : public thread_module::thread_base {
public:
    PeriodicTask() {
        // Set 60-second wake interval
        set_wake_interval(std::chrono::seconds(60));
    }
    
protected:
    auto do_work() -> result_void override {
        try {
            perform_periodic_task();
            return {};
        } catch (const std::exception& e) {
            return make_error(e.what());
        }
    }
};

// Usage
auto periodic_task = std::make_unique<PeriodicTask>();
periodic_task->start();

// Later
periodic_task->stop();
```

### Error Handling Improvements

```cpp
// Before: Exception-based error handling
try {
    pool.enqueue([]() {
        try {
            process_data();
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    });
} catch (const std::exception& e) {
    std::cerr << "Failed to enqueue job: " << e.what() << std::endl;
}

// After: Result-based error handling
auto job = std::make_unique<thread_module::callback_job>(
    []() -> std::optional<std::string> {
        // Try to process data
        auto result = process_data();
        if (result.has_error()) {
            log_module::write_error("Processing error: {}", result.get_error().message());
            return result.get_error().message();
        }
        return std::nullopt;
    }
);

auto error = pool->enqueue(std::move(job));
if (error.has_value()) {
    log_module::write_error("Failed to enqueue job: {}", error.value());
    // Handle error
}
```

## Step 5: Testing and Validation

After migrating to Thread System, thoroughly test your code:

### Unit Tests for Thread System Components

```cpp
// Test thread_base functionality
TEST(ThreadBaseTest, StartStopCycle) {
    // Create a custom thread_base
    class TestThread : public thread_module::thread_base {
    public:
        std::atomic<int> counter{0};
        
    protected:
        auto do_work() -> result_void override {
            counter++;
            return {};
        }
        
        auto should_continue_work() const -> bool override {
            return counter < 10;
        }
    };
    
    auto test_thread = std::make_unique<TestThread>();
    
    // Start the thread
    auto result = test_thread->start();
    ASSERT_FALSE(result.has_value()) << "Thread failed to start: " 
                                    << result.value_or("unknown error");
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Stop the thread
    result = test_thread->stop();
    ASSERT_FALSE(result.has_value()) << "Thread failed to stop: " 
                                    << result.value_or("unknown error");
    
    // Verify counter value
    EXPECT_EQ(test_thread->counter, 10) << "Thread did not execute expected number of iterations";
}

// Test thread_pool functionality
TEST(ThreadPoolTest, JobExecution) {
    // Create thread pool
    auto [pool, error] = create_default(4);
    ASSERT_FALSE(error.has_value()) << "Failed to create thread pool: " 
                                   << error.value_or("unknown error");
    
    // Create atomic counter for job completion tracking
    std::atomic<int> completed_jobs{0};
    
    // Prepare jobs
    const int job_count = 100;
    std::vector<std::unique_ptr<thread_module::job>> jobs;
    jobs.reserve(job_count);
    
    for (int i = 0; i < job_count; ++i) {
        jobs.push_back(std::make_unique<thread_module::callback_job>(
            [&completed_jobs]() -> std::optional<std::string> {
                completed_jobs++;
                return std::nullopt;
            }
        ));
    }
    
    // Enqueue jobs
    error = pool->enqueue_batch(std::move(jobs));
    ASSERT_FALSE(error.has_value()) << "Failed to enqueue jobs: " 
                                   << error.value_or("unknown error");
    
    // Start pool
    error = pool->start();
    ASSERT_FALSE(error.has_value()) << "Failed to start thread pool: " 
                                   << error.value_or("unknown error");
    
    // Wait for completion (with timeout)
    const auto timeout = std::chrono::seconds(5);
    const auto start_time = std::chrono::steady_clock::now();
    
    while (completed_jobs < job_count) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        if (std::chrono::steady_clock::now() - start_time > timeout) {
            pool->stop();
            FAIL() << "Timeout waiting for jobs to complete. Completed: " 
                  << completed_jobs << " of " << job_count;
            break;
        }
    }
    
    // Stop pool
    pool->stop();
    
    // Verify all jobs completed
    EXPECT_EQ(completed_jobs, job_count) << "Not all jobs were executed";
}
```

### Performance Validation

```cpp
// Performance comparison
void performance_comparison() {
    const int iterations = 10;
    const int job_count = 100000;
    
    // Measure original implementation
    double original_avg_time = 0.0;
    for (int i = 0; i < iterations; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        run_original_implementation(job_count);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
            
        original_avg_time += duration;
    }
    original_avg_time /= iterations;
    
    // Measure Thread System implementation
    double thread_system_avg_time = 0.0;
    for (int i = 0; i < iterations; ++i) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        run_thread_system_implementation(job_count);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count();
            
        thread_system_avg_time += duration;
    }
    thread_system_avg_time /= iterations;
    
    // Compare results
    std::cout << "Original implementation: " << original_avg_time << "ms\n";
    std::cout << "Thread System implementation: " << thread_system_avg_time << "ms\n";
    std::cout << "Performance change: " 
              << ((thread_system_avg_time / original_avg_time) - 1.0) * 100 << "%\n";
}
```

## Common Migration Challenges and Solutions

### Challenge 1: Adapting to Result-based Error Handling

**Problem**: Thread System uses `std::optional<std::string>` or `result_void` for error reporting instead of exceptions.

**Solution**:
```cpp
// Create a wrapper function to convert exception-based code
auto run_with_exception_handling(std::function<void()> func) -> std::optional<std::string> {
    try {
        func();
        return std::nullopt; // Success
    } catch (const std::exception& e) {
        return e.what(); // Return error message
    } catch (...) {
        return "Unknown error occurred";
    }
}

// Usage
auto job = std::make_unique<thread_module::callback_job>(
    [legacy_func]() -> std::optional<std::string> {
        return run_with_exception_handling(legacy_func);
    }
);
```

### Challenge 2: Managing Thread Lifecycle

**Problem**: Thread System has a different lifecycle model (start/stop) compared to raw threads.

**Solution**:
```cpp
// Create a manager class for Thread System components
class ThreadManager {
public:
    // Add a thread_base component
    auto add_thread(std::unique_ptr<thread_module::thread_base> thread) -> void {
        threads_.push_back(std::move(thread));
    }
    
    // Start all threads
    auto start_all() -> std::vector<std::pair<size_t, std::string>> {
        std::vector<std::pair<size_t, std::string>> errors;
        
        for (size_t i = 0; i < threads_.size(); ++i) {
            auto result = threads_[i]->start();
            if (result.has_value()) {
                errors.emplace_back(i, result.value());
            }
        }
        
        return errors;
    }
    
    // Stop all threads
    auto stop_all() -> std::vector<std::pair<size_t, std::string>> {
        std::vector<std::pair<size_t, std::string>> errors;
        
        for (size_t i = 0; i < threads_.size(); ++i) {
            auto result = threads_[i]->stop();
            if (result.has_value()) {
                errors.emplace_back(i, result.value());
            }
        }
        
        return errors;
    }
    
private:
    std::vector<std::unique_ptr<thread_module::thread_base>> threads_;
};
```

### Challenge 3: Adapting to Job-based Design

**Problem**: Converting callback-heavy code to Thread System's job-based design.

**Solution**:
```cpp
// Create a job factory for your specific domain
class JobFactory {
public:
    // Create a job from a callback function
    static auto create_callback_job(std::function<void()> callback) 
        -> std::unique_ptr<thread_module::job> {
        return std::make_unique<thread_module::callback_job>(
            [callback]() -> std::optional<std::string> {
                try {
                    callback();
                    return std::nullopt;
                } catch (const std::exception& e) {
                    return e.what();
                }
            }
        );
    }
    
    // Create a domain-specific job
    template<typename T>
    static auto create_processing_job(T data) -> std::unique_ptr<thread_module::job> {
        return std::make_unique<thread_module::callback_job>(
            [data]() -> std::optional<std::string> {
                return process_data(data);
            }
        );
    }
    
private:
    // Domain-specific processing function
    template<typename T>
    static auto process_data(const T& data) -> std::optional<std::string> {
        try {
            // Domain-specific processing
            return std::nullopt;
        } catch (const std::exception& e) {
            return e.what();
        }
    }
};
```

### Challenge 4: Integrating with Existing Libraries

**Problem**: Integrating Thread System with libraries that have their own threading models.

**Solution**:
```cpp
// Create an adapter for library integration
class LibraryThreadAdapter {
public:
    LibraryThreadAdapter(std::shared_ptr<thread_pool_module::thread_pool> pool)
        : thread_pool_(pool) {}
    
    // Convert library callback to Thread System job
    void register_library_callback(std::function<void(LibraryResult)> callback) {
        // Set up library to call our adapter function
        library_instance_.set_callback([this, callback](LibraryResult result) {
            // Create a job to process the callback on our thread pool
            auto job = std::make_unique<thread_module::callback_job>(
                [callback, result]() -> std::optional<std::string> {
                    callback(result);
                    return std::nullopt;
                }
            );
            
            thread_pool_->enqueue(std::move(job));
        });
    }
    
private:
    ExternalLibrary library_instance_;
    std::shared_ptr<thread_pool_module::thread_pool> thread_pool_;
};
```

## Migrating Specific Threading Models

### Single Background Thread

```cpp
// Before: Single background thread
class BackgroundProcessor {
public:
    BackgroundProcessor() : running_(true) {
        thread_ = std::thread(&BackgroundProcessor::run, this);
    }
    
    ~BackgroundProcessor() {
        running_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    
private:
    void run() {
        while (running_) {
            process_background_tasks();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    
    std::atomic<bool> running_;
    std::thread thread_;
};

// After: Thread System background thread
class BackgroundProcessor : public thread_module::thread_base {
public:
    BackgroundProcessor() : thread_base("background_processor") {
        // Wake up every second
        set_wake_interval(std::chrono::seconds(1));
    }
    
protected:
    auto do_work() -> result_void override {
        try {
            process_background_tasks();
            return {};
        } catch (const std::exception& e) {
            log_module::write_error("Background processing error: {}", e.what());
            return make_error(e.what());
        }
    }
};
```

### Worker Thread Pool

```cpp
// Before: Custom worker pool
class WorkerPool {
public:
    WorkerPool(size_t thread_count) {
        workers_.reserve(thread_count);
        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back(std::thread(&WorkerPool::worker_function, this));
        }
    }
    
    void add_task(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        tasks_.push(std::move(task));
        condition_.notify_one();
    }
    
    ~WorkerPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            shutdown_ = true;
        }
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
private:
    void worker_function() {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] {
                    return shutdown_ || !tasks_.empty();
                });
                
                if (shutdown_ && tasks_.empty()) {
                    return;
                }
                
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            task();
        }
    }
    
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool shutdown_ = false;
};

// After: Thread System thread pool
class WorkerPool {
public:
    WorkerPool(size_t thread_count) {
        auto [pool, error] = create_default(thread_count);
        if (error.has_value()) {
            throw std::runtime_error("Failed to create thread pool: " + 
                                    error.value_or("unknown error"));
        }
        
        thread_pool_ = std::move(pool);
        auto start_result = thread_pool_->start();
        if (start_result.has_value()) {
            throw std::runtime_error("Failed to start thread pool: " + 
                                    start_result.value_or("unknown error"));
        }
    }
    
    void add_task(std::function<void()> task) {
        auto job = std::make_unique<thread_module::callback_job>(
            [task]() -> std::optional<std::string> {
                try {
                    task();
                    return std::nullopt;
                } catch (const std::exception& e) {
                    return e.what();
                }
            }
        );
        
        auto error = thread_pool_->enqueue(std::move(job));
        if (error.has_value()) {
            throw std::runtime_error("Failed to enqueue job: " + 
                                    error.value_or("unknown error"));
        }
    }
    
    ~WorkerPool() {
        thread_pool_->stop();
    }
    
private:
    std::shared_ptr<thread_pool_module::thread_pool> thread_pool_;
};
```

### Event Loop

```cpp
// Before: Event loop
class EventLoop {
public:
    void run() {
        running_ = true;
        while (running_) {
            Event event;
            if (event_queue_.try_dequeue(event)) {
                process_event(event);
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
    
    void stop() {
        running_ = false;
    }
    
    void push_event(const Event& event) {
        event_queue_.enqueue(event);
    }
    
private:
    void process_event(const Event& event) {
        // Handle event
    }
    
    ConcurrentQueue<Event> event_queue_;
    std::atomic<bool> running_{false};
};

// After: Thread System based event loop
class EventLoop : public thread_module::thread_base {
public:
    EventLoop() : thread_base("event_loop") {
        // Wake up frequently to check for events
        set_wake_interval(std::chrono::milliseconds(10));
    }
    
    void push_event(const Event& event) {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        event_queue_.push(event);
    }
    
protected:
    auto do_work() -> result_void override {
        Event event;
        bool has_event = false;
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!event_queue_.empty()) {
                event = event_queue_.front();
                event_queue_.pop();
                has_event = true;
            }
        }
        
        if (has_event) {
            try {
                process_event(event);
            } catch (const std::exception& e) {
                return make_error(e.what());
            }
        }
        
        return {};
    }
    
private:
    void process_event(const Event& event) {
        // Handle event
    }
    
    std::queue<Event> event_queue_;
    std::mutex queue_mutex_;
};
```

## Best Practices for Successful Migration

1. **Incremental migration**: Migrate one subsystem at a time, rather than the entire codebase
2. **Start with core components**: Begin with central threading components that have wide impacts
3. **Create adaptation layers**: Build bridges between Thread System and existing code
4. **Thorough testing**: Validate correctness and performance at each step
5. **Monitor error handling**: Ensure all errors are properly captured and reported
6. **Document rationale**: Record why specific migration decisions were made
7. **Train the team**: Ensure all developers understand Thread System concepts

## Migration Checklist

Use this checklist to track your migration progress:

- [ ] Identify all threading code in the application
- [ ] Select appropriate Thread System components for each usage
- [ ] Develop a phased migration plan
- [ ] Create adaptation code where needed
- [ ] Refactor one component at a time
- [ ] Write tests for the migrated code
- [ ] Verify correctness and performance
- [ ] Update documentation
- [ ] Review for further optimizations

## Conclusion

Migrating to Thread System provides significant advantages for code clarity, maintainability, and correctness. By following this guide, you can systematically transform your existing threading code to leverage Thread System's structured approach and advanced features.

Remember that successful migration is an incremental process. Take the time to understand each component, test thoroughly, and gradually build expertise with Thread System's capabilities.