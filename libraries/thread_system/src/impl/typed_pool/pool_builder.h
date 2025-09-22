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
 * @file pool_builder.h
 * @brief Builder pattern implementation for typed_thread_pool construction
 * 
 * This file provides a builder pattern for constructing typed thread pools
 * with various configuration options in a fluent interface style.
 */

#include "../detail/forward_declarations.h"
#include "config.h"
#include "job_types.h"
#include <memory>
#include <vector>
#include <string>

namespace kcenon::thread {
    
    /**
     * @brief Builder class for constructing typed thread pools
     * 
     * This class provides a fluent interface for building typed thread pools
     * with various configuration options. It ensures consistent construction
     * and validation of pool parameters.
     */
    template<detail::JobType job_type = job_types>
    class typed_thread_pool_builder {
    public:
        /**
         * @brief Construct a new builder with default settings
         */
        typed_thread_pool_builder() = default;
        
        /**
         * @brief Set the title/name for the thread pool
         * @param title The human-readable name for the pool
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& with_title(const std::string& title) {
            title_ = title;
            return *this;
        }
        
        /**
         * @brief Set the number of worker threads
         * @param count Number of worker threads to create
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& with_worker_count(size_t count) {
            worker_count_ = count;
            return *this;
        }
        
        /**
         * @brief Set the queue size for the thread pool
         * @param size Maximum number of jobs that can be queued
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& with_queue_size(size_t size) {
            queue_size_ = size;
            return *this;
        }
        
        /**
         * @brief Set the job types that workers should handle
         * @param types Vector of job types for worker filtering
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& with_job_types(const std::vector<job_type>& types) {
            job_types_ = types;
            return *this;
        }
        
        /**
         * @brief Enable or disable time tagging for workers
         * @param enable Whether to use time tags in worker threads
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& with_time_tagging(bool enable) {
            use_time_tag_ = enable;
            return *this;
        }
        
        /**
         * @brief Enable or disable automatic pool startup
         * @param auto_start Whether to start the pool immediately after building
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& with_auto_start(bool auto_start) {
            auto_start_ = auto_start;
            return *this;
        }
        
        /**
         * @brief Validate the current configuration
         * @return true if configuration is valid, false otherwise
         */
        [[nodiscard]] bool validate() const {
            return worker_count_ >= config::min_workers && 
                   worker_count_ <= config::max_workers &&
                   queue_size_ > 0 &&
                   !title_.empty();
        }
        
        /**
         * @brief Build the typed thread pool with current configuration
         * @return Shared pointer to the constructed thread pool
         * @throws std::invalid_argument if configuration is invalid
         */
        [[nodiscard]] std::shared_ptr<typed_thread_pool_t<job_type>> build() {
            if (!validate()) {
                throw std::invalid_argument("Invalid thread pool configuration");
            }
            
            auto pool = std::make_shared<typed_thread_pool_t<job_type>>(title_);
            
            // Create workers with specified job types
            for (size_t i = 0; i < worker_count_; ++i) {
                auto worker = std::make_unique<typed_thread_worker_t<job_type>>(
                    job_types_, use_time_tag_
                );
                pool->enqueue(std::move(worker));
            }
            
            if (auto_start_) {
                auto result = pool->start();
                if (result.has_error()) {
                    throw std::runtime_error("Failed to start thread pool: " + 
                                           result.get_error().message());
                }
            }
            
            return pool;
        }
        
        /**
         * @brief Reset the builder to default values
         * @return Reference to this builder for chaining
         */
        typed_thread_pool_builder& reset() {
            title_ = "typed_thread_pool";
            worker_count_ = config::default_worker_count;
            queue_size_ = config::default_queue_size;
            job_types_.clear();
            use_time_tag_ = true;
            auto_start_ = false;
            return *this;
        }
        
    private:
        std::string title_ = "typed_thread_pool";
        size_t worker_count_ = config::default_worker_count;
        size_t queue_size_ = config::default_queue_size;
        std::vector<job_type> job_types_;
        bool use_time_tag_ = true;
        bool auto_start_ = false;
    };
    
    /**
     * @brief Convenience function to create a builder
     * @return A new thread pool builder instance
     */
    template<detail::JobType job_type = job_types>
    [[nodiscard]] typed_thread_pool_builder<job_type> create_pool_builder() {
        return typed_thread_pool_builder<job_type>{};
    }
    
} // namespace kcenon::thread
