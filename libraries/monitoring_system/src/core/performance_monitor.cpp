/**
 * @file performance_monitor.cpp
 * @brief Performance monitoring implementation
 */

#include <kcenon/monitoring/core/performance_monitor.h>
#include <shared_mutex>

namespace monitoring_system {

result<bool> performance_profiler::record_sample(
    const std::string& operation_name,
    std::chrono::nanoseconds duration,
    bool success) {

    if (!enabled_) {
        return result<bool>(true);
    }

    std::unique_lock<std::shared_mutex> lock(profiles_mutex_);

    auto& profile = profiles_[operation_name];
    if (!profile) {
        profile = std::make_unique<profile_data>();
    }

    lock.unlock();

    // Update counters
    profile->call_count.fetch_add(1);
    if (!success) {
        profile->error_count.fetch_add(1);
    }

    // Record sample
    std::lock_guard sample_lock(profile->mutex);

    // Limit samples to prevent unbounded growth
    if (profile->samples.size() >= max_samples_per_operation_) {
        // Remove oldest sample (simple ring buffer behavior)
        profile->samples.erase(profile->samples.begin());
    }

    profile->samples.push_back(duration);

    return monitoring_system::result<bool>(true);
}

monitoring_system::result<monitoring_system::performance_metrics> monitoring_system::performance_profiler::get_metrics(
    const std::string& operation_name) const {

    std::shared_lock<std::shared_mutex> lock(profiles_mutex_);

    auto it = profiles_.find(operation_name);
    if (it == profiles_.end()) {
        return monitoring_system::make_error<monitoring_system::performance_metrics>(
            monitoring_system::monitoring_error_code::not_found,
            "Operation not found: " + operation_name
        );
    }

    const auto& profile = it->second;

    std::lock_guard sample_lock(profile->mutex);

    monitoring_system::performance_metrics metrics;
    metrics.operation_name = operation_name;
    metrics.call_count = profile->call_count.load();
    metrics.error_count = profile->error_count.load();

    if (!profile->samples.empty()) {
        auto total = std::chrono::nanoseconds::zero();
        auto min_sample = profile->samples[0];
        auto max_sample = profile->samples[0];

        for (const auto& sample : profile->samples) {
            total += sample;
            if (sample < min_sample) min_sample = sample;
            if (sample > max_sample) max_sample = sample;
        }

        metrics.min_duration = min_sample;
        metrics.max_duration = max_sample;
        metrics.mean_duration = total / profile->samples.size();
        metrics.call_count = profile->samples.size();
    }

    return result<performance_metrics>(metrics);
}

// system_monitor implementation
struct system_monitor::monitor_impl {
    std::atomic<bool> monitoring{false};
    std::thread monitor_thread;
    std::vector<system_metrics> history;
    mutable std::mutex history_mutex;
    std::chrono::milliseconds interval{1000};

    ~monitor_impl() {
        if (monitoring) {
            monitoring = false;
            if (monitor_thread.joinable()) {
                monitor_thread.join();
            }
        }
    }
};

system_monitor::system_monitor() : impl_(std::make_unique<monitor_impl>()) {}

system_monitor::~system_monitor() = default;

system_monitor::system_monitor(system_monitor&&) noexcept = default;
system_monitor& system_monitor::operator=(system_monitor&&) noexcept = default;

result<system_metrics> system_monitor::get_current_metrics() const {
    system_metrics metrics;
    metrics.timestamp = std::chrono::system_clock::now();

    // Stub implementation - return dummy values
    metrics.cpu_usage_percent = 10.0;
    metrics.memory_usage_percent = 25.0;
    metrics.memory_usage_bytes = 1024 * 1024 * 100; // 100MB
    metrics.available_memory_bytes = 1024 * 1024 * 500; // 500MB
    metrics.thread_count = 10;
    metrics.handle_count = 50;

    return make_success(metrics);
}

result<bool> system_monitor::start_monitoring(std::chrono::milliseconds interval) {
    if (impl_->monitoring) {
        return make_success(true);
    }

    impl_->interval = interval;
    impl_->monitoring = true;

    return make_success(true);
}

result<bool> system_monitor::stop_monitoring() {
    if (!impl_->monitoring) {
        return make_success(true);
    }

    impl_->monitoring = false;
    if (impl_->monitor_thread.joinable()) {
        impl_->monitor_thread.join();
    }

    return make_success(true);
}

bool system_monitor::is_monitoring() const {
    return impl_->monitoring;
}

std::vector<system_metrics> system_monitor::get_history(std::chrono::seconds duration) const {
    (void)duration; // Suppress unused parameter warning
    std::lock_guard<std::mutex> lock(impl_->history_mutex);
    return impl_->history; // Simplified stub
}

// performance_monitor additional methods
result<metrics_snapshot> performance_monitor::collect() {
    metrics_snapshot snapshot;
    snapshot.capture_time = std::chrono::system_clock::now();
    snapshot.source_id = name_;

    // Add system metrics
    auto system_metrics_result = system_monitor_.get_current_metrics();
    if (system_metrics_result) {
        auto& sys_metrics = system_metrics_result.value();

        // Convert system metrics to metric_value format
        snapshot.add_metric("cpu_usage", sys_metrics.cpu_usage_percent);
        snapshot.add_metric("memory_usage", sys_metrics.memory_usage_percent);
        snapshot.add_metric("memory_bytes", static_cast<double>(sys_metrics.memory_usage_bytes));
        snapshot.add_metric("thread_count", static_cast<double>(sys_metrics.thread_count));
    }

    return make_success(snapshot);
}

result<bool> performance_monitor::check_thresholds() const {
    return make_success(true); // Stub implementation
}

// performance_profiler additional methods
std::vector<performance_metrics> performance_profiler::get_all_metrics() const {
    std::vector<performance_metrics> result;
    std::shared_lock<std::shared_mutex> lock(profiles_mutex_);

    for (const auto& [name, profile] : profiles_) {
        auto metrics_result = get_metrics(name);
        if (metrics_result) {
            result.push_back(metrics_result.value());
        }
    }

    return result;
}

result<bool> performance_profiler::clear_samples(const std::string& operation_name) {
    std::unique_lock<std::shared_mutex> lock(profiles_mutex_);

    auto it = profiles_.find(operation_name);
    if (it != profiles_.end()) {
        std::lock_guard<std::mutex> data_lock(it->second->mutex);
        it->second->samples.clear();
        it->second->call_count = 0;
        it->second->error_count = 0;
    }

    return make_success(true);
}

void performance_profiler::clear_all_samples() {
    std::unique_lock<std::shared_mutex> lock(profiles_mutex_);

    for (auto& [name, profile] : profiles_) {
        std::lock_guard<std::mutex> data_lock(profile->mutex);
        profile->samples.clear();
        profile->call_count = 0;
        profile->error_count = 0;
    }
}

// Global instance
performance_monitor& global_performance_monitor() {
    static performance_monitor instance;
    return instance;
}

} // namespace monitoring_system