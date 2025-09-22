/**
 * @file logger_metrics.h
 * @brief Logger performance metrics and monitoring
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>

namespace kcenon::logger::metrics {

/**
 * @brief Performance statistics for logger operations
 */
struct logger_performance_stats {
    std::atomic<uint64_t> messages_logged{0};          ///< Total messages logged
    std::atomic<uint64_t> messages_dropped{0};         ///< Messages dropped due to queue full
    std::atomic<uint64_t> total_log_time_ns{0};        ///< Total time spent logging (nanoseconds)
    std::atomic<uint64_t> queue_size{0};               ///< Current queue size
    std::atomic<uint64_t> max_queue_size{0};           ///< Maximum queue size reached
    std::atomic<uint64_t> writer_errors{0};            ///< Number of writer errors
    std::atomic<uint64_t> flush_operations{0};         ///< Number of flush operations

    /**
     * @brief Get messages per second
     */
    double get_messages_per_second() const {
        auto msgs = messages_logged.load();
        auto time_ns = total_log_time_ns.load();
        if (time_ns == 0) return 0.0;
        return static_cast<double>(msgs) * 1'000'000'000.0 / static_cast<double>(time_ns);
    }

    /**
     * @brief Get average enqueue time in nanoseconds
     */
    uint64_t get_avg_enqueue_time_ns() const {
        auto msgs = messages_logged.load();
        auto time_ns = total_log_time_ns.load();
        if (msgs == 0) return 0;
        return time_ns / msgs;
    }

    /**
     * @brief Get queue utilization percentage
     */
    double get_queue_utilization_percent() const {
        auto current = queue_size.load();
        auto max_size = max_queue_size.load();
        if (max_size == 0) return 0.0;
        return static_cast<double>(current) * 100.0 / static_cast<double>(max_size);
    }

    /**
     * @brief Reset all statistics
     */
    void reset() {
        messages_logged.store(0);
        messages_dropped.store(0);
        total_log_time_ns.store(0);
        queue_size.store(0);
        max_queue_size.store(0);
        writer_errors.store(0);
        flush_operations.store(0);
    }
};

/**
 * @brief Global logger metrics instance
 */
extern logger_performance_stats g_logger_stats;

/**
 * @brief Record a logged message
 */
inline void record_message_logged(uint64_t time_ns) {
    g_logger_stats.messages_logged.fetch_add(1);
    g_logger_stats.total_log_time_ns.fetch_add(time_ns);
}

/**
 * @brief Record a dropped message
 */
inline void record_message_dropped() {
    g_logger_stats.messages_dropped.fetch_add(1);
}

/**
 * @brief Update queue size metrics
 */
inline void update_queue_size(uint64_t current_size) {
    g_logger_stats.queue_size.store(current_size);

    // Update max queue size if necessary
    uint64_t current_max = g_logger_stats.max_queue_size.load();
    while (current_size > current_max) {
        if (g_logger_stats.max_queue_size.compare_exchange_weak(current_max, current_size)) {
            break;
        }
    }
}

/**
 * @brief Record a writer error
 */
inline void record_writer_error() {
    g_logger_stats.writer_errors.fetch_add(1);
}

/**
 * @brief Record a flush operation
 */
inline void record_flush_operation() {
    g_logger_stats.flush_operations.fetch_add(1);
}

} // namespace kcenon::logger::metrics