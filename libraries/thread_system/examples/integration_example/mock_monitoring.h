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

#include <kcenon/thread/interfaces/monitoring_interface.h>
#include <iostream>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>

/**
 * @brief Mock monitoring implementation for demonstration
 * 
 * In a real application, this would be replaced with:
 * #include <monitoring_system/monitoring.h>
 * using monitoring_module::monitoring;
 */
class mock_monitoring : public ::monitoring_interface::monitoring_interface {
public:
    mock_monitoring() 
        : active_(false)
        , total_collections_(0)
        , max_history_(100) {
    }
    
    void update_system_metrics(const ::monitoring_interface::system_metrics& metrics) override {
        std::lock_guard<std::mutex> lock(mutex_);
        current_snapshot_.system = metrics;
        current_snapshot_.capture_time = std::chrono::steady_clock::now();
    }
    
    void update_thread_pool_metrics(const ::monitoring_interface::thread_pool_metrics& metrics) override {
        std::lock_guard<std::mutex> lock(mutex_);
        current_snapshot_.thread_pool = metrics;
        current_snapshot_.capture_time = std::chrono::steady_clock::now();
    }
    
    void update_worker_metrics(std::size_t worker_id, const ::monitoring_interface::worker_metrics& metrics) override {
        std::lock_guard<std::mutex> lock(mutex_);
        // Simple aggregation
        current_snapshot_.worker.jobs_processed += metrics.jobs_processed;
        current_snapshot_.worker.total_processing_time_ns += metrics.total_processing_time_ns;
        current_snapshot_.capture_time = std::chrono::steady_clock::now();
    }
    
    ::monitoring_interface::metrics_snapshot get_current_snapshot() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_snapshot_;
    }
    
    std::vector<::monitoring_interface::metrics_snapshot> get_recent_snapshots(std::size_t count) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<::monitoring_interface::metrics_snapshot> result;
        size_t n = std::min(count, history_.size());
        result.reserve(n);
        
        // Return newest first
        for (auto it = history_.rbegin(); it != history_.rend() && result.size() < n; ++it) {
            result.push_back(*it);
        }
        
        return result;
    }
    
    bool is_active() const override {
        return active_.load();
    }
    
    void start() {
        if (!active_.exchange(true)) {
            std::cout << "[MockMonitoring] Started" << std::endl;
            collection_thread_ = std::thread(&mock_monitoring::collect_loop, this);
        }
    }
    
    void stop() {
        if (active_.exchange(false)) {
            if (collection_thread_.joinable()) {
                collection_thread_.join();
            }
            std::cout << "[MockMonitoring] Stopped" << std::endl;
        }
    }
    
    struct monitoring_stats {
        std::uint64_t total_collections;
    };
    
    monitoring_stats get_stats() const {
        return {total_collections_.load()};
    }
    
private:
    void collect_loop() {
        while (active_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            if (active_.load()) {
                std::lock_guard<std::mutex> lock(mutex_);
                
                // Store snapshot in history
                history_.push_back(current_snapshot_);
                if (history_.size() > max_history_) {
                    history_.pop_front();
                }
                
                total_collections_.fetch_add(1);
            }
        }
    }
    
private:
    ::monitoring_interface::metrics_snapshot current_snapshot_;
    std::deque<::monitoring_interface::metrics_snapshot> history_;
    mutable std::mutex mutex_;
    
    std::atomic<bool> active_;
    std::thread collection_thread_;
    std::atomic<std::uint64_t> total_collections_;
    const size_t max_history_;
};
