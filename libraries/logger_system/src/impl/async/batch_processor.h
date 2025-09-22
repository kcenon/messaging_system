#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

/**
 * @file batch_processor.h
 * @brief Optimized batch processing engine for log entries
 *
 * This file provides an enhanced batch processor with dynamic sizing,
 * back-pressure handling, and optimized flush conditions.
 */

#include "lockfree_queue.h"
#include <kcenon/logger/interfaces/log_entry.h>
#include <kcenon/logger/writers/base_writer.h>
#include <kcenon/logger/interfaces/logger_types.h>
#include <kcenon/logger/core/error_codes.h>
#include <vector>
#include <chrono>
#include <atomic>
#include <memory>
#include <thread>
#include <functional>
#include <mutex>              // Added for synchronization primitives
#include <condition_variable> // Added for threading coordination

namespace kcenon::logger::async {

/**
 * @brief Advanced batch processor with dynamic sizing and back-pressure handling
 */
class batch_processor {
public:
    /**
     * @brief Configuration for batch processor
     */
    struct config {
        size_t initial_batch_size{100};      ///< Initial batch size
        size_t min_batch_size{10};           ///< Minimum batch size
        size_t max_batch_size{1000};         ///< Maximum batch size

        std::chrono::milliseconds max_wait_time{1000};    ///< Maximum wait time
        std::chrono::milliseconds min_wait_time{10};      ///< Minimum wait time

        bool enable_dynamic_sizing{true};     ///< Enable dynamic batch sizing
        bool enable_back_pressure{true};      ///< Enable back-pressure handling

        double size_increase_factor{1.5};     ///< Factor for increasing batch size
        double size_decrease_factor{0.8};     ///< Factor for decreasing batch size

        size_t back_pressure_threshold{5000}; ///< Queue size threshold for back-pressure
        std::chrono::microseconds back_pressure_delay{100}; ///< Delay when under back-pressure

        config() noexcept {}
    };

    /**
     * @brief Batch entry structure
     */
    struct batch_entry {
        logger_system::log_level level;
        std::string message;
        std::string file;
        int line;
        std::string function;
        std::chrono::system_clock::time_point timestamp;

        batch_entry() = default;

        batch_entry(logger_system::log_level lvl,
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
            , timestamp(ts) {}
    };

    /**
     * @brief Processing statistics
     */
    struct processing_stats {
        std::atomic<uint64_t> total_batches{0};
        std::atomic<uint64_t> total_entries{0};
        std::atomic<uint64_t> dropped_entries{0};
        std::atomic<uint64_t> back_pressure_events{0};
        std::atomic<uint64_t> dynamic_size_adjustments{0};

        std::atomic<uint64_t> flush_by_size{0};
        std::atomic<uint64_t> flush_by_time{0};
        std::atomic<uint64_t> flush_by_manual{0};

        std::atomic<double> average_batch_size{0.0};
        std::atomic<double> average_processing_time_ms{0.0};

        void reset() {
            total_batches = 0;
            total_entries = 0;
            dropped_entries = 0;
            back_pressure_events = 0;
            dynamic_size_adjustments = 0;
            flush_by_size = 0;
            flush_by_time = 0;
            flush_by_manual = 0;
            average_batch_size = 0.0;
            average_processing_time_ms = 0.0;
        }
    };

    /**
     * @brief Constructor
     * @param writer Target writer for batch output
     * @param cfg Configuration
     */
    explicit batch_processor(std::unique_ptr<base_writer> writer,
                           const config& cfg = config{});

    /**
     * @brief Destructor
     */
    ~batch_processor();

    /**
     * @brief Start the batch processor
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop the batch processor
     * @param flush_remaining Whether to flush remaining entries
     */
    void stop(bool flush_remaining = true);

    /**
     * @brief Add entry to the batch queue
     * @param entry Log entry to add
     * @return true if added successfully, false if queue is full
     */
    bool add_entry(batch_entry&& entry);

    /**
     * @brief Add entry to the batch queue (copy version)
     * @param entry Log entry to add
     * @return true if added successfully, false if queue is full
     */
    bool add_entry(const batch_entry& entry);

    /**
     * @brief Force flush current batch
     */
    void flush();

    /**
     * @brief Check if processor is healthy
     * @return true if running and writer is healthy
     */
    bool is_healthy() const;

    /**
     * @brief Get current processing statistics
     * @return Reference to current stats
     */
    const processing_stats& get_stats() const { return stats_; }

    /**
     * @brief Reset statistics
     */
    void reset_stats() { stats_.reset(); }

    /**
     * @brief Get current queue size
     * @return Approximate queue size
     */
    size_t get_queue_size() const;

    /**
     * @brief Get current batch size setting
     * @return Current batch size
     */
    size_t get_current_batch_size() const {
        return current_batch_size_.load(std::memory_order_relaxed);
    }

private:
    /**
     * @brief Main processing loop
     */
    void process_loop();

    /**
     * @brief Process current batch
     * @param batch Vector of entries to process
     * @return Number of entries processed
     */
    size_t process_batch(std::vector<batch_entry>& batch);

    /**
     * @brief Collect entries for batch processing
     * @param batch Vector to fill with entries
     * @param max_entries Maximum entries to collect
     * @param deadline Time deadline for collection
     * @return Number of entries collected
     */
    size_t collect_entries(std::vector<batch_entry>& batch,
                          size_t max_entries,
                          std::chrono::steady_clock::time_point deadline);

    /**
     * @brief Adjust batch size based on performance metrics
     */
    void adjust_batch_size();

    /**
     * @brief Handle back-pressure conditions
     * @return true if should continue processing
     */
    bool handle_back_pressure();

    /**
     * @brief Check if batch should be flushed based on time
     * @param last_flush_time Time of last flush
     * @return true if timeout reached
     */
    bool should_flush_by_time(std::chrono::steady_clock::time_point last_flush_time) const;

    /**
     * @brief Update processing statistics
     * @param batch_size Size of processed batch
     * @param processing_time Time taken to process
     * @param flush_reason Reason for flush
     */
    void update_stats(size_t batch_size,
                     std::chrono::nanoseconds processing_time,
                     const std::string& flush_reason);

    // Configuration
    config config_;

    // Writer
    std::unique_ptr<base_writer> writer_;

    // Queue
    static constexpr size_t queue_size = 8192;  // Must be power of 2
    std::unique_ptr<lockfree_spsc_queue<batch_entry, queue_size>> queue_;

    // Processing thread
    std::thread processing_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> should_stop_{false};

    // Dynamic sizing
    std::atomic<size_t> current_batch_size_;
    std::atomic<std::chrono::milliseconds> current_wait_time_;

    // Statistics
    mutable processing_stats stats_;

    // Performance metrics for dynamic adjustment
    std::atomic<double> recent_processing_time_ms_{0.0};
    std::atomic<size_t> recent_queue_size_{0};
    std::chrono::steady_clock::time_point last_adjustment_time_;
};

/**
 * @brief Factory function to create a batch processor
 * @param writer Target writer
 * @param cfg Configuration
 * @return Unique pointer to batch processor
 */
std::unique_ptr<batch_processor> make_batch_processor(
    std::unique_ptr<base_writer> writer,
    const batch_processor::config& cfg = batch_processor::config{});

} // namespace kcenon::logger::async