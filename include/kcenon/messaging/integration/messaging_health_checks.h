// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file messaging_health_checks.h
 * @brief Health check adapters integrating with common_system health monitoring
 *
 * Provides health check implementations that bridge messaging_system's existing
 * health monitoring (message_bus_health_monitor) with the common_system
 * standardized health check interface (health_check, health_monitor).
 *
 * Components:
 * - messaging_health_check:  Overall message bus health via threshold analysis
 * - queue_health_check:      Queue saturation and backpressure monitoring
 * - transport_health_check:  Transport layer connectivity health
 *
 * Integration helpers:
 * - create_messaging_composite_check(): Composite aggregating all checks
 * - register_messaging_health_checks(): Registration with global health monitor
 */

#pragma once

#include <kcenon/messaging/adapters/transport_interface.h>
#include <kcenon/messaging/collectors/message_bus_collector.h>

#include <kcenon/common/interfaces/monitoring/composite_health_check.h>
#include <kcenon/common/interfaces/monitoring/health_check.h>
#include <kcenon/common/interfaces/monitoring/health_monitor.h>

#include <format>
#include <functional>
#include <memory>
#include <string>

namespace kcenon::messaging::integration {

// ============================================================================
// Status Mapping
// ============================================================================

/**
 * @brief Map messaging health status to common_system health status
 *
 * Mapping:
 * - healthy   -> healthy
 * - degraded  -> degraded
 * - unhealthy -> unhealthy
 * - critical  -> unhealthy (common_system has no critical level)
 */
inline common::interfaces::health_status map_health_status(
    collectors::message_bus_health_status status) {
    switch (status) {
        case collectors::message_bus_health_status::healthy:
            return common::interfaces::health_status::healthy;
        case collectors::message_bus_health_status::degraded:
            return common::interfaces::health_status::degraded;
        case collectors::message_bus_health_status::unhealthy:
            return common::interfaces::health_status::unhealthy;
        case collectors::message_bus_health_status::critical:
            return common::interfaces::health_status::unhealthy;
        default:
            return common::interfaces::health_status::unknown;
    }
}

// ============================================================================
// messaging_health_check
// ============================================================================

/**
 * @class messaging_health_check
 * @brief Health check for overall message bus health
 *
 * Delegates to the existing message_bus_health_monitor for threshold-based
 * anomaly detection (queue saturation, failure rates, latency, throughput).
 *
 * Example:
 * @code
 * auto check = std::make_shared<messaging_health_check>(
 *     "primary_bus",
 *     [&bus]() { return bus.get_stats(); });
 *
 * auto result = check->check();
 * if (result.is_healthy()) { ... }
 * @endcode
 */
class messaging_health_check : public common::interfaces::health_check {
public:
    using stats_provider = std::function<collectors::message_bus_stats()>;

    /**
     * @brief Construct a messaging health check
     * @param bus_name Identifier for the message bus being monitored
     * @param provider Function returning current bus statistics
     * @param thresholds Health threshold configuration
     */
    messaging_health_check(
        std::string bus_name,
        stats_provider provider,
        const collectors::message_bus_health_thresholds& thresholds = {})
        : bus_name_(std::move(bus_name))
        , stats_provider_(std::move(provider))
        , monitor_(thresholds) {}

    [[nodiscard]] std::string get_name() const override {
        return std::format("messaging.{}", bus_name_);
    }

    [[nodiscard]] common::interfaces::health_check_type get_type() const override {
        return common::interfaces::health_check_type::readiness;
    }

    common::interfaces::health_check_result check() override {
        common::interfaces::health_check_result result;
        auto start = std::chrono::steady_clock::now();

        if (!stats_provider_) {
            result.status = common::interfaces::health_status::unknown;
            result.message = "No stats provider configured";
            return result;
        }

        auto stats = stats_provider_();
        auto report = monitor_.analyze_health(stats, bus_name_);

        result.status = map_health_status(report.status);

        if (report.issues.empty()) {
            result.message = "Message bus healthy";
        } else {
            result.message = std::format("{} issue(s) detected", report.issues.size());
            for (std::size_t i = 0; i < report.issues.size(); ++i) {
                result.metadata[std::format("issue_{}", i)] = report.issues[i];
            }
        }

        for (const auto& [key, value] : report.metrics) {
            result.metadata[key] = std::format("{:.2f}", value);
        }

        result.metadata["bus_name"] = bus_name_;
        result.metadata["is_running"] = stats.is_running ? "true" : "false";

        auto end = std::chrono::steady_clock::now();
        result.check_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        return result;
    }

private:
    std::string bus_name_;
    stats_provider stats_provider_;
    collectors::message_bus_health_monitor monitor_;
};

// ============================================================================
// queue_health_check
// ============================================================================

/**
 * @class queue_health_check
 * @brief Health check for message queue saturation
 *
 * Monitors queue depth relative to capacity and reports health based
 * on configurable saturation thresholds.
 */
class queue_health_check : public common::interfaces::health_check {
public:
    using stats_provider = std::function<collectors::message_bus_stats()>;

    /**
     * @brief Construct a queue health check
     * @param bus_name Identifier for the message bus
     * @param provider Function returning current bus statistics
     * @param warn_threshold Queue utilization warning threshold (0.0-1.0)
     * @param critical_threshold Queue utilization critical threshold (0.0-1.0)
     */
    queue_health_check(
        std::string bus_name,
        stats_provider provider,
        double warn_threshold = 0.7,
        double critical_threshold = 0.9)
        : bus_name_(std::move(bus_name))
        , stats_provider_(std::move(provider))
        , warn_threshold_(warn_threshold)
        , critical_threshold_(critical_threshold) {}

    [[nodiscard]] std::string get_name() const override {
        return std::format("messaging.{}.queue", bus_name_);
    }

    [[nodiscard]] common::interfaces::health_check_type get_type() const override {
        return common::interfaces::health_check_type::readiness;
    }

    [[nodiscard]] bool is_critical() const override { return false; }

    common::interfaces::health_check_result check() override {
        common::interfaces::health_check_result result;
        auto start = std::chrono::steady_clock::now();

        if (!stats_provider_) {
            result.status = common::interfaces::health_status::unknown;
            result.message = "No stats provider configured";
            return result;
        }

        auto stats = stats_provider_();
        double utilization = stats.queue_utilization_percent / 100.0;

        if (utilization >= critical_threshold_) {
            result.status = common::interfaces::health_status::unhealthy;
            result.message = std::format(
                "Queue critically saturated: {:.1f}%", stats.queue_utilization_percent);
        } else if (utilization >= warn_threshold_) {
            result.status = common::interfaces::health_status::degraded;
            result.message = std::format(
                "Queue nearing capacity: {:.1f}%", stats.queue_utilization_percent);
        } else {
            result.status = common::interfaces::health_status::healthy;
            result.message = std::format(
                "Queue utilization normal: {:.1f}%", stats.queue_utilization_percent);
        }

        result.metadata["queue_depth"] = std::to_string(stats.queue_depth);
        result.metadata["queue_capacity"] = std::to_string(stats.queue_capacity);
        result.metadata["utilization_percent"] =
            std::format("{:.2f}", stats.queue_utilization_percent);

        auto end = std::chrono::steady_clock::now();
        result.check_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        return result;
    }

private:
    std::string bus_name_;
    stats_provider stats_provider_;
    double warn_threshold_;
    double critical_threshold_;
};

// ============================================================================
// transport_health_check
// ============================================================================

/**
 * @class transport_health_check
 * @brief Health check for transport layer connectivity
 *
 * Monitors the connection state of a transport interface and reports
 * health accordingly.
 */
class transport_health_check : public common::interfaces::health_check {
public:
    /**
     * @brief Construct a transport health check
     * @param name Transport identifier
     * @param transport Transport interface to monitor
     */
    transport_health_check(
        std::string name,
        std::shared_ptr<adapters::transport_interface> transport)
        : name_(std::move(name))
        , transport_(std::move(transport)) {}

    [[nodiscard]] std::string get_name() const override {
        return std::format("messaging.transport.{}", name_);
    }

    [[nodiscard]] common::interfaces::health_check_type get_type() const override {
        return common::interfaces::health_check_type::dependency;
    }

    common::interfaces::health_check_result check() override {
        common::interfaces::health_check_result result;
        auto start = std::chrono::steady_clock::now();

        if (!transport_) {
            result.status = common::interfaces::health_status::unknown;
            result.message = "No transport configured";
            return result;
        }

        auto state = transport_->get_state();
        switch (state) {
            case adapters::transport_state::connected:
                result.status = common::interfaces::health_status::healthy;
                result.message = "Transport connected";
                break;
            case adapters::transport_state::connecting:
            case adapters::transport_state::disconnecting:
                result.status = common::interfaces::health_status::degraded;
                result.message = std::format("Transport in transition: {}",
                    state == adapters::transport_state::connecting
                        ? "connecting" : "disconnecting");
                break;
            case adapters::transport_state::disconnected:
                result.status = common::interfaces::health_status::unhealthy;
                result.message = "Transport disconnected";
                break;
            case adapters::transport_state::error:
                result.status = common::interfaces::health_status::unhealthy;
                result.message = "Transport error";
                break;
        }

        auto transport_stats = transport_->get_statistics();
        result.metadata["transport_name"] = name_;
        result.metadata["messages_sent"] =
            std::to_string(transport_stats.messages_sent);
        result.metadata["messages_received"] =
            std::to_string(transport_stats.messages_received);
        result.metadata["errors"] = std::to_string(transport_stats.errors);

        auto end = std::chrono::steady_clock::now();
        result.check_duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        return result;
    }

private:
    std::string name_;
    std::shared_ptr<adapters::transport_interface> transport_;
};

// ============================================================================
// Composite and Registration Helpers
// ============================================================================

/**
 * @brief Create a composite health check aggregating all messaging components
 * @param bus_name Message bus identifier
 * @param stats_provider Function returning current bus statistics
 * @param transports Optional map of transport name to transport interface
 * @param thresholds Health threshold configuration
 * @return Shared pointer to the composite health check
 */
inline std::shared_ptr<common::interfaces::composite_health_check>
create_messaging_composite_check(
    const std::string& bus_name,
    std::function<collectors::message_bus_stats()> stats_provider,
    const std::unordered_map<std::string,
        std::shared_ptr<adapters::transport_interface>>& transports = {},
    const collectors::message_bus_health_thresholds& thresholds = {}) {

    auto composite = std::make_shared<common::interfaces::composite_health_check>(
        std::format("messaging.{}.composite", bus_name),
        common::interfaces::health_check_type::readiness);

    // Add main bus health check
    composite->add_check(std::make_shared<messaging_health_check>(
        bus_name, stats_provider, thresholds));

    // Add queue health check
    composite->add_check(std::make_shared<queue_health_check>(
        bus_name, stats_provider,
        thresholds.queue_saturation_warn,
        thresholds.queue_saturation_critical));

    // Add transport health checks
    for (const auto& [name, transport] : transports) {
        composite->add_check(
            std::make_shared<transport_health_check>(name, transport));
    }

    return composite;
}

/**
 * @brief Register messaging health checks with the global health monitor
 * @param bus_name Message bus identifier
 * @param stats_provider Function returning current bus statistics
 * @param transports Optional map of transport name to transport interface
 * @param thresholds Health threshold configuration
 * @return Result indicating success or failure
 */
inline common::Result<bool> register_messaging_health_checks(
    const std::string& bus_name,
    std::function<collectors::message_bus_stats()> stats_provider,
    const std::unordered_map<std::string,
        std::shared_ptr<adapters::transport_interface>>& transports = {},
    const collectors::message_bus_health_thresholds& thresholds = {}) {

    auto& monitor = common::interfaces::global_health_monitor();

    // Register main bus check
    auto bus_check = std::make_shared<messaging_health_check>(
        bus_name, stats_provider, thresholds);
    auto bus_result = monitor.register_check(bus_check->get_name(), bus_check);
    if (bus_result.is_err()) {
        return bus_result;
    }

    // Register queue check
    auto queue_check_ptr = std::make_shared<queue_health_check>(
        bus_name, stats_provider,
        thresholds.queue_saturation_warn,
        thresholds.queue_saturation_critical);
    auto queue_result = monitor.register_check(
        queue_check_ptr->get_name(), queue_check_ptr);
    if (queue_result.is_err()) {
        return queue_result;
    }

    // Set up dependency: queue check depends on bus check
    monitor.add_dependency(queue_check_ptr->get_name(), bus_check->get_name());

    // Register transport checks
    for (const auto& [name, transport] : transports) {
        auto transport_check_ptr =
            std::make_shared<transport_health_check>(name, transport);
        auto transport_result = monitor.register_check(
            transport_check_ptr->get_name(), transport_check_ptr);
        if (transport_result.is_err()) {
            return transport_result;
        }

        // Transport depends on bus being operational
        monitor.add_dependency(
            transport_check_ptr->get_name(), bus_check->get_name());
    }

    return common::Result<bool>::ok(true);
}

}  // namespace kcenon::messaging::integration
