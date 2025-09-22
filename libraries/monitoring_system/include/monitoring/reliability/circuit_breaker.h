#pragma once

// Circuit breaker implementation - stub for compatibility
#include <atomic>
#include <chrono>
#include <functional>

namespace monitoring_system {

/**
 * @brief Circuit breaker states
 */
enum class circuit_state {
    closed,
    open,
    half_open
};

/**
 * @brief Circuit breaker configuration
 */
struct circuit_breaker_config {
    size_t failure_threshold = 5;
    std::chrono::milliseconds timeout = std::chrono::milliseconds(60000);
    std::chrono::milliseconds reset_timeout = std::chrono::milliseconds(60000);
    size_t success_threshold = 3;
};

/**
 * @brief Circuit breaker metrics
 */
struct circuit_breaker_metrics {
    size_t total_calls = 0;
    size_t successful_calls = 0;
    size_t failed_calls = 0;
    size_t rejected_calls = 0;
    size_t state_transitions = 0;
};

/**
 * @brief Basic circuit breaker implementation - stub
 */
template<typename T = void>
class circuit_breaker {
public:
    using config = circuit_breaker_config;

    circuit_breaker() : config_() {}
    explicit circuit_breaker(const std::string& name) : name_(name), config_() {}
    explicit circuit_breaker(const std::string& name, const config& cfg) : name_(name), config_(cfg) {}

    template<typename Func, typename Fallback>
    auto execute(Func&& func, Fallback&& fallback) -> decltype(func()) {
        metrics_.total_calls++;
        // Stub implementation - always execute main function
        try {
            auto result = func();
            metrics_.successful_calls++;
            return result;
        } catch (...) {
            metrics_.failed_calls++;
            return fallback();
        }
    }

    template<typename Func>
    auto execute(Func&& func) -> decltype(func()) {
        metrics_.total_calls++;
        auto result = func();
        metrics_.successful_calls++;
        return result;
    }

    circuit_state get_state() const { return state_.load(); }
    size_t get_failure_count() const { return failure_count_.load(); }
    circuit_breaker_metrics get_metrics() const { return metrics_; }

private:
    std::string name_;
    config config_;
    std::atomic<size_t> failure_count_{0};
    std::atomic<circuit_state> state_{circuit_state::closed};
    mutable circuit_breaker_metrics metrics_;
};

} // namespace monitoring_system