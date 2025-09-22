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
 * @file typed_job_interface.h
 * @brief Interface definitions for typed jobs
 * 
 * This file contains the base interfaces that all typed jobs must implement,
 * providing a clean separation between interface and implementation.
 */

#include "../detail/forward_declarations.h"
#include "../detail/type_traits.h"
#include "../../thread_base/sync/error_handling.h"
#include <string>
#include <memory>

namespace typed_thread_pool_module {
    
    using namespace thread_module; // For result_void
    
    /**
     * @brief Base interface for all typed jobs
     * 
     * This interface defines the contract that all typed jobs must follow.
     * It provides type information and execution capabilities while maintaining
     * type safety through templates.
     */
    template<detail::JobType job_type>
    class typed_job_interface {
    public:
        virtual ~typed_job_interface() = default;
        
        /**
         * @brief Get the type/priority of this job
         * @return The job type used for scheduling decisions
         */
        [[nodiscard]] virtual job_type type() const noexcept = 0;
        
        /**
         * @brief Execute the job's work
         * @return Result indicating success or failure
         */
        [[nodiscard]] virtual result_void execute() = 0;
        
        /**
         * @brief Get a human-readable description of this job
         * @return Description string for debugging/logging
         */
        [[nodiscard]] virtual std::string description() const = 0;
        
        /**
         * @brief Check if this job can be executed
         * @return true if job is ready for execution
         */
        [[nodiscard]] virtual bool is_ready() const noexcept {
            return true; // Default implementation - job is always ready
        }
        
        /**
         * @brief Check if this job has been cancelled
         * @return true if job execution should be skipped
         */
        [[nodiscard]] virtual bool is_cancelled() const noexcept {
            return false; // Default implementation - job is not cancellable
        }
        
        /**
         * @brief Cancel this job if possible
         * @return true if job was successfully cancelled
         */
        virtual bool cancel() noexcept {
            return false; // Default implementation - job cannot be cancelled
        }
        
        /**
         * @brief Get the estimated execution time for this job
         * @return Estimated execution time in microseconds, 0 if unknown
         */
        [[nodiscard]] virtual uint64_t estimated_execution_time_us() const noexcept {
            return 0; // Default implementation - unknown execution time
        }
        
        /**
         * @brief Check if this job has higher priority than another job
         * @param other The other job to compare against
         * @return true if this job should be executed before the other job
         */
        [[nodiscard]] virtual bool has_higher_priority(const typed_job_interface& other) const noexcept {
            return detail::higher_priority(type(), other.type());
        }
    };
    
    /**
     * @brief Shared pointer type for typed job interfaces
     */
    template<detail::JobType job_type>
    using typed_job_ptr = std::shared_ptr<typed_job_interface<job_type>>;
    
    /**
     * @brief Weak pointer type for typed job interfaces
     */
    template<detail::JobType job_type>
    using typed_job_weak_ptr = std::weak_ptr<typed_job_interface<job_type>>;
    
    /**
     * @brief Factory interface for creating typed jobs
     * 
     * This interface allows for pluggable job creation strategies,
     * enabling custom job factories, pooling, or other creation patterns.
     */
    template<detail::JobType job_type>
    class typed_job_factory_interface {
    public:
        virtual ~typed_job_factory_interface() = default;
        
        /**
         * @brief Create a new job with the specified type and callback
         * @param type The job type/priority
         * @param callback The function to execute
         * @param description Optional description for the job
         * @return Shared pointer to the created job
         */
        template<detail::JobCallable F>
        [[nodiscard]] virtual typed_job_ptr<job_type> create_job(
            job_type type,
            F&& callback,
            const std::string& description = "callback_job"
        ) = 0;
        
        /**
         * @brief Release resources associated with completed jobs
         * 
         * This method can be called periodically to clean up any
         * internal caches or pools maintained by the factory.
         */
        virtual void cleanup() noexcept {}
    };
    
} // namespace typed_thread_pool_module