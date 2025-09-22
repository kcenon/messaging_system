#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <monitoring/reliability/resource_manager.h>

using namespace monitoring_system;

class ResourceManagementTest : public ::testing::Test {
protected:
    void SetUp() override {
        call_count = 0;
        success_count = 0;
    }
    
    void TearDown() override {
        // Clean up any resources if needed
    }
    
    std::atomic<int> call_count{0};
    std::atomic<int> success_count{0};
    
    // Helper function for testing operations
    result<int> test_operation() {
        ++call_count;
        ++success_count;
        return make_success(42);
    }
    
    // Helper function that simulates work
    void simulate_work(std::chrono::milliseconds duration = std::chrono::milliseconds(10)) {
        std::this_thread::sleep_for(duration);
    }
};

// Token Bucket Rate Limiter Tests
TEST_F(ResourceManagementTest, TokenBucketBasicOperation) {
    auto limiter = create_token_bucket_limiter("test_limiter", 10, 10);
    
    // Should be able to acquire tokens initially
    EXPECT_TRUE(limiter->try_acquire(1));
    EXPECT_TRUE(limiter->try_acquire(5));
    
    // Should fail when bucket is empty
    EXPECT_FALSE(limiter->try_acquire(10));
}

TEST_F(ResourceManagementTest, TokenBucketRefill) {
    auto limiter = create_token_bucket_limiter("test_limiter", 100, 10);
    
    // Exhaust all tokens
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(limiter->try_acquire(1));
    }
    EXPECT_FALSE(limiter->try_acquire(1));
    
    // Wait for refill
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // Should have tokens again
    EXPECT_TRUE(limiter->try_acquire(1));
}

TEST_F(ResourceManagementTest, TokenBucketExecute) {
    auto limiter = create_token_bucket_limiter("test_limiter", 100, 5,
        throttling_strategy::reject);
    
    // Execute operations within limit
    for (int i = 0; i < 5; ++i) {
        auto result = limiter->execute([this]() { return test_operation(); });
        EXPECT_TRUE(result);
        EXPECT_EQ(result.value(), 42);
    }
    
    // Should reject when limit exceeded
    auto result = limiter->execute([this]() { return test_operation(); });
    EXPECT_FALSE(result);
    EXPECT_EQ(result.get_error().code, monitoring_error_code::resource_exhausted);
}

// Leaky Bucket Rate Limiter Tests  
TEST_F(ResourceManagementTest, LeakyBucketBasicOperation) {
    auto limiter = create_leaky_bucket_limiter("test_limiter", 10, 10);
    
    // Should be able to add items to bucket
    EXPECT_TRUE(limiter->try_acquire(1));
    EXPECT_TRUE(limiter->try_acquire(5));
    
    // Should fail when bucket is full
    EXPECT_FALSE(limiter->try_acquire(10));
}

TEST_F(ResourceManagementTest, LeakyBucketLeak) {
    auto limiter = create_leaky_bucket_limiter("test_limiter", 100, 5);
    
    // Fill bucket completely
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(limiter->try_acquire(1));
    }
    EXPECT_FALSE(limiter->try_acquire(1));
    
    // Wait for leak
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Should have space again
    EXPECT_TRUE(limiter->try_acquire(1));
}

// Memory Quota Manager Tests
TEST_F(ResourceManagementTest, MemoryQuotaBasicAllocation) {
    auto manager = create_memory_quota_manager("test_memory", 1024,
        throttling_strategy::reject);
    
    // Should allow allocation within quota
    auto result1 = manager->allocate(512);
    EXPECT_TRUE(result1);
    EXPECT_EQ(manager->current_usage(), 512);
    
    // Should allow another allocation
    auto result2 = manager->allocate(256);
    EXPECT_TRUE(result2);
    EXPECT_EQ(manager->current_usage(), 768);
    
    // Should reject when quota exceeded
    auto result3 = manager->allocate(512);
    EXPECT_FALSE(result3);
    EXPECT_EQ(result3.get_error().code, monitoring_error_code::resource_exhausted);
}

TEST_F(ResourceManagementTest, MemoryQuotaDeallocation) {
    auto manager = create_memory_quota_manager("test_memory", 1024);
    
    // Allocate memory
    auto result = manager->allocate(512);
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->current_usage(), 512);
    
    // Deallocate memory
    manager->deallocate(256);
    EXPECT_EQ(manager->current_usage(), 256);
    
    // Should be able to allocate again
    result = manager->allocate(512);
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->current_usage(), 768);
}

TEST_F(ResourceManagementTest, MemoryQuotaThresholds) {
    resource_quota quota(resource_type::memory, 1000, throttling_strategy::reject);
    quota.warning_threshold = 700;
    quota.critical_threshold = 900;
    
    auto manager = std::make_unique<memory_quota_manager>("test_memory", quota);
    
    // Should not be over thresholds initially
    EXPECT_FALSE(manager->is_over_warning_threshold());
    EXPECT_FALSE(manager->is_over_critical_threshold());
    
    // Allocate to warning level
    manager->allocate(750);
    EXPECT_TRUE(manager->is_over_warning_threshold());
    EXPECT_FALSE(manager->is_over_critical_threshold());
    
    // Allocate to critical level
    manager->allocate(150);
    EXPECT_TRUE(manager->is_over_warning_threshold());
    EXPECT_TRUE(manager->is_over_critical_threshold());
}

TEST_F(ResourceManagementTest, MemoryQuotaMetrics) {
    auto manager = create_memory_quota_manager("test_memory", 1024);
    
    // Check initial metrics
    auto metrics = manager->get_metrics();
    EXPECT_EQ(metrics.current_usage.load(), 0);
    EXPECT_EQ(metrics.total_allocations.load(), 0);
    
    // Allocate and check metrics
    manager->allocate(512);
    metrics = manager->get_metrics();
    EXPECT_EQ(metrics.current_usage.load(), 512);
    EXPECT_EQ(metrics.total_allocations.load(), 1);
    EXPECT_EQ(metrics.peak_usage.load(), 512);
    
    // Allocate more and check peak
    manager->allocate(256);
    metrics = manager->get_metrics();
    EXPECT_EQ(metrics.peak_usage.load(), 768);
}

// CPU Throttler Tests
TEST_F(ResourceManagementTest, CPUThrottlerBasicOperation) {
    cpu_throttle_config config;
    config.max_cpu_usage = 0.8;
    config.strategy = throttling_strategy::reject;
    config.check_interval = std::chrono::milliseconds(10);
    
    auto throttler = std::make_unique<cpu_throttler>("test_cpu", config);
    
    // Should execute when CPU usage is low
    auto result = throttler->execute([this]() { return test_operation(); });
    EXPECT_TRUE(result);
    EXPECT_EQ(result.value(), 42);
}

TEST_F(ResourceManagementTest, CPUThrottlerMetrics) {
    cpu_throttle_config config;
    config.max_cpu_usage = 0.8;
    config.strategy = throttling_strategy::delay;
    
    auto throttler = std::make_unique<cpu_throttler>("test_cpu", config);
    
    // Execute operation and check metrics
    throttler->execute([this]() { return test_operation(); });
    
    auto metrics = throttler->get_metrics();
    EXPECT_EQ(metrics.total_allocations.load(), 1);
}

// Resource Manager Tests
TEST_F(ResourceManagementTest, ResourceManagerRateLimiter) {
    auto manager = create_resource_manager("test_manager");
    
    rate_limit_config config;
    config.rate_per_second = 100;
    config.burst_capacity = 10;
    
    auto result = manager->add_rate_limiter("api_limiter", config);
    EXPECT_TRUE(result);
    
    auto limiter = manager->get_rate_limiter("api_limiter");
    EXPECT_NE(limiter, nullptr);
    EXPECT_EQ(limiter->get_name(), "api_limiter");
}

TEST_F(ResourceManagementTest, ResourceManagerMemoryQuota) {
    auto manager = create_resource_manager("test_manager");
    
    resource_quota quota(resource_type::memory, 2048, throttling_strategy::reject);
    
    auto result = manager->add_memory_quota("memory_quota", quota);
    EXPECT_TRUE(result);
    
    auto memory_manager = manager->get_memory_quota("memory_quota");
    EXPECT_NE(memory_manager, nullptr);
    EXPECT_EQ(memory_manager->get_name(), "memory_quota");
}

TEST_F(ResourceManagementTest, ResourceManagerCPUThrottler) {
    auto manager = create_resource_manager("test_manager");
    
    cpu_throttle_config config;
    config.max_cpu_usage = 0.7;
    config.strategy = throttling_strategy::delay;
    
    auto result = manager->add_cpu_throttler("cpu_throttler", config);
    EXPECT_TRUE(result);
    
    auto throttler = manager->get_cpu_throttler("cpu_throttler");
    EXPECT_NE(throttler, nullptr);
    EXPECT_EQ(throttler->get_name(), "cpu_throttler");
}

TEST_F(ResourceManagementTest, ResourceManagerDuplicateNames) {
    auto manager = create_resource_manager("test_manager");
    
    rate_limit_config config;
    config.rate_per_second = 100;
    config.burst_capacity = 10;
    
    // First addition should succeed
    auto result1 = manager->add_rate_limiter("duplicate_name", config);
    EXPECT_TRUE(result1);
    
    // Second addition with same name should fail
    auto result2 = manager->add_rate_limiter("duplicate_name", config);
    EXPECT_FALSE(result2);
    EXPECT_EQ(result2.get_error().code, monitoring_error_code::already_exists);
}

TEST_F(ResourceManagementTest, ResourceManagerHealthCheck) {
    auto manager = create_resource_manager("test_manager");
    
    // Add components
    resource_quota quota(resource_type::memory, 1024, throttling_strategy::reject);
    manager->add_memory_quota("memory", quota);
    
    cpu_throttle_config cpu_config;
    cpu_config.max_cpu_usage = 0.8;
    manager->add_cpu_throttler("cpu", cpu_config);
    
    // Should be healthy initially
    auto health = manager->is_healthy();
    EXPECT_TRUE(health);
    EXPECT_TRUE(health.value());
}

TEST_F(ResourceManagementTest, ResourceManagerMetrics) {
    auto manager = create_resource_manager("test_manager");
    
    // Add rate limiter
    rate_limit_config rate_config;
    rate_config.rate_per_second = 100;
    rate_config.burst_capacity = 10;
    manager->add_rate_limiter("rate", rate_config);
    
    // Add memory quota
    resource_quota quota(resource_type::memory, 1024, throttling_strategy::reject);
    manager->add_memory_quota("memory", quota);
    
    // Get all metrics
    auto all_metrics = manager->get_all_metrics();
    EXPECT_EQ(all_metrics.size(), 2);
    EXPECT_TRUE(all_metrics.find("rate_rate") != all_metrics.end());
    EXPECT_TRUE(all_metrics.find("memory_memory") != all_metrics.end());
}

// Configuration Validation Tests
TEST_F(ResourceManagementTest, RateLimitConfigValidation) {
    rate_limit_config config;
    
    // Valid config
    config.rate_per_second = 100;
    config.burst_capacity = 10;
    EXPECT_TRUE(config.validate());
    
    // Invalid rate
    config.rate_per_second = 0;
    EXPECT_FALSE(config.validate());
    
    // Invalid burst capacity
    config.rate_per_second = 100;
    config.burst_capacity = 0;
    EXPECT_FALSE(config.validate());
}

TEST_F(ResourceManagementTest, ResourceQuotaValidation) {
    resource_quota quota;
    
    // Valid quota
    quota.type = resource_type::memory;
    quota.max_value = 1024;
    quota.warning_threshold = 700;
    quota.critical_threshold = 900;
    EXPECT_TRUE(quota.validate());
    
    // Invalid max value
    quota.max_value = 0;
    EXPECT_FALSE(quota.validate());
    
    // Invalid thresholds
    quota.max_value = 1024;
    quota.warning_threshold = 1100;
    EXPECT_FALSE(quota.validate());
    
    quota.warning_threshold = 700;
    quota.critical_threshold = 600;
    EXPECT_FALSE(quota.validate());
}

TEST_F(ResourceManagementTest, CPUThrottleConfigValidation) {
    cpu_throttle_config config;
    
    // Valid config
    config.max_cpu_usage = 0.8;
    config.warning_threshold = 0.7;
    EXPECT_TRUE(config.validate());
    
    // Invalid max CPU usage
    config.max_cpu_usage = 0.0;
    EXPECT_FALSE(config.validate());
    
    config.max_cpu_usage = 1.5;
    EXPECT_FALSE(config.validate());
    
    // Invalid warning threshold
    config.max_cpu_usage = 0.8;
    config.warning_threshold = 0.9;
    EXPECT_FALSE(config.validate());
}

// Concurrency Tests
TEST_F(ResourceManagementTest, RateLimiterConcurrency) {
    auto limiter = create_token_bucket_limiter("concurrent_test", 1000, 100,
        throttling_strategy::reject);
    
    const int num_threads = 10;
    const int operations_per_thread = 10;
    std::atomic<int> successful_operations{0};
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                if (limiter->try_acquire(1)) {
                    successful_operations++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Should have some successful operations
    EXPECT_GT(successful_operations.load(), 0);
    EXPECT_LE(successful_operations.load(), 100); // Limited by burst capacity
}

TEST_F(ResourceManagementTest, MemoryQuotaConcurrency) {
    auto manager = create_memory_quota_manager("concurrent_memory", 10000,
        throttling_strategy::reject);
    
    const int num_threads = 5;
    const int allocations_per_thread = 10;
    std::atomic<int> successful_allocations{0};
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < allocations_per_thread; ++j) {
                if (manager->allocate(500)) {
                    successful_allocations++;
                    // Simulate some work then deallocate
                    simulate_work(std::chrono::milliseconds(1));
                    manager->deallocate(500);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(successful_allocations.load(), 0);
    EXPECT_EQ(manager->current_usage(), 0); // All should be deallocated
}

// Performance Tests
TEST_F(ResourceManagementTest, RateLimiterPerformance) {
    auto limiter = create_token_bucket_limiter("perf_test", 10000, 1000);
    
    const int num_operations = 1000;
    auto start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        limiter->try_acquire(1);
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start);
    
    // Should complete operations reasonably quickly
    EXPECT_LT(duration.count(), 100000); // Less than 100ms
}

TEST_F(ResourceManagementTest, MemoryQuotaPerformance) {
    auto manager = create_memory_quota_manager("perf_memory", 1000000);
    
    const int num_operations = 1000;
    auto start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < num_operations; ++i) {
        manager->allocate(100);
        manager->deallocate(100);
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start);
    
    // Should complete operations reasonably quickly
    EXPECT_LT(duration.count(), 50000); // Less than 50ms
}