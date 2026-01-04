// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file message_handler_interface.h
 * @brief Message handler interfaces with CRTP support
 *
 * Provides both virtual and CRTP-based message handler patterns.
 * CRTP eliminates virtual function overhead for performance-critical paths
 * while maintaining API compatibility through type erasure wrappers.
 */

#pragma once

#include <kcenon/common/patterns/result.h>

#include <concepts>
#include <memory>
#include <type_traits>

namespace kcenon::messaging {

// Forward declarations
class message;

// =============================================================================
// C++20 Concepts for Message Handlers
// =============================================================================

/**
 * @concept MessageHandlerConcept
 * @brief Validates that a type can serve as a CRTP message handler
 *
 * Types satisfying this concept must provide handle_impl and can_handle_impl
 * methods with the expected signatures.
 *
 * @tparam T The type to validate
 */
template<typename T>
concept MessageHandlerConcept = requires(T handler, const message& msg) {
    { handler.handle_impl(msg) } -> std::same_as<common::VoidResult>;
    { handler.can_handle_impl(msg) } -> std::convertible_to<bool>;
};

// =============================================================================
// CRTP Base Class
// =============================================================================

/**
 * @class message_handler_base
 * @brief CRTP base class for message handlers
 *
 * Provides compile-time polymorphism for message handling, eliminating
 * virtual function call overhead. Derived classes implement handle_impl
 * and can_handle_impl methods.
 *
 * @tparam Derived The derived class type (CRTP pattern)
 *
 * @example
 * class topic_handler : public message_handler_base<topic_handler> {
 *     friend class message_handler_base<topic_handler>;
 *
 *     common::VoidResult handle_impl(const message& msg) {
 *         // Process message...
 *         return common::ok();
 *     }
 *
 *     bool can_handle_impl(const message& msg) const {
 *         return msg.topic() == "my-topic";
 *     }
 * };
 */
template<typename Derived>
class message_handler_base {
public:
    /**
     * @brief Handle a message using compile-time dispatch
     * @param msg Message to process
     * @return Result indicating success or error
     */
    common::VoidResult handle(const message& msg) {
        return static_cast<Derived*>(this)->handle_impl(msg);
    }

    /**
     * @brief Check if handler can process this message
     * @param msg Message to check
     * @return true if handler can process, false otherwise
     */
    [[nodiscard]] bool can_handle(const message& msg) const {
        return static_cast<const Derived*>(this)->can_handle_impl(msg);
    }

protected:
    /**
     * @brief Default handle implementation (no-op)
     * @param msg Message to process
     * @return Success result
     */
    common::VoidResult handle_impl(const message& /*msg*/) {
        return common::ok();
    }

    /**
     * @brief Default can_handle implementation
     * @param msg Message to check
     * @return true (accepts all messages by default)
     */
    [[nodiscard]] bool can_handle_impl(const message& /*msg*/) const {
        return true;
    }

    ~message_handler_base() = default;
    message_handler_base() = default;
    message_handler_base(const message_handler_base&) = default;
    message_handler_base& operator=(const message_handler_base&) = default;
    message_handler_base(message_handler_base&&) = default;
    message_handler_base& operator=(message_handler_base&&) = default;
};

// =============================================================================
// Virtual Interface (for type erasure and polymorphic containers)
// =============================================================================

/**
 * @interface message_handler_interface
 * @brief Abstract interface for message handlers
 *
 * This interface defines the contract for message processing components.
 * Implementations can be used with message bus, topic router, and other
 * messaging components to handle messages in a decoupled manner.
 *
 * For performance-critical paths, consider using message_handler_base<>
 * with message_handler_wrapper for type erasure when heterogeneous
 * collections are needed.
 */
class message_handler_interface {
public:
    virtual ~message_handler_interface() = default;

    /**
     * @brief Handle a message
     * @param msg Message to process
     * @return Result indicating success or error
     */
    virtual common::VoidResult handle(const message& msg) = 0;

    /**
     * @brief Check if handler can process this message
     * @param msg Message to check
     * @return true if handler can process, false otherwise
     *
     * This method allows handlers to selectively process messages based on
     * content, metadata, or other criteria.
     */
    virtual bool can_handle(const message& msg) const = 0;
};

// =============================================================================
// Type Erasure Wrapper
// =============================================================================

/**
 * @class message_handler_wrapper
 * @brief Type erasure wrapper for CRTP message handlers
 *
 * Wraps a CRTP-based handler to implement message_handler_interface,
 * allowing CRTP handlers to be stored in heterogeneous containers while
 * still benefiting from compile-time dispatch within the wrapper.
 *
 * @tparam Handler The CRTP handler type
 *
 * @example
 * class my_handler : public message_handler_base<my_handler> {
 *     // ...implementation...
 * };
 *
 * // Store in polymorphic container
 * std::vector<std::unique_ptr<message_handler_interface>> handlers;
 * handlers.push_back(std::make_unique<message_handler_wrapper<my_handler>>());
 */
template<typename Handler>
class message_handler_wrapper : public message_handler_interface {
public:
    /**
     * @brief Default construct with default-constructed handler
     */
    message_handler_wrapper() : handler_() {}

    /**
     * @brief Construct with a handler instance
     * @param handler Handler to wrap (moved)
     */
    explicit message_handler_wrapper(Handler handler)
        : handler_(std::move(handler)) {}

    /**
     * @brief Construct handler in-place with arguments
     * @tparam Args Constructor argument types
     * @param args Arguments to forward to handler constructor
     */
    template<typename... Args>
    explicit message_handler_wrapper(std::in_place_t, Args&&... args)
        : handler_(std::forward<Args>(args)...) {}

    common::VoidResult handle(const message& msg) override {
        return handler_.handle(msg);
    }

    bool can_handle(const message& msg) const override {
        return handler_.can_handle(msg);
    }

    /**
     * @brief Get direct access to the wrapped handler
     * @return Reference to the handler
     */
    Handler& get() { return handler_; }

    /**
     * @brief Get direct access to the wrapped handler (const)
     * @return Const reference to the handler
     */
    [[nodiscard]] const Handler& get() const { return handler_; }

private:
    Handler handler_;
};

/**
 * @brief Create a wrapped CRTP handler
 * @tparam Handler The CRTP handler type
 * @tparam Args Constructor argument types
 * @param args Arguments to forward to handler constructor
 * @return Unique pointer to the wrapped handler
 */
template<typename Handler, typename... Args>
std::unique_ptr<message_handler_interface> make_message_handler(Args&&... args) {
    return std::make_unique<message_handler_wrapper<Handler>>(
        std::in_place, std::forward<Args>(args)...);
}

} // namespace kcenon::messaging
