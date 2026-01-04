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
 *
 * Supports both virtual polymorphism and CRTP (Curiously Recurring Template
 * Pattern) for performance-critical paths. CRTP eliminates virtual function
 * call overhead while type erasure wrappers maintain API compatibility.
 *
 * C++20 Concepts are provided for compile-time type validation of
 * task handlers, offering clearer error messages and self-documenting
 * interface requirements.
 */

#pragma once

#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_context.h>
#include <kcenon/common/patterns/result.h>

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>

namespace kcenon::messaging::task {

// Forward declaration
class task_context;

// =============================================================================
// C++20 Concepts for Task Handlers
// =============================================================================

/**
 * @concept TaskHandlerCallable
 * @brief A callable type that can handle task execution.
 *
 * Types satisfying this concept can be invoked with a task and task_context,
 * returning a Result containing a value_container. This replaces SFINAE-based
 * constraints with clearer compile-time error messages.
 *
 * @tparam F The callable type to validate
 *
 * Example usage:
 * @code
 * template<TaskHandlerCallable Handler>
 * void register_handler(const std::string& name, Handler&& handler) {
 *     // Handler is guaranteed to be callable with (const task&, task_context&)
 *     // and return common::Result<container_module::value_container>
 * }
 * @endcode
 */
template<typename F>
concept TaskHandlerCallable = std::invocable<F, const task&, task_context&> &&
	std::same_as<std::invoke_result_t<F, const task&, task_context&>,
				 common::Result<container_module::value_container>>;

/**
 * @concept TaskHandlerLike
 * @brief A type that satisfies the task handler interface requirements.
 *
 * Types satisfying this concept can be used as task handlers, providing
 * name() and execute() methods with appropriate signatures.
 *
 * @tparam T The type to validate
 *
 * Example usage:
 * @code
 * template<TaskHandlerLike Handler>
 * void add_handler(std::shared_ptr<Handler> handler) {
 *     // Handler is guaranteed to have name() and execute() methods
 * }
 * @endcode
 */
template<typename T>
concept TaskHandlerLike = requires(T t, const task& tsk, task_context& ctx) {
	{ t.name() } -> std::convertible_to<std::string>;
	{ t.execute(tsk, ctx) } -> std::same_as<common::Result<container_module::value_container>>;
};

/**
 * @concept TaskHandlerCRTPConcept
 * @brief Validates that a type can serve as a CRTP task handler
 *
 * Types satisfying this concept must provide name_impl and execute_impl
 * methods with the expected signatures.
 *
 * @tparam T The type to validate
 */
template<typename T>
concept TaskHandlerCRTPConcept = requires(T handler, const task& t, task_context& ctx) {
	{ handler.name_impl() } -> std::convertible_to<std::string>;
	{ handler.execute_impl(t, ctx) } -> std::same_as<common::Result<container_module::value_container>>;
};

// =============================================================================
// CRTP Base Class
// =============================================================================

/**
 * @class task_handler_base
 * @brief CRTP base class for task handlers
 *
 * Provides compile-time polymorphism for task handling, eliminating
 * virtual function call overhead. Derived classes implement name_impl,
 * execute_impl, and optional hook methods (on_retry_impl, on_failure_impl,
 * on_success_impl).
 *
 * @tparam Derived The derived class type (CRTP pattern)
 *
 * @example
 * class email_handler : public task_handler_base<email_handler> {
 *     friend class task_handler_base<email_handler>;
 *
 *     std::string name_impl() const { return "email.send"; }
 *
 *     common::Result<container_module::value_container> execute_impl(
 *         const task& t, task_context& ctx) {
 *         // Send email logic...
 *         container_module::value_container result;
 *         result.add("status", "sent");
 *         return common::ok(result);
 *     }
 * };
 */
template<typename Derived>
class task_handler_base {
public:
	/**
	 * @brief Get the handler name using compile-time dispatch
	 * @return Handler name
	 */
	[[nodiscard]] std::string name() const {
		return static_cast<const Derived*>(this)->name_impl();
	}

	/**
	 * @brief Execute the task using compile-time dispatch
	 * @param t The task to execute
	 * @param ctx Execution context
	 * @return Result containing the output or error
	 */
	common::Result<container_module::value_container> execute(
		const task& t, task_context& ctx) {
		return static_cast<Derived*>(this)->execute_impl(t, ctx);
	}

	/**
	 * @brief Hook called before a retry attempt
	 * @param t The task being retried
	 * @param attempt The retry attempt number (1-based)
	 */
	void on_retry(const task& t, size_t attempt) {
		static_cast<Derived*>(this)->on_retry_impl(t, attempt);
	}

	/**
	 * @brief Hook called when a task fails permanently
	 * @param t The failed task
	 * @param error The error message
	 */
	void on_failure(const task& t, const std::string& error) {
		static_cast<Derived*>(this)->on_failure_impl(t, error);
	}

	/**
	 * @brief Hook called when a task succeeds
	 * @param t The completed task
	 * @param result The task result
	 */
	void on_success(const task& t, const container_module::value_container& result) {
		static_cast<Derived*>(this)->on_success_impl(t, result);
	}

protected:
	// Default implementations for optional hooks
	void on_retry_impl(const task& /*t*/, size_t /*attempt*/) {}
	void on_failure_impl(const task& /*t*/, const std::string& /*error*/) {}
	void on_success_impl(const task& /*t*/, const container_module::value_container& /*result*/) {}

	~task_handler_base() = default;
	task_handler_base() = default;
	task_handler_base(const task_handler_base&) = default;
	task_handler_base& operator=(const task_handler_base&) = default;
	task_handler_base(task_handler_base&&) = default;
	task_handler_base& operator=(task_handler_base&&) = default;
};

// =============================================================================
// Virtual Interface (for type erasure and polymorphic containers)
// =============================================================================

/**
 * @class task_handler_interface
 * @brief Abstract interface for task execution handlers
 *
 * Task handlers are responsible for executing tasks and returning results.
 * Implement this interface to create custom task handlers with full
 * lifecycle control including retry and failure hooks.
 *
 * For performance-critical paths, consider using task_handler_base<>
 * with task_handler_wrapper for type erasure when heterogeneous
 * collections are needed.
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

/**
 * @brief Create a task handler from any callable satisfying TaskHandlerCallable
 *
 * This concept-constrained overload provides better compile-time error messages
 * when the callable doesn't match the expected signature.
 *
 * @tparam Handler A type satisfying TaskHandlerCallable concept
 * @param name Handler name
 * @param handler Any callable matching the task handler signature
 * @return Unique pointer to task_handler_interface
 *
 * @example
 * // Lambda with explicit types
 * auto handler = make_task_handler("add", [](const task& t, task_context& ctx)
 *     -> common::Result<container_module::value_container> {
 *     // Implementation
 * });
 *
 * // Function object
 * struct MyHandler {
 *     common::Result<container_module::value_container>
 *     operator()(const task& t, task_context& ctx) const { ... }
 * };
 * auto handler = make_task_handler("process", MyHandler{});
 */
template<TaskHandlerCallable Handler>
inline std::unique_ptr<task_handler_interface> make_task_handler(
	std::string name,
	Handler&& handler) {
	return std::make_unique<lambda_task_handler>(
		std::move(name),
		simple_task_handler(std::forward<Handler>(handler)));
}

// =============================================================================
// Type Erasure Wrapper for CRTP Handlers
// =============================================================================

/**
 * @class task_handler_wrapper
 * @brief Type erasure wrapper for CRTP task handlers
 *
 * Wraps a CRTP-based handler to implement task_handler_interface,
 * allowing CRTP handlers to be stored in heterogeneous containers while
 * still benefiting from compile-time dispatch within the wrapper.
 *
 * @tparam Handler The CRTP handler type
 *
 * @example
 * class my_handler : public task_handler_base<my_handler> {
 *     friend class task_handler_base<my_handler>;
 *
 *     std::string name_impl() const { return "my.task"; }
 *     common::Result<container_module::value_container> execute_impl(
 *         const task& t, task_context& ctx) { ... }
 * };
 *
 * // Store in polymorphic container
 * std::vector<std::shared_ptr<task_handler_interface>> handlers;
 * handlers.push_back(std::make_shared<task_handler_wrapper<my_handler>>());
 */
template<typename Handler>
class task_handler_wrapper : public task_handler_interface {
public:
	/**
	 * @brief Default construct with default-constructed handler
	 */
	task_handler_wrapper() : handler_() {}

	/**
	 * @brief Construct with a handler instance
	 * @param handler Handler to wrap (moved)
	 */
	explicit task_handler_wrapper(Handler handler)
		: handler_(std::move(handler)) {}

	/**
	 * @brief Construct handler in-place with arguments
	 * @tparam Args Constructor argument types
	 * @param args Arguments to forward to handler constructor
	 */
	template<typename... Args>
	explicit task_handler_wrapper(std::in_place_t, Args&&... args)
		: handler_(std::forward<Args>(args)...) {}

	std::string name() const override {
		return handler_.name();
	}

	common::Result<container_module::value_container> execute(
		const task& t, task_context& ctx) override {
		return handler_.execute(t, ctx);
	}

	void on_retry(const task& t, size_t attempt) override {
		handler_.on_retry(t, attempt);
	}

	void on_failure(const task& t, const std::string& error) override {
		handler_.on_failure(t, error);
	}

	void on_success(const task& t,
					const container_module::value_container& result) override {
		handler_.on_success(t, result);
	}

	/**
	 * @brief Get direct access to the wrapped handler
	 * @return Reference to the handler
	 */
	Handler& get() { return handler_; }

	/**
	 * @brief Get direct access to the wrapped handler (const)
	 * @return Const reference to the handler
	 */
	[[nodiscard]] const Handler& get() const { return handler_; }

private:
	Handler handler_;
};

/**
 * @brief Create a wrapped CRTP task handler
 * @tparam Handler The CRTP handler type
 * @tparam Args Constructor argument types
 * @param args Arguments to forward to handler constructor
 * @return Shared pointer to the wrapped handler
 */
template<typename Handler, typename... Args>
std::shared_ptr<task_handler_interface> make_crtp_task_handler(Args&&... args) {
	return std::make_shared<task_handler_wrapper<Handler>>(
		std::in_place, std::forward<Args>(args)...);
}

}  // namespace kcenon::messaging::task
