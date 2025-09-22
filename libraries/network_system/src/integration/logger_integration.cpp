/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
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
 * @file logger_integration.cpp
 * @brief Implementation of logger system integration
 *
 * @author kcenon
 * @date 2025-09-20
 */

#include "network_system/integration/logger_integration.h"
#include <iostream>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef BUILD_WITH_LOGGER_SYSTEM
#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/writers/console_writer.h>
#endif

namespace network_system::integration {

// Helper function to convert log level to string
static const char* level_to_string(log_level level) {
    switch (level) {
        case log_level::trace: return "TRACE";
        case log_level::debug: return "DEBUG";
        case log_level::info:  return "INFO ";
        case log_level::warn:  return "WARN ";
        case log_level::error: return "ERROR";
        case log_level::fatal: return "FATAL";
        default: return "UNKN ";
    }
}

// Helper function to get current timestamp
static std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

//============================================================================
// basic_logger implementation
//============================================================================

class basic_logger::impl {
public:
    explicit impl(log_level min_level) : min_level_(static_cast<int>(min_level)) {}

    void log(log_level level, const std::string& message) {
        if (static_cast<int>(level) < min_level_) return;

        std::lock_guard<std::mutex> lock(mutex_);

        // Use cerr for errors and above, cout for others
        auto& stream = (level >= log_level::error) ? std::cerr : std::cout;

        stream << "[" << get_timestamp() << "] "
               << "[" << level_to_string(level) << "] "
               << "[network_system] "
               << message << std::endl;
    }

    void log(log_level level, const std::string& message,
            const std::string& file, int line, const std::string& function) {
        if (static_cast<int>(level) < min_level_) return;

        std::lock_guard<std::mutex> lock(mutex_);

        auto& stream = (level >= log_level::error) ? std::cerr : std::cout;

        stream << "[" << get_timestamp() << "] "
               << "[" << level_to_string(level) << "] "
               << "[network_system] "
               << message
               << " (" << file << ":" << line << " in " << function << ")"
               << std::endl;
    }

    bool is_level_enabled(log_level level) const {
        return static_cast<int>(level) >= min_level_;
    }

    void flush() {
        std::cout.flush();
        std::cerr.flush();
    }

    void set_min_level(log_level level) {
        min_level_ = static_cast<int>(level);
    }

    log_level get_min_level() const {
        return static_cast<log_level>(min_level_.load());
    }

private:
    mutable std::mutex mutex_;
    std::atomic<int> min_level_;
};

basic_logger::basic_logger(log_level min_level)
    : pimpl_(std::make_unique<impl>(min_level)) {}

basic_logger::~basic_logger() = default;

void basic_logger::log(log_level level, const std::string& message) {
    pimpl_->log(level, message);
}

void basic_logger::log(log_level level, const std::string& message,
                      const std::string& file, int line, const std::string& function) {
    pimpl_->log(level, message, file, line, function);
}

bool basic_logger::is_level_enabled(log_level level) const {
    return pimpl_->is_level_enabled(level);
}

void basic_logger::flush() {
    pimpl_->flush();
}

void basic_logger::set_min_level(log_level level) {
    pimpl_->set_min_level(level);
}

log_level basic_logger::get_min_level() const {
    return pimpl_->get_min_level();
}

//============================================================================
// logger_system_adapter implementation (only when logger_system is available)
//============================================================================

#ifdef BUILD_WITH_LOGGER_SYSTEM

// Helper function to convert our log_level to logger_system's log_level
static logger_system::log_level convert_level(log_level level) {
    switch (level) {
        case log_level::trace: return logger_system::log_level::trace;
        case log_level::debug: return logger_system::log_level::debug;
        case log_level::info:  return logger_system::log_level::info;
        case log_level::warn:  return logger_system::log_level::warn;
        case log_level::error: return logger_system::log_level::error;
        case log_level::fatal: return logger_system::log_level::fatal;
        default: return logger_system::log_level::info;
    }
}

class logger_system_adapter::impl {
public:
    impl(bool async, size_t buffer_size)
        : logger_(std::make_shared<kcenon::logger::logger>(async, buffer_size)),
          started_(false) {
        // Add console writer by default
        logger_->add_writer(std::make_unique<kcenon::logger::console_writer>());
    }

    void log(log_level level, const std::string& message) {
        logger_->log(convert_level(level), message);
    }

    void log(log_level level, const std::string& message,
            const std::string& file, int line, const std::string& function) {
        logger_->log(convert_level(level), message, file, line, function);
    }

    bool is_level_enabled(log_level level) const {
        return logger_->is_enabled(convert_level(level));
    }

    void flush() {
        logger_->flush();
    }

    void start() {
        if (!started_) {
            logger_->start();
            started_ = true;
        }
    }

    void stop() {
        if (started_) {
            logger_->stop();
            started_ = false;
        }
    }

private:
    std::shared_ptr<kcenon::logger::logger> logger_;
    std::atomic<bool> started_;
};

logger_system_adapter::logger_system_adapter(bool async, size_t buffer_size)
    : pimpl_(std::make_unique<impl>(async, buffer_size)) {
    // Auto-start the logger
    pimpl_->start();
}

logger_system_adapter::~logger_system_adapter() {
    pimpl_->stop();
}

void logger_system_adapter::log(log_level level, const std::string& message) {
    pimpl_->log(level, message);
}

void logger_system_adapter::log(log_level level, const std::string& message,
                                const std::string& file, int line, const std::string& function) {
    pimpl_->log(level, message, file, line, function);
}

bool logger_system_adapter::is_level_enabled(log_level level) const {
    return pimpl_->is_level_enabled(level);
}

void logger_system_adapter::flush() {
    pimpl_->flush();
}

void logger_system_adapter::start() {
    pimpl_->start();
}

void logger_system_adapter::stop() {
    pimpl_->stop();
}

#endif // BUILD_WITH_LOGGER_SYSTEM

//============================================================================
// logger_integration_manager implementation
//============================================================================

class logger_integration_manager::impl {
public:
    impl() {
        // Create default logger if logger_system is available
#ifdef BUILD_WITH_LOGGER_SYSTEM
        logger_ = std::make_shared<logger_system_adapter>();
#else
        logger_ = std::make_shared<basic_logger>();
#endif
    }

    void set_logger(std::shared_ptr<logger_interface> logger) {
        std::lock_guard<std::mutex> lock(mutex_);
        logger_ = logger;
    }

    std::shared_ptr<logger_interface> get_logger() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!logger_) {
            logger_ = std::make_shared<basic_logger>();
        }
        return logger_;
    }

    void log(log_level level, const std::string& message) {
        auto logger = get_logger();
        if (logger) {
            logger->log(level, message);
        }
    }

    void log(log_level level, const std::string& message,
            const std::string& file, int line, const std::string& function) {
        auto logger = get_logger();
        if (logger) {
            logger->log(level, message, file, line, function);
        }
    }

private:
    mutable std::mutex mutex_;
    std::shared_ptr<logger_interface> logger_;
};

logger_integration_manager& logger_integration_manager::instance() {
    static logger_integration_manager instance;
    return instance;
}

logger_integration_manager::logger_integration_manager()
    : pimpl_(std::make_unique<impl>()) {}

logger_integration_manager::~logger_integration_manager() = default;

void logger_integration_manager::set_logger(std::shared_ptr<logger_interface> logger) {
    pimpl_->set_logger(logger);
}

std::shared_ptr<logger_interface> logger_integration_manager::get_logger() {
    return pimpl_->get_logger();
}

void logger_integration_manager::log(log_level level, const std::string& message) {
    pimpl_->log(level, message);
}

void logger_integration_manager::log(log_level level, const std::string& message,
                                    const std::string& file, int line, const std::string& function) {
    pimpl_->log(level, message, file, line, function);
}

} // namespace network_system::integration