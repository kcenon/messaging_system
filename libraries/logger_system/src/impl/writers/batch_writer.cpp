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

#include <kcenon/logger/writers/batch_writer.h>
#include <algorithm>

namespace kcenon::logger {

batch_writer::batch_writer(std::unique_ptr<base_writer> underlying_writer,
                          const config& cfg)
    : config_(cfg)
    , underlying_writer_(std::move(underlying_writer))
    , last_flush_time_(std::chrono::steady_clock::now()) {
    
    if (!underlying_writer_) {
        throw std::invalid_argument("Underlying writer cannot be null");
    }
    
    // Reserve space for batch to avoid reallocations
    batch_.reserve(config_.max_batch_size);
}

batch_writer::~batch_writer() {
    shutting_down_ = true;
    
    // Flush any remaining entries
    if (!batch_.empty()) {
        flush();
    }
}

result_void batch_writer::write(logger_system::log_level level,
                               const std::string& message,
                               const std::string& file,
                               int line,
                               const std::string& function,
                               const std::chrono::system_clock::time_point& timestamp) {
    
    if (shutting_down_) {
        return make_logger_error(logger_error_code::queue_stopped, "Batch writer is shutting down");
    }
    
    bool should_flush = false;
    
    {
        std::lock_guard<std::mutex> lock(batch_mutex_);
        
        // Add entry to batch
        batch_.emplace_back(level, message, file, line, function, timestamp);
        stats_.total_entries++;
        
        // Check if we should flush
        should_flush = should_flush_by_size();
        if (should_flush) {
            stats_.flush_on_size++;
        }
    }
    
    // Flush outside of lock to avoid blocking other writers
    if (should_flush) {
        return flush();
    }
    
    return result_void{};
}

result_void batch_writer::flush() {
    if (shutting_down_ && batch_.empty()) {
        return result_void{};
    }
    
    std::lock_guard<std::mutex> lock(batch_mutex_);
    
    if (!shutting_down_) {
        stats_.manual_flushes++;
    }
    
    return flush_batch_unsafe();
}

result_void batch_writer::flush_batch_unsafe() {
    if (batch_.empty()) {
        return result_void{};
    }
    
    // Write all entries in the batch
    result_void last_result;
    
    for (const auto& entry : batch_) {
        auto result = underlying_writer_->write(
            entry.level,
            entry.message,
            entry.file,
            entry.line,
            entry.function,
            entry.timestamp
        );
        
        if (!result) {
            last_result = result;
            stats_.dropped_entries++;
        }
    }
    
    // Flush the underlying writer
    auto flush_result = underlying_writer_->flush();
    if (!flush_result && !last_result) {
        last_result = flush_result;
    }
    
    // Update statistics
    stats_.total_batches++;
    
    // Clear the batch
    batch_.clear();
    last_flush_time_ = std::chrono::steady_clock::now();
    
    // Return the last error if any
    return last_result ? result_void{} : last_result;
}

std::string batch_writer::get_name() const {
    return "batch_writer[" + underlying_writer_->get_name() + "]";
}

bool batch_writer::is_healthy() const {
    return !shutting_down_ && underlying_writer_->is_healthy();
}

size_t batch_writer::get_current_batch_size() const {
    std::lock_guard<std::mutex> lock(batch_mutex_);
    return batch_.size();
}

void batch_writer::reset_stats() {
    stats_.total_batches = 0;
    stats_.total_entries = 0;
    stats_.dropped_entries = 0;
    stats_.flush_on_size = 0;
    stats_.flush_on_timeout = 0;
    stats_.manual_flushes = 0;
}

bool batch_writer::should_flush_by_size() const {
    return batch_.size() >= config_.max_batch_size;
}

bool batch_writer::should_flush_by_time() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_flush_time_
    );
    return elapsed >= config_.flush_interval;
}

// Factory function implementation
std::unique_ptr<batch_writer> make_batch_writer(
    std::unique_ptr<base_writer> writer,
    size_t batch_size,
    std::chrono::milliseconds flush_interval) {
    
    batch_writer::config cfg;
    cfg.max_batch_size = batch_size;
    cfg.flush_interval = flush_interval;
    
    return std::make_unique<batch_writer>(std::move(writer), cfg);
}

} // namespace kcenon::logger