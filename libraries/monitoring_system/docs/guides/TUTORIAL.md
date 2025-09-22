# Monitoring System Tutorial

## Introduction

Welcome to the Monitoring System tutorial! This guide will walk you through using the monitoring system in your applications, from basic setup to advanced features.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Basic Monitoring](#basic-monitoring)
3. [Distributed Tracing](#distributed-tracing)
4. [Health Monitoring](#health-monitoring)
5. [Reliability Features](#reliability-features)
6. [Error Handling with Result Pattern](#error-handling-with-result-pattern)
7. [Best Practices](#best-practices)

---

## Getting Started

### Prerequisites

- C++17 or later compiler
- CMake 3.15 or later
- Thread support

### Installation

1. Clone the repository:
```bash
git clone <repository-url>
cd monitoring_system
```

2. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

3. Run tests:
```bash
./tests/monitoring_system_tests
```

### Including in Your Project

Add to your CMakeLists.txt:
```cmake
add_subdirectory(monitoring_system)
target_link_libraries(your_app PRIVATE monitoring_system)
```

Include headers in your code:
```cpp
#include <monitoring/monitoring.h>
#include <monitoring/performance/performance_monitor.h>
```

---

## Basic Monitoring

### Step 1: Initialize the Monitoring System

```cpp
#include <monitoring/monitoring.h>

using namespace monitoring_system;

// Configure monitoring
monitoring_config config;
config.history_size = 1000;
config.collection_interval = std::chrono::seconds(1);

// Build monitoring instance
monitoring_builder builder;
auto monitoring_result = builder
    .with_history_size(config.history_size)
    .with_collection_interval(config.collection_interval)
    .enable_compression(true)
    .build();

if (!monitoring_result) {
    // Handle error
    std::cerr << "Failed: " << monitoring_result.get_error().message << std::endl;
    return;
}

auto& monitoring = *monitoring_result.value();
```

### Step 2: Add Collectors

```cpp
// Add performance monitor
auto perf_monitor = std::make_unique<performance_monitor>("my_app");
monitoring.add_collector(std::move(perf_monitor));

// Start monitoring
monitoring.start();
```

### Step 3: Record Metrics

```cpp
// Record custom metrics
monitoring.record_metric("request_count", 1.0, metric_unit::count);
monitoring.record_metric("response_time", 45.3, metric_unit::milliseconds);
monitoring.record_metric("memory_usage", 128.5, metric_unit::megabytes);

// Use scoped timer for automatic duration measurement
{
    auto timer = perf_monitor->time_operation("database_query");
    // ... perform database query ...
} // Timer automatically records duration when destroyed
```

### Step 4: Query Metrics

```cpp
// Get snapshot of current metrics
auto snapshot = monitoring.get_snapshot();
if (snapshot) {
    for (const auto& [name, data] : snapshot.value().metrics) {
        std::cout << name << ": " << data.values.size() << " samples" << std::endl;
    }
}

// Get statistics
auto stats = monitoring.get_statistics();
std::cout << "Metrics recorded: " << stats.metrics_recorded << std::endl;
```

### Complete Example

See [basic_monitoring_example.cpp](basic_monitoring_example.cpp) for a complete working example.

---

## Distributed Tracing

### Creating Spans

```cpp
#include <monitoring/tracing/distributed_tracer.h>

distributed_tracer tracer;

// Start a root span
auto root_span = tracer.start_span("process_request", "frontend_service");
if (root_span) {
    auto span = root_span.value();
    
    // Add tags
    span->tags["http.method"] = "GET";
    span->tags["http.url"] = "/api/users";
    span->tags["user.id"] = "12345";
    
    // Add baggage (propagated to children)
    span->baggage["session.id"] = "abc123";
    
    // Finish span
    tracer.finish_span(span);
}
```

### Parent-Child Relationships

```cpp
// Create child span
auto child_span = tracer.start_child_span(*parent_span, "database_query");
if (child_span) {
    auto span = child_span.value();
    span->tags["db.type"] = "postgresql";
    span->tags["db.statement"] = "SELECT * FROM users";
    
    // Perform operation...
    
    tracer.finish_span(span);
}
```

### Context Propagation

```cpp
// Extract context for propagation
auto context = tracer.extract_context(*span);

// Inject into HTTP headers
std::map<std::string, std::string> headers;
tracer.inject_context(context, headers);

// In receiving service, extract context
auto extracted = tracer.extract_context_from_carrier(headers);
if (extracted) {
    // Continue trace
    auto continued_span = tracer.start_span_from_context(
        extracted.value(), 
        "continued_operation"
    );
}
```

### Using Macros for Convenience

```cpp
void process_request() {
    TRACE_SPAN("process_request");
    
    // Span automatically created and will be finished when function exits
    
    validate_input();
    
    {
        TRACE_CHILD_SPAN(*_scoped_span, "database_operation");
        // Child span for this block
        query_database();
    }
}
```

### Complete Example

See [distributed_tracing_example.cpp](distributed_tracing_example.cpp) for a complete working example.

---

## Health Monitoring

### Setting Up Health Checks

```cpp
#include <monitoring/health/health_monitor.h>

health_monitor monitor;

// Register a liveness check
monitor.register_check("database",
    health_check_builder()
        .with_name("database_check")
        .with_type(health_check_type::liveness)
        .with_check([]() {
            // Check database connection
            if (can_connect_to_database()) {
                return health_check_result::healthy("Database connected");
            }
            return health_check_result::unhealthy("Cannot connect to database");
        })
        .with_timeout(5s)
        .critical(true)
        .build()
);

// Register a readiness check
monitor.register_check("api",
    health_check_builder()
        .with_name("api_check")
        .with_type(health_check_type::readiness)
        .with_check([]() {
            // Check if API is ready
            if (api_initialized && !overloaded) {
                return health_check_result::healthy("API ready");
            }
            if (overloaded) {
                return health_check_result::degraded("High load");
            }
            return health_check_result::unhealthy("API not ready");
        })
        .build()
);
```

### Health Dependencies

```cpp
// Define dependencies between services
monitor.add_dependency("api", "database");
monitor.add_dependency("api", "cache");

// Dependencies are checked in order
auto results = monitor.check_all();
```

### Recovery Handlers

```cpp
// Register automatic recovery
monitor.register_recovery_handler("database",
    []() -> bool {
        // Attempt to reconnect
        return reconnect_to_database();
    }
);
```

### Health Endpoints

```cpp
// Create HTTP endpoint for health checks
void health_endpoint(const http_request& req, http_response& res) {
    auto health = monitor.get_overall_status();
    
    if (health == health_status::healthy) {
        res.status = 200;
        res.body = "OK";
    } else if (health == health_status::degraded) {
        res.status = 200;
        res.body = "DEGRADED";
    } else {
        res.status = 503;
        res.body = "UNHEALTHY";
    }
}
```

### Complete Example

See [health_reliability_example.cpp](health_reliability_example.cpp) for a complete working example.

---

## Reliability Features

### Circuit Breakers

Prevent cascading failures by stopping calls to failing services:

```cpp
#include <monitoring/reliability/circuit_breaker.h>

// Configure circuit breaker
circuit_breaker_config config;
config.failure_threshold = 5;        // Open after 5 failures
config.reset_timeout = 30s;          // Try again after 30 seconds
config.success_threshold = 2;        // Need 2 successes to close

circuit_breaker<std::string> breaker("external_api", config);

// Use circuit breaker
auto result = breaker.execute(
    []() { 
        // Call external service
        return call_external_api(); 
    },
    []() { 
        // Fallback when circuit is open
        return result<std::string>::success("cached_response"); 
    }
);
```

### Retry Policies

Automatically retry failed operations:

```cpp
#include <monitoring/reliability/retry_policy.h>

// Configure retry
retry_config config;
config.max_attempts = 3;
config.strategy = retry_strategy::exponential_backoff;
config.initial_delay = 100ms;
config.max_delay = 5s;

retry_policy<std::string> retry(config);

// Execute with retry
auto result = retry.execute([]() {
    return potentially_failing_operation();
});
```

### Error Boundaries

Isolate errors to prevent system-wide failures:

```cpp
#include <monitoring/reliability/error_boundary.h>

error_boundary boundary("critical_section");

// Set error handler
boundary.set_error_handler([](const error_info& error) {
    log_error("Error in critical section: {}", error.message);
    send_alert(error);
});

// Execute within boundary
auto result = boundary.execute<int>([]() {
    return risky_operation();
});
```

### Combining Reliability Features

```cpp
// Layered reliability: retry → circuit breaker → error boundary
auto reliable_operation = [&]() {
    return error_boundary.execute<std::string>([&]() {
        return circuit_breaker.execute([&]() {
            return retry_policy.execute([&]() {
                return external_service_call();
            });
        });
    });
};
```

---

## Error Handling with Result Pattern

### Basic Usage

The Result pattern provides explicit error handling without exceptions:

```cpp
#include <monitoring/core/result_types.h>

// Function that may fail
result<int> parse_config_value(const std::string& str) {
    try {
        int value = std::stoi(str);
        return result<int>::success(value);
    } catch (...) {
        return make_error<int>(
            monitoring_error_code::invalid_argument,
            "Cannot parse integer: " + str
        );
    }
}

// Using the result
auto result = parse_config_value("42");
if (result) {
    std::cout << "Value: " << result.value() << std::endl;
} else {
    std::cout << "Error: " << result.get_error().message << std::endl;
}
```

### Chaining Operations

```cpp
// Chain operations with and_then
auto process = parse_config_value("100")
    .and_then([](int value) {
        if (value < 0) {
            return make_error<int>(
                monitoring_error_code::out_of_range,
                "Value must be positive"
            );
        }
        return result<int>::success(value * 2);
    })
    .map([](int value) {
        return value + 10;
    });

// Error recovery with or_else
auto with_default = parse_config_value("invalid")
    .or_else([](const error_info&) {
        return result<int>::success(42); // Default value
    });
```

### Result in APIs

```cpp
class DatabaseClient {
public:
    result<User> get_user(int id) {
        if (!connected_) {
            return make_error<User>(
                monitoring_error_code::unavailable,
                "Database not connected"
            );
        }
        
        auto query_result = execute_query("SELECT * FROM users WHERE id = ?", id);
        if (!query_result) {
            return make_error<User>(
                query_result.get_error().code,
                "Query failed: " + query_result.get_error().message
            );
        }
        
        User user;
        // ... parse user from query result ...
        return result<User>::success(user);
    }
};
```

### Complete Example

See [result_pattern_example.cpp](result_pattern_example.cpp) for more examples.

---

## Best Practices

### 1. Resource Management

Always use RAII for automatic resource management:

```cpp
// Good: Automatic cleanup
{
    scoped_timer timer(&profiler, "operation");
    perform_operation();
} // Timer automatically records duration

// Good: Scoped span
{
    TRACE_SPAN("process_batch");
    process_batch();
} // Span automatically finished
```

### 2. Error Handling

Always check Results:

```cpp
// Good: Check result
auto result = operation();
if (!result) {
    log_error("Operation failed: {}", result.get_error().message);
    return result; // Propagate error
}
use_value(result.value());

// Bad: Ignore errors
operation(); // Result ignored!
```

### 3. Configuration

Validate configuration before use:

```cpp
monitoring_config config;
// ... set config values ...

auto validation = config.validate();
if (!validation) {
    log_error("Invalid config: {}", validation.get_error().message);
    return;
}
```

### 4. Performance

Start with conservative settings and tune based on measurements:

```cpp
// Start conservative
config.sampling_rate = 0.01;  // 1% sampling
config.collection_interval = 10s;

// Monitor overhead
auto overhead = monitor.get_overhead_percent();
if (overhead < 2.0) {
    // Can afford more detail
    config.sampling_rate = 0.1;  // 10% sampling
}
```

### 5. Testing

Test monitoring in your unit tests:

```cpp
TEST(MyService, MetricsRecorded) {
    MyService service;
    service.process_request();
    
    auto metrics = service.get_metrics();
    EXPECT_TRUE(metrics.has_value());
    EXPECT_GT(metrics.value().size(), 0);
}
```

### 6. Production Deployment

Use different configurations for different environments:

```cpp
monitoring_config get_config(Environment env) {
    switch (env) {
        case Environment::Development:
            return dev_config();      // Full detail, no sampling
        case Environment::Staging:
            return staging_config();   // Moderate detail
        case Environment::Production:
            return production_config(); // Optimized for low overhead
    }
}
```

### 7. Troubleshooting

Enable debug logging when investigating issues:

```cpp
#ifdef DEBUG
    monitoring.enable_debug_logging(true);
    monitoring.set_log_level(log_level::trace);
#endif
```

---

## Advanced Topics

### Custom Collectors

Create custom metric collectors:

```cpp
class CustomCollector : public metrics_collector {
public:
    std::string get_name() const override {
        return "custom_collector";
    }
    
    result<metrics_snapshot> collect() override {
        metrics_snapshot snapshot;
        
        // Collect custom metrics
        metric_data data;
        data.name = "custom_metric";
        data.unit = metric_unit::count;
        data.values.push_back({get_custom_value(), now()});
        
        snapshot.metrics["custom_metric"] = data;
        return result<metrics_snapshot>::success(snapshot);
    }
};

// Register custom collector
monitoring.add_collector(std::make_unique<CustomCollector>());
```

### Custom Exporters

Create custom exporters for your backend:

```cpp
class CustomExporter : public metrics_exporter {
public:
    result<bool> export_batch(const std::vector<metric_data>& metrics) override {
        // Send metrics to your backend
        for (const auto& metric : metrics) {
            send_to_backend(metric);
        }
        return result<bool>::success(true);
    }
};
```

### Integration with Existing Systems

#### Prometheus Integration

```cpp
#include <monitoring/export/metric_exporters.h>

prometheus_exporter exporter;
exporter.serve_metrics("/metrics", 9090);

// Metrics available at http://localhost:9090/metrics
```

#### OpenTelemetry Integration

```cpp
#include <monitoring/adapters/opentelemetry_adapter.h>

opentelemetry_adapter adapter;
adapter.export_traces(spans);
adapter.export_metrics(metrics);
```

---

## Troubleshooting

### High Memory Usage

1. Check queue sizes:
```cpp
auto stats = monitoring.get_queue_stats();
if (stats.queue_depth > 10000) {
    // Queue backing up - increase flush frequency
    config.flush_interval = 1s;
}
```

2. Enable memory limits:
```cpp
config.max_memory_mb = 50;
config.memory_warning_threshold = 0.8;
```

### Missing Metrics

1. Check if collectors are enabled:
```cpp
for (const auto& collector : monitoring.get_collectors()) {
    std::cout << collector->get_name() << ": " 
              << (collector->is_enabled() ? "enabled" : "disabled") 
              << std::endl;
}
```

2. Verify sampling rate:
```cpp
if (config.sampling_rate < 0.01) {
    // Very low sampling - might miss events
    config.sampling_rate = 0.1;
}
```

### Performance Issues

1. Use adaptive optimization:
```cpp
adaptive_optimizer optimizer;
optimizer.set_target_overhead(5.0); // Max 5% CPU
optimizer.enable_auto_tuning(true);
```

2. Profile the monitoring system:
```cpp
auto profile = monitoring.profile_overhead();
std::cout << "Monitoring overhead: " << profile.cpu_percent << "%" << std::endl;
```

For more troubleshooting tips, see the [Troubleshooting Guide](../docs/TROUBLESHOOTING.md).

---

## Further Resources

- [API Reference](../docs/API_REFERENCE.md) - Complete API documentation
- [Architecture Guide](../docs/ARCHITECTURE_GUIDE.md) - System design and architecture
- [Performance Tuning](../docs/PERFORMANCE_TUNING.md) - Optimization guide
- [Examples](.) - Working code examples

---

## Getting Help

- Check the [documentation](../docs/)
- Look at the [examples](.)
- Review the [tests](../tests/) for usage patterns
- Report issues on GitHub

---

## Conclusion

You now have the knowledge to:
- ✅ Set up basic monitoring
- ✅ Implement distributed tracing
- ✅ Configure health checks
- ✅ Use reliability features
- ✅ Handle errors properly
- ✅ Follow best practices

Start with the basic example and gradually add more features as needed. Remember to measure the monitoring overhead and adjust configuration accordingly.

Happy monitoring! 🎉