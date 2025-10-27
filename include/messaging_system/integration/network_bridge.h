#pragma once

#ifdef HAS_NETWORK_SYSTEM
#include <network_system/compatibility.h>
#endif

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>
#include "../core/message_bus.h"
#include "../core/messaging_container.h"
#include <memory>
#include <atomic>
#include <cstdint>

namespace messaging {

#ifdef HAS_NETWORK_SYSTEM
// Alias for compatibility with network_system's exported namespace
namespace network = network_module;

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

    VoidResult start();
    VoidResult stop();
    bool is_running() const { return running_.load(); }

private:
    // Called on I/O thread
    VoidResult on_message_received(
        std::shared_ptr<network::messaging_session> session,
        std::vector<uint8_t> data
    );

    // Executed on worker thread
    VoidResult process_message(
        std::shared_ptr<network::messaging_session> session,
        const MessagingContainer& msg
    );

    VoidResult send_response(
        std::shared_ptr<network::messaging_session> session,
        const MessagingContainer& response
    );
};

#endif // HAS_NETWORK_SYSTEM

} // namespace messaging
