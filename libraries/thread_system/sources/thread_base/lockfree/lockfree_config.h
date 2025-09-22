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

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace thread_module::lockfree {
    /**
     * @brief Configuration constants for lock-free data structures
     */
    
    /// Cache line size for alignment (typical for modern CPUs)
    constexpr size_t CACHE_LINE_SIZE = 64;
    
    /// Default number of hazard pointers per thread
    constexpr size_t DEFAULT_HAZARD_POINTER_COUNT = 8;
    
    /// Default size for node pools
    constexpr size_t DEFAULT_NODE_POOL_SIZE = 1024;
    
    /// Maximum number of threads supported by hazard pointer system
    constexpr size_t MAX_THREAD_COUNT = 256;
    
    /// Backoff delay constants for lock-free algorithms (in CPU cycles)
    constexpr uint32_t MIN_BACKOFF_DELAY = 1;
    constexpr uint32_t MAX_BACKOFF_DELAY = 1024;
    
    /// Retry limits for CAS operations
    constexpr uint32_t MAX_CAS_RETRIES = 100;
    
    /// Performance monitoring thresholds
    constexpr uint64_t HIGH_CONTENTION_THRESHOLD = 1000;   ///< Operations per second indicating high contention
    constexpr uint64_t LOW_CONTENTION_THRESHOLD = 100;     ///< Operations per second indicating low contention
    constexpr uint32_t ADAPTATION_WINDOW_SIZE = 10000;     ///< Number of operations in adaptation window

    /**
     * @brief Performance statistics for lock-free operations
     * 
     * This structure tracks performance metrics that can be used
     * for adaptive behavior in lock-free data structures.
     */
    struct performance_stats {
        /// Total number of operations performed
        std::atomic<uint64_t> operations_count{0};
        
        /// Number of times contention was detected
        std::atomic<uint64_t> contention_count{0};
        
        /// Number of times the algorithm had to adapt/retry
        std::atomic<uint64_t> adaptation_count{0};
        
        /// Number of successful CAS operations
        std::atomic<uint64_t> successful_cas_count{0};
        
        /// Number of failed CAS operations
        std::atomic<uint64_t> failed_cas_count{0};
        
        /// Total time spent in backoff (nanoseconds)
        std::atomic<uint64_t> backoff_time_ns{0};

        /**
         * @brief Calculate contention ratio
         * @return Ratio of contended operations to total operations (0.0 to 1.0)
         */
        [[nodiscard]] double contention_ratio() const noexcept {
            const auto ops = operations_count.load(std::memory_order_relaxed);
            const auto contention = contention_count.load(std::memory_order_relaxed);
            return ops > 0 ? static_cast<double>(contention) / ops : 0.0;
        }

        /**
         * @brief Calculate CAS success ratio
         * @return Ratio of successful CAS to total CAS operations (0.0 to 1.0)
         */
        [[nodiscard]] double cas_success_ratio() const noexcept {
            const auto successful = successful_cas_count.load(std::memory_order_relaxed);
            const auto failed = failed_cas_count.load(std::memory_order_relaxed);
            const auto total = successful + failed;
            return total > 0 ? static_cast<double>(successful) / total : 0.0;
        }

        /**
         * @brief Reset all statistics to zero
         */
        void reset() noexcept {
            operations_count.store(0, std::memory_order_relaxed);
            contention_count.store(0, std::memory_order_relaxed);
            adaptation_count.store(0, std::memory_order_relaxed);
            successful_cas_count.store(0, std::memory_order_relaxed);
            failed_cas_count.store(0, std::memory_order_relaxed);
            backoff_time_ns.store(0, std::memory_order_relaxed);
        }
    };

    /**
     * @brief Memory ordering policies for lock-free operations
     */
    enum class memory_ordering_policy {
        relaxed,    ///< Use relaxed memory ordering for maximum performance
        acquire,    ///< Use acquire-release semantics for correctness
        sequential  ///< Use sequential consistency for strongest guarantees
    };

    /**
     * @brief Backoff strategy for contended operations
     */
    enum class backoff_strategy {
        none,       ///< No backoff - retry immediately
        linear,     ///< Linear backoff - increase delay linearly
        exponential ///< Exponential backoff - double delay each time
    };

    /**
     * @brief Configuration for adaptive behavior
     */
    struct adaptive_config {
        /// Window size for performance measurement
        uint32_t measurement_window = ADAPTATION_WINDOW_SIZE;
        
        /// Threshold for switching to lock-free mode
        double lockfree_threshold = 0.1;  // 10% contention or less
        
        /// Threshold for switching to mutex mode
        double mutex_threshold = 0.5;     // 50% contention or more
        
        /// Minimum time between adaptations (milliseconds)
        uint32_t min_adaptation_interval_ms = 100;
        
        /// Enable performance statistics collection
        bool enable_stats = true;
        
        /// Backoff strategy to use
        backoff_strategy backoff = backoff_strategy::exponential;
        
        /// Memory ordering policy
        memory_ordering_policy ordering = memory_ordering_policy::acquire;
    };

    /**
     * @brief Get default adaptive configuration
     * @return Default configuration suitable for most use cases
     */
    [[nodiscard]] inline adaptive_config default_adaptive_config() noexcept {
        return adaptive_config{};
    }

    /**
     * @brief Get high-performance adaptive configuration
     * @return Configuration optimized for maximum performance
     */
    [[nodiscard]] inline adaptive_config high_performance_config() noexcept {
        adaptive_config config;
        config.enable_stats = false;  // Disable stats for maximum performance
        config.ordering = memory_ordering_policy::relaxed;
        config.backoff = backoff_strategy::linear;
        return config;
    }

    /**
     * @brief Get conservative adaptive configuration
     * @return Configuration optimized for correctness over performance
     */
    [[nodiscard]] inline adaptive_config conservative_config() noexcept {
        adaptive_config config;
        config.lockfree_threshold = 0.05;  // More conservative threshold
        config.mutex_threshold = 0.3;
        config.ordering = memory_ordering_policy::sequential;
        config.min_adaptation_interval_ms = 500;
        return config;
    }
}