#include "kcenon/messaging/services/container/container_service.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <sstream>
#include <stdexcept>

namespace kcenon::messaging::services::container {

    namespace {
        constexpr uint32_t kSerializationMagic = 0x4B434E31;
        constexpr std::array<uint8_t, 4> kRleHeader{'R', 'L', 'E', '1'};

        enum class serialized_type : uint8_t {
            string = 0,
            int64 = 1,
            double_type = 2,
            boolean = 3,
            bytes = 4
        };

        void write_uint32(std::vector<uint8_t>& buffer, uint32_t value) {
            for (int i = 0; i < 4; ++i) {
                buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
            }
        }

        bool read_uint32(const std::vector<uint8_t>& input, size_t& offset, uint32_t& value) {
            if (offset + 4 > input.size()) {
                return false;
            }
            value = 0;
            for (int i = 0; i < 4; ++i) {
                value |= static_cast<uint32_t>(input[offset + i]) << (i * 8);
            }
            offset += 4;
            return true;
        }

        void write_uint64(std::vector<uint8_t>& buffer, uint64_t value) {
            for (int i = 0; i < 8; ++i) {
                buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
            }
        }

        bool read_uint64(const std::vector<uint8_t>& input, size_t& offset, uint64_t& value) {
            if (offset + 8 > input.size()) {
                return false;
            }
            value = 0;
            for (int i = 0; i < 8; ++i) {
                value |= static_cast<uint64_t>(input[offset + i]) << (i * 8);
            }
            offset += 8;
            return true;
        }

        void write_string(std::vector<uint8_t>& buffer, const std::string& str) {
            write_uint32(buffer, static_cast<uint32_t>(str.size()));
            buffer.insert(buffer.end(), str.begin(), str.end());
        }

        bool read_string(const std::vector<uint8_t>& input, size_t& offset, std::string& out) {
            uint32_t length = 0;
            if (!read_uint32(input, offset, length)) {
                return false;
            }
            if (offset + length > input.size()) {
                return false;
            }
            out.assign(reinterpret_cast<const char*>(input.data() + offset), length);
            offset += length;
            return true;
        }

        void write_bytes(std::vector<uint8_t>& buffer, const std::vector<uint8_t>& data) {
            write_uint32(buffer, static_cast<uint32_t>(data.size()));
            buffer.insert(buffer.end(), data.begin(), data.end());
        }

        bool read_bytes(const std::vector<uint8_t>& input, size_t& offset, std::vector<uint8_t>& data) {
            uint32_t length = 0;
            if (!read_uint32(input, offset, length)) {
                return false;
            }
            if (offset + length > input.size()) {
                return false;
            }
            using diff_t = std::vector<uint8_t>::difference_type;
            data.assign(input.begin() + static_cast<diff_t>(offset),
                        input.begin() + static_cast<diff_t>(offset + length));
            offset += length;
            return true;
        }

        void write_double(std::vector<uint8_t>& buffer, double value) {
            uint64_t bits = 0;
            static_assert(sizeof(bits) == sizeof(value));
            std::memcpy(&bits, &value, sizeof(double));
            write_uint64(buffer, bits);
        }

        bool read_double(const std::vector<uint8_t>& input, size_t& offset, double& value) {
            uint64_t bits = 0;
            if (!read_uint64(input, offset, bits)) {
                return false;
            }
            std::memcpy(&value, &bits, sizeof(double));
            return true;
        }

        bool is_rle_encoded(const std::vector<uint8_t>& input) {
            return input.size() > kRleHeader.size() &&
                   std::equal(kRleHeader.begin(), kRleHeader.end(), input.begin());
        }
    } // namespace

    container_service::container_service(const container_config& config)
        : config_(config) {
    }

    bool container_service::initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == service_state::running) {
            return true;
        }
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
            std::vector<uint8_t> buffer;
            buffer.reserve(256);

            write_uint32(buffer, kSerializationMagic);
            write_string(buffer, payload.topic);
            write_uint32(buffer, static_cast<uint32_t>(payload.data.size()));

            for (const auto& [key, value] : payload.data) {
                write_string(buffer, key);
                std::visit([&buffer](const auto& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        buffer.push_back(static_cast<uint8_t>(serialized_type::string));
                        write_string(buffer, v);
                    } else if constexpr (std::is_same_v<T, int64_t>) {
                        buffer.push_back(static_cast<uint8_t>(serialized_type::int64));
                        write_uint64(buffer, static_cast<uint64_t>(v));
                    } else if constexpr (std::is_same_v<T, double>) {
                        buffer.push_back(static_cast<uint8_t>(serialized_type::double_type));
                        write_double(buffer, v);
                    } else if constexpr (std::is_same_v<T, bool>) {
                        buffer.push_back(static_cast<uint8_t>(serialized_type::boolean));
                        buffer.push_back(static_cast<uint8_t>(v ? 1 : 0));
                    } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                        buffer.push_back(static_cast<uint8_t>(serialized_type::bytes));
                        write_bytes(buffer, v);
                    }
                }, value);
            }

            write_bytes(buffer, payload.binary_data);

            output = std::move(buffer);
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
            if (input.size() < sizeof(uint32_t)) {
                return false;
            }

            size_t offset = 0;
            uint32_t magic = 0;
            if (!read_uint32(input, offset, magic) || magic != kSerializationMagic) {
                return false;
            }

            std::string topic;
            if (!read_string(input, offset, topic)) {
                return false;
            }

            payload.topic = std::move(topic);
            payload.data.clear();

            uint32_t data_count = 0;
            if (!read_uint32(input, offset, data_count)) {
                return false;
            }

            for (uint32_t i = 0; i < data_count; ++i) {
                std::string key;
                if (!read_string(input, offset, key)) {
                    return false;
                }
                if (offset >= input.size()) {
                    return false;
                }

                const auto type = static_cast<serialized_type>(input[offset++]);
                core::message_value value;

                switch (type) {
                    case serialized_type::string: {
                        std::string str_value;
                        if (!read_string(input, offset, str_value)) {
                            return false;
                        }
                        value = std::move(str_value);
                        break;
                    }
                    case serialized_type::int64: {
                        uint64_t raw = 0;
                        if (!read_uint64(input, offset, raw)) {
                            return false;
                        }
                        value = static_cast<int64_t>(raw);
                        break;
                    }
                    case serialized_type::double_type: {
                        double dbl = 0.0;
                        if (!read_double(input, offset, dbl)) {
                            return false;
                        }
                        value = dbl;
                        break;
                    }
                    case serialized_type::boolean: {
                        if (offset >= input.size()) {
                            return false;
                        }
                        value = input[offset++] != 0;
                        break;
                    }
                    case serialized_type::bytes: {
                        std::vector<uint8_t> bytes;
                        if (!read_bytes(input, offset, bytes)) {
                            return false;
                        }
                        value = std::move(bytes);
                        break;
                    }
                    default:
                        return false;
                }

                payload.data.emplace(std::move(key), std::move(value));
            }

            if (!read_bytes(input, offset, payload.binary_data)) {
                return false;
            }

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
            if (!config_.enable_compression || input.empty()) {
                output = input;
                return true;
            }

            std::vector<uint8_t> compressed;
            compressed.reserve(input.size() / 2 + kRleHeader.size());
            compressed.insert(compressed.end(), kRleHeader.begin(), kRleHeader.end());

            size_t i = 0;
            while (i < input.size()) {
                uint8_t value = input[i];
                uint8_t count = 1;

                while ((i + count) < input.size() && input[i + count] == value && count < 0xFF) {
                    ++count;
                }

                compressed.push_back(count);
                compressed.push_back(value);
                i += count;
            }

            if (compressed.size() >= input.size()) {
                output = input;
            } else {
                output = std::move(compressed);
            }

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
            if (input.empty()) {
                output.clear();
                return true;
            }

            if (!is_rle_encoded(input)) {
                output = input;
                return true;
            }

            output.clear();
            size_t index = kRleHeader.size();
            while (index < input.size()) {
                if (index + 1 >= input.size()) {
                    return false;
                }

                uint8_t count = input[index];
                uint8_t value = input[index + 1];
                index += 2;

                output.insert(output.end(), count, value);
            }

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
        auto subscribe_topic = [this](const std::string& topic) {
            bus_->subscribe(topic, [this, topic](const core::message& msg) {
                if (container_service_) {
                    container_service_->handle_message(msg);
                }
                if (bus_) {
                    core::message response;
                    response.payload.topic = "container.response";
                    response.payload.data["operation"] = topic;
                    response.payload.data["original_topic"] = msg.payload.topic;
                    response.payload.data["status"] = std::string("processed");
                    response.metadata.priority = msg.metadata.priority;
                    bus_->publish(response);
                }
            });
        };

        for (const std::string& topic : {
            "container.serialize", "container.deserialize", "container.validate",
            "container.compress", "container.decompress"
        }) {
            subscribe_topic(topic);
        }
    }

} // namespace kcenon::messaging::services::container
