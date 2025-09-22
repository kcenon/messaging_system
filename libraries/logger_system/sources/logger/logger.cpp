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

#include "logger.h"
#include "core/log_collector.h"
#include "writers/base_writer.h"
#include "filters/log_filter.h"
#include "routing/log_router.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <mutex>

namespace logger_module {

// Implementation class
class logger::impl {
public:
    impl(bool async, std::size_t buffer_size)
        : async_(async)
        , buffer_size_(buffer_size)
        , min_level_(thread_module::log_level::trace)
        , running_(false)
        , router_(std::make_unique<log_router>())
        , metrics_enabled_(false) {
        if (async_) {
            collector_ = std::make_unique<log_collector>(buffer_size_);
        }
    }
    
    ~impl() {
        stop();
    }
    
    void log(thread_module::log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) {
        if (level > min_level_.load(std::memory_order_acquire)) {
            return;
        }
        
        // Apply global filter
        if (global_filter_ && !global_filter_->should_log(level, message, file, line, function)) {
            return;
        }
        
        auto timestamp = std::chrono::system_clock::now();
        
        // Record metrics if enabled
        if (metrics_enabled_ && metrics_collector_) {
            auto start = std::chrono::high_resolution_clock::now();
            
            if (async_ && collector_) {
                // Async logging through collector
                bool enqueued = collector_->enqueue(level, message, file, line, function, timestamp);
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                
                if (enqueued) {
                    metrics_collector_->record_enqueue(message.size(), duration);
                } else {
                    metrics_collector_->record_drop();
                }
                
                // Update queue metrics
                auto [current, max] = collector_->get_queue_metrics();
                metrics_collector_->update_queue_size(current, max);
            } else {
                // Sync logging directly to writers
                std::lock_guard<std::mutex> lock(writers_mutex_);
                
                // Get routed writers
                auto routed_writers = router_->route(level, message, file, line, function, timestamp, named_writers_);
                
                for (auto* writer : routed_writers) {
                    auto write_start = std::chrono::high_resolution_clock::now();
                    bool success = writer->write(level, message, file, line, function, timestamp);
                    auto write_end = std::chrono::high_resolution_clock::now();
                    auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(write_end - write_start);
                    
                    metrics_collector_->record_write(writer->get_name(), message.size(), write_duration, success);
                }
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
                metrics_collector_->record_enqueue(message.size(), duration);
            }
            
            metrics_collector_->record_processed(message.size());
        } else {
            // Normal logging without metrics
            if (async_ && collector_) {
                collector_->enqueue(level, message, file, line, function, timestamp);
            } else {
                std::lock_guard<std::mutex> lock(writers_mutex_);
                
                // Get routed writers
                auto routed_writers = router_->route(level, message, file, line, function, timestamp, named_writers_);
                
                for (auto* writer : routed_writers) {
                    writer->write(level, message, file, line, function, timestamp);
                }
            }
        }
    }
    
    void flush() {
        if (async_ && collector_) {
            collector_->flush();
        }
        
        std::lock_guard<std::mutex> lock(writers_mutex_);
        for (auto& [name, writer] : named_writers_) {
            writer->flush();
        }
    }
    
    void add_writer(std::unique_ptr<base_writer> writer) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        
        // Generate a name if not using named_writers
        std::string name = writer->get_name() + "_" + std::to_string(named_writers_.size());
        
        if (async_ && collector_) {
            collector_->add_writer(writer.get());
        }
        
        // Keep in both collections for backward compatibility
        writers_.push_back(writer.get());
        named_writers_[name] = std::move(writer);
    }
    
    void add_writer(const std::string& name, std::unique_ptr<base_writer> writer) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        if (async_ && collector_) {
            collector_->add_writer(writer.get());
        }
        writers_.push_back(writer.get());
        named_writers_[name] = std::move(writer);
    }
    
    void clear_writers() {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        if (async_ && collector_) {
            collector_->clear_writers();
        }
        writers_.clear();
        named_writers_.clear();
    }
    
    void start() {
        if (async_ && collector_ && !running_.exchange(true)) {
            collector_->start();
        }
    }
    
    void stop() {
        if (running_.exchange(false)) {
            if (async_ && collector_) {
                collector_->stop();
            }
            flush();
        }
    }
    
    bool is_running() const {
        return running_.load();
    }
    
    void set_min_level(thread_module::log_level level) {
        min_level_.store(level, std::memory_order_release);
    }
    
    thread_module::log_level get_min_level() const {
        return min_level_.load(std::memory_order_acquire);
    }
    
    void enable_metrics_collection(bool enable) {
        if (enable && !metrics_collector_) {
            metrics_collector_ = std::make_unique<logger_metrics_collector>();
        }
        metrics_enabled_ = enable;
    }
    
    bool is_metrics_collection_enabled() const {
        return metrics_enabled_;
    }
    
    performance_metrics get_current_metrics() const {
        if (metrics_collector_) {
            return metrics_collector_->get_snapshot();
        }
        return performance_metrics{};
    }
    
    std::unique_ptr<performance_metrics> get_metrics_history(std::chrono::seconds duration) const {
        (void)duration;  // For Phase 1, we'll return just the current snapshot
        // For Phase 1, we'll return just the current snapshot
        // In a future phase, we could implement a time-series storage
        if (metrics_collector_) {
            return std::make_unique<performance_metrics>(metrics_collector_->get_snapshot());
        }
        return nullptr;
    }
    
    void reset_metrics() {
        if (metrics_collector_) {
            metrics_collector_->reset();
        }
    }
    
    logger_metrics_collector* get_metrics_collector() {
        return metrics_collector_.get();
    }
    
    bool remove_writer(const std::string& name) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        auto it = named_writers_.find(name);
        if (it != named_writers_.end()) {
            if (async_ && collector_) {
                // Note: log_collector doesn't support individual writer removal
                // This is a limitation of the current design
            }
            named_writers_.erase(it);
            return true;
        }
        return false;
    }
    
    base_writer* get_writer(const std::string& name) {
        std::lock_guard<std::mutex> lock(writers_mutex_);
        auto it = named_writers_.find(name);
        return it != named_writers_.end() ? it->second.get() : nullptr;
    }
    
    void set_filter(std::unique_ptr<log_filter> filter) {
        global_filter_ = std::move(filter);
    }
    
    log_router& get_router() {
        return *router_;
    }
    
private:
    bool async_;
    std::size_t buffer_size_;
    std::atomic<thread_module::log_level> min_level_;
    std::atomic<bool> running_;
    std::unique_ptr<log_collector> collector_;
    std::vector<base_writer*> writers_;  // For backward compatibility
    std::unordered_map<std::string, std::unique_ptr<base_writer>> named_writers_;
    mutable std::mutex writers_mutex_;
    
    // Filtering and routing
    std::unique_ptr<log_filter> global_filter_;
    std::unique_ptr<log_router> router_;
    
    // Metrics collection
    bool metrics_enabled_;
    std::unique_ptr<logger_metrics_collector> metrics_collector_;
};

// Logger implementation
logger::logger(bool async, std::size_t buffer_size)
    : pimpl_(std::make_unique<impl>(async, buffer_size)) {
}

logger::~logger() = default;

void logger::log(thread_module::log_level level, const std::string& message) {
    pimpl_->log(level, message, "", 0, "");
}

void logger::log(thread_module::log_level level, const std::string& message,
                 const std::string& file, int line, const std::string& function) {
    pimpl_->log(level, message, file, line, function);
}

bool logger::is_enabled(thread_module::log_level level) const {
    return level <= pimpl_->get_min_level();
}

void logger::flush() {
    pimpl_->flush();
}

void logger::add_writer(std::unique_ptr<base_writer> writer) {
    pimpl_->add_writer(std::move(writer));
}

void logger::clear_writers() {
    pimpl_->clear_writers();
}

void logger::set_min_level(thread_module::log_level level) {
    pimpl_->set_min_level(level);
}

thread_module::log_level logger::get_min_level() const {
    return pimpl_->get_min_level();
}

void logger::start() {
    pimpl_->start();
}

void logger::stop() {
    pimpl_->stop();
}

bool logger::is_running() const {
    return pimpl_->is_running();
}

void logger::enable_metrics_collection(bool enable) {
    pimpl_->enable_metrics_collection(enable);
}

bool logger::is_metrics_collection_enabled() const {
    return pimpl_->is_metrics_collection_enabled();
}

performance_metrics logger::get_current_metrics() const {
    return pimpl_->get_current_metrics();
}

std::unique_ptr<performance_metrics> logger::get_metrics_history(std::chrono::seconds duration) const {
    return pimpl_->get_metrics_history(duration);
}

void logger::reset_metrics() {
    pimpl_->reset_metrics();
}

logger_metrics_collector* logger::get_metrics_collector() {
    return pimpl_->get_metrics_collector();
}

void logger::add_writer(const std::string& name, std::unique_ptr<base_writer> writer) {
    pimpl_->add_writer(name, std::move(writer));
}

bool logger::remove_writer(const std::string& name) {
    return pimpl_->remove_writer(name);
}

base_writer* logger::get_writer(const std::string& name) {
    return pimpl_->get_writer(name);
}

void logger::set_filter(std::unique_ptr<log_filter> filter) {
    pimpl_->set_filter(std::move(filter));
}

log_router& logger::get_router() {
    return pimpl_->get_router();
}

} // namespace logger_module