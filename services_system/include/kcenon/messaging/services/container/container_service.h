#pragma once

#include "../service_interface.h"
#include "../../core/message_bus.h"
#include <unordered_map>
#include <vector>
#include <mutex>

namespace kcenon::messaging::services::container {

    // Container operation types
    enum class container_operation {
        serialize,
        deserialize,
        validate,
        transform,
        compress,
        decompress
    };

    // Container service configuration
    struct container_config {
        bool enable_compression = true;
        bool enable_validation = true;
        size_t max_message_size = 1024 * 1024;  // 1MB
        std::string default_format = "json";
    };

    // Container service implementation
    class container_service : public service_interface {
    public:
        explicit container_service(const container_config& config = {});
        ~container_service() override = default;

        // service_interface implementation
        bool initialize() override;
        void shutdown() override;
        service_state get_state() const override { return state_; }
        std::string get_service_name() const override { return "container_service"; }
        std::string get_service_version() const override { return "2.0.0"; }
        void handle_message(const core::message& msg) override;
        bool can_handle_topic(const std::string& topic) const override;
        bool is_healthy() const override;

        // Container-specific operations
        bool serialize_payload(const core::message_payload& payload, std::vector<uint8_t>& output);
        bool deserialize_payload(const std::vector<uint8_t>& input, core::message_payload& payload);
        bool validate_payload(const core::message_payload& payload);
        bool compress_data(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);
        bool decompress_data(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

        // Statistics
        struct statistics {
            std::atomic<uint64_t> serializations{0};
            std::atomic<uint64_t> deserializations{0};
            std::atomic<uint64_t> validations{0};
            std::atomic<uint64_t> compressions{0};
            std::atomic<uint64_t> errors{0};
        };

        const statistics& get_statistics() const { return stats_; }

    private:
        container_config config_;
        service_state state_ = service_state::uninitialized;
        mutable std::mutex mutex_;
        statistics stats_;

        // Supported topics
        std::vector<std::string> supported_topics_ = {
            "container.serialize",
            "container.deserialize",
            "container.validate",
            "container.compress",
            "container.decompress"
        };

        void process_serialize_request(const core::message& msg);
        void process_deserialize_request(const core::message& msg);
        void process_validate_request(const core::message& msg);
        void process_compress_request(const core::message& msg);
        void process_decompress_request(const core::message& msg);
    };

    // Container service adapter
    class container_service_adapter : public service_adapter {
    public:
        explicit container_service_adapter(std::shared_ptr<container_service> service)
            : service_adapter(service), container_service_(service) {}

        void register_with_bus(core::message_bus* bus) override;

    private:
        std::shared_ptr<container_service> container_service_;
        core::message_bus* bus_ = nullptr;
    };

} // namespace kcenon::messaging::services::container
