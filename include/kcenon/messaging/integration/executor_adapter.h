// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file executor_adapter.h
 * @brief Adapter for using common_system IExecutor with messaging_system
 *
 * This header provides an adapter that enables message processing using
 * the common_system's IExecutor interface, allowing for flexible threading
 * backend integration.
 */

#pragma once

#include <kcenon/common/interfaces/executor_interface.h>
#include <kcenon/common/patterns/result.h>
#include "../core/message.h"
#include <functional>
#include <memory>
#include <string>

namespace kcenon::messaging::integration {

/**
 * @class message_processor_job
 * @brief IJob implementation for processing messages
 *
 * This class wraps message processing logic in an IJob interface,
 * enabling execution via the common_system's IExecutor.
 *
 * Example usage:
 * @code
 * auto executor = container.resolve<common::interfaces::IExecutor>();
 *
 * auto job = std::make_unique<message_processor_job>(
 *     std::move(msg),
 *     [](const message& m) {
 *         // Process message
 *         return common::ok();
 *     }
 * );
 *
 * executor->execute(std::move(job));
 * @endcode
 */
class message_processor_job : public common::interfaces::IJob {
public:
    using handler_t = std::function<common::VoidResult(const message&)>;

    /**
     * @brief Construct a message processor job
     * @param msg Message to process
     * @param handler Handler function to process the message
     * @param priority Job priority (default: 0)
     */
    message_processor_job(message msg, handler_t handler, int priority = 0)
        : msg_(std::move(msg)),
          handler_(std::move(handler)),
          priority_(priority) {}

    /**
     * @brief Execute the job
     * @return VoidResult indicating success or failure
     */
    common::VoidResult execute() override {
        if (!handler_) {
            return common::make_error<std::monostate>(
                -1, "No handler registered", "messaging::message_processor_job");
        }
        return handler_(msg_);
    }

    /**
     * @brief Get the name of the job
     * @return Job name including topic info
     */
    std::string get_name() const override {
        return "message_processor[" + msg_.metadata().topic + "]";
    }

    /**
     * @brief Get the priority of the job
     * @return Job priority
     */
    int get_priority() const override { return priority_; }

    /**
     * @brief Get the message being processed
     * @return Const reference to the message
     */
    const message& get_message() const { return msg_; }

private:
    message msg_;
    handler_t handler_;
    int priority_;
};

/**
 * @class message_reply_job
 * @brief IJob implementation for request-reply pattern
 *
 * This job handles request processing and generates a reply message.
 */
class message_reply_job : public common::interfaces::IJob {
public:
    using handler_t = std::function<common::Result<message>(const message&)>;

    /**
     * @brief Construct a message reply job
     * @param request Request message
     * @param handler Handler function that returns a reply
     * @param reply_callback Callback to send the reply
     * @param priority Job priority
     */
    message_reply_job(
        message request,
        handler_t handler,
        std::function<void(common::Result<message>)> reply_callback,
        int priority = 0)
        : request_(std::move(request)),
          handler_(std::move(handler)),
          reply_callback_(std::move(reply_callback)),
          priority_(priority) {}

    common::VoidResult execute() override {
        if (!handler_) {
            auto error = common::make_error<message>(
                -1, "No handler registered", "messaging::message_reply_job");
            if (reply_callback_) {
                reply_callback_(std::move(error));
            }
            return common::make_error<std::monostate>(
                -1, "No handler registered", "messaging::message_reply_job");
        }

        auto result = handler_(request_);
        if (reply_callback_) {
            reply_callback_(std::move(result));
        }
        return common::ok();
    }

    std::string get_name() const override {
        return "message_reply[" + request_.metadata().topic + "]";
    }

    int get_priority() const override { return priority_; }

private:
    message request_;
    handler_t handler_;
    std::function<void(common::Result<message>)> reply_callback_;
    int priority_;
};

/**
 * @class executor_message_handler
 * @brief Adapter for processing messages via IExecutor
 *
 * This class provides a high-level interface for submitting message
 * processing jobs to an IExecutor implementation.
 */
class executor_message_handler {
public:
    /**
     * @brief Construct with an executor
     * @param executor Shared pointer to executor
     */
    explicit executor_message_handler(
        std::shared_ptr<common::interfaces::IExecutor> executor)
        : executor_(std::move(executor)) {}

    /**
     * @brief Process a message asynchronously
     * @param msg Message to process
     * @param handler Handler function
     * @param priority Job priority
     * @return Result containing future or error
     */
    common::Result<std::future<void>> process_async(
        message msg,
        message_processor_job::handler_t handler,
        int priority = 0) {

        if (!executor_) {
            return common::make_error<std::future<void>>(
                -1, "No executor available", "messaging::executor_message_handler");
        }

        auto job = std::make_unique<message_processor_job>(
            std::move(msg), std::move(handler), priority);

        return executor_->execute(std::move(job));
    }

    /**
     * @brief Process a request and get a reply asynchronously
     * @param request Request message
     * @param handler Handler function that returns a reply
     * @param priority Job priority
     * @return Result containing future for completion or error
     */
    common::Result<std::future<void>> request_async(
        message request,
        message_reply_job::handler_t handler,
        std::function<void(common::Result<message>)> reply_callback,
        int priority = 0) {

        if (!executor_) {
            return common::make_error<std::future<void>>(
                -1, "No executor available", "messaging::executor_message_handler");
        }

        auto job = std::make_unique<message_reply_job>(
            std::move(request), std::move(handler),
            std::move(reply_callback), priority);

        return executor_->execute(std::move(job));
    }

    /**
     * @brief Get the executor
     * @return Shared pointer to executor
     */
    std::shared_ptr<common::interfaces::IExecutor> get_executor() const {
        return executor_;
    }

    /**
     * @brief Check if executor is available and running
     * @return true if executor is available and running
     */
    bool is_available() const {
        return executor_ && executor_->is_running();
    }

private:
    std::shared_ptr<common::interfaces::IExecutor> executor_;
};

}  // namespace kcenon::messaging::integration
