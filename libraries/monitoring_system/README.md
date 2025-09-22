# Monitoring System

[![CI](https://github.com/kcenon/monitoring_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/monitoring_system/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

A real-time performance monitoring system for C++20 applications with low-overhead metrics collection. Part of the integrated threading ecosystem.

## 🔗 Project Ecosystem Integration

This monitoring system is a key component of a comprehensive threading and observability ecosystem:

### Project Dependencies
- **[thread_system](https://github.com/kcenon/thread_system)**: Core dependency providing monitoring interfaces
  - Implements: `monitoring_interface::monitoring_interface`
  - Provides: Interface contracts for metrics collection
  - Role: Foundation interfaces for monitoring subsystem

### Related Projects
- **[logger_system](https://github.com/kcenon/logger_system)**: Complementary logging functionality
  - Relationship: Both integrate with thread_system
  - Synergy: Combined monitoring and logging for complete observability
  - Integration: Can monitor logging performance and log monitoring events

- **[integrated_thread_system](https://github.com/kcenon/integrated_thread_system)**: Complete integration examples
  - Usage: Demonstrates monitoring_system integration patterns
  - Benefits: Production-ready examples with full ecosystem
  - Reference: Complete application templates with monitoring

### Integration Architecture
```
┌─────────────────┐
│  thread_system  │ ← Core interfaces (monitoring_interface)
└─────────┬───────┘
          │ implements
┌─────────▼───────┐     ┌─────────────────┐
│monitoring_system│ ◄──► │  logger_system  │
└─────────────────┘     └─────────────────┘
          │                       │
          └───────┬───────────────┘
                  ▼
    ┌─────────────────────────┐
    │integrated_thread_system │
    └─────────────────────────┘
```

### Integration Benefits
- **Thread pool metrics**: Real-time tracking of worker threads, queue depths, and performance
- **System monitoring**: CPU, memory, and resource utilization tracking
- **Lock-free collection**: High-performance metrics without impacting application performance
- **Historical analysis**: Ring buffer storage for trend analysis and performance profiling
- **Custom collectors**: Extensible architecture for domain-specific metrics

> 📖 **[Complete Architecture Guide](../ARCHITECTURE.md)**: Comprehensive documentation of the entire ecosystem architecture, dependency relationships, and integration patterns.

## Features

- **Low Overhead**: Minimal impact on application performance
- **Real-time Metrics**: System, thread pool, and worker metrics
- **Ring Buffer Storage**: Efficient circular buffer for historical data
- **Thread-safe**: Lock-free operations where possible
- **Extensible**: Easy to add custom metrics
- **Integration Ready**: Works seamlessly with Thread System
- **Multi-Process Support**: Monitor multiple processes and thread pools independently
- **Process-based Monitoring**: Track metrics per process with unique identification
- **Thread Pool Distinction**: Monitor multiple thread pools within each process

## Integration with Thread System

This monitoring system implements the monitoring interface from [Thread System](https://github.com/kcenon/thread_system):

```cpp
#include <monitoring_system/monitoring.h>
#include <thread_system/interfaces/service_container.h>

// Create and configure monitoring
auto monitor = std::make_shared<monitoring_module::monitoring>();
monitor->start();

// Register in service container
thread_module::service_container::global()
    .register_singleton<monitoring_interface::monitoring_interface>(monitor);

// Thread system components will now report metrics automatically
```

## Quick Start

### Basic Usage

```cpp
#include <monitoring_system/monitoring.h>

int main() {
    // Create monitoring instance
    auto monitor = std::make_shared<monitoring_module::monitoring>();
    
    // Start monitoring
    monitor->start();
    
    // Update metrics
    monitoring_interface::system_metrics sys_metrics;
    sys_metrics.cpu_usage_percent = 45;
    sys_metrics.memory_usage_bytes = 1024 * 1024 * 512; // 512MB
    sys_metrics.active_threads = 8;
    monitor->update_system_metrics(sys_metrics);
    
    // Get current snapshot
    auto snapshot = monitor->get_current_snapshot();
    
    // Get historical data
    auto history = monitor->get_recent_snapshots(10);
    
    return 0;
}
```

### Multi-Process Monitoring

```cpp
#include <monitoring_system/multi_process_monitoring.h>

// Create multi-process monitoring instance
auto multi_monitor = std::make_shared<monitoring_module::multi_process_monitoring>();
multi_monitor->start();

// Register processes
monitoring_interface::process_identifier process1{
    .pid = getpid(),
    .process_name = "main_process",
    .start_time = std::chrono::steady_clock::now()
};
multi_monitor->register_process(process1);

// Register thread pools
monitoring_interface::thread_pool_identifier pool1{
    .process_id = process1,
    .pool_name = "worker_pool",
    .instance_id = 1
};
multi_monitor->register_thread_pool(pool1);

// Update process-specific metrics
multi_monitor->update_process_system_metrics(process1, sys_metrics);

// Get multi-process snapshot
auto multi_snapshot = multi_monitor->get_multi_process_snapshot();
```

### Thread Pool Analysis

```cpp
#include <monitoring_system/thread_pool_analyzer.h>

// Analyze thread pool performance
auto pool_metrics = multi_monitor->get_thread_pool_metrics(pool1);
auto summary = thread_pool_analyzer::analyze_pool(pool_metrics);

// Detect bottlenecks
auto bottleneck = thread_pool_analyzer::detect_bottleneck(pool_metrics);
if (bottleneck) {
    std::cout << "Bottleneck detected: " << *bottleneck << std::endl;
}

// Compare two thread pools
auto comparison = thread_pool_analyzer::compare_pools(pool1_metrics, pool2_metrics);
std::cout << "Performance winner: " << comparison.performance_winner << std::endl;

// Get optimization suggestions
auto suggestions = thread_pool_analyzer::suggest_optimizations(pool_metrics);
for (const auto& suggestion : suggestions) {
    std::cout << "Suggestion: " << suggestion << std::endl;
}
```

### Advanced Analysis Dashboard

```cpp
#include <monitoring_system/analysis_dashboard.h>

// Create dashboard with configuration
analysis_dashboard::dashboard_config config;
config.trend_window_size = 60;
config.anomaly_threshold = 3.0;
config.enable_alerts = true;

auto dashboard = std::make_unique<analysis_dashboard>(monitor, config);

// Generate comprehensive health report
auto health_report = dashboard->generate_health_report();
std::cout << "System Health: " << health_report.health_status 
          << " (" << health_report.overall_health_score << "%)" << std::endl;

// Get performance predictions
auto forecast = dashboard->generate_forecast(std::chrono::seconds(300));
for (const auto& [metric, prediction] : forecast.cpu_predictions) {
    std::cout << metric << " predicted: " << prediction.predicted_value << std::endl;
}

// Detect anomalies
auto anomalies = dashboard->detect_real_time_anomalies();
for (const auto& anomaly : anomalies) {
    std::cout << "Anomaly: " << anomaly.description << std::endl;
}

// Export dashboard data
std::string json_data = dashboard->export_json();
```

### Performance Optimization

```cpp
#include <monitoring_system/performance_optimizer.h>

// Configure optimization
performance_optimizer::optimization_config opt_config;
opt_config.enable_compression = true;
opt_config.enable_batching = true;
opt_config.enable_tiered_storage = true;
opt_config.enable_adaptive_sampling = true;
opt_config.batch_size = 100;

auto optimizer = std::make_unique<performance_optimizer>(opt_config);

// Process metrics with optimization
optimizer->optimize_metric(snapshot);

// Auto-scaling based on metrics
auto_scaler::scaling_policy policy;
policy.cpu_threshold_up = 80.0;
policy.cpu_threshold_down = 30.0;
policy.cooldown = std::chrono::seconds(60);

auto scaler = std::make_unique<auto_scaler>(policy);
auto decision = scaler->decide(current_metrics);

if (decision.recommended_action == auto_scaler::scaling_decision::action::scale_up) {
    std::cout << "Scale up recommended: " << decision.reason << std::endl;
}

// Distributed aggregation for multi-node systems
distributed_aggregator::aggregation_config agg_config;
agg_config.enable_parallel_aggregation = true;
agg_config.worker_threads = 4;

auto aggregator = std::make_unique<distributed_aggregator>(agg_config);
aggregator->add_local_metrics("node1", node1_snapshot);
aggregator->add_local_metrics("node2", node2_snapshot);

auto global_metrics = aggregator->aggregate_global();
```

### Custom Metrics Collection

```cpp
class custom_collector : public monitoring_module::metrics_collector {
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        // Collect custom metrics
        snapshot.system.cpu_usage_percent = get_cpu_usage();
        snapshot.system.memory_usage_bytes = get_memory_usage();
    }
};

// Register custom collector
monitor->add_collector(std::make_unique<custom_collector>());
```

## Metrics Types

### System Metrics
- CPU usage percentage
- Memory usage in bytes
- Active thread count
- Total memory allocations

### Thread Pool Metrics
- Jobs completed
- Jobs pending
- Total execution time
- Average latency
- Worker thread count
- Idle thread count

### Worker Metrics
- Jobs processed per worker
- Processing time per worker
- Idle time
- Context switches

### Multi-Process Metrics
- Process identification (PID, name, start time)
- Per-process system metrics
- Per-process thread pool metrics
- Cross-process performance comparison
- Process-specific alert thresholds

### Thread Pool Analysis
- Pool performance scoring and health checks
- Worker load distribution analysis
- Bottleneck detection and optimization suggestions
- Pool type classification (CPU-bound, IO-bound, Balanced, Idle)
- Comparative analysis between pools
- Load balance scoring

### Advanced Analytics
- **Trend Analysis**: Linear regression, moving averages, seasonality detection
- **Anomaly Detection**: Z-score based outlier detection with severity levels
- **Predictive Analytics**: Linear predictions with confidence intervals
- **Correlation Analysis**: Cross-metric correlation discovery
- **Capacity Planning**: Resource exhaustion predictions
- **Change Point Detection**: Identify significant metric changes
- **Alert Management**: Configurable alerts with cooldown periods
- **Health Scoring**: Comprehensive system and component health assessment

### Performance Optimization
- **Tiered Storage**: Hot/warm/cold data tiers with automatic aging
- **Lock-free Queues**: High-performance lock-free data structures
- **Compressed Storage**: Memory-efficient metric compression
- **Batch Processing**: Efficient batch metric processing
- **Adaptive Sampling**: Dynamic sampling rate based on system load
- **Auto-scaling**: Automatic resource scaling based on metrics
- **Distributed Aggregation**: Parallel metrics aggregation for multi-node systems
- **Memory Optimization**: Tiered storage with compression for long-term retention

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Build Options

- `BUILD_TESTS`: Build unit tests (default: ON)
- `BUILD_BENCHMARKS`: Build performance benchmarks (default: OFF)
- `BUILD_SAMPLES`: Build example programs (default: ON)

## Installation

```bash
cmake --build . --target install
```

## CMake Integration

```cmake
find_package(MonitoringSystem REQUIRED)
target_link_libraries(your_target PRIVATE MonitoringSystem::monitoring)
```

## Performance

The monitoring system is designed for minimal overhead:
- Lock-free ring buffer for metrics storage
- Atomic operations for counter updates
- Configurable collection intervals
- Zero-allocation steady state operation

## License

BSD 3-Clause License - see LICENSE file for details.