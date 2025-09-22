#pragma once

/**
 * @file observer_interface.h
 * @brief Observer pattern interface for monitoring system event handling
 *
 * This file defines the core observer pattern interfaces that enable
 * loose coupling between monitoring components through event-driven
 * communication.
 */

#include <chrono>
#include <memory>
#include <string>
#include <variant>
#include "metric_types_adapter.h"
#include "../core/result_types.h"

namespace monitoring_system {

// Forward declarations
class metric_event;
class system_event;
class state_change_event;

/**
 * @class metric_event
 * @brief Event fired when a metric is collected
 */
class metric_event {
public:
    metric_event(const std::string& source, const metric& data)
        : source_(source), metric_data_(data), timestamp_(std::chrono::steady_clock::now()) {}

    const std::string& source() const { return source_; }
    const metric& data() const { return metric_data_; }
    std::chrono::steady_clock::time_point timestamp() const { return timestamp_; }

private:
    std::string source_;
    metric metric_data_;
    std::chrono::steady_clock::time_point timestamp_;
};

/**
 * @class system_event
 * @brief Generic system event for monitoring components
 */
class system_event {
public:
    enum class event_type {
        component_started,
        component_stopped,
        error_occurred,
        warning_raised,
        configuration_changed,
        threshold_exceeded
    };

    system_event(event_type type, const std::string& component, const std::string& message)
        : type_(type), component_(component), message_(message),
          timestamp_(std::chrono::steady_clock::now()) {}

    event_type type() const { return type_; }
    const std::string& component() const { return component_; }
    const std::string& message() const { return message_; }
    std::chrono::steady_clock::time_point timestamp() const { return timestamp_; }

private:
    event_type type_;
    std::string component_;
    std::string message_;
    std::chrono::steady_clock::time_point timestamp_;
};

/**
 * @class state_change_event
 * @brief Event fired when system state changes
 */
class state_change_event {
public:
    enum class state {
        healthy,
        degraded,
        critical,
        unknown
    };

    state_change_event(const std::string& component, state old_state, state new_state)
        : component_(component), old_state_(old_state), new_state_(new_state),
          timestamp_(std::chrono::steady_clock::now()) {}

    const std::string& component() const { return component_; }
    state old_state() const { return old_state_; }
    state new_state() const { return new_state_; }
    std::chrono::steady_clock::time_point timestamp() const { return timestamp_; }

private:
    std::string component_;
    state old_state_;
    state new_state_;
    std::chrono::steady_clock::time_point timestamp_;
};

/**
 * @class interface_monitoring_observer
 * @brief Pure virtual interface for monitoring event observers
 *
 * Components implementing this interface can subscribe to monitoring
 * events and react to metrics, system events, and state changes.
 */
class interface_monitoring_observer {
public:
    virtual ~interface_monitoring_observer() = default;

    /**
     * @brief Called when a metric is collected
     * @param event The metric collection event
     */
    virtual void on_metric_collected(const metric_event& event) = 0;

    /**
     * @brief Called when a system event occurs
     * @param event The system event
     */
    virtual void on_event_occurred(const system_event& event) = 0;

    /**
     * @brief Called when system state changes
     * @param event The state change event
     */
    virtual void on_system_state_changed(const state_change_event& event) = 0;
};

/**
 * @class interface_observable
 * @brief Interface for components that can be observed
 */
class interface_observable {
public:
    virtual ~interface_observable() = default;

    /**
     * @brief Register an observer for events
     * @param observer The observer to register
     * @return Result indicating success or failure
     */
    virtual result_void register_observer(std::shared_ptr<interface_monitoring_observer> observer) = 0;

    /**
     * @brief Unregister an observer
     * @param observer The observer to unregister
     * @return Result indicating success or failure
     */
    virtual result_void unregister_observer(std::shared_ptr<interface_monitoring_observer> observer) = 0;

    /**
     * @brief Notify all observers of a metric event
     * @param event The metric event to broadcast
     */
    virtual void notify_metric(const metric_event& event) = 0;

    /**
     * @brief Notify all observers of a system event
     * @param event The system event to broadcast
     */
    virtual void notify_event(const system_event& event) = 0;

    /**
     * @brief Notify all observers of a state change
     * @param event The state change event to broadcast
     */
    virtual void notify_state_change(const state_change_event& event) = 0;
};

} // namespace monitoring_system