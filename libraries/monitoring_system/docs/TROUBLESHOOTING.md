# Troubleshooting Guide

## Overview

This guide helps diagnose and resolve common issues with the Monitoring System. Each section includes symptoms, diagnostic steps, and solutions.

## Quick Diagnostic Commands

```bash
# Check system status
./monitoring_cli status

# View recent errors
./monitoring_cli errors --last 100

# Check health
./monitoring_cli health --verbose

# View configuration
./monitoring_cli config --show

# Test connectivity
./monitoring_cli test --component all
```

## Common Issues

### 1. Monitoring System Won't Start

#### Symptoms
- Application crashes on startup
- Error messages about initialization failure
- Process exits immediately

#### Diagnostic Steps
```cpp
// Enable debug logging
monitoring_config config;
config.log_level = log_level::debug;
config.enable_startup_diagnostics = true;

// Check initialization result
auto result = monitoring_system.initialize();
if (!result) {
    std::cerr << "Init failed: " << result.get_error().message << "\n";
}
```

#### Solutions

**Missing Dependencies:**
```bash
# Check library dependencies
ldd monitoring_system
otool -L monitoring_system  # macOS

# Install missing libraries
apt-get install libstdc++6  # Linux
brew install gcc            # macOS
```

**Permission Issues:**
```bash
# Check file permissions
ls -la /var/log/monitoring/
ls -la /var/lib/monitoring/

# Fix permissions
sudo chown -R $USER:$USER /var/log/monitoring/
sudo chmod 755 /var/log/monitoring/
```

**Port Already in Use:**
```cpp
// Change port
exporter_config config;
config.port = 9091;  // Use different port
```

### 2. High Memory Usage

#### Symptoms
- Continuously growing memory consumption
- Out of memory errors
- System becomes unresponsive

#### Diagnostic Steps
```cpp
// Enable memory profiling
memory_profiler profiler;
profiler.start();

// Get memory stats
auto stats = monitoring_system.get_memory_stats();
std::cout << "Total allocated: " << stats.total_allocated_mb << " MB\n";
std::cout << "Active objects: " << stats.active_objects << "\n";
std::cout << "Largest allocation: " << stats.largest_allocation_mb << " MB\n";

// Check for leaks
auto leaks = profiler.detect_leaks();
for (const auto& leak : leaks) {
    std::cout << "Potential leak: " << leak.location << " (" 
              << leak.size_bytes << " bytes)\n";
}
```

#### Solutions

**Memory Leaks:**
```cpp
// Fix circular references
std::weak_ptr<Component> weak_ref = component;  // Use weak_ptr

// Ensure proper cleanup
class Resource {
    ~Resource() {
        // Always clean up
        if (buffer_) {
            delete[] buffer_;
            buffer_ = nullptr;
        }
    }
};

// Use RAII
auto resource = std::make_unique<Resource>();  // Automatic cleanup
```

**Unbounded Queues:**
```cpp
// Set queue limits
queue_config config;
config.max_size = 10000;
config.overflow_policy = overflow_policy::drop_oldest;

// Monitor queue size
if (queue.size() > config.max_size * 0.9) {
    log_warning("Queue near capacity: {}", queue.size());
}
```

**Retention Issues:**
```cpp
// Configure retention
retention_config config;
config.max_age = 24h;
config.max_size_mb = 1000;
config.cleanup_interval = 1h;
```

### 3. High CPU Usage

#### Symptoms
- CPU usage consistently > 10%
- Application becomes sluggish
- System load increases

#### Diagnostic Steps
```cpp
// Profile CPU usage
cpu_profiler profiler;
profiler.start();

// Run for 60 seconds
std::this_thread::sleep_for(60s);

// Get hotspots
auto hotspots = profiler.get_hotspots();
for (const auto& hotspot : hotspots) {
    std::cout << hotspot.function << ": " 
              << hotspot.cpu_percent << "%\n";
}
```

#### Solutions

**Excessive Sampling:**
```cpp
// Reduce sampling rate
config.sampling_rate = 0.01;  // 1% instead of 100%

// Use adaptive sampling
adaptive_sampler sampler;
sampler.set_max_throughput(1000);  // Limit to 1000/sec
```

**Lock Contention:**
```cpp
// Use lock-free structures
lock_free_queue<Data> queue;  // Instead of std::queue with mutex

// Reduce lock scope
{
    std::lock_guard<std::mutex> lock(mutex_);
    // Minimal work under lock
    data_ = new_data;
}  // Lock released immediately

// Use read-write locks
std::shared_mutex rw_mutex;
// Multiple readers
std::shared_lock<std::shared_mutex> read_lock(rw_mutex);
// Single writer
std::unique_lock<std::shared_mutex> write_lock(rw_mutex);
```

**Inefficient Algorithms:**
```cpp
// Use efficient data structures
std::unordered_map<Key, Value> map;  // O(1) instead of O(log n)

// Batch operations
std::vector<Data> batch;
batch.reserve(1000);
// Collect data...
process_batch(batch);  // Process all at once
```

### 4. Missing Metrics/Traces

#### Symptoms
- No data in dashboards
- Incomplete traces
- Missing health checks

#### Diagnostic Steps
```cpp
// Check if collectors are enabled
for (const auto& collector : monitoring_system.get_collectors()) {
    std::cout << collector->get_name() << ": " 
              << (collector->is_enabled() ? "enabled" : "disabled") << "\n";
}

// Verify data flow
auto stats = monitoring_system.get_pipeline_stats();
std::cout << "Collected: " << stats.metrics_collected << "\n";
std::cout << "Exported: " << stats.metrics_exported << "\n";
std::cout << "Dropped: " << stats.metrics_dropped << "\n";
```

#### Solutions

**Sampling Issues:**
```cpp
// Check sampling configuration
if (config.sampling_rate < 0.01) {
    log_warning("Very low sampling rate: {}", config.sampling_rate);
}

// Temporarily disable sampling for debugging
config.sampling_rate = 1.0;  // 100% sampling
```

**Export Failures:**
```cpp
// Check exporter status
auto status = exporter.get_status();
if (status.last_error) {
    std::cout << "Export error: " << status.last_error.message << "\n";
}

// Add retry logic
retry_policy<bool> retry;
retry.with_max_attempts(3)
     .with_backoff(exponential_backoff{100ms, 2.0});

auto result = retry.execute([&]() {
    return exporter.export_batch(data);
});
```

**Context Propagation:**
```cpp
// Ensure context is propagated
auto span = tracer.start_span("operation");
thread_context::set_current_span(span);

// In child thread
auto parent_span = thread_context::get_current_span();
auto child_span = tracer.start_child_span(parent_span, "child_op");
```

### 5. Storage Issues

#### Symptoms
- Failed writes to storage
- Slow query performance
- Data corruption

#### Diagnostic Steps
```cpp
// Test storage backend
storage_diagnostics diag(storage_backend);
auto results = diag.run_tests();

for (const auto& test : results) {
    std::cout << test.name << ": " 
              << (test.passed ? "PASS" : "FAIL") << "\n";
    if (!test.passed) {
        std::cout << "  Error: " << test.error_message << "\n";
    }
}

// Check storage stats
auto stats = storage_backend.get_stats();
std::cout << "Write latency: " << stats.avg_write_latency_ms << " ms\n";
std::cout << "Read latency: " << stats.avg_read_latency_ms << " ms\n";
std::cout << "Failed writes: " << stats.failed_writes << "\n";
```

#### Solutions

**Connection Issues:**
```cpp
// Add connection pooling
connection_pool pool;
pool.set_min_connections(5);
pool.set_max_connections(20);
pool.set_validation_query("SELECT 1");

// Add reconnection logic
storage_backend.set_reconnect_policy(
    reconnect_policy{
        .max_attempts = 10,
        .initial_delay = 1s,
        .max_delay = 30s
    }
);
```

**Performance Issues:**
```cpp
// Enable batching
storage_config config;
config.batch_size = 1000;
config.flush_interval = 5s;

// Add caching
cache_config cache_cfg;
cache_cfg.max_size_mb = 100;
cache_cfg.ttl = 60s;
storage_backend.enable_cache(cache_cfg);

// Use appropriate indexes
// For SQL backends
storage_backend.execute(
    "CREATE INDEX idx_timestamp ON metrics(timestamp)"
);
```

### 6. Network Issues

#### Symptoms
- Connection timeouts
- Failed exports
- High latency

#### Diagnostic Steps
```bash
# Test connectivity
ping monitoring-backend.example.com
telnet monitoring-backend.example.com 4317
curl -v https://monitoring-backend.example.com/health

# Check DNS
nslookup monitoring-backend.example.com
dig monitoring-backend.example.com

# Trace route
traceroute monitoring-backend.example.com
```

#### Solutions

**Timeout Issues:**
```cpp
// Increase timeouts
network_config config;
config.connect_timeout = 10s;
config.read_timeout = 30s;
config.write_timeout = 30s;

// Add keep-alive
config.keep_alive = true;
config.keep_alive_interval = 30s;
```

**SSL/TLS Issues:**
```cpp
// Configure TLS
tls_config config;
config.verify_peer = true;
config.ca_cert_path = "/path/to/ca.crt";
config.client_cert_path = "/path/to/client.crt";
config.client_key_path = "/path/to/client.key";

// Disable verification for testing (NOT for production!)
config.verify_peer = false;
```

### 7. Circuit Breaker Issues

#### Symptoms
- Circuit breaker always open
- Circuit breaker never opens
- Unexpected fallback behavior

#### Diagnostic Steps
```cpp
// Check circuit breaker state
auto state = circuit_breaker.get_state();
auto metrics = circuit_breaker.get_metrics();

std::cout << "State: " << to_string(state) << "\n";
std::cout << "Failed calls: " << metrics.failed_calls << "\n";
std::cout << "Success calls: " << metrics.successful_calls << "\n";
std::cout << "Rejected calls: " << metrics.rejected_calls << "\n";
```

#### Solutions

**Always Open:**
```cpp
// Adjust thresholds
circuit_breaker_config config;
config.failure_threshold = 10;  // Increase threshold
config.failure_ratio = 0.5;     // 50% failure rate
config.reset_timeout = 30s;     // Shorter reset time

// Manual reset
circuit_breaker.reset();
```

**Never Opens:**
```cpp
// More sensitive configuration
config.failure_threshold = 3;   // Lower threshold
config.failure_ratio = 0.3;     // 30% failure rate
config.timeout = 1s;            // Shorter timeout
```

## Debugging Tools

### Enable Debug Logging

```cpp
// Global debug mode
debug_config config;
config.enable_all = true;
config.log_level = log_level::trace;
config.include_timestamps = true;
config.include_thread_id = true;

monitoring_system.enable_debug(config);
```

### Memory Debugging

```cpp
// Enable memory tracking
#ifdef DEBUG
class MemoryTracker {
    static std::unordered_map<void*, size_t> allocations_;
    
public:
    static void* allocate(size_t size) {
        void* ptr = malloc(size);
        allocations_[ptr] = size;
        return ptr;
    }
    
    static void deallocate(void* ptr) {
        allocations_.erase(ptr);
        free(ptr);
    }
    
    static void report_leaks() {
        for (const auto& [ptr, size] : allocations_) {
            std::cout << "Leak: " << ptr << " (" << size << " bytes)\n";
        }
    }
};
#endif
```

### Performance Debugging

```cpp
// Timing macros
#define TIME_BLOCK(name) \
    auto _timer_##name = monitoring_system::scoped_timer(#name);

// Usage
void process_request() {
    TIME_BLOCK(process_request);
    
    {
        TIME_BLOCK(validation);
        validate_input();
    }
    
    {
        TIME_BLOCK(processing);
        do_work();
    }
}
```

## Health Check Verification

```cpp
// Comprehensive health check
class SystemHealthCheck {
public:
    struct HealthReport {
        bool is_healthy;
        std::vector<std::string> issues;
        std::map<std::string, double> metrics;
    };
    
    HealthReport check_all() {
        HealthReport report;
        report.is_healthy = true;
        
        // Check CPU
        auto cpu = get_cpu_usage();
        report.metrics["cpu_percent"] = cpu;
        if (cpu > 80.0) {
            report.issues.push_back("High CPU usage: " + std::to_string(cpu));
            report.is_healthy = false;
        }
        
        // Check memory
        auto mem = get_memory_usage_mb();
        report.metrics["memory_mb"] = mem;
        if (mem > 1000.0) {
            report.issues.push_back("High memory usage: " + std::to_string(mem));
            report.is_healthy = false;
        }
        
        // Check queues
        auto queue_depth = get_queue_depth();
        report.metrics["queue_depth"] = queue_depth;
        if (queue_depth > 5000) {
            report.issues.push_back("Queue backlog: " + std::to_string(queue_depth));
            report.is_healthy = false;
        }
        
        return report;
    }
};
```

## Emergency Procedures

### System Overload

```cpp
// Emergency throttling
void emergency_throttle() {
    // Reduce sampling to minimum
    config.sampling_rate = 0.001;  // 0.1%
    
    // Increase intervals
    config.collection_interval = 60s;
    
    // Disable non-critical features
    config.enable_tracing = false;
    config.enable_profiling = false;
    
    // Clear queues
    metric_queue.clear();
    trace_queue.clear();
    
    log_error("Emergency throttling activated");
}
```

### Data Recovery

```cpp
// Recover from corrupted storage
void recover_storage() {
    // Backup existing data
    storage_backend.backup("/tmp/backup");
    
    // Attempt repair
    storage_backend.repair();
    
    // Validate data
    auto validation = storage_backend.validate();
    if (!validation.is_valid) {
        // Restore from backup
        storage_backend.restore("/tmp/backup");
    }
}
```

## Monitoring System Logs

### Log Locations

```bash
# Default log locations
/var/log/monitoring/system.log     # System logs
/var/log/monitoring/error.log      # Error logs
/var/log/monitoring/metrics.log    # Metric logs
/var/log/monitoring/trace.log      # Trace logs
```

### Log Analysis

```bash
# Find errors
grep ERROR /var/log/monitoring/system.log

# Find warnings
grep WARN /var/log/monitoring/system.log

# Find specific component
grep "storage_backend" /var/log/monitoring/system.log

# Tail logs
tail -f /var/log/monitoring/system.log

# Analyze log patterns
awk '/ERROR/ {print $4}' system.log | sort | uniq -c | sort -rn
```

## Getting Help

### Diagnostic Information to Collect

When reporting issues, include:

1. **System Information:**
```bash
uname -a
cat /etc/os-release
g++ --version
```

2. **Configuration:**
```cpp
monitoring_system.dump_config("/tmp/config.json");
```

3. **Logs:**
```bash
tar -czf logs.tar.gz /var/log/monitoring/
```

4. **Stack Trace:**
```bash
gdb ./monitoring_app core
(gdb) bt full
```

5. **Performance Metrics:**
```cpp
auto report = monitoring_system.generate_diagnostic_report();
report.save("/tmp/diagnostic_report.json");
```

### Support Channels

- GitHub Issues: [Report bugs and request features]
- Documentation: [API Reference](API_REFERENCE.md)
- Examples: [Code examples](../examples/)

## Conclusion

Most issues can be resolved by:
1. Checking logs for error messages
2. Verifying configuration
3. Ensuring sufficient resources
4. Testing connectivity
5. Adjusting thresholds and limits

For persistent issues, collect diagnostic information and consult the support channels.