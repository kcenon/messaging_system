// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#pragma once

namespace kcenon::messaging {

/**
 * @class integration_detector
 * @brief Compile-time detection of system integration availability
 *
 * This utility provides compile-time detection of optional system integrations.
 * It follows the pattern established in common_system for consistent integration
 * detection across all systems.
 *
 * Detection is based on preprocessor macros that should be defined when the
 * respective systems are available:
 * - KCENON_COMMON_SYSTEM_AVAILABLE: common_system is available (provides ILogger)
 * - KCENON_THREAD_SYSTEM_AVAILABLE: thread_system is available
 * - KCENON_MONITORING_SYSTEM_AVAILABLE: monitoring_system is available
 *
 * Note: As of Issue #94, logging is now provided through common_system's
 * ILogger interface with runtime binding via GlobalLoggerRegistry, rather
 * than direct logger_system dependency.
 *
 * Usage:
 * @code
 * if constexpr (integration_detector::has_thread_system()) {
 *     // Use thread_system features
 * } else {
 *     // Fallback to standalone implementation
 * }
 * @endcode
 */
class integration_detector {
public:
    /**
     * @brief Check if thread_system is available at compile time
     * @return true if thread_system headers are included and available
     *
     * When available, messaging_system can use thread_system's thread pool
     * for async message processing instead of creating its own threads.
     */
    static constexpr bool has_thread_system() {
#ifdef KCENON_THREAD_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if common_system is available at compile time
     * @return true if common_system headers are included and available
     *
     * When available, messaging_system uses common_system's ILogger interface
     * with GlobalLoggerRegistry for structured logging. This is the preferred
     * method as of Issue #94.
     */
    static constexpr bool has_common_system() {
#ifdef KCENON_COMMON_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if logger_system is available at compile time
     * @return true if logger_system headers are included and available
     *
     * @deprecated Use has_common_system() instead. As of Issue #94, logging
     * is provided through common_system's ILogger interface with runtime
     * binding via GlobalLoggerRegistry.
     *
     * This method now returns has_common_system() for backward compatibility.
     */
    [[deprecated("Use has_common_system() instead - logging now uses common_system ILogger")]]
    static constexpr bool has_logger_system() {
        return has_common_system();
    }

    /**
     * @brief Check if monitoring_system is available at compile time
     * @return true if monitoring_system headers are included and available
     *
     * When available, messaging_system can emit metrics about message
     * throughput, latency, queue sizes, etc.
     */
    static constexpr bool has_monitoring_system() {
#ifdef KCENON_MONITORING_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if container_system is available at compile time
     * @return true if container_system headers are included and available
     *
     * When available, messaging_system can use container_system's value_container
     * for message payloads.
     */
    static constexpr bool has_container_system() {
#ifdef KCENON_CONTAINER_SYSTEM_AVAILABLE
        return true;
#else
        return false;
#endif
    }

    /**
     * @brief Check if any integration is available
     * @return true if at least one system integration is available
     */
    static constexpr bool has_any_integration() {
        return has_common_system() ||
               has_thread_system() ||
               has_monitoring_system() ||
               has_container_system();
    }

    /**
     * @brief Check if all integrations are available
     * @return true if all system integrations are available
     */
    static constexpr bool has_full_integration() {
        return has_common_system() &&
               has_thread_system() &&
               has_monitoring_system() &&
               has_container_system();
    }
};

} // namespace kcenon::messaging
