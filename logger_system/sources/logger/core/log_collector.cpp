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
        , running_(false) {
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
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Check if queue is full
            if (queue_.size() >= buffer_size_) {
                // Drop the message
                return false;
            }
            
            queue_.push({level, message, file, line, function, timestamp});
        }
        
        queue_cv_.notify_one();
        return true;
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
            queue_cv_.notify_all();
            if (worker_thread_.joinable()) {
                worker_thread_.join();
            }
        }
    }
    
    void flush() {
        // Process all remaining entries
        std::unique_lock<std::mutex> lock(queue_mutex_);
        while (!queue_.empty()) {
            auto entry = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            
            write_to_all(entry);
            
            lock.lock();
        }
        
        // Flush all writers
        std::lock_guard<std::mutex> writer_lock(writers_mutex_);
        for (auto* writer : writers_) {
            writer->flush();
        }
    }
    
    std::pair<size_t, size_t> get_queue_metrics() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return {queue_.size(), buffer_size_};
    }
    
private:
    void process_loop() {
        while (running_.load()) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            // Wait for entries or shutdown
            queue_cv_.wait(lock, [this] {
                return !queue_.empty() || !running_.load();
            });
            
            // Process batch of entries
            std::vector<log_entry> batch;
            while (!queue_.empty() && batch.size() < 100) {
                batch.push_back(std::move(queue_.front()));
                queue_.pop();
            }
            lock.unlock();
            
            // Write batch to all writers
            for (const auto& entry : batch) {
                write_to_all(entry);
            }
        }
        
        // Process any remaining entries
        flush();
    }
    
    void write_to_all(const log_entry& entry) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        for (auto* writer : writers_) {
            // Ignore write failures for now
            writer->write(entry.level, entry.message, entry.file,
                         entry.line, entry.function, entry.timestamp);
        }
    }
    
private:
    std::size_t buffer_size_;
    std::atomic<bool> running_;
    std::thread worker_thread_;
    
    std::queue<log_entry> queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    
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

} // namespace logger_module