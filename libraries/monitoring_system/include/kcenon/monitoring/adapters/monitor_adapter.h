#pragma once

#include <kcenon/thread/interfaces/shared_interfaces.h>
#include <kcenon/monitoring/core/performance_monitor.h>
#include <memory>
#include <chrono>

namespace monitoring_system::adapters {

/**
 * @brief Adapter to make performance_monitor compatible with IMonitorable interface
 */
class monitor_adapter : public shared::IMonitorable, public shared::IService {
public:
    /**
     * @brief Constructor with performance monitor instance
     * @param monitor Performance monitor instance
     */
    explicit monitor_adapter(std::shared_ptr<monitoring_system::performance_monitor> monitor)
        : monitor_(std::move(monitor)) {
    }

    /**
     * @brief Default constructor - creates a default monitor
     */
    monitor_adapter() {
        // Create default monitor configuration
        monitoring_system::performance_monitor::config config;
        config.enable_cpu_monitoring = true;
        config.enable_memory_monitoring = true;
        config.sampling_interval_ms = 1000;
        monitor_ = std::make_shared<monitoring_system::performance_monitor>(config);
    }

    // IMonitorable interface
    shared::MetricsSnapshot get_metrics() const override {
        shared::MetricsSnapshot snapshot;
        snapshot.timestamp = std::chrono::steady_clock::now();

        if (monitor_) {
            auto sys_metrics = monitor_->get_system_metrics();

            snapshot.cpu_usage = sys_metrics.cpu_usage_percent;
            snapshot.memory_usage_mb = sys_metrics.memory_usage_mb;

            // Get thread metrics if available
            if (sys_metrics.thread_count > 0) {
                snapshot.active_threads = sys_metrics.thread_count;
            }

            // Get additional metrics from performance data
            auto perf_data = monitor_->get_performance_data();
            if (!perf_data.profiles.empty()) {
                // Calculate average task duration from profiles
                double total_duration = 0.0;
                size_t count = 0;
                for (const auto& [name, profile] : perf_data.profiles) {
                    if (profile.average_duration_ms > 0) {
                        total_duration += profile.average_duration_ms;
                        count++;
                    }
                }
                if (count > 0) {
                    snapshot.average_task_duration_ms = total_duration / count;
                }
            }
        }

        return snapshot;
    }

    void set_metrics_enabled(bool enabled) override {
        metrics_enabled_ = enabled;
        if (monitor_) {
            if (enabled) {
                monitor_->start();
            } else {
                monitor_->stop();
            }
        }
    }

    // IService interface
    bool initialize() override {
        if (monitor_) {
            monitor_->start();
            is_running_ = true;
            return true;
        }
        return false;
    }

    void shutdown() override {
        if (monitor_) {
            monitor_->stop();
        }
        is_running_ = false;
    }

    bool is_running() const override {
        return is_running_ && monitor_ != nullptr;
    }

    std::string name() const override {
        return "MonitorAdapter";
    }

    /**
     * @brief Get the underlying performance monitor
     * @return Performance monitor instance
     */
    std::shared_ptr<monitoring_system::performance_monitor> get_monitor() const {
        return monitor_;
    }

    /**
     * @brief Set monitoring configuration
     * @param config Monitor configuration
     */
    void set_config(const monitoring_system::performance_monitor::config& config) {
        if (monitor_) {
            // Stop, reconfigure, and restart if running
            bool was_running = is_running_;
            if (was_running) {
                monitor_->stop();
            }

            monitor_ = std::make_shared<monitoring_system::performance_monitor>(config);

            if (was_running) {
                monitor_->start();
            }
        }
    }

private:
    std::shared_ptr<monitoring_system::performance_monitor> monitor_;
    bool metrics_enabled_{true};
    bool is_running_{false};
};

} // namespace monitoring_system::adapters