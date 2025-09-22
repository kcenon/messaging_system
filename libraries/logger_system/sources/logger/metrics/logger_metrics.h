#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <mutex>

namespace logger_module {

/**
 * @struct writer_metrics
 * @brief Performance metrics for a specific log writer
 */
struct writer_metrics {
    std::atomic<uint64_t> messages_written{0};
    std::atomic<uint64_t> bytes_written{0};
    std::atomic<uint64_t> write_failures{0};
    std::atomic<uint64_t> total_write_time_us{0};
    std::atomic<uint64_t> min_write_time_us{UINT64_MAX};
    std::atomic<uint64_t> max_write_time_us{0};
    
    uint64_t get_avg_write_time_us() const {
        uint64_t msgs = messages_written.load();
        if (msgs == 0) return 0;
        return total_write_time_us.load() / msgs;
    }
};

/**
 * @struct performance_metrics
 * @brief Comprehensive performance metrics for the logger
 */
struct performance_metrics {
    // Delete copy constructor and copy assignment
    performance_metrics(const performance_metrics&) = delete;
    performance_metrics& operator=(const performance_metrics&) = delete;
    
    // Default constructor
    performance_metrics() = default;
    
    // Move constructor
    performance_metrics(performance_metrics&& other) noexcept
        : messages_enqueued(other.messages_enqueued.load())
        , bytes_enqueued(other.bytes_enqueued.load())
        , messages_processed(other.messages_processed.load())
        , bytes_processed(other.bytes_processed.load())
        , current_queue_size(other.current_queue_size.load())
        , max_queue_size(other.max_queue_size.load())
        , messages_dropped(other.messages_dropped.load())
        , total_enqueue_time_ns(other.total_enqueue_time_ns.load())
        , min_enqueue_time_ns(other.min_enqueue_time_ns.load())
        , max_enqueue_time_ns(other.max_enqueue_time_ns.load())
        , writer_stats(std::move(other.writer_stats))
        , start_time(other.start_time)
        , last_reset(other.last_reset) {}
    
    // Move assignment
    performance_metrics& operator=(performance_metrics&& other) noexcept {
        if (this != &other) {
            messages_enqueued.store(other.messages_enqueued.load());
            bytes_enqueued.store(other.bytes_enqueued.load());
            messages_processed.store(other.messages_processed.load());
            bytes_processed.store(other.bytes_processed.load());
            current_queue_size.store(other.current_queue_size.load());
            max_queue_size.store(other.max_queue_size.load());
            messages_dropped.store(other.messages_dropped.load());
            total_enqueue_time_ns.store(other.total_enqueue_time_ns.load());
            min_enqueue_time_ns.store(other.min_enqueue_time_ns.load());
            max_enqueue_time_ns.store(other.max_enqueue_time_ns.load());
            writer_stats = std::move(other.writer_stats);
            start_time = other.start_time;
            last_reset = other.last_reset;
        }
        return *this;
    }
    // Throughput metrics
    std::atomic<uint64_t> messages_enqueued{0};
    std::atomic<uint64_t> bytes_enqueued{0};
    std::atomic<uint64_t> messages_processed{0};
    std::atomic<uint64_t> bytes_processed{0};
    
    // Queue health
    std::atomic<size_t> current_queue_size{0};
    std::atomic<size_t> max_queue_size{0};
    std::atomic<uint64_t> messages_dropped{0};
    
    // Latency metrics (in nanoseconds)
    std::atomic<uint64_t> total_enqueue_time_ns{0};
    std::atomic<uint64_t> min_enqueue_time_ns{UINT64_MAX};
    std::atomic<uint64_t> max_enqueue_time_ns{0};
    
    // Writer performance (protected by mutex for map access)
    mutable std::mutex writer_metrics_mutex;
    std::unordered_map<std::string, writer_metrics> writer_stats;
    
    // Time window for rate calculations
    std::chrono::steady_clock::time_point start_time{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point last_reset{std::chrono::steady_clock::now()};
    
    // Calculated metrics
    double get_messages_per_second() const {
        auto duration = std::chrono::steady_clock::now() - start_time;
        auto seconds = std::chrono::duration<double>(duration).count();
        return seconds > 0 ? messages_processed.load() / seconds : 0;
    }
    
    double get_bytes_per_second() const {
        auto duration = std::chrono::steady_clock::now() - start_time;
        auto seconds = std::chrono::duration<double>(duration).count();
        return seconds > 0 ? bytes_processed.load() / seconds : 0;
    }
    
    uint64_t get_avg_enqueue_time_ns() const {
        uint64_t msgs = messages_enqueued.load();
        if (msgs == 0) return 0;
        return total_enqueue_time_ns.load() / msgs;
    }
    
    float get_queue_utilization_percent() const {
        size_t max_size = max_queue_size.load();
        if (max_size == 0) return 0;
        return (current_queue_size.load() * 100.0f) / max_size;
    }
    
    float get_drop_rate_percent() const {
        uint64_t total = messages_enqueued.load();
        if (total == 0) return 0;
        return (messages_dropped.load() * 100.0f) / total;
    }
};

/**
 * @class logger_metrics_collector
 * @brief Collects and manages logger performance metrics
 */
class logger_metrics_collector {
public:
    logger_metrics_collector() = default;
    
    // Record enqueue operation
    void record_enqueue(size_t message_size, std::chrono::nanoseconds enqueue_time) {
        metrics_.messages_enqueued.fetch_add(1);
        metrics_.bytes_enqueued.fetch_add(message_size);
        metrics_.total_enqueue_time_ns.fetch_add(enqueue_time.count());
        
        // Update min/max
        uint64_t time_ns = enqueue_time.count();
        update_atomic_min(metrics_.min_enqueue_time_ns, time_ns);
        update_atomic_max(metrics_.max_enqueue_time_ns, time_ns);
    }
    
    // Record dropped message
    void record_drop() {
        metrics_.messages_dropped.fetch_add(1);
    }
    
    // Record processed message
    void record_processed(size_t message_size) {
        metrics_.messages_processed.fetch_add(1);
        metrics_.bytes_processed.fetch_add(message_size);
    }
    
    // Record writer performance
    void record_write(const std::string& writer_name, 
                     size_t message_size,
                     std::chrono::microseconds write_time,
                     bool success) {
        std::lock_guard<std::mutex> lock(metrics_.writer_metrics_mutex);
        auto& writer = metrics_.writer_stats[writer_name];
        
        if (success) {
            writer.messages_written.fetch_add(1);
            writer.bytes_written.fetch_add(message_size);
            writer.total_write_time_us.fetch_add(write_time.count());
            
            uint64_t time_us = write_time.count();
            update_atomic_min(writer.min_write_time_us, time_us);
            update_atomic_max(writer.max_write_time_us, time_us);
        } else {
            writer.write_failures.fetch_add(1);
        }
    }
    
    // Update queue size
    void update_queue_size(size_t current_size, size_t max_size) {
        metrics_.current_queue_size.store(current_size);
        metrics_.max_queue_size.store(max_size);
    }
    
    // Get current metrics snapshot
    performance_metrics get_snapshot() const {
        performance_metrics snapshot;
        
        // Copy atomic values
        snapshot.messages_enqueued.store(metrics_.messages_enqueued.load());
        snapshot.bytes_enqueued.store(metrics_.bytes_enqueued.load());
        snapshot.messages_processed.store(metrics_.messages_processed.load());
        snapshot.bytes_processed.store(metrics_.bytes_processed.load());
        snapshot.current_queue_size.store(metrics_.current_queue_size.load());
        snapshot.max_queue_size.store(metrics_.max_queue_size.load());
        snapshot.messages_dropped.store(metrics_.messages_dropped.load());
        snapshot.total_enqueue_time_ns.store(metrics_.total_enqueue_time_ns.load());
        snapshot.min_enqueue_time_ns.store(metrics_.min_enqueue_time_ns.load());
        snapshot.max_enqueue_time_ns.store(metrics_.max_enqueue_time_ns.load());
        
        // Copy time points
        snapshot.start_time = metrics_.start_time;
        snapshot.last_reset = metrics_.last_reset;
        
        // Deep copy writer metrics
        std::lock_guard<std::mutex> lock(metrics_.writer_metrics_mutex);
        for (const auto& [name, writer] : metrics_.writer_stats) {
            auto& dest = snapshot.writer_stats[name];
            dest.messages_written.store(writer.messages_written.load());
            dest.bytes_written.store(writer.bytes_written.load());
            dest.write_failures.store(writer.write_failures.load());
            dest.total_write_time_us.store(writer.total_write_time_us.load());
            dest.min_write_time_us.store(writer.min_write_time_us.load());
            dest.max_write_time_us.store(writer.max_write_time_us.load());
        }
        
        return snapshot;
    }
    
    // Reset metrics
    void reset() {
        // Reset atomic values
        metrics_.messages_enqueued.store(0);
        metrics_.bytes_enqueued.store(0);
        metrics_.messages_processed.store(0);
        metrics_.bytes_processed.store(0);
        metrics_.current_queue_size.store(0);
        metrics_.max_queue_size.store(0);
        metrics_.messages_dropped.store(0);
        metrics_.total_enqueue_time_ns.store(0);
        metrics_.min_enqueue_time_ns.store(UINT64_MAX);
        metrics_.max_enqueue_time_ns.store(0);
        
        // Reset time points
        metrics_.start_time = std::chrono::steady_clock::now();
        metrics_.last_reset = metrics_.start_time;
        
        // Clear writer stats
        std::lock_guard<std::mutex> lock(metrics_.writer_metrics_mutex);
        metrics_.writer_stats.clear();
    }
    
private:
    mutable performance_metrics metrics_;
    
    // Helper to update atomic minimum
    void update_atomic_min(std::atomic<uint64_t>& atomic_val, uint64_t new_val) {
        uint64_t current = atomic_val.load();
        while (new_val < current && !atomic_val.compare_exchange_weak(current, new_val));
    }
    
    // Helper to update atomic maximum
    void update_atomic_max(std::atomic<uint64_t>& atomic_val, uint64_t new_val) {
        uint64_t current = atomic_val.load();
        while (new_val > current && !atomic_val.compare_exchange_weak(current, new_val));
    }
};

} // namespace logger_module