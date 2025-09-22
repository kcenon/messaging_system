#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file metric_exporters.h
 * @brief Metric data exporters for various monitoring and observability systems
 * 
 * This file provides exporters for popular metric collection systems:
 * - Prometheus (Pull-based metrics system with HTTP endpoint)
 * - StatsD (Push-based UDP metrics aggregation system)
 * - OpenTelemetry Metrics (OTLP metrics protocol)
 * 
 * Each exporter handles format conversion and protocol-specific transmission
 * to the respective metrics backend.
 */

#include "../core/result_types.h"
#include "../core/error_codes.h"
#include "../interfaces/monitoring_interface.h"
#include "../interfaces/monitorable_interface.h"
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
#include <sstream>
#include <regex>
#include <mutex>
#include <cctype>
#include <cstdint>

namespace monitoring_system {

/**
 * @enum metric_export_format
 * @brief Supported metric export formats
 */
enum class metric_export_format {
    prometheus_text,        ///< Prometheus text exposition format
    prometheus_protobuf,    ///< Prometheus protocol buffers format
    statsd_plain,          ///< StatsD plain UDP format
    statsd_datadog,        ///< StatsD DataDog extension format
    otlp_grpc,             ///< OTLP gRPC metrics protocol
    otlp_http_json,        ///< OTLP HTTP JSON metrics protocol
    otlp_http_protobuf     ///< OTLP HTTP Protocol Buffers metrics
};

/**
 * @enum metric_type
 * @brief Metric types supported by exporters
 */
enum class metric_type {
    counter,    ///< Monotonically increasing counter
    gauge,      ///< Instantaneous value that can go up and down
    histogram,  ///< Distribution of values with buckets
    summary,    ///< Pre-calculated quantiles and count/sum
    timer       ///< StatsD-specific timer metric
};

/**
 * @struct metric_export_config
 * @brief Configuration for metric exporters
 */
struct metric_export_config {
    std::string endpoint;                                    ///< Endpoint URL or address
    std::uint16_t port = 0;                                 ///< Port number (for UDP/TCP)
    metric_export_format format = metric_export_format::prometheus_text;
    std::chrono::milliseconds push_interval{15000};         ///< Push interval for push-based systems
    std::chrono::milliseconds timeout{5000};                ///< Request timeout
    std::size_t max_batch_size = 1000;                     ///< Maximum metrics per batch
    std::size_t max_queue_size = 10000;                    ///< Maximum queued metrics
    bool enable_compression = false;                         ///< Enable data compression
    std::unordered_map<std::string, std::string> headers;   ///< Custom HTTP headers
    std::unordered_map<std::string, std::string> labels;    ///< Default labels/tags
    std::string job_name = "monitoring_system";             ///< Prometheus job name
    std::string instance_id;                                ///< Instance identifier
    
    /**
     * @brief Validate export configuration
     */
    result_void validate() const {
        if (endpoint.empty() && port == 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Either endpoint or port must be specified");
        }
        
        if (push_interval.count() <= 0) {
            return result_void(monitoring_error_code::invalid_configuration,
                             "Push interval must be positive");
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
 * @struct prometheus_metric_data
 * @brief Prometheus-specific metric representation
 */
struct prometheus_metric_data {
    std::string name;
    metric_type type;
    double value;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> labels;
    std::string help_text;
    
    /**
     * @brief Convert to Prometheus text format
     */
    std::string to_prometheus_text() const {
        std::ostringstream ss;
        
        // Add HELP line
        if (!help_text.empty()) {
            ss << "# HELP " << name << " " << help_text << "\n";
        }
        
        // Add TYPE line
        std::string type_str;
        switch (type) {
            case metric_type::counter: type_str = "counter"; break;
            case metric_type::gauge: type_str = "gauge"; break;
            case metric_type::histogram: type_str = "histogram"; break;
            case metric_type::summary: type_str = "summary"; break;
            case metric_type::timer: type_str = "gauge"; break; // Timer as gauge in Prometheus
        }
        ss << "# TYPE " << name << " " << type_str << "\n";
        
        // Add metric line
        ss << name;
        
        if (!labels.empty()) {
            ss << "{";
            bool first = true;
            for (const auto& [key, label_value] : labels) {
                if (!first) ss << ",";
                ss << key << "=\"" << escape_label_value(label_value) << "\"";
                first = false;
            }
            ss << "}";
        }
        
        ss << " " << value;
        
        // Add timestamp if available
        if (timestamp != std::chrono::system_clock::time_point{}) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                timestamp.time_since_epoch()).count();
            ss << " " << ms;
        }
        
        ss << "\n";
        return ss.str();
    }
    
private:
    std::string escape_label_value(const std::string& label_value) const {
        std::string escaped = label_value;
        // Escape backslashes, quotes, and newlines
        escaped = std::regex_replace(escaped, std::regex("\\\\"), "\\\\");
        escaped = std::regex_replace(escaped, std::regex("\""), "\\\"");
        escaped = std::regex_replace(escaped, std::regex("\n"), "\\n");
        return escaped;
    }
};

/**
 * @struct statsd_metric_data
 * @brief StatsD-specific metric representation
 */
struct statsd_metric_data {
    std::string name;
    metric_type type;
    double value;
    double sample_rate = 1.0;
    std::unordered_map<std::string, std::string> tags;
    
    /**
     * @brief Convert to StatsD format
     */
    std::string to_statsd_format(bool datadog_format = false) const {
        std::ostringstream ss;
        
        ss << name << ":" << value << "|";
        
        // Add type indicator
        switch (type) {
            case metric_type::counter: ss << "c"; break;
            case metric_type::gauge: ss << "g"; break;
            case metric_type::timer: ss << "ms"; break;
            case metric_type::histogram: ss << "h"; break;
            case metric_type::summary: ss << "s"; break;
        }
        
        // Add sample rate if not 1.0
        if (sample_rate != 1.0) {
            ss << "|@" << sample_rate;
        }
        
        // Add tags (DataDog format)
        if (datadog_format && !tags.empty()) {
            ss << "|#";
            bool first = true;
            for (const auto& [key, tag_value] : tags) {
                if (!first) ss << ",";
                ss << key << ":" << tag_value;
                first = false;
            }
        }
        
        return ss.str();
    }
};

/**
 * @class metric_exporter_interface
 * @brief Abstract interface for metric exporters
 */
class metric_exporter_interface {
public:
    virtual ~metric_exporter_interface() = default;
    
    /**
     * @brief Export a batch of metrics
     */
    virtual result_void export_metrics(const std::vector<monitoring_data>& metrics) = 0;
    
    /**
     * @brief Export a single metrics snapshot
     */
    virtual result_void export_snapshot(const metrics_snapshot& snapshot) = 0;
    
    /**
     * @brief Flush any pending metrics
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
    
    /**
     * @brief Start the exporter (for pull-based systems)
     */
    virtual result_void start() { return result_void::success(); }
    
    /**
     * @brief Stop the exporter
     */
    virtual result_void stop() { return result_void::success(); }
};

/**
 * @class prometheus_exporter
 * @brief Prometheus metric exporter implementation
 */
class prometheus_exporter : public metric_exporter_interface {
private:
    metric_export_config config_;
    std::atomic<std::size_t> exported_metrics_{0};
    std::atomic<std::size_t> failed_exports_{0};
    std::atomic<std::size_t> scrape_requests_{0};
    std::vector<prometheus_metric_data> current_metrics_;
    mutable std::mutex metrics_mutex_;
    
public:
    explicit prometheus_exporter(const metric_export_config& config)
        : config_(config) {}
    
    /**
     * @brief Convert monitoring_data to Prometheus format
     */
    std::vector<prometheus_metric_data> convert_monitoring_data(const monitoring_data& data) const {
        std::vector<prometheus_metric_data> prom_metrics;
        
        for (const auto& [name, value] : data.get_metrics()) {
            prometheus_metric_data metric;
            metric.name = sanitize_metric_name(name);
            metric.type = infer_metric_type(name, value);
            metric.value = value;
            metric.timestamp = data.get_timestamp();
            metric.help_text = "Metric from " + data.get_component_name();
            
            // Add component name as label
            metric.labels["component"] = data.get_component_name();
            
            // Add custom labels from config
            for (const auto& [key, label_value] : config_.labels) {
                metric.labels[key] = label_value;
            }
            
            // Add tags as labels
            for (const auto& [key, tag_value] : data.get_tags()) {
                metric.labels[sanitize_label_name(key)] = tag_value;
            }
            
            if (!config_.instance_id.empty()) {
                metric.labels["instance"] = config_.instance_id;
            }
            
            prom_metrics.push_back(std::move(metric));
        }
        
        return prom_metrics;
    }
    
    /**
     * @brief Convert metrics_snapshot to Prometheus format
     */
    std::vector<prometheus_metric_data> convert_snapshot(const metrics_snapshot& snapshot) const {
        std::vector<prometheus_metric_data> prom_metrics;
        
        for (const auto& metric_val : snapshot.metrics) {
            prometheus_metric_data metric;
            metric.name = sanitize_metric_name(metric_val.name);
            metric.type = infer_metric_type(metric_val.name, metric_val.value);
            metric.value = metric_val.value;
            metric.timestamp = metric_val.timestamp;
            metric.help_text = "System metric";
            
            // Add source as label
            if (!snapshot.source_id.empty()) {
                metric.labels["source"] = snapshot.source_id;
            }
            
            // Add custom labels from config
            for (const auto& [key, label_value] : config_.labels) {
                metric.labels[key] = label_value;
            }
            
            // Add tags as labels
            for (const auto& [key, tag_value] : metric_val.tags) {
                metric.labels[sanitize_label_name(key)] = tag_value;
            }
            
            if (!config_.instance_id.empty()) {
                metric.labels["instance"] = config_.instance_id;
            }
            
            prom_metrics.push_back(std::move(metric));
        }
        
        return prom_metrics;
    }
    
    result_void export_metrics(const std::vector<monitoring_data>& metrics) override {
        try {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            current_metrics_.clear();
            
            for (const auto& data : metrics) {
                auto prom_metrics = convert_monitoring_data(data);
                current_metrics_.insert(current_metrics_.end(), 
                                      prom_metrics.begin(), prom_metrics.end());
            }
            
            exported_metrics_ += metrics.size();
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "Prometheus export failed: " + std::string(e.what()));
        }
    }
    
    result_void export_snapshot(const metrics_snapshot& snapshot) override {
        try {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto prom_metrics = convert_snapshot(snapshot);
            current_metrics_.insert(current_metrics_.end(), 
                                  prom_metrics.begin(), prom_metrics.end());
            
            exported_metrics_++;
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "Prometheus snapshot export failed: " + std::string(e.what()));
        }
    }
    
    /**
     * @brief Get current metrics in Prometheus format (for HTTP endpoint)
     */
    std::string get_metrics_text() const {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        std::ostringstream ss;
        
        for (const auto& metric : current_metrics_) {
            ss << metric.to_prometheus_text();
        }
        
        // Increment scrape counter
        const_cast<std::atomic<std::size_t>&>(scrape_requests_)++;
        
        return ss.str();
    }
    
    result_void flush() override {
        // Prometheus is pull-based, so flush is a no-op
        return result_void::success();
    }
    
    result_void shutdown() override {
        return flush();
    }
    
    std::unordered_map<std::string, std::size_t> get_stats() const override {
        return {
            {"exported_metrics", exported_metrics_.load()},
            {"failed_exports", failed_exports_.load()},
            {"scrape_requests", scrape_requests_.load()},
            {"current_metrics_count", current_metrics_.size()}
        };
    }
    
private:
    std::string sanitize_metric_name(const std::string& name) const {
        std::string sanitized = name;
        // Replace invalid characters with underscores
        std::regex invalid_chars("[^a-zA-Z0-9_:]");
        sanitized = std::regex_replace(sanitized, invalid_chars, "_");
        
        // Ensure it starts with a letter or underscore
        if (!sanitized.empty() && !std::isalpha(sanitized[0]) && sanitized[0] != '_') {
            sanitized = "_" + sanitized;
        }
        
        return sanitized;
    }
    
    std::string sanitize_label_name(const std::string& name) const {
        std::string sanitized = name;
        // Replace invalid characters with underscores
        std::regex invalid_chars("[^a-zA-Z0-9_]");
        sanitized = std::regex_replace(sanitized, invalid_chars, "_");
        
        // Ensure it starts with a letter or underscore
        if (!sanitized.empty() && !std::isalpha(sanitized[0]) && sanitized[0] != '_') {
            sanitized = "_" + sanitized;
        }
        
        return sanitized;
    }
    
    metric_type infer_metric_type(const std::string& name, double /*value*/) const {
        // Simple heuristics for metric type inference
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        if (lower_name.find("count") != std::string::npos ||
            lower_name.find("total") != std::string::npos ||
            lower_name.find("requests") != std::string::npos) {
            return metric_type::counter;
        } else if (lower_name.find("histogram") != std::string::npos ||
                  lower_name.find("bucket") != std::string::npos) {
            return metric_type::histogram;
        } else if (lower_name.find("summary") != std::string::npos ||
                  lower_name.find("quantile") != std::string::npos) {
            return metric_type::summary;
        } else {
            return metric_type::gauge; // Default to gauge
        }
    }
};

/**
 * @class statsd_exporter
 * @brief StatsD metric exporter implementation
 */
class statsd_exporter : public metric_exporter_interface {
private:
    metric_export_config config_;
    std::atomic<std::size_t> exported_metrics_{0};
    std::atomic<std::size_t> failed_exports_{0};
    std::atomic<std::size_t> sent_packets_{0};
    
public:
    explicit statsd_exporter(const metric_export_config& config)
        : config_(config) {}
    
    /**
     * @brief Convert monitoring_data to StatsD format
     */
    std::vector<statsd_metric_data> convert_monitoring_data(const monitoring_data& data) const {
        std::vector<statsd_metric_data> statsd_metrics;
        
        for (const auto& [name, value] : data.get_metrics()) {
            statsd_metric_data metric;
            metric.name = sanitize_metric_name(name);
            metric.type = infer_metric_type(name, value);
            metric.value = value;
            metric.sample_rate = 1.0;
            
            // Add component as tag
            metric.tags["component"] = data.get_component_name();
            
            // Add custom tags from config
            for (const auto& [key, tag_value] : config_.labels) {
                metric.tags[key] = tag_value;
            }
            
            // Add tags from data
            for (const auto& [key, tag_value] : data.get_tags()) {
                metric.tags[key] = tag_value;
            }
            
            if (!config_.instance_id.empty()) {
                metric.tags["instance"] = config_.instance_id;
            }
            
            statsd_metrics.push_back(std::move(metric));
        }
        
        return statsd_metrics;
    }
    
    /**
     * @brief Convert metrics_snapshot to StatsD format
     */
    std::vector<statsd_metric_data> convert_snapshot(const metrics_snapshot& snapshot) const {
        std::vector<statsd_metric_data> statsd_metrics;
        
        for (const auto& metric_val : snapshot.metrics) {
            statsd_metric_data metric;
            metric.name = sanitize_metric_name(metric_val.name);
            metric.type = infer_metric_type(metric_val.name, metric_val.value);
            metric.value = metric_val.value;
            metric.sample_rate = 1.0;
            
            // Add source as tag
            if (!snapshot.source_id.empty()) {
                metric.tags["source"] = snapshot.source_id;
            }
            
            // Add custom tags from config
            for (const auto& [key, tag_value] : config_.labels) {
                metric.tags[key] = tag_value;
            }
            
            // Add tags from metric
            for (const auto& [key, tag_value] : metric_val.tags) {
                metric.tags[key] = tag_value;
            }
            
            if (!config_.instance_id.empty()) {
                metric.tags["instance"] = config_.instance_id;
            }
            
            statsd_metrics.push_back(std::move(metric));
        }
        
        return statsd_metrics;
    }
    
    result_void export_metrics(const std::vector<monitoring_data>& metrics) override {
        try {
            std::vector<std::string> statsd_lines;
            
            for (const auto& data : metrics) {
                auto statsd_metrics = convert_monitoring_data(data);
                for (const auto& metric : statsd_metrics) {
                    bool datadog_format = (config_.format == metric_export_format::statsd_datadog);
                    statsd_lines.push_back(metric.to_statsd_format(datadog_format));
                }
            }
            
            // Send via UDP (simulated)
            auto send_result = send_udp_batch(statsd_lines);
            if (send_result) {
                exported_metrics_ += metrics.size();
                sent_packets_++;
            } else {
                failed_exports_++;
                return send_result;
            }
            
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "StatsD export failed: " + std::string(e.what()));
        }
    }
    
    result_void export_snapshot(const metrics_snapshot& snapshot) override {
        try {
            auto statsd_metrics = convert_snapshot(snapshot);
            std::vector<std::string> statsd_lines;
            
            for (const auto& metric : statsd_metrics) {
                bool datadog_format = (config_.format == metric_export_format::statsd_datadog);
                statsd_lines.push_back(metric.to_statsd_format(datadog_format));
            }
            
            // Send via UDP (simulated)
            auto send_result = send_udp_batch(statsd_lines);
            if (send_result) {
                exported_metrics_++;
                sent_packets_++;
            } else {
                failed_exports_++;
                return send_result;
            }
            
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "StatsD snapshot export failed: " + std::string(e.what()));
        }
    }
    
    result_void flush() override {
        // StatsD is push-based and sends immediately, so flush is a no-op
        return result_void::success();
    }
    
    result_void shutdown() override {
        return flush();
    }
    
    std::unordered_map<std::string, std::size_t> get_stats() const override {
        return {
            {"exported_metrics", exported_metrics_.load()},
            {"failed_exports", failed_exports_.load()},
            {"sent_packets", sent_packets_.load()}
        };
    }
    
private:
    result_void send_udp_batch(const std::vector<std::string>& lines) {
        // Simulate UDP sending
        // In real implementation, this would create UDP socket and send packets
        (void)lines; // Suppress unused parameter warning
        return result_void::success();
    }
    
    std::string sanitize_metric_name(const std::string& name) const {
        std::string sanitized = name;
        // Replace dots and spaces with underscores for StatsD
        std::regex invalid_chars("[.\\s]+");
        sanitized = std::regex_replace(sanitized, invalid_chars, "_");
        return sanitized;
    }
    
    metric_type infer_metric_type(const std::string& name, double /*value*/) const {
        // Simple heuristics for StatsD metric type inference
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        
        if (lower_name.find("count") != std::string::npos ||
            lower_name.find("total") != std::string::npos) {
            return metric_type::counter;
        } else if (lower_name.find("time") != std::string::npos ||
                  lower_name.find("duration") != std::string::npos ||
                  lower_name.find("latency") != std::string::npos) {
            return metric_type::timer;
        } else {
            return metric_type::gauge; // Default to gauge
        }
    }
};

/**
 * @class otlp_metrics_exporter
 * @brief OpenTelemetry Protocol (OTLP) metrics exporter implementation
 */
class otlp_metrics_exporter : public metric_exporter_interface {
private:
    metric_export_config config_;
    std::unique_ptr<opentelemetry_metrics_adapter> otel_adapter_;
    std::atomic<std::size_t> exported_metrics_{0};
    std::atomic<std::size_t> failed_exports_{0};
    
public:
    explicit otlp_metrics_exporter(const metric_export_config& config, const otel_resource& resource)
        : config_(config), otel_adapter_(std::make_unique<opentelemetry_metrics_adapter>(resource)) {}
    
    result_void export_metrics(const std::vector<monitoring_data>& metrics) override {
        try {
            for (const auto& data : metrics) {
                // Convert to OpenTelemetry format
                auto otel_result = otel_adapter_->convert_monitoring_data(data);
                if (!otel_result) {
                    failed_exports_++;
                    return result_void(monitoring_error_code::processing_failed,
                                     "Failed to convert metrics to OTEL format: " + otel_result.get_error().message);
                }
                
                const auto& otel_metrics = otel_result.value();
                
                // Send via appropriate OTLP protocol
                auto send_result = send_otlp_batch(otel_metrics);
                if (!send_result) {
                    failed_exports_++;
                    return send_result;
                }
            }
            
            exported_metrics_ += metrics.size();
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "OTLP metrics export failed: " + std::string(e.what()));
        }
    }
    
    result_void export_snapshot(const metrics_snapshot& snapshot) override {
        try {
            // Convert to OpenTelemetry format
            auto otel_result = otel_adapter_->convert_metrics(snapshot);
            if (!otel_result) {
                failed_exports_++;
                return result_void(monitoring_error_code::processing_failed,
                                 "Failed to convert snapshot to OTEL format: " + otel_result.get_error().message);
            }
            
            const auto& otel_metrics = otel_result.value();
            
            // Send via appropriate OTLP protocol
            auto send_result = send_otlp_batch(otel_metrics);
            if (!send_result) {
                failed_exports_++;
                return send_result;
            }
            
            exported_metrics_++;
            return result_void::success();
            
        } catch (const std::exception& e) {
            failed_exports_++;
            return result_void(monitoring_error_code::operation_failed,
                             "OTLP snapshot export failed: " + std::string(e.what()));
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
            {"exported_metrics", exported_metrics_.load()},
            {"failed_exports", failed_exports_.load()}
        };
    }
    
private:
    result_void send_otlp_batch(const std::vector<otel_metric_data>& metrics) {
        // Simulate OTLP sending based on format
        // In real implementation, this would use OTLP client
        (void)metrics; // Suppress unused parameter warning
        return result_void::success();
    }
};

/**
 * @class metric_exporter_factory
 * @brief Factory for creating metric exporters
 */
class metric_exporter_factory {
public:
    /**
     * @brief Create a metric exporter based on format
     */
    static std::unique_ptr<metric_exporter_interface> create_exporter(
        const metric_export_config& config,
        const otel_resource& resource = create_service_resource("monitoring_system", "2.0.0")) {
        
        switch (config.format) {
            case metric_export_format::prometheus_text:
            case metric_export_format::prometheus_protobuf:
                return std::make_unique<prometheus_exporter>(config);
                
            case metric_export_format::statsd_plain:
            case metric_export_format::statsd_datadog:
                return std::make_unique<statsd_exporter>(config);
                
            case metric_export_format::otlp_grpc:
            case metric_export_format::otlp_http_json:
            case metric_export_format::otlp_http_protobuf:
                return std::make_unique<otlp_metrics_exporter>(config, resource);
                
            default:
                return nullptr;
        }
    }
    
    /**
     * @brief Get supported formats for a specific backend
     */
    static std::vector<metric_export_format> get_supported_formats(const std::string& backend) {
        if (backend == "prometheus") {
            return {metric_export_format::prometheus_text, metric_export_format::prometheus_protobuf};
        } else if (backend == "statsd") {
            return {metric_export_format::statsd_plain, metric_export_format::statsd_datadog};
        } else if (backend == "otlp") {
            return {metric_export_format::otlp_grpc, metric_export_format::otlp_http_json, 
                   metric_export_format::otlp_http_protobuf};
        }
        return {};
    }
};

/**
 * @brief Helper function to create a Prometheus exporter
 */
inline std::unique_ptr<prometheus_exporter> create_prometheus_exporter(
    std::uint16_t port = 9090,
    const std::string& job_name = "monitoring_system") {
    
    metric_export_config config;
    config.port = port;
    config.format = metric_export_format::prometheus_text;
    config.job_name = job_name;
    return std::make_unique<prometheus_exporter>(config);
}

/**
 * @brief Helper function to create a StatsD exporter
 */
inline std::unique_ptr<statsd_exporter> create_statsd_exporter(
    const std::string& host = "localhost",
    std::uint16_t port = 8125,
    bool datadog_format = false) {
    
    metric_export_config config;
    config.endpoint = host;
    config.port = port;
    config.format = datadog_format ? metric_export_format::statsd_datadog : metric_export_format::statsd_plain;
    return std::make_unique<statsd_exporter>(config);
}

/**
 * @brief Helper function to create an OTLP metrics exporter
 */
inline std::unique_ptr<otlp_metrics_exporter> create_otlp_metrics_exporter(
    const std::string& endpoint,
    const otel_resource& resource,
    metric_export_format format = metric_export_format::otlp_grpc) {
    
    metric_export_config config;
    config.endpoint = endpoint;
    config.format = format;
    return std::make_unique<otlp_metrics_exporter>(config, resource);
}

} // namespace monitoring_system