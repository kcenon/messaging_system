#pragma once

#include <kcenon/common/patterns/result.h>
#include "message.h"
#include "message_queue.h"
#include "topic_router.h"
#include "../backends/backend_interface.h"
#include <memory>
#include <atomic>
#include <vector>
#include <future>
#include <unordered_map>

namespace kcenon::messaging {

/**
 * @struct message_bus_config
 * @brief Configuration for message bus
 */
struct message_bus_config {
    size_t queue_capacity = 10000;
    size_t worker_threads = 4;
    bool enable_priority_queue = true;
    bool enable_dead_letter_queue = true;
    bool enable_metrics = true;
    std::chrono::milliseconds processing_timeout{5000};
};

/**
 * @class message_bus
 * @brief Central message hub for publish-subscribe messaging
 */
class message_bus {
    message_bus_config config_;
    std::shared_ptr<backend_interface> backend_;
    std::unique_ptr<message_queue> queue_;
    std::unique_ptr<topic_router> router_;
    std::unique_ptr<message_queue> dead_letter_queue_;
    std::atomic<bool> running_{false};
    std::vector<std::future<void>> workers_;

    // Statistics
    struct statistics {
        std::atomic<uint64_t> messages_published{0};
        std::atomic<uint64_t> messages_processed{0};
        std::atomic<uint64_t> messages_failed{0};
        std::atomic<uint64_t> messages_dropped{0};
    } stats_;

public:
    explicit message_bus(
        std::shared_ptr<backend_interface> backend,
        message_bus_config config = {}
    );
    ~message_bus();

    // Lifecycle
    common::VoidResult start();
    common::VoidResult stop();
    bool is_running() const { return running_.load(); }

    // Configuration accessors
    size_t worker_count() const { return config_.worker_threads; }

    // Publishing
    common::VoidResult publish(const message& msg);
    common::VoidResult publish(const std::string& topic, message msg);

    // Subscription
    common::Result<uint64_t> subscribe(
        const std::string& topic_pattern,
        subscription_callback callback,
        message_filter filter = nullptr,
        int priority = 5
    );

    common::VoidResult unsubscribe(uint64_t subscription_id);

    // Request-Reply pattern
    common::Result<message> request(
        const message& request,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{5000}
    );

    // Statistics
    struct statistics_snapshot {
        uint64_t messages_published;
        uint64_t messages_processed;
        uint64_t messages_failed;
        uint64_t messages_dropped;
    };

    statistics_snapshot get_statistics() const;
    void reset_statistics();

private:
    void process_messages();
    common::VoidResult handle_message(const message& msg);
    void start_workers();
    void stop_workers();
};

} // namespace kcenon::messaging
