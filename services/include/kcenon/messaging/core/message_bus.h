#pragma once

#include "message_types.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <future>

namespace kcenon::messaging::core {

    // Forward declarations
    class message_router;
    class message_queue;
    class message_dispatcher;

    // Message bus configuration
    struct message_bus_config {
        size_t worker_threads = 4;
        size_t max_queue_size = 10000;
        std::chrono::milliseconds processing_timeout{30000};
        bool enable_priority_queue = true;
        bool enable_message_persistence = false;
        bool enable_metrics = true;
    };

    class message_bus {
    public:
        explicit message_bus(const message_bus_config& config = {});
        ~message_bus();

        // Disable copy/move for singleton-like behavior
        message_bus(const message_bus&) = delete;
        message_bus& operator=(const message_bus&) = delete;
        message_bus(message_bus&&) = delete;
        message_bus& operator=(message_bus&&) = delete;

        // Lifecycle management
        bool initialize();
        void shutdown();
        bool is_running() const { return running_.load(); }

        // Message publishing
        bool publish(const message& msg);
        bool publish(const std::string& topic, const message_payload& payload,
                    const std::string& sender = "");

        // Subscription management
        void subscribe(const std::string& topic, message_handler handler);
        void unsubscribe(const std::string& topic, const message_handler& handler);
        void unsubscribe_all(const std::string& topic);

        // Request-Response pattern
        std::future<message> request(const message& request_msg);
        void respond(const message& original_msg, const message& response_msg);

        // Topic and routing management
        std::vector<std::string> get_topics() const;
        size_t get_subscriber_count(const std::string& topic) const;

        // Statistics and monitoring
        struct statistics {
            std::atomic<uint64_t> messages_published{0};
            std::atomic<uint64_t> messages_processed{0};
            std::atomic<uint64_t> messages_failed{0};
            std::atomic<uint64_t> active_subscriptions{0};
            std::atomic<uint64_t> pending_requests{0};
        };

        // Return a copy of statistics values (atomics cannot be copied directly)
        struct statistics_snapshot {
            uint64_t messages_published;
            uint64_t messages_processed;
            uint64_t messages_failed;
            uint64_t active_subscriptions;
            uint64_t pending_requests;
        };

        statistics_snapshot get_statistics() const {
            return {
                stats_.messages_published.load(),
                stats_.messages_processed.load(),
                stats_.messages_failed.load(),
                stats_.active_subscriptions.load(),
                stats_.pending_requests.load()
            };
        }
        void reset_statistics();

    private:
        // Internal components
        std::unique_ptr<message_router> router_;
        std::unique_ptr<message_queue> queue_;
        std::unique_ptr<message_dispatcher> dispatcher_;

        // Configuration
        message_bus_config config_;

        // Threading
        std::vector<std::thread> worker_threads_;
        std::atomic<bool> running_{false};
        std::atomic<bool> shutdown_requested_{false};

        // Statistics
        mutable statistics stats_;

        // Worker thread function
        void worker_thread_func();

        // Internal helper methods
        bool validate_message(const message& msg) const;
        void update_statistics(const message& msg, bool success);
    };

} // namespace kcenon::messaging::core