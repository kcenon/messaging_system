#pragma once

#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/core/message_bus.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

namespace kcenon::messaging::testing {

/**
 * @brief Wait for a condition with timeout
 */
template<typename Predicate>
bool wait_for_condition(Predicate&& pred, std::chrono::milliseconds timeout = std::chrono::milliseconds{1000}) {
    auto start = std::chrono::steady_clock::now();

    while (!pred()) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > timeout) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    return true;
}

/**
 * @brief Create a test message
 */
inline message create_test_message(const std::string& topic, const std::string& content = "test") {
    auto msg_result = message_builder()
        .topic(topic)
        .type(message_type::event)
        .priority(message_priority::normal)
        .source("test_source")
        .build();

    if (!msg_result.is_ok()) {
        throw std::runtime_error("Failed to create test message");
    }

    return msg_result.unwrap();
}

/**
 * @brief Message counter for testing
 */
class MessageCounter {
    std::atomic<int> count_{0};
    std::atomic<int> error_count_{0};

public:
    void increment() {
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    void increment_error() {
        error_count_.fetch_add(1, std::memory_order_relaxed);
    }

    int count() const {
        return count_.load(std::memory_order_relaxed);
    }

    int error_count() const {
        return error_count_.load(std::memory_order_relaxed);
    }

    void reset() {
        count_.store(0, std::memory_order_relaxed);
        error_count_.store(0, std::memory_order_relaxed);
    }
};

/**
 * @brief Create a counting callback
 */
inline subscription_callback create_counting_callback(MessageCounter& counter) {
    return [&counter](const message& msg) -> common::VoidResult {
        counter.increment();
        return common::ok();
    };
}

/**
 * @brief Create a callback that stores messages
 */
inline subscription_callback create_storing_callback(std::vector<message>& storage, std::mutex& mutex) {
    return [&storage, &mutex](const message& msg) -> common::VoidResult {
        std::lock_guard lock(mutex);
        storage.push_back(msg);
        return common::ok();
    };
}

} // namespace kcenon::messaging::testing
