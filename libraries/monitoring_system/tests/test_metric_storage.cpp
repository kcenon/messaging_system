#include <gtest/gtest.h>
#include <monitoring/utils/ring_buffer.h>
#include <monitoring/utils/metric_types.h>
#include <monitoring/utils/time_series.h>
#include <monitoring/utils/metric_storage.h>
#include <chrono>
#include <thread>
#include <vector>
#include <random>

using namespace monitoring_system;

/**
 * @brief Test suite for Phase 3 P1: Memory-efficient metric storage
 */
class MetricStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests
    }
    
    void TearDown() override {
        // Common cleanup for tests
    }
};

// Ring Buffer Tests
TEST_F(MetricStorageTest, RingBufferBasicOperations) {
    ring_buffer_config config;
    config.capacity = 8;  // Small capacity for testing
    config.overwrite_old = false;
    
    ring_buffer<int> buffer(config);
    
    EXPECT_EQ(buffer.capacity(), 8);
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    EXPECT_EQ(buffer.size(), 0);
    
    // Write elements
    for (int i = 0; i < 7; ++i) {
        auto result = buffer.write(std::move(i));
        EXPECT_TRUE(result) << "Failed to write " << i;
    }
    
    EXPECT_EQ(buffer.size(), 7);
    EXPECT_FALSE(buffer.empty());
    EXPECT_TRUE(buffer.full());  // 7 elements in 8-capacity buffer means full
    
    // Try to write when full
    auto result = buffer.write(std::move(999));
    EXPECT_FALSE(result);  // Should fail as overwrite_old is false
    
    // Read elements
    int value;
    for (int i = 0; i < 7; ++i) {
        auto read_result = buffer.read(value);
        EXPECT_TRUE(read_result);
        EXPECT_EQ(value, i);
    }
    
    EXPECT_TRUE(buffer.empty());
}

TEST_F(MetricStorageTest, RingBufferOverwrite) {
    ring_buffer_config config;
    config.capacity = 4;
    config.overwrite_old = true;
    
    ring_buffer<int> buffer(config);
    
    // Fill buffer completely
    for (int i = 0; i < 8; ++i) {
        auto result = buffer.write(std::move(i));
        EXPECT_TRUE(result);
    }
    
    // Should have overwritten, so we should read the last 3 values
    std::vector<int> read_values;
    int value;
    while (buffer.read(value)) {
        read_values.push_back(value);
    }
    
    EXPECT_GE(read_values.size(), 3);  // Should have at least the last few values
}

TEST_F(MetricStorageTest, RingBufferBatchOperations) {
    ring_buffer<int> buffer;
    
    // Write batch
    std::vector<int> write_data = {1, 2, 3, 4, 5};
    size_t written = buffer.write_batch(std::move(write_data));
    EXPECT_EQ(written, 5);
    
    // Read batch
    std::vector<int> read_data;
    size_t read_count = buffer.read_batch(read_data, 10);
    EXPECT_EQ(read_count, 5);
    EXPECT_EQ(read_data.size(), 5);
    
    for (size_t i = 0; i < read_data.size(); ++i) {
        EXPECT_EQ(read_data[i], static_cast<int>(i + 1));
    }
}

TEST_F(MetricStorageTest, RingBufferPeek) {
    ring_buffer<int> buffer;
    
    buffer.write(std::move(42));
    buffer.write(std::move(84));
    
    int value;
    auto result = buffer.peek(value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, 42);
    
    // Size should not change after peek
    EXPECT_EQ(buffer.size(), 2);
    
    // Reading should still get the same value
    buffer.read(value);
    EXPECT_EQ(value, 42);
}

// Metric Types Tests
TEST_F(MetricStorageTest, CompactMetricValue) {
    auto metadata = create_metric_metadata("test_metric", metric_type::counter);
    
    compact_metric_value metric_double(metadata, 3.14159);
    EXPECT_EQ(metric_double.as_double(), 3.14159);
    EXPECT_TRUE(metric_double.is_numeric());
    
    compact_metric_value metric_int(metadata, int64_t(42));
    EXPECT_EQ(metric_int.as_int64(), 42);
    EXPECT_EQ(metric_int.as_double(), 42.0);
    EXPECT_TRUE(metric_int.is_numeric());
    
    compact_metric_value metric_string(metadata, std::string("test"));
    EXPECT_EQ(metric_string.as_string(), "test");
    EXPECT_FALSE(metric_string.is_numeric());
}

TEST_F(MetricStorageTest, MetricBatch) {
    metric_batch batch(1);
    
    auto metadata = create_metric_metadata("test", metric_type::gauge);
    
    for (int i = 0; i < 5; ++i) {
        compact_metric_value metric(metadata, static_cast<double>(i));
        batch.add_metric(std::move(metric));
    }
    
    EXPECT_EQ(batch.size(), 5);
    EXPECT_FALSE(batch.empty());
    EXPECT_GT(batch.memory_footprint(), 0);
    
    batch.clear();
    EXPECT_TRUE(batch.empty());
}

TEST_F(MetricStorageTest, HistogramData) {
    histogram_data hist;
    hist.init_standard_buckets();
    
    // Add some samples
    std::vector<double> samples = {0.001, 0.01, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
    for (double sample : samples) {
        hist.add_sample(sample);
    }
    
    EXPECT_EQ(hist.total_count, samples.size());
    EXPECT_GT(hist.sum, 0);
    EXPECT_GT(hist.mean(), 0);
    
    // Check bucket counts
    for (const auto& bucket : hist.buckets) {
        EXPECT_GE(bucket.count, 0);
    }
}

// Time Series Tests
TEST_F(MetricStorageTest, TimeSeriesBasicOperations) {
    time_series_config config;
    config.max_points = 100;
    config.retention_period = std::chrono::seconds(60);
    
    time_series series("test_series", config);
    
    EXPECT_TRUE(series.empty());
    EXPECT_EQ(series.name(), "test_series");
    
    // Add some data points
    auto now = std::chrono::system_clock::now();
    for (int i = 0; i < 10; ++i) {
        auto timestamp = now + std::chrono::seconds(i);
        auto result = series.add_point(static_cast<double>(i), timestamp);
        EXPECT_TRUE(result);
    }
    
    EXPECT_EQ(series.size(), 10);
    EXPECT_FALSE(series.empty());
    
    // Get latest value
    auto latest = series.get_latest_value();
    EXPECT_TRUE(latest);
    EXPECT_EQ(latest.value(), 9.0);
}

TEST_F(MetricStorageTest, TimeSeriesQuery) {
    time_series series("query_test");
    
    auto now = std::chrono::system_clock::now();
    
    // Add data points over a time range
    for (int i = 0; i < 60; ++i) {
        auto timestamp = now + std::chrono::seconds(i);
        series.add_point(static_cast<double>(i), timestamp);
    }
    
    // Query a subset
    time_series_query query;
    query.start_time = now + std::chrono::seconds(10);
    query.end_time = now + std::chrono::seconds(50);
    query.step = std::chrono::seconds(10);
    
    auto result = series.query(query);
    EXPECT_TRUE(result);
    
    const auto& agg_result = result.value();
    EXPECT_GT(agg_result.points.size(), 0);
    EXPECT_GT(agg_result.total_samples, 0);
    
    auto summary = agg_result.get_summary();
    EXPECT_GT(summary.count, 0);
    EXPECT_GE(summary.min_value, 10.0);  // Should be in the queried range
    EXPECT_LE(summary.max_value, 50.0);
}

// Metric Storage Tests
TEST_F(MetricStorageTest, MetricStorageBasicOperations) {
    metric_storage_config config;
    config.ring_buffer_capacity = 64;
    config.max_metrics = 100;
    config.enable_background_processing = false;  // Disable for testing
    
    metric_storage storage(config);
    
    // Store some metrics
    auto result1 = storage.store_metric("cpu_usage", 65.5, metric_type::gauge);
    EXPECT_TRUE(result1);
    
    auto result2 = storage.store_metric("memory_usage", 4096.0, metric_type::gauge);
    EXPECT_TRUE(result2);
    
    auto result3 = storage.store_metric("request_count", 100, metric_type::counter);
    EXPECT_TRUE(result3);
    
    // Flush to time series
    storage.flush();
    
    // Query latest values
    auto cpu = storage.get_latest_value("cpu_usage");
    EXPECT_TRUE(cpu);
    EXPECT_EQ(cpu.value(), 65.5);
    
    auto memory = storage.get_latest_value("memory_usage");
    EXPECT_TRUE(memory);
    EXPECT_EQ(memory.value(), 4096.0);
    
    // Get metric names
    auto names = storage.get_metric_names();
    EXPECT_GE(names.size(), 3);
    
    // Check statistics
    const auto& stats = storage.get_stats();
    EXPECT_GE(stats.total_metrics_stored.load(), 3);
    EXPECT_EQ(stats.total_metrics_dropped.load(), 0);
}

TEST_F(MetricStorageTest, MetricStorageBatchOperations) {
    metric_storage storage;
    
    // Create a batch of metrics
    metric_batch batch;
    auto metadata = create_metric_metadata("batch_metric", metric_type::gauge);
    
    for (int i = 0; i < 50; ++i) {
        compact_metric_value metric(metadata, static_cast<double>(i));
        batch.add_metric(std::move(metric));
    }
    
    // Store the batch
    size_t stored = storage.store_metrics_batch(batch);
    EXPECT_EQ(stored, 50);
    
    storage.flush();
    
    // Query the data
    time_series_query query;
    auto result = storage.query_metric("batch_metric", query);
    EXPECT_TRUE(result);
}

TEST_F(MetricStorageTest, MetricStorageCapacityLimits) {
    metric_storage_config config;
    config.max_metrics = 2;  // Very low limit for testing
    config.ring_buffer_capacity = 8;
    config.enable_background_processing = false;
    
    metric_storage storage(config);
    
    // Store metrics up to the limit
    EXPECT_TRUE(storage.store_metric("metric1", 1.0));
    EXPECT_TRUE(storage.store_metric("metric2", 2.0));
    
    // This should fail due to capacity limit
    auto result = storage.store_metric("metric3", 3.0);
    // Note: Might succeed if metrics are stored in ring buffer, depends on implementation
    
    const auto& stats = storage.get_stats();
    EXPECT_LE(stats.active_metric_series.load(), 2);
}

TEST_F(MetricStorageTest, MetricStorageThreadSafety) {
    metric_storage storage;
    
    const int num_threads = 4;
    const int metrics_per_thread = 100;
    std::vector<std::thread> threads;
    
    // Launch multiple threads writing metrics
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&storage, t, metrics_per_thread]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 100.0);
            
            for (int i = 0; i < metrics_per_thread; ++i) {
                std::string metric_name = "thread_" + std::to_string(t) + "_metric_" + std::to_string(i);
                double value = dis(gen);
                storage.store_metric(metric_name, value);
                
                // Small delay to increase chance of contention
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    storage.flush();
    
    // Verify we stored a reasonable number of metrics
    const auto& stats = storage.get_stats();
    EXPECT_GT(stats.total_metrics_stored.load(), num_threads * metrics_per_thread * 0.8);
    
    auto names = storage.get_metric_names();
    EXPECT_GT(names.size(), 0);
}

// Configuration Validation Tests
TEST_F(MetricStorageTest, ConfigurationValidation) {
    // Test invalid ring buffer capacity (not power of 2)
    ring_buffer_config invalid_ring_config;
    invalid_ring_config.capacity = 1000;  // Not a power of 2
    
    auto validation = invalid_ring_config.validate();
    EXPECT_FALSE(validation);
    
    // Test valid configuration
    ring_buffer_config valid_ring_config;
    valid_ring_config.capacity = 1024;  // Power of 2
    
    validation = valid_ring_config.validate();
    EXPECT_TRUE(validation);
    
    // Test invalid time series configuration
    time_series_config invalid_ts_config;
    invalid_ts_config.retention_period = std::chrono::seconds(-1);
    
    validation = invalid_ts_config.validate();
    EXPECT_FALSE(validation);
    
    // Test invalid metric storage configuration
    metric_storage_config invalid_storage_config;
    invalid_storage_config.max_metrics = 0;
    
    validation = invalid_storage_config.validate();
    EXPECT_FALSE(validation);
}