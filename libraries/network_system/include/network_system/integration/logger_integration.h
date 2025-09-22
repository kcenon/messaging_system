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

#pragma once

/**
 * @file logger_integration.h
 * @brief Logger system integration interface for network_system
 *
 * This interface provides integration with logger_system for centralized
 * logging and monitoring capabilities.
 *
 * @author kcenon
 * @date 2025-09-20
 */

#include <memory>
#include <string>
#include <string_view>

namespace network_system::integration {

/**
 * @enum log_level
 * @brief Log severity levels matching logger_system
 */
enum class log_level : int {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    error = 4,
    fatal = 5
};

/**
 * @class logger_interface
 * @brief Abstract interface for logger integration
 *
 * This interface allows network_system to work with any logger
 * implementation, including the logger_system module.
 */
class logger_interface {
public:
    virtual ~logger_interface() = default;

    /**
     * @brief Log a message with specified level
     * @param level Log severity level
     * @param message Message to log
     */
    virtual void log(log_level level, const std::string& message) = 0;

    /**
     * @brief Log a message with source location information
     * @param level Log severity level
     * @param message Message to log
     * @param file Source file name
     * @param line Line number
     * @param function Function name
     */
    virtual void log(log_level level, const std::string& message,
                    const std::string& file, int line,
                    const std::string& function) = 0;

    /**
     * @brief Check if a log level is enabled
     * @param level Log level to check
     * @return true if the level is enabled
     */
    virtual bool is_level_enabled(log_level level) const = 0;

    /**
     * @brief Flush any buffered log messages
     */
    virtual void flush() = 0;
};

/**
 * @class basic_logger
 * @brief Basic console logger implementation for standalone use
 *
 * This provides a simple logger implementation for when
 * logger_system is not available.
 */
class basic_logger : public logger_interface {
public:
    /**
     * @brief Constructor with minimum log level
     * @param min_level Minimum level to log (default: info)
     */
    explicit basic_logger(log_level min_level = log_level::info);

    ~basic_logger() override;

    // logger_interface implementation
    void log(log_level level, const std::string& message) override;
    void log(log_level level, const std::string& message,
            const std::string& file, int line,
            const std::string& function) override;
    bool is_level_enabled(log_level level) const override;
    void flush() override;

    /**
     * @brief Set minimum log level
     * @param level New minimum level
     */
    void set_min_level(log_level level);

    /**
     * @brief Get current minimum log level
     * @return Current minimum level
     */
    log_level get_min_level() const;

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

#ifdef BUILD_WITH_LOGGER_SYSTEM
/**
 * @class logger_system_adapter
 * @brief Adapter for logger_system integration
 *
 * This adapter wraps logger_system's logger class to provide
 * the logger_interface implementation.
 */
class logger_system_adapter : public logger_interface {
public:
    /**
     * @brief Constructor
     * @param async Enable async logging (default: true)
     * @param buffer_size Buffer size for async mode (default: 8192)
     */
    explicit logger_system_adapter(bool async = true, size_t buffer_size = 8192);

    ~logger_system_adapter() override;

    // logger_interface implementation
    void log(log_level level, const std::string& message) override;
    void log(log_level level, const std::string& message,
            const std::string& file, int line,
            const std::string& function) override;
    bool is_level_enabled(log_level level) const override;
    void flush() override;

    /**
     * @brief Start the logger (required for async mode)
     */
    void start();

    /**
     * @brief Stop the logger
     */
    void stop();

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};
#endif // BUILD_WITH_LOGGER_SYSTEM

/**
 * @class logger_integration_manager
 * @brief Manager for logger system integration
 *
 * This class manages the integration between network_system and
 * logger implementations.
 */
class logger_integration_manager {
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton instance
     */
    static logger_integration_manager& instance();

    /**
     * @brief Set the logger implementation
     * @param logger Logger to use
     */
    void set_logger(std::shared_ptr<logger_interface> logger);

    /**
     * @brief Get the current logger
     * @return Current logger (creates basic logger if none set)
     */
    std::shared_ptr<logger_interface> get_logger();

    /**
     * @brief Log a message
     * @param level Log level
     * @param message Message to log
     */
    void log(log_level level, const std::string& message);

    /**
     * @brief Log a message with location
     * @param level Log level
     * @param message Message to log
     * @param file Source file
     * @param line Line number
     * @param function Function name
     */
    void log(log_level level, const std::string& message,
            const std::string& file, int line, const std::string& function);

private:
    logger_integration_manager();
    ~logger_integration_manager();

    class impl;
    std::unique_ptr<impl> pimpl_;
};

// Convenience macros for logging with automatic source location
#define NETWORK_LOG_TRACE(msg) \
    network_system::integration::logger_integration_manager::instance().log( \
        network_system::integration::log_level::trace, msg, __FILE__, __LINE__, __FUNCTION__)

#define NETWORK_LOG_DEBUG(msg) \
    network_system::integration::logger_integration_manager::instance().log( \
        network_system::integration::log_level::debug, msg, __FILE__, __LINE__, __FUNCTION__)

#define NETWORK_LOG_INFO(msg) \
    network_system::integration::logger_integration_manager::instance().log( \
        network_system::integration::log_level::info, msg, __FILE__, __LINE__, __FUNCTION__)

#define NETWORK_LOG_WARN(msg) \
    network_system::integration::logger_integration_manager::instance().log( \
        network_system::integration::log_level::warn, msg, __FILE__, __LINE__, __FUNCTION__)

#define NETWORK_LOG_ERROR(msg) \
    network_system::integration::logger_integration_manager::instance().log( \
        network_system::integration::log_level::error, msg, __FILE__, __LINE__, __FUNCTION__)

#define NETWORK_LOG_FATAL(msg) \
    network_system::integration::logger_integration_manager::instance().log( \
        network_system::integration::log_level::fatal, msg, __FILE__, __LINE__, __FUNCTION__)

} // namespace network_system::integration