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
		result.set_value("status", std::string("success"));
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
		result.set_value("sum", 42);
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
		result.set_value("processed", true);
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
