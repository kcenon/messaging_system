#pragma once

/**
 * @file metric_collector_interface.h
 * @brief Abstract interface for metric collection components
 *
 * This file defines the interface for components that collect metrics
 * from various sources and publish them to observers.
 */

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "observer_interface.h"
#include "metric_types_adapter.h"
#include "../core/result_types.h"

namespace monitoring_system {

/**
 * @class metric_filter
 * @brief Filter configuration for metric collection
 */
class metric_filter {
public:
    enum class filter_type {
        include_all,
        include_specific,
        exclude_specific
    };

    metric_filter(filter_type type = filter_type::include_all)
        : type_(type) {}

    void add_metric_type(const std::string& type) {
        metric_types_.push_back(type);
    }

    bool should_collect(const std::string& metric_type) const {
        switch (type_) {
            case filter_type::include_all:
                return true;
            case filter_type::include_specific:
                return std::find(metric_types_.begin(), metric_types_.end(), metric_type) != metric_types_.end();
            case filter_type::exclude_specific:
                return std::find(metric_types_.begin(), metric_types_.end(), metric_type) == metric_types_.end();
        }
        return true;
    }

private:
    filter_type type_;
    std::vector<std::string> metric_types_;
};

/**
 * @class collection_config
 * @brief Configuration for metric collection
 */
struct collection_config {
    std::chrono::milliseconds interval{std::chrono::seconds(1)};
    metric_filter filter;
    bool batch_collection{false};
    size_t batch_size{100};
    bool async_collection{true};

    result_void validate() const {
        if (interval.count() <= 0) {
            return result_void::error(monitoring_error_code::invalid_configuration,
                                    "Collection interval must be positive");
        }
        if (batch_collection && batch_size == 0) {
            return result_void::error(monitoring_error_code::invalid_configuration,
                                    "Batch size must be positive when batch collection is enabled");
        }
        return result_void::success();
    }
};

/**
 * @class interface_metric_collector
 * @brief Pure virtual interface for metric collectors
 *
 * Components implementing this interface can collect various types
 * of metrics and publish them to registered observers.
 */
class interface_metric_collector : public interface_observable {
public:
    virtual ~interface_metric_collector() = default;

    /**
     * @brief Collect metrics based on current configuration
     * @return Result containing collected metrics or error
     */
    virtual result<std::vector<metric>> collect_metrics() = 0;

    /**
     * @brief Start automatic metric collection
     * @param config Collection configuration
     * @return Result indicating success or failure
     */
    virtual result_void start_collection(const collection_config& config) = 0;

    /**
     * @brief Stop automatic metric collection
     * @return Result indicating success or failure
     */
    virtual result_void stop_collection() = 0;

    /**
     * @brief Check if collector is currently active
     * @return True if collecting, false otherwise
     */
    virtual bool is_collecting() const = 0;

    /**
     * @brief Get the types of metrics this collector supports
     * @return Vector of supported metric type names
     */
    virtual std::vector<std::string> get_metric_types() const = 0;

    /**
     * @brief Get current collection configuration
     * @return Current configuration
     */
    virtual collection_config get_config() const = 0;

    /**
     * @brief Update collection configuration
     * @param config New configuration
     * @return Result indicating success or failure
     */
    virtual result_void update_config(const collection_config& config) = 0;

    /**
     * @brief Force immediate metric collection
     * @return Result containing collected metrics or error
     */
    virtual result<std::vector<metric>> force_collect() = 0;

    /**
     * @brief Get collector statistics
     * @return Statistics about collection performance
     */
    virtual metric_stats get_stats() const = 0;

    /**
     * @brief Reset collector statistics
     */
    virtual void reset_stats() = 0;
};

/**
 * @class interface_metric_source
 * @brief Interface for components that provide metrics
 */
class interface_metric_source {
public:
    virtual ~interface_metric_source() = default;

    /**
     * @brief Get current metrics from this source
     * @return Vector of current metrics
     */
    virtual std::vector<metric> get_current_metrics() const = 0;

    /**
     * @brief Get the name of this metric source
     * @return Source name
     */
    virtual std::string get_source_name() const = 0;

    /**
     * @brief Check if this source is healthy
     * @return True if healthy, false otherwise
     */
    virtual bool is_healthy() const = 0;
};

/**
 * @class interface_aggregated_collector
 * @brief Interface for collectors that aggregate metrics from multiple sources
 */
class interface_aggregated_collector : public interface_metric_collector {
public:
    virtual ~interface_aggregated_collector() = default;

    /**
     * @brief Register a metric source
     * @param source The metric source to register
     * @return Result indicating success or failure
     */
    virtual result_void register_source(std::shared_ptr<interface_metric_source> source) = 0;

    /**
     * @brief Unregister a metric source
     * @param source_name The name of the source to unregister
     * @return Result indicating success or failure
     */
    virtual result_void unregister_source(const std::string& source_name) = 0;

    /**
     * @brief Get all registered sources
     * @return Vector of registered source names
     */
    virtual std::vector<std::string> get_registered_sources() const = 0;
};

} // namespace monitoring_system