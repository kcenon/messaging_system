/**
 * @file health_reliability_example.cpp
 * @brief Example demonstrating health monitoring and reliability features
 * 
 * This example shows how to:
 * - Set up health checks
 * - Configure circuit breakers
 * - Implement retry policies
 * - Handle cascading failures
 */

#include <iostream>
#include <thread>
#include <random>
#include <atomic>

#include "monitoring/health/health_monitor.h"
#include "monitoring/reliability/circuit_breaker.h"
#include "monitoring/reliability/retry_policy.h"
#include "monitoring/reliability/error_boundary.h"
#include "monitoring/core/result_types.h"
#include "monitoring/core/error_codes.h"

using namespace monitoring_system;
using namespace std::chrono_literals;

// Simulate a database connection
class DatabaseConnection {
private:
    std::atomic<bool> is_healthy_{true};
    std::atomic<int> query_count_{0};
    std::mt19937 rng_{std::random_device{}()};
    
public:
    void set_healthy(bool healthy) {
        is_healthy_ = healthy;
    }
    
    result<std::string> execute_query(const std::string& query) {
        query_count_++;
        
        // Simulate latency
        std::this_thread::sleep_for(10ms);
        
        // Simulate failures
        if (!is_healthy_) {
            return make_error<std::string>(
                monitoring_error_code::service_unavailable,
                "Database connection lost"
            );
        }
        
        // Random transient failures (10% chance)
        std::uniform_int_distribution<> dist(1, 10);
        if (dist(rng_) == 1) {
            return make_error<std::string>(
                monitoring_error_code::operation_timeout,
                "Query timeout"
            );
        }
        
        return make_success<std::string>("Query result for: " + query);
    }
    
    int get_query_count() const { return query_count_; }
};

// Simulate an external API
class ExternalApiClient {
private:
    std::atomic<int> failure_count_{0};
    std::atomic<int> call_count_{0};
    
public:
    result<std::string> call_api(const std::string& endpoint) {
        call_count_++;
        
        // Simulate increasing failures
        if (failure_count_ > 5) {
            // API is down
            return make_error<std::string>(
                monitoring_error_code::service_unavailable,
                "Service unavailable"
            );
        }
        
        // Simulate intermittent failures
        if (call_count_ % 3 == 0) {
            failure_count_++;
            return make_error<std::string>(
                monitoring_error_code::operation_failed,
                "Internal server error"
            );
        }
        
        failure_count_ = 0;  // Reset on success
        return make_success<std::string>("API response from: " + endpoint);
    }
    
    void reset() {
        failure_count_ = 0;
        call_count_ = 0;
    }
    
    int get_call_count() const { return call_count_; }
};

// Demonstrate health monitoring
void demonstrate_health_monitoring() {
    std::cout << "\n=== Health Monitoring Demo ===" << std::endl;
    
    // Create health monitor
    health_monitor_config config;
    config.check_interval = 2s;
    config.cache_duration = 1s;
    
    health_monitor monitor(config);
    
    // Create database connection for health checks
    auto database = std::make_shared<DatabaseConnection>();
    
    // Register liveness check
    monitor.register_check("database_liveness",
        std::make_shared<functional_health_check>(
            "database_liveness",
            health_check_type::liveness,
            [database]() -> health_check_result {
                // Simple ping check
                auto result = database->execute_query("SELECT 1");
                if (result) {
                    return health_check_result::healthy("Database is alive");
                } else {
                    return health_check_result::unhealthy(
                        "Database unreachable: " + result.get_error().message
                    );
                }
            },
            500ms,  // timeout
            true    // critical
        )
    );
    
    // Register readiness check
    monitor.register_check("database_readiness",
        std::make_shared<functional_health_check>(
            "database_readiness",
            health_check_type::readiness,
            [database]() -> health_check_result {
                // Check if database can handle queries
                auto result = database->execute_query("SELECT COUNT(*) FROM users");
                if (result) {
                    int query_count = database->get_query_count();
                    if (query_count > 100) {
                        return health_check_result::degraded(
                            "High query count: " + std::to_string(query_count)
                        );
                    }
                    return health_check_result::healthy("Database ready");
                } else {
                    return health_check_result::unhealthy(
                        "Database not ready: " + result.get_error().message
                    );
                }
            },
            1000ms,  // timeout
            false    // non-critical
        )
    );
    
    // Register startup check
    monitor.register_check("system_startup",
        std::make_shared<functional_health_check>(
            "system_startup",
            health_check_type::startup,
            []() -> health_check_result {
                // Check system initialization
                static bool initialized = false;
                if (!initialized) {
                    std::this_thread::sleep_for(100ms);  // Simulate initialization
                    initialized = true;
                }
                return health_check_result::healthy("System initialized");
            }
        )
    );
    
    // Start health monitoring
    monitor.start();
    
    std::cout << "Health monitoring started" << std::endl;
    
    // Perform health checks
    std::cout << "\n1. Initial health check:" << std::endl;
    auto all_checks = monitor.check_all();
    for (const auto& [name, result] : all_checks) {
        std::cout << "  " << name << ": " 
                 << (result.status == health_status::healthy ? "HEALTHY" :
                     result.status == health_status::degraded ? "DEGRADED" : "UNHEALTHY")
                 << " - " << result.message << std::endl;
    }
    
    // Get overall status
    auto overall = monitor.get_overall_status();
    std::cout << "  Overall status: " 
             << (overall == health_status::healthy ? "HEALTHY" :
                 overall == health_status::degraded ? "DEGRADED" : "UNHEALTHY")
             << std::endl;
    
    // Simulate database failure
    std::cout << "\n2. Simulating database failure..." << std::endl;
    database->set_healthy(false);
    std::this_thread::sleep_for(1s);
    
    all_checks = monitor.check_all();
    for (const auto& [name, result] : all_checks) {
        if (name.find("database") != std::string::npos) {
            std::cout << "  " << name << ": " 
                     << (result.status == health_status::healthy ? "HEALTHY" : "UNHEALTHY")
                     << " - " << result.message << std::endl;
        }
    }
    
    // Register recovery handler
    monitor.register_recovery_handler("database_liveness",
        [database]() -> bool {
            std::cout << "  Attempting database recovery..." << std::endl;
            database->set_healthy(true);
            return true;
        }
    );
    
    // Recover database
    std::cout << "\n3. Triggering recovery..." << std::endl;
    monitor.refresh();
    std::this_thread::sleep_for(2s);
    
    all_checks = monitor.check_all();
    std::cout << "  Database status after recovery: "
             << (all_checks["database_liveness"].status == health_status::healthy ? 
                 "HEALTHY" : "UNHEALTHY") << std::endl;
    
    // Get health report
    std::cout << "\n4. Health Report:" << std::endl;
    std::cout << monitor.get_health_report() << std::endl;
    
    monitor.stop();
}

// Demonstrate circuit breaker
void demonstrate_circuit_breaker() {
    std::cout << "\n=== Circuit Breaker Demo ===" << std::endl;
    
    // Create external API client
    auto api_client = std::make_shared<ExternalApiClient>();
    
    // Configure circuit breaker
    circuit_breaker_config cb_config;
    cb_config.failure_threshold = 3;
    // cb_config.failure_ratio = 0.5; // Not available in current API
    cb_config.timeout = 100ms;
    cb_config.reset_timeout = 2s;
    cb_config.success_threshold = 2;
    
    circuit_breaker<std::string> breaker("api_breaker", cb_config);
    
    std::cout << "Circuit breaker configured:" << std::endl;
    std::cout << "  Failure threshold: " << cb_config.failure_threshold << std::endl;
    std::cout << "  Reset timeout: 2s" << std::endl;
    
    // Define the operation
    auto api_operation = [api_client]() -> result<std::string> {
        return api_client->call_api("/users");
    };
    
    // Define fallback
    auto fallback = []() -> result<std::string> {
        return make_success<std::string>("Cached response (fallback)");
    };
    
    // Make calls through circuit breaker
    std::cout << "\n1. Making API calls through circuit breaker:" << std::endl;
    
    for (int i = 1; i <= 10; ++i) {
        auto result = breaker.execute(api_operation, fallback);
        
        std::cout << "  Call " << i << ": ";
        if (result) {
            std::cout << "SUCCESS - " << result.value() << std::endl;
        } else {
            std::cout << "FAILED - " << result.get_error().message << std::endl;
        }
        
        // Check circuit state
        auto state = breaker.get_state();
        if (state == circuit_state::open) {
            std::cout << "    [Circuit OPEN - using fallback]" << std::endl;
        } else if (state == circuit_state::half_open) {
            std::cout << "    [Circuit HALF-OPEN - testing]" << std::endl;
        }
        
        std::this_thread::sleep_for(300ms);
    }
    
    // Get circuit breaker metrics
    auto metrics = breaker.get_metrics();
    std::cout << "\n2. Circuit Breaker Metrics:" << std::endl;
    std::cout << "  Total calls: " << metrics.total_calls << std::endl;
    std::cout << "  Successful calls: " << metrics.successful_calls << std::endl;
    std::cout << "  Failed calls: " << metrics.failed_calls << std::endl;
    std::cout << "  Rejected calls: " << metrics.rejected_calls << std::endl;
    std::cout << "  State transitions: " << metrics.state_transitions << std::endl;
    
    // Wait for circuit to reset
    std::cout << "\n3. Waiting for circuit reset..." << std::endl;
    api_client->reset();  // Reset API client
    std::this_thread::sleep_for(3s);
    
    // Try again after reset
    std::cout << "\n4. Trying after reset:" << std::endl;
    for (int i = 1; i <= 3; ++i) {
        auto result = breaker.execute(api_operation, fallback);
        std::cout << "  Call " << i << ": ";
        if (result) {
            std::cout << "SUCCESS" << std::endl;
        } else {
            std::cout << "FAILED" << std::endl;
        }
    }
}

// Demonstrate retry policy (simplified)
void demonstrate_retry_policy() {
    std::cout << "\n=== Retry Policy Demo ===" << std::endl;
    
    // Configure retry policy
    retry_config config;
    config.max_attempts = 3;
    config.strategy = retry_strategy::exponential_backoff;
    config.initial_delay = 100ms;
    config.max_delay = 2s;
    config.backoff_multiplier = 2.0;
    
    std::cout << "Retry policy configured:" << std::endl;
    std::cout << "  Max attempts: " << config.max_attempts << std::endl;
    std::cout << "  Strategy: exponential backoff" << std::endl;
    std::cout << "  Initial delay: 100ms" << std::endl;
    
    // Simulate manual retry logic (since retry_policy class not available)
    std::cout << "\n1. Executing flaky operation with manual retry:" << std::endl;
    
    std::atomic<int> attempt_count{0};
    auto flaky_operation = [&attempt_count]() -> result<std::string> {
        attempt_count++;
        std::cout << "  Attempt " << attempt_count << "..." << std::endl;
        
        // Fail first 2 attempts
        if (attempt_count <= 2) {
            return make_error<std::string>(
                monitoring_error_code::operation_timeout,
                "Operation timed out"
            );
        }
        
        return make_success<std::string>("Operation succeeded!");
    };
    
    result<std::string> final_result = make_error<std::string>(
        monitoring_error_code::operation_failed, "Initialization pending");
    for (int i = 0; i < static_cast<int>(config.max_attempts); ++i) {
        final_result = flaky_operation();
        if (final_result) {
            break;
        }
        
        // Wait before retry
        if (i < static_cast<int>(config.max_attempts) - 1) {
            auto delay = config.initial_delay * static_cast<int>(std::pow(config.backoff_multiplier, i));
            std::this_thread::sleep_for(delay);
        }
    }
    
    if (final_result) {
        std::cout << "  Final result: SUCCESS - " << final_result.value() << std::endl;
    } else {
        std::cout << "  Final result: FAILED - " << final_result.get_error().message << std::endl;
    }
    
    std::cout << "  Total attempts: " << attempt_count << std::endl;
}

// Demonstrate error boundaries
void demonstrate_error_boundaries() {
    std::cout << "\n=== Error Boundaries Demo ===" << std::endl;
    
    // Configure error boundary
    error_boundary_config config;
    config.error_threshold = 5;  // Use correct field name
    config.error_window = 60s;
    config.enable_fallback_logging = true;  // Use correct field name
    
    error_boundary<std::string> boundary("critical_section", config);  // Specify template type
    
    // Set error handler
    boundary.set_error_handler([](const error_info& error, degradation_level level) {
        std::cout << "  Error handler called: " << error.message 
                 << " (degradation level: " << static_cast<int>(level) << ")" << std::endl;
    });
    
    std::cout << "Error boundary configured:" << std::endl;
    std::cout << "  Max errors: " << config.error_threshold << std::endl;
    std::cout << "  Error window: 60s" << std::endl;
    
    // Execute operations within boundary
    std::cout << "\n1. Executing operations within error boundary:" << std::endl;
    
    for (int i = 1; i <= 7; ++i) {
        auto result = boundary.execute([i]() -> ::result<std::string> {
            std::cout << "  Operation " << i << ": ";
            
            // Simulate failures on odd numbers
            if (i % 2 == 1) {
                std::cout << "FAILED" << std::endl;
                return make_error<std::string>(
                    monitoring_error_code::operation_failed,
                    "Operation " + std::to_string(i) + " failed"
                );
            }
            
            std::cout << "SUCCESS" << std::endl;
            return make_success<std::string>("Result " + std::to_string(i));
        });
        
        if (!result && result.get_error().code == monitoring_error_code::circuit_breaker_open) {
            std::cout << "    [Error boundary triggered - too many errors]" << std::endl;
            break;
        }
    }
    
    // Get statistics
    auto stats = boundary.get_metrics();
    std::cout << "\n2. Error Boundary Statistics:" << std::endl;
    std::cout << "  Total operations: " << stats.total_operations << std::endl;
    std::cout << "  Failed operations: " << stats.failed_operations << std::endl;
    std::cout << "  Success rate: " 
             << (stats.total_operations > 0 ? 
                 100.0 * (stats.total_operations - stats.failed_operations) / stats.total_operations : 0)
             << "%" << std::endl;
}

int main() {
    std::cout << "=== Health Monitoring & Reliability Example ===" << std::endl;
    
    try {
        // Part 1: Health Monitoring
        demonstrate_health_monitoring();
        
        // Part 2: Circuit Breaker
        demonstrate_circuit_breaker();
        
        // Part 3: Retry Policy
        demonstrate_retry_policy();
        
        // Part 4: Error Boundaries
        demonstrate_error_boundaries();
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    
    return 0;
}