#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file high_performance_async_writer.h
 * @brief High-performance asynchronous writer using lock-free queue and memory pooling
 *
 * This file provides an optimized async writer that uses lock-free queues,
 * memory pooling, and dynamic batch processing for maximum performance.
 */

#include "lockfree_queue.h"
#include "batch_processor.h"
#include "../memory/object_pool.h"
#include "../memory/log_entry_pool.h"
#include <kcenon/logger/writers/base_writer.h>
#include <kcenon/logger/core/error_codes.h>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>  // Added for std::function usage

namespace kcenon::logger::async {

/**
 * @brief High-performance asynchronous writer
 *
 * This writer combines lock-free queuing, memory pooling, and batch processing
 * to achieve maximum throughput while maintaining low latency.
 */
class high_performance_async_writer : public base_writer {
public:
    /**
     * @brief Configuration for the high-performance async writer
     */
    struct config {
        // Queue configuration
        size_t queue_size{8192};              ///< Queue size (must be power of 2)

        // Batch processing configuration
        batch_processor::config batch_config; ///< Batch processor configuration

        // Memory pool configuration
        memory::object_pool<memory::log_entry_pool::pooled_log_entry>::config pool_config;

        // Performance tuning
        bool enable_memory_pooling{true};     ///< Enable memory pooling
        bool enable_batch_processing{true};   ///< Enable batch processing

        std::chrono::microseconds flush_timeout{1000}; ///< Flush timeout

        config() {
            // Optimize batch config for high performance
            batch_config.initial_batch_size = 50;
            batch_config.max_batch_size = 500;
            batch_config.min_batch_size = 10;
            batch_config.max_wait_time = std::chrono::milliseconds{100};
            batch_config.enable_dynamic_sizing = true;
            batch_config.enable_back_pressure = true;

            // Optimize pool config
            pool_config.initial_size = 200;
            pool_config.max_size = 2000;
            pool_config.allow_growth = true;
        }
    };

    /**
     * @brief Performance statistics
     */
    struct performance_stats {
        std::atomic<uint64_t> total_writes{0};
        std::atomic<uint64_t> successful_writes{0};
        std::atomic<uint64_t> dropped_writes{0};
        std::atomic<uint64_t> queue_full_events{0};

        std::atomic<double> average_latency_us{0.0};
        std::atomic<double> throughput_per_second{0.0};

        std::chrono::steady_clock::time_point start_time{std::chrono::steady_clock::now()};

        double success_ratio() const {
            const auto total = total_writes.load();
            return total > 0 ? static_cast<double>(successful_writes.load()) / total : 0.0;
        }

        void reset() {
            total_writes = 0;
            successful_writes = 0;
            dropped_writes = 0;
            queue_full_events = 0;
            average_latency_us = 0.0;
            throughput_per_second = 0.0;
            start_time = std::chrono::steady_clock::now();
        }
    };

    /**
     * @brief Constructor
     * @param wrapped_writer The underlying writer to wrap
     * @param cfg Configuration
     */
    explicit high_performance_async_writer(
        std::unique_ptr<base_writer> wrapped_writer,
        const config& cfg = config{});

    /**
     * @brief Destructor
     */
    ~high_performance_async_writer() override;

    /**
     * @brief Start the async writer
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop the async writer
     * @param flush_remaining Whether to flush remaining messages
     */
    void stop(bool flush_remaining = true);

    /**
     * @brief Write a log message asynchronously
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param timestamp Timestamp
     * @return result_void indicating success or error
     */
    result_void write(logger_system::log_level level,
                     const std::string& message,
                     const std::string& file,
                     int line,
                     const std::string& function,
                     const std::chrono::system_clock::time_point& timestamp) override;

    /**
     * @brief Flush all pending messages
     * @return result_void indicating success or error
     */
    result_void flush() override;

    /**
     * @brief Check if the writer is healthy
     * @return true if healthy, false otherwise
     */
    bool is_healthy() const override;

    /**
     * @brief Get the name of this writer
     * @return Writer name
     */
    std::string get_name() const override;

    /**
     * @brief Set whether to use color output
     * @param use_color Enable/disable color output
     */
    void set_use_color(bool use_color) override;

    /**
     * @brief Get performance statistics
     * @return Reference to performance statistics
     */
    const performance_stats& get_stats() const { return stats_; }

    /**
     * @brief Reset performance statistics
     */
    void reset_stats() { stats_.reset(); }

    /**
     * @brief Get current queue utilization
     * @return Queue utilization as percentage (0.0 to 1.0)
     */
    double get_queue_utilization() const;

    /**
     * @brief Get batch processor statistics if enabled
     * @return Pointer to batch processor stats, or nullptr if not enabled
     */
    const batch_processor::processing_stats* get_batch_stats() const;

private:
    /**
     * @brief Log entry optimized for high-performance queuing
     */
    struct queued_log_entry {
        logger_system::log_level level;
        std::string message;
        std::string file;
        int line;
        std::string function;
        std::chrono::system_clock::time_point timestamp;
        std::chrono::steady_clock::time_point enqueue_time; ///< For latency tracking

        queued_log_entry() = default;

        queued_log_entry(logger_system::log_level lvl,
                        std::string msg,
                        std::string f,
                        int l,
                        std::string func,
                        std::chrono::system_clock::time_point ts)
            : level(lvl)
            , message(std::move(msg))
            , file(std::move(f))
            , line(l)
            , function(std::move(func))
            , timestamp(ts)
            , enqueue_time(std::chrono::steady_clock::now()) {}
    };

    /**
     * @brief Write directly to the underlying writer (fallback mode)
     */
    result_void write_direct(logger_system::log_level level,
                           const std::string& message,
                           const std::string& file,
                           int line,
                           const std::string& function,
                           const std::chrono::system_clock::time_point& timestamp);

    /**
     * @brief Update performance statistics
     */
    void update_stats(bool success, std::chrono::nanoseconds latency);

    /**
     * @brief Convert to batch entry format
     */
    batch_processor::batch_entry to_batch_entry(const queued_log_entry& entry) const;

    // Configuration
    config config_;

    // Underlying writer
    std::unique_ptr<base_writer> wrapped_writer_;

    // High-performance components
    std::unique_ptr<batch_processor> batch_processor_;
    std::unique_ptr<memory::object_pool<memory::log_entry_pool::pooled_log_entry>> memory_pool_;

    // State management
    std::atomic<bool> running_{false};

    // Performance tracking
    mutable performance_stats stats_;
};

/**
 * @brief Factory function to create a high-performance async writer
 * @param writer Writer to wrap
 * @param cfg Configuration
 * @return Unique pointer to the writer
 */
std::unique_ptr<high_performance_async_writer> make_high_performance_async_writer(
    std::unique_ptr<base_writer> writer,
    const high_performance_async_writer::config& cfg = high_performance_async_writer::config{});

} // namespace kcenon::logger::async