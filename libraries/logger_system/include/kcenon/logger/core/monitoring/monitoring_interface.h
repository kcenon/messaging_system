/**
 * @file monitoring_interface.h
 * @brief Interface for logger monitoring and health checks
 */

#pragma once

#include <string>
#include <unordered_map>
#include <chrono>

namespace kcenon::logger::monitoring {

/**
 * @brief Health status enumeration
 */
enum class health_status {
    healthy,
    warning,
    critical,
    unknown
};

/**
 * @brief Monitoring data structure
 */
struct monitoring_data {
    health_status status{health_status::unknown};
    std::unordered_map<std::string, double> metrics;
    std::unordered_map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point timestamp{std::chrono::system_clock::now()};
    std::string status_message;
};

/**
 * @brief Interface for logger monitoring
 */
class monitoring_interface {
public:
    virtual ~monitoring_interface() = default;

    /**
     * @brief Get current monitoring data
     * @return Current monitoring data snapshot
     */
    virtual monitoring_data get_monitoring_data() const = 0;

    /**
     * @brief Check if the logger is healthy
     * @return True if healthy, false otherwise
     */
    virtual bool is_healthy() const = 0;

    /**
     * @brief Get health status
     * @return Current health status
     */
    virtual health_status get_health_status() const = 0;

    /**
     * @brief Set a custom metric
     * @param name Metric name
     * @param value Metric value
     */
    virtual void set_metric(const std::string& name, double value) = 0;

    /**
     * @brief Get a specific metric value
     * @param name Metric name
     * @return Metric value, or 0.0 if not found
     */
    virtual double get_metric(const std::string& name) const = 0;

    /**
     * @brief Reset all monitoring data
     */
    virtual void reset() = 0;
};

/**
 * @brief Basic monitoring implementation
 */
class basic_monitoring : public monitoring_interface {
private:
    mutable monitoring_data data_;

public:
    monitoring_data get_monitoring_data() const override {
        data_.timestamp = std::chrono::system_clock::now();
        return data_;
    }

    bool is_healthy() const override {
        return data_.status == health_status::healthy;
    }

    health_status get_health_status() const override {
        return data_.status;
    }

    void set_metric(const std::string& name, double value) override {
        data_.metrics[name] = value;

        // Auto-update health status based on metrics
        if (name == "error_rate" && value > 0.1) {
            data_.status = health_status::warning;
        } else if (name == "queue_utilization" && value > 0.9) {
            data_.status = health_status::critical;
        } else if (data_.status == health_status::unknown) {
            data_.status = health_status::healthy;
        }
    }

    double get_metric(const std::string& name) const override {
        auto it = data_.metrics.find(name);
        return it != data_.metrics.end() ? it->second : 0.0;
    }

    void reset() override {
        data_ = monitoring_data{};
    }
};

} // namespace kcenon::logger::monitoring