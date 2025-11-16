#pragma once

#include <kcenon/common/patterns/result.h>
#include "../core/message.h"
#include <chrono>
#include <cstddef>

namespace kcenon::messaging {

/**
 * @interface queue_interface
 * @brief Abstract interface for message queues
 */
class queue_interface {
public:
	virtual ~queue_interface() = default;

	/**
	 * @brief Enqueue a message
	 * @param msg Message to enqueue
	 * @return Result indicating success or error
	 */
	virtual common::VoidResult enqueue(message msg) = 0;

	/**
	 * @brief Dequeue a message (blocking)
	 * @param timeout Maximum time to wait
	 * @return Result containing message or error
	 */
	virtual common::Result<message> dequeue(
		std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
	) = 0;

	/**
	 * @brief Try to dequeue (non-blocking)
	 * @return Result containing message or empty if queue is empty
	 */
	virtual common::Result<message> try_dequeue() = 0;

	/**
	 * @brief Get current queue size
	 */
	virtual size_t size() const = 0;

	/**
	 * @brief Check if queue is empty
	 */
	virtual bool empty() const = 0;

	/**
	 * @brief Clear all messages
	 */
	virtual void clear() = 0;
};

}  // namespace kcenon::messaging
