#include "kcenon/messaging/services/container/container_service.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace kcenon::messaging::services::container {

    container_service::container_service(const container_config& config)
        : config_(config) {
    }

    bool container_service::initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != service_state::uninitialized) {
            return false;
        }

        state_ = service_state::initializing;

        try {
            // Initialize container service components
            // In a full implementation, this would set up serialization libraries,
            // compression algorithms, etc.

            state_ = service_state::running;
            return true;
        } catch (const std::exception& e) {
            state_ = service_state::error;
            return false;
        }
    }

    void container_service::shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == service_state::running) {
            state_ = service_state::stopping;
            // Cleanup resources
            state_ = service_state::stopped;
        }
    }

    void container_service::handle_message(const core::message& msg) {
        if (state_ != service_state::running) {
            return;
        }

        const std::string& topic = msg.payload.topic;

        if (topic == "container.serialize") {
            process_serialize_request(msg);
        } else if (topic == "container.deserialize") {
            process_deserialize_request(msg);
        } else if (topic == "container.validate") {
            process_validate_request(msg);
        } else if (topic == "container.compress") {
            process_compress_request(msg);
        } else if (topic == "container.decompress") {
            process_decompress_request(msg);
        }
    }

    bool container_service::can_handle_topic(const std::string& topic) const {
        return std::find(supported_topics_.begin(), supported_topics_.end(), topic)
               != supported_topics_.end();
    }

    bool container_service::is_healthy() const {
        return state_ == service_state::running;
    }

    bool container_service::serialize_payload(const core::message_payload& payload,
                                             std::vector<uint8_t>& output) {
        try {
            // Simplified serialization - in production would use proper serialization library
            std::ostringstream oss;
            oss << "topic:" << payload.topic << ";";

            for (const auto& [key, value] : payload.data) {
                oss << key << ":";
                std::visit([&oss](const auto& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        oss << v;
                    } else if constexpr (std::is_same_v<T, int64_t>) {
                        oss << v;
                    } else if constexpr (std::is_same_v<T, double>) {
                        oss << v;
                    } else if constexpr (std::is_same_v<T, bool>) {
                        oss << (v ? "true" : "false");
                    }
                }, value);
                oss << ";";
            }

            std::string serialized = oss.str();
            output.assign(serialized.begin(), serialized.end());

            stats_.serializations.fetch_add(1);
            return true;
        } catch (const std::exception& e) {
            stats_.errors.fetch_add(1);
            return false;
        }
    }

    bool container_service::deserialize_payload(const std::vector<uint8_t>& input,
                                               core::message_payload& payload) {
        try {
            // Simplified deserialization
            std::string data(input.begin(), input.end());
            // In production, would use proper parsing

            stats_.deserializations.fetch_add(1);
            return true;
        } catch (const std::exception& e) {
            stats_.errors.fetch_add(1);
            return false;
        }
    }

    bool container_service::validate_payload(const core::message_payload& payload) {
        try {
            // Basic validation
            bool valid = !payload.topic.empty();
            if (config_.max_message_size > 0) {
                // Estimate payload size
                size_t estimated_size = payload.topic.size() + payload.binary_data.size();
                for (const auto& [key, value] : payload.data) {
                    estimated_size += key.size() + 32;  // Rough estimate
                }
                valid = valid && (estimated_size <= config_.max_message_size);
            }

            stats_.validations.fetch_add(1);
            return valid;
        } catch (const std::exception& e) {
            stats_.errors.fetch_add(1);
            return false;
        }
    }

    bool container_service::compress_data(const std::vector<uint8_t>& input,
                                         std::vector<uint8_t>& output) {
        try {
            // Simplified compression - in production would use zlib/lz4/etc.
            if (!config_.enable_compression) {
                output = input;
                return true;
            }

            // Dummy compression: just copy for now
            output = input;

            stats_.compressions.fetch_add(1);
            return true;
        } catch (const std::exception& e) {
            stats_.errors.fetch_add(1);
            return false;
        }
    }

    bool container_service::decompress_data(const std::vector<uint8_t>& input,
                                           std::vector<uint8_t>& output) {
        try {
            // Dummy decompression: just copy for now
            output = input;
            return true;
        } catch (const std::exception& e) {
            stats_.errors.fetch_add(1);
            return false;
        }
    }

    void container_service::process_serialize_request(const core::message& msg) {
        // Implementation would extract payload from message and serialize it
        stats_.serializations.fetch_add(1);
    }

    void container_service::process_deserialize_request(const core::message& msg) {
        // Implementation would extract binary data and deserialize it
        stats_.deserializations.fetch_add(1);
    }

    void container_service::process_validate_request(const core::message& msg) {
        // Implementation would validate the message payload
        stats_.validations.fetch_add(1);
    }

    void container_service::process_compress_request(const core::message& msg) {
        // Implementation would compress the message data
        stats_.compressions.fetch_add(1);
    }

    void container_service::process_decompress_request(const core::message& msg) {
        // Implementation would decompress the message data
        stats_.compressions.fetch_add(1);
    }

    // Adapter implementation
    void container_service_adapter::register_with_bus(core::message_bus* bus) {
        if (!bus || !container_service_) {
            return;
        }

        bus_ = bus;

        // Subscribe to all topics this service can handle
        for (const std::string& topic : {
            "container.serialize", "container.deserialize", "container.validate",
            "container.compress", "container.decompress"
        }) {
            bus_->subscribe(topic, [this](const core::message& msg) {
                if (container_service_) {
                    container_service_->handle_message(msg);
                }
            });
        }
    }

} // namespace kcenon::messaging::services::container
