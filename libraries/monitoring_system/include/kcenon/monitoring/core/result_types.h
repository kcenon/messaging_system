#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file result_types.h
 * @brief Result pattern type definitions for monitoring system
 * 
 * This file provides Result pattern types that integrate with thread_system's
 * error handling approach, ensuring consistent error management across the
 * monitoring system.
 */

#include "error_codes.h"
#include <variant>
#include <optional>
#include <memory>
#include <string>
#include <source_location>
#include <type_traits>
#include <stdexcept>

namespace monitoring_system {

/**
 * @struct error_info
 * @brief Extended error information with context
 */
struct error_info {
    monitoring_error_code code;
    std::string message;
    std::source_location location;
    std::optional<std::string> context;
    
    error_info(monitoring_error_code c, 
               const std::string& msg = "",
               const std::source_location& loc = std::source_location::current(),
               const std::optional<std::string>& ctx = std::nullopt)
        : code(c)
        , message(msg.empty() ? error_code_to_string(c) : msg)
        , location(loc)
        , context(ctx) {}
    
    /**
     * @brief Get formatted error string
     * @return Formatted error message with location information
     */
    std::string to_string() const {
        std::string result = "[" + error_code_to_string(code) + "] " + message;
        result += " (at " + std::string(location.file_name()) + ":" + 
                  std::to_string(location.line()) + ")";
        if (context.has_value()) {
            result += " Context: " + context.value();
        }
        return result;
    }
};

/**
 * @class result
 * @brief Result type for operations that may fail
 * @tparam T The success value type
 * 
 * This class follows the Result pattern used in thread_system,
 * providing explicit error handling without exceptions.
 */
template<typename T>
class result {
private:
    std::variant<T, error_info> value_;
    
    // Private tags for constructor disambiguation
    struct success_tag {};
    struct error_tag {};
    
public:
    // Constructors - simple and compatible
    result(T&& value) : value_(std::forward<T>(value)) {}
    result(const T& value) : value_(value) {}
    result(error_info&& error) : value_(std::move(error)) {}
    result(const error_info& error) : value_(error) {}
    
    // Convenience constructor for error codes
    result(monitoring_error_code code, 
           const std::string& message = "",
           const std::source_location& loc = std::source_location::current())
        : value_(error_info(code, message, loc)) {}
    
    // Check if result contains a value
    bool has_value() const {
        return std::holds_alternative<T>(value_);
    }
    
    // Implicit bool conversion
    operator bool() const {
        return has_value();
    }
    
    // Access the value (throws if error)
    T& value() {
        if (!has_value()) {
            throw std::runtime_error(get_error().to_string());
        }
        return std::get<T>(value_);
    }
    
    const T& value() const {
        if (!has_value()) {
            throw std::runtime_error(get_error().to_string());
        }
        return std::get<T>(value_);
    }
    
    // Access the value with default
    T value_or(T&& default_value) const {
        return has_value() ? std::get<T>(value_) : std::forward<T>(default_value);
    }
    
    // Access the error
    const error_info& get_error() const {
        return std::get<error_info>(value_);
    }
    
    // Operator-> for direct access
    T* operator->() {
        return &value();
    }
    
    const T* operator->() const {
        return &value();
    }
    
    // Operator* for dereferencing
    T& operator*() {
        return value();
    }
    
    const T& operator*() const {
        return value();
    }
    
    // Monadic operations
    template<typename F>
    auto map(F&& f) -> result<decltype(f(std::declval<T>()))> {
        using U = decltype(f(std::declval<T>()));
        if (has_value()) {
            return result<U>(f(std::get<T>(value_)));
        }
        return result<U>(get_error());
    }
    
    template<typename F>
    auto and_then(F&& f) -> decltype(f(std::declval<T>())) {
        using U = decltype(f(std::declval<T>()));
        if (has_value()) {
            return f(std::get<T>(value_));
        }
        return U(get_error());
    }
    
    template<typename F>
    result or_else(F&& f) {
        if (!has_value()) {
            return f(get_error());
        }
        return *this;
    }
};

/**
 * @class result_void
 * @brief Specialization for operations with no return value
 */
class result_void {
private:
    std::optional<error_info> error_;
    
public:
    // Constructors
    result_void() = default;
    result_void(error_info&& error) : error_(std::move(error)) {}
    result_void(const error_info& error) : error_(error) {}
    
    // Convenience constructor for error codes
    result_void(monitoring_error_code code,
                const std::string& message = "",
                const std::source_location& loc = std::source_location::current())
        : error_(error_info(code, message, loc)) {}
    
    // Static factory for success
    static result_void success() {
        return result_void();
    }
    
    // Static factory for error
    static result_void error(monitoring_error_code code,
                            const std::string& message = "") {
        return result_void(code, message);
    }
    
    // Check if operation succeeded
    bool is_success() const {
        return !error_.has_value();
    }
    
    // Implicit bool conversion
    operator bool() const {
        return is_success();
    }
    
    // Access the error
    const error_info& get_error() const {
        return error_.value();
    }
    
    // Check for specific error
    bool is_error(monitoring_error_code code) const {
        return error_.has_value() && error_->code == code;
    }
};

// Helper functions for creating results

/**
 * @brief Create a successful result
 * @tparam T The value type
 * @param value The success value
 * @return result<T> containing the value
 */
template<typename T>
result<std::decay_t<T>> make_success(T&& value) {
    return result<std::decay_t<T>>(std::forward<T>(value));
}

/**
 * @brief Create an error result
 * @tparam T The expected value type
 * @param code The error code
 * @param message Optional error message
 * @return result<T> containing the error
 */
template<typename T>
result<T> make_error(monitoring_error_code code, 
                     const std::string& message = "") {
    return result<T>(code, message);
}

/**
 * @brief Create an error result with context
 * @tparam T The expected value type
 * @param code The error code
 * @param message Error message
 * @param context Additional context
 * @return result<T> containing the error with context
 */
template<typename T>
result<T> make_error_with_context(monitoring_error_code code,
                                  const std::string& message,
                                  const std::string& context) {
    error_info error(code, message);
    error.context = context;
    return result<T>(error);
}

// Macro for propagating errors
#define MONITORING_TRY(expr) \
    do { \
        auto _result = (expr); \
        if (!_result) { \
            return _result.get_error(); \
        } \
    } while(0)

#define MONITORING_TRY_ASSIGN(var, expr) \
    auto _result_##var = (expr); \
    if (!_result_##var) { \
        return _result_##var.get_error(); \
    } \
    auto& var = *_result_##var;

} // namespace monitoring_system