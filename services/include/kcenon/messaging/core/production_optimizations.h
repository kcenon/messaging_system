#pragma once

#include "config.h"
#include "message_types.h"
#include <memory>
#include <chrono>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <queue>

namespace kcenon::messaging::core {

    // Production performance metrics
    struct production_metrics {
        std::atomic<uint64_t> total_messages_processed{0};
        std::atomic<uint64_t> messages_per_second{0};
        std::atomic<uint64_t> peak_queue_size{0};
        std::atomic<uint64_t> memory_usage_bytes{0};
        std::atomic<double> average_latency_ms{0.0};
        std::atomic<uint64_t> connection_pool_hits{0};
        std::atomic<uint64_t> connection_pool_misses{0};
        std::chrono::steady_clock::time_point last_reset;

        production_metrics() : last_reset(std::chrono::steady_clock::now()) {}

        void reset() {
            total_messages_processed = 0;
            messages_per_second = 0;
            peak_queue_size = 0;
            memory_usage_bytes = 0;
            average_latency_ms = 0.0;
            connection_pool_hits = 0;
            connection_pool_misses = 0;
            last_reset = std::chrono::steady_clock::now();
        }

        double get_uptime_hours() const {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::hours>(now - last_reset);
            return duration.count();
        }
    };

    // Memory pool for message objects
    template<typename T>
    class object_pool {
    private:
        std::queue<std::unique_ptr<T>> available_objects_;
        std::mutex pool_mutex_;
        size_t max_size_;
        std::atomic<size_t> total_created_{0};
        std::atomic<size_t> pool_hits_{0};

    public:
        explicit object_pool(size_t max_size = 1000) : max_size_(max_size) {}

        std::unique_ptr<T> acquire() {
            std::lock_guard<std::mutex> lock(pool_mutex_);
            if (!available_objects_.empty()) {
                auto obj = std::move(available_objects_.front());
                available_objects_.pop();
                pool_hits_++;
                return obj;
            }

            // Create new object if pool is empty
            total_created_++;
            return std::make_unique<T>();
        }

        void release(std::unique_ptr<T> obj) {
            if (!obj) return;

            std::lock_guard<std::mutex> lock(pool_mutex_);
            if (available_objects_.size() < max_size_) {
                // Reset object to clean state
                *obj = T{};
                available_objects_.push(std::move(obj));
            }
            // If pool is full, object will be automatically destroyed
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(pool_mutex_);
            return available_objects_.size();
        }

        size_t total_created() const { return total_created_; }
        size_t pool_hits() const { return pool_hits_; }
        double hit_ratio() const {
            auto hits = pool_hits_.load();
            auto total = total_created_.load();
            return total > 0 ? static_cast<double>(hits) / total : 0.0;
        }
    };

    // Production-optimized message allocator
    class production_message_allocator {
    private:
        object_pool<message> message_pool_;
        object_pool<message_payload> payload_pool_;

    public:
        production_message_allocator(size_t message_pool_size = 1000, size_t payload_pool_size = 1000)
            : message_pool_(message_pool_size), payload_pool_(payload_pool_size) {}

        std::unique_ptr<message> allocate_message() {
            return message_pool_.acquire();
        }

        void deallocate_message(std::unique_ptr<message> msg) {
            message_pool_.release(std::move(msg));
        }

        std::unique_ptr<message_payload> allocate_payload() {
            return payload_pool_.acquire();
        }

        void deallocate_payload(std::unique_ptr<message_payload> payload) {
            payload_pool_.release(std::move(payload));
        }

        struct allocator_stats {
            size_t message_pool_size;
            size_t payload_pool_size;
            size_t messages_created;
            size_t payloads_created;
            double message_hit_ratio;
            double payload_hit_ratio;
        };

        allocator_stats get_stats() const {
            return {
                message_pool_.size(),
                payload_pool_.size(),
                message_pool_.total_created(),
                payload_pool_.total_created(),
                message_pool_.hit_ratio(),
                payload_pool_.hit_ratio()
            };
        }
    };

    // Production configuration optimization
    class production_config_optimizer {
    public:
        static config::messaging_config optimize_for_production(const config::messaging_config& base_config) {
            auto optimized = base_config;

            // Optimize message bus settings for production
            optimized.message_bus.max_queue_size = std::max(optimized.message_bus.max_queue_size, static_cast<size_t>(100000));
            optimized.message_bus.worker_threads = std::max(optimized.message_bus.worker_threads,
                static_cast<size_t>(std::thread::hardware_concurrency() * 2));

            // Enable all performance features
            optimized.message_bus.enable_priority_queue = true;
            optimized.message_bus.enable_batching = true;
            optimized.message_bus.batch_size = 100;
            optimized.message_bus.batch_timeout = std::chrono::milliseconds(10);

            // Enable compression for network efficiency
            optimized.container.enable_compression = true;

            // Optimize monitoring for production
            optimized.monitoring.enable = true;
            optimized.monitoring.collection_interval = std::chrono::seconds(30);
            optimized.monitoring.enable_metrics_export = true;

            // Set production environment
            optimized.environment = "production";

            return optimized;
        }

        static config::messaging_config optimize_for_development(const config::messaging_config& base_config) {
            auto optimized = base_config;

            // Smaller queues for development
            optimized.message_bus.max_queue_size = 10000;
            optimized.message_bus.worker_threads = 2;

            // Disable compression for easier debugging
            optimized.container.enable_compression = false;

            // More frequent monitoring for development
            optimized.monitoring.enable = true;
            optimized.monitoring.collection_interval = std::chrono::seconds(5);

            // Set development environment
            optimized.environment = "development";

            return optimized;
        }
    };

    // Health check system for production
    class production_health_monitor {
    private:
        production_metrics metrics_;
        std::atomic<bool> running_{false};
        std::thread monitor_thread_;
        std::condition_variable monitor_cv_;
        std::mutex monitor_mutex_;
        std::chrono::seconds check_interval_{30};

        // Health thresholds
        static constexpr double MAX_AVERAGE_LATENCY_MS = 100.0;
        static constexpr uint64_t MAX_QUEUE_SIZE_THRESHOLD = 50000;
        static constexpr double MIN_MEMORY_EFFICIENCY = 0.8;

    public:
        explicit production_health_monitor(std::chrono::seconds interval = std::chrono::seconds(30))
            : check_interval_(interval) {}

        ~production_health_monitor() {
            stop();
        }

        void start() {
            if (running_) return;

            running_ = true;
            monitor_thread_ = std::thread([this]() { monitor_loop(); });
        }

        void stop() {
            if (!running_) return;

            running_ = false;
            monitor_cv_.notify_all();
            if (monitor_thread_.joinable()) {
                monitor_thread_.join();
            }
        }

        void record_message_processed(std::chrono::milliseconds latency) {
            metrics_.total_messages_processed++;

            // Update average latency
            auto current_avg = metrics_.average_latency_ms.load();
            auto new_avg = (current_avg + latency.count()) / 2.0;
            metrics_.average_latency_ms = new_avg;
        }

        void record_queue_size(uint64_t current_size) {
            auto current_peak = metrics_.peak_queue_size.load();
            if (current_size > current_peak) {
                metrics_.peak_queue_size = current_size;
            }
        }

        void record_memory_usage(uint64_t bytes) {
            metrics_.memory_usage_bytes = bytes;
        }

        struct health_status {
            bool is_healthy = true;
            double average_latency_ms = 0.0;
            uint64_t peak_queue_size = 0;
            uint64_t memory_usage_mb = 0;
            uint64_t messages_per_second = 0;
            std::vector<std::string> warnings;
            std::vector<std::string> errors;
        };

        health_status get_health_status() const {
            health_status status;
            status.average_latency_ms = metrics_.average_latency_ms;
            status.peak_queue_size = metrics_.peak_queue_size;
            status.memory_usage_mb = metrics_.memory_usage_bytes / (1024 * 1024);
            status.messages_per_second = metrics_.messages_per_second;

            // Check health conditions
            if (status.average_latency_ms > MAX_AVERAGE_LATENCY_MS) {
                status.is_healthy = false;
                status.errors.push_back("Average latency too high: " +
                    std::to_string(status.average_latency_ms) + "ms");
            }

            if (status.peak_queue_size > MAX_QUEUE_SIZE_THRESHOLD) {
                status.warnings.push_back("Queue size approaching limit: " +
                    std::to_string(status.peak_queue_size));
            }

            if (status.memory_usage_mb > 1000) {  // 1GB threshold
                status.warnings.push_back("High memory usage: " +
                    std::to_string(status.memory_usage_mb) + "MB");
            }

            return status;
        }

        production_metrics get_metrics() const {
            return metrics_;
        }

    private:
        void monitor_loop() {
            auto last_check = std::chrono::steady_clock::now();
            uint64_t last_message_count = 0;

            while (running_) {
                std::unique_lock<std::mutex> lock(monitor_mutex_);
                if (monitor_cv_.wait_for(lock, check_interval_, [this] { return !running_; })) {
                    break;
                }

                // Calculate messages per second
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_check);

                if (elapsed.count() > 0) {
                    auto current_count = metrics_.total_messages_processed.load();
                    auto messages_in_period = current_count - last_message_count;
                    metrics_.messages_per_second = messages_in_period / elapsed.count();

                    last_message_count = current_count;
                    last_check = now;
                }

                // Log health status periodically
                auto health = get_health_status();
                if (!health.is_healthy) {
                    // In a real production system, this would trigger alerts
                    // For now, we'll just record it
                }
            }
        }
    };

    // Production-ready factory for optimized components
    class production_component_factory {
    public:
        static std::unique_ptr<production_message_allocator> create_message_allocator(
            const config::messaging_config& config) {

            size_t message_pool_size = config.message_bus.max_queue_size / 10;  // 10% of queue size
            size_t payload_pool_size = message_pool_size;

            return std::make_unique<production_message_allocator>(message_pool_size, payload_pool_size);
        }

        static std::unique_ptr<production_health_monitor> create_health_monitor(
            const config::messaging_config& config) {

            auto interval = config.monitoring.collection_interval;
            return std::make_unique<production_health_monitor>(interval);
        }

        static config::messaging_config create_optimized_config(const std::string& environment) {
            config::messaging_config base_config;

            if (environment == "production") {
                return production_config_optimizer::optimize_for_production(base_config);
            } else if (environment == "development") {
                return production_config_optimizer::optimize_for_development(base_config);
            } else {
                // Default staging configuration
                auto config = production_config_optimizer::optimize_for_production(base_config);
                config.environment = "staging";
                config.message_bus.max_queue_size = 50000;  // Between dev and prod
                config.message_bus.worker_threads = std::thread::hardware_concurrency();
                return config;
            }
        }
    };

} // namespace kcenon::messaging::core