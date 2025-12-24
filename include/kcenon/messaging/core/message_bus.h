#pragma once

#include <kcenon/common/patterns/result.h>
#include "message.h"
#include "message_queue.h"
#include "topic_router.h"
#include "../backends/backend_interface.h"
#include "../adapters/transport_interface.h"
#include <memory>
#include <atomic>
#include <vector>
#include <future>
#include <unordered_map>

namespace kcenon::messaging {

/**
 * @enum transport_mode
 * @brief Defines how message_bus handles message routing
 */
enum class transport_mode {
    local,   ///< Local-only: messages are routed only to local subscribers
    remote,  ///< Remote-only: messages are sent only via transport
    hybrid   ///< Hybrid: messages are routed both locally and remotely
};

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

    // Transport configuration
    transport_mode mode = transport_mode::local;
    std::shared_ptr<adapters::transport_interface> transport = nullptr;
    std::string local_node_id;  ///< Unique identifier for distributed routing
};

// Forward declaration for friend class
class message_processing_job;

/**
 * @class message_bus
 * @brief Central message hub for publish-subscribe messaging
 */
class message_bus {
    friend class message_processing_job;

    message_bus_config config_;
    std::shared_ptr<backend_interface> backend_;
    std::unique_ptr<message_queue> queue_;
    std::unique_ptr<topic_router> router_;
    std::unique_ptr<message_queue> dead_letter_queue_;
    std::shared_ptr<adapters::transport_interface> transport_;
    std::atomic<bool> running_{false};
    std::vector<std::future<void>> workers_;

    // Statistics
    struct statistics {
        std::atomic<uint64_t> messages_published{0};
        std::atomic<uint64_t> messages_processed{0};
        std::atomic<uint64_t> messages_failed{0};
        std::atomic<uint64_t> messages_dropped{0};
        std::atomic<uint64_t> messages_sent_remote{0};
        std::atomic<uint64_t> messages_received_remote{0};
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
        uint64_t messages_sent_remote;
        uint64_t messages_received_remote;
    };

    statistics_snapshot get_statistics() const;
    void reset_statistics();

    // Transport accessors
    transport_mode get_transport_mode() const { return config_.mode; }
    bool has_transport() const { return transport_ != nullptr; }
    bool is_transport_connected() const;

private:
    void process_messages();
    void process_single_message();
    common::VoidResult handle_message(const message& msg);
    void start_workers();
    void stop_workers();

    // Transport helpers
    void setup_transport_handlers();
    void handle_remote_message(const message& msg);
    common::VoidResult send_to_remote(const message& msg);
    common::VoidResult route_local(const message& msg);
};

} // namespace kcenon::messaging
