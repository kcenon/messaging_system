#pragma once

#include "../service_interface.h"
#include "../../core/message_bus.h"
#include <atomic>
#include <mutex>

namespace kcenon::messaging::services::network {

    // Network service configuration
    struct network_config {
        std::string listen_address = "0.0.0.0";
        uint16_t listen_port = 8080;
        size_t max_connections = 1000;
        std::chrono::seconds connection_timeout{30};
        bool enable_ssl = false;
    };

    // Network service implementation
    class network_service : public service_interface {
    public:
        explicit network_service(const network_config& config = {});
        ~network_service() override = default;

        // service_interface implementation
        bool initialize() override;
        void shutdown() override;
        service_state get_state() const override { return state_; }
        std::string get_service_name() const override { return "network_service"; }
        std::string get_service_version() const override { return "1.0.0"; }
        void handle_message(const core::message& msg) override;
        bool can_handle_topic(const std::string& topic) const override;
        bool is_healthy() const override;

        // Network-specific operations
        bool send_message(const std::string& destination, const core::message& msg);
        bool broadcast_message(const core::message& msg);

        // Statistics
        struct statistics {
            std::atomic<uint64_t> messages_sent{0};
            std::atomic<uint64_t> messages_received{0};
            std::atomic<uint64_t> active_connections{0};
            std::atomic<uint64_t> failed_connections{0};
        };

        const statistics& get_statistics() const { return stats_; }

    private:
        network_config config_;
        service_state state_ = service_state::uninitialized;
        mutable std::mutex mutex_;
        statistics stats_;

        void process_send_request(const core::message& msg);
        void process_broadcast_request(const core::message& msg);
    };

    // Network service adapter
    class network_service_adapter : public service_adapter {
    public:
        explicit network_service_adapter(std::shared_ptr<network_service> service)
            : service_adapter(service), network_service_(service) {}

        void register_with_bus(core::message_bus* bus) override;

    private:
        std::shared_ptr<network_service> network_service_;
        core::message_bus* bus_ = nullptr;
    };

} // namespace kcenon::messaging::services::network
