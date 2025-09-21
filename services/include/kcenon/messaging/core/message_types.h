#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <variant>
#include <functional>
#include <atomic>

namespace kcenon::messaging::core {

    // Message priority levels
    enum class message_priority : int {
        low = 0,
        normal = 1,
        high = 2,
        critical = 3
    };

    // Message types
    enum class message_type {
        request,
        response,
        notification,
        broadcast
    };

    // Message status
    enum class message_status {
        pending,
        processing,
        completed,
        failed,
        timeout
    };

    // Message value type (supports various data types)
    using message_value = std::variant<
        std::string,
        int64_t,
        double,
        bool,
        std::vector<uint8_t>  // Binary data
    >;

    // Message metadata
    struct message_metadata {
        std::string id;
        std::string sender;
        std::string recipient;
        message_type type = message_type::request;
        message_priority priority = message_priority::normal;
        message_status status = message_status::pending;
        std::chrono::system_clock::time_point timestamp;
        std::chrono::milliseconds timeout{5000};  // 5 second default
        std::unordered_map<std::string, std::string> headers;

        message_metadata() : timestamp(std::chrono::system_clock::now()) {}
    };

    // Message payload
    struct message_payload {
        std::string topic;
        std::unordered_map<std::string, message_value> data;
        std::vector<uint8_t> binary_data;  // For large binary payloads

        // Helper methods
        template<typename T>
        void set(const std::string& key, const T& value) {
            data[key] = value;
        }

        template<typename T>
        T get(const std::string& key, const T& default_value = T{}) const {
            auto it = data.find(key);
            if (it != data.end()) {
                if (std::holds_alternative<T>(it->second)) {
                    return std::get<T>(it->second);
                }
            }
            return default_value;
        }
    };

    // Complete message structure
    class message {
    public:
        message_metadata metadata;
        message_payload payload;

        message() = default;

        message(const std::string& topic, const std::string& sender = "", const std::string& recipient = "") {
            payload.topic = topic;
            metadata.sender = sender;
            metadata.recipient = recipient;
            generate_id();
        }

        // Copy and move constructors
        message(const message&) = default;
        message(message&&) = default;
        message& operator=(const message&) = default;
        message& operator=(message&&) = default;

        // Convenience methods
        bool is_expired() const {
            auto now = std::chrono::system_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - metadata.timestamp);
            return elapsed > metadata.timeout;
        }

        void set_priority(message_priority prio) {
            metadata.priority = prio;
        }

        void set_timeout(std::chrono::milliseconds timeout) {
            metadata.timeout = timeout;
        }

    private:
        void generate_id() {
            // Simple ID generation - in production would use UUID
            static std::atomic<uint64_t> counter{0};
            metadata.id = "msg_" + std::to_string(counter.fetch_add(1));
        }
    };

    // Message handler function type
    using message_handler = std::function<void(const message&)>;

    // Response handler function type
    using response_handler = std::function<void(const message&, const message&)>;

} // namespace kcenon::messaging::core