#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file trace_exporters.h
 * @brief Trace data exporters for various distributed tracing systems
 * 
 * This file provides exporters for popular distributed tracing systems:
 * - Jaeger (Uber's distributed tracing system)
 * - Zipkin (Twitter's distributed tracing system)
 * - OTLP (OpenTelemetry Protocol)
 * 
 * Each exporter handles format conversion and network transmission
 * to the respective tracing backend.
 */

#include "../core/result_types.h"
#include "../core/error_codes.h"
#include "../tracing/distributed_tracer.h"
#include "opentelemetry_adapter.h"
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <optional>
#include <functional>
#include <atomic>
#include <algorithm>
#include <unordered_map>

namespace monitoring_system {

/**
 * @enum trace_export_format
 * @brief Supported trace export formats
 */
enum class trace_export_format {
    jaeger_thrift,      ///< Jaeger Thrift protocol
    jaeger_grpc,        ///< Jaeger gRPC protocol
    zipkin_json,        ///< Zipkin JSON v2 format
    zipkin_protobuf,    ///< Zipkin Protocol Buffers format
    otlp_grpc,          ///< OTLP gRPC protocol
    otlp_http_json,     ///< OTLP HTTP JSON protocol
    otlp_http_protobuf  ///< OTLP HTTP Protocol Buffers
};

/**
 * @struct trace_export_config
 * @brief Configuration for trace exporters
 */
struct trace_export_config {
    std::string endpoint;                                    ///< Endpoint URL
    trace_export_format format = trace_export_format::otlp_grpc;
    std::chrono::milliseconds timeout{30000};               ///< Request timeout
    std::chrono::milliseconds batch_timeout{5000};          ///< Batch export timeout
    std::size_t max_batch_size = 512;                      ///< Maximum spans per batch
    std::size_t max_queue_size = 2048;                     ///< Maximum queued spans
    bool enable_compression = true;                          ///< Enable data compression
    std::unordered_map<std::string, std::string> headers;   ///< Custom HTTP headers
    std::optional<std::string> service_name;                ///< Override service name
    
    /**
     * @brief Validate export configuration
     */
    result_void validate() const {
        if (endpoint.empty()) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Export endpoint cannot be empty");
        }
        
        if (timeout.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Timeout must be positive");
        }
        
        if (max_batch_size == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Batch size must be greater than 0");
        }
        
        if (max_queue_size < max_batch_size) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Queue size must be at least batch size");
        }
        
        return result_void::success();
    }
};

/**
 * @struct jaeger_span_data
 * @brief Jaeger-specific span representation
 */
struct jaeger_span_data {
    std::string trace_id;
    std::string span_id;
    std::string parent_span_id;
    std::string operation_name;
    std::string service_name;
    std::chrono::microseconds start_time;
    std::chrono::microseconds duration;
    std::vector<std::pair<std::string, std::string>> tags;
    std::vector<std::pair<std::string, std::string>> process_tags;
    
    /**
     * @brief Convert to Jaeger Thrift format
     */
    std::string to_thrift_json() const;
    
    /**
     * @brief Convert to Jaeger protobuf format
     */
    std::vector<uint8_t> to_protobuf() const;
};

/**
 * @struct zipkin_span_data
 * @brief Zipkin-specific span representation
 */
struct zipkin_span_data {
    std::string trace_id;
    std::string span_id;
    std::string parent_id;
    std::string name;
    std::string kind;
    std::chrono::microseconds timestamp;
    std::chrono::microseconds duration;
    std::string local_endpoint_service_name;
    std::string remote_endpoint_service_name;
    std::unordered_map<std::string, std::string> tags;
    bool shared = false;
    
    /**
     * @brief Convert to Zipkin JSON v2 format
     */
    std::string to_json_v2() const;
    
    /**
     * @brief Convert to Zipkin protobuf format
     */
    std::vector<uint8_t> to_protobuf() const;
};

/**
 * @class trace_exporter_interface
 * @brief Abstract interface for trace exporters
 */
class trace_exporter_interface {
public:
    virtual ~trace_exporter_interface() = default;
    
    /**
     * @brief Export a batch of spans
     */
    virtual result_void export_spans(const std::vector<trace_span>& spans) = 0;
    
    /**
     * @brief Flush any pending spans
     */
    virtual result_void flush() = 0;
    
    /**
     * @brief Shutdown the exporter
     */
    virtual result_void shutdown() = 0;
    
    /**
     * @brief Get exporter statistics
     */
    virtual std::unordered_map<std::string, std::size_t> get_stats() const = 0;
};

/**
 * @class jaeger_exporter
 * @brief Jaeger trace exporter implementation
 */
class jaeger_exporter : public trace_exporter_interface {
private:
    trace_export_config config_;
    std::atomic<std::size_t> exported_spans_{0};
    std::atomic<std::size_t> failed_exports_{0};
    std::atomic<std::size_t> dropped_spans_{0};
    
public:
    explicit jaeger_exporter(const trace_export_config& config)
        : config_(config) {}
    
    /**
     * @brief Convert internal span to Jaeger format
     */
    jaeger_span_data convert_span(const trace_span& span) const {
        jaeger_span_data jaeger_span;
        jaeger_span.trace_id = span.trace_id;
        jaeger_span.span_id = span.span_id;
        jaeger_span.parent_span_id = span.parent_span_id;
        jaeger_span.operation_name = span.operation_name;
        jaeger_span.service_name = config_.service_name.value_or(span.service_name);
        
        // Convert timestamps
        auto start_epoch = span.start_time.time_since_epoch();
        jaeger_span.start_time = std::chrono::duration_cast<std::chrono::microseconds>(start_epoch);
        
        auto end_epoch = span.end_time.time_since_epoch();
        jaeger_span.duration = std::chrono::duration_cast<std::chrono::microseconds>(end_epoch - start_epoch);
        
        // Convert tags
        for (const auto& [key, value] : span.tags) {
            jaeger_span.tags.emplace_back(key, value);
        }
        
        // Add process tags
        jaeger_span.process_tags.emplace_back("service.name", jaeger_span.service_name);
        
        return jaeger_span;
    }
    
    result_void export_spans(const std::vector<trace_span>& spans) override {
        try {
            std::vector<jaeger_span_data> jaeger_spans;
            jaeger_spans.reserve(spans.size());
            
            for (const auto& span : spans) {
                jaeger_spans.push_back(convert_span(span));
            }
            
            // Convert to appropriate format and send
            result_void send_result;
            if (config_.format == trace_export_format::jaeger_thrift) {
                send_result = send_thrift_batch(jaeger_spans);
            } else if (config_.format == trace_export_format::jaeger_grpc) {
                send_result = send_grpc_batch(jaeger_spans);
            } else {
                return result_void(monitoring_error_code::invalid_configuration,
                                 "Invalid Jaeger export format");
            }
            
            if (send_result) {
                exported_spans_ += spans.size();
            } else {
                failed_exports_++;
                return send_result;
            }
            
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "Jaeger export failed: " + std::string(e.what()));
        }
    }
    
    result_void flush() override {
        // Jaeger exporter typically sends immediately, so flush is a no-op
        return result_void::success();
    }
    
    result_void shutdown() override {
        return flush();
    }
    
    std::unordered_map<std::string, std::size_t> get_stats() const override {
        return {
            {"exported_spans", exported_spans_.load()},
            {"failed_exports", failed_exports_.load()},
            {"dropped_spans", dropped_spans_.load()}
        };
    }
    
private:
    result_void send_thrift_batch(const std::vector<jaeger_span_data>& spans) {
        // Simulate Thrift protocol sending
        // In real implementation, this would serialize to Thrift and send via HTTP/UDP
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
    
    result_void send_grpc_batch(const std::vector<jaeger_span_data>& spans) {
        // Simulate gRPC protocol sending
        // In real implementation, this would use Jaeger gRPC client
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
};

/**
 * @class zipkin_exporter
 * @brief Zipkin trace exporter implementation
 */
class zipkin_exporter : public trace_exporter_interface {
private:
    trace_export_config config_;
    std::atomic<std::size_t> exported_spans_{0};
    std::atomic<std::size_t> failed_exports_{0};
    std::atomic<std::size_t> dropped_spans_{0};
    
public:
    explicit zipkin_exporter(const trace_export_config& config)
        : config_(config) {}
    
    /**
     * @brief Convert internal span to Zipkin format
     */
    zipkin_span_data convert_span(const trace_span& span) const {
        zipkin_span_data zipkin_span;
        zipkin_span.trace_id = span.trace_id;
        zipkin_span.span_id = span.span_id;
        zipkin_span.parent_id = span.parent_span_id;
        zipkin_span.name = span.operation_name;
        zipkin_span.local_endpoint_service_name = config_.service_name.value_or(span.service_name);
        
        // Convert timestamps (Zipkin uses microseconds since epoch)
        auto start_epoch = span.start_time.time_since_epoch();
        zipkin_span.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(start_epoch);
        
        auto end_epoch = span.end_time.time_since_epoch();
        zipkin_span.duration = std::chrono::duration_cast<std::chrono::microseconds>(end_epoch - start_epoch);
        
        // Determine span kind from tags
        auto kind_it = span.tags.find("span.kind");
        if (kind_it != span.tags.end()) {
            zipkin_span.kind = kind_it->second;
        } else {
            zipkin_span.kind = "INTERNAL";
        }
        
        // Convert tags (exclude special fields)
        for (const auto& [key, value] : span.tags) {
            if (key != "span.kind") {
                zipkin_span.tags[key] = value;
            }
        }
        
        return zipkin_span;
    }
    
    result_void export_spans(const std::vector<trace_span>& spans) override {
        try {
            std::vector<zipkin_span_data> zipkin_spans;
            zipkin_spans.reserve(spans.size());
            
            for (const auto& span : spans) {
                zipkin_spans.push_back(convert_span(span));
            }
            
            // Convert to appropriate format and send
            result_void send_result;
            if (config_.format == trace_export_format::zipkin_json) {
                send_result = send_json_batch(zipkin_spans);
            } else if (config_.format == trace_export_format::zipkin_protobuf) {
                send_result = send_protobuf_batch(zipkin_spans);
            } else {
                return result_void(monitoring_error_code::invalid_configuration,
                                 "Invalid Zipkin export format");
            }
            
            if (send_result) {
                exported_spans_ += spans.size();
            } else {
                failed_exports_++;
                return send_result;
            }
            
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "Zipkin export failed: " + std::string(e.what()));
        }
    }
    
    result_void flush() override {
        // Zipkin exporter typically sends immediately, so flush is a no-op
        return result_void::success();
    }
    
    result_void shutdown() override {
        return flush();
    }
    
    std::unordered_map<std::string, std::size_t> get_stats() const override {
        return {
            {"exported_spans", exported_spans_.load()},
            {"failed_exports", failed_exports_.load()},
            {"dropped_spans", dropped_spans_.load()}
        };
    }
    
private:
    result_void send_json_batch(const std::vector<zipkin_span_data>& spans) {
        // Simulate JSON format sending via HTTP
        // In real implementation, this would serialize to JSON and POST to Zipkin
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
    
    result_void send_protobuf_batch(const std::vector<zipkin_span_data>& spans) {
        // Simulate Protocol Buffers format sending
        // In real implementation, this would serialize to protobuf and send via HTTP
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
};

/**
 * @class otlp_exporter
 * @brief OpenTelemetry Protocol (OTLP) trace exporter implementation
 */
class otlp_exporter : public trace_exporter_interface {
private:
    trace_export_config config_;
    std::unique_ptr<opentelemetry_tracer_adapter> otel_adapter_;
    std::atomic<std::size_t> exported_spans_{0};
    std::atomic<std::size_t> failed_exports_{0};
    std::atomic<std::size_t> dropped_spans_{0};
    
public:
    explicit otlp_exporter(const trace_export_config& config, const otel_resource& resource)
        : config_(config), otel_adapter_(std::make_unique<opentelemetry_tracer_adapter>(resource)) {}
    
    result_void export_spans(const std::vector<trace_span>& spans) override {
        try {
            // Convert to OpenTelemetry format first
            auto otel_result = otel_adapter_->convert_spans(spans);
            if (!otel_result) {
                failed_exports_++;
                return result_void(monitoring_error_code::processing_failed,
                                 "Failed to convert spans to OTEL format: " + otel_result.get_error().message);
            }
            
            const auto& otel_spans = otel_result.value();
            
            // Send via appropriate OTLP protocol
            result_void send_result;
            if (config_.format == trace_export_format::otlp_grpc) {
                send_result = send_grpc_batch(otel_spans);
            } else if (config_.format == trace_export_format::otlp_http_json) {
                send_result = send_http_json_batch(otel_spans);
            } else if (config_.format == trace_export_format::otlp_http_protobuf) {
                send_result = send_http_protobuf_batch(otel_spans);
            } else {
                return result_void(monitoring_error_code::invalid_configuration,
                                 "Invalid OTLP export format");
            }
            
            if (send_result) {
                exported_spans_ += spans.size();
            } else {
                failed_exports_++;
                return send_result;
            }
            
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "OTLP export failed: " + std::string(e.what()));
        }
    }
    
    result_void flush() override {
        // OTLP exporter typically sends immediately, so flush is a no-op
        return result_void::success();
    }
    
    result_void shutdown() override {
        return flush();
    }
    
    std::unordered_map<std::string, std::size_t> get_stats() const override {
        return {
            {"exported_spans", exported_spans_.load()},
            {"failed_exports", failed_exports_.load()},
            {"dropped_spans", dropped_spans_.load()}
        };
    }
    
private:
    result_void send_grpc_batch(const std::vector<otel_span_data>& spans) {
        // Simulate OTLP gRPC sending
        // In real implementation, this would use OTLP gRPC client
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
    
    result_void send_http_json_batch(const std::vector<otel_span_data>& spans) {
        // Simulate OTLP HTTP JSON sending
        // In real implementation, this would serialize OTEL spans to JSON and POST
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
    
    result_void send_http_protobuf_batch(const std::vector<otel_span_data>& spans) {
        // Simulate OTLP HTTP protobuf sending
        // In real implementation, this would serialize OTEL spans to protobuf and POST
        (void)spans; // Suppress unused parameter warning
        return result_void::success();
    }
};

/**
 * @class trace_exporter_factory
 * @brief Factory for creating trace exporters
 */
class trace_exporter_factory {
public:
    /**
     * @brief Create a trace exporter based on format
     */
    static std::unique_ptr<trace_exporter_interface> create_exporter(
        const trace_export_config& config,
        const otel_resource& resource = create_service_resource("monitoring_system", "2.0.0")) {
        
        switch (config.format) {
            case trace_export_format::jaeger_thrift:
            case trace_export_format::jaeger_grpc:
                return std::make_unique<jaeger_exporter>(config);
                
            case trace_export_format::zipkin_json:
            case trace_export_format::zipkin_protobuf:
                return std::make_unique<zipkin_exporter>(config);
                
            case trace_export_format::otlp_grpc:
            case trace_export_format::otlp_http_json:
            case trace_export_format::otlp_http_protobuf:
                return std::make_unique<otlp_exporter>(config, resource);
                
            default:
                return nullptr;
        }
    }
    
    /**
     * @brief Get supported formats for a specific backend
     */
    static std::vector<trace_export_format> get_supported_formats(const std::string& backend) {
        if (backend == "jaeger") {
            return {trace_export_format::jaeger_thrift, trace_export_format::jaeger_grpc};
        } else if (backend == "zipkin") {
            return {trace_export_format::zipkin_json, trace_export_format::zipkin_protobuf};
        } else if (backend == "otlp") {
            return {trace_export_format::otlp_grpc, trace_export_format::otlp_http_json, 
                   trace_export_format::otlp_http_protobuf};
        }
        return {};
    }
};

/**
 * @brief Helper function to create a Jaeger exporter
 */
inline std::unique_ptr<jaeger_exporter> create_jaeger_exporter(
    const std::string& endpoint,
    trace_export_format format = trace_export_format::jaeger_grpc) {
    
    trace_export_config config;
    config.endpoint = endpoint;
    config.format = format;
    return std::make_unique<jaeger_exporter>(config);
}

/**
 * @brief Helper function to create a Zipkin exporter
 */
inline std::unique_ptr<zipkin_exporter> create_zipkin_exporter(
    const std::string& endpoint,
    trace_export_format format = trace_export_format::zipkin_json) {
    
    trace_export_config config;
    config.endpoint = endpoint;
    config.format = format;
    return std::make_unique<zipkin_exporter>(config);
}

/**
 * @brief Helper function to create an OTLP exporter
 */
inline std::unique_ptr<otlp_exporter> create_otlp_exporter(
    const std::string& endpoint,
    const otel_resource& resource,
    trace_export_format format = trace_export_format::otlp_grpc) {
    
    trace_export_config config;
    config.endpoint = endpoint;
    config.format = format;
    return std::make_unique<otlp_exporter>(config, resource);
}

} // namespace monitoring_system