#pragma once

// Error boundary implementation - stub for compatibility
#include <chrono>
#include <functional>
#include <string>
#include <atomic>
#include "monitoring/core/result_types.h"

namespace monitoring_system {

/**
 * @brief Degradation levels
 */
enum class degradation_level {
    none = 0,
    low = 1,
    medium = 2,
    high = 3,
    critical = 4
};

/**
 * @brief Error boundary configuration
 */
struct error_boundary_config {
    size_t error_threshold = 5;
    std::chrono::seconds error_window = std::chrono::seconds(60);
    bool enable_fallback_logging = true;
    degradation_level max_degradation = degradation_level::high;
};

/**
 * @brief Error boundary metrics
 */
struct error_boundary_metrics {
    size_t total_operations = 0;
    size_t failed_operations = 0;
    size_t recovered_operations = 0;
};

/**
 * @brief Basic error boundary implementation - stub
 */
template<typename T = void>
class error_boundary {
public:
    using config = error_boundary_config;

    error_boundary() : config_() {}
    explicit error_boundary(const std::string& name) : name_(name), config_() {}
    explicit error_boundary(const std::string& name, const config& cfg) : name_(name), config_(cfg) {}

    template<typename Func>
    auto execute(Func&& func) -> decltype(func()) {
        metrics_.total_operations++;
        try {
            return func();
        } catch (...) {
            metrics_.failed_operations++;
            throw;
        }
    }

    void set_error_handler(std::function<void(const error_info&, degradation_level)> handler) {
        error_handler_ = handler;
    }

    error_boundary_metrics get_metrics() const {
        return metrics_;
    }

private:
    std::string name_;
    config config_;
    std::function<void(const error_info&, degradation_level)> error_handler_;
    mutable error_boundary_metrics metrics_;
};

} // namespace monitoring_system