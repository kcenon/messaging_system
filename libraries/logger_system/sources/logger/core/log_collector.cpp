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

#include "log_collector.h"
#include "../writers/base_writer.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <array>

// Lock-free SPSC ring buffer for high-performance logging
template<typename T, size_t Size>
class lockfree_ring_buffer {
private:
    static_assert((Size & (Size - 1)) == 0, "Size must be power of 2");
    static constexpr size_t mask = Size - 1;

    std::array<T, Size> buffer_;
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};

public:
    bool push(T&& item) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) & mask;

        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // Buffer is full
        }

        buffer_[current_tail] = std::move(item);
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const size_t current_head = head_.load(std::memory_order_relaxed);

        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer is empty
        }

        item = std::move(buffer_[current_head]);
        head_.store((current_head + 1) & mask, std::memory_order_release);
        return true;
    }

    size_t size() const {
        const size_t tail = tail_.load(std::memory_order_acquire);
        const size_t head = head_.load(std::memory_order_acquire);
        return (tail - head) & mask;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    static constexpr size_t capacity() { return Size; }
};

namespace logger_module {

struct log_entry {
    thread_module::log_level level;
    std::string message;
    std::string file;
    int line;
    std::string function;
    std::chrono::system_clock::time_point timestamp;
};

class log_collector::impl {
public:
    explicit impl(std::size_t buffer_size)
        : buffer_size_(buffer_size)
        , running_(false)
        , dropped_messages_(0)
        , processed_messages_(0) {
        // Ensure buffer size is power of 2 for lock-free ring buffer
        size_t power_of_2_size = 1;
        while (power_of_2_size < buffer_size) {
            power_of_2_size <<= 1;
        }
        actual_buffer_size_ = power_of_2_size;
    }
    
    ~impl() {
        stop();
    }
    
    bool enqueue(thread_module::log_level level,
                 const std::string& message,
                 const std::string& file,
                 int line,
                 const std::string& function,
                 const std::chrono::system_clock::time_point& timestamp) {

        // Use lock-free ring buffer for high-performance enqueueing
        log_entry entry{level, message, file, line, function, timestamp};

        // Try to push to lock-free buffer first
        if (ring_buffer_.push(std::move(entry))) {
            // Notify worker thread using atomic flag
            has_work_.store(true, std::memory_order_release);
            return true;
        }

        // If lock-free buffer is full, increment dropped count
        dropped_messages_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    
    void add_writer(base_writer* writer) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        writers_.push_back(writer);
    }
    
    void clear_writers() {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        writers_.clear();
    }
    
    void start() {
        if (!running_.exchange(true)) {
            worker_thread_ = std::thread(&impl::process_loop, this);
        }
    }
    
    void stop() {
        if (running_.exchange(false)) {
            // Signal that work is available to wake up the thread
            has_work_.store(true, std::memory_order_release);
            if (worker_thread_.joinable()) {
                worker_thread_.join();
            }
        }
    }
    
    void flush() {
        flush_remaining();
    }

    std::pair<size_t, size_t> get_queue_metrics() const {
        return {ring_buffer_.size(), ring_buffer_.capacity()};
    }

    // New method to get performance metrics
    struct performance_stats {
        uint64_t processed_messages;
        uint64_t dropped_messages;
        size_t current_queue_size;
        size_t queue_capacity;
    };

    performance_stats get_performance_stats() const {
        return {
            processed_messages_.load(std::memory_order_relaxed),
            dropped_messages_.load(std::memory_order_relaxed),
            ring_buffer_.size(),
            ring_buffer_.capacity()
        };
    }
    
private:
    void process_loop() {
        std::vector<log_entry> batch;
        batch.reserve(256); // Pre-allocate for better performance

        while (running_.load(std::memory_order_acquire)) {
            // Check for work using atomic flag
            if (!has_work_.load(std::memory_order_acquire)) {
                // Brief sleep to avoid busy-waiting
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            }

            // Process entries from lock-free ring buffer
            batch.clear();
            log_entry entry;

            // Batch process up to 256 entries for efficiency
            while (batch.size() < 256 && ring_buffer_.pop(entry)) {
                batch.push_back(std::move(entry));
            }

            // Reset work flag if ring buffer is empty
            if (ring_buffer_.empty()) {
                has_work_.store(false, std::memory_order_release);
            }

            // Write batch to all writers
            if (!batch.empty()) {
                write_batch_to_all(batch);
                processed_messages_.fetch_add(batch.size(), std::memory_order_relaxed);
            }
        }

        // Process any remaining entries
        flush_remaining();
    }
    
    void write_to_all(const log_entry& entry) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        for (auto* writer : writers_) {
            // Ignore write failures for now
            writer->write(entry.level, entry.message, entry.file,
                         entry.line, entry.function, entry.timestamp);
        }
    }

    void write_batch_to_all(const std::vector<log_entry>& batch) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        for (auto* writer : writers_) {
            for (const auto& entry : batch) {
                // Ignore write failures for now
                writer->write(entry.level, entry.message, entry.file,
                             entry.line, entry.function, entry.timestamp);
            }
        }
    }

    void flush_remaining() {
        // Process all remaining entries
        log_entry entry;
        while (ring_buffer_.pop(entry)) {
            write_to_all(entry);
            processed_messages_.fetch_add(1, std::memory_order_relaxed);
        }

        // Flush all writers
        std::lock_guard<std::mutex> writer_lock(writers_mutex_);
        for (auto* writer : writers_) {
            writer->flush();
        }
    }
    
private:
    std::size_t buffer_size_;
    std::size_t actual_buffer_size_;
    std::atomic<bool> running_;
    std::thread worker_thread_;

    // Lock-free ring buffer for high-performance logging
    lockfree_ring_buffer<log_entry, 16384> ring_buffer_; // 16K entries
    std::atomic<bool> has_work_{false};

    // Performance metrics
    std::atomic<uint64_t> dropped_messages_;
    std::atomic<uint64_t> processed_messages_;

    // Writers management (still needs mutex for dynamic changes)
    std::vector<base_writer*> writers_;
    std::mutex writers_mutex_;
};

// log_collector implementation
log_collector::log_collector(std::size_t buffer_size)
    : pimpl_(std::make_unique<impl>(buffer_size)) {
}

log_collector::~log_collector() = default;

bool log_collector::enqueue(thread_module::log_level level,
                           const std::string& message,
                           const std::string& file,
                           int line,
                           const std::string& function,
                           const std::chrono::system_clock::time_point& timestamp) {
    return pimpl_->enqueue(level, message, file, line, function, timestamp);
}

void log_collector::add_writer(base_writer* writer) {
    pimpl_->add_writer(writer);
}

void log_collector::clear_writers() {
    pimpl_->clear_writers();
}

void log_collector::start() {
    pimpl_->start();
}

void log_collector::stop() {
    pimpl_->stop();
}

void log_collector::flush() {
    pimpl_->flush();
}

std::pair<size_t, size_t> log_collector::get_queue_metrics() const {
    return pimpl_->get_queue_metrics();
}

log_collector::performance_stats log_collector::get_performance_stats() const {
    auto stats = pimpl_->get_performance_stats();
    return {stats.processed_messages, stats.dropped_messages,
            stats.current_queue_size, stats.queue_capacity};
}

} // namespace logger_module