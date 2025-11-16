#include <kcenon/messaging/backends/standalone_backend.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

using namespace kcenon::messaging;
using namespace kcenon::common;

// Test standalone_backend construction
TEST(StandaloneBackendTest, Construction) {
	standalone_backend backend;
	EXPECT_FALSE(backend.is_ready());
}

TEST(StandaloneBackendTest, ConstructionWithThreads) {
	standalone_backend backend(4);
	EXPECT_FALSE(backend.is_ready());
}

// Test initialization
TEST(StandaloneBackendTest, Initialize) {
	standalone_backend backend(2);
	auto result = backend.initialize();

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(backend.is_ready());

	backend.shutdown();
}

TEST(StandaloneBackendTest, DoubleInitialize) {
	standalone_backend backend(2);

	auto result1 = backend.initialize();
	ASSERT_TRUE(result1.is_ok());

	auto result2 = backend.initialize();
	EXPECT_FALSE(result2.is_ok());

	backend.shutdown();
}

// Test shutdown
TEST(StandaloneBackendTest, Shutdown) {
	standalone_backend backend(2);
	backend.initialize();

	auto result = backend.shutdown();
	EXPECT_TRUE(result.is_ok());
	EXPECT_FALSE(backend.is_ready());
}

TEST(StandaloneBackendTest, ShutdownWithoutInitialize) {
	standalone_backend backend(2);
	auto result = backend.shutdown();
	EXPECT_FALSE(result.is_ok());
}

// Test executor
TEST(StandaloneBackendTest, GetExecutor) {
	standalone_backend backend(2);
	backend.initialize();

	auto executor = backend.get_executor();
	ASSERT_NE(executor, nullptr);
	EXPECT_TRUE(executor->is_running());

	backend.shutdown();
}

TEST(StandaloneBackendTest, ExecutorBeforeInitialize) {
	standalone_backend backend(2);
	auto executor = backend.get_executor();
	EXPECT_EQ(executor, nullptr);
}

// Test logger and monitoring (should be nullptr for standalone)
TEST(StandaloneBackendTest, NoLogger) {
	standalone_backend backend(2);
	backend.initialize();

	auto logger = backend.get_logger();
	EXPECT_EQ(logger, nullptr);

	backend.shutdown();
}

TEST(StandaloneBackendTest, NoMonitoring) {
	standalone_backend backend(2);
	backend.initialize();

	auto monitoring = backend.get_monitoring();
	EXPECT_EQ(monitoring, nullptr);

	backend.shutdown();
}

// Test executor functionality
TEST(StandaloneBackendTest, ExecuteJob) {
	standalone_backend backend(2);
	backend.initialize();

	auto executor = backend.get_executor();
	ASSERT_NE(executor, nullptr);

	// Simple job that sets a flag
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

	// Wait for job to complete
	auto future = std::move(result.unwrap());
	future.wait();

	EXPECT_TRUE(job_executed.load());

	backend.shutdown();
}

// Test destructor cleanup
TEST(StandaloneBackendTest, DestructorCleanup) {
	{
		standalone_backend backend(2);
		backend.initialize();
		EXPECT_TRUE(backend.is_ready());
		// Destructor should clean up
	}
	// No crash = success
	SUCCEED();
}

// Test with zero threads (should default to 1)
TEST(StandaloneBackendTest, ZeroThreads) {
	standalone_backend backend(0);
	auto result = backend.initialize();

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(backend.is_ready());

	auto executor = backend.get_executor();
	ASSERT_NE(executor, nullptr);
	EXPECT_GT(executor->worker_count(), 0);

	backend.shutdown();
}

// Test multiple jobs
TEST(StandaloneBackendTest, MultipleJobs) {
	standalone_backend backend(4);
	backend.initialize();

	auto executor = backend.get_executor();
	ASSERT_NE(executor, nullptr);

	std::atomic<int> counter{0};

	class counter_job : public kcenon::common::interfaces::IJob {
	public:
		counter_job(std::atomic<int>& counter) : counter_(counter) {}

		VoidResult execute() override {
			counter_.fetch_add(1);
			return ok();
		}

		std::string get_name() const override {
			return "counter_job";
		}

	private:
		std::atomic<int>& counter_;
	};

	constexpr int num_jobs = 10;
	std::vector<std::future<void>> futures;

	for (int i = 0; i < num_jobs; ++i) {
		auto job = std::make_unique<counter_job>(counter);
		auto result = executor->execute(std::move(job));
		ASSERT_TRUE(result.is_ok());
		futures.push_back(std::move(result.unwrap()));
	}

	// Wait for all jobs
	for (auto& future : futures) {
		future.wait();
	}

	EXPECT_EQ(counter.load(), num_jobs);

	backend.shutdown();
}
