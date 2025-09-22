/**
 * @file monitoring_factory.h
 * @brief Factory for creating monitoring instances
 */

#pragma once

#include <kcenon/logger/core/monitoring/monitoring_interface.h>
#include <memory>

namespace kcenon::logger::monitoring {

/**
 * @brief Factory class for creating monitoring instances
 */
class monitoring_factory {
public:
    /**
     * @brief Monitor type enumeration
     */
    enum class monitor_type {
        automatic,
        basic,
        advanced
    };
    /**
     * @brief Create a default monitoring instance
     * @return Shared pointer to monitoring interface
     */
    static std::shared_ptr<monitoring_interface> create_default() {
        return std::make_shared<basic_monitoring>();
    }

    /**
     * @brief Create a monitoring instance with specific configuration
     * @param config Configuration parameters for monitoring
     * @return Shared pointer to monitoring interface
     */
    template<typename ConfigType>
    static std::shared_ptr<monitoring_interface> create_with_config(const ConfigType& config) {
        (void)config; // Suppress unused parameter warning
        return create_default();
    }

    /**
     * @brief Get the global monitoring instance
     * @return Reference to the global monitoring instance
     */
    static monitoring_interface& get_global_monitoring() {
        static basic_monitoring global_instance;
        return global_instance;
    }

    /**
     * @brief Create a monitoring instance with specific health thresholds
     * @param error_rate_threshold Error rate threshold for warning status
     * @param queue_utilization_threshold Queue utilization threshold for critical status
     * @return Shared pointer to monitoring interface
     */
    static std::shared_ptr<monitoring_interface> create_with_thresholds(
        double error_rate_threshold = 0.1,
        double queue_utilization_threshold = 0.9) {

        auto monitor = create_default();
        monitor->set_metric("error_rate_threshold", error_rate_threshold);
        monitor->set_metric("queue_utilization_threshold", queue_utilization_threshold);
        return monitor;
    }
};

} // namespace kcenon::logger::monitoring