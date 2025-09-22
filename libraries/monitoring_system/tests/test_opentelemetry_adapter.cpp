/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

#include <gtest/gtest.h>
#include <kcenon/monitoring/exporters/opentelemetry_adapter.h>
// Note: distributed_tracer.h does not exist in include directory
// #include <kcenon/monitoring/tracing/distributed_tracer.h>
#include <kcenon/monitoring/interfaces/monitoring_interface.h>
#include <thread>
#include <chrono>

using namespace monitoring_system;

class OpenTelemetryAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        resource = create_service_resource("test_service", "1.0.0", "test_namespace");
        compatibility_layer = create_opentelemetry_compatibility_layer(resource);
    }
    
    void TearDown() override {
        if (compatibility_layer) {
            compatibility_layer->shutdown();
        }
    }
    
    otel_resource resource;
    std::unique_ptr<opentelemetry_compatibility_layer> compatibility_layer;
};

TEST_F(OpenTelemetryAdapterTest, ResourceCreation) {
    EXPECT_EQ(resource.type, otel_resource_type::service);
    
    auto service_name = resource.get_attribute("service.name");
    ASSERT_TRUE(service_name);
    EXPECT_EQ(service_name.value(), "test_service");
    
    auto service_version = resource.get_attribute("service.version");
    ASSERT_TRUE(service_version);
    EXPECT_EQ(service_version.value(), "1.0.0");
    
    auto service_namespace = resource.get_attribute("service.namespace");
    ASSERT_TRUE(service_namespace);
    EXPECT_EQ(service_namespace.value(), "test_namespace");
    
    auto sdk_name = resource.get_attribute("telemetry.sdk.name");
    ASSERT_TRUE(sdk_name);
    EXPECT_EQ(sdk_name.value(), "monitoring_system");
}

TEST_F(OpenTelemetryAdapterTest, AttributeOperations) {
    otel_attribute attr("test.key", "test.value");
    EXPECT_EQ(attr.key, "test.key");
    EXPECT_EQ(attr.value, "test.value");
    
    otel_attribute attr2("test.key", "test.value");
    EXPECT_EQ(attr, attr2);
    
    otel_attribute attr3("different.key", "test.value");
    EXPECT_NE(attr, attr3);
}

TEST_F(OpenTelemetryAdapterTest, SpanContextCreation) {
    otel_span_context context("trace123", "span456");
    EXPECT_EQ(context.trace_id, "trace123");
    EXPECT_EQ(context.span_id, "span456");
    EXPECT_TRUE(context.is_valid);
    EXPECT_FALSE(context.is_remote);
    
    otel_span_context invalid_context;
    EXPECT_FALSE(invalid_context.is_valid);
}

TEST_F(OpenTelemetryAdapterTest, SpanDataOperations) {
    otel_span_data span;
    span.name = "test_operation";
    span.kind = otel_span_kind::server;
    span.status_code = otel_status_code::ok;
    span.start_time = std::chrono::system_clock::now();
    
    EXPECT_FALSE(span.is_ended());
    EXPECT_EQ(span.duration().count(), 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    span.end_time = std::chrono::system_clock::now();
    
    EXPECT_TRUE(span.is_ended());
    EXPECT_GT(span.duration().count(), 0);
    
    span.add_attribute("http.method", "GET");
    span.add_event("request_started");
    
    EXPECT_EQ(span.attributes.size(), 1);
    EXPECT_EQ(span.events.size(), 1);
    EXPECT_EQ(span.attributes[0].key, "http.method");
    EXPECT_EQ(span.attributes[0].value, "GET");
}

TEST_F(OpenTelemetryAdapterTest, MetricDataOperations) {
    otel_metric_data metric;
    metric.name = "cpu_usage";
    metric.description = "CPU usage percentage";
    metric.unit = "percent";
    metric.value = 75.5;
    metric.timestamp = std::chrono::system_clock::now();
    
    metric.add_attribute("host.name", "server01");
    metric.add_attribute("service.name", "web_server");
    
    EXPECT_EQ(metric.name, "cpu_usage");
    EXPECT_EQ(metric.value, 75.5);
    EXPECT_EQ(metric.attributes.size(), 2);
    EXPECT_EQ(metric.attributes[0].key, "host.name");
    EXPECT_EQ(metric.attributes[0].value, "server01");
}

TEST_F(OpenTelemetryAdapterTest, TracerAdapterSpanConversion) {
    opentelemetry_tracer_adapter adapter(resource);
    
    trace_span internal_span;
    internal_span.operation_name = "database_query";
    internal_span.trace_id = "trace123";
    internal_span.span_id = "span456";
    internal_span.parent_span_id = "parent789";
    internal_span.start_time = std::chrono::system_clock::now();
    internal_span.end_time = internal_span.start_time + std::chrono::milliseconds(100);
    internal_span.tags["span.kind"] = "client";
    internal_span.tags["db.statement"] = "SELECT * FROM users";
    internal_span.tags["error"] = "false";
    
    auto result = adapter.convert_span(internal_span);
    ASSERT_TRUE(result);
    
    const auto& otel_span = result.value();
    EXPECT_EQ(otel_span.name, "database_query");
    EXPECT_EQ(otel_span.context.trace_id, "trace123");
    EXPECT_EQ(otel_span.context.span_id, "span456");
    EXPECT_EQ(otel_span.parent_context.span_id, "parent789");
    EXPECT_EQ(otel_span.kind, otel_span_kind::client);
    EXPECT_EQ(otel_span.status_code, otel_status_code::ok);
    EXPECT_EQ(otel_span.attributes.size(), 1);
    
    // Find db.statement attribute
    bool found_db_statement = false;
    for (const auto& attr : otel_span.attributes) {
        if (attr.key == "db.statement" && attr.value == "SELECT * FROM users") {
            found_db_statement = true;
            break;
        }
    }
    EXPECT_TRUE(found_db_statement);
}

TEST_F(OpenTelemetryAdapterTest, TracerAdapterErrorSpanConversion) {
    opentelemetry_tracer_adapter adapter(resource);
    
    trace_span error_span;
    error_span.operation_name = "failed_operation";
    error_span.trace_id = "trace123";
    error_span.span_id = "span456";
    error_span.start_time = std::chrono::system_clock::now();
    error_span.end_time = error_span.start_time + std::chrono::milliseconds(50);
    error_span.tags["error"] = "true";
    error_span.tags["error.message"] = "Connection timeout";
    
    auto result = adapter.convert_span(error_span);
    ASSERT_TRUE(result);
    
    const auto& otel_span = result.value();
    EXPECT_EQ(otel_span.status_code, otel_status_code::error);
    EXPECT_EQ(otel_span.status_message, "Connection timeout");
}

TEST_F(OpenTelemetryAdapterTest, TracerAdapterMultipleSpans) {
    opentelemetry_tracer_adapter adapter(resource);
    
    std::vector<trace_span> spans;
    for (int i = 0; i < 3; ++i) {
        trace_span span;
        span.operation_name = "operation_" + std::to_string(i);
        span.trace_id = "trace123";
        span.span_id = "span" + std::to_string(i);
        span.start_time = std::chrono::system_clock::now();
        span.end_time = span.start_time + std::chrono::milliseconds(10);
        spans.push_back(span);
    }
    
    auto result = adapter.convert_spans(spans);
    ASSERT_TRUE(result);
    
    const auto& otel_spans = result.value();
    EXPECT_EQ(otel_spans.size(), 3);
    
    for (size_t i = 0; i < otel_spans.size(); ++i) {
        EXPECT_EQ(otel_spans[i].name, "operation_" + std::to_string(i));
        EXPECT_EQ(otel_spans[i].context.span_id, "span" + std::to_string(i));
    }
}

TEST_F(OpenTelemetryAdapterTest, MetricsAdapterConversion) {
    opentelemetry_metrics_adapter adapter(resource);
    
    monitoring_data data("test_component");
    data.add_metric("cpu_usage", 75.5);
    data.add_metric("memory_usage", 1024.0);
    data.add_tag("environment", "production");
    data.add_tag("region", "us-west-2");
    
    auto result = adapter.convert_monitoring_data(data);
    ASSERT_TRUE(result);
    
    const auto& otel_metrics = result.value();
    EXPECT_EQ(otel_metrics.size(), 2);
    
    // Find CPU usage metric
    bool found_cpu = false;
    for (const auto& metric : otel_metrics) {
        if (metric.name == "cpu_usage") {
            EXPECT_EQ(metric.value, 75.5);
            found_cpu = true;
            
            // Check attributes include tags
            bool found_env = false;
            for (const auto& attr : metric.attributes) {
                if (attr.key == "environment" && attr.value == "production") {
                    found_env = true;
                    break;
                }
            }
            EXPECT_TRUE(found_env);
        }
    }
    EXPECT_TRUE(found_cpu);
}

TEST_F(OpenTelemetryAdapterTest, CompatibilityLayerInitialization) {
    EXPECT_TRUE(compatibility_layer);
    
    auto init_result = compatibility_layer->initialize();
    EXPECT_TRUE(init_result);
    
    // Double initialization should fail
    auto double_init = compatibility_layer->initialize();
    EXPECT_FALSE(double_init);
    EXPECT_EQ(double_init.get_error().code, monitoring_error_code::already_exists);
    
    auto shutdown_result = compatibility_layer->shutdown();
    EXPECT_TRUE(shutdown_result);
}

TEST_F(OpenTelemetryAdapterTest, CompatibilityLayerSpanExport) {
    auto init_result = compatibility_layer->initialize();
    ASSERT_TRUE(init_result);
    
    std::vector<trace_span> spans;
    trace_span span;
    span.operation_name = "test_operation";
    span.trace_id = "trace123";
    span.span_id = "span456";
    span.start_time = std::chrono::system_clock::now();
    span.end_time = span.start_time + std::chrono::milliseconds(10);
    spans.push_back(span);
    
    auto export_result = compatibility_layer->export_spans(spans);
    EXPECT_TRUE(export_result);
    
    auto stats = compatibility_layer->get_stats();
    EXPECT_EQ(stats.pending_spans, 1);
    EXPECT_EQ(stats.pending_metrics, 0);
    
    auto flush_result = compatibility_layer->flush();
    EXPECT_TRUE(flush_result);
    
    stats = compatibility_layer->get_stats();
    EXPECT_EQ(stats.pending_spans, 0);
}

TEST_F(OpenTelemetryAdapterTest, CompatibilityLayerMetricExport) {
    auto init_result = compatibility_layer->initialize();
    ASSERT_TRUE(init_result);
    
    monitoring_data data("test_component");
    data.add_metric("test_metric", 42.0);
    data.add_tag("test_tag", "test_value");
    
    auto export_result = compatibility_layer->export_metrics(data);
    EXPECT_TRUE(export_result);
    
    auto stats = compatibility_layer->get_stats();
    EXPECT_EQ(stats.pending_metrics, 1);
    EXPECT_EQ(stats.pending_spans, 0);
    
    auto flush_result = compatibility_layer->flush();
    EXPECT_TRUE(flush_result);
    
    stats = compatibility_layer->get_stats();
    EXPECT_EQ(stats.pending_metrics, 0);
}

TEST_F(OpenTelemetryAdapterTest, CompatibilityLayerUninitializedExport) {
    std::vector<trace_span> spans;
    trace_span span;
    spans.push_back(span);
    
    auto export_result = compatibility_layer->export_spans(spans);
    EXPECT_FALSE(export_result);
    EXPECT_EQ(export_result.get_error().code, monitoring_error_code::invalid_state);
    
    monitoring_data data("test");
    auto metrics_export_result = compatibility_layer->export_metrics(data);
    EXPECT_FALSE(metrics_export_result);
    EXPECT_EQ(metrics_export_result.get_error().code, monitoring_error_code::invalid_state);
}

TEST_F(OpenTelemetryAdapterTest, CompatibilityLayerResourceAccess) {
    const auto& layer_resource = compatibility_layer->get_resource();
    EXPECT_EQ(layer_resource.type, otel_resource_type::service);
    
    auto service_name = layer_resource.get_attribute("service.name");
    EXPECT_TRUE(service_name);
    EXPECT_EQ(service_name.value(), "test_service");
}

TEST_F(OpenTelemetryAdapterTest, ExporterConfigValidation) {
    opentelemetry_exporter_config valid_config;
    valid_config.endpoint = "http://localhost:4317";
    valid_config.protocol = "grpc";
    valid_config.timeout = std::chrono::milliseconds(5000);
    valid_config.max_batch_size = 100;
    
    auto validation = valid_config.validate();
    EXPECT_TRUE(validation);
    
    // Test invalid endpoint
    opentelemetry_exporter_config invalid_endpoint;
    invalid_endpoint.endpoint = "";
    auto endpoint_validation = invalid_endpoint.validate();
    EXPECT_FALSE(endpoint_validation);
    EXPECT_EQ(endpoint_validation.get_error().code, monitoring_error_code::invalid_configuration);
    
    // Test invalid protocol
    opentelemetry_exporter_config invalid_protocol;
    invalid_protocol.protocol = "invalid";
    auto protocol_validation = invalid_protocol.validate();
    EXPECT_FALSE(protocol_validation);
    
    // Test invalid timeout
    opentelemetry_exporter_config invalid_timeout;
    invalid_timeout.timeout = std::chrono::milliseconds(0);
    auto timeout_validation = invalid_timeout.validate();
    EXPECT_FALSE(timeout_validation);
    
    // Test invalid batch size
    opentelemetry_exporter_config invalid_batch;
    invalid_batch.max_batch_size = 0;
    auto batch_validation = invalid_batch.validate();
    EXPECT_FALSE(batch_validation);
}

TEST_F(OpenTelemetryAdapterTest, FactoryFunctions) {
    // Test service resource creation
    auto service_resource = create_service_resource("my_service", "2.0.0", "production");
    EXPECT_EQ(service_resource.type, otel_resource_type::service);
    
    auto name = service_resource.get_attribute("service.name");
    EXPECT_TRUE(name);
    EXPECT_EQ(name.value(), "my_service");
    
    auto version = service_resource.get_attribute("service.version");
    EXPECT_TRUE(version);
    EXPECT_EQ(version.value(), "2.0.0");
    
    // Test compatibility layer factory functions
    auto layer1 = create_opentelemetry_compatibility_layer(service_resource);
    EXPECT_TRUE(layer1);
    
    auto layer2 = create_opentelemetry_compatibility_layer("test_service", "1.0.0");
    EXPECT_TRUE(layer2);
    
    const auto& layer2_resource = layer2->get_resource();
    auto layer2_name = layer2_resource.get_attribute("service.name");
    EXPECT_TRUE(layer2_name);
    EXPECT_EQ(layer2_name.value(), "test_service");
}