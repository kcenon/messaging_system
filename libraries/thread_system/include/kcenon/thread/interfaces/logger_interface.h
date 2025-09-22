// Copyright (c) 2025, 🍀☀🌕🌥 🌊
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <memory>
#include <mutex>
#include <string>

namespace kcenon::thread {

/**
 * @brief Log level enumeration
 */
enum class log_level {
  critical = 0,
  error = 1,
  warning = 2,
  info = 3,
  debug = 4,
  trace = 5
};

/**
 * @brief Logger interface for thread system
 *
 * This interface allows the thread system to log messages without
 * depending on a specific logger implementation.
 */
class logger_interface {
public:
  virtual ~logger_interface() = default;

  /**
   * @brief Log a message with specified level
   * @param level Log level
   * @param message Log message
   */
  virtual void log(log_level level, const std::string& message) = 0;

  /**
   * @brief Log a message with source location information
   * @param level Log level
   * @param message Log message
   * @param file Source file name
   * @param line Source line number
   * @param function Function name
   */
  virtual void log(log_level level, const std::string& message,
                   const std::string& file, int line,
                   const std::string& function) = 0;

  /**
   * @brief Check if logging is enabled for the specified level
   * @param level Log level to check
   * @return true if logging is enabled for this level
   */
  virtual bool is_enabled(log_level level) const = 0;

  /**
   * @brief Flush any buffered log messages
   */
  virtual void flush() = 0;
};

/**
 * @brief Global logger registry
 *
 * Manages the global logger instance used by the thread system.
 */
class logger_registry {
public:
  /**
   * @brief Set the global logger instance
   * @param logger Logger implementation
   */
  static void set_logger(std::shared_ptr<logger_interface> logger);

  /**
   * @brief Get the global logger instance
   * @return Current logger instance, may be nullptr
   */
  static std::shared_ptr<logger_interface> get_logger();

  /**
   * @brief Clear the global logger instance
   */
  static void clear_logger();

private:
  static std::shared_ptr<logger_interface> logger_;
  static std::mutex mutex_;
};

// Convenience macros for logging
#define THREAD_LOG_IF_ENABLED(level, message)                                  \
  do {                                                                         \
    if (auto logger = kcenon::thread::logger_registry::get_logger()) {         \
      if (logger->is_enabled(level)) {                                        \
        logger->log(level, message, __FILE__, __LINE__, __FUNCTION__);        \
      }                                                                        \
    }                                                                          \
  } while (0)

#define THREAD_LOG_CRITICAL(message)                                          \
  THREAD_LOG_IF_ENABLED(kcenon::thread::log_level::critical, message)
#define THREAD_LOG_ERROR(message)                                             \
  THREAD_LOG_IF_ENABLED(kcenon::thread::log_level::error, message)
#define THREAD_LOG_WARNING(message)                                           \
  THREAD_LOG_IF_ENABLED(kcenon::thread::log_level::warning, message)
#define THREAD_LOG_INFO(message)                                              \
  THREAD_LOG_IF_ENABLED(kcenon::thread::log_level::info, message)
#define THREAD_LOG_DEBUG(message)                                             \
  THREAD_LOG_IF_ENABLED(kcenon::thread::log_level::debug, message)
#define THREAD_LOG_TRACE(message)                                             \
  THREAD_LOG_IF_ENABLED(kcenon::thread::log_level::trace, message)

} // namespace kcenon::thread