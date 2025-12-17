// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/collectors/message_bus_collector.h>

#include <algorithm>
#include <cmath>
#include <numeric>

#ifdef BUILD_WITH_MONITORING_SYSTEM

namespace kcenon::messaging::collectors {

// ============================================================================
// message_bus_collector Implementation
// ============================================================================

message_bus_collector::message_bus_collector()
    : init_time_(std::chrono::steady_clock::now()) {}

message_bus_collector::~message_bus_collector() = default;

bool message_bus_collector::initialize(
    const std::unordered_map<std::string, std::string>& config) {

    // Parse configuration
    auto it = config.find("enable_latency_tracking");
    if (it != config.end()) {
        enable_latency_tracking_ = (it->second == "true" || it->second == "1");
    }

    it = config.find("latency_sample_size");
    if (it != config.end()) {
        try {
            max_latency_samples_ = std::stoull(it->second);
        } catch (...) {
            // Keep default
        }
    }

    it = config.find("enable_topic_metrics");
    if (it != config.end()) {
        enable_topic_metrics_ = (it->second == "true" || it->second == "1");
    }

    it = config.find("use_event_bus");
    if (it != config.end()) {
        use_event_bus_ = (it->second == "true" || it->second == "1");
    }

    // Subscribe to events if enabled
    if (use_event_bus_) {
        subscribe_to_events();
    }

    is_healthy_.store(true);
    return true;
}

std::vector<monitoring::metric> message_bus_collector::collect() {
    ++collection_count_;

    std::vector<monitoring::metric> metrics;

    try {
        // Collect from primary bus
        if (primary_bus_) {
            auto stats = collect_from_primary_bus();
            add_bus_metrics(metrics, "primary", stats);

            if (enable_topic_metrics_) {
                add_topic_metrics(metrics, "primary", stats);
            }

            update_throughput_tracking("primary", stats);

            std::lock_guard<std::mutex> lock(buses_mutex_);
            last_stats_["primary"] = stats;
        }

        // Collect from registered buses
        {
            std::lock_guard<std::mutex> lock(buses_mutex_);
            for (const auto& [name, provider] : bus_providers_) {
                try {
                    auto stats = provider();
                    add_bus_metrics(metrics, name, stats);

                    if (enable_topic_metrics_) {
                        add_topic_metrics(metrics, name, stats);
                    }

                    update_throughput_tracking(name, stats);
                    last_stats_[name] = stats;
                } catch (...) {
                    ++collection_errors_;
                }
            }
        }

        is_healthy_.store(true);
    } catch (...) {
        ++collection_errors_;
        is_healthy_.store(false);
    }

    return metrics;
}

std::vector<std::string> message_bus_collector::get_metric_types() const {
    return {
        "messaging_messages_published_total",
        "messaging_messages_processed_total",
        "messaging_messages_failed_total",
        "messaging_messages_dropped_total",
        "messaging_queue_depth",
        "messaging_queue_capacity",
        "messaging_queue_utilization_percent",
        "messaging_throughput_per_second",
        "messaging_latency_average_ms",
        "messaging_latency_max_ms",
        "messaging_latency_min_ms",
        "messaging_topic_count",
        "messaging_total_subscribers",
        "messaging_subscribers_per_topic",
        "messaging_worker_threads",
        "messaging_is_running"
    };
}

bool message_bus_collector::is_healthy() const {
    return is_healthy_.load();
}

std::unordered_map<std::string, double> message_bus_collector::get_statistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);

    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        now - init_time_).count();

    return {
        {"collection_count", static_cast<double>(collection_count_.load())},
        {"collection_errors", static_cast<double>(collection_errors_.load())},
        {"uptime_seconds", static_cast<double>(uptime)},
        {"registered_buses", static_cast<double>(bus_providers_.size() +
                                                  (primary_bus_ ? 1 : 0))}
    };
}

void message_bus_collector::set_message_bus(std::shared_ptr<message_bus> bus) {
    primary_bus_ = std::move(bus);
}

void message_bus_collector::register_message_bus(
    const std::string& name,
    std::function<message_bus_stats()> stats_provider) {

    std::lock_guard<std::mutex> lock(buses_mutex_);
    bus_providers_[name] = std::move(stats_provider);
}

void message_bus_collector::unregister_message_bus(const std::string& name) {
    std::lock_guard<std::mutex> lock(buses_mutex_);
    bus_providers_.erase(name);
    last_stats_.erase(name);

    {
        std::lock_guard<std::mutex> latency_lock(latency_mutex_);
        latency_samples_.erase(name);
    }

    {
        std::lock_guard<std::mutex> throughput_lock(throughput_mutex_);
        throughput_trackers_.erase(name);
    }
}

std::vector<std::string> message_bus_collector::get_registered_buses() const {
    std::lock_guard<std::mutex> lock(buses_mutex_);

    std::vector<std::string> names;
    if (primary_bus_) {
        names.push_back("primary");
    }

    for (const auto& [name, _] : bus_providers_) {
        names.push_back(name);
    }

    return names;
}

void message_bus_collector::record_latency(const std::string& bus_name,
                                            double latency_ms) {
    if (!enable_latency_tracking_) {
        return;
    }

    std::lock_guard<std::mutex> lock(latency_mutex_);

    auto& samples = latency_samples_[bus_name];
    samples.push_back({latency_ms, std::chrono::steady_clock::now()});

    while (samples.size() > max_latency_samples_) {
        samples.pop_front();
    }
}

std::optional<std::tuple<double, double, double>>
message_bus_collector::get_latency_stats(const std::string& bus_name) const {
    std::lock_guard<std::mutex> lock(latency_mutex_);

    auto it = latency_samples_.find(bus_name);
    if (it == latency_samples_.end() || it->second.empty()) {
        return std::nullopt;
    }

    return calculate_latency_stats(it->second);
}

void message_bus_collector::set_latency_tracking(bool enable) {
    enable_latency_tracking_ = enable;
}

void message_bus_collector::set_topic_metrics(bool enable) {
    enable_topic_metrics_ = enable;
}

void message_bus_collector::set_latency_sample_size(size_t size) {
    max_latency_samples_ = size;
}

message_bus_stats message_bus_collector::collect_from_primary_bus() {
    message_bus_stats stats;

    if (!primary_bus_) {
        return stats;
    }

    auto snapshot = primary_bus_->get_statistics();
    stats.messages_published = snapshot.messages_published;
    stats.messages_processed = snapshot.messages_processed;
    stats.messages_failed = snapshot.messages_failed;
    stats.messages_dropped = snapshot.messages_dropped;

    stats.worker_thread_count = primary_bus_->worker_count();
    stats.is_running = primary_bus_->is_running();

    // Calculate latency from samples if available
    {
        std::lock_guard<std::mutex> lock(latency_mutex_);
        auto it = latency_samples_.find("primary");
        if (it != latency_samples_.end() && !it->second.empty()) {
            auto [avg, max_val, min_val] = calculate_latency_stats(it->second);
            stats.average_latency_ms = avg;
            stats.max_latency_ms = max_val;
            stats.min_latency_ms = min_val;
        }
    }

    // Calculate throughput
    {
        std::lock_guard<std::mutex> lock(throughput_mutex_);
        auto it = throughput_trackers_.find("primary");
        if (it != throughput_trackers_.end()) {
            stats.throughput_per_second = it->second.current_throughput;
        }
    }

    return stats;
}

std::vector<monitoring::metric> message_bus_collector::collect_from_buses() {
    std::vector<monitoring::metric> metrics;

    std::lock_guard<std::mutex> lock(buses_mutex_);
    for (const auto& [name, provider] : bus_providers_) {
        try {
            auto stats = provider();
            add_bus_metrics(metrics, name, stats);
            last_stats_[name] = stats;
        } catch (...) {
            ++collection_errors_;
        }
    }

    return metrics;
}

void message_bus_collector::add_bus_metrics(
    std::vector<monitoring::metric>& metrics,
    const std::string& bus_name,
    const message_bus_stats& stats) {

    // Counter metrics (totals)
    metrics.push_back(create_metric(
        "messaging_messages_published_total",
        static_cast<double>(stats.messages_published),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_messages_processed_total",
        static_cast<double>(stats.messages_processed),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_messages_failed_total",
        static_cast<double>(stats.messages_failed),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_messages_dropped_total",
        static_cast<double>(stats.messages_dropped),
        bus_name));

    // Gauge metrics (current state)
    metrics.push_back(create_metric(
        "messaging_queue_depth",
        static_cast<double>(stats.queue_depth),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_queue_capacity",
        static_cast<double>(stats.queue_capacity),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_queue_utilization_percent",
        stats.queue_utilization_percent,
        bus_name));

    // Throughput metrics
    metrics.push_back(create_metric(
        "messaging_throughput_per_second",
        stats.throughput_per_second,
        bus_name));

    // Latency metrics
    if (enable_latency_tracking_) {
        metrics.push_back(create_metric(
            "messaging_latency_average_ms",
            stats.average_latency_ms,
            bus_name));

        metrics.push_back(create_metric(
            "messaging_latency_max_ms",
            stats.max_latency_ms,
            bus_name));

        metrics.push_back(create_metric(
            "messaging_latency_min_ms",
            stats.min_latency_ms,
            bus_name));
    }

    // Worker metrics
    metrics.push_back(create_metric(
        "messaging_worker_threads",
        static_cast<double>(stats.worker_thread_count),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_is_running",
        stats.is_running ? 1.0 : 0.0,
        bus_name));
}

void message_bus_collector::add_topic_metrics(
    std::vector<monitoring::metric>& metrics,
    const std::string& bus_name,
    const message_bus_stats& stats) {

    // Overall topic metrics
    metrics.push_back(create_metric(
        "messaging_topic_count",
        static_cast<double>(stats.topic_count),
        bus_name));

    metrics.push_back(create_metric(
        "messaging_total_subscribers",
        static_cast<double>(stats.total_subscriber_count),
        bus_name));

    // Per-topic subscriber counts
    for (const auto& [topic, count] : stats.subscribers_per_topic) {
        monitoring::metric m;
        m.name = "messaging_subscribers_per_topic";
        m.value = static_cast<double>(count);
        m.tags["bus"] = bus_name;
        m.tags["topic"] = topic;
        m.tags["collector"] = get_name();
        m.type = monitoring::metric_type::gauge;
        m.timestamp = std::chrono::system_clock::now();
        metrics.push_back(m);
    }
}

void message_bus_collector::update_throughput_tracking(
    const std::string& bus_name,
    const message_bus_stats& stats) {

    std::lock_guard<std::mutex> lock(throughput_mutex_);

    auto& tracker = throughput_trackers_[bus_name];
    auto now = std::chrono::steady_clock::now();

    if (tracker.window_start == std::chrono::steady_clock::time_point{}) {
        // Initialize tracker
        tracker.window_start = now;
        tracker.messages_at_start = stats.messages_processed;
        tracker.current_throughput = 0.0;
        return;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - tracker.window_start).count();

    if (elapsed >= 1000) {  // Update every second
        uint64_t messages_processed = stats.messages_processed - tracker.messages_at_start;
        tracker.current_throughput = static_cast<double>(messages_processed) /
                                     (static_cast<double>(elapsed) / 1000.0);

        // Reset window
        tracker.window_start = now;
        tracker.messages_at_start = stats.messages_processed;
    }
}

std::tuple<double, double, double> message_bus_collector::calculate_latency_stats(
    const std::deque<latency_sample>& samples) const {

    if (samples.empty()) {
        return {0.0, 0.0, 0.0};
    }

    double sum = 0.0;
    double max_val = std::numeric_limits<double>::lowest();
    double min_val = std::numeric_limits<double>::max();

    for (const auto& sample : samples) {
        sum += sample.latency_ms;
        max_val = std::max(max_val, sample.latency_ms);
        min_val = std::min(min_val, sample.latency_ms);
    }

    double avg = sum / static_cast<double>(samples.size());
    return {avg, max_val, min_val};
}

monitoring::metric message_bus_collector::create_metric(
    const std::string& name,
    double value,
    const std::string& bus_name) const {

    monitoring::metric m;
    m.name = name;
    m.value = value;
    m.tags["bus"] = bus_name;
    m.tags["collector"] = get_name();
    m.type = monitoring::metric_type::gauge;
    m.timestamp = std::chrono::system_clock::now();
    return m;
}

void message_bus_collector::subscribe_to_events() {
    // Event subscription would be implemented here when event_bus is available
    // This is a placeholder for future event-driven metrics collection
}

void message_bus_collector::handle_messaging_event(
    const messaging_metric_event& event) {
    // Handle incoming events
    (void)event;
}

// ============================================================================
// message_bus_health_monitor Implementation
// ============================================================================

message_bus_health_monitor::message_bus_health_monitor(
    const health_thresholds& thresholds)
    : thresholds_(thresholds) {}

message_bus_health_monitor::health_report
message_bus_health_monitor::analyze_health(
    const message_bus_stats& stats,
    const std::string& bus_name) {

    health_report report;
    report.bus_name = bus_name;
    report.timestamp = std::chrono::steady_clock::now();

    // Run all health checks
    check_queue_saturation(report, stats);
    check_failure_rate(report, stats);
    check_latency(report, stats);
    check_throughput(report, stats);

    // Determine overall status
    report.status = calculate_status(report.issues);

    // Store metrics
    report.metrics["queue_utilization"] = stats.queue_utilization_percent;
    report.metrics["throughput"] = stats.throughput_per_second;
    report.metrics["average_latency"] = stats.average_latency_ms;

    if (stats.messages_published > 0) {
        report.metrics["failure_rate"] =
            static_cast<double>(stats.messages_failed) /
            static_cast<double>(stats.messages_published);
    }

    // Add to history
    {
        std::lock_guard<std::mutex> lock(history_mutex_);
        health_history_.push_back(report);
        while (health_history_.size() > max_history_size_) {
            health_history_.erase(health_history_.begin());
        }
    }

    return report;
}

message_bus_health_monitor::health_status
message_bus_health_monitor::get_overall_health(
    const std::unordered_map<std::string, message_bus_stats>& bus_stats) {

    health_status worst = health_status::healthy;

    for (const auto& [name, stats] : bus_stats) {
        auto report = analyze_health(stats, name);
        if (static_cast<int>(report.status) > static_cast<int>(worst)) {
            worst = report.status;
        }
    }

    return worst;
}

void message_bus_health_monitor::update_thresholds(
    const health_thresholds& thresholds) {

    std::lock_guard<std::mutex> lock(thresholds_mutex_);
    thresholds_ = thresholds;
}

message_bus_health_monitor::health_thresholds
message_bus_health_monitor::get_thresholds() const {

    std::lock_guard<std::mutex> lock(thresholds_mutex_);
    return thresholds_;
}

std::vector<message_bus_health_monitor::health_report>
message_bus_health_monitor::get_health_history(
    const std::optional<std::string>& bus_name,
    size_t max_count) const {

    std::lock_guard<std::mutex> lock(history_mutex_);

    std::vector<health_report> result;

    for (auto it = health_history_.rbegin();
         it != health_history_.rend() && result.size() < max_count;
         ++it) {
        if (!bus_name.has_value() || it->bus_name == bus_name.value()) {
            result.push_back(*it);
        }
    }

    return result;
}

void message_bus_health_monitor::clear_history() {
    std::lock_guard<std::mutex> lock(history_mutex_);
    health_history_.clear();
}

message_bus_health_monitor::health_status
message_bus_health_monitor::calculate_status(
    const std::vector<std::string>& issues) const {

    size_t critical_count = 0;
    size_t warn_count = 0;

    for (const auto& issue : issues) {
        if (issue.find("CRITICAL") != std::string::npos) {
            ++critical_count;
        } else {
            ++warn_count;
        }
    }

    if (critical_count >= 2) {
        return health_status::critical;
    } else if (critical_count >= 1) {
        return health_status::unhealthy;
    } else if (warn_count >= 2) {
        return health_status::degraded;
    } else if (warn_count >= 1) {
        return health_status::degraded;
    }

    return health_status::healthy;
}

void message_bus_health_monitor::check_queue_saturation(
    health_report& report,
    const message_bus_stats& stats) {

    std::lock_guard<std::mutex> lock(thresholds_mutex_);

    double utilization = stats.queue_utilization_percent / 100.0;

    if (utilization >= thresholds_.queue_saturation_critical) {
        report.issues.push_back(
            "CRITICAL: Queue saturation at " +
            std::to_string(static_cast<int>(stats.queue_utilization_percent)) + "%");
    } else if (utilization >= thresholds_.queue_saturation_warn) {
        report.issues.push_back(
            "WARNING: Queue utilization high at " +
            std::to_string(static_cast<int>(stats.queue_utilization_percent)) + "%");
    }
}

void message_bus_health_monitor::check_failure_rate(
    health_report& report,
    const message_bus_stats& stats) {

    if (stats.messages_published == 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(thresholds_mutex_);

    double failure_rate = static_cast<double>(stats.messages_failed) /
                          static_cast<double>(stats.messages_published);

    if (failure_rate >= thresholds_.failure_rate_critical) {
        report.issues.push_back(
            "CRITICAL: Message failure rate at " +
            std::to_string(static_cast<int>(failure_rate * 100)) + "%");
    } else if (failure_rate >= thresholds_.failure_rate_warn) {
        report.issues.push_back(
            "WARNING: Message failure rate elevated at " +
            std::to_string(static_cast<int>(failure_rate * 100)) + "%");
    }
}

void message_bus_health_monitor::check_latency(
    health_report& report,
    const message_bus_stats& stats) {

    std::lock_guard<std::mutex> lock(thresholds_mutex_);

    if (stats.average_latency_ms >= thresholds_.latency_critical_ms) {
        report.issues.push_back(
            "CRITICAL: Message latency high at " +
            std::to_string(static_cast<int>(stats.average_latency_ms)) + "ms");
    } else if (stats.average_latency_ms >= thresholds_.latency_warn_ms) {
        report.issues.push_back(
            "WARNING: Message latency elevated at " +
            std::to_string(static_cast<int>(stats.average_latency_ms)) + "ms");
    }
}

void message_bus_health_monitor::check_throughput(
    health_report& report,
    const message_bus_stats& stats) {

    std::lock_guard<std::mutex> lock(baseline_mutex_);

    auto it = baseline_throughput_.find(report.bus_name);
    if (it == baseline_throughput_.end()) {
        // Set baseline on first check
        baseline_throughput_[report.bus_name] = stats.throughput_per_second;
        return;
    }

    double baseline = it->second;
    if (baseline <= 0) {
        // Update baseline if it was zero
        baseline_throughput_[report.bus_name] = stats.throughput_per_second;
        return;
    }

    double drop_ratio = 1.0 - (stats.throughput_per_second / baseline);

    std::lock_guard<std::mutex> thresholds_lock(thresholds_mutex_);

    if (drop_ratio >= thresholds_.throughput_drop_critical) {
        report.issues.push_back(
            "CRITICAL: Throughput dropped by " +
            std::to_string(static_cast<int>(drop_ratio * 100)) + "%");
    } else if (drop_ratio >= thresholds_.throughput_drop_warn) {
        report.issues.push_back(
            "WARNING: Throughput dropped by " +
            std::to_string(static_cast<int>(drop_ratio * 100)) + "%");
    }

    // Update baseline with exponential moving average
    baseline_throughput_[report.bus_name] =
        baseline * 0.9 + stats.throughput_per_second * 0.1;
}

}  // namespace kcenon::messaging::collectors

#endif // BUILD_WITH_MONITORING_SYSTEM
