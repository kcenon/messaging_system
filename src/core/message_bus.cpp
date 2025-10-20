#include "messaging_system/core/message_bus.h"
#include "messaging_system/error_codes.h"
#include <kcenon/common/patterns/error_info.h>

namespace messaging {

MessageBus::MessageBus(
    std::shared_ptr<common::IExecutor> io_executor,
    std::shared_ptr<common::IExecutor> work_executor,
    std::shared_ptr<TopicRouter> router
)
    : io_executor_(std::move(io_executor))
    , work_executor_(std::move(work_executor))
    , router_(std::move(router))
{}

common::Result<void> MessageBus::start() {
    if (running_.load()) {
        return common::VoidResult::error(
            common::error_info{
                error::INVALID_MESSAGE,
                "MessageBus already running",
                "MessageBus::start",
                ""
            }
        );
    }

    running_.store(true);
    return common::VoidResult::ok();
}

common::Result<void> MessageBus::stop() {
    if (!running_.load()) {
        return common::VoidResult::error(
            common::error_info{
                error::INVALID_MESSAGE,
                "MessageBus not running",
                "MessageBus::stop",
                ""
            }
        );
    }

    running_.store(false);
    return common::VoidResult::ok();
}

common::Result<void> MessageBus::publish_async(MessagingContainer msg) {
    if (!running_.load()) {
        return common::VoidResult::error(
            common::error_info{
                error::PUBLICATION_FAILED,
                "MessageBus not running",
                "MessageBus::publish_async",
                ""
            }
        );
    }

    // TODO: Implement actual async publication via executor
    return router_->route(msg);
}

common::Result<void> MessageBus::publish_sync(const MessagingContainer& msg) {
    if (!running_.load()) {
        return common::VoidResult::error(
            common::error_info{
                error::PUBLICATION_FAILED,
                "MessageBus not running",
                "MessageBus::publish_sync",
                ""
            }
        );
    }

    return router_->route(msg);
}

common::Result<uint64_t> MessageBus::subscribe(const std::string& topic, SubscriberCallback callback) {
    if (!running_.load()) {
        return common::Result<uint64_t>::error(
            common::error_info{
                error::SUBSCRIPTION_FAILED,
                "MessageBus not running",
                "MessageBus::subscribe",
                ""
            }
        );
    }

    return router_->subscribe(topic, std::move(callback));
}

common::Result<void> MessageBus::unsubscribe(uint64_t subscription_id) {
    return router_->unsubscribe(subscription_id);
}

} // namespace messaging
