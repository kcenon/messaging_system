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

        // TODO: Register message receive callback
        // Current limitation: network_system::messaging_server does not expose
        // a callback mechanism for received messages. The on_receive method in
        // messaging_session is private and cannot be overridden.
        //
        // To complete this integration, network_system needs to provide one of:
        // 1. Public set_receive_callback() on messaging_server
        // 2. Virtual on_receive() method in messaging_session that can be overridden
        // 3. Factory method to inject custom session types
        //
        // Current implementation only starts the TCP server but cannot process
        // received messages. Messages sent by clients will be received by the
        // network layer but not forwarded to the message_bus.

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
    // TODO: Implement message deserialization and routing
    //
    // This method should:
    // 1. Deserialize data into MessagingContainer
    // 2. Submit process_message() to work_executor for background processing
    // 3. Handle deserialization errors
    //
    // Current status: NOT CALLED (see start() method TODO comment)

    (void)session;  // Unused
    (void)data;     // Unused

    return VoidResult::error(
        error::NETWORK_ERROR,
        "on_message_received not yet implemented - callback mechanism unavailable"
    );
}

VoidResult MessagingNetworkBridge::process_message(
    std::shared_ptr<network::messaging_session> session,
    const MessagingContainer& msg
) {
    // TODO: Implement message processing
    //
    // This method should:
    // 1. Route message through message_bus
    // 2. Collect response from subscribers
    // 3. Call send_response() with the response
    // 4. Handle routing errors
    //
    // Current status: NOT CALLED (see start() method TODO comment)

    (void)session;  // Unused
    (void)msg;      // Unused

    return VoidResult::error(
        error::NETWORK_ERROR,
        "process_message not yet implemented - callback mechanism unavailable"
    );
}

VoidResult MessagingNetworkBridge::send_response(
    std::shared_ptr<network::messaging_session> session,
    const MessagingContainer& response
) {
    // TODO: Implement response sending
    //
    // This method should:
    // 1. Serialize MessagingContainer to std::vector<uint8_t>
    // 2. Call session->send_packet(std::move(serialized_data))
    // 3. Handle serialization and send errors
    //
    // Current status: NOT CALLED (see start() method TODO comment)

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

    // TODO: Implement actual serialization and send
    (void)response;  // Unused

    return VoidResult::error(
        error::NETWORK_ERROR,
        "send_response not yet implemented - serialization not implemented"
    );
}

} // namespace messaging

#endif // HAS_NETWORK_SYSTEM
