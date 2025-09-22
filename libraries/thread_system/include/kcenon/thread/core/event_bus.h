#pragma once

#include <any>
#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <kcenon/thread/core/thread_pool.h>

namespace kcenon::thread {

/**
 * @brief Event base class for type safety
 */
class event_base {
public:
    virtual ~event_base() = default;

    /**
     * @brief Get event type name
     * @return Event type name
     */
    virtual std::string type_name() const = 0;

    /**
     * @brief Get event timestamp
     * @return Event timestamp
     */
    std::chrono::steady_clock::time_point timestamp() const {
        return timestamp_;
    }

protected:
    std::chrono::steady_clock::time_point timestamp_{std::chrono::steady_clock::now()};
};

/**
 * @brief Event Bus for publish-subscribe pattern
 *
 * Provides asynchronous event distribution across system components
 */
class event_bus {
public:
    /**
     * @brief Handler function type
     */
    using handler_func = std::function<void(const std::any&)>;

    /**
     * @brief Subscription handle for managing subscriptions
     */
    class subscription {
    public:
        subscription() = default;
        subscription(event_bus* bus, std::type_index type, std::size_t id)
            : bus_(bus), type_(type), id_(id) {}

        /**
         * @brief Unsubscribe from events
         */
        void unsubscribe() {
            if (bus_) {
                bus_->unsubscribe(type_, id_);
                bus_ = nullptr;
            }
        }

        /**
         * @brief Check if subscription is active
         */
        bool is_active() const {
            return bus_ != nullptr;
        }

        // Move operations
        subscription(subscription&& other) noexcept
            : bus_(other.bus_), type_(other.type_), id_(other.id_) {
            other.bus_ = nullptr;
        }

        subscription& operator=(subscription&& other) noexcept {
            if (this != &other) {
                unsubscribe();
                bus_ = other.bus_;
                type_ = other.type_;
                id_ = other.id_;
                other.bus_ = nullptr;
            }
            return *this;
        }

        // Disable copy
        subscription(const subscription&) = delete;
        subscription& operator=(const subscription&) = delete;

        ~subscription() {
            unsubscribe();
        }

    private:
        event_bus* bus_{nullptr};
        std::type_index type_{typeid(void)};
        std::size_t id_{0};
    };

    /**
     * @brief Constructor
     * @param thread_pool Optional thread pool for async processing
     */
    explicit event_bus(std::shared_ptr<thread_pool> pool = nullptr)
        : thread_pool_(pool) {
        if (!thread_pool_) {
            // Create default thread pool with 2 threads for event processing
            thread_pool_ = std::make_shared<thread_pool>(2);
        }
    }

    /**
     * @brief Publish an event asynchronously
     * @tparam Event Event type
     * @param event Event to publish
     */
    template<typename Event>
    void publish(const Event& event) {
        auto type = std::type_index(typeid(Event));

        // Get handlers snapshot to avoid holding lock during callbacks
        std::vector<handler_func> handlers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handlers_.find(type);
            if (it != handlers_.end()) {
                for (const auto& [id, handler] : it->second) {
                    handlers_copy.push_back(handler);
                }
            }
        }

        // Process handlers asynchronously
        if (!handlers_copy.empty() && thread_pool_) {
            thread_pool_->submit([handlers_copy, event]() {
                for (const auto& handler : handlers_copy) {
                    try {
                        handler(std::any(event));
                    } catch (...) {
                        // Log error but continue with other handlers
                    }
                }
            });
        }
    }

    /**
     * @brief Publish an event synchronously
     * @tparam Event Event type
     * @param event Event to publish
     */
    template<typename Event>
    void publish_sync(const Event& event) {
        auto type = std::type_index(typeid(Event));

        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            for (const auto& [id, handler] : it->second) {
                try {
                    handler(std::any(event));
                } catch (...) {
                    // Log error but continue with other handlers
                }
            }
        }
    }

    /**
     * @brief Subscribe to events of a specific type
     * @tparam Event Event type to subscribe to
     * @param handler Handler function
     * @return Subscription handle
     */
    template<typename Event>
    [[nodiscard]] subscription subscribe(std::function<void(const Event&)> handler) {
        auto type = std::type_index(typeid(Event));
        auto wrapped_handler = [handler](const std::any& any_event) {
            try {
                const auto& event = std::any_cast<const Event&>(any_event);
                handler(event);
            } catch (const std::bad_any_cast&) {
                // Type mismatch - should not happen
            }
        };

        std::lock_guard<std::mutex> lock(mutex_);
        auto id = next_handler_id_++;
        handlers_[type][id] = wrapped_handler;

        return subscription(this, type, id);
    }

    /**
     * @brief Clear all subscriptions for a specific event type
     * @tparam Event Event type
     */
    template<typename Event>
    void clear_subscriptions() {
        auto type = std::type_index(typeid(Event));
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.erase(type);
    }

    /**
     * @brief Clear all subscriptions
     */
    void clear_all_subscriptions() {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.clear();
    }

    /**
     * @brief Get the number of subscribers for an event type
     * @tparam Event Event type
     * @return Number of subscribers
     */
    template<typename Event>
    std::size_t subscriber_count() const {
        auto type = std::type_index(typeid(Event));
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            return it->second.size();
        }
        return 0;
    }

    /**
     * @brief Get singleton instance
     * @return Event bus instance
     */
    static event_bus& instance() {
        static event_bus instance;
        return instance;
    }

private:
    /**
     * @brief Unsubscribe a specific handler
     */
    void unsubscribe(std::type_index type, std::size_t id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = handlers_.find(type);
        if (it != handlers_.end()) {
            it->second.erase(id);
            if (it->second.empty()) {
                handlers_.erase(it);
            }
        }
    }

    mutable std::mutex mutex_;
    std::unordered_map<std::type_index, std::unordered_map<std::size_t, handler_func>> handlers_;
    std::shared_ptr<thread_pool> thread_pool_;
    std::size_t next_handler_id_{1};
};

// Common event types

/**
 * @brief System startup event
 */
struct system_startup_event : event_base {
    std::string system_name;

    explicit system_startup_event(std::string name)
        : system_name(std::move(name)) {}

    std::string type_name() const override {
        return "SystemStartupEvent";
    }
};

/**
 * @brief System shutdown event
 */
struct system_shutdown_event : event_base {
    std::string system_name;
    std::string reason;

    system_shutdown_event(std::string name, std::string reason_msg = "")
        : system_name(std::move(name)), reason(std::move(reason_msg)) {}

    std::string type_name() const override {
        return "SystemShutdownEvent";
    }
};

/**
 * @brief Configuration changed event
 */
struct config_changed_event : event_base {
    std::string config_path;
    std::any old_value;
    std::any new_value;

    config_changed_event(std::string path, std::any old_val, std::any new_val)
        : config_path(std::move(path))
        , old_value(std::move(old_val))
        , new_value(std::move(new_val)) {}

    std::string type_name() const override {
        return "ConfigChangedEvent";
    }
};

/**
 * @brief Performance alert event
 */
struct performance_alert_event : event_base {
    enum class severity { info, warning, critical };

    severity level;
    std::string message;
    double metric_value;

    performance_alert_event(severity lvl, std::string msg, double value)
        : level(lvl), message(std::move(msg)), metric_value(value) {}

    std::string type_name() const override {
        return "PerformanceAlertEvent";
    }
};

} // namespace kcenon::thread