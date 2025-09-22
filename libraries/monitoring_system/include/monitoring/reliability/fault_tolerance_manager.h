#pragma once

// Fault tolerance manager - stub for compatibility
#include "circuit_breaker.h"
#include "retry_policy.h"
#include "error_boundary.h"

namespace monitoring_system {

/**
 * @brief Basic fault tolerance manager - stub implementation
 */
class fault_tolerance_manager {
public:
    fault_tolerance_manager() = default;

    template<typename Func>
    auto execute_with_fault_tolerance(Func&& func) -> decltype(func()) {
        // Stub implementation - direct execution
        return func();
    }

    void configure_circuit_breaker(const circuit_breaker::config& cfg) {
        circuit_breaker_ = std::make_unique<circuit_breaker>(cfg);
    }

    void configure_retry_policy(const retry_policy::config& cfg) {
        retry_policy_ = std::make_unique<retry_policy>(cfg);
    }

private:
    std::unique_ptr<circuit_breaker> circuit_breaker_;
    std::unique_ptr<retry_policy> retry_policy_;
    std::unique_ptr<error_boundary> error_boundary_;
};

} // namespace monitoring_system