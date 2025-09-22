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

#include <memory>
#include <functional>
#include <string>

namespace kcenon::thread::jobs {
    /**
     * @brief Job priority levels for type-based scheduling
     */
    enum class job_priority : uint8_t {
        low = 0,      ///< Low priority jobs (background tasks)
        normal = 1,   ///< Normal priority jobs (regular tasks)
        high = 2      ///< High priority jobs (urgent tasks)
    };

    /**
     * @brief Job execution states
     */
    enum class job_state : uint8_t {
        pending,      ///< Job is waiting to be executed
        running,      ///< Job is currently being executed
        completed,    ///< Job has completed successfully
        cancelled,    ///< Job was cancelled before execution
        failed        ///< Job execution failed
    };

    /**
     * @brief Interface for executable job objects
     * 
     * This interface defines the contract that all job objects must follow.
     * It provides a clean abstraction for work units that can be scheduled
     * and executed by thread pools.
     */
    class job_interface {
    public:
        virtual ~job_interface() = default;

        /**
         * @brief Execute the job
         * @return true if execution was successful, false otherwise
         */
        virtual bool execute() = 0;

        /**
         * @brief Get the priority of this job
         * @return Job priority level
         */
        [[nodiscard]] virtual job_priority priority() const = 0;

        /**
         * @brief Get the current state of this job
         * @return Current job state
         */
        [[nodiscard]] virtual job_state state() const = 0;

        /**
         * @brief Get a description of this job (for debugging/logging)
         * @return Human-readable job description
         */
        [[nodiscard]] virtual std::string description() const = 0;

        /**
         * @brief Cancel the job if it hasn't started executing yet
         * @return true if job was cancelled, false if already running/completed
         */
        virtual bool cancel() = 0;
    };

    /**
     * @brief Shared pointer type for job objects
     */
    using job_ptr = std::shared_ptr<job_interface>;

    /**
     * @brief Weak pointer type for job objects
     */
    using job_weak_ptr = std::weak_ptr<job_interface>;

    /**
     * @brief Function type for simple callback-based jobs
     */
    using job_function = std::function<void()>;

    /**
     * @brief Function type for jobs that can be cancelled
     */
    using cancellable_job_function = std::function<void(const std::atomic<bool>&)>;

    /**
     * @brief Utility function to create a job priority from integer
     * @param priority_value Integer priority (0=low, 1=normal, 2=high)
     * @return Corresponding job_priority enum value
     */
    [[nodiscard]] inline job_priority make_priority(int priority_value) {
        switch (priority_value) {
            case 0: return job_priority::low;
            case 1: return job_priority::normal;
            case 2: return job_priority::high;
            default: return job_priority::normal;
        }
    }

    /**
     * @brief Convert job priority to string representation
     * @param priority Job priority to convert
     * @return String representation of the priority
     */
    [[nodiscard]] inline std::string to_string(job_priority priority) {
        switch (priority) {
            case job_priority::low: return "low";
            case job_priority::normal: return "normal";
            case job_priority::high: return "high";
            default: return "unknown";
        }
    }

    /**
     * @brief Convert job state to string representation
     * @param state Job state to convert
     * @return String representation of the state
     */
    [[nodiscard]] inline std::string to_string(job_state state) {
        switch (state) {
            case job_state::pending: return "pending";
            case job_state::running: return "running";
            case job_state::completed: return "completed";
            case job_state::cancelled: return "cancelled";
            case job_state::failed: return "failed";
            default: return "unknown";
        }
    }
}