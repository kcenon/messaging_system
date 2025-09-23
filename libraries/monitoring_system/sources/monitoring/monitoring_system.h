#pragma once

#include "core/monitor.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>

namespace monitoring {

    using namespace kcenon::monitoring::core;

    // Main monitoring system class
    class monitoring_system {
    private:
        std::shared_ptr<monitor> monitor_;
        std::atomic<bool> running_{false};
        std::thread export_thread_;
        std::chrono::seconds export_interval_{60};

        // Metric storage
        std::unordered_map<std::string, std::shared_ptr<counter>> counters_;
        std::unordered_map<std::string, std::shared_ptr<gauge>> gauges_;
        std::unordered_map<std::string, std::shared_ptr<timer>> timers_;
        mutable std::mutex metrics_mutex_;

        // Export callbacks
        using export_callback = std::function<void(const std::vector<std::shared_ptr<metric>>&)>;
        std::vector<export_callback> export_callbacks_;

    public:
        monitoring_system()
            : monitor_(std::make_shared<monitor>("system")) {}

        ~monitoring_system() {
            shutdown();
        }

        // Initialize the monitoring system
        bool initialize() {
            if (!running_.load()) {
                running_.store(true);
                start_export_thread();
                return true;
            }
            return false;
        }

        // Shutdown the monitoring system
        void shutdown() {
            if (running_.load()) {
                running_.store(false);
                if (export_thread_.joinable()) {
                    export_thread_.join();
                }
            }
        }

        // Check if system is running
        bool is_running() const {
            return running_.load();
        }

        // Record a metric value
        void record_metric(const std::string& name, double value,
                          const std::unordered_map<std::string, std::string>& labels = {}) {
            if (!is_running()) return;

            std::lock_guard<std::mutex> lock(metrics_mutex_);

            // Check if it's a counter
            auto counter_it = counters_.find(name);
            if (counter_it != counters_.end()) {
                counter_it->second->increment(static_cast<int64_t>(value));
                return;
            }

            // Check if it's a gauge
            auto gauge_it = gauges_.find(name);
            if (gauge_it != gauges_.end()) {
                gauge_it->second->set(value);
                return;
            }

            // If metric doesn't exist, create a gauge by default
            auto new_gauge = monitor_->create_gauge(name, "", labels);
            if (new_gauge) {
                new_gauge->set(value);
                gauges_[name] = new_gauge;
            }
        }

        // Create a counter metric
        std::shared_ptr<counter> create_counter(const std::string& name,
                                               const std::string& description = "",
                                               const std::unordered_map<std::string, std::string>& labels = {}) {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto metric = monitor_->create_counter(name, description, labels);
            if (metric) {
                counters_[name] = metric;
            }
            return metric;
        }

        // Create a gauge metric
        std::shared_ptr<gauge> create_gauge(const std::string& name,
                                           const std::string& description = "",
                                           const std::unordered_map<std::string, std::string>& labels = {}) {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto metric = monitor_->create_gauge(name, description, labels);
            if (metric) {
                gauges_[name] = metric;
            }
            return metric;
        }

        // Create a timer metric
        std::shared_ptr<timer> create_timer(const std::string& name,
                                           const std::string& description = "",
                                           const std::unordered_map<std::string, std::string>& labels = {}) {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto metric = monitor_->create_timer(name, description, labels);
            if (metric) {
                timers_[name] = metric;
            }
            return metric;
        }

        // Get a specific metric
        std::shared_ptr<metric> get_metric(const std::string& name) const {
            return monitor_->get_metric(name);
        }

        // Get all metrics
        std::vector<std::shared_ptr<metric>> get_all_metrics() const {
            return monitor_->get_all_metrics();
        }

        // Register an export callback
        void register_exporter(export_callback callback) {
            export_callbacks_.push_back(std::move(callback));
        }

        // Set export interval
        void set_export_interval(std::chrono::seconds interval) {
            export_interval_ = interval;
        }

        // Manually trigger export
        void export_metrics() {
            auto metrics = get_all_metrics();
            for (const auto& callback : export_callbacks_) {
                callback(metrics);
            }
        }

    private:
        void start_export_thread() {
            export_thread_ = std::thread([this]() {
                while (running_.load()) {
                    std::this_thread::sleep_for(export_interval_);
                    if (running_.load()) {
                        export_metrics();
                    }
                }
            });
        }
    };

    // Global monitoring system instance
    inline std::shared_ptr<monitoring_system> get_default_monitoring_system() {
        static std::shared_ptr<monitoring_system> instance = std::make_shared<monitoring_system>();
        return instance;
    }

} // namespace monitoring