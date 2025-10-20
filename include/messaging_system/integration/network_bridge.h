#pragma once

#include <kcenon/network/core/messaging_server.h>
#include <kcenon/network/core/messaging_client.h>
#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>
#include "../core/message_bus.h"
#include "../core/messaging_container.h"
#include <memory>
#include <atomic>
#include <cstdint>

namespace messaging {

class MessagingNetworkBridge {
    std::shared_ptr<network::messaging_server> server_;
    std::shared_ptr<common::IExecutor> io_executor_;
    std::shared_ptr<common::IExecutor> work_executor_;
    std::shared_ptr<MessageBus> message_bus_;
    uint16_t port_;
    std::atomic<bool> running_{false};

public:
    MessagingNetworkBridge(
        uint16_t port,
        std::shared_ptr<common::IExecutor> io_executor,
        std::shared_ptr<common::IExecutor> work_executor,
        std::shared_ptr<MessageBus> message_bus
    );

    common::Result<void> start();
    common::Result<void> stop();
    bool is_running() const { return running_.load(); }

private:
    // Called on I/O thread
    common::Result<void> on_message_received(
        std::shared_ptr<network::messaging_session> session,
        std::vector<uint8_t> data
    );

    // Executed on worker thread
    common::Result<void> process_message(
        std::shared_ptr<network::messaging_session> session,
        const MessagingContainer& msg
    );

    common::Result<void> send_response(
        std::shared_ptr<network::messaging_session> session,
        const MessagingContainer& response
    );
};

} // namespace messaging
