// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#pragma once

#include <kcenon/common/patterns/result.h>
#include <memory>

namespace kcenon::messaging {

// Forward declarations
class message;

/**
 * @interface message_handler_interface
 * @brief Abstract interface for message handlers
 *
 * This interface defines the contract for message processing components.
 * Implementations can be used with message bus, topic router, and other
 * messaging components to handle messages in a decoupled manner.
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

} // namespace kcenon::messaging
