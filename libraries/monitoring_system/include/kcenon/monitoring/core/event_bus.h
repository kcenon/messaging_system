#pragma once

/**
 * @file event_bus.h
 * @brief Lightweight event bus implementation for monitoring system
 *
 * This file implements a thread-safe, high-performance event bus
 * that enables decoupled communication between monitoring components
 * using the publish-subscribe pattern.
 */

#include <algorithm>
#include <any>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "../interfaces/event_bus_interface.h"
#include "result_types.h"
#include "error_codes.h"

namespace monitoring_system {

/**
 * @class event_envelope
 * @brief Container for events with metadata
 */
struct event_envelope {
    std::type_index type;
    std::any payload;
    event_priority priority;
    std::chrono::steady_clock::time_point timestamp;
    uint64_t id;

    event_envelope(std::type_index t, std::any p, event_priority pr)
        : type(t), payload(std::move(p)), priority(pr),
          timestamp(std::chrono::steady_clock::now()),
          id(generate_id()) {}

    bool operator<(const event_envelope& other) const {
        // Higher priority events come first
        if (priority != other.priority) {
            return static_cast<int>(priority) < static_cast<int>(other.priority);
        }
        // For same priority, older events come first
        return timestamp > other.timestamp;
    }

private:
    static uint64_t generate_id() {
        static std::atomic<uint64_t> counter{0};
        return counter.fetch_add(1, std::memory_order_relaxed);
    }
};

/**
 * @class event_handler_wrapper
 * @brief Wrapper for event handlers with metadata
 */
struct event_handler_wrapper {
    std::function<void(const std::any&)> handler;
    event_priority priority;
    uint64_t id;

    event_handler_wrapper(std::function<void(const std::any&)> h,
                         event_priority p, uint64_t i)
        : handler(std::move(h)), priority(p), id(i) {}
};

/**
 * @struct event_bus_config
 * @brief Configuration for event bus
 */
struct event_bus_config {
    size_t max_queue_size = 10000;
    size_t worker_thread_count = 2;
    std::chrono::milliseconds processing_interval{10};
    bool auto_start = false;
    bool enable_back_pressure = true;
    size_t back_pressure_threshold = 8000;
};

/**
 * @class event_bus
 * @brief Thread-safe event bus implementation
 *
 * Provides high-performance event distribution with:
 * - Priority-based event processing
 * - Back-pressure management
 * - Async and sync processing modes
 * - Type-safe publish/subscribe
 */
class event_bus : public interface_event_bus {
public:
    using config = event_bus_config;

    explicit event_bus(const config& cfg = config())
        : config_(cfg), is_running_(false), stop_requested_(false),
          total_events_published_(0), total_events_processed_(0),
          total_events_dropped_(0) {

        if (config_.auto_start) {
            start();
        }
    }

    ~event_bus() {
        stop();
    }

    // Start the event bus
    result_void start() override {
        std::lock_guard<std::mutex> lock(bus_mutex_);

        if (is_running_) {
            return result_void::error(monitoring_error_code::already_started,
                                     "Event bus is already running");
        }

        stop_requested_ = false;
        is_running_ = true;

        // Start worker threads
        for (size_t i = 0; i < config_.worker_thread_count; ++i) {
            workers_.emplace_back(&event_bus::process_events_worker, this);
        }

        return result_void::success();
    }

    // Stop the event bus
    result_void stop() override {
        {
            std::lock_guard<std::mutex> lock(bus_mutex_);
            if (!is_running_) {
                return result_void::success();
            }

            stop_requested_ = true;
            is_running_ = false;
        }

        // Wake up all worker threads
        queue_cv_.notify_all();

        // Wait for all workers to finish
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers_.clear();

        // Process any remaining events
        process_all_pending();

        return result_void::success();
    }

    // Check if event bus is active
    bool is_active() const override {
        std::lock_guard<std::mutex> lock(bus_mutex_);
        return is_running_;
    }

    // Get pending event count
    size_t get_pending_event_count() const override {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return event_queue_.size();
    }

    // Process all pending events synchronously
    result_void process_pending_events() override {
        process_all_pending();
        return result_void::success();
    }

    // Unsubscribe from events
    result_void unsubscribe_event(const subscription_token& token) override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);

        auto it = handlers_.find(token.get_event_type());
        if (it != handlers_.end()) {
            auto& handler_list = it->second;
            handler_list.erase(
                std::remove_if(handler_list.begin(), handler_list.end(),
                              [&token](const event_handler_wrapper& wrapper) {
                                  return wrapper.id == token.get_handler_id();
                              }),
                handler_list.end()
            );

            if (handler_list.empty()) {
                handlers_.erase(it);
            }
        }

        return result_void::success();
    }

    // Get statistics
    struct stats {
        uint64_t total_published;
        uint64_t total_processed;
        uint64_t total_dropped;
        size_t current_queue_size;
        size_t subscriber_count;
        bool is_back_pressure_active;
    };

    stats get_stats() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        std::lock_guard<std::mutex> lock2(handlers_mutex_);

        size_t total_subscribers = 0;
        for (const auto& [type, handlers] : handlers_) {
            total_subscribers += handlers.size();
        }

        return {
            total_events_published_.load(),
            total_events_processed_.load(),
            total_events_dropped_.load(),
            event_queue_.size(),
            total_subscribers,
            event_queue_.size() >= config_.back_pressure_threshold
        };
    }

protected:
    // Publish event implementation
    result_void publish_event_impl(std::type_index event_type,
                                  std::any event) override {
        // Check for back pressure
        if (config_.enable_back_pressure) {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (event_queue_.size() >= config_.max_queue_size) {
                total_events_dropped_.fetch_add(1);
                return result_void::error(monitoring_error_code::resource_exhausted,
                                        "Event queue is full");
            }

            if (event_queue_.size() >= config_.back_pressure_threshold) {
                // Apply back pressure - could implement adaptive strategies here
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // Queue the event
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            event_queue_.emplace(event_type, std::move(event), event_priority::normal);
            total_events_published_.fetch_add(1);
        }

        queue_cv_.notify_one();
        return result_void::success();
    }

    // Subscribe event implementation
    result<subscription_token> subscribe_event_impl(
        std::type_index event_type,
        std::function<void(const std::any&)> handler,
        uint64_t handler_id,
        event_priority priority) override {

        std::lock_guard<std::mutex> lock(handlers_mutex_);

        handlers_[event_type].emplace_back(std::move(handler), priority, handler_id);

        // Sort handlers by priority
        std::sort(handlers_[event_type].begin(), handlers_[event_type].end(),
                 [](const event_handler_wrapper& a, const event_handler_wrapper& b) {
                     return static_cast<int>(a.priority) > static_cast<int>(b.priority);
                 });

        return subscription_token(event_type, handler_id);
    }

    // Clear subscriptions implementation
    result_void clear_subscriptions_impl(std::type_index event_type) override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        handlers_.erase(event_type);
        return result_void::success();
    }

    // Get subscriber count implementation
    size_t get_subscriber_count_impl(std::type_index event_type) const override {
        std::lock_guard<std::mutex> lock(handlers_mutex_);
        auto it = handlers_.find(event_type);
        return it != handlers_.end() ? it->second.size() : 0;
    }

private:
    // Process events worker thread
    void process_events_worker() {
        while (!stop_requested_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Wait for events or stop signal
            queue_cv_.wait_for(lock, config_.processing_interval,
                              [this] { return !event_queue_.empty() || stop_requested_; });

            if (stop_requested_ && event_queue_.empty()) {
                break;
            }

            // Process a batch of events
            std::vector<event_envelope> batch;
            size_t batch_size = std::min(size_t(10), event_queue_.size());

            for (size_t i = 0; i < batch_size && !event_queue_.empty(); ++i) {
                batch.push_back(std::move(const_cast<event_envelope&>(event_queue_.top())));
                event_queue_.pop();
            }

            lock.unlock();

            // Process the batch
            for (auto& envelope : batch) {
                dispatch_event(envelope);
                total_events_processed_.fetch_add(1);
            }
        }
    }

    // Process all pending events
    void process_all_pending() {
        std::lock_guard<std::mutex> lock(queue_mutex_);

        while (!event_queue_.empty()) {
            auto envelope = std::move(const_cast<event_envelope&>(event_queue_.top()));
            event_queue_.pop();
            dispatch_event(envelope);
            total_events_processed_.fetch_add(1);
        }
    }

    // Dispatch event to handlers
    void dispatch_event(const event_envelope& envelope) {
        std::lock_guard<std::mutex> lock(handlers_mutex_);

        auto it = handlers_.find(envelope.type);
        if (it != handlers_.end()) {
            for (const auto& wrapper : it->second) {
                try {
                    wrapper.handler(envelope.payload);
                } catch (const std::exception& e) {
                    // Log error but continue processing
                    // In production, this would be logged properly
                }
            }
        }
    }

    config config_;
    mutable std::mutex bus_mutex_;
    mutable std::mutex queue_mutex_;
    mutable std::mutex handlers_mutex_;

    std::priority_queue<event_envelope> event_queue_;
    std::unordered_map<std::type_index, std::vector<event_handler_wrapper>> handlers_;

    std::vector<std::thread> workers_;
    std::condition_variable queue_cv_;

    std::atomic<bool> is_running_;
    std::atomic<bool> stop_requested_;

    std::atomic<uint64_t> total_events_published_;
    std::atomic<uint64_t> total_events_processed_;
    std::atomic<uint64_t> total_events_dropped_;
};

} // namespace monitoring_system