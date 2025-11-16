#include <kcenon/messaging/backends/integration_backend.h>
#include <gtest/gtest.h>

#include <atomic>
#include <memory>

using namespace kcenon::messaging;
using namespace kcenon::common;

// Simplified mock executor for testing
class simple_mock_executor : public kcenon::common::interfaces::IExecutor {
public:
	simple_mock_executor() : running_(true) {}
	virtual ~simple_mock_executor() = default;

	Result<std::future<void>> execute(std::unique_ptr<kcenon::common::interfaces::IJob>&& job) override {
		if (!running_) {
			return make_error<std::future<void>>(
				-1,
				"Executor not running",
				"simple_mock_executor"
			);
		}

		auto promise = std::make_shared<std::promise<void>>();
		auto future = promise->get_future();

		// Execute synchronously for testing
		auto result = job->execute();
		if (result.is_ok()) {
			promise->set_value();
		} else {
			promise->set_exception(std::make_exception_ptr(
				std::runtime_error(std::string(result.error().message))
			));
		}

		execute_count_++;
		return ok(std::move(future));
	}

	Result<std::future<void>> execute_delayed(
		std::unique_ptr<kcenon::common::interfaces::IJob>&& job,
		std::chrono::milliseconds /* delay */) override {
		return execute(std::move(job));
	}

	size_t worker_count() const override { return 4; }
	size_t pending_tasks() const override { return 0; }
	bool is_running() const override { return running_; }

	void shutdown(bool /* wait_for_completion */ = true) override {
		running_ = false;
	}

	int get_execute_count() const { return execute_count_.load(); }

private:
	std::atomic<bool> running_{true};
	std::atomic<int> execute_count_{0};
};

// Test integration_backend construction
TEST(IntegrationBackendTest, ConstructionWithExecutor) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	EXPECT_FALSE(backend.is_ready());
	EXPECT_EQ(backend.get_executor(), executor);
}

TEST(IntegrationBackendTest, ConstructionWithNullLogger) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor, nullptr, nullptr);

	EXPECT_FALSE(backend.is_ready());
	EXPECT_EQ(backend.get_executor(), executor);
	EXPECT_EQ(backend.get_logger(), nullptr);
	EXPECT_EQ(backend.get_monitoring(), nullptr);
}

// Test initialization
TEST(IntegrationBackendTest, Initialize) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	auto result = backend.initialize();
	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(backend.is_ready());
}

TEST(IntegrationBackendTest, InitializeWithoutExecutor) {
	integration_backend backend(nullptr);

	auto result = backend.initialize();
	EXPECT_FALSE(result.is_ok());
	EXPECT_FALSE(backend.is_ready());
}

TEST(IntegrationBackendTest, DoubleInitialize) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	auto result1 = backend.initialize();
	ASSERT_TRUE(result1.is_ok());

	auto result2 = backend.initialize();
	EXPECT_FALSE(result2.is_ok());
}

// Test shutdown
TEST(IntegrationBackendTest, Shutdown) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	backend.initialize();
	auto result = backend.shutdown();

	EXPECT_TRUE(result.is_ok());
	EXPECT_FALSE(backend.is_ready());
}

TEST(IntegrationBackendTest, ShutdownWithoutInitialize) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	auto result = backend.shutdown();
	EXPECT_FALSE(result.is_ok());
}

// Test executor access
TEST(IntegrationBackendTest, GetExecutor) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	backend.initialize();
	EXPECT_EQ(backend.get_executor(), executor);
}

// Test logger access (nullptr case)
TEST(IntegrationBackendTest, GetLoggerWhenNull) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor, nullptr, nullptr);

	backend.initialize();
	EXPECT_EQ(backend.get_logger(), nullptr);
}

// Test monitoring access (nullptr case)
TEST(IntegrationBackendTest, GetMonitoringWhenNull) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor, nullptr, nullptr);

	backend.initialize();
	EXPECT_EQ(backend.get_monitoring(), nullptr);
}

// Test executor functionality
TEST(IntegrationBackendTest, ExecuteJob) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);
	backend.initialize();

	std::atomic<bool> job_executed{false};

	class test_job : public kcenon::common::interfaces::IJob {
	public:
		test_job(std::atomic<bool>& flag) : flag_(flag) {}

		VoidResult execute() override {
			flag_.store(true);
			return ok();
		}

		std::string get_name() const override {
			return "test_job";
		}

	private:
		std::atomic<bool>& flag_;
	};

	auto job = std::make_unique<test_job>(job_executed);
	auto result = executor->execute(std::move(job));

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(job_executed.load());
	EXPECT_EQ(executor->get_execute_count(), 1);
}

// Test that backend doesn't shutdown services it doesn't own
TEST(IntegrationBackendTest, DoesNotShutdownExternalServices) {
	auto executor = std::make_shared<simple_mock_executor>();

	{
		integration_backend backend(executor, nullptr, nullptr);
		backend.initialize();
		backend.shutdown();
	}

	// Executor should still be valid and running
	EXPECT_TRUE(executor->is_running());
}

// Test service lifecycle independence
TEST(IntegrationBackendTest, ServiceLifecycleIndependence) {
	auto executor = std::make_shared<simple_mock_executor>();
	integration_backend backend(executor);

	backend.initialize();

	// Shutdown external executor
	executor->shutdown();

	// Backend should still be "initialized" (doesn't own executor)
	// but executor is no longer running
	EXPECT_TRUE(backend.is_ready()); // Backend thinks it's ready
	EXPECT_FALSE(executor->is_running()); // But executor is not

	backend.shutdown();
}
