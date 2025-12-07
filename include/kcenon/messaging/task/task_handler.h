// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file task_handler.h
 * @brief Task handler interface for distributed task queue system
 *
 * Defines the interface for task handlers that execute tasks in the
 * distributed task queue. Handlers can be implemented as classes
 * inheriting from task_handler_interface or as simple lambda functions.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/common/patterns/result.h>

#include <functional>
#include <memory>
#include <string>

namespace kcenon::messaging::task {

// Forward declaration
class task_context;

/**
 * @class task_handler_interface
 * @brief Abstract interface for task execution handlers
 *
 * Task handlers are responsible for executing tasks and returning results.
 * Implement this interface to create custom task handlers with full
 * lifecycle control including retry and failure hooks.
 *
 * @example
 * class email_handler : public task_handler_interface {
 * public:
 *     std::string name() const override { return "email.send"; }
 *
 *     common::Result<container_module::value_container> execute(
 *         const task& t,
 *         task_context& ctx
 *     ) override {
 *         auto email = t.payload().get_string("to");
 *         // Send email logic...
 *         container_module::value_container result;
 *         result.add("status", "sent");
 *         return common::ok(result);
 *     }
 * };
 */
class task_handler_interface {
public:
	virtual ~task_handler_interface() = default;

	/**
	 * @brief Get the handler name
	 *
	 * The handler name is used to match tasks to handlers.
	 * It should match the task_name field of tasks that this handler processes.
	 *
	 * @return Handler name (e.g., "email.send", "image.process")
	 */
	virtual std::string name() const = 0;

	/**
	 * @brief Execute the task
	 *
	 * Main execution method called by the worker when processing a task.
	 * The implementation should perform the actual work and return the result.
	 *
	 * @param t The task to execute
	 * @param ctx Execution context for progress tracking and logging
	 * @return Result containing the output value_container or error
	 */
	virtual common::Result<container_module::value_container> execute(
		const task& t,
		task_context& ctx) = 0;

	/**
	 * @brief Hook called before a retry attempt
	 *
	 * This optional hook is called when a task is about to be retried
	 * after a failure. Use it for cleanup or logging purposes.
	 *
	 * @param t The task being retried
	 * @param attempt The retry attempt number (1-based)
	 */
	virtual void on_retry(const task& t, size_t attempt) {
		(void)t;
		(void)attempt;
	}

	/**
	 * @brief Hook called when a task fails permanently
	 *
	 * This optional hook is called when a task has exhausted all retry
	 * attempts and failed permanently. Use it for error reporting or cleanup.
	 *
	 * @param t The failed task
	 * @param error The error message
	 */
	virtual void on_failure(const task& t, const std::string& error) {
		(void)t;
		(void)error;
	}

	/**
	 * @brief Hook called when a task succeeds
	 *
	 * This optional hook is called when a task completes successfully.
	 * Use it for logging or post-processing.
	 *
	 * @param t The completed task
	 * @param result The task result
	 */
	virtual void on_success(const task& t,
							const container_module::value_container& result) {
		(void)t;
		(void)result;
	}
};

/**
 * @brief Lambda-based simple task handler type
 *
 * A function type for simple task handlers that don't need the full
 * interface. Use this for quick, stateless task implementations.
 *
 * @example
 * simple_task_handler add_handler = [](const task& t, task_context& ctx) {
 *     auto a = t.payload().get_int("a");
 *     auto b = t.payload().get_int("b");
 *     container_module::value_container result;
 *     result.add("sum", a + b);
 *     return common::ok(result);
 * };
 */
using simple_task_handler = std::function<common::Result<container_module::value_container>(
	const task&,
	task_context&)>;

/**
 * @class lambda_task_handler
 * @brief Adapter to wrap a lambda as a task_handler_interface
 *
 * This class wraps a simple_task_handler lambda into the full
 * task_handler_interface, allowing lambdas to be used wherever
 * task handlers are expected.
 */
class lambda_task_handler : public task_handler_interface {
public:
	/**
	 * @brief Construct a lambda handler
	 * @param handler_name The name for this handler
	 * @param handler The lambda function to execute
	 */
	lambda_task_handler(std::string handler_name, simple_task_handler handler)
		: name_(std::move(handler_name)), handler_(std::move(handler)) {}

	std::string name() const override { return name_; }

	common::Result<container_module::value_container> execute(
		const task& t,
		task_context& ctx) override {
		if (!handler_) {
			return common::Result<container_module::value_container>(
				common::error_info{-1, "Handler function is null"});
		}
		return handler_(t, ctx);
	}

private:
	std::string name_;
	simple_task_handler handler_;
};

/**
 * @brief Create a task handler from a lambda
 * @param name Handler name
 * @param handler Lambda function
 * @return Unique pointer to task_handler_interface
 */
inline std::unique_ptr<task_handler_interface> make_handler(
	std::string name,
	simple_task_handler handler) {
	return std::make_unique<lambda_task_handler>(std::move(name),
												 std::move(handler));
}

}  // namespace kcenon::messaging::task
