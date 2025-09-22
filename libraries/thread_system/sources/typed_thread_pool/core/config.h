#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

/**
 * @file config.h
 * @brief Central configuration for typed_thread_pool module
 * 
 * This file contains compile-time configuration constants and default values
 * that can be used throughout the typed_thread_pool module.
 */

#include "job_types.h"
#include <cstddef>
#include <thread>

namespace typed_thread_pool_module::config {
    
    // Default type settings
    using default_job_type = job_types;
    
    // Performance configuration - Updated for modern hardware
    constexpr size_t default_queue_size = 4096;  // Increased for better throughput
    constexpr size_t default_worker_count = std::thread::hardware_concurrency() > 0 ?
                                          std::thread::hardware_concurrency() : 8;
    constexpr size_t max_workers = 128;  // Increased for high-core systems
    constexpr size_t min_workers = 1;

    // Queue behavior settings - Optimized timing
    constexpr size_t default_wait_timeout_ms = 50;   // Reduced for better responsiveness
    constexpr size_t default_shutdown_timeout_ms = 3000;  // Reduced for faster shutdown

    // Feature flags - Enhanced capabilities
    constexpr bool enable_statistics = true;
    constexpr bool enable_priority_boost = true;  // Enabled for dynamic priority adjustment
    constexpr bool enable_work_stealing = true;
    constexpr bool enable_adaptive_sizing = true;  // Enabled for dynamic worker management

    // Memory management - Improved pooling
    constexpr size_t default_job_pool_size = 2048;  // Increased pool size
    constexpr bool enable_job_recycling = true;

    // Debugging and monitoring - Enhanced monitoring
    constexpr bool enable_debug_logging = false;
    constexpr bool enable_performance_monitoring = true;  // Enabled for production insights
    constexpr size_t monitoring_interval_ms = 500;  // Reduced for better granularity

    // New adaptive configuration
    constexpr size_t adaptive_evaluation_interval_ms = 2000;
    constexpr double cpu_utilization_threshold_high = 0.8;
    constexpr double cpu_utilization_threshold_low = 0.3;
    constexpr size_t min_idle_time_before_scale_down_ms = 5000;
    
    /**
     * @brief Validates configuration values at compile time
     */
    static_assert(default_queue_size > 0, "Queue size must be positive");
    static_assert(default_worker_count >= min_workers, "Worker count must be at least minimum");
    static_assert(default_worker_count <= max_workers, "Worker count must not exceed maximum");
    static_assert(default_wait_timeout_ms > 0, "Wait timeout must be positive");
    static_assert(default_shutdown_timeout_ms > 0, "Shutdown timeout must be positive");
    
} // namespace typed_thread_pool_module::config