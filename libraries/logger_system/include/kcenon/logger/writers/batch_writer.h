#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "base_writer.h"
#include "../interfaces/log_entry.h"
#include <vector>
#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>

namespace kcenon::logger {

/**
 * @brief Batch writer that accumulates log entries and writes them in batches
 * 
 * This writer wraps another writer and accumulates log entries up to a
 * configurable batch size or timeout, then writes them all at once.
 * This reduces the number of I/O operations and improves performance.
 * 
 * Features:
 * - Configurable batch size
 * - Automatic flush on timeout
 * - Thread-safe batch accumulation
 * - Preserves original timestamps
 */
class batch_writer : public base_writer {
public:
    /**
     * @brief Configuration for batch writer
     */
    struct config {
        size_t max_batch_size;  // Maximum entries before auto-flush
        std::chrono::milliseconds flush_interval;  // Auto-flush interval
        bool preserve_order;  // Maintain strict ordering
        
        config()
            : max_batch_size(100)
            , flush_interval(1000)
            , preserve_order(true) {}
    };
    
    /**
     * @brief Construct a batch writer
     * @param underlying_writer The actual writer to batch writes for
     * @param cfg Batch configuration
     */
    explicit batch_writer(std::unique_ptr<base_writer> underlying_writer,
                          const config& cfg = config{});
    
    /**
     * @brief Destructor - ensures all pending entries are flushed
     */
    ~batch_writer() override;
    
    /**
     * @brief Write a log entry to the batch
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param timestamp Time of log entry
     * @return result_void indicating success or error
     */
    result_void write(logger_system::log_level level,
                     const std::string& message,
                     const std::string& file,
                     int line,
                     const std::string& function,
                     const std::chrono::system_clock::time_point& timestamp) override;
    
    /**
     * @brief Flush the batch to the underlying writer
     * @return result_void indicating success or error
     */
    result_void flush() override;
    
    /**
     * @brief Get writer name
     * @return Name of the batch writer
     */
    std::string get_name() const override;
    
    /**
     * @brief Check if writer is healthy
     * @return true if both batch writer and underlying writer are healthy
     */
    bool is_healthy() const override;
    
    /**
     * @brief Get current batch size
     * @return Number of entries currently in the batch
     */
    size_t get_current_batch_size() const;
    
    /**
     * @brief Get batch statistics
     */
    struct batch_stats {
        std::atomic<uint64_t> total_batches{0};
        std::atomic<uint64_t> total_entries{0};
        std::atomic<uint64_t> dropped_entries{0};
        std::atomic<uint64_t> flush_on_size{0};
        std::atomic<uint64_t> flush_on_timeout{0};
        std::atomic<uint64_t> manual_flushes{0};
        
        double average_batch_size() const {
            return total_batches > 0 ? 
                   static_cast<double>(total_entries) / total_batches : 0.0;
        }
    };
    
    /**
     * @brief Get batch statistics
     * @return Current batch statistics
     */
    const batch_stats& get_stats() const { return stats_; }
    
    /**
     * @brief Reset batch statistics
     */
    void reset_stats();
    
private:
    /**
     * @brief Internal structure to store batch entry
     */
    struct batch_entry {
        logger_system::log_level level;
        std::string message;
        std::string file;
        int line;
        std::string function;
        std::chrono::system_clock::time_point timestamp;
        
        batch_entry(logger_system::log_level lvl,
                   const std::string& msg,
                   const std::string& f,
                   int l,
                   const std::string& func,
                   const std::chrono::system_clock::time_point& ts)
            : level(lvl), message(msg), file(f), line(l), 
              function(func), timestamp(ts) {}
    };
    
    /**
     * @brief Flush batch without locking (caller must hold lock)
     * @return result_void indicating success or error
     */
    result_void flush_batch_unsafe();
    
    /**
     * @brief Check if batch should be flushed based on size
     * @return true if batch is full
     */
    bool should_flush_by_size() const;
    
    /**
     * @brief Check if batch should be flushed based on time
     * @return true if timeout has elapsed
     */
    bool should_flush_by_time() const;
    
    // Configuration
    config config_;
    
    // Underlying writer
    std::unique_ptr<base_writer> underlying_writer_;
    
    // Batch storage
    mutable std::mutex batch_mutex_;
    std::vector<batch_entry> batch_;
    std::chrono::steady_clock::time_point last_flush_time_;
    
    // Statistics
    mutable batch_stats stats_;
    
    // State
    std::atomic<bool> shutting_down_{false};
};

/**
 * @brief Factory function to create a batch writer
 * @param writer The underlying writer to wrap
 * @param batch_size Maximum batch size
 * @param flush_interval Auto-flush interval in milliseconds
 * @return Unique pointer to the batch writer
 */
std::unique_ptr<batch_writer> make_batch_writer(
    std::unique_ptr<base_writer> writer,
    size_t batch_size = 100,
    std::chrono::milliseconds flush_interval = std::chrono::milliseconds{1000});

} // namespace kcenon::logger