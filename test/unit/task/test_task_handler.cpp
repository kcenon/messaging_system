#include <kcenon/messaging/task/task_handler.h>
#include <kcenon/messaging/task/task_context.h>
#include <gtest/gtest.h>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;
using tsk::task_handler_interface;
using tsk::simple_task_handler;
using tsk::lambda_task_handler;
using tsk::make_handler;
using tsk::task_context;
using tsk::task_builder;
using task_t = tsk::task;

// ============================================================================
// lambda_task_handler tests
// ============================================================================

TEST(LambdaTaskHandlerTest, BasicExecution) {
	simple_task_handler fn = [](const task_t& t, task_context& ctx) {
		(void)ctx;
		(void)t;
		container_module::value_container result;
		result.set("status", std::string("success"));
		return cmn::ok(result);
	};

	lambda_task_handler handler("test.handler", fn);

	EXPECT_EQ(handler.name(), "test.handler");

	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = handler.execute(task, ctx);

	ASSERT_FALSE(exec_result.is_err());
}

TEST(LambdaTaskHandlerTest, NullHandlerFails) {
	lambda_task_handler handler("null.handler", nullptr);

	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = handler.execute(task, ctx);

	EXPECT_TRUE(exec_result.is_err());
}

// ============================================================================
// make_handler helper tests
// ============================================================================

TEST(MakeHandlerTest, CreatesValidHandler) {
	auto handler = make_handler("math.add", [](const task_t& t, task_context& ctx) {
		(void)ctx;
		(void)t;
		container_module::value_container result;
		result.set("sum", 42);
		return cmn::ok(result);
	});

	EXPECT_NE(handler, nullptr);
	EXPECT_EQ(handler->name(), "math.add");
}

// ============================================================================
// Custom task_handler_interface implementation tests
// ============================================================================

class test_task_handler : public task_handler_interface {
public:
	std::string name() const override { return "test.custom"; }

	cmn::Result<container_module::value_container> execute(
		const task_t& t,
		task_context& ctx) override {
		execution_count_++;
		ctx.update_progress(0.5, "Processing...");
		(void)t;

		container_module::value_container result;
		result.set("processed", true);
		return cmn::ok(result);
	}

	void on_retry(const task_t& /*t*/, size_t attempt) override {
		retry_count_++;
		last_retry_attempt_ = attempt;
	}

	void on_failure(const task_t& /*t*/, const std::string& error) override {
		failure_count_++;
		last_error_ = error;
	}

	void on_success(const task_t& /*t*/,
					const container_module::value_container& /*result*/) override {
		success_count_++;
	}

	int execution_count() const { return execution_count_; }
	int retry_count() const { return retry_count_; }
	int failure_count() const { return failure_count_; }
	int success_count() const { return success_count_; }
	size_t last_retry_attempt() const { return last_retry_attempt_; }
	const std::string& last_error() const { return last_error_; }

private:
	int execution_count_ = 0;
	int retry_count_ = 0;
	int failure_count_ = 0;
	int success_count_ = 0;
	size_t last_retry_attempt_ = 0;
	std::string last_error_;
};

TEST(CustomTaskHandlerTest, ExecutionWorks) {
	test_task_handler handler;

	auto task_result = task_builder("test.custom").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = handler.execute(task, ctx);

	ASSERT_FALSE(exec_result.is_err());
	EXPECT_EQ(handler.execution_count(), 1);
}

TEST(CustomTaskHandlerTest, HooksAreCalled) {
	test_task_handler handler;

	auto task_result = task_builder("test.custom").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	// Test on_retry hook
	handler.on_retry(task, 1);
	EXPECT_EQ(handler.retry_count(), 1);
	EXPECT_EQ(handler.last_retry_attempt(), 1);

	handler.on_retry(task, 2);
	EXPECT_EQ(handler.retry_count(), 2);
	EXPECT_EQ(handler.last_retry_attempt(), 2);

	// Test on_failure hook
	handler.on_failure(task, "Something went wrong");
	EXPECT_EQ(handler.failure_count(), 1);
	EXPECT_EQ(handler.last_error(), "Something went wrong");

	// Test on_success hook
	container_module::value_container result;
	handler.on_success(task, result);
	EXPECT_EQ(handler.success_count(), 1);
}

// ============================================================================
// task_handler_interface base class tests
// ============================================================================

class minimal_handler : public task_handler_interface {
public:
	std::string name() const override { return "minimal"; }

	cmn::Result<container_module::value_container> execute(
		const task_t& /*t*/,
		task_context& /*ctx*/) override {
		return cmn::ok(container_module::value_container());
	}
};

TEST(TaskHandlerInterfaceTest, DefaultHooksDoNothing) {
	minimal_handler handler;

	auto task_result = task_builder("test").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	// These should not throw - default implementations are empty
	handler.on_retry(task, 1);
	handler.on_failure(task, "error");
	container_module::value_container result;
	handler.on_success(task, result);

	// If we get here without throwing, the test passes
	SUCCEED();
}

// ============================================================================
// CRTP task_handler_base tests
// ============================================================================

using tsk::task_handler_base;
using tsk::task_handler_wrapper;
using tsk::make_crtp_task_handler;

class crtp_test_handler : public task_handler_base<crtp_test_handler> {
	friend class task_handler_base<crtp_test_handler>;

public:
	crtp_test_handler() = default;
	explicit crtp_test_handler(std::string name) : handler_name_(std::move(name)) {}

	[[nodiscard]] std::string name_impl() const { return handler_name_; }

	cmn::Result<container_module::value_container> execute_impl(
		const task_t& t,
		task_context& ctx) {
		execution_count_++;
		ctx.update_progress(0.5, "CRTP Processing...");
		(void)t;

		container_module::value_container result;
		result.set("crtp", true);
		return cmn::ok(result);
	}

	void on_retry_impl(const task_t& /*t*/, size_t attempt) {
		retry_count_++;
		last_retry_attempt_ = attempt;
	}

	void on_failure_impl(const task_t& /*t*/, const std::string& error) {
		failure_count_++;
		last_error_ = error;
	}

	void on_success_impl(const task_t& /*t*/,
						 const container_module::value_container& /*result*/) {
		success_count_++;
	}

	[[nodiscard]] int execution_count() const { return execution_count_; }
	[[nodiscard]] int retry_count() const { return retry_count_; }
	[[nodiscard]] int failure_count() const { return failure_count_; }
	[[nodiscard]] int success_count() const { return success_count_; }
	[[nodiscard]] size_t last_retry_attempt() const { return last_retry_attempt_; }
	[[nodiscard]] const std::string& last_error() const { return last_error_; }

private:
	std::string handler_name_ = "crtp.test";
	int execution_count_ = 0;
	int retry_count_ = 0;
	int failure_count_ = 0;
	int success_count_ = 0;
	size_t last_retry_attempt_ = 0;
	std::string last_error_;
};

TEST(CRTPTaskHandlerTest, BasicExecution) {
	crtp_test_handler handler;

	EXPECT_EQ(handler.name(), "crtp.test");

	auto task_result = task_builder("crtp.test").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = handler.execute(task, ctx);

	ASSERT_FALSE(exec_result.is_err());
	EXPECT_EQ(handler.execution_count(), 1);
}

TEST(CRTPTaskHandlerTest, HooksAreCalled) {
	crtp_test_handler handler;

	auto task_result = task_builder("crtp.test").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	// Test on_retry hook
	handler.on_retry(task, 1);
	EXPECT_EQ(handler.retry_count(), 1);
	EXPECT_EQ(handler.last_retry_attempt(), 1);

	handler.on_retry(task, 2);
	EXPECT_EQ(handler.retry_count(), 2);
	EXPECT_EQ(handler.last_retry_attempt(), 2);

	// Test on_failure hook
	handler.on_failure(task, "CRTP error");
	EXPECT_EQ(handler.failure_count(), 1);
	EXPECT_EQ(handler.last_error(), "CRTP error");

	// Test on_success hook
	container_module::value_container result;
	handler.on_success(task, result);
	EXPECT_EQ(handler.success_count(), 1);
}

TEST(CRTPTaskHandlerTest, ConstructorWithName) {
	crtp_test_handler handler("custom.crtp.name");
	EXPECT_EQ(handler.name(), "custom.crtp.name");
}

// ============================================================================
// task_handler_wrapper tests (type erasure)
// ============================================================================

TEST(TaskHandlerWrapperTest, WrapsWithDefaultConstructor) {
	task_handler_wrapper<crtp_test_handler> wrapper;

	EXPECT_EQ(wrapper.name(), "crtp.test");

	auto task_result = task_builder("crtp.test").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = wrapper.execute(task, ctx);

	ASSERT_FALSE(exec_result.is_err());
	EXPECT_EQ(wrapper.get().execution_count(), 1);
}

TEST(TaskHandlerWrapperTest, WrapsWithHandlerInstance) {
	crtp_test_handler handler("wrapped.handler");
	task_handler_wrapper<crtp_test_handler> wrapper(std::move(handler));

	EXPECT_EQ(wrapper.name(), "wrapped.handler");
}

TEST(TaskHandlerWrapperTest, WrapsWithInPlace) {
	task_handler_wrapper<crtp_test_handler> wrapper(std::in_place, "inplace.handler");

	EXPECT_EQ(wrapper.name(), "inplace.handler");
}

TEST(TaskHandlerWrapperTest, HooksWorkThroughWrapper) {
	task_handler_wrapper<crtp_test_handler> wrapper;

	auto task_result = task_builder("crtp.test").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	// Test hooks through wrapper
	wrapper.on_retry(task, 3);
	EXPECT_EQ(wrapper.get().retry_count(), 1);
	EXPECT_EQ(wrapper.get().last_retry_attempt(), 3);

	wrapper.on_failure(task, "wrapped error");
	EXPECT_EQ(wrapper.get().failure_count(), 1);

	container_module::value_container result;
	wrapper.on_success(task, result);
	EXPECT_EQ(wrapper.get().success_count(), 1);
}

TEST(TaskHandlerWrapperTest, UsableAsInterface) {
	std::shared_ptr<task_handler_interface> handler =
		std::make_shared<task_handler_wrapper<crtp_test_handler>>("interface.handler");

	EXPECT_EQ(handler->name(), "interface.handler");

	auto task_result = task_builder("interface.handler").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = handler->execute(task, ctx);

	ASSERT_FALSE(exec_result.is_err());
}

// ============================================================================
// make_crtp_task_handler factory tests
// ============================================================================

TEST(MakeCRTPTaskHandlerTest, CreatesWrappedHandler) {
	auto handler = make_crtp_task_handler<crtp_test_handler>("factory.handler");

	EXPECT_NE(handler, nullptr);
	EXPECT_EQ(handler->name(), "factory.handler");

	auto task_result = task_builder("factory.handler").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);
	auto exec_result = handler->execute(task, ctx);

	ASSERT_FALSE(exec_result.is_err());
}

TEST(MakeCRTPTaskHandlerTest, DefaultConstruction) {
	auto handler = make_crtp_task_handler<crtp_test_handler>();

	EXPECT_NE(handler, nullptr);
	EXPECT_EQ(handler->name(), "crtp.test");
}
