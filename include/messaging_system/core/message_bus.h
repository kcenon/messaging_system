#pragma once

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>
#include "messaging_container.h"
#include "topic_router.h"
#include <memory>
#include <atomic>

namespace messaging {

using common::VoidResult;

class MessageBus {
    std::shared_ptr<common::interfaces::IExecutor> io_executor_;
    std::shared_ptr<common::interfaces::IExecutor> work_executor_;
    std::shared_ptr<TopicRouter> router_;
    std::atomic<bool> running_{false};

public:
    MessageBus(
        std::shared_ptr<common::interfaces::IExecutor> io_executor,
        std::shared_ptr<common::interfaces::IExecutor> work_executor,
        std::shared_ptr<TopicRouter> router
    );

    // Asynchronous publication
    VoidResult publish_async(MessagingContainer msg);

    // Synchronous publication (blocking)
    VoidResult publish_sync(const MessagingContainer& msg);

    // Subscription
    using SubscriberCallback = std::function<VoidResult(const MessagingContainer&)>;
    common::Result<uint64_t> subscribe(const std::string& topic, SubscriberCallback callback);

    // Unsubscribe
    VoidResult unsubscribe(uint64_t subscription_id);

    // Lifecycle
    VoidResult start();
    VoidResult stop();
    bool is_running() const { return running_.load(); }
};

} // namespace messaging
