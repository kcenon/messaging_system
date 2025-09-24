#include "kcenon/messaging/core/message_bus.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace kcenon::messaging::core {

    // Internal helper classes (simplified implementations)
    class message_router {
    public:
        void add_subscription(const std::string& topic, message_handler handler) {
            std::lock_guard<std::mutex> lock(mutex_);
            subscriptions_[topic].push_back(std::move(handler));
        }

        void remove_subscriptions(const std::string& topic) {
            std::lock_guard<std::mutex> lock(mutex_);
            subscriptions_.erase(topic);
        }

        std::vector<message_handler> get_handlers(const std::string& topic) const {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = subscriptions_.find(topic);
            return (it != subscriptions_.end()) ? it->second : std::vector<message_handler>{};
        }

        std::vector<std::string> get_topics() const {
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<std::string> topics;
            for (const auto& pair : subscriptions_) {
                topics.push_back(pair.first);
            }
            return topics;
        }

        size_t get_subscription_count(const std::string& topic) const {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = subscriptions_.find(topic);
            return (it != subscriptions_.end()) ? it->second.size() : 0;
        }

    private:
        mutable std::mutex mutex_;
        std::unordered_map<std::string, std::vector<message_handler>> subscriptions_;
    };

    class message_queue {
    public:
        explicit message_queue(size_t max_size) : max_size_(max_size) {}

        bool enqueue(const message& msg) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.size() >= max_size_) {
                return false;  // Queue full
            }
            queue_.push(msg);
            condition_.notify_one();
            return true;
        }

        bool dequeue(message& msg, std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (condition_.wait_for(lock, timeout, [this] { return !queue_.empty() || shutdown_; })) {
                if (!queue_.empty()) {
                    msg = queue_.front();
                    queue_.pop();
                    return true;
                }
            }
            return false;
        }

        void shutdown() {
            std::lock_guard<std::mutex> lock(mutex_);
            shutdown_ = true;
            condition_.notify_all();
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(mutex_);
            return queue_.size();
        }

    private:
        mutable std::mutex mutex_;
        std::condition_variable condition_;
        std::queue<message> queue_;
        size_t max_size_;
        bool shutdown_ = false;
    };

    class message_dispatcher {
    public:
        explicit message_dispatcher(message_router* router) : router_(router) {}

        void dispatch(const message& msg) {
            if (!router_) return;

            auto handlers = router_->get_handlers(msg.payload.topic);
            for (const auto& handler : handlers) {
                try {
                    handler(msg);
                } catch (const std::exception&) {
                    // Log error (in production would use proper logging)
                    // For now, silently continue to next handler
                }
            }
        }

    private:
        message_router* router_;
    };

    // message_bus implementation
    message_bus::message_bus(const message_bus_config& config)
        : config_(config)
        , router_(std::make_unique<message_router>())
        , queue_(std::make_unique<message_queue>(config.max_queue_size))
        , dispatcher_(std::make_unique<message_dispatcher>(router_.get()))
    {
    }

    message_bus::~message_bus() {
        shutdown();
    }

    bool message_bus::initialize() {
        if (running_.load()) {
            return false;  // Already running
        }

        try {
            // Start worker threads
            worker_threads_.reserve(config_.worker_threads);
            for (size_t i = 0; i < config_.worker_threads; ++i) {
                worker_threads_.emplace_back([this] { worker_thread_func(); });
            }

            running_.store(true);
            return true;
        } catch (const std::exception&) {
            shutdown();
            return false;
        }
    }

    void message_bus::shutdown() {
        if (!running_.load()) {
            return;  // Already shut down
        }

        shutdown_requested_.store(true);
        queue_->shutdown();

        // Wait for worker threads to finish
        for (auto& thread : worker_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        worker_threads_.clear();

        running_.store(false);
        shutdown_requested_.store(false);
    }

    bool message_bus::publish(const message& msg) {
        if (!running_.load() || !validate_message(msg)) {
            return false;
        }

        bool success = queue_->enqueue(msg);
        update_statistics(msg, success);

        if (success) {
            stats_.messages_published.fetch_add(1);
        }

        return success;
    }

    bool message_bus::publish(const std::string& topic, const message_payload& payload,
                             const std::string& sender) {
        message msg(topic, sender);
        msg.payload = payload;
        return publish(msg);
    }

    void message_bus::subscribe(const std::string& topic, message_handler handler) {
        router_->add_subscription(topic, std::move(handler));
        stats_.active_subscriptions.fetch_add(1);
    }

    void message_bus::unsubscribe_all(const std::string& topic) {
        size_t count = router_->get_subscription_count(topic);
        router_->remove_subscriptions(topic);
        stats_.active_subscriptions.fetch_sub(count);
    }

    std::future<message> message_bus::request(const message& request_msg) {
        // Simplified implementation - in production would use proper correlation IDs
        auto promise = std::make_shared<std::promise<message>>();
        auto future = promise->get_future();

        // Publish the request
        if (publish(request_msg)) {
            stats_.pending_requests.fetch_add(1);
        }

        return future;
    }

    void message_bus::respond(const message&, const message& response_msg) {
        // Simplified - in production would correlate with pending requests
        publish(response_msg);
    }

    std::vector<std::string> message_bus::get_topics() const {
        return router_->get_topics();
    }

    size_t message_bus::get_subscriber_count(const std::string& topic) const {
        return router_->get_subscription_count(topic);
    }

    void message_bus::reset_statistics() {
        stats_.messages_published.store(0);
        stats_.messages_processed.store(0);
        stats_.messages_failed.store(0);
        // Don't reset active_subscriptions and pending_requests as they represent current state
    }

    void message_bus::worker_thread_func() {
        message msg;
        while (!shutdown_requested_.load()) {
            if (queue_->dequeue(msg, std::chrono::milliseconds(100))) {
                try {
                    dispatcher_->dispatch(msg);
                    stats_.messages_processed.fetch_add(1);
                    update_statistics(msg, true);
                } catch (const std::exception&) {
                    stats_.messages_failed.fetch_add(1);
                    update_statistics(msg, false);
                }
            }
        }
    }

    bool message_bus::validate_message(const message& msg) const {
        return !msg.payload.topic.empty() && !msg.metadata.id.empty();
    }

    void message_bus::update_statistics(const message&, bool) {
        // Additional statistics tracking can be added here
        // For now, this is handled in specific methods
    }

} // namespace kcenon::messaging::core