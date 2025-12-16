// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file messaging_bootstrapper.h
 * @brief Integration module for UnifiedBootstrapper with messaging_system
 *
 * This header provides seamless integration between messaging_system and
 * common_system's UnifiedBootstrapper, enabling unified initialization
 * and shutdown coordination.
 *
 * Thread Safety:
 * - All functions are thread-safe when used with UnifiedBootstrapper.
 * - Shutdown hooks are properly coordinated with the bootstrapper lifecycle.
 *
 * @see unified_bootstrapper.h for the base bootstrapper implementation.
 * @see service_registration.h for low-level service registration.
 */

#pragma once

#include <kcenon/common/di/unified_bootstrapper.h>
#include "service_registration.h"
#include <atomic>
#include <memory>

namespace kcenon::messaging::di {

/**
 * @struct messaging_bootstrapper_options
 * @brief Extended configuration options for messaging with bootstrapper
 *
 * Extends the basic messaging_config with additional options specific
 * to UnifiedBootstrapper integration.
 */
struct messaging_bootstrapper_options {
    /// Core messaging configuration
    messaging_config config;

    /// Automatically start the message bus after registration
    bool auto_start = true;

    /// Register executor handler if IExecutor is available
    bool use_executor = true;

    /// Shutdown hook name for the message bus
    std::string shutdown_hook_name = "messaging_system";
};

/**
 * @class messaging_bootstrapper
 * @brief Helper class for integrating messaging_system with UnifiedBootstrapper
 *
 * This class provides convenient methods to register messaging services with
 * the unified bootstrapper and set up proper shutdown coordination.
 *
 * Usage Example (Basic):
 * @code
 * // Initialize unified bootstrapper first
 * auto init_result = common::di::unified_bootstrapper::initialize({
 *     .enable_logging = true,
 *     .register_signal_handlers = true
 * });
 *
 * // Then register messaging services
 * auto msg_result = messaging_bootstrapper::integrate({
 *     .config = {
 *         .worker_threads = 8,
 *         .queue_capacity = 2000,
 *         .enable_event_bridge = true
 *     },
 *     .auto_start = true
 * });
 *
 * // Get and use the message bus
 * auto bus = messaging_bootstrapper::get_message_bus();
 * if (bus) {
 *     bus->publish(some_message);
 * }
 *
 * // Shutdown is handled automatically via hooks
 * common::di::unified_bootstrapper::shutdown();
 * @endcode
 *
 * Usage Example (Fluent Builder):
 * @code
 * auto result = messaging_bootstrapper::builder()
 *     .with_worker_threads(8)
 *     .with_queue_capacity(2000)
 *     .with_event_bridge(true)
 *     .with_auto_start(true)
 *     .integrate();
 * @endcode
 */
class messaging_bootstrapper {
public:
    // Prevent instantiation
    messaging_bootstrapper() = delete;
    ~messaging_bootstrapper() = delete;
    messaging_bootstrapper(const messaging_bootstrapper&) = delete;
    messaging_bootstrapper& operator=(const messaging_bootstrapper&) = delete;

    /**
     * @brief Integrate messaging_system with UnifiedBootstrapper
     *
     * Performs the following steps:
     * 1. Registers messaging services with the service container
     * 2. Registers executor handler (if enabled and executor available)
     * 3. Sets up shutdown hook for graceful cleanup
     * 4. Optionally starts the message bus
     *
     * @param opts Configuration options
     * @return VoidResult indicating success or error
     *
     * Possible errors:
     * - NOT_INITIALIZED: UnifiedBootstrapper not initialized
     * - ALREADY_EXISTS: Messaging services already registered
     * - INTERNAL_ERROR: Service registration failed
     */
    static common::VoidResult integrate(
        const messaging_bootstrapper_options& opts = {});

    /**
     * @brief Remove messaging_system from UnifiedBootstrapper
     *
     * Unregisters all messaging services and removes shutdown hooks.
     * This is typically not needed as shutdown() handles cleanup.
     *
     * @return VoidResult indicating success or error
     */
    static common::VoidResult remove();

    /**
     * @brief Check if messaging is integrated
     *
     * @return true if messaging services are registered
     */
    static bool is_integrated();

    /**
     * @brief Get the registered message bus
     *
     * Convenience method to resolve IMessageBus from the container.
     *
     * @return Shared pointer to IMessageBus, or nullptr if not available
     */
    static std::shared_ptr<IMessageBus> get_message_bus();

    /**
     * @brief Get the event bridge
     *
     * Convenience method to resolve event bridge from the container.
     *
     * @return Shared pointer to messaging_event_bridge, or nullptr if not available
     */
    static std::shared_ptr<integration::messaging_event_bridge> get_event_bridge();

    /**
     * @brief Get current integration options
     *
     * @return Current options, or default options if not integrated
     */
    static messaging_bootstrapper_options get_options();

    // ========================================================================
    // Builder API
    // ========================================================================

    /**
     * @class builder
     * @brief Fluent builder for messaging bootstrapper configuration
     *
     * Provides a fluent interface for configuring and integrating
     * messaging_system with the UnifiedBootstrapper.
     */
    class builder {
    public:
        builder() = default;

        /**
         * @brief Set the number of worker threads
         * @param threads Number of worker threads
         * @return Reference to this builder
         */
        builder& with_worker_threads(size_t threads) {
            opts_.config.worker_threads = threads;
            return *this;
        }

        /**
         * @brief Set the message queue capacity
         * @param capacity Maximum queue capacity
         * @return Reference to this builder
         */
        builder& with_queue_capacity(size_t capacity) {
            opts_.config.queue_capacity = capacity;
            return *this;
        }

        /**
         * @brief Enable or disable the event bridge
         * @param enable true to enable event bridge
         * @return Reference to this builder
         */
        builder& with_event_bridge(bool enable = true) {
            opts_.config.enable_event_bridge = enable;
            return *this;
        }

        /**
         * @brief Enable or disable auto-start
         * @param enable true to auto-start message bus after registration
         * @return Reference to this builder
         */
        builder& with_auto_start(bool enable = true) {
            opts_.auto_start = enable;
            return *this;
        }

        /**
         * @brief Enable or disable executor integration
         * @param enable true to use IExecutor if available
         * @return Reference to this builder
         */
        builder& with_executor(bool enable = true) {
            opts_.use_executor = enable;
            return *this;
        }

        /**
         * @brief Set custom shutdown hook name
         * @param name Hook name for identification
         * @return Reference to this builder
         */
        builder& with_shutdown_hook_name(const std::string& name) {
            opts_.shutdown_hook_name = name;
            return *this;
        }

        /**
         * @brief Apply full messaging configuration
         * @param config Messaging configuration
         * @return Reference to this builder
         */
        builder& with_config(const messaging_config& config) {
            opts_.config = config;
            return *this;
        }

        /**
         * @brief Get the built options
         * @return Configuration options
         */
        messaging_bootstrapper_options build() const {
            return opts_;
        }

        /**
         * @brief Build and integrate in one step
         * @return VoidResult indicating success or error
         */
        common::VoidResult integrate() const {
            return messaging_bootstrapper::integrate(opts_);
        }

    private:
        messaging_bootstrapper_options opts_;
    };

private:
    /**
     * @brief Set up shutdown hook for message bus cleanup
     */
    static common::VoidResult setup_shutdown_hook(const std::string& hook_name);

    /**
     * @brief Handle shutdown - stop message bus and cleanup
     */
    static void shutdown_handler(std::chrono::milliseconds remaining_timeout);

    // State
    static std::atomic<bool> integrated_;
    static messaging_bootstrapper_options options_;
    static std::mutex mutex_;
};

// ============================================================================
// Implementation
// ============================================================================

inline std::atomic<bool> messaging_bootstrapper::integrated_{false};
inline messaging_bootstrapper_options messaging_bootstrapper::options_;
inline std::mutex messaging_bootstrapper::mutex_;

inline common::VoidResult messaging_bootstrapper::integrate(
    const messaging_bootstrapper_options& opts) {

    std::lock_guard<std::mutex> lock(mutex_);

    // Check if UnifiedBootstrapper is initialized
    if (!common::di::unified_bootstrapper::is_initialized()) {
        return common::make_error<std::monostate>(
            common::error_codes::NOT_INITIALIZED,
            "UnifiedBootstrapper is not initialized. Call "
            "unified_bootstrapper::initialize() first.",
            "messaging::bootstrapper"
        );
    }

    // Check if already integrated
    if (integrated_.load()) {
        return common::make_error<std::monostate>(
            common::error_codes::ALREADY_EXISTS,
            "Messaging services are already integrated",
            "messaging::bootstrapper"
        );
    }

    // Store options
    options_ = opts;

    // Get service container
    auto& container = common::di::unified_bootstrapper::services();

    // Register messaging services
    auto reg_result = register_messaging_services(container, opts.config);
    if (reg_result.is_err()) {
        return reg_result;
    }

    // Register executor handler if enabled
    if (opts.use_executor) {
        auto exec_result = register_executor_handler(container);
        // Executor registration failure is non-fatal (executor may not be available)
        (void)exec_result;
    }

    // Set up shutdown hook
    auto hook_result = setup_shutdown_hook(opts.shutdown_hook_name);
    if (hook_result.is_err()) {
        // Cleanup on failure
        unregister_messaging_services(container);
        return hook_result;
    }

    // Auto-start message bus if enabled
    if (opts.auto_start) {
        auto bus_result = container.resolve<IMessageBus>();
        if (bus_result.is_ok()) {
            auto start_result = bus_result.value()->start();
            if (start_result.is_err()) {
                // Cleanup on failure
                common::di::unified_bootstrapper::unregister_shutdown_hook(
                    opts.shutdown_hook_name);
                unregister_messaging_services(container);
                return start_result;
            }
        }
    }

    integrated_.store(true);
    return common::ok();
}

inline common::VoidResult messaging_bootstrapper::remove() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!integrated_.load()) {
        return common::make_error<std::monostate>(
            common::error_codes::NOT_FOUND,
            "Messaging services are not integrated",
            "messaging::bootstrapper"
        );
    }

    // Stop message bus if running
    auto bus = get_message_bus();
    if (bus && bus->is_running()) {
        bus->stop();
    }

    // Unregister shutdown hook
    common::di::unified_bootstrapper::unregister_shutdown_hook(
        options_.shutdown_hook_name);

    // Unregister services
    if (common::di::unified_bootstrapper::is_initialized()) {
        auto& container = common::di::unified_bootstrapper::services();
        unregister_messaging_services(container);
    }

    integrated_.store(false);
    options_ = messaging_bootstrapper_options{};

    return common::ok();
}

inline bool messaging_bootstrapper::is_integrated() {
    return integrated_.load();
}

inline std::shared_ptr<IMessageBus> messaging_bootstrapper::get_message_bus() {
    if (!integrated_.load()) {
        return nullptr;
    }

    try {
        auto& container = common::di::unified_bootstrapper::services();
        auto result = container.resolve<IMessageBus>();
        if (result.is_ok()) {
            return result.value();
        }
    } catch (...) {
        // Bootstrapper not initialized
    }

    return nullptr;
}

inline std::shared_ptr<integration::messaging_event_bridge>
messaging_bootstrapper::get_event_bridge() {
    if (!integrated_.load()) {
        return nullptr;
    }

    try {
        auto& container = common::di::unified_bootstrapper::services();
        auto result = container.resolve<integration::messaging_event_bridge>();
        if (result.is_ok()) {
            return result.value();
        }
    } catch (...) {
        // Bootstrapper not initialized
    }

    return nullptr;
}

inline messaging_bootstrapper_options messaging_bootstrapper::get_options() {
    std::lock_guard<std::mutex> lock(mutex_);
    return options_;
}

inline common::VoidResult messaging_bootstrapper::setup_shutdown_hook(
    const std::string& hook_name) {

    return common::di::unified_bootstrapper::register_shutdown_hook(
        hook_name,
        shutdown_handler
    );
}

inline void messaging_bootstrapper::shutdown_handler(
    std::chrono::milliseconds remaining_timeout) {

    (void)remaining_timeout;  // May be used for graceful shutdown timeout

    // Stop message bus gracefully
    auto bus = get_message_bus();
    if (bus && bus->is_running()) {
        bus->stop();
    }

    // Stop event bridge if running
    auto bridge = get_event_bridge();
    if (bridge && bridge->is_running()) {
        bridge->stop();
    }

    // Mark as not integrated (services will be cleared by bootstrapper)
    integrated_.store(false);
}

}  // namespace kcenon::messaging::di
