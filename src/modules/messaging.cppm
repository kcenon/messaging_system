// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file messaging.cppm
 * @brief Primary C++20 module for messaging_system.
 *
 * This is the main module interface for the messaging_system library.
 * It aggregates all module partitions to provide a single import point.
 *
 * Usage:
 * @code
 * import kcenon.messaging;
 *
 * using namespace kcenon::messaging;
 *
 * // Build and send a message
 * auto result = message_builder()
 *     .topic("orders.created")
 *     .type(message_type::event)
 *     .priority(message_priority::high)
 *     .build();
 *
 * if (result.has_value()) {
 *     auto& bus = message_bus::instance();
 *     bus.publish(result.value());
 * }
 *
 * // Pub/Sub pattern
 * pub_sub_broker broker;
 * broker.subscribe("orders.*", [](const message& msg) {
 *     std::cout << "Received: " << msg.metadata().topic << std::endl;
 * });
 *
 * // Distributed task queue
 * using namespace kcenon::messaging::task;
 * auto task_queue = std::make_unique<task_queue>();
 * auto async_result = task_queue->enqueue(my_task);
 * auto result = async_result.get();
 * @endcode
 *
 * Module Structure:
 * - kcenon.messaging:core - Core messaging (message, queue, bus, broker)
 * - kcenon.messaging:patterns - Messaging patterns (pub/sub, request/reply)
 * - kcenon.messaging:task - Distributed task queue system
 * - kcenon.messaging:integration - Transports, backends, DI
 *
 * Dependencies:
 * - kcenon.common (Tier 0) - Result<T>, error handling, interfaces
 * - kcenon.thread (Tier 1) - Thread pool integration
 * - kcenon.container (Tier 1) - Value container serialization
 * - kcenon.logger (Tier 2) - Logging integration
 * - kcenon.database (Tier 3) - Persistence (optional)
 * - kcenon.network (Tier 4) - Network transports (optional)
 */

export module kcenon.messaging;

// Import and re-export dependent modules
export import kcenon.common;
export import kcenon.thread;
export import kcenon.container;
export import kcenon.network;
export import kcenon.monitoring;

// Tier 1: Core messaging infrastructure
export import :core;

// Tier 2: Messaging patterns
export import :patterns;

// Tier 3: Distributed task queue
export import :task;

// Tier 4: Integration (transports, backends, DI)
export import :integration;

// =============================================================================
// Module-Level API
// =============================================================================

export namespace kcenon::messaging {

/**
 * @brief Version information for messaging_system module.
 */
struct module_version {
    static constexpr int major = 0;
    static constexpr int minor = 1;
    static constexpr int patch = 0;
    static constexpr int tweak = 0;
    static constexpr const char* string = "0.1.0.0";
    static constexpr const char* module_name = "kcenon.messaging";
};

/**
 * @brief Initialize the messaging system with default configuration
 * @return VoidResult - ok() on success, error on failure
 *
 * Possible errors:
 * - already_exists: Messaging system already initialized
 * - internal_error: System initialization failed
 */
kcenon::common::VoidResult initialize();

/**
 * @brief Shutdown the messaging system
 * @return VoidResult - ok() on success, error on failure
 *
 * Possible errors:
 * - not_initialized: Messaging system not initialized
 * - internal_error: Shutdown failed
 */
kcenon::common::VoidResult shutdown();

/**
 * @brief Check if messaging system is initialized
 * @return true if initialized, false otherwise
 */
bool is_initialized() noexcept;

/**
 * @brief Get the messaging system version string
 * @return Version string in "major.minor.patch.tweak" format
 */
constexpr const char* version() noexcept {
    return module_version::string;
}

} // namespace kcenon::messaging
