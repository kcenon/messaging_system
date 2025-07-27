# Thread System Examples

This guide contains practical examples demonstrating how to use the Thread System framework in real-world scenarios.

## Quick Start Examples

### Hello World with Thread Pool

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <iostream>

int main() {
    // Create thread pool
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    
    // Start pool
    pool->start();
    
    // Submit job
    pool->enqueue(std::make_unique<thread_module::callback_job>(
        []() -> thread_module::result_void {
            std::cout << "Hello from thread pool!" << std::endl;
            return {};
        }
    ));
    
    // Wait for completion
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Clean up
    pool->stop();
    
    return 0;
}
```

### Parallel Computation

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <numeric>
#include <vector>
#include <memory>

double parallel_sum(const std::vector<double>& data) {
    // Create thread pool
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add 4 workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    const size_t chunk_size = data.size() / 4;
    std::vector<std::shared_ptr<double>> results;
    
    for (size_t i = 0; i < 4; ++i) {
        auto start = data.begin() + i * chunk_size;
        auto end = (i == 3) ? data.end() : start + chunk_size;
        
        auto result = std::make_shared<double>(0.0);
        results.push_back(result);
        
        pool->enqueue(std::make_unique<thread_module::callback_job>(
            [start, end, result]() -> thread_module::result_void {
                *result = std::accumulate(start, end, 0.0);
                return {};
            }
        ));
    }
    
    // Wait for all jobs to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    double total = 0.0;
    for (const auto& result : results) {
        total += *result;
    }
    
    pool->stop();
    return total;
}
```

## Basic Examples

### Simple Job Submission

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <iostream>

int main() {
    // Create and configure pool
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add 2 workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    for (int i = 0; i < 2; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    
    if (auto error = pool->enqueue_batch(std::move(workers))) {
        std::cerr << "Failed to add workers: " << *error << std::endl;
        return 1;
    }
    
    if (auto error = pool->start()) {
        std::cerr << "Failed to start pool: " << *error << std::endl;
        return 1;
    }
    
    // Submit job with shared result
    auto result = std::make_shared<int>(0);
    pool->enqueue(std::make_unique<thread_module::callback_job>(
        [result]() -> thread_module::result_void {
            *result = 42;
            return {};
        }
    ));
    
    // Wait and get result
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Result: " << *result << std::endl;
    
    pool->stop();
    return 0;
}
```

### Error Handling

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <iostream>

int main() {
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    pool->enqueue_batch(std::move(workers));
    
    pool->start();
    
    // Submit job that fails
    pool->enqueue(std::make_unique<thread_module::callback_job>(
        []() -> thread_module::result_void {
            // Simulate error
            return thread_module::error_info(
                thread_module::error_type::runtime_error,
                "Something went wrong!"
            );
        }
    ));
    
    // The error will be logged by the worker
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    pool->stop();
    return 0;
}
```

### File Processing

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <mutex>

void process_files(const std::vector<std::filesystem::path>& files) {
    // Create pool
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    // Process each file
    std::vector<std::shared_ptr<size_t>> line_counts;
    auto output_mutex = std::make_shared<std::mutex>();
    
    for (const auto& file : files) {
        auto count = std::make_shared<size_t>(0);
        line_counts.push_back(count);
        
        pool->enqueue(std::make_unique<thread_module::callback_job>(
            [file, count, output_mutex]() -> thread_module::result_void {
                std::ifstream ifs(file);
                if (!ifs) {
                    return thread_module::error_info(
                        thread_module::error_type::file_not_found,
                        "Cannot open file: " + file.string()
                    );
                }
                
                std::string line;
                while (std::getline(ifs, line)) {
                    (*count)++;
                }
                
                {
                    std::lock_guard<std::mutex> lock(*output_mutex);
                    std::cout << "File " << file.filename().string() 
                              << " has " << *count << " lines" << std::endl;
                }
                return {};
            }
        ));
    }
    
    // Wait for all files to be processed
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Report total
    size_t total = 0;
    for (const auto& count : line_counts) {
        total += *count;
    }
    std::cout << "Total lines: " << total << std::endl;
    
    // Clean up
    pool->stop();
}
```

## Type-Based Thread Pool Examples

### Basic Type Scheduling

```cpp
#include "typed_thread_pool/pool/typed_thread_pool.h"
#include "typed_thread_pool/jobs/callback_typed_job.h"
#include <iostream>
#include <mutex>

int main() {
    // Create typed thread pool
    auto pool = std::make_shared<typed_thread_pool_module::typed_thread_pool>();
    auto output_mutex = std::make_shared<std::mutex>();
    
    // Add workers with different type responsibilities
    // Worker 1: Only handles High priority
    pool->enqueue(std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
        std::initializer_list<job_types>{job_types::High}
    ));
    
    // Worker 2: Handles Normal and Low priority
    pool->enqueue(std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
        std::initializer_list<job_types>{job_types::Normal, job_types::Low}
    ));
    
    pool->start();
    
    // Submit jobs with different priorities
    for (int i = 0; i < 5; ++i) {
        pool->enqueue(std::make_unique<typed_thread_pool_module::callback_typed_job<job_types>>(
            job_types::High,
            [i, output_mutex]() -> thread_module::result_void {
                std::lock_guard<std::mutex> lock(*output_mutex);
                std::cout << "High priority job " << i << std::endl;
                return {};
            }
        ));
        
        pool->enqueue(std::make_unique<typed_thread_pool_module::callback_typed_job<job_types>>(
            job_types::Low,
            [i, output_mutex]() -> thread_module::result_void {
                std::lock_guard<std::mutex> lock(*output_mutex);
                std::cout << "Low priority job " << i << std::endl;
                return {};
            }
        ));
    }
    
    // High priority jobs will be processed first
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    pool->stop();
    return 0;
}
```

### Custom Type Types

```cpp
#include "typed_thread_pool/pool/typed_thread_pool.h"
#include "typed_thread_pool/jobs/typed_job.h"
#include <iostream>
#include <mutex>

// Define custom priority types
enum class TaskPriority : uint8_t {
    Critical = 1,
    UserInteractive = 2,
    Background = 3,
    Maintenance = 4
};

// Custom job implementation
class MyCustomJob : public typed_thread_pool_module::typed_job_t<TaskPriority> {
private:
    std::string task_name_;
    std::shared_ptr<std::mutex> output_mutex_;
    
public:
    MyCustomJob(TaskPriority priority, const std::string& name, 
                std::shared_ptr<std::mutex> mutex)
        : typed_job_t<TaskPriority>(priority), 
          task_name_(name), 
          output_mutex_(mutex) {}
    
    auto operator()() -> thread_module::result_void override {
        std::lock_guard<std::mutex> lock(*output_mutex_);
        std::cout << "Executing priority " << static_cast<int>(get_type()) 
                  << ": " << task_name_ << std::endl;
        // Do actual work here
        return {};
    }
};

int main() {
    // Create pool with custom priority type
    auto pool = std::make_shared<typed_thread_pool_module::typed_thread_pool_t<TaskPriority>>();
    auto output_mutex = std::make_shared<std::mutex>();
    
    // Add workers
    pool->enqueue(std::make_unique<typed_thread_pool_module::typed_thread_worker_t<TaskPriority>>(
        std::initializer_list<TaskPriority>{TaskPriority::Critical, TaskPriority::UserInteractive}
    ));
    
    pool->start();
    
    // Submit custom jobs
    pool->enqueue(std::make_unique<MyCustomJob>(TaskPriority::Critical, "Database backup", output_mutex));
    pool->enqueue(std::make_unique<MyCustomJob>(TaskPriority::Background, "Cache cleanup", output_mutex));
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    pool->stop();
    return 0;
}
```

## Advanced Thread Pool Examples

### Custom Job Priority Queue

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <queue>
#include <mutex>
#include <condition_variable>

class PriorityJobQueue {
private:
    struct PriorityJob {
        int priority;
        std::function<void()> task;
        
        bool operator<(const PriorityJob& other) const {
            return priority < other.priority; // Higher priority first
        }
    };
    
    std::priority_queue<PriorityJob> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
    
public:
    void add_job(int priority, std::function<void()> task) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push({priority, std::move(task)});
        cv_.notify_one();
    }
    
    std::optional<std::function<void()>> get_job() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        
        if (stopped_ && queue_.empty()) {
            return std::nullopt;
        }
        
        auto job = queue_.top();
        queue_.pop();
        return job.task;
    }
    
    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
        cv_.notify_all();
    }
};

void example_priority_processing() {
    auto queue = std::make_shared<PriorityJobQueue>();
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    for (int i = 0; i < 2; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    // Worker thread to process priority queue
    pool->enqueue(std::make_unique<thread_module::callback_job>(
        [queue]() -> thread_module::result_void {
            while (auto job = queue->get_job()) {
                (*job)();
            }
            return {};
        }
    ));
    
    // Add jobs with different priorities
    queue->add_job(1, []() { std::cout << "Low priority task" << std::endl; });
    queue->add_job(10, []() { std::cout << "High priority task" << std::endl; });
    queue->add_job(5, []() { std::cout << "Medium priority task" << std::endl; });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    queue->stop();
    pool->stop();
}
```

### Performance Measurement

```cpp
#include "thread_base/core/thread_base.h"
#include <chrono>
#include <iostream>
#include <iomanip>

class PerformanceTimer {
private:
    std::string operation_;
    std::chrono::high_resolution_clock::time_point start_;
    
public:
    PerformanceTimer(const std::string& operation) 
        : operation_(operation), 
          start_(std::chrono::high_resolution_clock::now()) {
        std::cout << "Starting: " << operation_ << std::endl;
    }
    
    ~PerformanceTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        std::cout << "Completed: " << operation_ 
                  << " (" << std::fixed << std::setprecision(2) 
                  << duration.count() / 1000.0 << "ms)" << std::endl;
    }
};

void measure_thread_pool_performance() {
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Add workers
    std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
    }
    pool->enqueue_batch(std::move(workers));
    pool->start();
    
    {
        PerformanceTimer timer("1000 job submissions");
        
        std::atomic<int> completed{0};
        for (int i = 0; i < 1000; ++i) {
            pool->enqueue(std::make_unique<thread_module::callback_job>(
                [&completed]() -> thread_module::result_void {
                    // Simulate work
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                    completed++;
                    return {};
                }
            ));
        }
        
        // Wait for completion
        while (completed < 1000) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    pool->stop();
}
```

## Real-World Scenarios

### Web Server Request Handler

```cpp
#include "thread_pool/core/thread_pool.h"
#include "typed_thread_pool/pool/typed_thread_pool.h"
#include <iostream>
#include <mutex>

class WebServer {
private:
    std::shared_ptr<typed_thread_pool_module::typed_thread_pool> request_pool_;
    std::shared_ptr<thread_pool_module::thread_pool> io_pool_;
    std::shared_ptr<std::mutex> output_mutex_;
    
public:
    WebServer() : output_mutex_(std::make_shared<std::mutex>()) {
        // Request handling pool with priorities
        request_pool_ = std::make_shared<typed_thread_pool_module::typed_thread_pool>();
        
        // High priority worker for API requests
        request_pool_->enqueue(
            std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
                std::initializer_list<job_types>{job_types::High}
            )
        );
        
        // Normal priority workers for regular requests
        for (int i = 0; i < 4; ++i) {
            request_pool_->enqueue(
                std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
                    std::initializer_list<job_types>{job_types::Normal, job_types::Low}
                )
            );
        }
        
        // I/O pool for file operations
        io_pool_ = std::make_shared<thread_pool_module::thread_pool>();
        std::vector<std::unique_ptr<thread_pool_module::thread_worker>> io_workers;
        for (int i = 0; i < 2; ++i) {
            io_workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
        }
        io_pool_->enqueue_batch(std::move(io_workers));
    }
    
    void start() {
        request_pool_->start();
        io_pool_->start();
        std::lock_guard<std::mutex> lock(*output_mutex_);
        std::cout << "Web server started" << std::endl;
    }
    
    void handle_request(const std::string& path, bool is_api) {
        auto priority = is_api ? job_types::High : job_types::Normal;
        
        request_pool_->enqueue(
            std::make_unique<typed_thread_pool_module::callback_typed_job<job_types>>(
                priority,
                [this, path]() -> thread_module::result_void {
                    {
                        std::lock_guard<std::mutex> lock(*output_mutex_);
                        std::cout << "Processing request: " << path << std::endl;
                    }
                    
                    // Process request
                    if (path.starts_with("/static/")) {
                        serve_static_file(path.substr(8));
                    } else {
                        generate_response(path);
                    }
                    
                    return {};
                }
            )
        );
    }
    
    void stop() {
        request_pool_->stop();
        io_pool_->stop();
        std::lock_guard<std::mutex> lock(*output_mutex_);
        std::cout << "Web server stopped" << std::endl;
    }
    
private:
    void serve_static_file(const std::string& file) {
        io_pool_->enqueue(std::make_unique<thread_module::callback_job>(
            [this, file]() -> thread_module::result_void {
                // Read and serve file
                std::lock_guard<std::mutex> lock(*output_mutex_);
                std::cout << "Serving static file: " << file << std::endl;
                return {};
            }
        ));
    }
    
    void generate_response(const std::string& path) {
        // Generate dynamic response
        std::lock_guard<std::mutex> lock(*output_mutex_);
        std::cout << "Generating response for: " << path << std::endl;
    }
};
```

### Data Processing Pipeline

```cpp
#include "typed_thread_pool/pool/typed_thread_pool.h"
#include <queue>
#include <mutex>
#include <iostream>
#include <numeric>

class DataPipeline {
private:
    struct DataItem {
        int id;
        std::vector<double> data;
    };
    
    std::shared_ptr<typed_thread_pool_module::typed_thread_pool> pool_;
    std::queue<DataItem> input_queue_;
    std::mutex queue_mutex_;
    std::mutex output_mutex_;
    
public:
    DataPipeline() {
        pool_ = std::make_shared<typed_thread_pool_module::typed_thread_pool>();
        
        // Stage 1: Data ingestion (high priority)
        pool_->enqueue(
            std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
                std::initializer_list<job_types>{job_types::High}
            )
        );
        
        // Stage 2: Processing (normal priority)
        for (int i = 0; i < 3; ++i) {
            pool_->enqueue(
                std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
                    std::initializer_list<job_types>{job_types::Normal}
                )
            );
        }
        
        // Stage 3: Cleanup (low priority)
        pool_->enqueue(
            std::make_unique<typed_thread_pool_module::typed_thread_worker_t<job_types>>(
                std::initializer_list<job_types>{job_types::Low}
            )
        );
    }
    
    void process_batch(const std::vector<DataItem>& items) {
        // Stage 1: Ingest data
        for (const auto& item : items) {
            pool_->enqueue(
                std::make_unique<typed_thread_pool_module::callback_typed_job<job_types>>(
                    job_types::High,
                    [this, item]() -> thread_module::result_void {
                        validate_and_queue(item);
                        return {};
                    }
                )
            );
        }
        
        // Stage 2: Process data
        pool_->enqueue(
            std::make_unique<typed_thread_pool_module::callback_typed_job<job_types>>(
                job_types::Normal,
                [this]() -> thread_module::result_void {
                    process_queued_items();
                    return {};
                }
            )
        );
        
        // Stage 3: Cleanup
        pool_->enqueue(
            std::make_unique<typed_thread_pool_module::callback_typed_job<job_types>>(
                job_types::Low,
                [this]() -> thread_module::result_void {
                    cleanup_processed_data();
                    return {};
                }
            )
        );
    }
    
private:
    void validate_and_queue(const DataItem& item) {
        {
            std::lock_guard<std::mutex> lock(output_mutex_);
            std::cout << "Validating item " << item.id << std::endl;
        }
        
        // Validation logic
        if (item.data.empty()) {
            std::lock_guard<std::mutex> lock(output_mutex_);
            std::cerr << "Invalid data for item " << item.id << std::endl;
            return;
        }
        
        std::lock_guard<std::mutex> lock(queue_mutex_);
        input_queue_.push(item);
    }
    
    void process_queued_items() {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!input_queue_.empty()) {
            auto item = input_queue_.front();
            input_queue_.pop();
            
            // Process data
            double sum = std::accumulate(item.data.begin(), item.data.end(), 0.0);
            
            std::lock_guard<std::mutex> output_lock(output_mutex_);
            std::cout << "Processed item " << item.id 
                      << ": sum = " << sum << std::endl;
        }
    }
    
    void cleanup_processed_data() {
        std::lock_guard<std::mutex> lock(output_mutex_);
        std::cout << "Performing cleanup" << std::endl;
        // Cleanup logic
    }
};
```

## Best Practices

### Resource Management

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <iostream>
#include <atomic>
#include <chrono>

class ResourcePool {
private:
    std::shared_ptr<thread_pool_module::thread_pool> pool_;
    
public:
    ResourcePool() {
        pool_ = std::make_shared<thread_pool_module::thread_pool>();
        
        // Add workers
        std::vector<std::unique_ptr<thread_pool_module::thread_worker>> workers;
        for (int i = 0; i < 2; ++i) {
            workers.push_back(std::make_unique<thread_pool_module::thread_worker>());
        }
        pool_->enqueue_batch(std::move(workers));
        pool_->start();
    }
    
    // RAII pattern for automatic cleanup
    ~ResourcePool() {
        if (pool_) {
            pool_->stop();
        }
    }
    
    void process_with_timeout(std::function<void()> task, 
                             std::chrono::milliseconds timeout) {
        auto done = std::make_shared<std::atomic<bool>>(false);
        
        pool_->enqueue(std::make_unique<thread_module::callback_job>(
            [task, done]() -> thread_module::result_void {
                task();
                done->store(true);
                return {};
            }
        ));
        
        // Wait with timeout
        auto start = std::chrono::steady_clock::now();
        while (!done->load()) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                std::cerr << "Task timed out" << std::endl;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};
```

### Exception Safety

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
#include <future>
#include <optional>
#include <iostream>

template<typename T>
class SafeJobExecutor {
private:
    std::shared_ptr<thread_pool_module::thread_pool> pool_;
    
public:
    SafeJobExecutor(std::shared_ptr<thread_pool_module::thread_pool> pool) 
        : pool_(pool) {}
    
    std::future<std::optional<T>> execute_safe(std::function<T()> task) {
        auto promise = std::make_shared<std::promise<std::optional<T>>>();
        auto future = promise->get_future();
        
        pool_->enqueue(std::make_unique<thread_module::callback_job>(
            [task, promise]() -> thread_module::result_void {
                try {
                    T result = task();
                    promise->set_value(result);
                } catch (const std::exception& e) {
                    std::cerr << "Job failed: " << e.what() << std::endl;
                    promise->set_value(std::nullopt);
                } catch (...) {
                    std::cerr << "Job failed: unknown error" << std::endl;
                    promise->set_value(std::nullopt);
                }
                return {};
            }
        ));
        
        return future;
    }
};
```

## Examples with Optional Modules

The Thread System can be enhanced with optional modules like logger and monitoring. These modules are maintained as separate projects but can be integrated when needed.

### Integration with Logger Module

If you have the logger module installed separately, you can enhance your thread pool with logging capabilities:

```cpp
#include "thread_pool/core/thread_pool.h"
#include "thread_base/jobs/callback_job.h"
// #include "logger/core/logger.h" // Include if logger module is available

class ThreadPoolWithOptionalLogging {
private:
    std::shared_ptr<thread_pool_module::thread_pool> pool_;
    bool logging_enabled_;
    
public:
    ThreadPoolWithOptionalLogging(bool enable_logging = false) 
        : logging_enabled_(enable_logging) {
        pool_ = std::make_shared<thread_pool_module::thread_pool>();
        
        // Initialize logger if available and enabled
        #ifdef HAS_LOGGER_MODULE
        if (logging_enabled_) {
            log_module::start();
            log_module::write_information("Thread pool initialized with logging");
        }
        #endif
    }
    
    ~ThreadPoolWithOptionalLogging() {
        pool_->stop();
        
        #ifdef HAS_LOGGER_MODULE
        if (logging_enabled_) {
            log_module::stop();
        }
        #endif
    }
    
    void submit_job(std::function<void()> task) {
        pool_->enqueue(std::make_unique<thread_module::callback_job>(
            [this, task]() -> thread_module::result_void {
                #ifdef HAS_LOGGER_MODULE
                if (logging_enabled_) {
                    log_module::write_debug("Starting job execution");
                }
                #endif
                
                task();
                
                #ifdef HAS_LOGGER_MODULE
                if (logging_enabled_) {
                    log_module::write_debug("Job execution completed");
                }
                #endif
                
                return {};
            }
        ));
    }
};
```

### Custom Error Handler with Optional Monitoring

You can implement custom error handlers that integrate with external monitoring systems:

```cpp
#include "thread_pool/core/thread_pool.h"
#include "interfaces/error_handler.h"

class CustomErrorHandler : public thread_module::error_handler {
private:
    std::function<void(const std::string&)> monitor_callback_;
    
public:
    CustomErrorHandler(std::function<void(const std::string&)> callback = nullptr)
        : monitor_callback_(callback) {}
    
    void handle_error(const thread_module::error_info& error) override {
        // Basic error handling
        std::cerr << "Thread error: " << error.message() << std::endl;
        
        // Optional monitoring integration
        if (monitor_callback_) {
            monitor_callback_(error.message());
        }
        
        // Could also integrate with external monitoring services
        #ifdef HAS_MONITORING_MODULE
        monitoring_module::report_error(error.type(), error.message());
        #endif
    }
};

// Usage example
void setup_thread_pool_with_monitoring() {
    auto pool = std::make_shared<thread_pool_module::thread_pool>();
    
    // Set up custom error handler with monitoring callback
    auto error_handler = std::make_unique<CustomErrorHandler>(
        [](const std::string& error) {
            // Send to external monitoring service
            std::cout << "Monitoring alert: " << error << std::endl;
        }
    );
    
    // Configure pool with custom error handler
    // pool->set_error_handler(std::move(error_handler));
}
```

### Building with Optional Modules

When building your application, you can conditionally include optional modules:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyApplication)

# Core thread system
find_package(ThreadSystem REQUIRED)

# Optional modules
find_package(LoggerModule QUIET)
find_package(MonitoringModule QUIET)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ThreadSystem::thread_system)

# Conditionally link optional modules
if(LoggerModule_FOUND)
    target_compile_definitions(my_app PRIVATE HAS_LOGGER_MODULE)
    target_link_libraries(my_app PRIVATE LoggerModule::logger)
endif()

if(MonitoringModule_FOUND)
    target_compile_definitions(my_app PRIVATE HAS_MONITORING_MODULE)
    target_link_libraries(my_app PRIVATE MonitoringModule::monitoring)
endif()
```

### Compilation Instructions

For basic examples without optional modules:
```bash
# Linux/macOS
g++ -std=c++20 -I/path/to/thread_system/include example.cpp -lthread_system -pthread

# Windows (MSVC)
cl /std:c++20 /I"path\to\thread_system\include" example.cpp thread_system.lib
```

For examples with optional modules:
```bash
# With logger module
g++ -std=c++20 -DHAS_LOGGER_MODULE \
    -I/path/to/thread_system/include \
    -I/path/to/logger_module/include \
    example.cpp -lthread_system -llogger -pthread

# With both logger and monitoring
g++ -std=c++20 -DHAS_LOGGER_MODULE -DHAS_MONITORING_MODULE \
    -I/path/to/thread_system/include \
    -I/path/to/logger_module/include \
    -I/path/to/monitoring_module/include \
    example.cpp -lthread_system -llogger -lmonitoring -pthread
```