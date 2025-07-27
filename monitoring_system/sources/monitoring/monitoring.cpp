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

#include "monitoring.h"
#include "storage/ring_buffer.h"
#include <mutex>
#include <condition_variable>
#include <unordered_map>

namespace monitoring_module {

class monitoring::impl {
public:
    impl(std::size_t history_size, std::uint32_t collection_interval_ms)
        : history_(history_size)
        , active_(false)
        , collection_interval_ms_(collection_interval_ms)
        , total_collections_(0)
        , dropped_snapshots_(0)
        , collector_errors_(0)
        , start_time_(std::chrono::steady_clock::now()) {
    }
    
    ~impl() {
        stop();
    }
    
    void start() {
        if (!active_.exchange(true)) {
            collection_thread_ = std::thread(&impl::collection_loop, this);
        }
    }
    
    void stop() {
        if (active_.exchange(false)) {
            cv_.notify_all();
            if (collection_thread_.joinable()) {
                collection_thread_.join();
            }
        }
    }
    
    void update_system_metrics(const system_metrics& metrics) {
        std::lock_guard<std::mutex> lock(current_mutex_);
        current_snapshot_.system = metrics;
        current_snapshot_.capture_time = std::chrono::steady_clock::now();
    }
    
    void update_thread_pool_metrics(const thread_pool_metrics& metrics) {
        std::lock_guard<std::mutex> lock(current_mutex_);
        current_snapshot_.thread_pool = metrics;
        current_snapshot_.capture_time = std::chrono::steady_clock::now();
    }
    
    void update_worker_metrics(std::size_t worker_id, const worker_metrics& metrics) {
        std::lock_guard<std::mutex> lock(current_mutex_);
        worker_metrics_[worker_id] = metrics;
        
        // Aggregate worker metrics
        worker_metrics aggregated{};
        for (const auto& [id, m] : worker_metrics_) {
            aggregated.jobs_processed += m.jobs_processed;
            aggregated.total_processing_time_ns += m.total_processing_time_ns;
            aggregated.idle_time_ns += m.idle_time_ns;
            aggregated.context_switches += m.context_switches;
        }
        current_snapshot_.worker = aggregated;
        current_snapshot_.capture_time = std::chrono::steady_clock::now();
    }
    
    metrics_snapshot get_current_snapshot() const {
        std::lock_guard<std::mutex> lock(current_mutex_);
        return current_snapshot_;
    }
    
    std::vector<metrics_snapshot> get_recent_snapshots(std::size_t count) const {
        return history_.get_recent(count);
    }
    
    bool is_active() const {
        return active_.load();
    }
    
    void add_collector(std::unique_ptr<metrics_collector> collector) {
        std::lock_guard<std::mutex> lock(collectors_mutex_);
        collectors_.push_back(std::move(collector));
    }
    
    void clear_collectors() {
        std::lock_guard<std::mutex> lock(collectors_mutex_);
        collectors_.clear();
    }
    
    void set_collection_interval(std::uint32_t interval_ms) {
        collection_interval_ms_.store(interval_ms);
        cv_.notify_all();
    }
    
    std::uint32_t get_collection_interval() const {
        return collection_interval_ms_.load();
    }
    
    void collect_now() {
        collect_metrics();
    }
    
    void clear_history() {
        history_.clear();
    }
    
    monitoring_stats get_stats() const {
        return monitoring_stats{
            total_collections_.load(),
            dropped_snapshots_.load(),
            collector_errors_.load(),
            start_time_
        };
    }
    
private:
    void collection_loop() {
        while (active_.load()) {
            std::unique_lock<std::mutex> lock(cv_mutex_);
            
            auto interval = std::chrono::milliseconds(collection_interval_ms_.load());
            cv_.wait_for(lock, interval, [this] { return !active_.load(); });
            
            if (active_.load()) {
                collect_metrics();
            }
        }
    }
    
    void collect_metrics() {
        metrics_snapshot snapshot;
        
        // Get current metrics
        {
            std::lock_guard<std::mutex> lock(current_mutex_);
            snapshot = current_snapshot_;
        }
        
        // Run custom collectors
        {
            std::lock_guard<std::mutex> lock(collectors_mutex_);
            for (auto& collector : collectors_) {
                try {
                    collector->collect(snapshot);
                } catch (...) {
                    collector_errors_.fetch_add(1);
                }
            }
        }
        
        // Store in history
        if (!history_.push(snapshot)) {
            dropped_snapshots_.fetch_add(1);
        }
        
        total_collections_.fetch_add(1);
    }
    
private:
    ring_buffer<metrics_snapshot> history_;
    metrics_snapshot current_snapshot_;
    mutable std::mutex current_mutex_;
    
    std::unordered_map<std::size_t, worker_metrics> worker_metrics_;
    
    std::vector<std::unique_ptr<metrics_collector>> collectors_;
    std::mutex collectors_mutex_;
    
    std::atomic<bool> active_;
    std::atomic<std::uint32_t> collection_interval_ms_;
    
    std::thread collection_thread_;
    std::condition_variable cv_;
    std::mutex cv_mutex_;
    
    // Statistics
    std::atomic<std::uint64_t> total_collections_;
    std::atomic<std::uint64_t> dropped_snapshots_;
    std::atomic<std::uint64_t> collector_errors_;
    std::chrono::steady_clock::time_point start_time_;
};

// monitoring implementation
monitoring::monitoring(std::size_t history_size, std::uint32_t collection_interval_ms)
    : pimpl_(std::make_unique<impl>(history_size, collection_interval_ms)) {
}

monitoring::~monitoring() = default;

void monitoring::update_system_metrics(const system_metrics& metrics) {
    pimpl_->update_system_metrics(metrics);
}

void monitoring::update_thread_pool_metrics(const thread_pool_metrics& metrics) {
    pimpl_->update_thread_pool_metrics(metrics);
}

void monitoring::update_worker_metrics(std::size_t worker_id, const worker_metrics& metrics) {
    pimpl_->update_worker_metrics(worker_id, metrics);
}

metrics_snapshot monitoring::get_current_snapshot() const {
    return pimpl_->get_current_snapshot();
}

std::vector<metrics_snapshot> monitoring::get_recent_snapshots(std::size_t count) const {
    return pimpl_->get_recent_snapshots(count);
}

bool monitoring::is_active() const {
    return pimpl_->is_active();
}

void monitoring::start() {
    pimpl_->start();
}

void monitoring::stop() {
    pimpl_->stop();
}

void monitoring::add_collector(std::unique_ptr<metrics_collector> collector) {
    pimpl_->add_collector(std::move(collector));
}

void monitoring::clear_collectors() {
    pimpl_->clear_collectors();
}

void monitoring::set_collection_interval(std::uint32_t interval_ms) {
    pimpl_->set_collection_interval(interval_ms);
}

std::uint32_t monitoring::get_collection_interval() const {
    return pimpl_->get_collection_interval();
}

void monitoring::collect_now() {
    pimpl_->collect_now();
}

void monitoring::clear_history() {
    pimpl_->clear_history();
}

monitoring::monitoring_stats monitoring::get_stats() const {
    return pimpl_->get_stats();
}

} // namespace monitoring_module