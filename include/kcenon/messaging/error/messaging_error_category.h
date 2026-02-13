// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file messaging_error_category.h
 * @brief Error category for messaging_system typed error codes
 *
 * Provides a messaging-specific error category that integrates with
 * common_system's typed_error_code infrastructure. This enables:
 * - Type-safe error codes that carry their origin category
 * - Direct integration with Result<T> via typed_error_code
 * - Clear identification of messaging errors vs other system errors
 *
 * Usage Example:
 * @code
 * #include <kcenon/messaging/error/messaging_error_category.h>
 *
 * // Create a typed error code
 * auto ec = kcenon::messaging::make_messaging_error_code(
 *     kcenon::messaging::error::queue_full);
 *
 * // Use with Result<T>
 * return Result<void>::err(ec);
 *
 * // Check category
 * if (ec.category() == messaging_error_category::instance()) {
 *     // Handle messaging-specific error
 * }
 * @endcode
 *
 * @see error_codes.h for error code constants
 * @see common_system's error_category.h for base class
 */

#pragma once

#include "error_codes.h"

#include <kcenon/common/error/error_category.h>

namespace kcenon::messaging {

/**
 * @class messaging_error_category
 * @brief Error category for messaging_system error codes
 *
 * Singleton error category that maps messaging error codes (-700 to -799)
 * to human-readable messages. Inherits from common::error_category and
 * follows the same pattern as common_error_category.
 *
 * Thread Safety:
 * - Stateless singleton, safe for concurrent access
 * - C++11 guarantees thread-safe static local initialization
 */
class messaging_error_category : public common::error_category {
public:
    /**
     * @brief Returns the singleton instance
     *
     * Thread-safe due to C++11 static local variable initialization.
     *
     * @return Reference to the singleton instance
     */
    static const messaging_error_category& instance() noexcept {
        static messaging_error_category inst;
        return inst;
    }

    /**
     * @brief Returns the category name
     * @return "messaging"
     */
    std::string_view name() const noexcept override {
        return "messaging";
    }

    /**
     * @brief Returns a human-readable message for the error code
     *
     * Delegates to error::get_error_message() which contains the
     * canonical mapping for all messaging error codes.
     *
     * @param code Error code value (expected range: -700 to -799)
     * @return Human-readable error message
     */
    std::string message(int code) const override {
        return std::string(error::get_error_message(code));
    }

private:
    messaging_error_category() = default;
};

/**
 * @brief Create a typed_error_code for a messaging error code
 *
 * Convenience function that wraps an integer error code with the
 * messaging_error_category.
 *
 * @param code Messaging error code (from error:: namespace)
 * @return typed_error_code with messaging_error_category
 *
 * @code
 * auto ec = make_messaging_error_code(error::queue_full);
 * // ec.category().name() == "messaging"
 * // ec.message() == "Message queue full"
 * @endcode
 */
inline common::typed_error_code make_messaging_error_code(int code) noexcept {
    return common::typed_error_code(code, messaging_error_category::instance());
}

} // namespace kcenon::messaging
