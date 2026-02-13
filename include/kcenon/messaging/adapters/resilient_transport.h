// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file resilient_transport.h
 * @brief Resilient transport wrapper with retry and circuit breaker
 *
 * Provides reliability features on top of any transport:
 * - Automatic retry with exponential backoff
 * - Circuit breaker pattern for fault isolation
 * - Timeout management
 * - Fallback support
 */

#pragma once

#include <kcenon/messaging/adapters/transport_interface.h>

#include <kcenon/common/resilience/resilience.h>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>

namespace kcenon::messaging::adapters {

/**
 * @brief Circuit breaker state from common_system resilience module
 *
 * Values: CLOSED (normal), OPEN (failing fast), HALF_OPEN (testing recovery)
 */
using circuit_state = common::resilience::circuit_state;

/**
 * @brief Circuit breaker configuration from common_system resilience module
 *
 * Fields:
 * - failure_threshold: Failures before opening circuit (default: 5)
 * - success_threshold: Successes to close circuit in half-open (default: 2)
 * - failure_window: Time window for failure tracking (default: 60s)
 * - timeout: Time before transitioning OPEN to HALF_OPEN (default: 30s)
 * - half_open_max_requests: Test requests in half-open state (default: 3)
 */
using circuit_breaker_config = common::resilience::circuit_breaker_config;

/**
 * @struct retry_config
 * @brief Configuration for retry behavior
 */
struct retry_config {
    std::size_t max_retries = 3;
    std::chrono::milliseconds initial_delay{100};
    double backoff_multiplier = 2.0;
    std::chrono::milliseconds max_delay{10000};
    bool retry_on_timeout = true;
};

/**
 * @struct resilient_transport_config
 * @brief Configuration for resilient transport
 */
struct resilient_transport_config {
    retry_config retry;
    circuit_breaker_config circuit_breaker;
    std::chrono::milliseconds operation_timeout{30000};
    bool enable_fallback = false;
};

/**
 * @struct resilience_statistics
 * @brief Statistics for resilience features
 */
struct resilience_statistics {
    // Retry statistics
    uint64_t total_attempts = 0;
    uint64_t successful_first_attempts = 0;
    uint64_t successful_retries = 0;
    uint64_t failed_after_retries = 0;

    // Circuit breaker statistics
    uint64_t circuit_opens = 0;
    uint64_t circuit_closes = 0;
    uint64_t rejected_by_circuit = 0;
    circuit_state current_circuit_state = circuit_state::CLOSED;

    // Timing
    std::chrono::milliseconds avg_success_latency{0};
    std::chrono::milliseconds avg_failure_latency{0};
};

/**
 * @class resilient_transport
 * @brief Transport wrapper providing resilience features
 *
 * This class wraps any transport_interface implementation and adds:
 * - Automatic retry with configurable backoff
 * - Circuit breaker to prevent cascade failures
 * - Timeout management
 * - Optional fallback when primary transport fails
 *
 * Usage Example:
 * @code
 * // Create underlying transport
 * websocket_transport_config ws_config;
 * ws_config.host = "primary.example.com";
 * ws_config.port = 8080;
 * auto primary = std::make_shared<websocket_transport>(ws_config);
 *
 * // Create resilient wrapper
 * resilient_transport_config config;
 * config.retry.max_retries = 3;
 * config.retry.initial_delay = std::chrono::milliseconds(100);
 * config.circuit_breaker.failure_threshold = 5;
 * config.circuit_breaker.timeout = std::chrono::seconds(30);
 *
 * auto resilient = std::make_shared<resilient_transport>(primary, config);
 *
 * // Optionally set fallback transport
 * ws_config.host = "backup.example.com";
 * auto backup = std::make_shared<websocket_transport>(ws_config);
 * resilient->set_fallback(backup);
 *
 * // Use as normal transport
 * resilient->connect();
 * resilient->send(message);
 * @endcode
 */
class resilient_transport : public transport_interface {
public:
    /**
     * @brief Construct resilient transport wrapper
     * @param transport Underlying transport to wrap
     * @param config Resilience configuration
     */
    resilient_transport(
        std::shared_ptr<transport_interface> transport,
        const resilient_transport_config& config = {});

    /**
     * @brief Destructor
     */
    ~resilient_transport() override;

    // transport_interface implementation
    common::VoidResult connect() override;
    common::VoidResult disconnect() override;
    bool is_connected() const override;
    transport_state get_state() const override;

    common::VoidResult send(const message& msg) override;
    common::VoidResult send_binary(const std::vector<uint8_t>& data) override;

    void set_message_handler(
        std::function<void(const message&)> handler) override;
    void set_binary_handler(
        std::function<void(const std::vector<uint8_t>&)> handler) override;
    void set_state_handler(
        std::function<void(transport_state)> handler) override;
    void set_error_handler(
        std::function<void(const std::string&)> handler) override;

    transport_statistics get_statistics() const override;
    void reset_statistics() override;

    // Resilience-specific methods

    /**
     * @brief Set fallback transport for when primary fails
     * @param fallback Fallback transport
     */
    void set_fallback(std::shared_ptr<transport_interface> fallback);

    /**
     * @brief Get current circuit breaker state
     * @return Current circuit state
     */
    circuit_state get_circuit_state() const;

    /**
     * @brief Force circuit breaker to open state
     */
    void force_circuit_open();

    /**
     * @brief Force circuit breaker to closed state
     */
    void force_circuit_close();

    /**
     * @brief Get resilience-specific statistics
     * @return Resilience statistics
     */
    resilience_statistics get_resilience_statistics() const;

    /**
     * @brief Reset resilience statistics
     */
    void reset_resilience_statistics();

    /**
     * @brief Update retry configuration
     * @param config New retry configuration
     */
    void set_retry_config(const retry_config& config);

    /**
     * @brief Update circuit breaker configuration
     * @param config New circuit breaker configuration
     */
    void set_circuit_breaker_config(const circuit_breaker_config& config);

    /**
     * @brief Set callback for circuit state changes
     * @param handler Callback with new state
     */
    void set_circuit_state_handler(
        std::function<void(circuit_state)> handler);

    /**
     * @brief Set callback for retry events
     * @param handler Callback with attempt number and delay
     */
    void set_retry_handler(
        std::function<void(std::size_t attempt, std::chrono::milliseconds delay)> handler);

private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

} // namespace kcenon::messaging::adapters
