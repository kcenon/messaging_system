#include "kcenon/messaging/core/message_bus.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <vector>

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
        explicit message_queue(size_t max_size, bool enable_priority)
            : max_size_(max_size)
            , enable_priority_(enable_priority)
        {}

        bool enqueue(const message& msg) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (size_locked() >= max_size_) {
                return false;  // Queue full
            }

            if (enable_priority_) {
                priority_queue_.push(queued_message{msg, next_sequence_++});
                if (!priority_first_enqueue_seen_) {
                    priority_first_enqueue_seen_ = true;
                    priority_first_enqueue_time_ = std::chrono::steady_clock::now();
                }
                if (!priority_warmup_done_ && next_sequence_ >= priority_warmup_size_) {
                    priority_warmup_done_ = true;
                    condition_.notify_all();
                }
            } else {
                queue_.push(msg);
            }
            condition_.notify_one();
            return true;
        }

        bool dequeue(message& msg, std::chrono::milliseconds timeout) {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!shutdown_) {
                auto ready = [this] {
                    if (shutdown_) {
                        return true;
                    }
                    if (!has_items_locked()) {
                        return false;
                    }
                    if (!enable_priority_) {
                        return true;
                    }

                    if (!priority_warmup_done_) {
                        if (priority_queue_.size() >= priority_warmup_size_) {
                            priority_warmup_done_ = true;
                        } else if (priority_first_enqueue_seen_) {
                            auto now = std::chrono::steady_clock::now();
                            if ((now - priority_first_enqueue_time_) >= priority_warmup_timeout_) {
                                priority_warmup_done_ = true;
                            }
                        }
                    }

                    return priority_warmup_done_;
                };

                auto wait_duration = (!enable_priority_ || priority_warmup_done_)
                    ? timeout
                    : priority_warmup_timeout_;

                if (!condition_.wait_for(lock, wait_duration, ready)) {
                    continue;
                }

                if (!has_items_locked()) {
                    continue;
                }

                if (enable_priority_) {
                    msg = priority_queue_.top().msg;
                    priority_queue_.pop();

                    if (should_requeue_for_higher_priority(lock, msg)) {
                        continue;
                    }
                } else {
                    msg = queue_.front();
                    queue_.pop();
                }
                return true;
            }
            return false;
        }

        bool should_requeue_for_higher_priority(std::unique_lock<std::mutex>& lock, message& current_msg) {
            if (!enable_priority_ || priority_reorder_window_.count() == 0) {
                return false;
            }

            int current_priority = static_cast<int>(current_msg.metadata.priority);
            if (current_priority >= static_cast<int>(message_priority::critical)) {
                return false;
            }

            auto deadline = std::chrono::steady_clock::now() + priority_reorder_window_;
            while (!shutdown_ && std::chrono::steady_clock::now() < deadline) {
                if (!priority_queue_.empty()) {
                    int next_priority = static_cast<int>(priority_queue_.top().msg.metadata.priority);
                    if (next_priority > current_priority) {
                        priority_queue_.push(queued_message{current_msg, next_sequence_++});
                        condition_.notify_one();
                        return true;
                    }
                }

                if (condition_.wait_until(lock, deadline) == std::cv_status::timeout) {
                    break;
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
            return size_locked();
        }

    private:
        struct queued_message {
            message msg;
            uint64_t sequence;
        };

        struct priority_compare {
            bool operator()(const queued_message& lhs, const queued_message& rhs) const {
                auto lhs_priority = static_cast<int>(lhs.msg.metadata.priority);
                auto rhs_priority = static_cast<int>(rhs.msg.metadata.priority);

                if (lhs_priority == rhs_priority) {
                    // Earlier sequence should be processed first
                    return lhs.sequence > rhs.sequence;
                }

                // Higher numeric priority value should be processed first
                return lhs_priority < rhs_priority;
            }
        };

        bool has_items_locked() const {
            return enable_priority_ ? !priority_queue_.empty() : !queue_.empty();
        }

        size_t size_locked() const {
            return enable_priority_ ? priority_queue_.size() : queue_.size();
        }

        mutable std::mutex mutex_;
        std::condition_variable condition_;
        std::queue<message> queue_;
        std::priority_queue<queued_message, std::vector<queued_message>, priority_compare> priority_queue_;
        uint64_t next_sequence_{0};
        const size_t max_size_;
        const bool enable_priority_;
        const size_t priority_warmup_size_ = 256;
        const std::chrono::milliseconds priority_warmup_timeout_{2};
        const std::chrono::microseconds priority_reorder_window_{500};
        bool priority_warmup_done_ = false;
        bool priority_first_enqueue_seen_ = false;
        std::chrono::steady_clock::time_point priority_first_enqueue_time_{};
        bool shutdown_ = false;
    };

    class message_dispatcher {
    public:
        explicit message_dispatcher(message_router* router, bool ordered_dispatch)
            : router_(router)
            , ordered_dispatch_(ordered_dispatch) {}

        void dispatch(const message& msg) {
            if (!router_) return;

            auto handlers = router_->get_handlers(msg.payload.topic);
            if (ordered_dispatch_) {
                std::lock_guard<std::mutex> lock(dispatch_mutex_);
                invoke_handlers(handlers, msg);
            } else {
                invoke_handlers(handlers, msg);
            }
        }

    private:
        void invoke_handlers(const std::vector<message_handler>& handlers, const message& msg) {
            for (const auto& handler : handlers) {
                try {
                    handler(msg);
                } catch (const std::exception& e) {
                    // Swallow and continue to maintain message flow
                }
            }
        }

        message_router* router_;
        bool ordered_dispatch_ = false;
        std::mutex dispatch_mutex_;
    };

    // message_bus implementation
    message_bus::message_bus(const message_bus_config& config)
        : router_(std::make_unique<message_router>())
        , queue_(std::make_unique<message_queue>(config.max_queue_size, config.enable_priority_queue))
        , dispatcher_(std::make_unique<message_dispatcher>(router_.get(), config.enable_priority_queue))
        , config_(config)
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
            size_t thread_count = config_.enable_priority_queue ? 1 : config_.worker_threads;
            thread_count = std::max<size_t>(1, thread_count);
            worker_threads_.reserve(thread_count);
            for (size_t i = 0; i < thread_count; ++i) {
                worker_threads_.emplace_back([this] { worker_thread_func(); });
            }

            running_.store(true);
            return true;
        } catch (const std::exception& e) {
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

    void message_bus::respond(const message& original_msg, const message& response_msg) {
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
                } catch (const std::exception& e) {
                    stats_.messages_failed.fetch_add(1);
                    update_statistics(msg, false);
                }
            }
        }
    }

    bool message_bus::validate_message(const message& msg) const {
        return !msg.payload.topic.empty() && !msg.metadata.id.empty();
    }

    void message_bus::update_statistics(const message& msg, bool success) {
        // Additional statistics tracking can be added here
        // For now, this is handled in specific methods
    }

} // namespace kcenon::messaging::core
