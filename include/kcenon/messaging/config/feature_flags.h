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

} // namespace kcenon::messaging::config
