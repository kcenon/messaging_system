# Monitoring Integration Template

This directory provides a template for integrating external monitoring implementations with the thread_system.

## Integration Steps

1. **Implement the Interface**
   ```cpp
   #include <thread_system/interfaces/monitoring_interface.h>
   
   class MyMonitoring : public monitoring_interface::monitoring_interface {
   public:
       void update_system_metrics(const monitoring_interface::system_metrics& metrics) override {
           // Record system-level metrics
       }
       
       void update_thread_pool_metrics(const monitoring_interface::thread_pool_metrics& metrics) override {
           // Record thread pool metrics
       }
       
       void update_thread_pool_metrics(const std::string& pool_name,
                                      std::uint32_t pool_instance_id,
                                      const monitoring_interface::thread_pool_metrics& metrics) override {
           // Record metrics with pool identification
       }
       
       void update_worker_metrics(std::size_t worker_id, 
                                 const monitoring_interface::worker_metrics& metrics) override {
           // Record per-worker metrics
       }
       
       monitoring_interface::metrics_snapshot get_current_snapshot() const override {
           // Return current metrics snapshot
       }
       
       std::vector<monitoring_interface::metrics_snapshot> 
       get_recent_snapshots(std::size_t count) const override {
           // Return recent snapshots
       }
       
       bool is_active() const override {
           return true; // Return monitoring status
       }
   };
   ```

2. **Register with Thread System**
   ```cpp
   // Create monitoring instance
   auto monitoring = std::make_shared<MyMonitoring>();
   
   // Pass via thread_context
   thread_module::thread_context context;
   context.set_monitoring(monitoring);
   
   // Create thread pool with monitoring
   auto pool = std::make_shared<thread_pool_module::thread_pool>("MyPool", context);
   ```

3. **CMake Integration**
   ```cmake
   find_package(ThreadSystemCore REQUIRED)
   
   add_library(my_monitoring_integration
       src/my_monitoring.cpp
   )
   
   target_link_libraries(my_monitoring_integration
       PUBLIC
           ThreadSystem::Core
           MyMonitoringLibrary
   )
   ```

## Example: Prometheus Integration

```cpp
#include <thread_system/interfaces/monitoring_interface.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>

class PrometheusMonitoring : public monitoring_interface::monitoring_interface {
private:
    std::shared_ptr<prometheus::Registry> registry_;
    
    prometheus::Family<prometheus::Gauge>& cpu_usage_;
    prometheus::Family<prometheus::Gauge>& memory_usage_;
    prometheus::Family<prometheus::Counter>& jobs_completed_;
    prometheus::Family<prometheus::Gauge>& active_threads_;
    prometheus::Family<prometheus::Histogram>& job_latency_;
    
    mutable std::mutex snapshot_mutex_;
    monitoring_interface::metrics_snapshot current_snapshot_;
    
public:
    explicit PrometheusMonitoring(std::shared_ptr<prometheus::Registry> registry)
        : registry_(registry),
          cpu_usage_(prometheus::BuildGauge()
              .Name("thread_system_cpu_usage_percent")
              .Help("CPU usage percentage")
              .Register(*registry_)),
          memory_usage_(prometheus::BuildGauge()
              .Name("thread_system_memory_usage_bytes")
              .Help("Memory usage in bytes")
              .Register(*registry_)),
          jobs_completed_(prometheus::BuildCounter()
              .Name("thread_system_jobs_completed_total")
              .Help("Total number of completed jobs")
              .Register(*registry_)),
          active_threads_(prometheus::BuildGauge()
              .Name("thread_system_active_threads")
              .Help("Number of active threads")
              .Register(*registry_)),
          job_latency_(prometheus::BuildHistogram()
              .Name("thread_system_job_latency_seconds")
              .Help("Job execution latency")
              .Register(*registry_)) {}
    
    void update_system_metrics(const monitoring_interface::system_metrics& metrics) override {
        cpu_usage_.Add({}).Set(metrics.cpu_usage_percent);
        memory_usage_.Add({}).Set(metrics.memory_usage_bytes);
        active_threads_.Add({}).Set(metrics.active_threads);
        
        std::lock_guard<std::mutex> lock(snapshot_mutex_);
        current_snapshot_.system = metrics;
    }
    
    void update_thread_pool_metrics(const std::string& pool_name,
                                   std::uint32_t pool_instance_id,
                                   const monitoring_interface::thread_pool_metrics& metrics) override {
        const std::map<std::string, std::string> labels = {
            {"pool_name", pool_name},
            {"instance_id", std::to_string(pool_instance_id)}
        };
        
        jobs_completed_.Add(labels).Increment(metrics.jobs_completed);
        
        if (metrics.average_latency_ns > 0) {
            job_latency_.Add(labels).Observe(metrics.average_latency_ns / 1e9);
        }
        
        std::lock_guard<std::mutex> lock(snapshot_mutex_);
        current_snapshot_.thread_pool = metrics;
    }
    
    // ... other methods implementation
};
```

## Metric Guidelines

### System Metrics
- **CPU Usage**: Percentage (0-100)
- **Memory Usage**: Bytes
- **Active Threads**: Count
- **Total Allocations**: Count

### Thread Pool Metrics
- **Jobs Completed**: Counter (monotonic)
- **Jobs Pending**: Gauge
- **Execution Time**: Nanoseconds
- **Worker Threads**: Count
- **Idle Threads**: Count

### Worker Metrics
- **Jobs Processed**: Counter per worker
- **Processing Time**: Total nanoseconds
- **Idle Time**: Total nanoseconds
- **Context Switches**: Count

## Best Practices

1. **Low Overhead**: Minimize metric collection overhead
2. **Thread Safety**: Use appropriate synchronization
3. **Buffering**: Batch metrics updates when possible
4. **Sampling**: Consider sampling for high-frequency metrics
5. **Labels**: Use consistent label naming conventions
6. **Time Series**: Maintain proper time series data
7. **Aggregation**: Pre-aggregate where appropriate