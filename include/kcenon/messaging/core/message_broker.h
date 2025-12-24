// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#pragma once

#include <kcenon/common/patterns/result.h>

#include "message.h"
#include "topic_router.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace kcenon::messaging {

// =============================================================================
// Forward Declarations
// =============================================================================

class message_broker_impl;

// =============================================================================
// Message Handler Type
// =============================================================================

/**
 * @brief Handler function type for processing messages in a route
 */
using message_handler = std::function<common::VoidResult(const message&)>;

// =============================================================================
// Broker Configuration
// =============================================================================

/**
 * @struct broker_config
 * @brief Configuration for message_broker
 */
struct broker_config {
	/// Maximum number of routes that can be registered
	size_t max_routes = 1000;

	/// Whether to enable statistics collection
	bool enable_statistics = true;

	/// Whether to log routing operations at trace level
	bool enable_trace_logging = false;

	/// Default timeout for route operations (0 = no timeout)
	std::chrono::milliseconds default_timeout{0};
};

// =============================================================================
// Broker Statistics
// =============================================================================

/**
 * @struct broker_statistics
 * @brief Runtime statistics for message_broker
 */
struct broker_statistics {
	/// Total number of messages routed
	uint64_t messages_routed = 0;

	/// Number of messages successfully delivered
	uint64_t messages_delivered = 0;

	/// Number of messages that failed to route
	uint64_t messages_failed = 0;

	/// Number of messages with no matching route
	uint64_t messages_unrouted = 0;

	/// Number of active routes
	uint64_t active_routes = 0;

	/// Timestamp when statistics were last reset
	std::chrono::steady_clock::time_point last_reset;
};

// =============================================================================
// Route Information
// =============================================================================

/**
 * @struct route_info
 * @brief Information about a registered route
 */
struct route_info {
	/// Unique route identifier
	std::string route_id;

	/// Topic pattern for matching (supports wildcards)
	std::string topic_pattern;

	/// Route priority (higher = processed first)
	int priority = 5;

	/// Whether the route is currently active
	bool active = true;

	/// Number of messages processed by this route
	uint64_t messages_processed = 0;
};

// =============================================================================
// Message Broker Class
// =============================================================================

/**
 * @class message_broker
 * @brief Central message routing component with advanced routing capabilities
 *
 * The message_broker provides topic-based message routing with support for:
 * - Wildcard topic patterns (* for single level, # for multi-level)
 * - Priority-based route ordering
 * - Route management (add, remove, enable, disable)
 * - Statistics collection
 *
 * This class integrates with topic_router for pattern matching while providing
 * a higher-level abstraction for route management.
 *
 * @example
 * ```cpp
 * message_broker broker;
 * broker.start();
 *
 * // Add a route for user events
 * broker.add_route("user-handler", "user.*", [](const message& msg) {
 *     // Process user message
 *     return common::ok();
 * });
 *
 * // Route a message
 * message msg("user.created");
 * broker.route(msg);
 *
 * broker.stop();
 * ```
 */
class message_broker {
public:
	/**
	 * @brief Construct a message broker with optional configuration
	 * @param config Broker configuration
	 */
	explicit message_broker(broker_config config = {});

	/**
	 * @brief Destructor - ensures proper cleanup
	 */
	~message_broker();

	// Non-copyable
	message_broker(const message_broker&) = delete;
	message_broker& operator=(const message_broker&) = delete;

	// Movable
	message_broker(message_broker&&) noexcept;
	message_broker& operator=(message_broker&&) noexcept;

	// =========================================================================
	// Lifecycle Management
	// =========================================================================

	/**
	 * @brief Start the message broker
	 * @return Result indicating success or error
	 */
	common::VoidResult start();

	/**
	 * @brief Stop the message broker
	 * @return Result indicating success or error
	 */
	common::VoidResult stop();

	/**
	 * @brief Check if the broker is currently running
	 * @return true if running, false otherwise
	 */
	bool is_running() const;

	// =========================================================================
	// Route Management
	// =========================================================================

	/**
	 * @brief Add a new route
	 * @param route_id Unique identifier for the route
	 * @param topic_pattern Topic pattern to match (supports * and # wildcards)
	 * @param handler Handler function for matched messages
	 * @param priority Route priority (0-10, higher = processed first)
	 * @return Result indicating success or error
	 */
	common::VoidResult add_route(const std::string& route_id,
								 const std::string& topic_pattern,
								 message_handler handler,
								 int priority = 5);

	/**
	 * @brief Remove a route by ID
	 * @param route_id Route identifier to remove
	 * @return Result indicating success or error
	 */
	common::VoidResult remove_route(const std::string& route_id);

	/**
	 * @brief Enable a previously disabled route
	 * @param route_id Route identifier to enable
	 * @return Result indicating success or error
	 */
	common::VoidResult enable_route(const std::string& route_id);

	/**
	 * @brief Disable a route without removing it
	 * @param route_id Route identifier to disable
	 * @return Result indicating success or error
	 */
	common::VoidResult disable_route(const std::string& route_id);

	/**
	 * @brief Check if a route exists
	 * @param route_id Route identifier to check
	 * @return true if route exists, false otherwise
	 */
	bool has_route(const std::string& route_id) const;

	/**
	 * @brief Get information about a route
	 * @param route_id Route identifier
	 * @return Route information or error if not found
	 */
	common::Result<route_info> get_route(const std::string& route_id) const;

	/**
	 * @brief Get all registered routes
	 * @return Vector of route information
	 */
	std::vector<route_info> get_routes() const;

	/**
	 * @brief Get number of registered routes
	 * @return Number of routes
	 */
	size_t route_count() const;

	/**
	 * @brief Clear all routes
	 */
	void clear_routes();

	// =========================================================================
	// Message Routing
	// =========================================================================

	/**
	 * @brief Route a message to matching handlers
	 * @param msg Message to route
	 * @return Result indicating success or error
	 *
	 * The message is matched against all active routes based on topic pattern.
	 * Routes are processed in priority order (highest first).
	 */
	common::VoidResult route(const message& msg);

	// =========================================================================
	// Statistics
	// =========================================================================

	/**
	 * @brief Get current broker statistics
	 * @return Broker statistics snapshot
	 */
	broker_statistics get_statistics() const;

	/**
	 * @brief Reset all statistics to zero
	 */
	void reset_statistics();

private:
	std::unique_ptr<message_broker_impl> impl_;
};

}  // namespace kcenon::messaging
