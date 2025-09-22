#pragma once

#include "../service_interface.h"
#include "../../core/message_types.h"
#include <memory>
#include <atomic>
#include <chrono>
#include <unordered_map>

#ifdef HAS_MONITORING_SYSTEM
#include <monitoring/monitoring_system.h>
#endif

namespace kcenon::messaging::services::monitoring {

    // Monitoring metrics structure
    struct messaging_metrics {
        std::atomic<uint64_t> messages_published{0};
        std::atomic<uint64_t> messages_consumed{0};
        std::atomic<uint64_t> messages_failed{0};
        std::atomic<uint64_t> active_subscribers{0};
        std::atomic<double> average_processing_time{0.0};
        std::chrono::steady_clock::time_point last_reset;

        messaging_metrics() : last_reset(std::chrono::steady_clock::now()) {}

        void reset() {
            messages_published = 0;
            messages_consumed = 0;
            messages_failed = 0;
            active_subscribers = 0;
            average_processing_time = 0.0;
            last_reset = std::chrono::steady_clock::now();
        }
    };

    // Monitoring service interface
    class monitoring_service : public service_interface {
    public:
        virtual ~monitoring_service() = default;

        // Service lifecycle
        bool initialize() override { return true; }
        void shutdown() override {}
        bool is_running() const override { return true; }

        // Monitoring specific methods
        virtual void record_message_published(const std::string& topic) = 0;
        virtual void record_message_consumed(const std::string& topic, std::chrono::milliseconds processing_time) = 0;
        virtual void record_message_failed(const std::string& topic, const std::string& error) = 0;
        virtual void record_subscriber_added(const std::string& topic) = 0;
        virtual void record_subscriber_removed(const std::string& topic) = 0;

        virtual messaging_metrics get_metrics() const = 0;
        virtual std::unordered_map<std::string, uint64_t> get_topic_metrics() const = 0;
    };

    // Internal monitoring service implementation
    class internal_monitoring_service : public monitoring_service {
    private:
        mutable std::mutex metrics_mutex_;
        messaging_metrics metrics_;
        std::unordered_map<std::string, uint64_t> topic_publish_counts_;
        std::unordered_map<std::string, uint64_t> topic_consume_counts_;
        std::unordered_map<std::string, uint64_t> topic_subscriber_counts_;

    public:
        internal_monitoring_service() = default;

        void record_message_published(const std::string& topic) override {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_.messages_published++;
            topic_publish_counts_[topic]++;
        }

        void record_message_consumed(const std::string& topic, std::chrono::milliseconds processing_time) override {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_.messages_consumed++;
            topic_consume_counts_[topic]++;

            // Update average processing time
            auto current_avg = metrics_.average_processing_time.load();
            auto new_avg = (current_avg + processing_time.count()) / 2.0;
            metrics_.average_processing_time = new_avg;
        }

        void record_message_failed(const std::string& topic, const std::string& error) override {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_.messages_failed++;
        }

        void record_subscriber_added(const std::string& topic) override {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            metrics_.active_subscribers++;
            topic_subscriber_counts_[topic]++;
        }

        void record_subscriber_removed(const std::string& topic) override {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            if (metrics_.active_subscribers > 0) {
                metrics_.active_subscribers--;
            }
            if (topic_subscriber_counts_[topic] > 0) {
                topic_subscriber_counts_[topic]--;
            }
        }

        messaging_metrics get_metrics() const override {
            messaging_metrics snapshot;
            snapshot.messages_published = metrics_.messages_published.load();
            snapshot.messages_consumed = metrics_.messages_consumed.load();
            snapshot.messages_failed = metrics_.messages_failed.load();
            snapshot.active_subscribers = metrics_.active_subscribers.load();
            snapshot.average_processing_time = metrics_.average_processing_time.load();
            snapshot.last_reset = metrics_.last_reset;
            return snapshot;
        }

        std::unordered_map<std::string, uint64_t> get_topic_metrics() const override {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            std::unordered_map<std::string, uint64_t> result;

            for (const auto& [topic, count] : topic_publish_counts_) {
                result[topic + "_published"] = count;
            }
            for (const auto& [topic, count] : topic_consume_counts_) {
                result[topic + "_consumed"] = count;
            }
            for (const auto& [topic, count] : topic_subscriber_counts_) {
                result[topic + "_subscribers"] = count;
            }

            return result;
        }
    };

#ifdef HAS_MONITORING_SYSTEM
    // External monitoring service implementation
    class external_monitoring_service : public monitoring_service {
    private:
        std::unique_ptr<monitoring::monitoring_system> external_monitor_;
        messaging_metrics metrics_;

    public:
        external_monitoring_service() {
            external_monitor_ = std::make_unique<monitoring::monitoring_system>();
        }

        bool initialize() override {
            if (external_monitor_) {
                return external_monitor_->initialize();
            }
            return false;
        }

        void shutdown() override {
            if (external_monitor_) {
                external_monitor_->shutdown();
            }
        }

        bool is_running() const override {
            return external_monitor_ && external_monitor_->is_running();
        }

        void record_message_published(const std::string& topic) override {
            metrics_.messages_published++;
            if (external_monitor_) {
                external_monitor_->record_metric("messaging.published", 1, {{"topic", topic}});
            }
        }

        void record_message_consumed(const std::string& topic, std::chrono::milliseconds processing_time) override {
            metrics_.messages_consumed++;
            auto current_avg = metrics_.average_processing_time.load();
            auto new_avg = (current_avg + processing_time.count()) / 2.0;
            metrics_.average_processing_time = new_avg;

            if (external_monitor_) {
                external_monitor_->record_metric("messaging.consumed", 1, {{"topic", topic}});
                external_monitor_->record_metric("messaging.processing_time", processing_time.count(), {{"topic", topic}});
            }
        }

        void record_message_failed(const std::string& topic, const std::string& error) override {
            metrics_.messages_failed++;
            if (external_monitor_) {
                external_monitor_->record_metric("messaging.failed", 1, {{"topic", topic}, {"error", error}});
            }
        }

        void record_subscriber_added(const std::string& topic) override {
            metrics_.active_subscribers++;
            if (external_monitor_) {
                external_monitor_->record_metric("messaging.subscribers_added", 1, {{"topic", topic}});
            }
        }

        void record_subscriber_removed(const std::string& topic) override {
            if (metrics_.active_subscribers > 0) {
                metrics_.active_subscribers--;
            }
            if (external_monitor_) {
                external_monitor_->record_metric("messaging.subscribers_removed", 1, {{"topic", topic}});
            }
        }

        messaging_metrics get_metrics() const override {
            messaging_metrics snapshot;
            snapshot.messages_published = metrics_.messages_published.load();
            snapshot.messages_consumed = metrics_.messages_consumed.load();
            snapshot.messages_failed = metrics_.messages_failed.load();
            snapshot.active_subscribers = metrics_.active_subscribers.load();
            snapshot.average_processing_time = metrics_.average_processing_time.load();
            snapshot.last_reset = metrics_.last_reset;
            return snapshot;
        }

        std::unordered_map<std::string, uint64_t> get_topic_metrics() const override {
            // For external monitoring, we might not track detailed topic metrics locally
            return {};
        }
    };
#endif

    // Monitoring service adapter
    class monitoring_service_adapter : public service_adapter {
    private:
        std::shared_ptr<monitoring_service> monitoring_service_;

    public:
        explicit monitoring_service_adapter(std::shared_ptr<monitoring_service> service)
            : service_adapter(service), monitoring_service_(std::move(service)) {}

        void register_with_bus(core::message_bus* bus) {
            // Subscribe to monitoring topics
            if (bus && monitoring_service_) {
                bus->subscribe("monitoring.*", [this](const core::message& msg) {
                    this->handle_monitoring_message(msg);
                });
            }
        }

    private:
        void handle_monitoring_message(const core::message& msg) {
            if (!monitoring_service_) return;

            // Process the monitoring message based on its topic
            if (msg.payload.topic == "monitoring.message_published") {
                auto original_topic = msg.payload.get<std::string>("original_topic", "");
                monitoring_service_->record_message_published(original_topic);
            }
            else if (msg.payload.topic == "monitoring.message_consumed") {
                auto original_topic = msg.payload.get<std::string>("original_topic", "");
                auto processing_time = std::chrono::milliseconds(
                    msg.payload.get<int64_t>("processing_time_ms", 0)
                );
                monitoring_service_->record_message_consumed(original_topic, processing_time);
            }
            else if (msg.payload.topic == "monitoring.message_failed") {
                auto original_topic = msg.payload.get<std::string>("original_topic", "");
                auto error = msg.payload.get<std::string>("error", "");
                monitoring_service_->record_message_failed(original_topic, error);
            }
            else if (msg.payload.topic == "monitoring.subscriber_added") {
                auto topic = msg.payload.get<std::string>("topic", "");
                monitoring_service_->record_subscriber_added(topic);
            }
            else if (msg.payload.topic == "monitoring.subscriber_removed") {
                auto topic = msg.payload.get<std::string>("topic", "");
                monitoring_service_->record_subscriber_removed(topic);
            }
        }

    public:
        std::shared_ptr<monitoring_service> get_monitoring_service() const {
            return monitoring_service_;
        }
    };

    // Factory function for creating monitoring service
    inline std::shared_ptr<monitoring_service> create_monitoring_service(bool use_external = false) {
#ifdef HAS_MONITORING_SYSTEM
        if (use_external) {
            return std::make_shared<external_monitoring_service>();
        }
#endif
        return std::make_shared<internal_monitoring_service>();
    }

} // namespace kcenon::messaging::services::monitoring