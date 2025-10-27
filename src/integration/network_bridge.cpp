#include "messaging_system/integration/network_bridge.h"

#ifdef HAS_NETWORK_SYSTEM

#include "messaging_system/core/error_codes.h"
#include <stdexcept>

namespace messaging {

MessagingNetworkBridge::MessagingNetworkBridge(
    uint16_t port,
    std::shared_ptr<common::IExecutor> io_executor,
    std::shared_ptr<common::IExecutor> work_executor,
    std::shared_ptr<MessageBus> message_bus
) : port_(port),
    io_executor_(io_executor),
    work_executor_(work_executor),
    message_bus_(message_bus),
    running_(false) {

    if (!message_bus) {
        throw std::invalid_argument("message_bus cannot be null");
    }
    if (!io_executor) {
        throw std::invalid_argument("io_executor cannot be null");
    }
    if (!work_executor) {
        throw std::invalid_argument("work_executor cannot be null");
    }
}

VoidResult MessagingNetworkBridge::start() {
    if (running_.load()) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Network bridge is already running"
        );
    }

    try {
        // Create network_system server
        server_ = std::make_shared<network::messaging_server>("messaging_bridge_server");

        // Start the server on configured port
        auto result = server_->start_server(port_);

        if (!result) {
            return VoidResult::error(
                error::NETWORK_ERROR,
                std::string("Failed to start network server: ") + result.error().message
            );
        }

        running_.store(true);

        // Register receive callback
        auto self = shared_from_this();
        server_->set_receive_callback(
            [this, self](auto session, const auto& data) {
                on_message_received(session, std::vector<uint8_t>(data));
            }
        );

        // Optional: Register connection/disconnection callbacks for logging
        server_->set_connection_callback([](auto session) {
            // Log new connection
        });

        server_->set_disconnection_callback([](const auto& session_id) {
            // Log disconnection
        });

        return VoidResult::ok();

    } catch (const std::exception& e) {
        running_.store(false);
        return VoidResult::error(
            error::NETWORK_ERROR,
            std::string("Exception while starting network bridge: ") + e.what()
        );
    }
}

VoidResult MessagingNetworkBridge::stop() {
    if (!running_.load()) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Network bridge is not running"
        );
    }

    try {
        if (server_) {
            auto result = server_->stop_server();
            if (!result) {
                return VoidResult::error(
                    error::NETWORK_ERROR,
                    std::string("Failed to stop network server: ") + result.error().message
                );
            }
            server_.reset();
        }

        running_.store(false);
        return VoidResult::ok();

    } catch (const std::exception& e) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            std::string("Exception while stopping network bridge: ") + e.what()
        );
    }
}

VoidResult MessagingNetworkBridge::on_message_received(
    std::shared_ptr<network::messaging_session> session,
    std::vector<uint8_t> data
) {
    // 1. Deserialize data into MessagingContainer
    auto container_result = MessagingContainer::deserialize(data);
    if (!container_result) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Failed to deserialize message: " + container_result.error().message
        );
    }

    // 2. Submit to work_executor for background processing
    auto msg = std::move(container_result.value());
    work_executor_->execute([this, session, msg = std::move(msg)]() mutable {
        auto result = process_message(session, msg);
        if (!result) {
            // Log error but don't crash
            // Consider: send error response to client
        }
    });

    return VoidResult::ok();
}

VoidResult MessagingNetworkBridge::process_message(
    std::shared_ptr<network::messaging_session> session,
    const MessagingContainer& msg
) {
    // 1. Route message through message_bus
    auto publish_result = message_bus_->publish_sync(msg);
    if (!publish_result) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Failed to publish message: " + publish_result.error().message
        );
    }

    // 2. Collect response from subscribers
    // Note: Using request-response topic pattern (topic + "_response")
    // Subscribers should publish their responses to the response topic

    // 3. Send acknowledgment response back to client
    auto response_result = MessagingContainer::create(
        "messaging_bridge",
        msg.source(),
        msg.topic() + "_response"
    );

    if (response_result) {
        return send_response(session, response_result.value());
    }

    return VoidResult::ok();
}

VoidResult MessagingNetworkBridge::send_response(
    std::shared_ptr<network::messaging_session> session,
    const MessagingContainer& response
) {
    // Validate session
    if (!session) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Session is null"
        );
    }

    if (session->is_stopped()) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Session is stopped"
        );
    }

    // 1. Serialize MessagingContainer to std::vector<uint8_t>
    auto serialize_result = response.serialize();
    if (!serialize_result) {
        return VoidResult::error(
            error::NETWORK_ERROR,
            "Failed to serialize response: " + serialize_result.error().message
        );
    }

    // 2. Send via session
    session->send_packet(std::move(serialize_result.value()));

    return VoidResult::ok();
}

} // namespace messaging

#endif // HAS_NETWORK_SYSTEM
