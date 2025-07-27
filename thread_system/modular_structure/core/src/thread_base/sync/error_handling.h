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
 * @file error_handling.h
 * @brief Modern error handling for the thread system
 *
 * This file provides typed error handling capabilities using a result type pattern
 * that is similar to std::expected from C++23, but can be used with C++20.
 */

#include <string>
#include <optional>
#include <type_traits>
#include <string_view>
#include <ostream>
#include <functional>
#include <stdexcept>

namespace thread_module {

/**
 * @enum error_code
 * @brief Strongly typed error codes for thread system operations
 */
enum class error_code {
    // General errors
    success = 0,
    unknown_error,
    operation_canceled,
    operation_timeout,
    not_implemented,
    invalid_argument,
    
    // Thread errors
    thread_already_running = 100,
    thread_not_running,
    thread_start_failure,
    thread_join_failure,
    
    // Queue errors
    queue_full = 200,
    queue_empty,
    queue_stopped,
    
    // Job errors
    job_creation_failed = 300,
    job_execution_failed,
    job_invalid,
    
    // Resource errors
    resource_allocation_failed = 400,
    resource_limit_reached,
    
    // Synchronization errors
    mutex_error = 500,
    deadlock_detected,
    condition_variable_error,
    
    // IO errors
    io_error = 600,
    file_not_found
};

/**
 * @brief Converts an error_code to a string representation
 * @param code The error code to convert
 * @return A human-readable string describing the error
 */
inline std::string error_code_to_string(error_code code) {
    switch (code) {
        case error_code::success: return "Success";
        case error_code::unknown_error: return "Unknown error";
        case error_code::operation_canceled: return "Operation canceled";
        case error_code::operation_timeout: return "Operation timed out";
        case error_code::not_implemented: return "Not implemented";
        case error_code::invalid_argument: return "Invalid argument";
            
        case error_code::thread_already_running: return "Thread is already running";
        case error_code::thread_not_running: return "Thread is not running";
        case error_code::thread_start_failure: return "Failed to start thread";
        case error_code::thread_join_failure: return "Failed to join thread";
            
        case error_code::queue_full: return "Queue is full";
        case error_code::queue_empty: return "Queue is empty";
        case error_code::queue_stopped: return "Queue is stopped";
            
        case error_code::job_creation_failed: return "Failed to create job";
        case error_code::job_execution_failed: return "Failed to execute job";
        case error_code::job_invalid: return "Invalid job";
            
        case error_code::resource_allocation_failed: return "Failed to allocate resource";
        case error_code::resource_limit_reached: return "Resource limit reached";
            
        case error_code::mutex_error: return "Mutex error";
        case error_code::deadlock_detected: return "Deadlock detected";
        case error_code::condition_variable_error: return "Condition variable error";
            
        case error_code::io_error: return "I/O error";
        case error_code::file_not_found: return "File not found";
            
        default: return "Unknown error code";
    }
}

/**
 * @class error
 * @brief Represents an error in the thread system
 * 
 * This class encapsulates an error code and an optional message.
 */
class error {
public:
    /**
     * @brief Constructs an error with a code and optional message
     * @param code The error code
     * @param message Optional detailed message about the error
     */
    explicit error(error_code code, std::string message = {})
        : code_(code), message_(std::move(message)) {}
    
    /**
     * @brief Gets the error code
     * @return The error code
     */
    [[nodiscard]] error_code code() const noexcept { return code_; }
    
    /**
     * @brief Gets the error message
     * @return The error message
     */
    [[nodiscard]] const std::string& message() const noexcept { return message_; }
    
    /**
     * @brief Converts the error to a string representation
     * @return A string describing the error
     */
    [[nodiscard]] std::string to_string() const {
        if (message_.empty()) {
            return error_code_to_string(code_);
        }
        return error_code_to_string(code_) + ": " + message_;
    }
    
    /**
     * @brief Implicit conversion to string
     */
    operator std::string() const {
        return to_string();
    }
    
private:
    error_code code_;
    std::string message_;
};

// Forward declaration of result template
template <typename T>
class result;

/**
 * @brief Wrapper for void result
 * 
 * This class represents a result that doesn't return a value (void),
 * but can indicate success or failure.
 */
class result_void {
public:
    /**
     * @brief Constructs a successful result
     */
    result_void() = default;
    
    /**
     * @brief Constructs a result with an error
     * @param err The error
     */
    result_void(const error& err) : has_error_(true), error_(err) {}
    
    /**
     * @brief Checks if the result contains an error
     * @return true if the result contains an error, false otherwise
     */
    [[nodiscard]] bool has_error() const { return has_error_; }
    
    /**
     * @brief Gets the error
     * @return A reference to the contained error
     */
    [[nodiscard]] const error& get_error() const { return error_; }
    
    /**
     * @brief Converts to bool for condition checking
     * @return true if successful (no error), false otherwise
     */
    explicit operator bool() const { return !has_error_; }
    
private:
    bool has_error_ = false;
    error error_{error_code::success, ""};
};

/**
 * @class result
 * @brief A template class representing either a value or an error
 * 
 * This class is similar to std::expected from C++23, but can be used with C++20.
 * It holds either a value of type T or an error.
 */
template <typename T>
class result {
public:
    using value_type = T;
    using error_type = error;
    
    /**
     * @brief Constructs a result with a value
     * @param value The value to store
     */
    result(T value) : has_value_(true), value_(std::move(value)), error_(error_code::success, "") {}
    
    /**
     * @brief Constructs a result with an error
     * @param err The error to store
     */
    result(error err) : has_value_(false), error_(std::move(err)) {}
    
    /**
     * @brief Checks if the result contains a value
     * @return true if the result contains a value, false if it contains an error
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return has_value_;
    }
    
    /**
     * @brief Checks if the result contains a value
     * @return true if the result contains a value, false if it contains an error
     */
    [[nodiscard]] bool has_value() const noexcept {
        return has_value_;
    }
    
    /**
     * @brief Gets the value
     * @return A reference to the contained value
     * @throws std::runtime_error if the result contains an error
     */
    [[nodiscard]] T& value() & {
        if (!has_value_) {
            throw std::runtime_error("Cannot access value of an error result");
        }
        return value_;
    }
    
    /**
     * @brief Gets the value
     * @return A const reference to the contained value
     * @throws std::runtime_error if the result contains an error
     */
    [[nodiscard]] const T& value() const & {
        if (!has_value_) {
            throw std::runtime_error("Cannot access value of an error result");
        }
        return value_;
    }
    
    /**
     * @brief Gets the value
     * @return An rvalue reference to the contained value
     * @throws std::runtime_error if the result contains an error
     */
    [[nodiscard]] T&& value() && {
        if (!has_value_) {
            throw std::runtime_error("Cannot access value of an error result");
        }
        return std::move(value_);
    }
    
    /**
     * @brief Gets the error
     * @return A reference to the contained error
     * @throws std::runtime_error if the result contains a value
     */
    [[nodiscard]] error& get_error() & {
        if (has_value_) {
            throw std::runtime_error("Cannot get error from successful result");
        }
        return error_;
    }
    
    /**
     * @brief Gets the error
     * @return A const reference to the contained error
     * @throws std::runtime_error if the result contains a value
     */
    [[nodiscard]] const error& get_error() const & {
        if (has_value_) {
            throw std::runtime_error("Cannot get error from successful result");
        }
        return error_;
    }
    
    /**
     * @brief Gets the error
     * @return An rvalue reference to the contained error
     * @throws std::runtime_error if the result contains a value
     */
    [[nodiscard]] error&& get_error() && {
        if (has_value_) {
            throw std::runtime_error("Cannot get error from successful result");
        }
        return std::move(error_);
    }
    
    /**
     * @brief Gets the value or a default
     * @param default_value The value to return if the result contains an error
     * @return The contained value or the default
     */
    template <typename U>
    [[nodiscard]] T value_or(U&& default_value) const & {
        if (has_value_) {
            return value_;
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    
    /**
     * @brief Gets the value or a default
     * @param default_value The value to return if the result contains an error
     * @return The contained value or the default
     */
    template <typename U>
    [[nodiscard]] T value_or(U&& default_value) && {
        if (has_value_) {
            return std::move(value_);
        }
        return static_cast<T>(std::forward<U>(default_value));
    }
    
    /**
     * @brief Gets the value or throws an exception
     * @return The contained value
     * @throws std::runtime_error with the error message if the result contains an error
     */
    [[nodiscard]] T value_or_throw() const & {
        if (has_value_) {
            return value_;
        }
        throw std::runtime_error(error_.to_string());
    }
    
    /**
     * @brief Gets the value or throws an exception
     * @return The contained value
     * @throws std::runtime_error with the error message if the result contains an error
     */
    [[nodiscard]] T value_or_throw() && {
        if (has_value_) {
            return std::move(value_);
        }
        throw std::runtime_error(error_.to_string());
    }
    
    /**
     * @brief Maps the result to another type using a function
     * @param fn The function to apply to the value
     * @return A new result with the mapped value or the original error
     */
    template <typename Fn>
    [[nodiscard]] auto map(Fn&& fn) const -> result<std::invoke_result_t<Fn, const T&>> {
        using ResultType = result<std::invoke_result_t<Fn, const T&>>;
        
        if (has_value_) {
            return ResultType(std::invoke(std::forward<Fn>(fn), value_));
        }
        return ResultType(error_);
    }
    
    /**
     * @brief Maps the result to another type using a function that returns a result
     * @param fn The function to apply to the value
     * @return A new result with the mapped value or the original error
     */
    template <typename Fn>
    [[nodiscard]] auto and_then(Fn&& fn) const -> std::invoke_result_t<Fn, const T&> {
        using ResultType = std::invoke_result_t<Fn, const T&>;
        static_assert(std::is_same_v<typename ResultType::error_type, error>, "Function must return a result with the same error type");
        
        if (has_value_) {
            return std::invoke(std::forward<Fn>(fn), value_);
        }
        return ResultType(error_);
    }
    
private:
    bool has_value_ = false;
    T value_{};
    error error_{error_code::success, ""};
};

/**
 * @brief Specialization of result for void
 */
template <>
class result<void> {
public:
    using value_type = void;
    using error_type = error;
    
    /**
     * @brief Constructs a successful void result
     */
    result() : success_(true) {}
    
    /**
     * @brief Constructs a void result from a result_void
     */
    result(const result_void& r) : success_(!r.has_error()), error_(r.has_error() ? r.get_error() : error{error_code::success, ""}) {}
    
    /**
     * @brief Constructs a result with an error
     * @param err The error to store
     */
    result(error err) : success_(false), error_(std::move(err)) {}
    
    /**
     * @brief Checks if the result is successful
     * @return true if the result is successful, false if it contains an error
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return success_;
    }
    
    /**
     * @brief Checks if the result is successful
     * @return true if the result is successful, false if it contains an error
     */
    [[nodiscard]] bool has_value() const noexcept {
        return success_;
    }
    
    /**
     * @brief Gets the error
     * @return A reference to the contained error
     * @throws std::runtime_error if the result is successful
     */
    [[nodiscard]] error& get_error() & {
        if (success_) {
            throw std::runtime_error("Cannot get error from successful result");
        }
        return error_;
    }
    
    /**
     * @brief Gets the error
     * @return A const reference to the contained error
     * @throws std::runtime_error if the result is successful
     */
    [[nodiscard]] const error& get_error() const & {
        if (success_) {
            throw std::runtime_error("Cannot get error from successful result");
        }
        return error_;
    }
    
    /**
     * @brief Gets the error
     * @return An rvalue reference to the contained error
     * @throws std::runtime_error if the result is successful
     */
    [[nodiscard]] error&& get_error() && {
        if (success_) {
            throw std::runtime_error("Cannot get error from successful result");
        }
        return std::move(error_);
    }
    
    /**
     * @brief Throws an exception if the result contains an error
     * @throws std::runtime_error with the error message if the result contains an error
     */
    void value_or_throw() const {
        if (!success_) {
            throw std::runtime_error(error_.to_string());
        }
    }
    
    /**
     * @brief Maps the result to another type using a function
     * @param fn The function to apply if successful
     * @return A new result with the mapped value or the original error
     */
    template <typename Fn>
    [[nodiscard]] auto map(Fn&& fn) const -> result<std::invoke_result_t<Fn>> {
        using ResultType = result<std::invoke_result_t<Fn>>;
        
        if (success_) {
            return ResultType(std::invoke(std::forward<Fn>(fn)));
        }
        return ResultType(error_);
    }
    
    /**
     * @brief Maps the result to another type using a function that returns a result
     * @param fn The function to apply if successful
     * @return A new result with the mapped value or the original error
     */
    template <typename Fn>
    [[nodiscard]] auto and_then(Fn&& fn) const -> std::invoke_result_t<Fn> {
        using ResultType = std::invoke_result_t<Fn>;
        static_assert(std::is_same_v<typename ResultType::error_type, error>, "Function must return a result with the same error type");
        
        if (success_) {
            return std::invoke(std::forward<Fn>(fn));
        }
        return ResultType(error_);
    }
    
private:
    bool success_ = false;
    error error_{error_code::success, ""};
};

// Type aliases for common result types
template <typename T>
using result_t = result<T>;

// Helper to convert std::optional<std::string> to result<T>
template <typename T>
result<T> optional_error_to_result(const std::optional<std::string>& error, T&& value) {
    if (error) {
        return result<T>(thread_module::error{error_code::unknown_error, *error});
    }
    return result<T>(std::forward<T>(value));
}

// Helper to convert std::optional<std::string> to result<void>
inline result<void> optional_error_to_result(const std::optional<std::string>& error) {
    if (error) {
        return result<void>(thread_module::error{error_code::unknown_error, *error});
    }
    return result<void>();
}

// Compatibility layer for existing code
inline std::optional<std::string> result_to_optional_error(const result<void>& res) {
    if (res) {
        return std::nullopt;
    }
    return res.get_error().to_string();
}

template <typename T>
std::pair<std::optional<T>, std::optional<std::string>> result_to_pair(const result<T>& res) {
    if (res) {
        return {res.value(), std::nullopt};
    }
    return {std::nullopt, res.get_error().to_string()};
}

} // namespace thread_module