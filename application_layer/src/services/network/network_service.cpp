#include "kcenon/messaging/services/network/network_service.h"
#include <stdexcept>

namespace kcenon::messaging::services::network {

    network_service::network_service(const network_config& config)
        : config_(config) {
    }

    bool network_service::initialize() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == service_state::running) {
            return true;
        }
        if (state_ != service_state::uninitialized) {
            return false;
        }

        state_ = service_state::initializing;

        try {
            // Initialize network components
            // In a full implementation, this would set up sockets, SSL context, etc.

            state_ = service_state::running;
            return true;
        } catch (const std::exception& e) {
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
        stats_.messages_received.fetch_add(1);

        if (topic == "network.send") {
            process_send_request(msg);
        } else if (topic == "network.broadcast") {
            process_broadcast_request(msg);
        } else if (topic == "network.connect") {
            process_connect_request(msg);
        } else if (topic == "network.disconnect") {
            process_disconnect_request(msg);
        }
    }

    bool network_service::can_handle_topic(const std::string& topic) const {
        return topic == "network.send"
            || topic == "network.broadcast"
            || topic == "network.connect"
            || topic == "network.disconnect";
    }

    bool network_service::is_healthy() const {
        return state_ == service_state::running;
    }

    bool network_service::send_message(const std::string& destination, const core::message& msg) {
        if (state_ != service_state::running || destination.empty()) {
            return false;
        }

        try {
            (void)msg;
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
        std::string destination = msg.payload.get<std::string>("destination", "");
        if (destination.empty()) {
            destination = msg.payload.get<std::string>("recipient", "");
        }
        if (!destination.empty()) {
            send_message(destination, msg);
        }
    }

    void network_service::process_broadcast_request(const core::message& msg) {
        broadcast_message(msg);
    }

    bool network_service::connect_client(const std::string& client_id) {
        if (client_id.empty() || state_ != service_state::running) {
            return false;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (active_connections_.size() >= config_.max_connections) {
            stats_.failed_connections.fetch_add(1);
            return false;
        }

        auto [_, inserted] = active_connections_.insert(client_id);
        if (inserted) {
            stats_.active_connections.fetch_add(1);
        }
        return inserted;
    }

    bool network_service::disconnect_client(const std::string& client_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto erased = active_connections_.erase(client_id);
        if (erased > 0) {
            auto current = stats_.active_connections.load();
            if (current > 0) {
                stats_.active_connections.fetch_sub(1);
            }
        }
        return erased > 0;
    }

    void network_service::process_connect_request(const core::message& msg) {
        const std::string client_id = msg.payload.get<std::string>("client_id", msg.metadata.sender);
        if (!client_id.empty()) {
            connect_client(client_id);
        }
    }

    void network_service::process_disconnect_request(const core::message& msg) {
        const std::string client_id = msg.payload.get<std::string>("client_id", msg.metadata.sender);
        if (!client_id.empty()) {
            disconnect_client(client_id);
        }
    }

    // Adapter implementation
    void network_service_adapter::register_with_bus(core::message_bus* bus) {
        if (!bus || !network_service_) {
            return;
        }

        bus_ = bus;

        auto subscribe_topic = [this](const std::string& topic) {
            bus_->subscribe(topic, [this, topic](const core::message& msg) {
                if (network_service_) {
                    network_service_->handle_message(msg);
                }
                if (bus_) {
                    core::message response;
                    response.payload.topic = "network.response";
                    response.payload.data["operation"] = topic;
                    response.payload.data["status"] = std::string("processed");
                    response.payload.data["destination"] = msg.payload.get<std::string>("destination", "");
                    response.metadata.priority = msg.metadata.priority;
                    bus_->publish(response);
                }
            });
        };

        for (const std::string& topic : {
            "network.send", "network.broadcast", "network.connect", "network.disconnect"
        }) {
            subscribe_topic(topic);
        }
    }

} // namespace kcenon::messaging::services::network
