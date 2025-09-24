#include "kcenon/messaging/services/network/network_service.h"
#include <stdexcept>

namespace kcenon::messaging::services::network {

    network_service::network_service(const network_config& config)
        : config_(config) {
    }

    bool network_service::initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != service_state::uninitialized) {
            return false;
        }

        state_ = service_state::initializing;

        try {
            // Initialize network components
            // In a full implementation, this would set up sockets, SSL context, etc.

            state_ = service_state::running;
            return true;
        } catch (const std::exception&) {
            state_ = service_state::error;
            return false;
        }
    }

    void network_service::shutdown() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == service_state::running) {
            state_ = service_state::stopping;
            // Close connections and cleanup
            state_ = service_state::stopped;
        }
    }

    void network_service::handle_message(const core::message& msg) {
        if (state_ != service_state::running) {
            return;
        }

        const std::string& topic = msg.payload.topic;

        if (topic == "network.send") {
            process_send_request(msg);
        } else if (topic == "network.broadcast") {
            process_broadcast_request(msg);
        }
    }

    bool network_service::can_handle_topic(const std::string& topic) const {
        return topic == "network.send" || topic == "network.broadcast";
    }

    bool network_service::is_healthy() const {
        return state_ == service_state::running;
    }

    bool network_service::send_message(const std::string& destination, const core::message&) {
        if (state_ != service_state::running) {
            return false;
        }

        try {
            // Simplified implementation - would actually send over network
            stats_.messages_sent.fetch_add(1);
            return true;
        } catch (const std::exception& e) {
            stats_.failed_connections.fetch_add(1);
            return false;
        }
    }

    bool network_service::broadcast_message(const core::message& msg) {
        if (state_ != service_state::running) {
            return false;
        }

        try {
            // Simplified implementation - would broadcast to all connections
            stats_.messages_sent.fetch_add(1);
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }

    void network_service::process_send_request(const core::message& msg) {
        // Extract destination from message and send
        std::string destination = msg.payload.get<std::string>("destination", "");
        if (!destination.empty()) {
            send_message(destination, msg);
        }
    }

    void network_service::process_broadcast_request(const core::message& msg) {
        broadcast_message(msg);
    }

    // Adapter implementation
    void network_service_adapter::register_with_bus(core::message_bus* bus) {
        if (!bus || !network_service_) {
            return;
        }

        bus_ = bus;

        // Subscribe to network topics
        bus_->subscribe("network.send", [this](const core::message& msg) {
            if (network_service_) {
                network_service_->handle_message(msg);
            }
        });

        bus_->subscribe("network.broadcast", [this](const core::message& msg) {
            if (network_service_) {
                network_service_->handle_message(msg);
            }
        });
    }

} // namespace kcenon::messaging::services::network
