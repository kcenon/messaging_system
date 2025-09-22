#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <optional>
#include <variant>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include <map>
#include <set>

#include <kcenon/monitoring/web/dashboard_server.h>
#include <kcenon/monitoring/utils/metric_types.h>
#include <kcenon/monitoring/storage/metric_database.h>
#include <kcenon/monitoring/query/metric_query_engine.h>
#include <kcenon/monitoring/alerting/rule_engine.h>

namespace kcenon::monitoring::web {

// API versioning
enum class ApiVersion {
    V1,
    V2
};

// Query parameters for metric retrieval
struct MetricQueryParams {
    std::string metric_name;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::chrono::seconds interval = std::chrono::seconds(60);
    std::unordered_map<std::string, std::string> labels;
    std::vector<std::string> aggregations;
    size_t limit = 1000;
    size_t offset = 0;
    std::string order_by = "timestamp";
    bool descending = false;
};

// Aggregation result
struct AggregationResult {
    std::string function;
    double value;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> labels;
};

// Time series data point for JSON serialization
struct TimeSeriesPoint {
    double value;
    std::chrono::system_clock::time_point timestamp;
    std::unordered_map<std::string, std::string> labels;
};

// Metric metadata
struct MetricMetadata {
    std::string name;
    std::string type;
    std::string unit;
    std::string description;
    std::vector<std::string> label_keys;
    std::chrono::system_clock::time_point first_seen;
    std::chrono::system_clock::time_point last_seen;
    size_t data_points_count;
};

// Dashboard configuration
struct DashboardConfig {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> panels;
    std::unordered_map<std::string, std::string> settings;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

// Panel configuration
struct PanelConfig {
    std::string id;
    std::string title;
    std::string type; // graph, gauge, table, stat
    MetricQueryParams query;
    std::unordered_map<std::string, std::string> visualization_options;
    size_t refresh_interval_seconds = 30;
};

// Alert summary for dashboard
struct AlertSummary {
    size_t total_alerts;
    size_t critical_alerts;
    size_t warning_alerts;
    size_t info_alerts;
    std::vector<alerting::Alert> recent_alerts;
};

// System health status
struct SystemHealth {
    enum Status { HEALTHY, DEGRADED, UNHEALTHY };
    Status overall_status;
    std::unordered_map<std::string, Status> component_status;
    double cpu_usage_percent;
    double memory_usage_percent;
    double disk_usage_percent;
    size_t active_connections;
    std::chrono::system_clock::time_point last_check;
};

// Metric API handler
class MetricAPI {
public:
    MetricAPI();
    ~MetricAPI();

    // Initialize with storage backend
    void set_metric_database(std::shared_ptr<storage::MetricDatabase> db);
    void set_query_engine(std::shared_ptr<query::MetricQueryEngine> engine);
    void set_rule_engine(std::shared_ptr<alerting::RuleEngine> rule_engine);

    // Register API routes
    void register_routes(DashboardServer& server);

    // Core API endpoints
    HttpResponse get_metrics(const HttpRequest& request);
    HttpResponse get_metric_by_name(const HttpRequest& request, const std::string& metric_name);
    HttpResponse query_metrics(const HttpRequest& request);
    HttpResponse get_aggregations(const HttpRequest& request);

    // Metadata endpoints
    HttpResponse get_metric_metadata(const HttpRequest& request);
    HttpResponse list_metrics(const HttpRequest& request);
    HttpResponse get_label_values(const HttpRequest& request, const std::string& label_key);

    // Time series data endpoints
    HttpResponse get_time_series(const HttpRequest& request);
    HttpResponse export_time_series(const HttpRequest& request);

    // Dashboard endpoints
    HttpResponse get_dashboards(const HttpRequest& request);
    HttpResponse get_dashboard(const HttpRequest& request, const std::string& dashboard_id);
    HttpResponse create_dashboard(const HttpRequest& request);
    HttpResponse update_dashboard(const HttpRequest& request, const std::string& dashboard_id);
    HttpResponse delete_dashboard(const HttpRequest& request, const std::string& dashboard_id);

    // Alert endpoints
    HttpResponse get_alerts(const HttpRequest& request);
    HttpResponse get_alert_summary(const HttpRequest& request);
    HttpResponse acknowledge_alert(const HttpRequest& request, const std::string& alert_id);

    // System health endpoints
    HttpResponse get_health(const HttpRequest& request);
    HttpResponse get_ready(const HttpRequest& request);
    HttpResponse get_system_info(const HttpRequest& request);

    // Data transformation
    std::string metrics_to_json(const std::vector<metric>& metrics);
    std::string time_series_to_json(const std::vector<TimeSeriesPoint>& points);
    std::string aggregations_to_json(const std::vector<AggregationResult>& results);
    std::string metadata_to_json(const std::vector<MetricMetadata>& metadata);
    std::string dashboard_to_json(const DashboardConfig& dashboard);
    std::string alert_summary_to_json(const AlertSummary& summary);
    std::string health_to_json(const SystemHealth& health);

    // Query parsing
    MetricQueryParams parse_query_params(const HttpRequest& request);
    std::chrono::system_clock::time_point parse_timestamp(const std::string& timestamp_str);
    std::unordered_map<std::string, std::string> parse_labels(const std::string& labels_str);

    // Pagination support
    struct PaginationInfo {
        size_t total_items;
        size_t page_size;
        size_t current_page;
        size_t total_pages;
        bool has_next;
        bool has_prev;
    };

    PaginationInfo calculate_pagination(size_t total_items, size_t limit, size_t offset);
    std::string pagination_to_json(const PaginationInfo& info);

    // Response formatting
    HttpResponse json_response(const std::string& json_data, HttpStatus status = HttpStatus::OK);
    HttpResponse error_response(const std::string& error_message, HttpStatus status = HttpStatus::BAD_REQUEST);
    HttpResponse paginated_response(const std::string& data, const PaginationInfo& pagination);

    // WebSocket streaming
    void stream_metrics(const std::string& client_id, const MetricQueryParams& params);
    void stop_streaming(const std::string& client_id);

    // Export formats
    std::string export_csv(const std::vector<TimeSeriesPoint>& points);
    std::string export_prometheus(const std::vector<metric>& metrics);
    std::string export_influxdb_line_protocol(const std::vector<metric>& metrics);

    // Statistics
    struct ApiStats {
        size_t total_requests = 0;
        size_t successful_requests = 0;
        size_t failed_requests = 0;
        std::unordered_map<std::string, size_t> endpoint_counts;
        double average_response_time_ms = 0.0;
        std::chrono::system_clock::time_point start_time;
    };

    ApiStats get_stats() const;
    void reset_stats();

private:
    std::shared_ptr<storage::MetricDatabase> metric_db_;
    std::shared_ptr<query::MetricQueryEngine> query_engine_;
    std::shared_ptr<alerting::RuleEngine> rule_engine_;

    // Dashboard storage
    mutable std::mutex dashboards_mutex_;
    std::unordered_map<std::string, DashboardConfig> dashboards_;

    // Streaming clients
    mutable std::mutex streaming_mutex_;
    std::unordered_map<std::string, std::thread> streaming_threads_;
    std::unordered_map<std::string, std::atomic<bool>> streaming_flags_;

    // Statistics
    mutable std::mutex stats_mutex_;
    ApiStats stats_;

    // Cache
    mutable std::mutex cache_mutex_;
    std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> response_cache_;
    std::chrono::seconds cache_ttl_ = std::chrono::seconds(60);

    // Helper methods
    std::vector<metric> fetch_metrics(const MetricQueryParams& params);
    std::vector<AggregationResult> calculate_aggregations(const std::vector<metric>& metrics,
                                                         const std::vector<std::string>& functions);
    SystemHealth check_system_health();
    AlertSummary get_alert_summary_internal();

    // JSON serialization helpers
    std::string escape_json(const std::string& str);
    std::string format_timestamp(std::chrono::system_clock::time_point tp);
    std::string labels_to_json(const std::unordered_map<std::string, std::string>& labels);

    // Cache management
    std::optional<std::string> get_cached_response(const std::string& cache_key);
    void cache_response(const std::string& cache_key, const std::string& response);
    void cleanup_cache();

    // Validation
    bool validate_metric_name(const std::string& name);
    bool validate_time_range(std::chrono::system_clock::time_point start,
                            std::chrono::system_clock::time_point end);
    bool validate_query_params(const MetricQueryParams& params);
};

// Metric query builder for fluent API
class MetricQueryBuilder {
public:
    MetricQueryBuilder() = default;

    MetricQueryBuilder& metric(const std::string& name) {
        params_.metric_name = name;
        return *this;
    }

    MetricQueryBuilder& from(std::chrono::system_clock::time_point start) {
        params_.start_time = start;
        return *this;
    }

    MetricQueryBuilder& to(std::chrono::system_clock::time_point end) {
        params_.end_time = end;
        return *this;
    }

    MetricQueryBuilder& interval(std::chrono::seconds i) {
        params_.interval = i;
        return *this;
    }

    MetricQueryBuilder& label(const std::string& key, const std::string& value) {
        params_.labels[key] = value;
        return *this;
    }

    MetricQueryBuilder& aggregate(const std::string& function) {
        params_.aggregations.push_back(function);
        return *this;
    }

    MetricQueryBuilder& limit(size_t l) {
        params_.limit = l;
        return *this;
    }

    MetricQueryBuilder& offset(size_t o) {
        params_.offset = o;
        return *this;
    }

    MetricQueryBuilder& order_by(const std::string& field, bool desc = false) {
        params_.order_by = field;
        params_.descending = desc;
        return *this;
    }

    MetricQueryParams build() const {
        return params_;
    }

private:
    MetricQueryParams params_;
};

// JSON builder for API responses
class JsonBuilder {
public:
    JsonBuilder() : ss_("{") {}

    JsonBuilder& add(const std::string& key, const std::string& value) {
        if (!first_) ss_ << ",";
        ss_ << "\"" << key << "\":\"" << escape_string(value) << "\"";
        first_ = false;
        return *this;
    }

    JsonBuilder& add(const std::string& key, double value) {
        if (!first_) ss_ << ",";
        ss_ << "\"" << key << "\":" << value;
        first_ = false;
        return *this;
    }

    JsonBuilder& add(const std::string& key, int value) {
        if (!first_) ss_ << ",";
        ss_ << "\"" << key << "\":" << value;
        first_ = false;
        return *this;
    }

    JsonBuilder& add(const std::string& key, bool value) {
        if (!first_) ss_ << ",";
        ss_ << "\"" << key << "\":" << (value ? "true" : "false");
        first_ = false;
        return *this;
    }

    JsonBuilder& add_object(const std::string& key, const std::string& json_object) {
        if (!first_) ss_ << ",";
        ss_ << "\"" << key << "\":" << json_object;
        first_ = false;
        return *this;
    }

    JsonBuilder& add_array(const std::string& key, const std::vector<std::string>& items) {
        if (!first_) ss_ << ",";
        ss_ << "\"" << key << "\":[";
        for (size_t i = 0; i < items.size(); ++i) {
            if (i > 0) ss_ << ",";
            ss_ << "\"" << escape_string(items[i]) << "\"";
        }
        ss_ << "]";
        first_ = false;
        return *this;
    }

    std::string build() const {
        return ss_.str() + "}";
    }

private:
    mutable std::stringstream ss_;
    bool first_ = true;

    std::string escape_string(const std::string& str) const {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
};

} // namespace monitoring_system::web