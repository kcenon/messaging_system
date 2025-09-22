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

#include <memory>
#include <atomic>
#include <thread>

// Import interface from local copy
#include "monitoring_interface.h"

namespace monitoring_module {

// Re-export types from monitoring_interface for convenience
using monitoring_interface::system_metrics;
using monitoring_interface::thread_pool_metrics;
using monitoring_interface::worker_metrics;
using monitoring_interface::metrics_snapshot;

// Forward declarations
class metrics_collector;

/**
 * @brief Main monitoring implementation that implements thread_system's monitoring_interface
 * 
 * This monitoring system provides:
 * - Real-time metrics collection
 * - Historical data storage in ring buffer
 * - Low-overhead operation
 * - Thread-safe metrics updates
 * - Extensible collector system
 */
class monitoring : public monitoring_interface::monitoring_interface {
public:
    /**
     * @brief Constructor with configuration
     * @param history_size Number of historical snapshots to keep (default: 1000)
     * @param collection_interval_ms Interval between automatic collections (default: 1000ms)
     */
    explicit monitoring(std::size_t history_size = 1000,
                       std::uint32_t collection_interval_ms = 1000);
    
    /**
     * @brief Destructor
     */
    ~monitoring() override;
    
    // Implement monitoring_interface
    void update_system_metrics(const system_metrics& metrics) override;
    void update_thread_pool_metrics(const thread_pool_metrics& metrics) override;
    void update_worker_metrics(std::size_t worker_id, const worker_metrics& metrics) override;
    metrics_snapshot get_current_snapshot() const override;
    std::vector<metrics_snapshot> get_recent_snapshots(std::size_t count) const override;
    bool is_active() const override;
    
    // Additional monitoring-specific methods
    
    /**
     * @brief Start the monitoring system
     */
    void start();
    
    /**
     * @brief Stop the monitoring system
     */
    void stop();
    
    /**
     * @brief Add a custom metrics collector
     * @param collector Unique pointer to the collector
     */
    void add_collector(std::unique_ptr<metrics_collector> collector);
    
    /**
     * @brief Clear all custom collectors
     */
    void clear_collectors();
    
    /**
     * @brief Set collection interval
     * @param interval_ms Interval in milliseconds
     */
    void set_collection_interval(std::uint32_t interval_ms);
    
    /**
     * @brief Get collection interval
     * @return Current interval in milliseconds
     */
    std::uint32_t get_collection_interval() const;
    
    /**
     * @brief Force a metrics collection cycle
     */
    void collect_now();
    
    /**
     * @brief Clear all historical data
     */
    void clear_history();
    
    /**
     * @brief Get statistics about the monitoring system itself
     * @return Monitoring system statistics
     */
    struct monitoring_stats {
        std::uint64_t total_collections;
        std::uint64_t dropped_snapshots;
        std::uint64_t collector_errors;
        std::chrono::steady_clock::time_point start_time;
    };
    monitoring_stats get_stats() const;
    
private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

/**
 * @brief Base class for custom metrics collectors
 */
class metrics_collector {
public:
    virtual ~metrics_collector() = default;
    
    /**
     * @brief Collect metrics and update the snapshot
     * @param snapshot Snapshot to update with collected metrics
     */
    virtual void collect(metrics_snapshot& snapshot) = 0;
    
    /**
     * @brief Get collector name for debugging
     * @return Collector name
     */
    virtual std::string name() const = 0;
};

} // namespace monitoring_module