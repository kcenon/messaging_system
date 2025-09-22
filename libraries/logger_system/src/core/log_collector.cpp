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

#include <kcenon/logger/core/log_collector.h>
#include <kcenon/logger/writers/base_writer.h>
#include <kcenon/logger/interfaces/log_entry.h>
#ifdef USE_THREAD_SYSTEM_INTEGRATION
#include <kcenon/thread/interfaces/logger_interface.h>
#else
#include <kcenon/logger/interfaces/logger_interface.h>
#endif
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <vector>

namespace kcenon::logger {

class log_collector::impl {
public:
    explicit impl(std::size_t buffer_size)
        : buffer_size_(buffer_size)
        , running_(false) {
    }
    
    ~impl() {
        stop();
    }
    
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    bool enqueue(kcenon::thread::log_level level,
#else
    bool enqueue(logger_system::log_level level,
#endif
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
            
            // Create log_entry with optional source location
#ifdef USE_THREAD_SYSTEM_INTEGRATION
            // Convert kcenon::thread::log_level to logger_system::log_level
            logger_system::log_level logger_level;
            switch (level) {
                case kcenon::thread::log_level::trace: logger_level = logger_system::log_level::trace; break;
                case kcenon::thread::log_level::debug: logger_level = logger_system::log_level::debug; break;
                case kcenon::thread::log_level::info: logger_level = logger_system::log_level::info; break;
                case kcenon::thread::log_level::warning: logger_level = logger_system::log_level::warn; break;
                case kcenon::thread::log_level::error: logger_level = logger_system::log_level::error; break;
                case kcenon::thread::log_level::critical: logger_level = logger_system::log_level::fatal; break;
                default: logger_level = logger_system::log_level::info; break;
            }
#else
            // In standalone mode, no conversion needed
            logger_system::log_level logger_level = level;
#endif
            log_entry entry(logger_level, message, timestamp);
            if (!file.empty() || line != 0 || !function.empty()) {
                entry.location = source_location{file, line, function};
            }
            queue_.push(std::move(entry));
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
            std::string file = entry.location ? entry.location->file.to_string() : "";
            int line = entry.location ? entry.location->line : 0;
            std::string function = entry.location ? entry.location->function.to_string() : "";
            
            writer->write(entry.level, entry.message.to_string(), file,
                         line, function, entry.timestamp);
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

namespace {
#ifdef USE_THREAD_SYSTEM_INTEGRATION
// Convert logger_system::log_level to kcenon::thread::log_level
kcenon::thread::log_level convert_log_level(logger_system::log_level level) {
    switch (level) {
        case logger_system::log_level::trace: return kcenon::thread::log_level::trace;
        case logger_system::log_level::debug: return kcenon::thread::log_level::debug;
        case logger_system::log_level::info: return kcenon::thread::log_level::info;
        case logger_system::log_level::warn: return kcenon::thread::log_level::warning;
        case logger_system::log_level::error: return kcenon::thread::log_level::error;
        case logger_system::log_level::fatal: return kcenon::thread::log_level::critical;
        case logger_system::log_level::off: return kcenon::thread::log_level::critical; // fallback
        default: return kcenon::thread::log_level::info;
    }
}
#endif
}

bool log_collector::enqueue(logger_system::log_level level,
                           const std::string& message,
                           const std::string& file,
                           int line,
                           const std::string& function,
                           const std::chrono::system_clock::time_point& timestamp) {
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    return pimpl_->enqueue(convert_log_level(level), message, file, line, function, timestamp);
#else
    return pimpl_->enqueue(level, message, file, line, function, timestamp);
#endif
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

} // namespace kcenon::logger