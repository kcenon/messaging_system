#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <variant>

namespace kcenon::monitoring::core {

    // Forward declarations
    class metric;
    class metric_collector;
    class metric_registry;

    // Metric types
    enum class metric_type {
        COUNTER,
        GAUGE,
        HISTOGRAM,
        SUMMARY,
        TIMER
    };

    // Metric value variant
    using metric_value = std::variant<
        int64_t,
        double,
        std::chrono::nanoseconds
    >;

    // Labels for metrics
    using labels = std::unordered_map<std::string, std::string>;

    // Base metric interface
    class metric {
    public:
        virtual ~metric() = default;

        virtual std::string name() const = 0;
        virtual metric_type type() const = 0;
        virtual std::string description() const = 0;
        virtual metric_value value() const = 0;
        virtual labels get_labels() const = 0;
    };

    // Counter metric
    class counter : public metric {
    private:
        std::string name_;
        std::string description_;
        labels labels_;
        std::atomic<int64_t> value_{0};

    public:
        counter(std::string name, std::string description = "", labels labels = {})
            : name_(std::move(name))
            , description_(std::move(description))
            , labels_(std::move(labels)) {}

        void increment(int64_t delta = 1) {
            value_.fetch_add(delta, std::memory_order_relaxed);
        }

        int64_t get() const {
            return value_.load(std::memory_order_relaxed);
        }

        void reset() {
            value_.store(0, std::memory_order_relaxed);
        }

        // metric interface
        std::string name() const override { return name_; }
        metric_type type() const override { return metric_type::COUNTER; }
        std::string description() const override { return description_; }
        metric_value value() const override { return static_cast<int64_t>(get()); }
        labels get_labels() const override { return labels_; }
    };

    // Gauge metric
    class gauge : public metric {
    private:
        std::string name_;
        std::string description_;
        labels labels_;
        std::atomic<double> value_{0.0};

    public:
        gauge(std::string name, std::string description = "", labels labels = {})
            : name_(std::move(name))
            , description_(std::move(description))
            , labels_(std::move(labels)) {}

        void set(double value) {
            value_.store(value, std::memory_order_relaxed);
        }

        void increment(double delta = 1.0) {
            double current = value_.load(std::memory_order_relaxed);
            while (!value_.compare_exchange_weak(current, current + delta,
                                                  std::memory_order_relaxed,
                                                  std::memory_order_relaxed));
        }

        void decrement(double delta = 1.0) {
            increment(-delta);
        }

        double get() const {
            return value_.load(std::memory_order_relaxed);
        }

        // metric interface
        std::string name() const override { return name_; }
        metric_type type() const override { return metric_type::GAUGE; }
        std::string description() const override { return description_; }
        metric_value value() const override { return get(); }
        labels get_labels() const override { return labels_; }
    };

    // Timer metric
    class timer : public metric {
    private:
        std::string name_;
        std::string description_;
        labels labels_;
        std::chrono::steady_clock::time_point start_time_;
        std::chrono::nanoseconds elapsed_{0};
        bool running_{false};
        mutable std::mutex mutex_;

    public:
        timer(std::string name, std::string description = "", labels labels = {})
            : name_(std::move(name))
            , description_(std::move(description))
            , labels_(std::move(labels)) {}

        void start() {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                start_time_ = std::chrono::steady_clock::now();
                running_ = true;
            }
        }

        void stop() {
            std::lock_guard<std::mutex> lock(mutex_);
            if (running_) {
                auto end_time = std::chrono::steady_clock::now();
                elapsed_ += std::chrono::duration_cast<std::chrono::nanoseconds>(
                    end_time - start_time_
                );
                running_ = false;
            }
        }

        std::chrono::nanoseconds elapsed() const {
            std::lock_guard<std::mutex> lock(mutex_);
            if (running_) {
                auto current_time = std::chrono::steady_clock::now();
                return elapsed_ + std::chrono::duration_cast<std::chrono::nanoseconds>(
                    current_time - start_time_
                );
            }
            return elapsed_;
        }

        void reset() {
            std::lock_guard<std::mutex> lock(mutex_);
            elapsed_ = std::chrono::nanoseconds{0};
            running_ = false;
        }

        // metric interface
        std::string name() const override { return name_; }
        metric_type type() const override { return metric_type::TIMER; }
        std::string description() const override { return description_; }
        metric_value value() const override { return elapsed(); }
        labels get_labels() const override { return labels_; }
    };

    // Monitor class - main monitoring interface
    class monitor {
    private:
        std::string name_;
        std::unordered_map<std::string, std::shared_ptr<metric>> metrics_;
        mutable std::mutex metrics_mutex_;
        std::atomic<bool> enabled_{true};

    public:
        explicit monitor(std::string name = "default")
            : name_(std::move(name)) {}

        virtual ~monitor() = default;

        // Enable/disable monitoring
        void enable() { enabled_.store(true); }
        void disable() { enabled_.store(false); }
        bool is_enabled() const { return enabled_.load(); }

        // Create and register metrics
        std::shared_ptr<counter> create_counter(
            const std::string& name,
            const std::string& description = "",
            const labels& labels = {}
        ) {
            if (!is_enabled()) return nullptr;

            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto metric = std::make_shared<counter>(name, description, labels);
            metrics_[name] = metric;
            return metric;
        }

        std::shared_ptr<gauge> create_gauge(
            const std::string& name,
            const std::string& description = "",
            const labels& labels = {}
        ) {
            if (!is_enabled()) return nullptr;

            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto metric = std::make_shared<gauge>(name, description, labels);
            metrics_[name] = metric;
            return metric;
        }

        std::shared_ptr<timer> create_timer(
            const std::string& name,
            const std::string& description = "",
            const labels& labels = {}
        ) {
            if (!is_enabled()) return nullptr;

            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto metric = std::make_shared<timer>(name, description, labels);
            metrics_[name] = metric;
            return metric;
        }

        // Get metric by name
        std::shared_ptr<metric> get_metric(const std::string& name) const {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto it = metrics_.find(name);
            if (it != metrics_.end()) {
                return it->second;
            }
            return nullptr;
        }

        // Get all metrics
        std::vector<std::shared_ptr<metric>> get_all_metrics() const {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            std::vector<std::shared_ptr<metric>> result;
            result.reserve(metrics_.size());
            for (const auto& [name, metric] : metrics_) {
                result.push_back(metric);
            }
            return result;
        }

        // Clear all metrics
        void clear() {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_.clear();
        }

        // Get monitor name
        std::string name() const { return name_; }
    };

    // Global monitor instance
    inline std::shared_ptr<monitor> get_default_monitor() {
        static std::shared_ptr<monitor> instance = std::make_shared<monitor>("global");
        return instance;
    }

} // namespace kcenon::monitoring::core