// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file feature_flags.h
 * @brief Feature flags for conditional compilation
 *
 * This header provides compile-time feature flags that control which
 * optional features are available in the messaging system.
 */

#pragma once

// =============================================================================
// Network System Integration
// =============================================================================

/**
 * @def KCENON_WITH_NETWORK_SYSTEM
 * @brief Controls network_system integration for transport implementations
 *
 * When set to 1, websocket_transport and http_transport use network_system
 * for actual network I/O operations.
 *
 * When set to 0, transport implementations provide stub versions that return
 * error::not_supported for all operations.
 *
 * This is typically set via CMake:
 *   cmake -DKCENON_WITH_NETWORK_SYSTEM=ON  (enabled, default)
 *   cmake -DKCENON_WITH_NETWORK_SYSTEM=OFF (disabled, stub mode)
 */
#ifndef KCENON_WITH_NETWORK_SYSTEM
#define KCENON_WITH_NETWORK_SYSTEM 0
#endif

// =============================================================================
// Monitoring System Integration
// =============================================================================

/**
 * @def KCENON_WITH_MONITORING_SYSTEM
 * @brief Controls monitoring_system integration for metrics collection
 *
 * When set to 1, message_bus_collector provides full monitoring integration
 * with Prometheus-compatible metrics.
 *
 * When set to 0, message_bus_collector provides a null/stub implementation
 * that silently ignores all operations.
 *
 * This is typically set via CMake or inherited from common_system feature_flags.
 */
#ifndef KCENON_WITH_MONITORING_SYSTEM
#define KCENON_WITH_MONITORING_SYSTEM 0
#endif

// =============================================================================
// Feature Detection Helpers
// =============================================================================

namespace kcenon::messaging::config {

/**
 * @brief Check if network_system integration is available
 * @return true if KCENON_WITH_NETWORK_SYSTEM is enabled
 */
inline constexpr bool has_network_system() noexcept {
    return KCENON_WITH_NETWORK_SYSTEM != 0;
}

/**
 * @brief Check if monitoring_system integration is available
 * @return true if KCENON_WITH_MONITORING_SYSTEM is enabled
 */
inline constexpr bool has_monitoring_system() noexcept {
    return KCENON_WITH_MONITORING_SYSTEM != 0;
}

} // namespace kcenon::messaging::config
