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
#include <optional>
#include "logger_interface.h"
#include "monitoring_interface.h"
#include "service_container.h"

namespace kcenon::thread {

/**
 * @brief Context object that provides access to optional services
 *
 * This class uses composition to provide thread system components
 * with optional access to logger and monitoring services.
 */
class thread_context {
public:
    /**
     * @brief Default constructor - resolves services from global container
     */
    thread_context() 
        : logger_(service_container::global().resolve<logger_interface>())
        , monitoring_(service_container::global().resolve<monitoring_interface::monitoring_interface>()) {
    }

    /**
     * @brief Constructor with explicit service injection
     * @param logger Optional logger service
     * @param monitoring Optional monitoring service
     */
    explicit thread_context(
        std::shared_ptr<logger_interface> logger,
        std::shared_ptr<monitoring_interface::monitoring_interface> monitoring = nullptr)
        : logger_(std::move(logger))
        , monitoring_(std::move(monitoring)) {
    }

    /**
     * @brief Get the logger service
     * @return Logger service or nullptr if not available
     */
    std::shared_ptr<logger_interface> logger() const {
        return logger_;
    }

    /**
     * @brief Get the monitoring service
     * @return Monitoring service or nullptr if not available
     */
    std::shared_ptr<monitoring_interface::monitoring_interface> monitoring() const {
        return monitoring_;
    }

    /**
     * @brief Log a message if logger is available
     * @param level Log level
     * @param message Log message
     */
    void log(log_level level, const std::string& message) const {
        if (logger_) {
            logger_->log(level, message);
        }
    }

    /**
     * @brief Log a message with source information if logger is available
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     */
    void log(log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) const {
        if (logger_) {
            logger_->log(level, message, file, line, function);
        }
    }

    /**
     * @brief Update system metrics if monitoring is available
     * @param metrics System metrics to record
     */
    void update_system_metrics(const monitoring_interface::system_metrics& metrics) const {
        if (monitoring_) {
            monitoring_->update_system_metrics(metrics);
        }
    }

    /**
     * @brief Update thread pool metrics if monitoring is available
     * @param metrics Thread pool metrics to record
     */
    void update_thread_pool_metrics(const monitoring_interface::thread_pool_metrics& metrics) const {
        if (monitoring_) {
            monitoring_->update_thread_pool_metrics(metrics);
        }
    }

    /**
     * @brief Update thread pool metrics with pool identifier
     * @param pool_name Name of the thread pool
     * @param pool_instance_id Instance ID for multiple pools
     * @param metrics Thread pool metrics to record
     */
    void update_thread_pool_metrics(const std::string& pool_name,
                                   std::uint32_t pool_instance_id,
                                   const monitoring_interface::thread_pool_metrics& metrics) const {
        if (monitoring_) {
            monitoring_->update_thread_pool_metrics(pool_name, pool_instance_id, metrics);
        }
    }

    /**
     * @brief Update worker metrics if monitoring is available
     * @param worker_id Worker identifier
     * @param metrics Worker metrics to record
     */
    void update_worker_metrics(std::size_t worker_id, 
                              const monitoring_interface::worker_metrics& metrics) const {
        if (monitoring_) {
            monitoring_->update_worker_metrics(worker_id, metrics);
        }
    }

    /**
     * @brief Create a child context with the same services
     * @return New context with shared services
     */
    thread_context create_child() const {
        return thread_context(logger_, monitoring_);
    }

    /**
     * @brief Get context name
     * @return Context name
     */
    auto get_context_name() const -> std::string {
        return context_name_;
    }

    /**
     * @brief Set context name
     * @param name New context name
     * @return True if successful
     */
    auto set_context_name(const std::string& name) -> bool {
        context_name_ = name;
        return true;
    }

    /**
     * @brief Check if logger is available
     * @return True if logger is set
     */
    auto has_logger() const -> bool {
        return logger_ != nullptr;
    }

    /**
     * @brief Check if monitoring is available
     * @return True if monitoring is set
     */
    auto has_monitoring() const -> bool {
        return monitoring_ != nullptr;
    }

private:
    std::shared_ptr<logger_interface> logger_;
    std::shared_ptr<monitoring_interface::monitoring_interface> monitoring_;
    mutable std::string context_name_;
};

/**
 * @brief Builder for thread_context with fluent interface
 */
class thread_context_builder {
public:
    thread_context_builder& with_logger(std::shared_ptr<logger_interface> logger) {
        logger_ = std::move(logger);
        return *this;
    }

    thread_context_builder& with_monitoring(
        std::shared_ptr<monitoring_interface::monitoring_interface> monitoring) {
        monitoring_ = std::move(monitoring);
        return *this;
    }

    thread_context_builder& from_global_container() {
        logger_ = service_container::global().resolve<logger_interface>();
        monitoring_ = service_container::global().resolve<monitoring_interface::monitoring_interface>();
        return *this;
    }

    thread_context build() const {
        return thread_context(logger_, monitoring_);
    }

private:
    std::shared_ptr<logger_interface> logger_;
    std::shared_ptr<monitoring_interface::monitoring_interface> monitoring_;
};

} // namespace kcenon::thread