#pragma once

// Retry policy implementation - stub for compatibility
#include <chrono>
#include <functional>

namespace monitoring_system {

/**
 * @brief Retry strategies
 */
enum class retry_strategy {
    fixed_delay,
    exponential_backoff,
    linear_backoff
};

/**
 * @brief Retry configuration
 */
struct retry_config {
    size_t max_attempts = 3;
    retry_strategy strategy = retry_strategy::exponential_backoff;
    std::chrono::milliseconds initial_delay = std::chrono::milliseconds(1000);
    std::chrono::milliseconds max_delay = std::chrono::milliseconds(30000);
    double backoff_multiplier = 2.0;
};

/**
 * @brief Basic retry policy implementation - stub
 */
class retry_policy {
public:
    using config = retry_config;

    retry_policy() : config_() {}
    explicit retry_policy(const config& cfg) : config_(cfg) {}

    template<typename Func>
    auto execute(Func&& func) -> decltype(func()) {
        // Stub implementation - single attempt
        return func();
    }

private:
    config config_;
};

} // namespace monitoring_system