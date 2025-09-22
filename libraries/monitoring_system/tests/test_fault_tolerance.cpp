#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <monitoring/reliability/circuit_breaker.h>
#include <monitoring/reliability/retry_policy.h>
#include <monitoring/reliability/fault_tolerance_manager.h>

using namespace monitoring_system;

class FaultToleranceTest : public ::testing::Test {
protected:
    void SetUp() override {
        call_count = 0;
        success_after_attempts = 0;
    }
    
    void TearDown() override {
        // Clean up registries
        global_circuit_breaker_registry().clear();
        global_retry_executor_registry().clear();
        global_fault_tolerance_registry().clear();
    }
    
    std::atomic<int> call_count{0};
    std::atomic<int> success_after_attempts{0};
    
    // Helper function that fails for the first N attempts
    result<int> failing_operation() {
        int current_call = ++call_count;
        if (success_after_attempts > 0 && current_call <= success_after_attempts) {
            return make_error<int>(monitoring_error_code::operation_failed, 
                                 "Simulated failure on attempt " + std::to_string(current_call));
        }
        return make_success(42);
    }
    
    // Helper function that always fails
    result<int> always_failing_operation() {
        ++call_count;
        return make_error<int>(monitoring_error_code::operation_failed, "Always fails");
    }
    
    // Helper function that always succeeds
    result<int> always_succeeding_operation() {
        ++call_count;
        return make_success(100);
    }
    
    // Slow operation for timeout testing
    result<int> slow_operation(std::chrono::milliseconds delay) {
        ++call_count;
        std::this_thread::sleep_for(delay);
        return make_success(200);
    }
};

// Circuit Breaker Tests
TEST_F(FaultToleranceTest, CircuitBreakerClosedState) {
    circuit_breaker_config config;
    config.failure_threshold = 3;
    
    circuit_breaker<int> breaker("test_breaker", config);
    
    EXPECT_EQ(breaker.get_state(), circuit_state::closed);
    
    auto result = breaker.execute([this]() { return always_succeeding_operation(); });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 100);
    EXPECT_EQ(breaker.get_state(), circuit_state::closed);
    EXPECT_EQ(call_count.load(), 1);
}

TEST_F(FaultToleranceTest, CircuitBreakerOpensAfterFailures) {
    circuit_breaker_config config;
    config.failure_threshold = 3;
    
    circuit_breaker<int> breaker("test_breaker", config);
    
    // First 3 failures should open the circuit
    for (int i = 0; i < 3; ++i) {
        auto result = breaker.execute([this]() { return always_failing_operation(); });
        EXPECT_FALSE(result);
    }
    
    EXPECT_EQ(breaker.get_state(), circuit_state::open);
    EXPECT_EQ(call_count.load(), 3);
    
    // Next call should be rejected without calling operation
    auto result = breaker.execute([this]() { return always_failing_operation(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(call_count.load(), 3); // Should not increment
    EXPECT_EQ(result.get_error().code, monitoring_error_code::circuit_breaker_open);
}

TEST_F(FaultToleranceTest, CircuitBreakerHalfOpenTransition) {
    circuit_breaker_config config;
    config.failure_threshold = 2;
    config.reset_timeout = std::chrono::milliseconds(100);
    
    circuit_breaker<int> breaker("test_breaker", config);
    
    // Open the circuit
    for (int i = 0; i < 2; ++i) {
        breaker.execute([this]() { return always_failing_operation(); });
    }
    EXPECT_EQ(breaker.get_state(), circuit_state::open);
    
    // Wait for reset timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    // Next call should transition to half-open
    auto result = breaker.execute([this]() { return always_succeeding_operation(); });
    EXPECT_TRUE(result);
    EXPECT_EQ(breaker.get_state(), circuit_state::half_open);
}

TEST_F(FaultToleranceTest, CircuitBreakerHalfOpenToClosedTransition) {
    circuit_breaker_config config;
    config.failure_threshold = 2;
    config.success_threshold = 2;
    config.reset_timeout = std::chrono::milliseconds(50);
    
    circuit_breaker<int> breaker("test_breaker", config);
    
    // Open the circuit
    for (int i = 0; i < 2; ++i) {
        breaker.execute([this]() { return always_failing_operation(); });
    }
    
    // Wait and transition to half-open
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    breaker.execute([this]() { return always_succeeding_operation(); });
    EXPECT_EQ(breaker.get_state(), circuit_state::half_open);
    
    // One more success should close the circuit
    auto result = breaker.execute([this]() { return always_succeeding_operation(); });
    EXPECT_TRUE(result);
    EXPECT_EQ(breaker.get_state(), circuit_state::closed);
}

TEST_F(FaultToleranceTest, CircuitBreakerWithFallback) {
    circuit_breaker_config config;
    config.failure_threshold = 1;
    
    circuit_breaker<int> breaker("test_breaker", config);
    
    // Open the circuit with one failure
    breaker.execute([this]() { return always_failing_operation(); });
    EXPECT_EQ(breaker.get_state(), circuit_state::open);
    
    // Use fallback
    auto fallback = []() { return make_success(999); };
    auto result = breaker.execute([this]() { return always_failing_operation(); }, fallback);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 999);
}

TEST_F(FaultToleranceTest, CircuitBreakerMetrics) {
    circuit_breaker_config config;
    config.failure_threshold = 3;
    
    circuit_breaker<int> breaker("test_breaker", config);
    
    // Execute some operations
    breaker.execute([this]() { return always_succeeding_operation(); });
    breaker.execute([this]() { return always_failing_operation(); });
    breaker.execute([this]() { return always_succeeding_operation(); });
    
    auto metrics = breaker.get_metrics();
    EXPECT_EQ(metrics.total_calls.load(), 3);
    EXPECT_EQ(metrics.successful_calls.load(), 2);
    EXPECT_EQ(metrics.failed_calls.load(), 1);
    EXPECT_NEAR(metrics.get_success_rate(), 2.0/3.0, 0.01);
}

// Retry Policy Tests
TEST_F(FaultToleranceTest, RetryExecutorBasicRetry) {
    auto config = create_exponential_backoff_config(3, std::chrono::milliseconds(10));
    retry_executor<int> executor("test_retry", config);
    
    success_after_attempts = 2; // Succeed on 3rd attempt
    
    auto result = executor.execute([this]() { return failing_operation(); });
    
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
    EXPECT_EQ(call_count.load(), 3);
    
    auto metrics = executor.get_metrics();
    EXPECT_EQ(metrics.total_executions, 1);
    EXPECT_EQ(metrics.successful_executions, 1);
    EXPECT_EQ(metrics.total_retries, 2);
}

TEST_F(FaultToleranceTest, RetryExecutorMaxAttemptsExceeded) {
    auto config = create_exponential_backoff_config(2, std::chrono::milliseconds(10));
    retry_executor<int> executor("test_retry", config);
    
    auto result = executor.execute([this]() { return always_failing_operation(); });
    
    EXPECT_FALSE(result);
    EXPECT_EQ(call_count.load(), 2);
    
    auto metrics = executor.get_metrics();
    EXPECT_EQ(metrics.total_executions, 1);
    EXPECT_EQ(metrics.failed_executions, 1);
    EXPECT_EQ(metrics.total_retries, 1);
}

TEST_F(FaultToleranceTest, RetryExecutorFixedDelay) {
    retry_config config = create_fixed_delay_config(3, std::chrono::milliseconds(50));
    retry_executor<int> executor("test_retry", config);
    
    success_after_attempts = 2;
    
    auto start = std::chrono::steady_clock::now();
    auto result = executor.execute([this]() { return failing_operation(); });
    auto duration = std::chrono::steady_clock::now() - start;
    
    EXPECT_TRUE(result);
    EXPECT_GE(duration, std::chrono::milliseconds(100)); // At least 2 delays
}

TEST_F(FaultToleranceTest, RetryExecutorFibonacciBackoff) {
    auto config = create_fibonacci_backoff_config(4, std::chrono::milliseconds(10));
    retry_executor<int> executor("test_retry", config);
    
    success_after_attempts = 3;
    
    auto result = executor.execute([this]() { return failing_operation(); });
    
    EXPECT_TRUE(result);
    EXPECT_EQ(call_count.load(), 4);
}

TEST_F(FaultToleranceTest, RetryExecutorCustomShouldRetry) {
    retry_config config = create_exponential_backoff_config(3);
    config.should_retry = [](const error_info& error) {
        // Only retry operation_timeout errors
        return error.code == monitoring_error_code::operation_timeout;
    };
    
    retry_executor<int> executor("test_retry", config);
    
    // This should not retry because it's not a timeout error
    auto result = executor.execute([this]() { return always_failing_operation(); });
    
    EXPECT_FALSE(result);
    EXPECT_EQ(call_count.load(), 1); // No retries
}

// Fault Tolerance Manager Tests
TEST_F(FaultToleranceTest, FaultToleranceManagerCircuitBreakerFirst) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = true;
    config.enable_retry = true;
    config.circuit_breaker_first = true;
    config.circuit_config.failure_threshold = 2;
    config.retry_cfg = create_exponential_backoff_config(2, std::chrono::milliseconds(10));
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    success_after_attempts = 1; // Succeed on 2nd attempt
    
    auto result = manager.execute([this]() { return failing_operation(); });
    
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
    EXPECT_EQ(call_count.load(), 2); // Retry executed
}

TEST_F(FaultToleranceTest, FaultToleranceManagerRetryFirst) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = true;
    config.enable_retry = true;
    config.circuit_breaker_first = false;
    config.circuit_config.failure_threshold = 5;
    config.retry_cfg = create_exponential_backoff_config(3, std::chrono::milliseconds(10));
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    success_after_attempts = 2;
    
    auto result = manager.execute([this]() { return failing_operation(); });
    
    EXPECT_TRUE(result);
    EXPECT_EQ(call_count.load(), 3);
}

TEST_F(FaultToleranceTest, FaultToleranceManagerOnlyCircuitBreaker) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = true;
    config.enable_retry = false;
    config.circuit_config.failure_threshold = 2;
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    auto result = manager.execute([this]() { return always_succeeding_operation(); });
    
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 100);
}

TEST_F(FaultToleranceTest, FaultToleranceManagerOnlyRetry) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = false;
    config.enable_retry = true;
    config.retry_cfg = create_exponential_backoff_config(3, std::chrono::milliseconds(10));
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    success_after_attempts = 2;
    
    auto result = manager.execute([this]() { return failing_operation(); });
    
    EXPECT_TRUE(result);
    EXPECT_EQ(call_count.load(), 3);
}

TEST_F(FaultToleranceTest, FaultToleranceManagerWithTimeout) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = false;
    config.enable_retry = true;
    config.retry_cfg = create_exponential_backoff_config(2, std::chrono::milliseconds(10));
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    auto result = manager.execute_with_timeout(
        [this]() { return slow_operation(std::chrono::milliseconds(100)); },
        std::chrono::milliseconds(50)
    );
    
    EXPECT_FALSE(result);
    EXPECT_EQ(result.get_error().code, monitoring_error_code::operation_timeout);
}

TEST_F(FaultToleranceTest, FaultToleranceManagerMetrics) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = true;
    config.enable_retry = true;
    config.circuit_config.failure_threshold = 5;
    config.retry_cfg = create_exponential_backoff_config(2, std::chrono::milliseconds(10));
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    // Execute successful operation
    manager.execute([this]() { return always_succeeding_operation(); });
    
    // Execute failing operation (will retry once)
    manager.execute([this]() { return always_failing_operation(); });
    
    auto metrics = manager.get_metrics();
    EXPECT_EQ(metrics.total_operations, 2);
    EXPECT_EQ(metrics.successful_operations, 1);
    EXPECT_EQ(metrics.failed_operations, 1);
    EXPECT_NEAR(metrics.get_overall_success_rate(), 0.5, 0.01);
}

TEST_F(FaultToleranceTest, FaultToleranceManagerHealthCheck) {
    fault_tolerance_config config;
    config.enable_circuit_breaker = true;
    config.circuit_config.failure_threshold = 2;
    
    fault_tolerance_manager<int> manager("test_manager", config);
    
    // Initially healthy
    auto health = manager.is_healthy();
    EXPECT_TRUE(health);
    EXPECT_TRUE(health.value());
    
    // Open circuit breaker
    for (int i = 0; i < 2; ++i) {
        manager.execute([this]() { return always_failing_operation(); });
    }
    
    // Should now be unhealthy due to open circuit
    health = manager.is_healthy();
    EXPECT_TRUE(health);
    EXPECT_FALSE(health.value());
}

// Registry Tests
TEST_F(FaultToleranceTest, CircuitBreakerRegistry) {
    auto& registry = global_circuit_breaker_registry();
    
    auto breaker = std::make_shared<circuit_breaker<int>>("test_breaker");
    registry.register_circuit_breaker<int>("test", breaker);
    
    auto retrieved = registry.get_circuit_breaker<int>("test");
    EXPECT_EQ(retrieved, breaker);
    
    auto names = registry.get_all_names();
    EXPECT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "test");
    
    registry.remove_circuit_breaker("test");
    retrieved = registry.get_circuit_breaker<int>("test");
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(FaultToleranceTest, RetryExecutorRegistry) {
    auto& registry = global_retry_executor_registry();
    
    auto executor = std::make_shared<retry_executor<int>>("test_executor");
    registry.register_executor<int>("test", executor);
    
    auto retrieved = registry.get_executor<int>("test");
    EXPECT_EQ(retrieved, executor);
    
    auto names = registry.get_all_names();
    EXPECT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "test");
    
    registry.remove_executor("test");
    retrieved = registry.get_executor<int>("test");
    EXPECT_EQ(retrieved, nullptr);
}

TEST_F(FaultToleranceTest, FaultToleranceRegistry) {
    auto& registry = global_fault_tolerance_registry();
    
    auto manager = std::make_shared<fault_tolerance_manager<int>>("test_manager");
    registry.register_manager<int>("test", manager);
    
    auto retrieved = registry.get_manager<int>("test");
    EXPECT_EQ(retrieved, manager);
    
    auto names = registry.get_all_names();
    EXPECT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "test");
    
    registry.remove_manager("test");
    retrieved = registry.get_manager<int>("test");
    EXPECT_EQ(retrieved, nullptr);
}

// Configuration Validation Tests
TEST_F(FaultToleranceTest, CircuitBreakerConfigValidation) {
    circuit_breaker_config config;
    
    // Valid config
    EXPECT_TRUE(config.validate());
    
    // Invalid failure threshold
    config.failure_threshold = 0;
    EXPECT_FALSE(config.validate());
    
    // Reset to valid
    config.failure_threshold = 5;
    EXPECT_TRUE(config.validate());
    
    // Invalid success threshold
    config.success_threshold = 0;
    EXPECT_FALSE(config.validate());
}

TEST_F(FaultToleranceTest, RetryConfigValidation) {
    retry_config config;
    
    // Valid config
    EXPECT_TRUE(config.validate());
    
    // Invalid max attempts
    config.max_attempts = 0;
    EXPECT_FALSE(config.validate());
    
    // Reset to valid
    config.max_attempts = 3;
    EXPECT_TRUE(config.validate());
    
    // Invalid backoff multiplier
    config.backoff_multiplier = 0.5;
    EXPECT_FALSE(config.validate());
}

TEST_F(FaultToleranceTest, FaultToleranceConfigValidation) {
    fault_tolerance_config config;
    
    // Valid config
    EXPECT_TRUE(config.validate());
    
    // Both mechanisms disabled
    config.enable_circuit_breaker = false;
    config.enable_retry = false;
    EXPECT_FALSE(config.validate());
    
    // Enable one mechanism
    config.enable_retry = true;
    EXPECT_TRUE(config.validate());
}

// Concurrency Tests
TEST_F(FaultToleranceTest, CircuitBreakerConcurrency) {
    circuit_breaker_config config;
    config.failure_threshold = 10;
    
    circuit_breaker<int> breaker("concurrent_test", config);
    
    constexpr int num_threads = 4;
    constexpr int operations_per_thread = 100;
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_operations{0};
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&breaker, &successful_operations, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                auto result = breaker.execute([]() { return make_success(1); });
                if (result) {
                    successful_operations++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successful_operations.load(), num_threads * operations_per_thread);
    
    auto metrics = breaker.get_metrics();
    EXPECT_EQ(metrics.total_calls.load(), num_threads * operations_per_thread);
    EXPECT_EQ(metrics.successful_calls.load(), num_threads * operations_per_thread);
}

// Edge Cases
TEST_F(FaultToleranceTest, CircuitBreakerReset) {
    circuit_breaker_config config;
    config.failure_threshold = 2;
    
    circuit_breaker<int> breaker("reset_test", config);
    
    // Open the circuit
    for (int i = 0; i < 2; ++i) {
        breaker.execute([this]() { return always_failing_operation(); });
    }
    EXPECT_EQ(breaker.get_state(), circuit_state::open);
    
    // Reset the circuit
    breaker.reset();
    EXPECT_EQ(breaker.get_state(), circuit_state::closed);
    
    // Should work normally now
    auto result = breaker.execute([this]() { return always_succeeding_operation(); });
    EXPECT_TRUE(result);
}

TEST_F(FaultToleranceTest, RetryExecutorResetMetrics) {
    auto config = create_exponential_backoff_config(3, std::chrono::milliseconds(10));
    retry_executor<int> executor("reset_test", config);
    
    // Execute some operations
    executor.execute([this]() { return always_succeeding_operation(); });
    executor.execute([this]() { return always_failing_operation(); });
    
    auto metrics_before = executor.get_metrics();
    EXPECT_GT(metrics_before.total_executions, 0);
    
    // Reset metrics
    executor.reset_metrics();
    
    auto metrics_after = executor.get_metrics();
    EXPECT_EQ(metrics_after.total_executions, 0);
    EXPECT_EQ(metrics_after.successful_executions, 0);
    EXPECT_EQ(metrics_after.failed_executions, 0);
}