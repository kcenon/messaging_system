#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file opentelemetry_adapter.h
 * @brief OpenTelemetry compatibility layer for monitoring system integration
 * 
 * This file provides OpenTelemetry-compatible interfaces and adapters to enable
 * seamless integration with OpenTelemetry-based observability tools and platforms.
 * Supports trace, metrics, and logs exporters following OTel specifications.
 */

#include "../core/result_types.h"
#include "../core/error_codes.h"
#include "../tracing/distributed_tracer.h"
#include "../interfaces/monitoring_interface.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <memory>
#include <functional>
#include <mutex>

namespace monitoring_system {

/**
 * @enum otel_resource_type
 * @brief OpenTelemetry resource types
 */
enum class otel_resource_type {
    service,
    service_instance,
    host,
    container,
    process,
    runtime,
    telemetry_sdk,
    custom
};

/**
 * @enum otel_span_kind
 * @brief OpenTelemetry span kinds
 */
enum class otel_span_kind {
    unspecified = 0,
    internal = 1,
    server = 2,
    client = 3,
    producer = 4,
    consumer = 5
};

/**
 * @enum otel_status_code
 * @brief OpenTelemetry status codes
 */
enum class otel_status_code {
    unset = 0,
    ok = 1,
    error = 2
};

/**
 * @struct otel_attribute
 * @brief OpenTelemetry attribute representation
 */
struct otel_attribute {
    std::string key;
    std::string value;
    
    otel_attribute() = default;
    otel_attribute(const std::string& k, const std::string& v) : key(k), value(v) {}
    
    bool operator==(const otel_attribute& other) const {
        return key == other.key && value == other.value;
    }
};

/**
 * @struct otel_resource
 * @brief OpenTelemetry resource representation
 */
struct otel_resource {
    otel_resource_type type{otel_resource_type::service};
    std::vector<otel_attribute> attributes;
    
    void add_attribute(const std::string& key, const std::string& value) {
        attributes.emplace_back(key, value);
    }
    
    result<std::string> get_attribute(const std::string& key) const {
        for (const auto& attr : attributes) {
            if (attr.key == key) {
                return make_success(attr.value);
            }
        }
        return make_error<std::string>(monitoring_error_code::not_found,
                                     "Attribute not found: " + key);
    }
};

/**
 * @struct otel_span_context
 * @brief OpenTelemetry span context
 */
struct otel_span_context {
    std::string trace_id;
    std::string span_id;
    std::string trace_flags;
    std::string trace_state;
    bool is_valid{false};
    bool is_remote{false};
    
    otel_span_context() = default;
    otel_span_context(const std::string& tid, const std::string& sid)
        : trace_id(tid), span_id(sid), is_valid(true) {}
};

/**
 * @struct otel_span_data
 * @brief OpenTelemetry span data representation
 */
struct otel_span_data {
    otel_span_context context;
    otel_span_context parent_context;
    std::string name;
    otel_span_kind kind{otel_span_kind::internal};
    otel_status_code status_code{otel_status_code::unset};
    std::string status_message;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::vector<otel_attribute> attributes;
    std::vector<std::string> events;
    otel_resource resource;
    
    void add_attribute(const std::string& key, const std::string& value) {
        attributes.emplace_back(key, value);
    }
    
    void add_event(const std::string& event) {
        events.push_back(event);
    }
    
    bool is_ended() const {
        return end_time != std::chrono::system_clock::time_point{};
    }
    
    std::chrono::microseconds duration() const {
        if (!is_ended()) {
            return std::chrono::microseconds{0};
        }
        return std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    }
};

/**
 * @struct otel_metric_data
 * @brief OpenTelemetry metric data representation
 */
struct otel_metric_data {
    std::string name;
    std::string description;
    std::string unit;
    double value{0.0};
    std::vector<otel_attribute> attributes;
    std::chrono::system_clock::time_point timestamp;
    otel_resource resource;
    
    void add_attribute(const std::string& key, const std::string& attribute_value) {
        attributes.emplace_back(key, attribute_value);
    }
};

/**
 * @class opentelemetry_tracer_adapter
 * @brief Adapter for converting monitoring system traces to OpenTelemetry format
 */
class opentelemetry_tracer_adapter {
public:
    explicit opentelemetry_tracer_adapter(const otel_resource& resource)
        : resource_(resource) {}
    
    /**
     * @brief Convert internal span to OpenTelemetry span data
     */
    result<otel_span_data> convert_span(const trace_span& span) {
        otel_span_data otel_span;
        
        // Convert basic span information
        otel_span.name = span.operation_name;
        otel_span.context = otel_span_context(span.trace_id, span.span_id);
        
        if (!span.parent_span_id.empty()) {
            otel_span.parent_context = otel_span_context(span.trace_id, span.parent_span_id);
        }
        
        otel_span.start_time = span.start_time;
        otel_span.end_time = span.end_time;
        otel_span.resource = resource_;
        
        // First determine span kind from tags
        auto kind_it = span.tags.find("span.kind");
        if (kind_it != span.tags.end()) {
            otel_span.kind = parse_span_kind(kind_it->second);
        }
        
        // Set status from tags
        auto error_it = span.tags.find("error");
        if (error_it != span.tags.end() && error_it->second == "true") {
            otel_span.status_code = otel_status_code::error;
            auto error_msg_it = span.tags.find("error.message");
            if (error_msg_it != span.tags.end()) {
                otel_span.status_message = error_msg_it->second;
            }
        } else {
            otel_span.status_code = otel_status_code::ok;
        }
        
        // Convert tags to attributes, excluding special OpenTelemetry fields
        for (const auto& [key, value] : span.tags) {
            // Skip special fields that are handled above
            if (key != "span.kind" && key != "error" && key != "error.message") {
                otel_span.add_attribute(key, value);
            }
        }
        
        return make_success(std::move(otel_span));
    }
    
    /**
     * @brief Convert multiple spans to OpenTelemetry format
     */
    result<std::vector<otel_span_data>> convert_spans(const std::vector<trace_span>& spans) {
        std::vector<otel_span_data> otel_spans;
        otel_spans.reserve(spans.size());
        
        for (const auto& span : spans) {
            auto convert_result = convert_span(span);
            if (!convert_result) {
                return make_error<std::vector<otel_span_data>>(
                    convert_result.get_error().code,
                    "Failed to convert span: " + convert_result.get_error().message);
            }
            otel_spans.push_back(convert_result.value());
        }
        
        return make_success(std::move(otel_spans));
    }
    
    /**
     * @brief Create OpenTelemetry context from internal trace context
     */
    result<otel_span_context> create_context(const trace_context& context) {
        return make_success(otel_span_context(context.trace_id, context.span_id));
    }
    
private:
    otel_span_kind parse_span_kind(const std::string& kind_str) {
        if (kind_str == "server") return otel_span_kind::server;
        if (kind_str == "client") return otel_span_kind::client;
        if (kind_str == "producer") return otel_span_kind::producer;
        if (kind_str == "consumer") return otel_span_kind::consumer;
        if (kind_str == "internal") return otel_span_kind::internal;
        return otel_span_kind::unspecified;
    }
    
    otel_resource resource_;
};

/**
 * @class opentelemetry_metrics_adapter
 * @brief Adapter for converting monitoring system metrics to OpenTelemetry format
 */
class opentelemetry_metrics_adapter {
public:
    explicit opentelemetry_metrics_adapter(const otel_resource& resource)
        : resource_(resource) {}
    
    /**
     * @brief Convert metrics snapshot to OpenTelemetry metric data
     */
    result<std::vector<otel_metric_data>> convert_metrics(const metrics_snapshot& snapshot) {
        std::vector<otel_metric_data> otel_metrics;
        
        for (const auto& metric_value : snapshot.metrics) {
            otel_metric_data metric;
            metric.name = metric_value.name;
            metric.value = metric_value.value;
            metric.timestamp = metric_value.timestamp;
            metric.resource = resource_;
            
            // Add tags as attributes
            for (const auto& [tag_name, tag_value] : metric_value.tags) {
                metric.add_attribute(tag_name, tag_value);
            }
            
            // Add common resource attributes
            metric.add_attribute("service.name", resource_.get_attribute("service.name").value_or("unknown"));
            metric.add_attribute("service.version", resource_.get_attribute("service.version").value_or("unknown"));
            
            otel_metrics.push_back(std::move(metric));
        }
        
        return make_success(std::move(otel_metrics));
    }
    
    /**
     * @brief Convert monitoring data to OpenTelemetry metric data
     */
    result<std::vector<otel_metric_data>> convert_monitoring_data(const monitoring_data& data) {
        std::vector<otel_metric_data> otel_metrics;
        
        for (const auto& [name, value] : data.get_metrics()) {
            otel_metric_data metric;
            metric.name = name;
            metric.value = value;
            metric.timestamp = data.get_timestamp();
            metric.resource = resource_;
            
            // Add tags as attributes
            for (const auto& [tag_name, tag_value] : data.get_tags()) {
                metric.add_attribute(tag_name, tag_value);
            }
            
            otel_metrics.push_back(std::move(metric));
        }
        
        return make_success(std::move(otel_metrics));
    }
    
private:
    otel_resource resource_;
};

/**
 * @class opentelemetry_exporter_config
 * @brief Configuration for OpenTelemetry exporters
 */
struct opentelemetry_exporter_config {
    std::string endpoint{"http://localhost:4317"};
    std::string protocol{"grpc"};  // grpc, http/protobuf, http/json
    std::chrono::milliseconds timeout{30000};
    std::chrono::milliseconds export_interval{5000};
    std::size_t max_batch_size{512};
    std::unordered_map<std::string, std::string> headers;
    bool compression_enabled{true};
    std::string compression_type{"gzip"};
    
    result_void validate() const {
        if (endpoint.empty()) {
            return result_void::error(monitoring_error_code::invalid_configuration,
                                    "Exporter endpoint cannot be empty");
        }
        if (protocol != "grpc" && protocol != "http/protobuf" && protocol != "http/json") {
            return result_void::error(monitoring_error_code::invalid_configuration,
                                    "Invalid protocol: " + protocol);
        }
        if (timeout <= std::chrono::milliseconds(0)) {
            return result_void::error(monitoring_error_code::invalid_configuration,
                                    "Timeout must be positive");
        }
        if (max_batch_size == 0) {
            return result_void::error(monitoring_error_code::invalid_configuration,
                                    "Batch size must be positive");
        }
        return result_void{};
    }
};

/**
 * @class opentelemetry_compatibility_layer
 * @brief Main OpenTelemetry compatibility layer
 */
class opentelemetry_compatibility_layer {
public:
    explicit opentelemetry_compatibility_layer(const otel_resource& resource)
        : resource_(resource)
        , tracer_adapter_(resource)
        , metrics_adapter_(resource) {}
    
    /**
     * @brief Initialize the compatibility layer
     */
    result_void initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (initialized_) {
            return result_void::error(monitoring_error_code::already_exists,
                                    "Compatibility layer already initialized");
        }
        
        initialized_ = true;
        return result_void{};
    }
    
    /**
     * @brief Shutdown the compatibility layer
     */
    result_void shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return result_void{};
        }
        
        // Flush any pending data without re-locking
        // In a real implementation, this would send data to OpenTelemetry collectors
        pending_spans_.clear();
        pending_metrics_.clear();
        
        initialized_ = false;
        return result_void{};
    }
    
    /**
     * @brief Export spans using OpenTelemetry format
     */
    result_void export_spans(const std::vector<trace_span>& spans) {
        if (!initialized_) {
            return result_void::error(monitoring_error_code::invalid_state,
                                    "Compatibility layer not initialized");
        }
        
        auto convert_result = tracer_adapter_.convert_spans(spans);
        if (!convert_result) {
            return result_void::error(convert_result.get_error().code,
                                    "Failed to convert spans: " + convert_result.get_error().message);
        }
        
        // Store converted spans for batching
        std::lock_guard<std::mutex> lock(mutex_);
        const auto& otel_spans = convert_result.value();
        pending_spans_.insert(pending_spans_.end(), otel_spans.begin(), otel_spans.end());
        
        return result_void{};
    }
    
    /**
     * @brief Export metrics using OpenTelemetry format
     */
    result_void export_metrics(const monitoring_data& data) {
        if (!initialized_) {
            return result_void::error(monitoring_error_code::invalid_state,
                                    "Compatibility layer not initialized");
        }
        
        auto convert_result = metrics_adapter_.convert_monitoring_data(data);
        if (!convert_result) {
            return result_void::error(convert_result.get_error().code,
                                    "Failed to convert metrics: " + convert_result.get_error().message);
        }
        
        // Store converted metrics for batching
        std::lock_guard<std::mutex> lock(mutex_);
        const auto& otel_metrics = convert_result.value();
        pending_metrics_.insert(pending_metrics_.end(), otel_metrics.begin(), otel_metrics.end());
        
        return result_void{};
    }
    
    /**
     * @brief Flush pending data to exporters
     */
    result_void flush() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // In a real implementation, this would send data to OpenTelemetry collectors
        // For now, we'll just clear the pending data
        pending_spans_.clear();
        pending_metrics_.clear();
        
        return result_void{};
    }
    
    /**
     * @brief Get compatibility layer statistics
     */
    struct compatibility_stats {
        std::size_t spans_exported{0};
        std::size_t metrics_exported{0};
        std::size_t pending_spans{0};
        std::size_t pending_metrics{0};
        std::chrono::system_clock::time_point last_export;
        std::size_t export_errors{0};
    };
    
    compatibility_stats get_stats() const {
        std::lock_guard<std::mutex> lock(mutex_);
        compatibility_stats stats;
        stats.pending_spans = pending_spans_.size();
        stats.pending_metrics = pending_metrics_.size();
        stats.spans_exported = spans_exported_;
        stats.metrics_exported = metrics_exported_;
        stats.last_export = last_export_;
        stats.export_errors = export_errors_;
        return stats;
    }
    
    /**
     * @brief Get resource information
     */
    const otel_resource& get_resource() const {
        return resource_;
    }
    
private:
    otel_resource resource_;
    opentelemetry_tracer_adapter tracer_adapter_;
    opentelemetry_metrics_adapter metrics_adapter_;
    
    mutable std::mutex mutex_;
    bool initialized_{false};
    
    std::vector<otel_span_data> pending_spans_;
    std::vector<otel_metric_data> pending_metrics_;
    
    std::size_t spans_exported_{0};
    std::size_t metrics_exported_{0};
    std::chrono::system_clock::time_point last_export_;
    std::size_t export_errors_{0};
};

// Factory functions

/**
 * @brief Create OpenTelemetry resource with service information
 */
inline otel_resource create_service_resource(const std::string& service_name,
                                            const std::string& service_version = "1.0.0",
                                            const std::string& service_namespace = "") {
    otel_resource resource;
    resource.type = otel_resource_type::service;
    resource.add_attribute("service.name", service_name);
    resource.add_attribute("service.version", service_version);
    if (!service_namespace.empty()) {
        resource.add_attribute("service.namespace", service_namespace);
    }
    resource.add_attribute("telemetry.sdk.name", "monitoring_system");
    resource.add_attribute("telemetry.sdk.version", "0.5.0");
    resource.add_attribute("telemetry.sdk.language", "cpp");
    
    return resource;
}

/**
 * @brief Create OpenTelemetry compatibility layer
 */
inline std::unique_ptr<opentelemetry_compatibility_layer> 
create_opentelemetry_compatibility_layer(const otel_resource& resource) {
    return std::make_unique<opentelemetry_compatibility_layer>(resource);
}

/**
 * @brief Create OpenTelemetry compatibility layer with service resource
 */
inline std::unique_ptr<opentelemetry_compatibility_layer>
create_opentelemetry_compatibility_layer(const std::string& service_name,
                                        const std::string& service_version = "1.0.0") {
    auto resource = create_service_resource(service_name, service_version);
    return create_opentelemetry_compatibility_layer(resource);
}

} // namespace monitoring_system