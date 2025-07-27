#pragma once

/*
 * BSD 3-Clause License
 * 
 * Copyright (c) 2024, DongCheol Shin
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file worker_policy.h
 * @brief Worker behavior policies and configuration
 * 
 * This file defines policies that control how worker threads behave,
 * including scheduling strategies, idle handling, and work stealing.
 */

#include "../detail/forward_declarations.h"
#include "../core/config.h"
#include <chrono>
#include <string>

namespace thread_pool_module {
    
    /**
     * @brief Enumeration of worker states
     */
    enum class worker_state {
        created,     ///< Worker created but not started
        starting,    ///< Worker is starting up
        active,      ///< Worker is actively processing jobs
        idle,        ///< Worker is idle, waiting for work
        stopping,    ///< Worker is shutting down
        stopped      ///< Worker has stopped
    };
    
    /**
     * @brief Enumeration of scheduling policies
     */
    enum class scheduling_policy {
        fifo,        ///< First-in, first-out scheduling
        lifo,        ///< Last-in, first-out scheduling
        priority,    ///< Priority-based scheduling
        work_stealing ///< Work-stealing scheduling
    };
    
    /**
     * @brief Worker behavior policy configuration
     * 
     * This structure defines how a worker thread should behave
     * in various situations and under different load conditions.
     */
    struct worker_policy {
        // Scheduling behavior
        scheduling_policy scheduling = scheduling_policy::fifo;
        
        // Idle behavior
        std::chrono::milliseconds idle_timeout = config::default_worker_idle_timeout;
        bool yield_on_idle = config::default_yield_on_idle;
        bool sleep_when_idle = true;
        std::chrono::microseconds idle_sleep_duration{100};
        
        // Work stealing behavior
        bool enable_work_stealing = config::default_work_stealing;
        size_t max_steal_attempts = 3;
        std::chrono::microseconds steal_backoff{50};
        
        // Performance behavior
        bool pin_to_cpu = config::default_pin_threads;
        int preferred_cpu = -1; // -1 means no preference
        size_t max_jobs_per_batch = 10;
        
        // Error handling
        bool continue_on_exception = true;
        size_t max_consecutive_failures = 5;
        
        // Debugging and monitoring
        bool enable_statistics = config::enable_statistics;
        std::string worker_name_prefix = config::default_thread_prefix;
        
        /**
         * @brief Create a default worker policy
         */
        static worker_policy default_policy() {
            return worker_policy{};
        }
        
        /**
         * @brief Create a high-performance worker policy
         */
        static worker_policy high_performance() {
            worker_policy policy;
            policy.yield_on_idle = false;
            policy.sleep_when_idle = false;
            policy.enable_work_stealing = true;
            policy.max_jobs_per_batch = 20;
            return policy;
        }
        
        /**
         * @brief Create a low-latency worker policy
         */
        static worker_policy low_latency() {
            worker_policy policy;
            policy.scheduling = scheduling_policy::priority;
            policy.yield_on_idle = false;
            policy.idle_sleep_duration = std::chrono::microseconds{10};
            policy.max_jobs_per_batch = 1;
            return policy;
        }
        
        /**
         * @brief Create a power-efficient worker policy
         */
        static worker_policy power_efficient() {
            worker_policy policy;
            policy.yield_on_idle = true;
            policy.sleep_when_idle = true;
            policy.idle_sleep_duration = std::chrono::milliseconds{1};
            policy.enable_work_stealing = false;
            return policy;
        }
    };
    
    /**
     * @brief Convert worker state to string representation
     */
    constexpr const char* to_string(worker_state state) {
        switch (state) {
            case worker_state::created: return "created";
            case worker_state::starting: return "starting";
            case worker_state::active: return "active";
            case worker_state::idle: return "idle";
            case worker_state::stopping: return "stopping";
            case worker_state::stopped: return "stopped";
            default: return "unknown";
        }
    }
    
    /**
     * @brief Convert scheduling policy to string representation
     */
    constexpr const char* to_string(scheduling_policy policy) {
        switch (policy) {
            case scheduling_policy::fifo: return "fifo";
            case scheduling_policy::lifo: return "lifo";
            case scheduling_policy::priority: return "priority";
            case scheduling_policy::work_stealing: return "work_stealing";
            default: return "unknown";
        }
    }
    
} // namespace thread_pool_module