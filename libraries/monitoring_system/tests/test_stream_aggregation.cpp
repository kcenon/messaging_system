#include <gtest/gtest.h>
#include <monitoring/utils/stream_aggregator.h>
#include <monitoring/utils/aggregation_processor.h>
#include <monitoring/utils/metric_storage.h>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <cmath>

using namespace monitoring_system;

/**
 * @brief Test suite for Phase 3 P2: Statistical aggregation functions
 */
class StreamAggregationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests
    }
    
    void TearDown() override {
        // Common cleanup for tests
    }
    
    /**
     * @brief Generate normal distribution samples
     */
    std::vector<double> generate_normal_samples(size_t count, double mean = 0.0, double stddev = 1.0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> dis(mean, stddev);
        
        std::vector<double> samples;
        samples.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            samples.push_back(dis(gen));
        }
        
        return samples;
    }
    
    /**
     * @brief Generate uniform distribution samples
     */
    std::vector<double> generate_uniform_samples(size_t count, double min = 0.0, double max = 1.0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        
        std::vector<double> samples;
        samples.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            samples.push_back(dis(gen));
        }
        
        return samples;
    }
};

// Online Statistics Tests
TEST_F(StreamAggregationTest, OnlineStatisticsBasic) {
    online_statistics stats;
    
    EXPECT_EQ(stats.count(), 0);
    EXPECT_EQ(stats.mean(), 0.0);
    EXPECT_EQ(stats.variance(), 0.0);
    
    // Add some values
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0};
    for (double value : values) {
        stats.add_value(value);
    }
    
    EXPECT_EQ(stats.count(), 5);
    EXPECT_DOUBLE_EQ(stats.mean(), 3.0);  // (1+2+3+4+5)/5 = 3
    
    auto full_stats = stats.get_statistics();
    EXPECT_EQ(full_stats.count, 5);
    EXPECT_DOUBLE_EQ(full_stats.mean, 3.0);
    EXPECT_DOUBLE_EQ(full_stats.min_value, 1.0);
    EXPECT_DOUBLE_EQ(full_stats.max_value, 5.0);
    EXPECT_GT(full_stats.variance, 0.0);
    EXPECT_GT(full_stats.std_deviation, 0.0);
}

TEST_F(StreamAggregationTest, OnlineStatisticsLargeDataset) {
    online_statistics stats;
    
    // Generate a large dataset with known properties
    auto samples = generate_normal_samples(10000, 100.0, 15.0);
    
    for (double sample : samples) {
        stats.add_value(sample);
    }
    
    auto full_stats = stats.get_statistics();
    
    // Check that the statistics are close to the expected values
    EXPECT_NEAR(full_stats.mean, 100.0, 1.0);  // Should be close to 100
    EXPECT_NEAR(full_stats.std_deviation, 15.0, 1.0);  // Should be close to 15
    EXPECT_EQ(full_stats.count, 10000);
}

// Quantile Estimator Tests
TEST_F(StreamAggregationTest, QuantileEstimatorMedian) {
    quantile_estimator median_estimator(0.5);
    
    // Add values 1 through 100
    for (int i = 1; i <= 100; ++i) {
        median_estimator.add_observation(static_cast<double>(i));
    }
    
    double estimated_median = median_estimator.get_quantile();
    
    // For 1-100, median should be around 50.5
    EXPECT_NEAR(estimated_median, 50.5, 5.0);  // Allow some tolerance for P² algorithm
}

TEST_F(StreamAggregationTest, QuantileEstimatorPercentiles) {
    quantile_estimator p95_estimator(0.95);
    
    // Add uniform samples 0-100
    auto samples = generate_uniform_samples(1000, 0.0, 100.0);
    
    for (double sample : samples) {
        p95_estimator.add_observation(sample);
    }
    
    double p95 = p95_estimator.get_quantile();
    
    // 95th percentile should be around 95
    EXPECT_NEAR(p95, 95.0, 10.0);  // P² algorithm has some approximation error
}

// Moving Window Aggregator Tests
TEST_F(StreamAggregationTest, MovingWindowBasic) {
    moving_window_aggregator<double> window(std::chrono::milliseconds(1000), 100);
    
    auto now = std::chrono::system_clock::now();
    
    // Add values
    for (int i = 0; i < 10; ++i) {
        window.add_value(static_cast<double>(i), now + std::chrono::milliseconds(i * 10));
    }
    
    EXPECT_EQ(window.size(), 10);
    
    auto values = window.get_values();
    EXPECT_EQ(values.size(), 10);
    
    // Check values are correct
    for (size_t i = 0; i < values.size(); ++i) {
        EXPECT_EQ(values[i], static_cast<double>(i));
    }
}

TEST_F(StreamAggregationTest, MovingWindowExpiration) {
    moving_window_aggregator<double> window(std::chrono::milliseconds(100), 1000);
    
    auto now = std::chrono::system_clock::now();
    
    // Add old values (should expire)
    for (int i = 0; i < 5; ++i) {
        window.add_value(static_cast<double>(i), now - std::chrono::milliseconds(200));
    }
    
    // Add new values (should remain)
    for (int i = 10; i < 15; ++i) {
        window.add_value(static_cast<double>(i), now);
    }
    
    auto values = window.get_values();
    
    // Should only have the new values
    EXPECT_EQ(values.size(), 5);
    for (size_t i = 0; i < values.size(); ++i) {
        EXPECT_EQ(values[i], static_cast<double>(i + 10));
    }
}

// Stream Aggregator Tests
TEST_F(StreamAggregationTest, StreamAggregatorBasic) {
    stream_aggregator_config config;
    config.window_size = 1000;
    config.enable_outlier_detection = false;  // Disable for predictable testing
    
    stream_aggregator aggregator(config);
    
    // Add observations
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    for (double value : values) {
        auto result = aggregator.add_observation(value);
        EXPECT_TRUE(result);
    }
    
    auto stats = aggregator.get_statistics();
    
    EXPECT_EQ(stats.count, 10);
    EXPECT_DOUBLE_EQ(stats.mean, 5.5);
    EXPECT_DOUBLE_EQ(stats.min_value, 1.0);
    EXPECT_DOUBLE_EQ(stats.max_value, 10.0);
    EXPECT_GT(stats.std_deviation, 0.0);
    
    // Check percentiles
    EXPECT_GT(stats.percentiles.size(), 0);
}

TEST_F(StreamAggregationTest, StreamAggregatorPercentiles) {
    stream_aggregator aggregator;
    
    // Add 100 values from 1 to 100
    for (int i = 1; i <= 100; ++i) {
        aggregator.add_observation(static_cast<double>(i));
    }
    
    // Get specific percentiles
    auto p50_result = aggregator.get_percentile(0.5);
    auto p95_result = aggregator.get_percentile(0.95);
    auto p99_result = aggregator.get_percentile(0.99);
    
    EXPECT_TRUE(p50_result);
    EXPECT_TRUE(p95_result);
    EXPECT_TRUE(p99_result);
    
    // Check approximate values
    EXPECT_NEAR(p50_result.value(), 50.0, 10.0);
    EXPECT_NEAR(p95_result.value(), 95.0, 10.0);
    EXPECT_NEAR(p99_result.value(), 99.0, 10.0);
}

TEST_F(StreamAggregationTest, StreamAggregatorOutlierDetection) {
    stream_aggregator_config config;
    config.enable_outlier_detection = true;
    config.outlier_threshold = 2.0;  // 2 standard deviations
    
    stream_aggregator aggregator(config);
    
    // Add normal values around 50
    for (int i = 45; i <= 55; ++i) {
        aggregator.add_observation(static_cast<double>(i));
    }
    
    // Add outliers
    aggregator.add_observation(100.0);  // Should be detected as outlier
    aggregator.add_observation(0.0);    // Should be detected as outlier
    
    auto stats = aggregator.get_statistics();
    
    EXPECT_GT(stats.outlier_count, 0);
    EXPECT_GT(stats.outliers.size(), 0);
}

TEST_F(StreamAggregationTest, StreamAggregatorReset) {
    stream_aggregator aggregator;
    
    // Add some observations
    for (int i = 1; i <= 10; ++i) {
        aggregator.add_observation(static_cast<double>(i));
    }
    
    EXPECT_EQ(aggregator.count(), 10);
    
    // Reset
    aggregator.reset();
    
    EXPECT_EQ(aggregator.count(), 0);
    EXPECT_EQ(aggregator.mean(), 0.0);
    EXPECT_EQ(aggregator.variance(), 0.0);
}

// Aggregation Processor Tests
TEST_F(StreamAggregationTest, AggregationProcessorBasic) {
    auto storage = std::make_shared<metric_storage>();
    aggregation_processor processor(storage);
    
    // Create aggregation rule
    aggregation_rule rule;
    rule.source_metric = "test_metric";
    rule.target_metric_prefix = "test_metric_stats";
    rule.aggregation_interval = std::chrono::milliseconds(1000);
    rule.percentiles = {0.5, 0.95};
    rule.compute_rate = true;
    rule.detect_outliers = false;
    
    auto add_result = processor.add_aggregation_rule(rule);
    EXPECT_TRUE(add_result);
    
    // Add observations
    for (int i = 1; i <= 100; ++i) {
        auto result = processor.process_observation("test_metric", static_cast<double>(i));
        EXPECT_TRUE(result);
    }
    
    // Get current statistics
    auto stats_result = processor.get_current_statistics("test_metric");
    EXPECT_TRUE(stats_result);
    
    auto stats = stats_result.value();
    EXPECT_EQ(stats.count, 100);
    EXPECT_GT(stats.mean, 0.0);
}

TEST_F(StreamAggregationTest, AggregationProcessorMultipleMetrics) {
    aggregation_processor processor;
    
    // Add rules for multiple metrics
    std::vector<std::string> metric_names = {"cpu_usage", "memory_usage", "network_io"};
    
    for (const auto& metric_name : metric_names) {
        aggregation_rule rule;
        rule.source_metric = metric_name;
        rule.target_metric_prefix = metric_name + "_stats";
        rule.aggregation_interval = std::chrono::milliseconds(500);
        
        auto result = processor.add_aggregation_rule(rule);
        EXPECT_TRUE(result);
    }
    
    // Add observations to each metric
    for (const auto& metric_name : metric_names) {
        for (int i = 1; i <= 50; ++i) {
            auto result = processor.process_observation(metric_name, static_cast<double>(i));
            EXPECT_TRUE(result);
        }
    }
    
    // Check configured metrics
    auto configured = processor.get_configured_metrics();
    EXPECT_EQ(configured.size(), 3);
    
    for (const auto& metric_name : metric_names) {
        EXPECT_NE(std::find(configured.begin(), configured.end(), metric_name), configured.end());
    }
}

TEST_F(StreamAggregationTest, AggregationProcessorForceAggregation) {
    auto storage = std::make_shared<metric_storage>();
    aggregation_processor processor(storage);
    
    aggregation_rule rule;
    rule.source_metric = "response_time";
    rule.target_metric_prefix = "response_time_agg";
    rule.aggregation_interval = std::chrono::hours(1);  // Long interval
    rule.percentiles = {0.5, 0.9, 0.95, 0.99};
    
    processor.add_aggregation_rule(rule);
    
    // Add observations
    auto samples = generate_normal_samples(1000, 100.0, 20.0);
    for (double sample : samples) {
        processor.process_observation("response_time", sample);
    }
    
    // Force aggregation before interval
    auto result = processor.force_aggregation("response_time");
    EXPECT_TRUE(result);
    
    auto agg_result = result.value();
    EXPECT_EQ(agg_result.source_metric, "response_time");
    EXPECT_EQ(agg_result.samples_processed, 1000);
    EXPECT_GT(agg_result.processing_duration.count(), 0);
    
    // Check that aggregated metrics were stored
    storage->flush();
    auto latest = storage->get_latest_value("response_time_agg.mean");
    EXPECT_TRUE(latest);
    EXPECT_NEAR(latest.value(), 100.0, 10.0);
}

TEST_F(StreamAggregationTest, AggregationProcessorInvalidRule) {
    aggregation_processor processor;
    
    // Test invalid rule (empty source metric)
    aggregation_rule invalid_rule;
    invalid_rule.source_metric = "";  // Invalid
    invalid_rule.target_metric_prefix = "test";
    
    auto result = processor.add_aggregation_rule(invalid_rule);
    EXPECT_FALSE(result);
    
    // Test duplicate rule
    aggregation_rule valid_rule;
    valid_rule.source_metric = "test_metric";
    valid_rule.target_metric_prefix = "test_stats";
    
    auto result1 = processor.add_aggregation_rule(valid_rule);
    EXPECT_TRUE(result1);
    
    auto result2 = processor.add_aggregation_rule(valid_rule);  // Duplicate
    EXPECT_FALSE(result2);
}

// Utility Function Tests
TEST_F(StreamAggregationTest, PearsonCorrelation) {
    // Perfect positive correlation
    std::vector<double> x1 = {1, 2, 3, 4, 5};
    std::vector<double> y1 = {2, 4, 6, 8, 10};
    
    double corr1 = pearson_correlation(x1, y1);
    EXPECT_NEAR(corr1, 1.0, 0.001);
    
    // Perfect negative correlation
    std::vector<double> x2 = {1, 2, 3, 4, 5};
    std::vector<double> y2 = {5, 4, 3, 2, 1};
    
    double corr2 = pearson_correlation(x2, y2);
    EXPECT_NEAR(corr2, -1.0, 0.001);
    
    // No correlation
    std::vector<double> x3 = {1, 2, 3, 4, 5};
    std::vector<double> y3 = {3, 3, 3, 3, 3};  // Constant
    
    double corr3 = pearson_correlation(x3, y3);
    EXPECT_NEAR(corr3, 0.0, 0.001);
    
    // Different sizes (should return 0)
    std::vector<double> x4 = {1, 2, 3};
    std::vector<double> y4 = {1, 2, 3, 4, 5};
    
    double corr4 = pearson_correlation(x4, y4);
    EXPECT_EQ(corr4, 0.0);
}

TEST_F(StreamAggregationTest, StandardAggregationRules) {
    auto rules = create_standard_aggregation_rules();
    
    EXPECT_GT(rules.size(), 0);
    
    // Validate all rules
    for (const auto& rule : rules) {
        auto validation = rule.validate();
        EXPECT_TRUE(validation) << "Rule validation failed for: " << rule.source_metric;
    }
    
    // Check that standard metrics are included
    std::vector<std::string> expected_metrics = {"response_time", "request_count", "error_count"};
    
    for (const auto& expected : expected_metrics) {
        bool found = false;
        for (const auto& rule : rules) {
            if (rule.source_metric == expected) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "Expected metric not found: " << expected;
    }
}

// Configuration Validation Tests
TEST_F(StreamAggregationTest, ConfigurationValidation) {
    // Test invalid stream aggregator config
    stream_aggregator_config invalid_config;
    invalid_config.window_size = 0;  // Invalid
    
    auto validation = invalid_config.validate();
    EXPECT_FALSE(validation);
    
    // Test valid config
    stream_aggregator_config valid_config;
    valid_config.window_size = 1000;
    valid_config.window_duration = std::chrono::milliseconds(60000);
    
    validation = valid_config.validate();
    EXPECT_TRUE(validation);
    
    // Test invalid aggregation rule
    aggregation_rule invalid_rule;
    invalid_rule.source_metric = "test";
    invalid_rule.target_metric_prefix = "";  // Invalid
    
    validation = invalid_rule.validate();
    EXPECT_FALSE(validation);
}

// Thread Safety Tests
TEST_F(StreamAggregationTest, StreamAggregatorThreadSafety) {
    stream_aggregator aggregator;
    
    const int num_threads = 4;
    const int observations_per_thread = 1000;
    std::vector<std::thread> threads;
    
    // Launch multiple threads adding observations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&aggregator, t, observations_per_thread]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 100.0);
            
            for (int i = 0; i < observations_per_thread; ++i) {
                double value = dis(gen);
                aggregator.add_observation(value);
                
                // Small delay to increase chance of contention
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify we processed all observations
    EXPECT_EQ(aggregator.count(), num_threads * observations_per_thread);
    
    auto stats = aggregator.get_statistics();
    EXPECT_GT(stats.mean, 0.0);
    EXPECT_GT(stats.std_deviation, 0.0);
}