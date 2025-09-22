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

#include <chrono>
#include <atomic>
#include <memory>
#include <vector>
#include <cstdint>
#include <string>

namespace monitoring_interface {

    // Forward declarations for multi-process support
    struct process_identifier;
    struct thread_pool_identifier;
    struct process_thread_pool_metrics;
    struct multi_process_metrics_snapshot;
    class multi_process_monitoring_interface;

    /**
     * @struct system_metrics
     * @brief System-level performance metrics interface.
     */
    struct system_metrics {
        std::uint64_t cpu_usage_percent{0};
        std::uint64_t memory_usage_bytes{0};
        std::uint64_t active_threads{0};
        std::uint64_t total_allocations{0};
        std::chrono::steady_clock::time_point timestamp;
        
        virtual ~system_metrics() = default;
    };

    /**
     * @struct thread_pool_metrics
     * @brief Thread pool performance metrics interface.
     */
    struct thread_pool_metrics {
        std::uint64_t jobs_completed{0};
        std::uint64_t jobs_pending{0};
        std::uint64_t total_execution_time_ns{0};
        std::uint64_t average_latency_ns{0};
        std::uint64_t worker_threads{0};
        std::uint64_t idle_threads{0};
        std::chrono::steady_clock::time_point timestamp;
        
        // Multi-process support fields
        std::string pool_name;               // Pool identifier name
        std::uint32_t pool_instance_id{0};   // Instance ID for multiple pools
        
        virtual ~thread_pool_metrics() = default;
    };

    /**
     * @struct worker_metrics
     * @brief Worker thread performance metrics interface.
     */
    struct worker_metrics {
        std::uint64_t jobs_processed{0};
        std::uint64_t total_processing_time_ns{0};
        std::uint64_t idle_time_ns{0};
        std::uint64_t context_switches{0};
        std::chrono::steady_clock::time_point timestamp;
        
        virtual ~worker_metrics() = default;
    };

    /**
     * @struct metrics_snapshot
     * @brief Complete snapshot of all metrics at a point in time.
     */
    struct metrics_snapshot {
        system_metrics system;
        thread_pool_metrics thread_pool;
        worker_metrics worker;
        std::chrono::steady_clock::time_point capture_time;
        
        virtual ~metrics_snapshot() = default;
    };

    /**
     * @class monitoring_interface
     * @brief Abstract interface for monitoring system.
     * 
     * This interface allows the thread system to report metrics
     * without depending on a specific monitoring implementation.
     */
    class monitoring_interface {
    public:
        virtual ~monitoring_interface() = default;

        /**
         * @brief Updates system metrics.
         * @param metrics The system metrics to record
         */
        virtual void update_system_metrics(const system_metrics& metrics) = 0;

        /**
         * @brief Updates thread pool metrics.
         * @param metrics The thread pool metrics to record
         */
        virtual void update_thread_pool_metrics(const thread_pool_metrics& metrics) = 0;

        /**
         * @brief Updates thread pool metrics with pool identifier (for multi-pool scenarios).
         * @param pool_name Name of the thread pool
         * @param pool_instance_id Instance ID for multiple pools with same name
         * @param metrics The thread pool metrics to record
         */
        virtual void update_thread_pool_metrics(const std::string& pool_name, 
                                              std::uint32_t pool_instance_id,
                                              const thread_pool_metrics& metrics) {
            // Default implementation: copy identifiers and forward to original method
            thread_pool_metrics identified_metrics = metrics;
            identified_metrics.pool_name = pool_name;
            identified_metrics.pool_instance_id = pool_instance_id;
            update_thread_pool_metrics(identified_metrics);
        }

        /**
         * @brief Updates worker thread metrics.
         * @param worker_id Unique identifier for the worker
         * @param metrics The worker metrics to record
         */
        virtual void update_worker_metrics(std::size_t worker_id, const worker_metrics& metrics) = 0;

        /**
         * @brief Retrieves the most recent metrics snapshot.
         * @return Current metrics snapshot
         */
        virtual metrics_snapshot get_current_snapshot() const = 0;

        /**
         * @brief Retrieves multiple recent metrics snapshots.
         * @param count Number of snapshots to retrieve
         * @return Vector of metrics snapshots, newest first
         */
        virtual std::vector<metrics_snapshot> get_recent_snapshots(std::size_t count) const = 0;

        /**
         * @brief Checks if monitoring is currently active.
         * @return true if monitoring is active
         */
        virtual bool is_active() const = 0;
    };

    /**
     * @class null_monitoring
     * @brief No-op implementation of monitoring interface.
     * 
     * Used when monitoring is disabled or not configured.
     */
    class null_monitoring : public monitoring_interface {
    public:
        void update_system_metrics(const system_metrics&) override {}
        void update_thread_pool_metrics(const thread_pool_metrics&) override {}
        void update_worker_metrics(std::size_t, const worker_metrics&) override {}
        
        metrics_snapshot get_current_snapshot() const override {
            return metrics_snapshot{};
        }
        
        std::vector<metrics_snapshot> get_recent_snapshots(std::size_t) const override {
            return {};
        }
        
        bool is_active() const override { return false; }
    };

    /**
     * @class scoped_timer
     * @brief RAII timer for measuring operation duration.
     * 
     * This is a simplified version that works with the interface.
     */
    class scoped_timer {
    public:
        explicit scoped_timer(std::atomic<std::uint64_t>& target)
            : target_(target), start_time_(std::chrono::steady_clock::now()) {}

        ~scoped_timer() {
            auto end_time = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                end_time - start_time_).count();
            target_.fetch_add(static_cast<std::uint64_t>(duration));
        }

        scoped_timer(const scoped_timer&) = delete;
        scoped_timer& operator=(const scoped_timer&) = delete;
        scoped_timer(scoped_timer&&) = delete;
        scoped_timer& operator=(scoped_timer&&) = delete;

    private:
        std::atomic<std::uint64_t>& target_;
        std::chrono::steady_clock::time_point start_time_;
    };

} // namespace monitoring_interface