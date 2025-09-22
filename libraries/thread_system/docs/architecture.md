# Thread System Architecture

## Table of Contents
1. [Overview](#overview)
2. [Core Architecture](#core-architecture)
3. [Component Design](#component-design)
4. [Implementation Details](#implementation-details)
5. [Performance Characteristics](#performance-characteristics)
6. [Design Patterns](#design-patterns)
7. [Memory Management](#memory-management)
8. [Platform Support](#platform-support)
9. [Future Roadmap](#future-roadmap)

## Overview

The Thread System is a modern C++20 multi-threading framework designed for high-performance concurrent programming. It provides a modular, interface-based architecture with clear separation of concerns, enabling both ease of use and advanced customization.

### Architecture Principles

1. **Modular Design**: Core threading functionality separated from auxiliary features
2. **Interface-Based Architecture**: Clean contracts enable external module integration
3. **Layered Design**: Clear separation between core threading primitives and higher-level abstractions
4. **Adaptive Performance**: Runtime optimization based on workload characteristics
5. **Type Safety**: Strong typing with compile-time guarantees
6. **Zero-Overhead Abstractions**: Modern C++ techniques minimize runtime costs
7. **Platform Agnostic**: Consistent API across Windows, Linux, and macOS

## Core Architecture

### System Layers

```
┌─────────────────────────────────────────────────────┐
│                   Applications                      │
├─────────────────────────────────────────────────────┤
│              External Modules                       │
│         (Logger, Monitoring, etc.)                  │
├─────────────────────────────────────────────────────┤
│          High-Level APIs (Thread Pools)             │
├─────────────────────────────────────────────────────┤
│         Core Threading Components                   │
├─────────────────────────────────────────────────────┤
│          Platform Abstraction Layer                 │
├─────────────────────────────────────────────────────┤
│            Operating System APIs                    │
└─────────────────────────────────────────────────────┘
```

### Component Hierarchy

```
[Core Thread System - ~2,700 lines]

utilities
    ├── conversion      # String conversions
    ├── core           # Formatters, spans
    ├── io             # File operations
    ├── parsing        # Argument parsing
    └── time           # Date/time utilities

thread_base
    ├── core           # Base thread class
    ├── jobs           # Job abstraction
    ├── sync           # Synchronization primitives
    └── lockfree       # Lock-free data structures
        ├── memory     # Hazard pointers, node pools
        └── queues     # MPMC queues, adaptive queues

thread_pool
    ├── core           # Thread pool implementation
    ├── workers        # Worker thread management
    └── async          # Future extensions

typed_thread_pool
    ├── core           # Type interfaces
    ├── jobs           # Typed job implementations
    ├── pool           # Typed pool implementation
    └── scheduling     # Priority scheduling

[External Modules - Separate Projects]

logger_module           # https://github.com/kcenon/logger_module
    ├── core           # Logger implementation
    ├── jobs           # Log job processing
    ├── types          # Log types and formatting
    └── writers        # Output destinations

monitoring_module      # https://github.com/kcenon/monitoring_module
    ├── core           # Metrics collection
    ├── collectors     # Metric collectors
    └── storage        # Time-series storage
```

## Component Design

### Modular Architecture Benefits

The modularized architecture provides significant advantages:

- **Reduced Complexity**: Core thread system focused on threading (~2,700 lines vs ~11,400 lines)
- **Clean Interfaces**: Well-defined contracts between modules
- **Independent Development**: External modules can evolve separately
- **Selective Usage**: Include only the modules you need
- **Easier Testing**: Smaller, focused modules are easier to test
- **Better Maintainability**: Single responsibility per module

### Thread Base Module

The foundation of the system, providing:

- **thread_base**: Abstract base class for all worker threads
- **job**: Interface for work units
- **job_queue**: Thread-safe FIFO queue
- **Adaptive components**: Dynamic optimization based on contention

```cpp
class thread_base {
protected:
    virtual auto before_start() -> result_void { return {}; }
    virtual auto do_work() -> result_void = 0;
    virtual auto after_stop() -> result_void { return {}; }
};
```

### Thread Pool Module

Standard thread pool implementation with:

- **Dynamic worker management**: Add/remove workers at runtime
- **Adaptive queue optimization**: Automatic strategy selection
- **Batch processing support**: Process multiple jobs efficiently

### Typed Thread Pool Module

Advanced type-based scheduling:

- **Priority levels**: RealTime > Batch > Background
- **Per-type queues**: Separate queues for each job type
- **Worker specialization**: Workers can handle specific types

### Interface-Based External Module Integration

The thread system provides clean interfaces for external modules:

```cpp
// Example: Logger module integration
class logger_job : public job {
    // Logger-specific job implementation
};

// Example: Monitoring module integration
class metric_collector_job : public job {
    // Metric collection job implementation
};
```

External modules can:
- Implement the `job` interface for task execution
- Use thread pools for concurrent processing
- Leverage lock-free structures for performance
- Integrate without modifying core thread system

### Adaptive Queue System

The adaptive queue automatically switches between strategies:

```cpp
enum class queue_strategy {
    AUTO_DETECT,    // Automatic selection
    FORCE_LEGACY,   // Always use mutex
    FORCE_LOCKFREE, // Always use lock-free
    ADAPTIVE        // Dynamic switching
};
```

Performance characteristics:
- **Low contention**: Mutex-based (lower overhead)
- **High contention**: Lock-free MPMC (better scalability)
- **Automatic switching**: Based on runtime metrics

## Implementation Details

### Lock-Free MPMC Queue

Uses Michael & Scott algorithm with enhancements:

```cpp
template<typename T>
class lockfree_mpmc_queue {
    struct Node {
        std::atomic<T*> data{nullptr};
        std::atomic<Node*> next{nullptr};
    };
    
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
    hazard_pointer_manager hp_manager;
};
```

### Hazard Pointer Memory Management

Safe memory reclamation for lock-free structures:

```cpp
class hazard_pointer {
    // Protects pointer from deletion
    T* protect(std::atomic<T*>& ptr);
    
    // Retires pointer when safe
    void retire(T* ptr);
};
```

### Adaptive Strategy Selection

The system monitors these metrics:
- Contention level (CAS failures)
- Operation latency
- Queue depth
- Throughput

Strategy switches when:
- Contention > threshold for 100ms
- Performance degradation detected
- Queue characteristics change

## Performance Characteristics

### Measured Performance (Apple M1, 8 cores)

| Workload Type | Throughput | Latency | Scaling |
|---------------|------------|---------|---------|
| Empty jobs | 8.2M/s | 122ns | 95% |
| 1μs jobs | 1.5M/s | 667ns | 94% |
| 10μs jobs | 540K/s | 1.85μs | 92% |
| 100μs jobs | 70K/s | 14.3μs | 90% |

### Adaptive Queue Performance

| Scenario | Mutex Time | Lock-free Time | Improvement |
|----------|------------|----------------|-------------|
| Low contention (2 threads) | 45ms | 52ms | Mutex wins |
| High contention (16 threads) | 892ms | 756ms | +18% |
| Variable load | Baseline | Optimized | Automatic |

## Design Patterns

### Template Method Pattern
Used in `thread_base` for lifecycle management:

```cpp
void run() {
    before_start();
    while (!stop_requested()) {
        do_work();
    }
    after_stop();
}
```

### Command Pattern
Jobs encapsulate work as objects:

```cpp
class job {
    virtual auto do_work() -> result_void = 0;
};
```

### Strategy Pattern
Adaptive queues select strategies at runtime:

```cpp
class adaptive_job_queue {
    std::unique_ptr<queue_interface> current_strategy;
    
    void switch_strategy() {
        if (should_use_lockfree()) {
            current_strategy = make_lockfree();
        } else {
            current_strategy = make_mutex_based();
        }
    }
};
```

## Memory Management

### Smart Pointer Usage

- **unique_ptr**: Exclusive ownership (workers, jobs)
- **shared_ptr**: Shared ownership (queues, pools)
- **weak_ptr**: Break circular dependencies

### RAII Principles

All resources follow RAII:
```cpp
class thread_worker {
    std::unique_ptr<std::thread> thread_;
    // Automatic cleanup in destructor
};
```

### Lock-Free Memory Safety

Hazard pointers prevent use-after-free:
1. Protect pointer before access
2. Use pointer safely
3. Retire when done
4. Batch reclamation for efficiency

### Memory Optimization Strategies

#### Lazy Initialization
Adaptive queues now use lazy initialization to reduce initial memory footprint:
```cpp
class adaptive_job_queue {
    mutable std::unique_ptr<job_queue> legacy_queue_;
    mutable std::unique_ptr<lockfree_job_queue> mpmc_queue_;
    
    // Queues created only when needed
    auto ensure_legacy_queue() const -> void {
        if (!legacy_queue_) {
            legacy_queue_ = std::make_unique<job_queue>();
        }
    }
};
```

#### Optimized Node Pool
Node pools now use conservative initial allocation:
```cpp
// Before: 4 chunks × 1024 nodes = 4096 nodes pre-allocated
// After: 1 chunk × 256 nodes = 256 nodes pre-allocated
// Memory savings: ~93.75%
node_pool(size_t initial_chunks = 1, size_t chunk_size = 256);
```

#### Memory Usage Profile
- **Initial allocation**: Reduced by >90% for most components
- **On-demand growth**: Memory allocated as needed
- **Peak usage**: Unchanged - same maximum capacity
- **Performance impact**: Negligible - allocation amortized

## Platform Support

### Conditional Compilation

```cpp
#ifdef USE_STD_JTHREAD
    std::jthread worker_thread_;
#else
    std::thread worker_thread_;
    std::atomic<bool> stop_requested_;
#endif
```

### Platform-Specific Features

| Platform | Features | Optimizations |
|----------|----------|---------------|
| Linux | Full support | NUMA awareness |
| Windows | Full support | IOCP integration |
| macOS | Full support | GCD compatibility |

### Build Configuration

- **C++20**: Primary target with fallback
- **CMake 3.16+**: Cross-platform builds
- **vcpkg**: Dependency management

## Modularization Impact

### Before Modularization
- **Total Lines**: ~11,400
- **Components**: Monolithic with logger and monitoring
- **Dependencies**: Tightly coupled components
- **Build Time**: Longer due to larger codebase
- **Testing**: Complex due to interdependencies

### After Modularization
- **Core Lines**: ~2,700 (76% reduction)
- **Components**: Focused on threading only
- **Dependencies**: Clean interface-based separation
- **Build Time**: Significantly faster
- **Testing**: Simplified and more focused

### Key Improvements
- **8,700+ lines** moved to separate projects
- **Clear separation** of concerns
- **Optional dependencies** - use only what you need
- **Faster compilation** and linking
- **Easier to understand** and maintain

## Future Roadmap

### Short Term (Completed)
- ✅ Remove duplicate code
- ✅ Simplify architecture
- ✅ Add formatter macros
- ✅ Modularize logger into separate project
- ✅ Modularize monitoring into separate project
- ✅ Reduce core codebase by 76%

### Medium Term
- [ ] Enhanced monitoring dashboard
- [ ] Dynamic worker scaling
- [ ] Work-stealing queues
- [ ] CPU affinity support

### Long Term
- [ ] Distributed thread pools
- [ ] GPU task offloading
- [ ] Coroutine integration
- [ ] Real-time scheduling
- [ ] Plugin system for dynamic module loading
- [ ] Additional external modules (networking, serialization)

## Best Practices

### When to Use Each Component

1. **Standard Thread Pool**: General-purpose concurrent tasks
2. **Typed Thread Pool**: Priority-based scheduling requirements
3. **Adaptive Queues**: Variable or unknown workload patterns
4. **Lock-free Structures**: High-contention scenarios

### Module Integration Guidelines

1. **Core Only**: Use just the thread system for lightweight applications
2. **With Logger**: Add the logger module for production applications
3. **With Monitoring**: Add monitoring for performance-critical systems
4. **Custom Modules**: Implement the job interface for your own modules

### Configuration Guidelines

```cpp
// Low-latency configuration
auto pool = std::make_shared<thread_pool>();
for (int i = 0; i < cores; ++i) {
    auto worker = std::make_unique<thread_worker>();
    worker->set_wake_interval(std::chrono::microseconds(100));
    pool->enqueue(std::move(worker));
}

// High-throughput configuration
auto typed_pool = std::make_shared<typed_thread_pool<job_types>>();
typed_pool->set_queue_strategy(queue_strategy::ADAPTIVE);
```

### Performance Tuning

1. **Match workers to cores**: Avoid oversubscription
2. **Use batch operations**: Reduce synchronization overhead
3. **Profile first**: Measure before optimizing
4. **Consider job granularity**: Balance between overhead and parallelism

## Conclusion

The Thread System provides a robust, focused foundation for concurrent programming in modern C++. Through modularization, the core system has been reduced from ~11,400 lines to ~2,700 lines, resulting in a cleaner, more maintainable codebase. The interface-based architecture allows seamless integration with external modules like logging and monitoring, while the adaptive components ensure optimal performance across diverse workloads.

The modular design empowers developers to:
- Use only the components they need
- Integrate custom modules through clean interfaces
- Benefit from faster compilation and easier testing
- Focus on threading without auxiliary complexity

For practical examples, see the [samples directory](../samples/). For API details, consult the [API reference](./api-reference.md).

### External Module Projects
- **Logger Module**: [github.com/kcenon/logger_module](https://github.com/kcenon/logger_module)
- **Monitoring Module**: [github.com/kcenon/monitoring_module](https://github.com/kcenon/monitoring_module)