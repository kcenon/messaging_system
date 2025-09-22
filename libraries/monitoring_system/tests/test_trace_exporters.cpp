/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include <kcenon/monitoring/exporters/trace_exporters.h>
// Note: distributed_tracer.h does not exist in include directory
// #include <kcenon/monitoring/tracing/distributed_tracer.h>
#include <kcenon/monitoring/exporters/opentelemetry_adapter.h>
#include <thread>
#include <chrono>

using namespace monitoring_system;

class TraceExportersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test spans
        test_spans_ = create_test_spans();
        
        // Create OTEL resource
        otel_resource_ = create_service_resource("test_service", "1.0.0", "test_namespace");
    }
    
    std::vector<trace_span> create_test_spans() {
        std::vector<trace_span> spans;
        
        // Root span
        trace_span root_span;
        root_span.trace_id = "trace123";
        root_span.span_id = "span001";
        root_span.operation_name = "http_request";
        root_span.service_name = "web_service";
        root_span.start_time = std::chrono::system_clock::now();
        root_span.end_time = root_span.start_time + std::chrono::milliseconds(100);
        root_span.tags["http.method"] = "GET";
        root_span.tags["http.url"] = "/api/users";
        root_span.tags["span.kind"] = "server";
        spans.push_back(root_span);
        
        // Child span
        trace_span child_span;
        child_span.trace_id = "trace123";
        child_span.span_id = "span002";
        child_span.parent_span_id = "span001";
        child_span.operation_name = "database_query";
        child_span.service_name = "db_service";
        child_span.start_time = root_span.start_time + std::chrono::milliseconds(10);
        child_span.end_time = root_span.start_time + std::chrono::milliseconds(80);
        child_span.tags["db.statement"] = "SELECT * FROM users WHERE id = ?";
        child_span.tags["db.type"] = "postgresql";
        child_span.tags["span.kind"] = "client";
        spans.push_back(child_span);
        
        return spans;
    }
    
    std::vector<trace_span> test_spans_;
    otel_resource otel_resource_;
};

TEST_F(TraceExportersTest, TraceExportConfigValidation) {
    // Valid configuration
    trace_export_config valid_config;
    valid_config.endpoint = "http://jaeger:14268/api/traces";
    valid_config.format = trace_export_format::jaeger_thrift;
    valid_config.timeout = std::chrono::milliseconds(5000);
    valid_config.max_batch_size = 100;
    valid_config.max_queue_size = 1000;
    
    auto validation = valid_config.validate();
    EXPECT_TRUE(validation);
    
    // Invalid endpoint
    trace_export_config invalid_endpoint;
    invalid_endpoint.endpoint = "";
    auto endpoint_validation = invalid_endpoint.validate();
    EXPECT_FALSE(endpoint_validation);
    EXPECT_EQ(endpoint_validation.get_error().code, monitoring_error_code::invalid_configuration);
    
    // Invalid timeout
    trace_export_config invalid_timeout;
    invalid_timeout.endpoint = "http://test";
    invalid_timeout.timeout = std::chrono::milliseconds(0);
    auto timeout_validation = invalid_timeout.validate();
    EXPECT_FALSE(timeout_validation);
    
    // Invalid batch size
    trace_export_config invalid_batch;
    invalid_batch.endpoint = "http://test";
    invalid_batch.max_batch_size = 0;
    auto batch_validation = invalid_batch.validate();
    EXPECT_FALSE(batch_validation);
    
    // Invalid queue size
    trace_export_config invalid_queue;
    invalid_queue.endpoint = "http://test";
    invalid_queue.max_batch_size = 100;
    invalid_queue.max_queue_size = 50;
    auto queue_validation = invalid_queue.validate();
    EXPECT_FALSE(queue_validation);
}

TEST_F(TraceExportersTest, JaegerSpanConversion) {
    trace_export_config config;
    config.endpoint = "http://jaeger:14268/api/traces";
    config.format = trace_export_format::jaeger_thrift;
    config.service_name = "test_service";
    
    jaeger_exporter exporter(config);
    
    const auto& span = test_spans_[0];
    auto jaeger_span = exporter.convert_span(span);
    
    EXPECT_EQ(jaeger_span.trace_id, span.trace_id);
    EXPECT_EQ(jaeger_span.span_id, span.span_id);
    EXPECT_EQ(jaeger_span.operation_name, span.operation_name);
    EXPECT_EQ(jaeger_span.service_name, "test_service"); // Override from config
    
    // Check tags conversion
    bool found_http_method = false;
    bool found_http_url = false;
    for (const auto& [key, value] : jaeger_span.tags) {
        if (key == "http.method" && value == "GET") {
            found_http_method = true;
        }
        if (key == "http.url" && value == "/api/users") {
            found_http_url = true;
        }
    }
    EXPECT_TRUE(found_http_method);
    EXPECT_TRUE(found_http_url);
    
    // Check process tags
    bool found_service_name = false;
    for (const auto& [key, value] : jaeger_span.process_tags) {
        if (key == "service.name" && value == "test_service") {
            found_service_name = true;
        }
    }
    EXPECT_TRUE(found_service_name);
}

TEST_F(TraceExportersTest, JaegerExporterBasicFunctionality) {
    trace_export_config config;
    config.endpoint = "http://jaeger:14268/api/traces";
    config.format = trace_export_format::jaeger_thrift;
    
    jaeger_exporter exporter(config);
    
    // Export spans
    auto export_result = exporter.export_spans(test_spans_);
    EXPECT_TRUE(export_result);
    
    // Check statistics
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_spans"], test_spans_.size());
    EXPECT_EQ(stats["failed_exports"], 0);
    
    // Test flush and shutdown
    auto flush_result = exporter.flush();
    EXPECT_TRUE(flush_result);
    
    auto shutdown_result = exporter.shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(TraceExportersTest, ZipkinSpanConversion) {
    trace_export_config config;
    config.endpoint = "http://zipkin:9411/api/v2/spans";
    config.format = trace_export_format::zipkin_json;
    config.service_name = "test_service";
    
    zipkin_exporter exporter(config);
    
    const auto& span = test_spans_[0];
    auto zipkin_span = exporter.convert_span(span);
    
    EXPECT_EQ(zipkin_span.trace_id, span.trace_id);
    EXPECT_EQ(zipkin_span.span_id, span.span_id);
    EXPECT_EQ(zipkin_span.name, span.operation_name);
    EXPECT_EQ(zipkin_span.local_endpoint_service_name, "test_service");
    EXPECT_EQ(zipkin_span.kind, "server"); // From span.kind tag
    
    // Check tags conversion (span.kind should be excluded)
    EXPECT_EQ(zipkin_span.tags.count("span.kind"), 0);
    EXPECT_EQ(zipkin_span.tags.count("http.method"), 1);
    EXPECT_EQ(zipkin_span.tags["http.method"], "GET");
}

TEST_F(TraceExportersTest, ZipkinExporterBasicFunctionality) {
    trace_export_config config;
    config.endpoint = "http://zipkin:9411/api/v2/spans";
    config.format = trace_export_format::zipkin_json;
    
    zipkin_exporter exporter(config);
    
    // Export spans
    auto export_result = exporter.export_spans(test_spans_);
    EXPECT_TRUE(export_result);
    
    // Check statistics
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_spans"], test_spans_.size());
    EXPECT_EQ(stats["failed_exports"], 0);
    
    // Test flush and shutdown
    auto flush_result = exporter.flush();
    EXPECT_TRUE(flush_result);
    
    auto shutdown_result = exporter.shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(TraceExportersTest, OtlpExporterBasicFunctionality) {
    trace_export_config config;
    config.endpoint = "http://otlp-collector:4317";
    config.format = trace_export_format::otlp_grpc;
    
    otlp_exporter exporter(config, otel_resource_);
    
    // Export spans
    auto export_result = exporter.export_spans(test_spans_);
    EXPECT_TRUE(export_result);
    
    // Check statistics
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_spans"], test_spans_.size());
    EXPECT_EQ(stats["failed_exports"], 0);
    
    // Test flush and shutdown
    auto flush_result = exporter.flush();
    EXPECT_TRUE(flush_result);
    
    auto shutdown_result = exporter.shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(TraceExportersTest, TraceExporterFactory) {
    // Test Jaeger factory
    trace_export_config jaeger_config;
    jaeger_config.endpoint = "http://jaeger:14268";
    jaeger_config.format = trace_export_format::jaeger_grpc;
    
    auto jaeger_exporter = trace_exporter_factory::create_exporter(jaeger_config, otel_resource_);
    EXPECT_TRUE(jaeger_exporter);
    
    // Test Zipkin factory
    trace_export_config zipkin_config;
    zipkin_config.endpoint = "http://zipkin:9411";
    zipkin_config.format = trace_export_format::zipkin_json;
    
    auto zipkin_exporter = trace_exporter_factory::create_exporter(zipkin_config, otel_resource_);
    EXPECT_TRUE(zipkin_exporter);
    
    // Test OTLP factory
    trace_export_config otlp_config;
    otlp_config.endpoint = "http://otlp-collector:4317";
    otlp_config.format = trace_export_format::otlp_grpc;
    
    auto otlp_exporter = trace_exporter_factory::create_exporter(otlp_config, otel_resource_);
    EXPECT_TRUE(otlp_exporter);
}

TEST_F(TraceExportersTest, SupportedFormatsQuery) {
    auto jaeger_formats = trace_exporter_factory::get_supported_formats("jaeger");
    EXPECT_EQ(jaeger_formats.size(), 2);
    EXPECT_TRUE(std::find(jaeger_formats.begin(), jaeger_formats.end(), 
                         trace_export_format::jaeger_thrift) != jaeger_formats.end());
    EXPECT_TRUE(std::find(jaeger_formats.begin(), jaeger_formats.end(), 
                         trace_export_format::jaeger_grpc) != jaeger_formats.end());
    
    auto zipkin_formats = trace_exporter_factory::get_supported_formats("zipkin");
    EXPECT_EQ(zipkin_formats.size(), 2);
    EXPECT_TRUE(std::find(zipkin_formats.begin(), zipkin_formats.end(), 
                         trace_export_format::zipkin_json) != zipkin_formats.end());
    EXPECT_TRUE(std::find(zipkin_formats.begin(), zipkin_formats.end(), 
                         trace_export_format::zipkin_protobuf) != zipkin_formats.end());
    
    auto otlp_formats = trace_exporter_factory::get_supported_formats("otlp");
    EXPECT_EQ(otlp_formats.size(), 3);
    EXPECT_TRUE(std::find(otlp_formats.begin(), otlp_formats.end(), 
                         trace_export_format::otlp_grpc) != otlp_formats.end());
    
    auto unknown_formats = trace_exporter_factory::get_supported_formats("unknown");
    EXPECT_EQ(unknown_formats.size(), 0);
}

TEST_F(TraceExportersTest, HelperFunctions) {
    // Test Jaeger helper
    auto jaeger_exporter = create_jaeger_exporter("http://jaeger:14268", 
                                                 trace_export_format::jaeger_thrift);
    EXPECT_TRUE(jaeger_exporter);
    
    // Test Zipkin helper
    auto zipkin_exporter = create_zipkin_exporter("http://zipkin:9411", 
                                                 trace_export_format::zipkin_protobuf);
    EXPECT_TRUE(zipkin_exporter);
    
    // Test OTLP helper
    auto otlp_exporter = create_otlp_exporter("http://otlp:4317", otel_resource_, 
                                             trace_export_format::otlp_http_json);
    EXPECT_TRUE(otlp_exporter);
}

TEST_F(TraceExportersTest, InvalidFormatHandling) {
    trace_export_config invalid_jaeger_config;
    invalid_jaeger_config.endpoint = "http://jaeger:14268";
    invalid_jaeger_config.format = trace_export_format::zipkin_json; // Wrong format
    
    jaeger_exporter jaeger_exporter(invalid_jaeger_config);
    auto jaeger_result = jaeger_exporter.export_spans(test_spans_);
    EXPECT_FALSE(jaeger_result);
    EXPECT_EQ(jaeger_result.get_error().code, monitoring_error_code::invalid_configuration);
    
    trace_export_config invalid_zipkin_config;
    invalid_zipkin_config.endpoint = "http://zipkin:9411";
    invalid_zipkin_config.format = trace_export_format::jaeger_grpc; // Wrong format
    
    zipkin_exporter zipkin_exporter(invalid_zipkin_config);
    auto zipkin_result = zipkin_exporter.export_spans(test_spans_);
    EXPECT_FALSE(zipkin_result);
    EXPECT_EQ(zipkin_result.get_error().code, monitoring_error_code::invalid_configuration);
    
    trace_export_config invalid_otlp_config;
    invalid_otlp_config.endpoint = "http://otlp:4317";
    invalid_otlp_config.format = trace_export_format::jaeger_thrift; // Wrong format
    
    otlp_exporter otlp_exporter(invalid_otlp_config, otel_resource_);
    auto otlp_result = otlp_exporter.export_spans(test_spans_);
    EXPECT_FALSE(otlp_result);
    EXPECT_EQ(otlp_result.get_error().code, monitoring_error_code::invalid_configuration);
}

TEST_F(TraceExportersTest, EmptySpansHandling) {
    std::vector<trace_span> empty_spans;
    
    trace_export_config config;
    config.endpoint = "http://test:1234";
    config.format = trace_export_format::jaeger_grpc;
    
    jaeger_exporter exporter(config);
    auto result = exporter.export_spans(empty_spans);
    EXPECT_TRUE(result);
    
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_spans"], 0);
    EXPECT_EQ(stats["failed_exports"], 0);
}

TEST_F(TraceExportersTest, LargeSpanBatch) {
    // Create a large batch of spans
    std::vector<trace_span> large_batch;
    for (int i = 0; i < 1000; ++i) {
        trace_span span;
        span.trace_id = "trace" + std::to_string(i);
        span.span_id = "span" + std::to_string(i);
        span.operation_name = "operation_" + std::to_string(i);
        span.service_name = "test_service";
        span.start_time = std::chrono::system_clock::now();
        span.end_time = span.start_time + std::chrono::milliseconds(1);
        large_batch.push_back(span);
    }
    
    trace_export_config config;
    config.endpoint = "http://test:1234";
    config.format = trace_export_format::otlp_grpc;
    config.max_batch_size = 500; // Smaller than batch size
    
    otlp_exporter exporter(config, otel_resource_);
    auto result = exporter.export_spans(large_batch);
    EXPECT_TRUE(result);
    
    auto stats = exporter.get_stats();
    EXPECT_EQ(stats["exported_spans"], 1000);
}