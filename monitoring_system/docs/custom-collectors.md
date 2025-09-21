# Creating Custom Collectors

This guide explains how to create custom collectors for the Monitoring System to gather metrics from various sources.

## Overview

Collectors are responsible for gathering metrics during each collection cycle. The Monitoring System provides a flexible `metrics_collector` base class that you can extend to collect custom metrics.

## Base Collector Interface

All custom collectors must inherit from `metrics_collector`:

```cpp
class metrics_collector {
public:
    virtual ~metrics_collector() = default;
    
    // Called during each collection cycle
    virtual void collect(monitoring_interface::metrics_snapshot& snapshot) = 0;
    
    // Return a descriptive name for the collector
    virtual std::string name() const = 0;
};
```

## Simple Examples

### 1. CPU Usage Collector (Linux)

```cpp
#include <monitoring_system/monitoring.h>
#include <fstream>
#include <sstream>

class linux_cpu_collector : public monitoring_module::metrics_collector {
private:
    struct cpu_stats {
        long user, nice, system, idle, iowait, irq, softirq;
    };
    
    cpu_stats last_stats_{};
    
    cpu_stats read_cpu_stats() {
        std::ifstream stat_file("/proc/stat");
        std::string line;
        std::getline(stat_file, line);
        
        cpu_stats stats{};
        std::istringstream iss(line);
        std::string cpu;
        iss >> cpu >> stats.user >> stats.nice >> stats.system 
            >> stats.idle >> stats.iowait >> stats.irq >> stats.softirq;
        
        return stats;
    }
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        auto current = read_cpu_stats();
        
        // Calculate deltas
        long total_delta = (current.user - last_stats_.user) +
                          (current.nice - last_stats_.nice) +
                          (current.system - last_stats_.system) +
                          (current.idle - last_stats_.idle) +
                          (current.iowait - last_stats_.iowait) +
                          (current.irq - last_stats_.irq) +
                          (current.softirq - last_stats_.softirq);
        
        long active_delta = total_delta - (current.idle - last_stats_.idle);
        
        if (total_delta > 0) {
            snapshot.system.cpu_usage_percent = (active_delta * 100) / total_delta;
        }
        
        last_stats_ = current;
    }
    
    std::string name() const override {
        return "LinuxCPUCollector";
    }
};

// Usage
monitor->add_collector(std::make_unique<linux_cpu_collector>());
```

### 2. Memory Usage Collector

```cpp
class memory_collector : public monitoring_module::metrics_collector {
private:
    #ifdef __linux__
    std::uint64_t get_memory_usage() {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        std::uint64_t total_kb = 0, available_kb = 0;
        
        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                std::sscanf(line.c_str(), "MemTotal: %lu kB", &total_kb);
            } else if (line.find("MemAvailable:") == 0) {
                std::sscanf(line.c_str(), "MemAvailable: %lu kB", &available_kb);
            }
        }
        
        return (total_kb - available_kb) * 1024; // Convert to bytes
    }
    #elif defined(_WIN32)
    std::uint64_t get_memory_usage() {
        MEMORYSTATUSEX mem_info;
        mem_info.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&mem_info);
        return mem_info.ullTotalPhys - mem_info.ullAvailPhys;
    }
    #else
    std::uint64_t get_memory_usage() {
        return 0; // Not implemented
    }
    #endif
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        snapshot.system.memory_usage_bytes = get_memory_usage();
    }
    
    std::string name() const override {
        return "MemoryCollector";
    }
};
```

### 3. Process-Specific Collector

```cpp
class process_collector : public monitoring_module::metrics_collector {
private:
    pid_t pid_;
    
    struct process_stats {
        std::uint64_t cpu_time_ns;
        std::uint64_t memory_rss_bytes;
        std::uint64_t thread_count;
    };
    
    process_stats get_process_stats() {
        process_stats stats{};
        
        #ifdef __linux__
        // Read from /proc/[pid]/stat
        std::string stat_path = "/proc/" + std::to_string(pid_) + "/stat";
        std::ifstream stat_file(stat_path);
        
        if (stat_file.is_open()) {
            std::string line;
            std::getline(stat_file, line);
            
            // Parse the stat file (simplified)
            std::vector<std::string> fields;
            std::istringstream iss(line);
            std::string field;
            while (iss >> field) {
                fields.push_back(field);
            }
            
            if (fields.size() > 23) {
                // utime + stime (in clock ticks)
                long utime = std::stol(fields[13]);
                long stime = std::stol(fields[14]);
                stats.cpu_time_ns = (utime + stime) * (1000000000L / sysconf(_SC_CLK_TCK));
                
                // RSS pages
                long rss_pages = std::stol(fields[23]);
                stats.memory_rss_bytes = rss_pages * sysconf(_SC_PAGESIZE);
                
                // Thread count
                stats.thread_count = std::stoul(fields[19]);
            }
        }
        #endif
        
        return stats;
    }
    
public:
    explicit process_collector(pid_t pid = 0) 
        : pid_(pid ? pid : getpid()) {}
    
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        auto stats = get_process_stats();
        
        // Update relevant fields
        snapshot.system.memory_usage_bytes = stats.memory_rss_bytes;
        snapshot.system.active_threads = stats.thread_count;
        
        // Could extend metrics_snapshot to include process-specific data
    }
    
    std::string name() const override {
        return "ProcessCollector[" + std::to_string(pid_) + "]";
    }
};
```

## Advanced Collectors

### 1. Network Statistics Collector

```cpp
class network_stats_collector : public monitoring_module::metrics_collector {
private:
    struct network_stats {
        std::uint64_t bytes_sent;
        std::uint64_t bytes_received;
        std::uint64_t packets_sent;
        std::uint64_t packets_received;
    };
    
    network_stats last_stats_{};
    std::chrono::steady_clock::time_point last_collection_;
    
    network_stats read_network_stats() {
        network_stats total{};
        
        #ifdef __linux__
        std::ifstream net_dev("/proc/net/dev");
        std::string line;
        
        // Skip header lines
        std::getline(net_dev, line);
        std::getline(net_dev, line);
        
        while (std::getline(net_dev, line)) {
            if (line.find("lo:") != std::string::npos) {
                continue; // Skip loopback
            }
            
            std::istringstream iss(line);
            std::string interface;
            iss >> interface;
            
            network_stats iface_stats{};
            iss >> iface_stats.bytes_received
                >> iface_stats.packets_received
                >> std::ws // Skip some fields
                >> std::ws >> std::ws >> std::ws >> std::ws >> std::ws
                >> iface_stats.bytes_sent
                >> iface_stats.packets_sent;
            
            total.bytes_sent += iface_stats.bytes_sent;
            total.bytes_received += iface_stats.bytes_received;
            total.packets_sent += iface_stats.packets_sent;
            total.packets_received += iface_stats.packets_received;
        }
        #endif
        
        return total;
    }
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        auto current = read_network_stats();
        auto now = std::chrono::steady_clock::now();
        
        if (last_collection_ != std::chrono::steady_clock::time_point{}) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - last_collection_).count();
            
            if (duration > 0) {
                // Calculate rates (bytes per second)
                auto send_rate = (current.bytes_sent - last_stats_.bytes_sent) / duration;
                auto recv_rate = (current.bytes_received - last_stats_.bytes_received) / duration;
                
                // Would need to extend metrics_snapshot for network data
                // For now, could repurpose unused fields or use custom storage
            }
        }
        
        last_stats_ = current;
        last_collection_ = now;
    }
    
    std::string name() const override {
        return "NetworkStatsCollector";
    }
};
```

### 2. Disk I/O Collector

```cpp
class disk_io_collector : public monitoring_module::metrics_collector {
private:
    struct disk_stats {
        std::uint64_t read_bytes;
        std::uint64_t write_bytes;
        std::uint64_t read_ops;
        std::uint64_t write_ops;
    };
    
    std::unordered_map<std::string, disk_stats> last_stats_;
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        #ifdef __linux__
        std::ifstream diskstats("/proc/diskstats");
        std::string line;
        
        disk_stats total{};
        
        while (std::getline(diskstats, line)) {
            std::istringstream iss(line);
            int major, minor;
            std::string device;
            
            iss >> major >> minor >> device;
            
            // Skip partitions, only count whole devices
            if (device.find_first_of("0123456789") != std::string::npos) {
                continue;
            }
            
            disk_stats stats{};
            std::uint64_t tmp;
            
            // Skip to the fields we want
            iss >> stats.read_ops >> tmp >> stats.read_bytes >> tmp
                >> stats.write_ops >> tmp >> stats.write_bytes;
            
            // Convert sectors to bytes
            stats.read_bytes *= 512;
            stats.write_bytes *= 512;
            
            // Calculate deltas
            if (last_stats_.count(device)) {
                auto& last = last_stats_[device];
                total.read_bytes += stats.read_bytes - last.read_bytes;
                total.write_bytes += stats.write_bytes - last.write_bytes;
                total.read_ops += stats.read_ops - last.read_ops;
                total.write_ops += stats.write_ops - last.write_ops;
            }
            
            last_stats_[device] = stats;
        }
        
        // Store in custom fields or aggregate into existing metrics
        #endif
    }
    
    std::string name() const override {
        return "DiskIOCollector";
    }
};
```

### 3. Application-Specific Collector

```cpp
// Example: Database connection pool monitor
class db_pool_collector : public monitoring_module::metrics_collector {
private:
    std::shared_ptr<connection_pool> pool_;
    
public:
    explicit db_pool_collector(std::shared_ptr<connection_pool> pool)
        : pool_(pool) {}
    
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        if (!pool_) return;
        
        auto stats = pool_->get_statistics();
        
        // Map to thread pool metrics (similar concept)
        snapshot.thread_pool.worker_threads = stats.total_connections;
        snapshot.thread_pool.idle_threads = stats.idle_connections;
        snapshot.thread_pool.jobs_pending = stats.waiting_requests;
        snapshot.thread_pool.jobs_completed = stats.total_requests;
        
        // Calculate average latency
        if (stats.total_requests > 0) {
            snapshot.thread_pool.average_latency_ns = 
                stats.total_request_time_ns / stats.total_requests;
        }
    }
    
    std::string name() const override {
        return "DatabasePoolCollector";
    }
};
```

## Best Practices

### 1. Error Handling

```cpp
class robust_collector : public monitoring_module::metrics_collector {
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        try {
            // Potentially failing operation
            auto data = read_external_source();
            snapshot.system.custom_metric = process_data(data);
        } catch (const std::exception& e) {
            // Log error but don't propagate
            // The monitoring system handles collector errors
        }
    }
};
```

### 2. Caching Expensive Operations

```cpp
class cached_collector : public monitoring_module::metrics_collector {
private:
    mutable std::chrono::steady_clock::time_point last_update_;
    mutable std::uint64_t cached_value_;
    static constexpr auto cache_duration = std::chrono::seconds(5);
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        auto now = std::chrono::steady_clock::now();
        
        if (now - last_update_ > cache_duration) {
            cached_value_ = expensive_calculation();
            last_update_ = now;
        }
        
        snapshot.system.custom_metric = cached_value_;
    }
};
```

### 3. Thread-Safe Collectors

```cpp
class thread_safe_collector : public monitoring_module::metrics_collector {
private:
    mutable std::mutex mutex_;
    shared_resource resource_;
    
public:
    void collect(monitoring_interface::metrics_snapshot& snapshot) override {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot.system.custom_metric = resource_.get_value();
    }
    
    void update_resource(const data& new_data) {
        std::lock_guard<std::mutex> lock(mutex_);
        resource_.update(new_data);
    }
};
```

## Testing Custom Collectors

```cpp
TEST(CollectorTest, CustomCollector) {
    // Create monitoring instance
    auto monitor = std::make_shared<monitoring_module::monitoring>();
    
    // Add custom collector
    auto collector = std::make_unique<my_custom_collector>();
    monitor->add_collector(std::move(collector));
    
    // Force collection
    monitor->collect_now();
    
    // Verify results
    auto snapshot = monitor->get_current_snapshot();
    EXPECT_GT(snapshot.system.custom_metric, 0);
}

// Benchmark collector performance
static void BM_CollectorPerformance(benchmark::State& state) {
    auto collector = std::make_unique<my_custom_collector>();
    monitoring_interface::metrics_snapshot snapshot;
    
    for (auto _ : state) {
        collector->collect(snapshot);
    }
}
BENCHMARK(BM_CollectorPerformance);
```

## Integration Example

```cpp
int main() {
    // Create monitoring system
    auto monitor = std::make_shared<monitoring_module::monitoring>();
    
    // Add multiple collectors
    monitor->add_collector(std::make_unique<linux_cpu_collector>());
    monitor->add_collector(std::make_unique<memory_collector>());
    monitor->add_collector(std::make_unique<network_stats_collector>());
    monitor->add_collector(std::make_unique<disk_io_collector>());
    
    // Start monitoring
    monitor->start();
    
    // Your application runs here...
    
    // Get comprehensive metrics
    auto snapshot = monitor->get_current_snapshot();
    std::cout << "System Metrics:" << std::endl;
    std::cout << "  CPU: " << snapshot.system.cpu_usage_percent << "%" << std::endl;
    std::cout << "  Memory: " << snapshot.system.memory_usage_bytes / (1024*1024) << " MB" << std::endl;
    
    monitor->stop();
    return 0;
}
```