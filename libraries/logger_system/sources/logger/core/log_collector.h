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

#include "../logger_interface.h"
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>

namespace logger_module {

class base_writer;

/**
 * @brief Asynchronous log collector for high-performance logging
 * 
 * Collects log entries in a lock-free queue and processes them
 * in a background thread to minimize logging overhead.
 */
class log_collector {
public:
    /**
     * @brief Constructor
     * @param buffer_size Size of the log buffer
     */
    explicit log_collector(std::size_t buffer_size = 8192);
    
    /**
     * @brief Destructor
     */
    ~log_collector();
    
    /**
     * @brief Enqueue a log entry
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param timestamp Log timestamp
     */
    bool enqueue(thread_module::log_level level,
                 const std::string& message,
                 const std::string& file,
                 int line,
                 const std::string& function,
                 const std::chrono::system_clock::time_point& timestamp);
    
    /**
     * @brief Add a writer
     * @param writer Pointer to writer (ownership remains with caller)
     */
    void add_writer(base_writer* writer);
    
    /**
     * @brief Clear all writers
     */
    void clear_writers();
    
    /**
     * @brief Start the background processing thread
     */
    void start();
    
    /**
     * @brief Stop the background processing thread
     */
    void stop();
    
    /**
     * @brief Flush all pending log entries
     */
    void flush();
    
    /**
     * @brief Get queue metrics
     * @return Pair of (current_size, max_capacity)
     */
    std::pair<size_t, size_t> get_queue_metrics() const;

    /**
     * @brief Performance statistics structure
     */
    struct performance_stats {
        uint64_t processed_messages;
        uint64_t dropped_messages;
        size_t current_queue_size;
        size_t queue_capacity;
        double drop_rate() const {
            auto total = processed_messages + dropped_messages;
            return total > 0 ? static_cast<double>(dropped_messages) / total : 0.0;
        }
    };

    /**
     * @brief Get performance statistics
     * @return Current performance metrics
     */
    performance_stats get_performance_stats() const;
    
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace logger_module