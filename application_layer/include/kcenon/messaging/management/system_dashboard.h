#pragma once

#include "../core/message_types.h"
#include "../core/production_optimizations.h"
#include "../routing/advanced_router.h"
#include "../cluster/distributed_messaging.h"
#include "../persistence/message_persistence.h"
#include "../security/message_security.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <sstream>
#include <iomanip>

namespace kcenon::messaging::management {

    // System health status
    enum class health_status {
        healthy,
        warning,
        critical,
        unknown
    };

    // Dashboard metrics aggregator
    class metrics_aggregator {
    private:
        mutable std::mutex metrics_mutex_;
        std::unordered_map<std::string, std::atomic<uint64_t>> counters_;
        std::unordered_map<std::string, std::atomic<double>> gauges_;
        std::unordered_map<std::string, std::vector<double>> histograms_;

        static constexpr size_t MAX_HISTOGRAM_SIZE = 1000;

    public:
        void increment_counter(const std::string& name, uint64_t value = 1) {
            counters_[name] += value;
        }

        void set_gauge(const std::string& name, double value) {
            gauges_[name] = value;
        }

        void record_histogram(const std::string& name, double value) {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto& hist = histograms_[name];
            hist.push_back(value);

            if (hist.size() > MAX_HISTOGRAM_SIZE) {
                hist.erase(hist.begin());
            }
        }

        uint64_t get_counter(const std::string& name) const {
            auto it = counters_.find(name);
            return it != counters_.end() ? it->second.load() : 0;
        }

        double get_gauge(const std::string& name) const {
            auto it = gauges_.find(name);
            return it != gauges_.end() ? it->second.load() : 0.0;
        }

        struct histogram_stats {
            double min = 0.0;
            double max = 0.0;
            double avg = 0.0;
            double p95 = 0.0;
            double p99 = 0.0;
            size_t count = 0;
        };

        histogram_stats get_histogram_stats(const std::string& name) const {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto it = histograms_.find(name);
            if (it == histograms_.end() || it->second.empty()) {
                return {};
            }

            const auto& values = it->second;
            auto sorted_values = values;
            std::sort(sorted_values.begin(), sorted_values.end());

            histogram_stats stats;
            stats.count = sorted_values.size();
            stats.min = sorted_values.front();
            stats.max = sorted_values.back();

            double sum = 0.0;
            for (double v : sorted_values) sum += v;
            stats.avg = sum / sorted_values.size();

            size_t p95_idx = static_cast<size_t>(0.95 * sorted_values.size());
            size_t p99_idx = static_cast<size_t>(0.99 * sorted_values.size());

            stats.p95 = sorted_values[std::min(p95_idx, sorted_values.size() - 1)];
            stats.p99 = sorted_values[std::min(p99_idx, sorted_values.size() - 1)];

            return stats;
        }

        std::vector<std::string> get_all_metric_names() const {
            std::vector<std::string> names;

            for (const auto& [name, counter] : counters_) {
                names.push_back("counter:" + name);
            }
            for (const auto& [name, gauge] : gauges_) {
                names.push_back("gauge:" + name);
            }

            std::lock_guard<std::mutex> lock(metrics_mutex_);
            for (const auto& [name, hist] : histograms_) {
                names.push_back("histogram:" + name);
            }

            return names;
        }

        void reset() {
            for (auto& [name, counter] : counters_) {
                counter = 0;
            }
            for (auto& [name, gauge] : gauges_) {
                gauge = 0.0;
            }

            std::lock_guard<std::mutex> lock(metrics_mutex_);
            histograms_.clear();
        }
    };

    // System health monitor
    class system_health_monitor {
    private:
        health_status overall_status_ = health_status::unknown;
        std::unordered_map<std::string, health_status> component_status_;
        std::unordered_map<std::string, std::string> component_messages_;
        mutable std::shared_mutex health_mutex_;

        // Health thresholds
        static constexpr double CPU_WARNING_THRESHOLD = 70.0;
        static constexpr double CPU_CRITICAL_THRESHOLD = 90.0;
        static constexpr double MEMORY_WARNING_THRESHOLD = 80.0;
        static constexpr double MEMORY_CRITICAL_THRESHOLD = 95.0;
        static constexpr uint64_t QUEUE_WARNING_THRESHOLD = 10000;
        static constexpr uint64_t QUEUE_CRITICAL_THRESHOLD = 50000;

    public:
        void update_component_health(const std::string& component, health_status status,
                                   const std::string& message = "") {
            std::unique_lock<std::shared_mutex> lock(health_mutex_);
            component_status_[component] = status;
            component_messages_[component] = message;

            // Update overall status
            overall_status_ = health_status::healthy;
            for (const auto& [name, comp_status] : component_status_) {
                if (comp_status == health_status::critical) {
                    overall_status_ = health_status::critical;
                    break;
                } else if (comp_status == health_status::warning &&
                          overall_status_ != health_status::critical) {
                    overall_status_ = health_status::warning;
                }
            }
        }

        health_status get_overall_health() const {
            std::shared_lock<std::shared_mutex> lock(health_mutex_);
            return overall_status_;
        }

        struct component_health_info {
            health_status status;
            std::string message;
            std::chrono::system_clock::time_point last_updated;
        };

        std::unordered_map<std::string, component_health_info> get_component_health() const {
            std::shared_lock<std::shared_mutex> lock(health_mutex_);
            std::unordered_map<std::string, component_health_info> result;

            for (const auto& [name, status] : component_status_) {
                component_health_info info;
                info.status = status;
                info.last_updated = std::chrono::system_clock::now();

                auto msg_it = component_messages_.find(name);
                if (msg_it != component_messages_.end()) {
                    info.message = msg_it->second;
                }

                result[name] = info;
            }

            return result;
        }

        void evaluate_system_metrics(const metrics_aggregator& metrics) {
            // Check CPU usage
            double cpu_usage = metrics.get_gauge("system.cpu_usage_percent");
            if (cpu_usage >= CPU_CRITICAL_THRESHOLD) {
                update_component_health("CPU", health_status::critical,
                    "CPU usage critical: " + std::to_string(cpu_usage) + "%");
            } else if (cpu_usage >= CPU_WARNING_THRESHOLD) {
                update_component_health("CPU", health_status::warning,
                    "CPU usage high: " + std::to_string(cpu_usage) + "%");
            } else {
                update_component_health("CPU", health_status::healthy);
            }

            // Check memory usage
            double memory_usage = metrics.get_gauge("system.memory_usage_percent");
            if (memory_usage >= MEMORY_CRITICAL_THRESHOLD) {
                update_component_health("Memory", health_status::critical,
                    "Memory usage critical: " + std::to_string(memory_usage) + "%");
            } else if (memory_usage >= MEMORY_WARNING_THRESHOLD) {
                update_component_health("Memory", health_status::warning,
                    "Memory usage high: " + std::to_string(memory_usage) + "%");
            } else {
                update_component_health("Memory", health_status::healthy);
            }

            // Check queue sizes
            uint64_t queue_size = metrics.get_counter("message_bus.queue_size");
            if (queue_size >= QUEUE_CRITICAL_THRESHOLD) {
                update_component_health("MessageQueue", health_status::critical,
                    "Queue size critical: " + std::to_string(queue_size));
            } else if (queue_size >= QUEUE_WARNING_THRESHOLD) {
                update_component_health("MessageQueue", health_status::warning,
                    "Queue size high: " + std::to_string(queue_size));
            } else {
                update_component_health("MessageQueue", health_status::healthy);
            }
        }
    };

    // Real-time dashboard data
    struct dashboard_snapshot {
        std::chrono::system_clock::time_point timestamp;
        health_status overall_health;

        // System metrics
        uint64_t total_messages_processed;
        uint64_t messages_per_second;
        double average_latency_ms;
        uint64_t active_connections;
        double cpu_usage_percent;
        double memory_usage_percent;

        // Component statistics
        routing::advanced_router::routing_statistics routing_stats;
        cluster::distributed_broker::cluster_statistics cluster_stats;
        security::security_manager::security_statistics security_stats;

        // Health information
        std::unordered_map<std::string, system_health_monitor::component_health_info> component_health;

        // Performance metrics
        metrics_aggregator::histogram_stats latency_histogram;
        metrics_aggregator::histogram_stats throughput_histogram;
    };

    // Management command interface
    class management_command_processor {
    public:
        enum class command_result {
            success,
            invalid_command,
            invalid_parameters,
            permission_denied,
            execution_failed
        };

        struct command_response {
            command_result result;
            std::string message;
            std::unordered_map<std::string, std::string> data;
        };

        virtual ~management_command_processor() = default;
        virtual command_response execute_command(const std::string& command,
                                               const std::vector<std::string>& parameters,
                                               const std::string& user_token) = 0;
        virtual std::vector<std::string> get_available_commands() const = 0;
        virtual std::string get_command_help(const std::string& command) const = 0;
    };

    // System management commands
    class system_management_commands : public management_command_processor {
    private:
        std::function<metrics_aggregator*()> get_metrics_;
        std::function<system_health_monitor*()> get_health_monitor_;
        std::function<security::security_manager*()> get_security_manager_;

    public:
        system_management_commands(
            std::function<metrics_aggregator*()> get_metrics,
            std::function<system_health_monitor*()> get_health_monitor,
            std::function<security::security_manager*()> get_security_manager)
            : get_metrics_(std::move(get_metrics)),
              get_health_monitor_(std::move(get_health_monitor)),
              get_security_manager_(std::move(get_security_manager)) {}

        command_response execute_command(const std::string& command,
                                       const std::vector<std::string>& parameters,
                                       const std::string& user_token) override {
            // Basic authentication check
            if (get_security_manager_) {
                auto security_mgr = get_security_manager_();
                if (security_mgr && security_mgr->get_auth_manager()->verify_token(user_token)
                    != security::auth_result::success) {
                    return {command_result::permission_denied, "Invalid or expired token"};
                }
            }

            if (command == "status") {
                return handle_status_command();
            } else if (command == "metrics") {
                return handle_metrics_command(parameters);
            } else if (command == "health") {
                return handle_health_command();
            } else if (command == "reset-metrics") {
                return handle_reset_metrics_command();
            } else if (command == "help") {
                return handle_help_command(parameters);
            }

            return {command_result::invalid_command, "Unknown command: " + command};
        }

        std::vector<std::string> get_available_commands() const override {
            return {"status", "metrics", "health", "reset-metrics", "help"};
        }

        std::string get_command_help(const std::string& command) const override {
            if (command == "status") {
                return "status - Display overall system status";
            } else if (command == "metrics") {
                return "metrics [metric_name] - Display system metrics";
            } else if (command == "health") {
                return "health - Display component health information";
            } else if (command == "reset-metrics") {
                return "reset-metrics - Reset all collected metrics";
            } else if (command == "help") {
                return "help [command] - Display help information";
            }
            return "Unknown command";
        }

    private:
        command_response handle_status_command() {
            command_response response;
            response.result = command_result::success;
            response.message = "System status retrieved successfully";

            if (get_health_monitor_) {
                auto health_monitor = get_health_monitor_();
                if (health_monitor) {
                    auto health = health_monitor->get_overall_health();
                    response.data["overall_health"] = health_status_to_string(health);
                }
            }

            if (get_metrics_) {
                auto metrics = get_metrics_();
                if (metrics) {
                    response.data["total_messages"] = std::to_string(metrics->get_counter("messages.total"));
                    response.data["messages_per_second"] = std::to_string(metrics->get_gauge("messages.per_second"));
                }
            }

            return response;
        }

        command_response handle_metrics_command(const std::vector<std::string>& parameters) {
            command_response response;
            response.result = command_result::success;
            response.message = "Metrics retrieved successfully";

            if (!get_metrics_) {
                response.result = command_result::execution_failed;
                response.message = "Metrics not available";
                return response;
            }

            auto metrics = get_metrics_();
            if (!metrics) {
                response.result = command_result::execution_failed;
                response.message = "Metrics aggregator not available";
                return response;
            }

            if (parameters.empty()) {
                // Return all metrics
                auto names = metrics->get_all_metric_names();
                for (const auto& name : names) {
                    if (name.starts_with("counter:")) {
                        auto counter_name = name.substr(8);
                        response.data[name] = std::to_string(metrics->get_counter(counter_name));
                    } else if (name.starts_with("gauge:")) {
                        auto gauge_name = name.substr(6);
                        response.data[name] = std::to_string(metrics->get_gauge(gauge_name));
                    }
                }
            } else {
                // Return specific metric
                const auto& metric_name = parameters[0];
                response.data[metric_name] = std::to_string(metrics->get_counter(metric_name));
            }

            return response;
        }

        command_response handle_health_command() {
            command_response response;
            response.result = command_result::success;
            response.message = "Health information retrieved successfully";

            if (get_health_monitor_) {
                auto health_monitor = get_health_monitor_();
                if (health_monitor) {
                    auto component_health = health_monitor->get_component_health();
                    for (const auto& [component, health_info] : component_health) {
                        response.data[component + "_status"] = health_status_to_string(health_info.status);
                        response.data[component + "_message"] = health_info.message;
                    }
                }
            }

            return response;
        }

        command_response handle_reset_metrics_command() {
            command_response response;
            response.result = command_result::success;
            response.message = "Metrics reset successfully";

            if (get_metrics_) {
                auto metrics = get_metrics_();
                if (metrics) {
                    metrics->reset();
                } else {
                    response.result = command_result::execution_failed;
                    response.message = "Metrics aggregator not available";
                }
            } else {
                response.result = command_result::execution_failed;
                response.message = "Metrics not available";
            }

            return response;
        }

        command_response handle_help_command(const std::vector<std::string>& parameters) {
            command_response response;
            response.result = command_result::success;

            if (parameters.empty()) {
                response.message = "Available commands: ";
                auto commands = get_available_commands();
                for (const auto& cmd : commands) {
                    response.message += cmd + " ";
                }
            } else {
                response.message = get_command_help(parameters[0]);
            }

            return response;
        }

        std::string health_status_to_string(health_status status) const {
            switch (status) {
                case health_status::healthy: return "healthy";
                case health_status::warning: return "warning";
                case health_status::critical: return "critical";
                case health_status::unknown: return "unknown";
                default: return "unknown";
            }
        }
    };

    // Real-time dashboard manager
    class dashboard_manager {
    private:
        std::unique_ptr<metrics_aggregator> metrics_;
        std::unique_ptr<system_health_monitor> health_monitor_;
        std::unique_ptr<management_command_processor> command_processor_;

        // Dashboard update thread
        std::atomic<bool> running_{false};
        std::thread update_thread_;
        std::condition_variable update_cv_;
        std::mutex update_mutex_;
        std::chrono::seconds update_interval_{5};

        // Dashboard subscribers
        std::vector<std::function<void(const dashboard_snapshot&)>> subscribers_;
        mutable std::mutex subscribers_mutex_;

        // Latest snapshot
        dashboard_snapshot latest_snapshot_;
        mutable std::shared_mutex snapshot_mutex_;

    public:
        dashboard_manager()
            : metrics_(std::make_unique<metrics_aggregator>()),
              health_monitor_(std::make_unique<system_health_monitor>()) {

            command_processor_ = std::make_unique<system_management_commands>(
                [this]() { return metrics_.get(); },
                [this]() { return health_monitor_.get(); },
                []() -> security::security_manager* { return nullptr; }
            );
        }

        ~dashboard_manager() {
            stop();
        }

        void start(std::chrono::seconds interval = std::chrono::seconds(5)) {
            if (running_) return;

            update_interval_ = interval;
            running_ = true;
            update_thread_ = std::thread([this]() { update_loop(); });
        }

        void stop() {
            if (!running_) return;

            running_ = false;
            update_cv_.notify_all();
            if (update_thread_.joinable()) {
                update_thread_.join();
            }
        }

        void subscribe_to_updates(std::function<void(const dashboard_snapshot&)> callback) {
            std::lock_guard<std::mutex> lock(subscribers_mutex_);
            subscribers_.push_back(std::move(callback));
        }

        dashboard_snapshot get_latest_snapshot() const {
            std::shared_lock<std::shared_mutex> lock(snapshot_mutex_);
            return latest_snapshot_;
        }

        metrics_aggregator* get_metrics() { return metrics_.get(); }
        system_health_monitor* get_health_monitor() { return health_monitor_.get(); }
        management_command_processor* get_command_processor() { return command_processor_.get(); }

    private:
        void update_loop() {
            while (running_) {
                update_dashboard();

                std::unique_lock<std::mutex> lock(update_mutex_);
                if (update_cv_.wait_for(lock, update_interval_, [this] { return !running_; })) {
                    break;
                }
            }
        }

        void update_dashboard() {
            dashboard_snapshot snapshot;
            snapshot.timestamp = std::chrono::system_clock::now();

            // Update health evaluation
            health_monitor_->evaluate_system_metrics(*metrics_);

            // Collect basic metrics
            snapshot.total_messages_processed = metrics_->get_counter("messages.total");
            snapshot.messages_per_second = static_cast<uint64_t>(metrics_->get_gauge("messages.per_second"));
            snapshot.average_latency_ms = metrics_->get_gauge("messages.avg_latency_ms");
            snapshot.active_connections = metrics_->get_counter("connections.active");
            snapshot.cpu_usage_percent = metrics_->get_gauge("system.cpu_usage_percent");
            snapshot.memory_usage_percent = metrics_->get_gauge("system.memory_usage_percent");

            // Collect health information
            snapshot.overall_health = health_monitor_->get_overall_health();
            snapshot.component_health = health_monitor_->get_component_health();

            // Collect histogram statistics
            snapshot.latency_histogram = metrics_->get_histogram_stats("latency_ms");
            snapshot.throughput_histogram = metrics_->get_histogram_stats("throughput");

            // Update latest snapshot
            {
                std::unique_lock<std::shared_mutex> lock(snapshot_mutex_);
                latest_snapshot_ = snapshot;
            }

            // Notify subscribers
            std::lock_guard<std::mutex> lock(subscribers_mutex_);
            for (const auto& subscriber : subscribers_) {
                try {
                    subscriber(snapshot);
                } catch (const std::exception&) {
                    // Continue with other subscribers
                }
            }
        }
    };

    // Dashboard factory for easy setup
    class dashboard_factory {
    public:
        static std::unique_ptr<dashboard_manager> create_dashboard() {
            return std::make_unique<dashboard_manager>();
        }

        static std::unique_ptr<dashboard_manager> create_production_dashboard() {
            auto dashboard = create_dashboard();

            // Set up production monitoring
            dashboard->subscribe_to_updates([](const dashboard_snapshot& snapshot) {
                if (snapshot.overall_health == health_status::critical) {
                    // In production, this would trigger alerts
                }
            });

            return dashboard;
        }
    };

} // namespace kcenon::messaging::management