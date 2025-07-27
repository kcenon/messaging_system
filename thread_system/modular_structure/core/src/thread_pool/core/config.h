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
 * @brief Central configuration for thread_pool module
 * 
 * This file contains compile-time configuration constants and default values
 * that can be used throughout the thread_pool module.
 */

#include <cstddef>
#include <chrono>

namespace thread_pool_module::config {
    
    // Thread management configuration
    constexpr size_t default_thread_count = 4;
    constexpr size_t max_threads = 64;
    constexpr size_t min_threads = 1;
    
    // Queue configuration
    constexpr size_t default_queue_size = 1024;
    constexpr size_t unlimited_queue_size = 0;
    
    // Timing configuration
    constexpr auto default_wake_interval = std::chrono::milliseconds(100);
    constexpr auto default_shutdown_timeout = std::chrono::seconds(5);
    constexpr auto default_worker_idle_timeout = std::chrono::seconds(30);
    
    // Performance configuration
    constexpr bool default_yield_on_idle = true;
    constexpr bool default_work_stealing = false;
    constexpr bool default_pin_threads = false;
    constexpr bool default_use_priorities = false;
    
    // Resource limits
    constexpr size_t max_queue_size = 1024 * 1024; // 1M jobs max
    constexpr size_t default_stack_size = 1024 * 1024; // 1MB stack
    
    // Feature flags
    constexpr bool enable_coroutines = __cplusplus >= 202002L;
    constexpr bool enable_statistics = true;
    constexpr bool enable_debugging = false;
    
    // Thread naming
    constexpr const char* default_thread_prefix = "worker";
    constexpr const char* default_pool_name = "thread_pool";
    
    /**
     * @brief Validates configuration values at compile time
     */
    static_assert(default_thread_count >= min_threads, "Thread count must be at least minimum");
    static_assert(default_thread_count <= max_threads, "Thread count must not exceed maximum");
    static_assert(default_queue_size > 0 || default_queue_size == unlimited_queue_size, 
                  "Queue size must be positive or unlimited");
    static_assert(max_queue_size > default_queue_size, "Max queue size must exceed default");
    
} // namespace thread_pool_module::config