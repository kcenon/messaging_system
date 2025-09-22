#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>
#include <optional>
#include <variant>
#include <regex>
#include <future>
#include <set>
#include <map>

#include <kcenon/monitoring/alerting/rule_engine.h>

namespace kcenon::monitoring::alerting {

// Notification channel types
enum class NotificationChannel {
    EMAIL,
    SLACK,
    SMS,
    WEBHOOK,
    PAGERDUTY,
    OPSGENIE,
    CUSTOM
};

// Notification status
enum class NotificationStatus {
    PENDING,
    SENDING,
    SENT,
    FAILED,
    RETRY
};

// Notification priority
enum class NotificationPriority {
    LOW,
    MEDIUM,
    HIGH,
    URGENT
};

// Channel configuration base
struct ChannelConfig {
    NotificationChannel type;
    std::string name;
    bool enabled = true;
    std::chrono::seconds timeout = std::chrono::seconds(30);
    int max_retries = 3;
    std::chrono::seconds retry_delay = std::chrono::seconds(60);
};

// Email channel configuration
struct EmailConfig : ChannelConfig {
    std::string smtp_server;
    int smtp_port = 587;
    bool use_tls = true;
    std::string username;
    std::string password;
    std::string from_address;
    std::vector<std::string> to_addresses;
    std::vector<std::string> cc_addresses;
    std::vector<std::string> bcc_addresses;

    EmailConfig() { type = NotificationChannel::EMAIL; }
};

// Slack channel configuration
struct SlackConfig : ChannelConfig {
    std::string webhook_url;
    std::string channel;
    std::string username;
    std::string icon_emoji;
    bool use_attachments = true;

    SlackConfig() { type = NotificationChannel::SLACK; }
};

// SMS channel configuration
struct SMSConfig : ChannelConfig {
    std::string api_key;
    std::string api_secret;
    std::string from_number;
    std::vector<std::string> to_numbers;
    std::string provider; // twilio, nexmo, etc.

    SMSConfig() { type = NotificationChannel::SMS; }
};

// Webhook channel configuration
struct WebhookConfig : ChannelConfig {
    std::string url;
    std::string method = "POST";
    std::unordered_map<std::string, std::string> headers;
    std::string auth_type; // basic, bearer, apikey
    std::string auth_value;
    bool verify_ssl = true;

    WebhookConfig() { type = NotificationChannel::WEBHOOK; }
};

// Notification template
struct NotificationTemplate {
    std::string id;
    std::string name;
    std::string subject_template;
    std::string body_template;
    std::string format; // plain, html, markdown
    std::unordered_map<std::string, std::string> custom_fields;
};

// Notification request
struct NotificationRequest {
    std::string id;
    Alert alert;
    NotificationChannel channel;
    std::string channel_config_id;
    NotificationPriority priority;
    std::string template_id;
    std::unordered_map<std::string, std::string> custom_data;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point scheduled_at;
};

// Notification result
struct NotificationResult {
    std::string request_id;
    NotificationStatus status;
    std::string message;
    std::chrono::system_clock::time_point sent_at;
    int retry_count = 0;
    std::string error_details;
};

// Notification handler interface
class INotificationHandler {
public:
    virtual ~INotificationHandler() = default;
    virtual NotificationResult send(const NotificationRequest& request) = 0;
    virtual bool validate_config(const ChannelConfig& config) = 0;
    virtual NotificationChannel get_channel_type() const = 0;
};

// Email notification handler
class EmailHandler : public INotificationHandler {
public:
    EmailHandler();
    NotificationResult send(const NotificationRequest& request) override;
    bool validate_config(const ChannelConfig& config) override;
    NotificationChannel get_channel_type() const override {
        return NotificationChannel::EMAIL;
    }

private:
    bool send_smtp(const EmailConfig& config, const std::string& subject,
                  const std::string& body, const std::string& format);
};

// Slack notification handler
class SlackHandler : public INotificationHandler {
public:
    SlackHandler();
    NotificationResult send(const NotificationRequest& request) override;
    bool validate_config(const ChannelConfig& config) override;
    NotificationChannel get_channel_type() const override {
        return NotificationChannel::SLACK;
    }

private:
    std::string build_slack_message(const Alert& alert, const SlackConfig& config);
    bool send_webhook(const std::string& url, const std::string& payload);
};

// Webhook notification handler
class WebhookHandler : public INotificationHandler {
public:
    WebhookHandler();
    NotificationResult send(const NotificationRequest& request) override;
    bool validate_config(const ChannelConfig& config) override;
    NotificationChannel get_channel_type() const override {
        return NotificationChannel::WEBHOOK;
    }

private:
    bool send_http_request(const WebhookConfig& config, const std::string& payload);
};

// Notification manager
class NotificationManager {
public:
    NotificationManager();
    ~NotificationManager();

    // Channel configuration
    void add_channel_config(const std::string& id,
                           std::shared_ptr<ChannelConfig> config);
    void update_channel_config(const std::string& id,
                              std::shared_ptr<ChannelConfig> config);
    void remove_channel_config(const std::string& id);
    std::shared_ptr<ChannelConfig> get_channel_config(const std::string& id) const;

    // Template management
    void add_template(const NotificationTemplate& tmpl);
    void update_template(const std::string& id, const NotificationTemplate& tmpl);
    void remove_template(const std::string& id);
    std::optional<NotificationTemplate> get_template(const std::string& id) const;

    // Handler registration
    void register_handler(std::shared_ptr<INotificationHandler> handler);
    void unregister_handler(NotificationChannel channel);

    // Send notifications
    std::future<NotificationResult> send_notification(const NotificationRequest& request);
    std::vector<std::future<NotificationResult>> send_notifications(
        const std::vector<NotificationRequest>& requests);

    // Send alert to all configured channels
    std::vector<std::future<NotificationResult>> notify_alert(const Alert& alert);

    // Retry mechanism
    void retry_failed_notifications();
    void schedule_retry(const NotificationRequest& request, int retry_count);

    // Template rendering
    std::string render_template(const std::string& tmpl_string,
                               const std::unordered_map<std::string, std::string>& variables);
    std::pair<std::string, std::string> render_notification_content(
        const NotificationTemplate& tmpl, const Alert& alert);

    // Query notification status
    std::optional<NotificationResult> get_notification_status(const std::string& request_id) const;
    std::vector<NotificationResult> get_recent_notifications(size_t count) const;
    std::vector<NotificationResult> get_failed_notifications() const;

    // Statistics
    size_t get_pending_count() const;
    size_t get_sent_count() const { return sent_count_.load(); }
    size_t get_failed_count() const { return failed_count_.load(); }
    double get_success_rate() const;

    // Start/stop notification processing
    void start();
    void stop();

private:
    mutable std::mutex config_mutex_;
    mutable std::mutex template_mutex_;
    mutable std::mutex handler_mutex_;
    mutable std::mutex queue_mutex_;
    mutable std::mutex history_mutex_;

    // Configurations
    std::unordered_map<std::string, std::shared_ptr<ChannelConfig>> channel_configs_;
    std::unordered_map<std::string, NotificationTemplate> templates_;
    std::unordered_map<NotificationChannel, std::shared_ptr<INotificationHandler>> handlers_;

    // Processing queue
    std::priority_queue<
        std::pair<NotificationPriority, NotificationRequest>,
        std::vector<std::pair<NotificationPriority, NotificationRequest>>,
        std::function<bool(const std::pair<NotificationPriority, NotificationRequest>&,
                         const std::pair<NotificationPriority, NotificationRequest>&)>
    > notification_queue_;

    // Retry queue
    std::queue<std::pair<NotificationRequest, int>> retry_queue_;

    // History
    std::vector<NotificationResult> notification_history_;
    size_t max_history_size_ = 1000;

    // Statistics
    std::atomic<size_t> sent_count_{0};
    std::atomic<size_t> failed_count_{0};

    // Processing threads
    std::atomic<bool> running_{false};
    std::vector<std::thread> worker_threads_;
    std::thread retry_thread_;
    std::condition_variable cv_;
    std::condition_variable retry_cv_;

    // Worker configuration
    size_t worker_count_ = 4;

    // Helper methods
    void process_notifications();
    void process_retries();
    NotificationResult send_notification_internal(const NotificationRequest& request);
    bool should_retry(const NotificationResult& result, int retry_count) const;
    void add_to_history(const NotificationResult& result);
    std::string replace_variables(const std::string& text,
                                 const std::unordered_map<std::string, std::string>& vars);
};

// Notification builder for fluent API
class NotificationBuilder {
public:
    NotificationBuilder() : request_{} {
        request_.id = generate_id();
        request_.created_at = std::chrono::system_clock::now();
        request_.scheduled_at = request_.created_at;
    }

    NotificationBuilder& with_alert(const Alert& alert) {
        request_.alert = alert;
        return *this;
    }

    NotificationBuilder& with_channel(NotificationChannel channel) {
        request_.channel = channel;
        return *this;
    }

    NotificationBuilder& with_channel_config(const std::string& config_id) {
        request_.channel_config_id = config_id;
        return *this;
    }

    NotificationBuilder& with_priority(NotificationPriority priority) {
        request_.priority = priority;
        return *this;
    }

    NotificationBuilder& with_template(const std::string& template_id) {
        request_.template_id = template_id;
        return *this;
    }

    NotificationBuilder& add_custom_data(const std::string& key, const std::string& value) {
        request_.custom_data[key] = value;
        return *this;
    }

    NotificationBuilder& schedule_at(std::chrono::system_clock::time_point time) {
        request_.scheduled_at = time;
        return *this;
    }

    NotificationRequest build() const {
        return request_;
    }

private:
    NotificationRequest request_;

    std::string generate_id() const {
        // Generate unique ID implementation
        return "notif_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }
};

} // namespace monitoring_system::alerting