# Logger System Best Practices Guide

## Table of Contents
1. [Design Principles](#design-principles)
2. [Configuration Best Practices](#configuration-best-practices)
3. [Performance Guidelines](#performance-guidelines)
4. [Error Handling](#error-handling)
5. [Security Considerations](#security-considerations)
6. [Testing Strategies](#testing-strategies)
7. [Production Deployment](#production-deployment)
8. [Common Pitfalls](#common-pitfalls)

## Design Principles

### 1. Single Responsibility
Each logger instance should have a clear, single purpose:

```cpp
// Good: Separate loggers for different concerns
auto app_logger = create_logger("application");
auto audit_logger = create_logger("audit");
auto perf_logger = create_logger("performance");

// Avoid: One logger for everything
auto logger = create_logger("everything");
```

### 2. Dependency Injection
Pass loggers as dependencies rather than using global instances:

```cpp
// Good: Dependency injection
class DatabaseService {
    std::shared_ptr<logger> logger_;
public:
    explicit DatabaseService(std::shared_ptr<logger> logger)
        : logger_(std::move(logger)) {}
};

// Avoid: Global logger
class DatabaseService {
    void query() {
        global_logger->info("Query executed");  // Avoid
    }
};
```

### 3. Interface Segregation
Use minimal interfaces when possible:

```cpp
// Good: Accept minimal interface
void process_data(log_writer_interface& writer);

// Avoid: Requiring full logger when only writing is needed
void process_data(logger& full_logger);
```

## Configuration Best Practices

### Environment-Specific Configuration

```cpp
// Development environment
auto create_dev_logger() {
    return logger_builder()
        .with_console_writer()
        .with_min_level(log_level::trace)
        .with_colored_output(true)
        .with_source_location(true)
        .build();
}

// Production environment
auto create_prod_logger() {
    return logger_builder()
        .with_file_writer("/var/log/app.log")
        .with_min_level(log_level::info)
        .with_async_mode(true)
        .with_batch_writing(true)
        .with_rotation(100 * 1024 * 1024)  // 100MB
        .build();
}

// Automatic environment detection
auto create_logger() {
    const auto env = std::getenv("APP_ENV");
    if (env && std::string(env) == "production") {
        return create_prod_logger();
    }
    return create_dev_logger();
}
```

### Configuration Validation

```cpp
// Always validate configuration
auto validate_logger_config(const logger_config& config) -> result<void> {
    if (config.buffer_size < 1024) {
        return error("Buffer size too small");
    }
    if (config.queue_size < 100) {
        return error("Queue size insufficient for production");
    }
    if (config.rotation_size > 1024 * 1024 * 1024) {
        return error("Rotation size too large (>1GB)");
    }
    return success();
}
```

### Dynamic Reconfiguration

```cpp
class reconfigurable_logger {
    std::atomic<log_level> min_level_;
    
public:
    void set_level(log_level level) {
        min_level_.store(level);
        logger_->set_min_level(level);
    }
    
    void reload_config(const std::string& config_file) {
        auto config = load_config(config_file);
        apply_config(config);
    }
};
```

## Performance Guidelines

### 1. Minimize Allocations

```cpp
// Good: Pre-sized string reservation
void log_with_reservation(logger& log) {
    std::string message;
    message.reserve(256);  // Avoid reallocation
    message = format_message(data);
    log.info(message);
}

// Good: Use string_view for literals
log.info(std::string_view("Static message"));

// Avoid: Unnecessary string copies
std::string msg = "Message";
log.info(msg);  // Avoid copy
log.info(std::move(msg));  // Better
```

### 2. Lazy Evaluation

```cpp
// Good: Check level before expensive operations
if (logger->should_log(log_level::debug)) {
    auto debug_info = collect_expensive_debug_info();
    logger->debug("Debug info", {{"data", debug_info}});
}

// Avoid: Always computing expensive data
auto debug_info = collect_expensive_debug_info();
logger->debug("Debug info", {{"data", debug_info}});
```

### 3. Batch Operations

```cpp
// Good: Batch multiple log entries
class log_batcher {
    std::vector<log_entry> batch_;
    static constexpr size_t BATCH_SIZE = 100;
    
    void add(log_entry entry) {
        batch_.push_back(std::move(entry));
        if (batch_.size() >= BATCH_SIZE) {
            flush();
        }
    }
    
    void flush() {
        logger_->write_batch(batch_);
        batch_.clear();
    }
};
```

### 4. Async Logging Pattern

```cpp
// High-throughput async configuration
auto create_high_throughput_logger() {
    return logger_builder()
        .with_async_mode(true)
        .with_queue_size(100'000)
        .with_overflow_policy(overflow_policy::drop_oldest)
        .with_batch_writing(true)
        .with_batch_size(1000)
        .with_flush_interval(std::chrono::milliseconds(100))
        .build();
}
```

## Error Handling

### Graceful Degradation

```cpp
class resilient_logger {
    std::shared_ptr<logger> primary_;
    std::shared_ptr<logger> fallback_;
    
public:
    void log(log_level level, const std::string& msg) {
        try {
            primary_->log(level, msg);
        } catch (const std::exception& e) {
            // Fall back to secondary logger
            try {
                fallback_->log(level, msg);
            } catch (...) {
                // Last resort: stderr
                std::cerr << "[EMERGENCY] " << msg << std::endl;
            }
        }
    }
};
```

### Error Recovery

```cpp
// Automatic reconnection for file writers
class reconnecting_file_writer : public base_writer {
    void write(const log_entry& entry) override {
        if (!is_connected()) {
            if (!try_reconnect()) {
                buffer_entry(entry);  // Buffer until reconnected
                return;
            }
        }
        
        try {
            write_impl(entry);
        } catch (const std::exception& e) {
            handle_write_error(e);
        }
    }
};
```

### Circuit Breaker Pattern

```cpp
class circuit_breaker_logger {
    enum class State { CLOSED, OPEN, HALF_OPEN };
    std::atomic<State> state_{State::CLOSED};
    std::atomic<int> failure_count_{0};
    static constexpr int FAILURE_THRESHOLD = 5;
    
    void log(const std::string& msg) {
        if (state_ == State::OPEN) {
            if (should_attempt_reset()) {
                state_ = State::HALF_OPEN;
            } else {
                return;  // Circuit open, skip logging
            }
        }
        
        try {
            logger_->info(msg);
            if (state_ == State::HALF_OPEN) {
                state_ = State::CLOSED;
                failure_count_ = 0;
            }
        } catch (...) {
            if (++failure_count_ >= FAILURE_THRESHOLD) {
                state_ = State::OPEN;
            }
            throw;
        }
    }
};
```

## Security Considerations

### 1. Sanitize Sensitive Data

```cpp
class sanitizing_logger {
    std::regex credit_card_pattern{"\\b\\d{4}[\\s-]?\\d{4}[\\s-]?\\d{4}[\\s-]?\\d{4}\\b"};
    std::regex email_pattern{"\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}\\b"};
    
    std::string sanitize(const std::string& message) {
        std::string result = message;
        result = std::regex_replace(result, credit_card_pattern, "****-****-****-****");
        result = std::regex_replace(result, email_pattern, "****@****.***");
        return result;
    }
    
public:
    void log(const std::string& message) {
        logger_->info(sanitize(message));
    }
};
```

### 2. Access Control

```cpp
class access_controlled_logger {
    enum class LogAccess { READ, WRITE, ADMIN };
    std::unordered_map<std::string, LogAccess> permissions_;
    
    bool can_log(const std::string& user, log_level level) {
        auto it = permissions_.find(user);
        if (it == permissions_.end()) return false;
        
        // Critical logs require ADMIN access
        if (level == log_level::critical) {
            return it->second == LogAccess::ADMIN;
        }
        
        return it->second >= LogAccess::WRITE;
    }
};
```

### 3. Log Injection Prevention

```cpp
std::string escape_log_injection(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 1.2);
    
    for (char c : input) {
        switch (c) {
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            default:
                if (std::isprint(c)) {
                    output += c;
                } else {
                    output += "\\x" + to_hex(c);
                }
        }
    }
    return output;
}
```

## Testing Strategies

### 1. Mock Logger for Testing

```cpp
class mock_logger : public logger_interface {
    std::vector<std::string> captured_logs_;
    
public:
    void log(log_level level, const std::string& message) override {
        captured_logs_.push_back(message);
    }
    
    bool contains(const std::string& expected) const {
        return std::find(captured_logs_.begin(), 
                        captured_logs_.end(), 
                        expected) != captured_logs_.end();
    }
    
    size_t log_count() const { return captured_logs_.size(); }
    void clear() { captured_logs_.clear(); }
};

// Usage in tests
TEST(ServiceTest, LogsErrors) {
    auto mock_log = std::make_shared<mock_logger>();
    Service service(mock_log);
    
    service.process_invalid_data();
    
    EXPECT_TRUE(mock_log->contains("Invalid data"));
    EXPECT_EQ(mock_log->log_count(), 1);
}
```

### 2. Performance Testing

```cpp
void benchmark_logger_throughput() {
    auto logger = create_test_logger();
    const size_t iterations = 1'000'000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < iterations; ++i) {
        logger->info("Benchmark message", {{"iteration", i}});
    }
    
    logger->flush();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    auto throughput = iterations * 1000 / duration.count();
    
    std::cout << "Throughput: " << throughput << " msg/sec\n";
    EXPECT_GT(throughput, 100'000);  // Minimum expected throughput
}
```

### 3. Stress Testing

```cpp
void stress_test_concurrent_logging() {
    auto logger = create_thread_safe_logger();
    const size_t num_threads = 10;
    const size_t logs_per_thread = 10'000;
    
    std::vector<std::thread> threads;
    std::atomic<size_t> total_logged{0};
    
    for (size_t t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (size_t i = 0; i < logs_per_thread; ++i) {
                logger->info("Thread message", {
                    {"thread_id", t},
                    {"message_id", i}
                });
                total_logged++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(total_logged, num_threads * logs_per_thread);
}
```

## Production Deployment

### 1. Monitoring Integration

```cpp
class monitored_logger {
    std::shared_ptr<metrics_collector> metrics_;
    
    void log(log_level level, const std::string& message) {
        auto start = std::chrono::high_resolution_clock::now();
        
        logger_->log(level, message);
        
        auto duration = std::chrono::high_resolution_clock::now() - start;
        
        // Update metrics
        metrics_->record_log_latency(duration);
        metrics_->increment_log_count(level);
        
        // Alert on high latency
        if (duration > std::chrono::milliseconds(100)) {
            alert_->send("High logging latency detected");
        }
    }
};
```

### 2. Log Rotation Strategy

```cpp
// Time and size based rotation
auto setup_production_rotation() {
    return logger_builder()
        .with_file_writer("/var/log/app.log")
        .with_rotation_size(100 * 1024 * 1024)     // 100MB
        .with_rotation_time(std::chrono::hours(24)) // Daily
        .with_max_files(30)                         // Keep 30 days
        .with_compression(true)                     // Compress rotated files
        .build();
}
```

### 3. Graceful Shutdown

```cpp
class application {
    std::shared_ptr<logger> logger_;
    
    void shutdown() {
        logger_->info("Application shutting down");
        
        // Flush all pending logs
        logger_->flush();
        
        // Wait for async operations
        logger_->stop();
        
        // Final message
        logger_->info("Shutdown complete");
    }
};
```

## Common Pitfalls

### 1. Memory Leaks

```cpp
// Avoid: Circular references
class bad_service {
    std::shared_ptr<logger> logger_;
    
    bad_service() {
        logger_ = std::make_shared<logger>();
        // Avoid: Logger holds reference back to service
        logger_->set_context("service", this);  
    }
};

// Good: Use weak_ptr for callbacks
class good_service {
    std::shared_ptr<logger> logger_;
    
    good_service() {
        logger_ = std::make_shared<logger>();
        std::weak_ptr<good_service> weak_this = shared_from_this();
        logger_->set_error_handler([weak_this](const error& e) {
            if (auto self = weak_this.lock()) {
                self->handle_error(e);
            }
        });
    }
};
```

### 2. Deadlocks

```cpp
// Avoid: Logging while holding locks
class bad_example {
    std::mutex mutex_;
    
    void process() {
        std::lock_guard<std::mutex> lock(mutex_);
        logger_->info("Processing");  // Avoid: May deadlock
        do_work();
    }
};

// Good: Log outside critical sections
class good_example {
    std::mutex mutex_;
    
    void process() {
        logger_->info("Starting process");
        {
            std::lock_guard<std::mutex> lock(mutex_);
            do_work();
        }
        logger_->info("Process complete");
    }
};
```

### 3. Performance Bottlenecks

```cpp
// Avoid: Synchronous I/O in hot paths
void hot_path_bad() {
    for (int i = 0; i < 1000000; ++i) {
        logger_->info("Processing item " + std::to_string(i));  // Blocks
    }
}

// Good: Async logging in hot paths
void hot_path_good() {
    auto async_logger = create_async_logger();
    for (int i = 0; i < 1000000; ++i) {
        async_logger->info("Processing item", {{"id", i}});  // Non-blocking
    }
}
```

## Summary

Following these best practices will help you:
- Build more reliable and maintainable logging solutions
- Achieve better performance in production environments
- Avoid common pitfalls and security issues
- Create testable and debuggable applications
- Scale your logging infrastructure effectively

Remember: Good logging is invisible when everything works but invaluable when things go wrong.