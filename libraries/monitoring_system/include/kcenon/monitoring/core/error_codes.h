#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file error_codes.h
 * @brief Monitoring system specific error codes
 * 
 * This file defines error codes used throughout the monitoring system,
 * following the pattern established by thread_system and logger_system.
 */

#include <cstdint>
#include <string>

namespace monitoring_system {

/**
 * @enum monitoring_error_code
 * @brief Comprehensive error codes for monitoring system operations
 */
enum class monitoring_error_code : std::uint32_t {
    // Success
    success = 0,
    
    // Collection errors (1000-1999)
    collector_not_found = 1000,
    collection_failed = 1001,
    collector_initialization_failed = 1002,
    collector_already_exists = 1003,
    collector_disabled = 1004,
    invalid_collector_config = 1005,
    monitoring_disabled = 1006,
    
    // Storage errors (2000-2999)
    storage_full = 2000,
    storage_corrupted = 2001,
    compression_failed = 2002,
    storage_not_initialized = 2003,
    storage_write_failed = 2004,
    storage_read_failed = 2005,
    storage_empty = 2006,
    
    // Configuration errors (3000-3999)
    invalid_configuration = 3000,
    invalid_interval = 3001,
    invalid_capacity = 3002,
    configuration_not_found = 3003,
    configuration_parse_error = 3004,
    
    // System errors (4000-4999)
    system_resource_unavailable = 4000,
    permission_denied = 4001,
    out_of_memory = 4002,
    memory_allocation_failed = 4003,
    operation_timeout = 4004,
    operation_cancelled = 4005,
    
    // Integration errors (5000-5999)
    thread_system_not_available = 5000,
    logger_system_not_available = 5001,
    incompatible_version = 5002,
    adapter_initialization_failed = 5003,
    
    // Metrics errors (6000-6999)
    metric_not_found = 6000,
    invalid_metric_type = 6001,
    metric_overflow = 6002,
    aggregation_failed = 6003,
    processing_failed = 6004,
    
    // Health check errors (7000-7999)
    health_check_failed = 7000,
    health_check_timeout = 7001,
    health_check_not_registered = 7002,
    
    // Fault tolerance errors (8000-8099)
    circuit_breaker_open = 8000,
    circuit_breaker_half_open = 8001,
    retry_attempts_exhausted = 8002,
    operation_failed = 8003,
    network_error = 8004,
    service_unavailable = 8005,
    service_degraded = 8006,
    error_boundary_triggered = 8007,
    fallback_failed = 8008,
    recovery_failed = 8009,
    
    // General errors (8100-8999)
    invalid_argument = 8100,
    invalid_state = 8101,
    not_found = 8102,
    already_exists = 8103,
    resource_exhausted = 8104,
    already_started = 8105,
    dependency_missing = 8106,
    
    // Resource management errors (8200-8299)
    quota_exceeded = 8200,
    rate_limit_exceeded = 8201,
    cpu_throttled = 8202,
    memory_quota_exceeded = 8203,
    bandwidth_exceeded = 8204,
    resource_unavailable = 8205,
    
    // Data consistency errors (8300-8399)
    transaction_failed = 8300,
    transaction_timeout = 8301,
    transaction_aborted = 8302,
    validation_failed = 8303,
    data_corrupted = 8304,
    state_inconsistent = 8305,
    deadlock_detected = 8306,
    rollback_failed = 8307,
    
    // Unknown error
    unknown_error = 9999
};

/**
 * @brief Convert error code to string representation
 * @param code The error code to convert
 * @return String representation of the error code
 */
inline std::string error_code_to_string(monitoring_error_code code) {
    switch (code) {
        case monitoring_error_code::success:
            return "Success";
            
        // Collection errors
        case monitoring_error_code::collector_not_found:
            return "Collector not found";
        case monitoring_error_code::collection_failed:
            return "Collection failed";
        case monitoring_error_code::collector_initialization_failed:
            return "Collector initialization failed";
        case monitoring_error_code::collector_already_exists:
            return "Collector already exists";
        case monitoring_error_code::collector_disabled:
            return "Collector is disabled";
        case monitoring_error_code::invalid_collector_config:
            return "Invalid collector configuration";
        case monitoring_error_code::monitoring_disabled:
            return "Monitoring is disabled";
            
        // Storage errors
        case monitoring_error_code::storage_full:
            return "Storage is full";
        case monitoring_error_code::storage_corrupted:
            return "Storage is corrupted";
        case monitoring_error_code::compression_failed:
            return "Compression failed";
        case monitoring_error_code::storage_not_initialized:
            return "Storage not initialized";
        case monitoring_error_code::storage_write_failed:
            return "Storage write failed";
        case monitoring_error_code::storage_read_failed:
            return "Storage read failed";
        case monitoring_error_code::storage_empty:
            return "Storage is empty";
            
        // Configuration errors
        case monitoring_error_code::invalid_configuration:
            return "Invalid configuration";
        case monitoring_error_code::invalid_interval:
            return "Invalid interval";
        case monitoring_error_code::invalid_capacity:
            return "Invalid capacity";
        case monitoring_error_code::configuration_not_found:
            return "Configuration not found";
        case monitoring_error_code::configuration_parse_error:
            return "Configuration parse error";
            
        // System errors
        case monitoring_error_code::system_resource_unavailable:
            return "System resource unavailable";
        case monitoring_error_code::permission_denied:
            return "Permission denied";
        case monitoring_error_code::out_of_memory:
            return "Out of memory";
        case monitoring_error_code::memory_allocation_failed:
            return "Memory allocation failed";
        case monitoring_error_code::operation_timeout:
            return "Operation timeout";
        case monitoring_error_code::operation_cancelled:
            return "Operation cancelled";
            
        // Integration errors
        case monitoring_error_code::thread_system_not_available:
            return "Thread system not available";
        case monitoring_error_code::logger_system_not_available:
            return "Logger system not available";
        case monitoring_error_code::incompatible_version:
            return "Incompatible version";
        case monitoring_error_code::adapter_initialization_failed:
            return "Adapter initialization failed";
            
        // Metrics errors
        case monitoring_error_code::metric_not_found:
            return "Metric not found";
        case monitoring_error_code::invalid_metric_type:
            return "Invalid metric type";
        case monitoring_error_code::metric_overflow:
            return "Metric overflow";
        case monitoring_error_code::aggregation_failed:
            return "Aggregation failed";
            
        // Health check errors
        case monitoring_error_code::health_check_failed:
            return "Health check failed";
        case monitoring_error_code::health_check_timeout:
            return "Health check timeout";
        case monitoring_error_code::health_check_not_registered:
            return "Health check not registered";
            
        // Fault tolerance errors
        case monitoring_error_code::circuit_breaker_open:
            return "Circuit breaker is open";
        case monitoring_error_code::circuit_breaker_half_open:
            return "Circuit breaker is half-open";
        case monitoring_error_code::retry_attempts_exhausted:
            return "Retry attempts exhausted";
        case monitoring_error_code::operation_failed:
            return "Operation failed";
        case monitoring_error_code::network_error:
            return "Network error";
        case monitoring_error_code::service_unavailable:
            return "Service unavailable";
        case monitoring_error_code::service_degraded:
            return "Service operating in degraded mode";
        case monitoring_error_code::error_boundary_triggered:
            return "Error boundary triggered";
        case monitoring_error_code::fallback_failed:
            return "Fallback operation failed";
        case monitoring_error_code::recovery_failed:
            return "Recovery operation failed";
            
        // General errors
        case monitoring_error_code::invalid_argument:
            return "Invalid argument";
        case monitoring_error_code::invalid_state:
            return "Invalid state";
        case monitoring_error_code::not_found:
            return "Not found";
        case monitoring_error_code::already_exists:
            return "Already exists";
        case monitoring_error_code::resource_exhausted:
            return "Resource exhausted";
        case monitoring_error_code::already_started:
            return "Already started";
        case monitoring_error_code::dependency_missing:
            return "Dependency missing";

        // Resource management errors
        case monitoring_error_code::quota_exceeded:
            return "Quota exceeded";
        case monitoring_error_code::rate_limit_exceeded:
            return "Rate limit exceeded";
        case monitoring_error_code::cpu_throttled:
            return "CPU throttled";
        case monitoring_error_code::memory_quota_exceeded:
            return "Memory quota exceeded";
        case monitoring_error_code::bandwidth_exceeded:
            return "Bandwidth exceeded";
        case monitoring_error_code::resource_unavailable:
            return "Resource unavailable";
            
        // Data consistency errors
        case monitoring_error_code::transaction_failed:
            return "Transaction failed";
        case monitoring_error_code::transaction_timeout:
            return "Transaction timeout";
        case monitoring_error_code::transaction_aborted:
            return "Transaction aborted";
        case monitoring_error_code::validation_failed:
            return "Validation failed";
        case monitoring_error_code::data_corrupted:
            return "Data corrupted";
        case monitoring_error_code::state_inconsistent:
            return "State inconsistent";
        case monitoring_error_code::deadlock_detected:
            return "Deadlock detected";
        case monitoring_error_code::rollback_failed:
            return "Rollback failed";
            
        // Unknown error
        case monitoring_error_code::unknown_error:
        default:
            return "Unknown error";
    }
}

/**
 * @brief Get detailed error message
 * @param code The error code
 * @return Detailed error message with suggestions
 */
inline std::string get_error_details(monitoring_error_code code) {
    switch (code) {
        case monitoring_error_code::collector_not_found:
            return "The specified collector was not found. Check collector name and ensure it's registered.";
        case monitoring_error_code::storage_full:
            return "Storage capacity exceeded. Consider increasing buffer size or reducing collection frequency.";
        case monitoring_error_code::invalid_configuration:
            return "Configuration validation failed. Review configuration parameters and constraints.";
        case monitoring_error_code::thread_system_not_available:
            return "Thread system integration not available. Ensure thread_system is properly linked.";
        case monitoring_error_code::circuit_breaker_open:
            return "Circuit breaker is open, rejecting calls to protect downstream services. Wait for recovery or check service health.";
        case monitoring_error_code::retry_attempts_exhausted:
            return "All retry attempts have been exhausted. The operation failed permanently. Check service availability and error conditions.";
        case monitoring_error_code::operation_failed:
            return "The requested operation failed. Check service status, network connectivity, and input parameters.";
        case monitoring_error_code::service_degraded:
            return "Service is operating in degraded mode due to detected issues. Some features may be unavailable.";
        case monitoring_error_code::error_boundary_triggered:
            return "Error boundary has been triggered to prevent error propagation. Check upstream service health.";
        case monitoring_error_code::fallback_failed:
            return "Both primary operation and fallback mechanism failed. Check alternative service configurations.";
        case monitoring_error_code::quota_exceeded:
            return "Resource quota has been exceeded. Reduce resource consumption or increase quota limits.";
        case monitoring_error_code::rate_limit_exceeded:
            return "Rate limit has been exceeded. Reduce request frequency or increase rate limits.";
        case monitoring_error_code::cpu_throttled:
            return "Operation has been throttled due to high CPU usage. Reduce system load or adjust CPU limits.";
        case monitoring_error_code::memory_quota_exceeded:
            return "Memory quota has been exceeded. Free memory or increase memory quota limits.";
        case monitoring_error_code::bandwidth_exceeded:
            return "Bandwidth quota has been exceeded. Reduce data transfer or increase bandwidth limits.";
        case monitoring_error_code::resource_unavailable:
            return "Required resource is currently unavailable. Try again later or check resource status.";
        case monitoring_error_code::transaction_failed:
            return "Transaction failed to complete successfully. Check operation prerequisites and system state.";
        case monitoring_error_code::transaction_timeout:
            return "Transaction exceeded its timeout limit. Consider increasing timeout or reducing transaction scope.";
        case monitoring_error_code::transaction_aborted:
            return "Transaction was aborted due to conflicts or errors. Review transaction operations and retry.";
        case monitoring_error_code::validation_failed:
            return "Data validation failed. Check data integrity and consistency requirements.";
        case monitoring_error_code::data_corrupted:
            return "Data corruption detected. Run data repair operations or restore from backup.";
        case monitoring_error_code::state_inconsistent:
            return "System state is inconsistent across components. Synchronization or recovery needed.";
        case monitoring_error_code::deadlock_detected:
            return "Deadlock detected in transaction processing. Review locking strategy and transaction ordering.";
        case monitoring_error_code::rollback_failed:
            return "Transaction rollback failed. Manual cleanup may be required to restore consistent state.";
        default:
            return error_code_to_string(code);
    }
}

} // namespace monitoring_system