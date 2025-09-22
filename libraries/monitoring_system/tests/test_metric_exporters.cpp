/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include <kcenon/monitoring/exporters/metric_exporters.h>
#include <kcenon/monitoring/interfaces/monitorable_interface.h>
#include <kcenon/monitoring/interfaces/monitoring_interface.h>
#include <kcenon/monitoring/exporters/opentelemetry_adapter.h>
#include <thread>
#include <chrono>

using namespace monitoring_system;

class MetricExportersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test monitoring data
        test_data_ = create_test_monitoring_data();
        
        // Create test metrics snapshot
        test_snapshot_ = create_test_snapshot();
        
        // Create OTEL resource
        otel_resource_ = create_service_resource("test_service", "1.0.0", "test_namespace");
    }
    
    monitoring_data create_test_monitoring_data() {
        monitoring_data data("web_server");
        data.add_metric("http_requests_total", 1500.0);
        data.add_metric("http_request_duration_seconds", 0.250);
        data.add_metric("memory_usage_bytes", 1024000.0);
        data.add_metric("cpu_usage_percent", 75.5);
        
        data.add_tag("environment", "production");
        data.add_tag("region", "us-west-2");
        data.add_tag("version", "1.2.3");
        
        return data;
    }
    
    metrics_snapshot create_test_snapshot() {
        metrics_snapshot snapshot;
        snapshot.source_id = "system_monitor";
        snapshot.capture_time = std::chrono::system_clock::now();
        
        snapshot.add_metric("system_load_1m", 2.1);
        snapshot.add_metric("system_load_5m", 1.8);
        snapshot.add_metric("disk_usage_percent", 68.3);
        snapshot.add_metric("network_bytes_in", 987654.0);
        snapshot.add_metric("network_bytes_out", 654321.0);
        
        // Add tags to metrics
        snapshot.metrics[0].tags["host"] = "server01";
        snapshot.metrics[1].tags["host"] = "server01";
        snapshot.metrics[2].tags["mount"] = "/var";
        snapshot.metrics[3].tags["interface"] = "eth0";
        snapshot.metrics[4].tags["interface"] = "eth0";
        
        return snapshot;
    }
    
    monitoring_data test_data_;
    metrics_snapshot test_snapshot_;
    otel_resource otel_resource_;
};

TEST_F(MetricExportersTest, MetricExportConfigValidation) {
    // Valid configuration
    metric_export_config valid_config;
    valid_config.endpoint = "http://prometheus:9090";
    valid_config.format = metric_export_format::prometheus_text;
    valid_config.push_interval = std::chrono::milliseconds(15000);
    valid_config.max_batch_size = 1000;
    valid_config.max_queue_size = 10000;
    
    auto validation = valid_config.validate();
    EXPECT_TRUE(validation);
    
    // Valid configuration with port
    metric_export_config port_config;
    port_config.port = 8125;
    port_config.format = metric_export_format::statsd_plain;
    auto port_validation = port_config.validate();
    EXPECT_TRUE(port_validation);
    
    // Invalid configuration (no endpoint or port)
    metric_export_config invalid_config;
    invalid_config.format = metric_export_format::prometheus_text;
    auto invalid_validation = invalid_config.validate();
    EXPECT_FALSE(invalid_validation);
    EXPECT_EQ(invalid_validation.get_error().code, monitoring_error_code::invalid_configuration);
    
    // Invalid push interval
    metric_export_config invalid_interval;
    invalid_interval.endpoint = "http://test";
    invalid_interval.push_interval = std::chrono::milliseconds(0);
    auto interval_validation = invalid_interval.validate();
    EXPECT_FALSE(interval_validation);
    
    // Invalid batch size
    metric_export_config invalid_batch;
    invalid_batch.endpoint = "http://test";
    invalid_batch.max_batch_size = 0;
    auto batch_validation = invalid_batch.validate();
    EXPECT_FALSE(batch_validation);
    
    // Invalid queue size
    metric_export_config invalid_queue;
    invalid_queue.endpoint = "http://test";
    invalid_queue.max_batch_size = 1000;
    invalid_queue.max_queue_size = 500;
    auto queue_validation = invalid_queue.validate();
    EXPECT_FALSE(queue_validation);
}

TEST_F(MetricExportersTest, PrometheusMetricConversion) {
    metric_export_config config;
    config.endpoint = "http://prometheus:9090";
    config.format = metric_export_format::prometheus_text;
    config.instance_id = "test_instance";
    config.labels["datacenter"] = "dc1";
    
    prometheus_exporter exporter(config);
    
    // Test monitoring_data conversion
    auto prom_metrics = exporter.convert_monitoring_data(test_data_);
    EXPECT_EQ(prom_metrics.size(), 4); // 4 metrics in test data
    
    // Find http_requests_total metric
    auto it = std::find_if(prom_metrics.begin(), prom_metrics.end(),
        [](const prometheus_metric_data& m) { return m.name == "http_requests_total"; });
    ASSERT_NE(it, prom_metrics.end());
    
    const auto& requests_metric = *it;
    EXPECT_EQ(requests_metric.name, "http_requests_total");
    EXPECT_EQ(requests_metric.type, metric_type::counter);
    EXPECT_EQ(requests_metric.value, 1500.0);
    EXPECT_EQ(requests_metric.labels.at("component"), "web_server");
    EXPECT_EQ(requests_metric.labels.at("environment"), "production");
    EXPECT_EQ(requests_metric.labels.at("datacenter"), "dc1");
    EXPECT_EQ(requests_metric.labels.at("instance"), "test_instance");
    
    // Test snapshot conversion
    auto snapshot_metrics = exporter.convert_snapshot(test_snapshot_);
    EXPECT_EQ(snapshot_metrics.size(), 5); // 5 metrics in test snapshot
    
    // Check snapshot metric
    const auto& load_metric = snapshot_metrics[0];
    EXPECT_EQ(load_metric.name, "system_load_1m");
    EXPECT_EQ(load_metric.type, metric_type::gauge);
    EXPECT_EQ(load_metric.value, 2.1);
    EXPECT_EQ(load_metric.labels.at("source"), "system_monitor");
    EXPECT_EQ(load_metric.labels.at("host"), "server01");
}

TEST_F(MetricExportersTest, PrometheusTextFormat) {
    prometheus_metric_data metric;
    metric.name = "http_requests_total";
    metric.type = metric_type::counter;
    metric.value = 1500.0;
    metric.help_text = "Total number of HTTP requests";
    metric.labels["method"] = "GET";
    metric.labels["status"] = "200";
    metric.timestamp = std::chrono::system_clock::from_time_t(1640995200); // Fixed timestamp: 2022-01-01 00:00:00 UTC
    
    std::string prometheus_text = metric.to_prometheus_text();
    
    // Check format components
    EXPECT_NE(prometheus_text.find("# HELP http_requests_total Total number of HTTP requests"), std::string::npos);
    EXPECT_NE(prometheus_text.find("# TYPE http_requests_total counter"), std::string::npos);
    EXPECT_NE(prometheus_text.find("http_requests_total{"), std::string::npos);
    EXPECT_NE(prometheus_text.find("method=\"GET\""), std::string::npos);
    EXPECT_NE(prometheus_text.find("status=\"200\""), std::string::npos);
    EXPECT_NE(prometheus_text.find("} 1500"), std::string::npos);
}

TEST_F(MetricExportersTest, PrometheusExporterBasicFunctionality) {
    metric_export_config config;
    config.endpoint = "http://prometheus:9090";
    config.format = metric_export_format::prometheus_text;
    
    prometheus_exporter exporter(config);
    
    // Export monitoring data
    std::vector<monitoring_data> data_batch = {test_data_};
    auto export_result = exporter.export_metrics(data_batch);
    EXPECT_TRUE(export_result);
    
    // Export snapshot
    auto snapshot_result = exporter.export_snapshot(test_snapshot_);
    EXPECT_TRUE(snapshot_result);
    
    // Get metrics text
    std::string metrics_text = exporter.get_metrics_text();
    EXPECT_FALSE(metrics_text.empty());
    EXPECT_NE(metrics_text.find("http_requests_total"), std::string::npos);
    EXPECT_NE(metrics_text.find("system_load_1m"), std::string::npos);
    
    // Check statistics
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_metrics"], 2); // 1 data + 1 snapshot
    EXPECT_EQ(stats["failed_exports"], 0);
    EXPECT_EQ(stats["scrape_requests"], 1); // Called get_metrics_text once
    
    // Test flush and shutdown
    auto flush_result = exporter.flush();
    EXPECT_TRUE(flush_result);
    
    auto shutdown_result = exporter.shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(MetricExportersTest, StatsDMetricConversion) {
    metric_export_config config;
    config.endpoint = "statsd.example.com";
    config.port = 8125;
    config.format = metric_export_format::statsd_datadog;
    config.instance_id = "test_instance";
    config.labels["datacenter"] = "dc1";
    
    statsd_exporter exporter(config);
    
    // Test monitoring_data conversion
    auto statsd_metrics = exporter.convert_monitoring_data(test_data_);
    EXPECT_EQ(statsd_metrics.size(), 4); // 4 metrics in test data
    
    // Find http_requests_total metric
    auto it = std::find_if(statsd_metrics.begin(), statsd_metrics.end(),
        [](const statsd_metric_data& m) { return m.name == "http_requests_total"; });
    ASSERT_NE(it, statsd_metrics.end());
    
    const auto& requests_metric = *it;
    EXPECT_EQ(requests_metric.name, "http_requests_total");
    EXPECT_EQ(requests_metric.type, metric_type::counter);
    EXPECT_EQ(requests_metric.value, 1500.0);
    EXPECT_EQ(requests_metric.sample_rate, 1.0);
    EXPECT_EQ(requests_metric.tags.at("component"), "web_server");
    EXPECT_EQ(requests_metric.tags.at("environment"), "production");
    EXPECT_EQ(requests_metric.tags.at("datacenter"), "dc1");
    
    // Test snapshot conversion
    auto snapshot_metrics = exporter.convert_snapshot(test_snapshot_);
    EXPECT_EQ(snapshot_metrics.size(), 5); // 5 metrics in test snapshot
}

TEST_F(MetricExportersTest, StatsDTextFormat) {
    statsd_metric_data counter_metric;
    counter_metric.name = "http_requests_total";
    counter_metric.type = metric_type::counter;
    counter_metric.value = 1500.0;
    counter_metric.sample_rate = 1.0;
    counter_metric.tags["method"] = "GET";
    counter_metric.tags["status"] = "200";
    
    // Test plain StatsD format
    std::string plain_statsd = counter_metric.to_statsd_format(false);
    EXPECT_EQ(plain_statsd, "http_requests_total:1500|c");
    
    // Test DataDog format with tags
    std::string datadog_statsd = counter_metric.to_statsd_format(true);
    EXPECT_NE(datadog_statsd.find("http_requests_total:1500|c|#"), std::string::npos);
    EXPECT_NE(datadog_statsd.find("method:GET"), std::string::npos);
    EXPECT_NE(datadog_statsd.find("status:200"), std::string::npos);
    
    // Test timer metric
    statsd_metric_data timer_metric;
    timer_metric.name = "request_duration";
    timer_metric.type = metric_type::timer;
    timer_metric.value = 250.0;
    timer_metric.sample_rate = 0.1;
    
    std::string timer_statsd = timer_metric.to_statsd_format(false);
    EXPECT_EQ(timer_statsd, "request_duration:250|ms|@0.1");
}

TEST_F(MetricExportersTest, StatsDExporterBasicFunctionality) {
    metric_export_config config;
    config.endpoint = "statsd.example.com";
    config.port = 8125;
    config.format = metric_export_format::statsd_plain;
    
    statsd_exporter exporter(config);
    
    // Export monitoring data
    std::vector<monitoring_data> data_batch = {test_data_};
    auto export_result = exporter.export_metrics(data_batch);
    EXPECT_TRUE(export_result);
    
    // Export snapshot
    auto snapshot_result = exporter.export_snapshot(test_snapshot_);
    EXPECT_TRUE(snapshot_result);
    
    // Check statistics
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_metrics"], 2); // 1 data + 1 snapshot
    EXPECT_EQ(stats["failed_exports"], 0);
    EXPECT_EQ(stats["sent_packets"], 2); // 2 UDP packets sent
    
    // Test flush and shutdown
    auto flush_result = exporter.flush();
    EXPECT_TRUE(flush_result);
    
    auto shutdown_result = exporter.shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(MetricExportersTest, OtlpMetricsExporterBasicFunctionality) {
    metric_export_config config;
    config.endpoint = "http://otlp-collector:4317";
    config.format = metric_export_format::otlp_grpc;
    
    otlp_metrics_exporter exporter(config, otel_resource_);
    
    // Export monitoring data
    std::vector<monitoring_data> data_batch = {test_data_};
    auto export_result = exporter.export_metrics(data_batch);
    EXPECT_TRUE(export_result);
    
    // Export snapshot
    auto snapshot_result = exporter.export_snapshot(test_snapshot_);
    EXPECT_TRUE(snapshot_result);
    
    // Check statistics
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_metrics"], 2); // 1 data + 1 snapshot
    EXPECT_EQ(stats["failed_exports"], 0);
    
    // Test flush and shutdown
    auto flush_result = exporter.flush();
    EXPECT_TRUE(flush_result);
    
    auto shutdown_result = exporter.shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(MetricExportersTest, MetricExporterFactory) {
    // Test Prometheus factory
    metric_export_config prometheus_config;
    prometheus_config.endpoint = "http://prometheus:9090";
    prometheus_config.format = metric_export_format::prometheus_text;
    
    auto prometheus_exporter = metric_exporter_factory::create_exporter(prometheus_config, otel_resource_);
    EXPECT_TRUE(prometheus_exporter);
    
    // Test StatsD factory
    metric_export_config statsd_config;
    statsd_config.endpoint = "statsd.example.com";
    statsd_config.port = 8125;
    statsd_config.format = metric_export_format::statsd_datadog;
    
    auto statsd_exporter = metric_exporter_factory::create_exporter(statsd_config, otel_resource_);
    EXPECT_TRUE(statsd_exporter);
    
    // Test OTLP factory
    metric_export_config otlp_config;
    otlp_config.endpoint = "http://otlp-collector:4317";
    otlp_config.format = metric_export_format::otlp_http_json;
    
    auto otlp_exporter = metric_exporter_factory::create_exporter(otlp_config, otel_resource_);
    EXPECT_TRUE(otlp_exporter);
    
    // Test invalid format
    metric_export_config invalid_config;
    invalid_config.endpoint = "http://test";
    invalid_config.format = static_cast<metric_export_format>(999);
    
    auto invalid_exporter = metric_exporter_factory::create_exporter(invalid_config, otel_resource_);
    EXPECT_FALSE(invalid_exporter);
}

TEST_F(MetricExportersTest, SupportedFormatsQuery) {
    auto prometheus_formats = metric_exporter_factory::get_supported_formats("prometheus");
    EXPECT_EQ(prometheus_formats.size(), 2);
    EXPECT_TRUE(std::find(prometheus_formats.begin(), prometheus_formats.end(), 
                         metric_export_format::prometheus_text) != prometheus_formats.end());
    EXPECT_TRUE(std::find(prometheus_formats.begin(), prometheus_formats.end(), 
                         metric_export_format::prometheus_protobuf) != prometheus_formats.end());
    
    auto statsd_formats = metric_exporter_factory::get_supported_formats("statsd");
    EXPECT_EQ(statsd_formats.size(), 2);
    EXPECT_TRUE(std::find(statsd_formats.begin(), statsd_formats.end(), 
                         metric_export_format::statsd_plain) != statsd_formats.end());
    EXPECT_TRUE(std::find(statsd_formats.begin(), statsd_formats.end(), 
                         metric_export_format::statsd_datadog) != statsd_formats.end());
    
    auto otlp_formats = metric_exporter_factory::get_supported_formats("otlp");
    EXPECT_EQ(otlp_formats.size(), 3);
    EXPECT_TRUE(std::find(otlp_formats.begin(), otlp_formats.end(), 
                         metric_export_format::otlp_grpc) != otlp_formats.end());
    
    auto unknown_formats = metric_exporter_factory::get_supported_formats("unknown");
    EXPECT_EQ(unknown_formats.size(), 0);
}

TEST_F(MetricExportersTest, HelperFunctions) {
    // Test Prometheus helper
    auto prometheus_exporter = create_prometheus_exporter(9090, "test_job");
    EXPECT_TRUE(prometheus_exporter);
    
    // Test StatsD helper
    auto statsd_exporter = create_statsd_exporter("localhost", 8125, true);
    EXPECT_TRUE(statsd_exporter);
    
    // Test OTLP helper
    auto otlp_exporter = create_otlp_metrics_exporter("http://otlp:4317", otel_resource_, 
                                                     metric_export_format::otlp_http_json);
    EXPECT_TRUE(otlp_exporter);
}

TEST_F(MetricExportersTest, EmptyMetricsHandling) {
    std::vector<monitoring_data> empty_data;
    metrics_snapshot empty_snapshot;
    
    metric_export_config config;
    config.endpoint = "http://test:1234";
    config.format = metric_export_format::prometheus_text;
    
    prometheus_exporter exporter(config);
    
    auto data_result = exporter.export_metrics(empty_data);
    EXPECT_TRUE(data_result);
    
    auto snapshot_result = exporter.export_snapshot(empty_snapshot);
    EXPECT_TRUE(snapshot_result);
    
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_metrics"], 1); // Empty snapshot counts as 1
    EXPECT_EQ(stats["failed_exports"], 0);
}

TEST_F(MetricExportersTest, LargeMetricBatch) {
    // Create a large batch of monitoring data
    std::vector<monitoring_data> large_batch;
    for (int i = 0; i < 100; ++i) {
        monitoring_data data("service_" + std::to_string(i));
        data.add_metric("requests_total", i * 10.0);
        data.add_metric("response_time", i * 0.1);
        data.add_tag("instance", std::to_string(i));
        large_batch.push_back(data);
    }
    
    metric_export_config config;
    config.endpoint = "http://test:1234";
    config.format = metric_export_format::statsd_plain;
    config.max_batch_size = 50; // Smaller than batch size
    
    statsd_exporter exporter(config);
    auto result = exporter.export_metrics(large_batch);
    EXPECT_TRUE(result);
    
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_metrics"], 100);
}

TEST_F(MetricExportersTest, MetricNameSanitization) {
    metric_export_config config;
    config.endpoint = "http://prometheus:9090";
    config.format = metric_export_format::prometheus_text;
    
    prometheus_exporter exporter(config);
    
    // Create data with problematic metric names
    monitoring_data data("test_component");
    data.add_metric("http.requests-total", 100.0);  // Contains dots and dashes
    data.add_metric("123_invalid_start", 50.0);     // Starts with number
    data.add_metric("special@chars#metric", 75.0);   // Contains special characters
    
    auto prom_metrics = exporter.convert_monitoring_data(data);
    EXPECT_EQ(prom_metrics.size(), 3);
    
    // Check sanitized names exist
    std::vector<std::string> expected_names = {"http_requests_total", "_123_invalid_start", "special_chars_metric"};
    std::vector<std::string> actual_names;
    for (const auto& metric : prom_metrics) {
        actual_names.push_back(metric.name);
    }
    
    for (const auto& expected_name : expected_names) {
        EXPECT_NE(std::find(actual_names.begin(), actual_names.end(), expected_name), actual_names.end()) 
            << "Expected metric name '" << expected_name << "' not found";
    }
}

TEST_F(MetricExportersTest, MetricTypeInference) {
    metric_export_config config;
    config.endpoint = "statsd.example.com";
    config.port = 8125;
    config.format = metric_export_format::statsd_plain;
    
    statsd_exporter exporter(config);
    
    monitoring_data data("test_service");
    data.add_metric("requests_count", 100.0);       // Should be counter
    data.add_metric("requests_total", 200.0);       // Should be counter
    data.add_metric("response_time_ms", 250.0);     // Should be timer
    data.add_metric("request_duration", 0.5);       // Should be timer
    data.add_metric("cpu_usage", 75.5);             // Should be gauge
    data.add_metric("memory_available", 1024.0);    // Should be gauge
    
    auto statsd_metrics = exporter.convert_monitoring_data(data);
    
    // Check inferred types by finding specific metrics
    auto find_metric = [&](const std::string& name) -> const statsd_metric_data* {
        auto it = std::find_if(statsd_metrics.begin(), statsd_metrics.end(),
            [&name](const statsd_metric_data& m) { return m.name == name; });
        return (it != statsd_metrics.end()) ? &(*it) : nullptr;
    };
    
    EXPECT_EQ(find_metric("requests_count")->type, metric_type::counter);
    EXPECT_EQ(find_metric("requests_total")->type, metric_type::counter);
    EXPECT_EQ(find_metric("response_time_ms")->type, metric_type::timer);
    EXPECT_EQ(find_metric("request_duration")->type, metric_type::timer);
    EXPECT_EQ(find_metric("cpu_usage")->type, metric_type::gauge);
    EXPECT_EQ(find_metric("memory_available")->type, metric_type::gauge);
}