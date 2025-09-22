#include <gtest/gtest.h>
#include <monitoring/optimization/lockfree_queue.h>
#include <monitoring/optimization/memory_pool.h>
#include <monitoring/optimization/simd_aggregator.h>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <functional>

using namespace monitoring_system;

/**
 * @brief Test suite for Phase 3 P4: Lock-free data structures integration
 */
class OptimizationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests
    }
    
    void TearDown() override {
        // Common cleanup for tests
    }
    
    /**
     * @brief Generate test data
     */
    std::vector<double> generate_test_data(size_t size, double min_val = 0.0, double max_val = 100.0) {
        std::vector<double> data;
        data.reserve(size);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(min_val, max_val);
        
        for (size_t i = 0; i < size; ++i) {
            data.push_back(dist(gen));
        }
        
        return data;
    }
};

// Lock-free Queue Tests
TEST_F(OptimizationTest, LockfreeQueueBasicOperations) {
    lockfree_queue_config config;
    config.initial_capacity = 16;
    config.max_capacity = 64;
    
    lockfree_queue<int> queue(config);
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
    
    // Push elements
    for (int i = 0; i < 10; ++i) {
        auto result = queue.push(i);
        EXPECT_TRUE(result) << "Failed to push " << i;
    }
    
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 10);
    
    // Pop elements
    for (int i = 0; i < 10; ++i) {
        auto result = queue.pop();
        EXPECT_TRUE(result) << "Failed to pop element " << i;
        EXPECT_EQ(result.value(), i);
    }
    
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0);
}

TEST_F(OptimizationTest, LockfreeQueueConcurrentAccess) {
    lockfree_queue_config config;
    config.initial_capacity = 1024;
    config.max_capacity = 4096;
    
    lockfree_queue<int> queue(config);
    
    const int num_producers = 4;
    const int num_consumers = 2;
    const int items_per_producer = 1000;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    std::atomic<int> total_consumed{0};
    std::atomic<bool> producers_done{false};
    
    // Start producers
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&queue, p, items_per_producer]() {
            for (int i = 0; i < items_per_producer; ++i) {
                int value = p * items_per_producer + i;
                while (!queue.push(value)) {
                    std::this_thread::yield();
                }
            }
        });
    }
    
    // Start consumers
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&queue, &total_consumed, &producers_done]() {
            while (!producers_done.load() || !queue.empty()) {
                auto result = queue.pop();
                if (result) {
                    total_consumed.fetch_add(1);
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }
    
    // Wait for producers
    for (auto& producer : producers) {
        producer.join();
    }
    producers_done.store(true);
    
    // Wait for consumers
    for (auto& consumer : consumers) {
        consumer.join();
    }
    
    EXPECT_EQ(total_consumed.load(), num_producers * items_per_producer);
    EXPECT_TRUE(queue.empty());
    
    const auto& stats = queue.get_statistics();
    EXPECT_GT(stats.get_push_success_rate(), 99.0);
    EXPECT_GT(stats.get_pop_success_rate(), 99.0);
}

// Memory Pool Tests
TEST_F(OptimizationTest, MemoryPoolBasicOperations) {
    memory_pool_config config;
    config.initial_blocks = 64;
    config.max_blocks = 256;
    config.block_size = 128;
    
    memory_pool pool(config);
    
    EXPECT_GT(pool.available_blocks(), 0);
    EXPECT_EQ(pool.total_blocks(), 64);
    
    // Allocate memory blocks
    std::vector<void*> allocated_blocks;
    for (int i = 0; i < 32; ++i) {
        auto result = pool.allocate();
        EXPECT_TRUE(result) << "Failed to allocate block " << i;
        allocated_blocks.push_back(result.value());
    }
    
    EXPECT_EQ(pool.available_blocks(), 32);  // 64 - 32 = 32
    
    // Deallocate blocks
    for (auto* ptr : allocated_blocks) {
        auto result = pool.deallocate(ptr);
        EXPECT_TRUE(result);
    }
    
    EXPECT_EQ(pool.available_blocks(), 64);  // All blocks returned
    
    const auto& stats = pool.get_statistics();
    EXPECT_EQ(stats.total_allocations.load(), 32);
    EXPECT_EQ(stats.total_deallocations.load(), 32);
    EXPECT_GT(stats.get_allocation_success_rate(), 99.0);
}

TEST_F(OptimizationTest, MemoryPoolObjectAllocation) {
    memory_pool_config config;
    config.initial_blocks = 128;
    config.block_size = 64;  // Enough for test objects
    
    memory_pool pool(config);
    
    struct TestObject {
        int value;
        double data;
        
        TestObject(int v, double d) : value(v), data(d) {}
    };
    
    // Allocate objects
    std::vector<TestObject*> objects;
    for (int i = 0; i < 50; ++i) {
        auto result = pool.allocate_object<TestObject>(i, i * 0.5);
        EXPECT_TRUE(result) << "Failed to allocate object " << i;
        
        auto* obj = result.value();
        EXPECT_EQ(obj->value, i);
        EXPECT_DOUBLE_EQ(obj->data, i * 0.5);
        
        objects.push_back(obj);
    }
    
    // Deallocate objects
    for (auto* obj : objects) {
        auto result = pool.deallocate_object(obj);
        EXPECT_TRUE(result);
    }
}

TEST_F(OptimizationTest, MemoryPoolConcurrentAccess) {
    memory_pool_config config;
    config.initial_blocks = 1024;
    config.max_blocks = 4096;
    config.block_size = 64;
    config.use_thread_local_cache = true;
    
    memory_pool pool(config);
    
    const int num_threads = 8;
    const int operations_per_thread = 500;
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_operations{0};
    
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&pool, &successful_operations, operations_per_thread]() {
            std::vector<void*> allocated_ptrs;
            
            // Allocate
            for (int i = 0; i < operations_per_thread; ++i) {
                auto result = pool.allocate();
                if (result) {
                    allocated_ptrs.push_back(result.value());
                }
            }
            
            // Deallocate
            for (auto* ptr : allocated_ptrs) {
                auto result = pool.deallocate(ptr);
                if (result) {
                    successful_operations.fetch_add(1);
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(successful_operations.load(), num_threads * operations_per_thread * 0.95);
    
    const auto& stats = pool.get_statistics();
    EXPECT_GT(stats.get_allocation_success_rate(), 95.0);
}

// SIMD Aggregator Tests
TEST_F(OptimizationTest, SIMDAggregatorBasicOperations) {
    simd_config config;
    simd_aggregator aggregator(config);
    
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    
    // Test sum
    auto sum_result = aggregator.sum(data);
    EXPECT_TRUE(sum_result);
    EXPECT_DOUBLE_EQ(sum_result.value(), 36.0);
    
    // Test mean
    auto mean_result = aggregator.mean(data);
    EXPECT_TRUE(mean_result);
    EXPECT_DOUBLE_EQ(mean_result.value(), 4.5);
    
    // Test min/max
    auto min_result = aggregator.min(data);
    EXPECT_TRUE(min_result);
    EXPECT_DOUBLE_EQ(min_result.value(), 1.0);
    
    auto max_result = aggregator.max(data);
    EXPECT_TRUE(max_result);
    EXPECT_DOUBLE_EQ(max_result.value(), 8.0);
    
    // Test variance
    auto var_result = aggregator.variance(data);
    EXPECT_TRUE(var_result);
    EXPECT_GT(var_result.value(), 0.0);  // Should be positive for this dataset
}

TEST_F(OptimizationTest, SIMDAggregatorLargeDataset) {
    simd_config config;
    config.enable_simd = true;
    simd_aggregator aggregator(config);
    
    // Generate large dataset
    auto data = generate_test_data(10000, 0.0, 100.0);
    
    // Test statistical summary
    auto summary_result = aggregator.compute_summary(data);
    EXPECT_TRUE(summary_result);
    
    const auto& summary = summary_result.value();
    EXPECT_EQ(summary.count, 10000);
    EXPECT_GT(summary.sum, 0.0);
    EXPECT_GT(summary.mean, 0.0);
    EXPECT_LT(summary.mean, 100.0);
    EXPECT_GT(summary.variance, 0.0);
    EXPECT_GT(summary.std_dev, 0.0);
    EXPECT_GE(summary.min_val, 0.0);
    EXPECT_LE(summary.max_val, 100.0);
    EXPECT_LT(summary.min_val, summary.max_val);
    
    // Check SIMD utilization
    const auto& stats = aggregator.get_statistics();
    if (aggregator.get_capabilities().avx2_available || aggregator.get_capabilities().neon_available) {
        EXPECT_GT(stats.get_simd_utilization(), 0.0);
    }
}

TEST_F(OptimizationTest, SIMDAggregatorPerformanceComparison) {
    // Test SIMD vs scalar performance
    simd_config simd_cfg;
    simd_cfg.enable_simd = true;
    
    simd_config scalar_cfg;
    scalar_cfg.enable_simd = false;
    
    simd_aggregator simd_agg(simd_cfg);
    simd_aggregator scalar_agg(scalar_cfg);
    
    auto large_data = generate_test_data(50000, 0.0, 1000.0);
    
    // Measure SIMD performance
    auto start_simd = std::chrono::high_resolution_clock::now();
    auto simd_summary = simd_agg.compute_summary(large_data);
    auto end_simd = std::chrono::high_resolution_clock::now();
    
    // Measure scalar performance
    auto start_scalar = std::chrono::high_resolution_clock::now();
    auto scalar_summary = scalar_agg.compute_summary(large_data);
    auto end_scalar = std::chrono::high_resolution_clock::now();
    
    EXPECT_TRUE(simd_summary);
    EXPECT_TRUE(scalar_summary);
    
    // Results should be approximately equal
    const auto& simd_result = simd_summary.value();
    const auto& scalar_result = scalar_summary.value();
    
    EXPECT_NEAR(simd_result.sum, scalar_result.sum, 1e-6);
    EXPECT_NEAR(simd_result.mean, scalar_result.mean, 1e-6);
    EXPECT_NEAR(simd_result.min_val, scalar_result.min_val, 1e-6);
    EXPECT_NEAR(simd_result.max_val, scalar_result.max_val, 1e-6);
    
    auto simd_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_simd - start_simd);
    auto scalar_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_scalar - start_scalar);
    
    // SIMD should be faster or at least comparable (depending on data size and CPU)
    std::cout << "SIMD duration: " << simd_duration.count() << " μs" << std::endl;
    std::cout << "Scalar duration: " << scalar_duration.count() << " μs" << std::endl;
    
    if (simd_agg.get_capabilities().avx2_available || simd_agg.get_capabilities().neon_available) {
        // SIMD should provide some benefit for large datasets
        EXPECT_LE(simd_duration.count(), scalar_duration.count() * 1.2);  // Allow 20% tolerance
    }
}

// Configuration Tests
TEST_F(OptimizationTest, ConfigurationValidation) {
    // Test lockfree queue config validation
    lockfree_queue_config invalid_queue_config;
    invalid_queue_config.initial_capacity = 0;  // Invalid
    EXPECT_FALSE(invalid_queue_config.validate());
    
    lockfree_queue_config valid_queue_config;
    valid_queue_config.initial_capacity = 1024;
    valid_queue_config.max_capacity = 4096;
    EXPECT_TRUE(valid_queue_config.validate());
    
    // Test memory pool config validation
    memory_pool_config invalid_pool_config;
    invalid_pool_config.block_size = 7;  // Not 8-byte aligned
    EXPECT_FALSE(invalid_pool_config.validate());
    
    memory_pool_config valid_pool_config;
    valid_pool_config.initial_blocks = 256;
    valid_pool_config.max_blocks = 1024;
    valid_pool_config.block_size = 64;
    EXPECT_TRUE(valid_pool_config.validate());
    
    // Test SIMD config validation
    simd_config invalid_simd_config;
    invalid_simd_config.vector_size = 7;  // Not power of 2
    EXPECT_FALSE(invalid_simd_config.validate());
    
    simd_config valid_simd_config;
    valid_simd_config.vector_size = 8;
    valid_simd_config.alignment = 32;
    EXPECT_TRUE(valid_simd_config.validate());
}

// Factory Functions Tests
TEST_F(OptimizationTest, FactoryFunctions) {
    // Test lockfree queue factory
    auto queue = make_lockfree_queue<int>();
    EXPECT_NE(queue, nullptr);
    EXPECT_TRUE(queue->empty());
    
    // Test memory pool factory
    auto pool = make_memory_pool();
    EXPECT_NE(pool, nullptr);
    EXPECT_GT(pool->available_blocks(), 0);
    
    // Test SIMD aggregator factory
    auto aggregator = make_simd_aggregator();
    EXPECT_NE(aggregator, nullptr);
    
    // Test SIMD functionality
    auto test_result = aggregator->test_simd();
    EXPECT_TRUE(test_result);
    EXPECT_TRUE(test_result.value());
}

// Default Configurations Tests
TEST_F(OptimizationTest, DefaultConfigurations) {
    // Test default queue configurations
    auto queue_configs = create_default_queue_configs();
    EXPECT_GE(queue_configs.size(), 3);
    
    for (const auto& config : queue_configs) {
        EXPECT_TRUE(config.validate());
    }
    
    // Test default pool configurations
    auto pool_configs = create_default_pool_configs();
    EXPECT_GE(pool_configs.size(), 3);
    
    for (const auto& config : pool_configs) {
        EXPECT_TRUE(config.validate());
    }
    
    // Test default SIMD configurations
    auto simd_configs = create_default_simd_configs();
    EXPECT_GE(simd_configs.size(), 3);
    
    for (const auto& config : simd_configs) {
        EXPECT_TRUE(config.validate());
    }
}

// Integration Tests
TEST_F(OptimizationTest, IntegrationTest) {
    // Test integration of all optimization components
    lockfree_queue<double> queue;
    memory_pool pool;
    simd_aggregator aggregator;
    
    // Generate data and push to queue
    auto test_data = generate_test_data(1000);
    
    for (double value : test_data) {
        auto result = queue.push(value);
        EXPECT_TRUE(result);
    }
    
    // Pop data from queue and collect in vector
    std::vector<double> collected_data;
    collected_data.reserve(test_data.size());
    
    while (!queue.empty()) {
        auto result = queue.pop();
        if (result) {
            collected_data.push_back(result.value());
        }
    }
    
    EXPECT_EQ(collected_data.size(), test_data.size());
    
    // Use SIMD aggregator to process collected data
    auto summary = aggregator.compute_summary(collected_data);
    EXPECT_TRUE(summary);
    
    const auto& stats = summary.value();
    EXPECT_EQ(stats.count, test_data.size());
    EXPECT_GT(stats.sum, 0.0);
    EXPECT_GT(stats.mean, 0.0);
    
    // Verify statistics from all components
    const auto& queue_stats = queue.get_statistics();
    const auto& pool_stats = pool.get_statistics();
    const auto& simd_stats = aggregator.get_statistics();
    
    EXPECT_GT(queue_stats.get_push_success_rate(), 99.0);
    EXPECT_GT(queue_stats.get_pop_success_rate(), 99.0);
    EXPECT_GT(pool_stats.get_allocation_success_rate(), 99.0);
    EXPECT_GT(simd_stats.total_elements_processed.load(), 0);
}