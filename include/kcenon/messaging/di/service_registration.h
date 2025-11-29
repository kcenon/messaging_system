// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file service_registration.h
 * @brief Service registration for messaging_system with common_system DI container
 *
 * This header provides functions to register messaging_system components
 * with the common_system dependency injection container.
 */

#pragma once

#include <kcenon/common/di/service_container.h>
#include <kcenon/common/patterns/result.h>
#include "../core/message_bus.h"
#include "../backends/standalone_backend.h"
#include "../integration/event_bridge.h"
#include "../integration/executor_adapter.h"
#include <memory>

namespace kcenon::messaging::di {

/**
 * @brief Interface for message bus (for DI registration)
 */
class IMessageBus {
public:
    virtual ~IMessageBus() = default;
    virtual common::VoidResult start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    virtual common::VoidResult publish(message msg) = 0;
    virtual size_t worker_count() const = 0;
};

/**
 * @brief Message bus wrapper implementing IMessageBus interface
 */
class message_bus_wrapper : public IMessageBus {
public:
    explicit message_bus_wrapper(std::shared_ptr<message_bus> bus)
        : bus_(std::move(bus)) {}

    common::VoidResult start() override {
        return bus_->start();
    }

    void stop() override {
        bus_->stop();
    }

    bool is_running() const override {
        return bus_->is_running();
    }

    common::VoidResult publish(message msg) override {
        return bus_->publish(std::move(msg));
    }

    size_t worker_count() const override {
        return bus_->worker_count();
    }

    std::shared_ptr<message_bus> get_bus() const {
        return bus_;
    }

private:
    std::shared_ptr<message_bus> bus_;
};

/**
 * @struct messaging_config
 * @brief Configuration for messaging service registration
 */
struct messaging_config {
    size_t worker_threads = 4;
    size_t queue_capacity = 1000;
    bool enable_event_bridge = true;
};

/**
 * @brief Register messaging services with the DI container
 *
 * Registers the following services:
 * - IMessageBus (singleton): Main message bus instance
 * - messaging_event_bridge (singleton): Event bridge (if enabled)
 *
 * Example usage:
 * @code
 * auto& container = common::di::service_container::global();
 *
 * messaging_config config;
 * config.worker_threads = 8;
 *
 * auto result = register_messaging_services(container, config);
 * if (result.is_ok()) {
 *     auto bus = container.resolve<IMessageBus>();
 * }
 * @endcode
 *
 * @param container The service container to register with
 * @param config Configuration for messaging services
 * @return VoidResult indicating success or error
 */
inline common::VoidResult register_messaging_services(
    common::di::IServiceContainer& container,
    const messaging_config& config = {}) {

    // Register message bus as singleton
    auto bus_result = container.register_factory<IMessageBus>(
        [config](common::di::IServiceContainer&) -> std::shared_ptr<IMessageBus> {
            // Create backend
            auto backend = std::make_shared<standalone_backend>(config.worker_threads);

            // Configure message bus
            message_bus_config bus_config;
            bus_config.worker_threads = config.worker_threads;
            bus_config.queue_capacity = config.queue_capacity;

            auto bus = std::make_shared<message_bus>(backend, bus_config);

            return std::make_shared<message_bus_wrapper>(bus);
        },
        common::di::service_lifetime::singleton
    );

    if (bus_result.is_err()) {
        return bus_result;
    }

    // Register event bridge if enabled
    if (config.enable_event_bridge) {
        auto bridge_result = container.register_factory<integration::messaging_event_bridge>(
            [](common::di::IServiceContainer& c) -> std::shared_ptr<integration::messaging_event_bridge> {
                auto bus_wrapper = c.resolve<IMessageBus>();
                if (bus_wrapper.is_err()) {
                    return nullptr;
                }

                auto wrapper = std::dynamic_pointer_cast<message_bus_wrapper>(bus_wrapper.value());
                if (!wrapper) {
                    return nullptr;
                }

                return std::make_shared<integration::messaging_event_bridge>(wrapper->get_bus());
            },
            common::di::service_lifetime::singleton
        );

        if (bridge_result.is_err()) {
            return bridge_result;
        }
    }

    return common::ok();
}

/**
 * @brief Register executor-based message handler
 *
 * Registers executor_message_handler if an IExecutor is available in the container.
 *
 * @param container The service container
 * @return VoidResult indicating success or error
 */
inline common::VoidResult register_executor_handler(
    common::di::IServiceContainer& container) {

    return container.register_factory<integration::executor_message_handler>(
        [](common::di::IServiceContainer& c) -> std::shared_ptr<integration::executor_message_handler> {
            auto executor_result = c.resolve<common::interfaces::IExecutor>();
            if (executor_result.is_err()) {
                // No executor available, return null handler
                return std::make_shared<integration::executor_message_handler>(nullptr);
            }

            return std::make_shared<integration::executor_message_handler>(
                executor_result.value());
        },
        common::di::service_lifetime::singleton
    );
}

/**
 * @brief Unregister all messaging services
 *
 * @param container The service container
 * @return VoidResult indicating success or error
 */
inline common::VoidResult unregister_messaging_services(
    common::di::IServiceContainer& container) {

    // Unregister in reverse order of registration
    container.unregister<integration::executor_message_handler>();
    container.unregister<integration::messaging_event_bridge>();
    container.unregister<IMessageBus>();

    return common::ok();
}

}  // namespace kcenon::messaging::di
