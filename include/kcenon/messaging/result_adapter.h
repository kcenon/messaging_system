/**
 * @file result_adapter.h
 * @brief Adapter for different Result implementations
 * @author kcenon
 * @date 2025
 *
 * This adapter provides compatibility between different Result implementations
 * used by common_system and the messaging_system.
 */

#pragma once

#ifdef HAS_COMMON_SYSTEM
#include <kcenon/common/patterns/result.h>
#endif

#include <string>
#include <optional>
#include <variant>
#include <type_traits>

// Add namespace aliases for compatibility
#ifdef HAS_COMMON_SYSTEM
namespace common {
    using namespace ::common;
}
#endif

namespace kcenon::common {

#ifdef HAS_COMMON_SYSTEM
// Use common_system's Result type directly
template<typename T>
using result = ::common::Result<T>;

using result_void = ::common::VoidResult;

// Add helper functions for compatibility
template<typename T>
inline bool is_ok(const result<T>& r) {
    return ::common::is_ok(r);
}

inline bool is_ok(const result_void& r) {
    return ::common::is_ok(r);
}

// Helper functions to match the expected interface
template<typename T>
inline result<std::decay_t<T>> make_success(T&& value) {
    return result<std::decay_t<T>>{std::forward<T>(value)};
}

inline result_void make_success_void() {
    return result_void{std::monostate{}};
}

template<typename T>
inline result<T> make_error(const std::string& error) {
    return result<T>{::common::error_info{-1, error, "messaging"}};
}

inline result_void make_error_void(const std::string& error) {
    return result_void{::common::error_info{-1, error, "messaging"}};
}

#else
// Fallback Result implementation for when common_system is not available

/**
 * @brief Simple Result type for error handling without common_system
 */
template<typename T>
class result {
public:
    // Constructors
    result(T value) : data_(std::move(value)), is_error_(false) {}

    result(const std::string& error)
        : error_message_(error), is_error_(true) {}

    // Success factory
    static result success(T value) {
        return result(std::move(value));
    }

    // Error factory
    static result error(const std::string& msg) {
        return result(msg);
    }

    // Check if result is successful
    bool has_value() const { return !is_error_; }
    bool is_success() const { return !is_error_; }
    bool is_error() const { return is_error_; }

    operator bool() const { return !is_error_; }

    // Get value (throws if error)
    T& value() {
        if (is_error_) {
            throw std::runtime_error("Result contains error: " + error_message_);
        }
        return std::get<T>(data_);
    }

    const T& value() const {
        if (is_error_) {
            throw std::runtime_error("Result contains error: " + error_message_);
        }
        return std::get<T>(data_);
    }

    // Get error message
    const std::string& error_message() const {
        return error_message_;
    }

private:
    std::variant<T, std::monostate> data_;
    std::string error_message_;
    bool is_error_;
};

/**
 * @brief Specialization for void results
 */
class result_void {
public:
    result_void() : is_error_(false) {}
    result_void(const std::string& error)
        : error_message_(error), is_error_(true) {}

    static result_void success() {
        return result_void();
    }

    static result_void error(const std::string& msg) {
        return result_void(msg);
    }

    bool has_value() const { return !is_error_; }
    bool is_success() const { return !is_error_; }
    bool is_error() const { return is_error_; }

    operator bool() const { return !is_error_; }

    const std::string& error_message() const {
        return error_message_;
    }

private:
    std::string error_message_;
    bool is_error_;
};

// Helper functions
template<typename T>
inline result<std::decay_t<T>> make_success(T&& value) {
    return result<std::decay_t<T>>::success(std::forward<T>(value));
}

inline result_void make_success_void() {
    return result_void::success();
}

template<typename T>
inline result<T> make_error(const std::string& error) {
    return result<T>::error(error);
}

inline result_void make_error_void(const std::string& error) {
    return result_void::error(error);
}

#endif // HAS_COMMON_SYSTEM

} // namespace kcenon::common