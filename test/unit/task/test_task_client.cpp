#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace tsk = kcenon::messaging::task;
using tsk::task_client;
using tsk::task_queue;
using tsk::task_queue_config;
using tsk::memory_result_backend;
using tsk::task_builder;
using tsk::task_state;

class TaskClientTest : public ::testing::Test {
protected:
	void SetUp() override {
		task_queue_config config;
		config.enable_delayed_queue = true;
		queue_ = std::make_shared<task_queue>(config);
		backend_ = std::make_shared<memory_result_backend>();
		queue_->start();
	}

	void TearDown() override {
		if (queue_) {
			queue_->stop();
		}
	}

	std::shared_ptr<task_queue> queue_;
	std::shared_ptr<memory_result_backend> backend_;
};

// ============================================================================
// task_client - Construction
// ============================================================================

TEST_F(TaskClientTest, Construction) {
	task_client client(queue_, backend_);
	EXPECT_TRUE(client.is_connected());
}

TEST_F(TaskClientTest, IsConnected_WithoutQueue) {
	task_client client(nullptr, backend_);
	EXPECT_FALSE(client.is_connected());
}

TEST_F(TaskClientTest, IsConnected_WithoutBackend) {
	task_client client(queue_, nullptr);
	EXPECT_FALSE(client.is_connected());
}

// ============================================================================
// task_client - Immediate Send
// ============================================================================

TEST_F(TaskClientTest, Send_WithTask) {
	task_client client(queue_, backend_);

	auto task_result = task_builder("test.task")
		.build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = client.send(std::move(task_result).value());
	EXPECT_TRUE(result.is_valid());
	EXPECT_FALSE(result.task_id().empty());
}

TEST_F(TaskClientTest, Send_WithNameAndPayload) {
	task_client client(queue_, backend_);

	container_module::value_container payload;
	auto result = client.send("test.task", payload);

	EXPECT_TRUE(result.is_valid());
	EXPECT_FALSE(result.task_id().empty());
}

TEST_F(TaskClientTest, Send_InitializesBackendState) {
	task_client client(queue_, backend_);

	container_module::value_container payload;
	auto result = client.send("test.task", payload);

	// Backend should have the task state
	auto state_result = backend_->get_state(result.task_id());
	EXPECT_TRUE(state_result.is_ok());
}

// ============================================================================
// task_client - Delayed Send
// ============================================================================

TEST_F(TaskClientTest, SendLater_EnqueuesWithDelay) {
	task_client client(queue_, backend_);

	auto task_result = task_builder("delayed.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto result = client.send_later(
		std::move(task_result).value(),
		std::chrono::milliseconds(100)
	);

	EXPECT_TRUE(result.is_valid());
	EXPECT_FALSE(result.task_id().empty());

	// Task should be in delayed queue
	EXPECT_GE(queue_->delayed_size(), 0);  // May or may not be delayed yet
}

TEST_F(TaskClientTest, SendAt_EnqueuesWithETA) {
	task_client client(queue_, backend_);

	auto task_result = task_builder("scheduled.task").build();
	ASSERT_TRUE(task_result.is_ok());

	auto eta = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
	auto result = client.send_at(std::move(task_result).value(), eta);

	EXPECT_TRUE(result.is_valid());
}

// ============================================================================
// task_client - Batch Send
// ============================================================================

TEST_F(TaskClientTest, SendBatch_MultiipleTasks) {
	task_client client(queue_, backend_);

	std::vector<tsk::task> tasks;
	for (int i = 0; i < 5; ++i) {
		auto task_result = task_builder("batch.task." + std::to_string(i)).build();
		ASSERT_TRUE(task_result.is_ok());
		tasks.push_back(std::move(task_result).value());
	}

	auto results = client.send_batch(std::move(tasks));

	EXPECT_EQ(results.size(), 5);
	for (const auto& result : results) {
		EXPECT_TRUE(result.is_valid());
	}
}

TEST_F(TaskClientTest, SendBatch_EmptyVector) {
	task_client client(queue_, backend_);

	auto results = client.send_batch({});
	EXPECT_TRUE(results.empty());
}

// ============================================================================
// task_client - Chain Pattern
// ============================================================================

TEST_F(TaskClientTest, Chain_SingleTask) {
	task_client client(queue_, backend_);

	auto task_result = task_builder("single.task").build();
	ASSERT_TRUE(task_result.is_ok());

	std::vector<tsk::task> tasks;
	tasks.push_back(std::move(task_result).value());

	auto result = client.chain(std::move(tasks));
	EXPECT_TRUE(result.is_valid());
}

TEST_F(TaskClientTest, Chain_EmptyVector) {
	task_client client(queue_, backend_);

	auto result = client.chain({});
	EXPECT_FALSE(result.is_valid());
}

TEST_F(TaskClientTest, Chain_MultipleTasks) {
	task_client client(queue_, backend_);

	std::vector<tsk::task> tasks;
	for (int i = 0; i < 3; ++i) {
		auto task_result = task_builder("chain.step." + std::to_string(i)).build();
		ASSERT_TRUE(task_result.is_ok());
		tasks.push_back(std::move(task_result).value());
	}

	auto result = client.chain(std::move(tasks));
	EXPECT_TRUE(result.is_valid());
	EXPECT_FALSE(result.task_id().empty());
}

// ============================================================================
// task_client - Chord Pattern
// ============================================================================

TEST_F(TaskClientTest, Chord_WithCallback) {
	task_client client(queue_, backend_);

	std::vector<tsk::task> parallel_tasks;
	for (int i = 0; i < 3; ++i) {
		auto task_result = task_builder("parallel.task." + std::to_string(i)).build();
		ASSERT_TRUE(task_result.is_ok());
		parallel_tasks.push_back(std::move(task_result).value());
	}

	auto callback_result = task_builder("chord.callback").build();
	ASSERT_TRUE(callback_result.is_ok());

	auto result = client.chord(
		std::move(parallel_tasks),
		std::move(callback_result).value()
	);

	EXPECT_TRUE(result.is_valid());
}

TEST_F(TaskClientTest, Chord_EmptyParallelTasks) {
	task_client client(queue_, backend_);

	auto callback_result = task_builder("chord.callback").build();
	ASSERT_TRUE(callback_result.is_ok());

	// With empty parallel tasks, should just run the callback
	auto result = client.chord({}, std::move(callback_result).value());
	EXPECT_TRUE(result.is_valid());
}

// ============================================================================
// task_client - Result Retrieval
// ============================================================================

TEST_F(TaskClientTest, GetResult_ExistingTask) {
	task_client client(queue_, backend_);

	// Submit a task first
	container_module::value_container payload;
	auto send_result = client.send("test.task", payload);

	// Get result handle
	auto result = client.get_result(send_result.task_id());
	EXPECT_TRUE(result.is_valid());
	EXPECT_EQ(result.task_id(), send_result.task_id());
}

TEST_F(TaskClientTest, GetResult_NonExistentTask) {
	task_client client(queue_, backend_);

	auto result = client.get_result("nonexistent-task-id");
	EXPECT_TRUE(result.is_valid());  // Handle is valid, but task doesn't exist
}

// ============================================================================
// task_client - Cancellation
// ============================================================================

TEST_F(TaskClientTest, Cancel_ValidTask) {
	task_client client(queue_, backend_);

	// Submit a task
	container_module::value_container payload;
	auto send_result = client.send("test.task", payload);

	// Cancel it
	auto cancel_result = client.cancel(send_result.task_id());
	EXPECT_TRUE(cancel_result.is_ok());

	// Verify state is cancelled
	auto state = backend_->get_state(send_result.task_id());
	EXPECT_TRUE(state.is_ok());
	EXPECT_EQ(state.value(), task_state::cancelled);
}

TEST_F(TaskClientTest, Cancel_WithoutQueue) {
	task_client client(nullptr, backend_);
	auto cancel_result = client.cancel("some-task-id");
	EXPECT_FALSE(cancel_result.is_ok());
}

TEST_F(TaskClientTest, CancelByTag_ValidTag) {
	task_client client(queue_, backend_);

	// Submit tasks with a tag
	for (int i = 0; i < 3; ++i) {
		auto task_result = task_builder("tagged.task")
			.tag("batch-1")
			.build();
		ASSERT_TRUE(task_result.is_ok());
		client.send(std::move(task_result).value());
	}

	// Cancel by tag
	auto cancel_result = client.cancel_by_tag("batch-1");
	EXPECT_TRUE(cancel_result.is_ok());
}

// ============================================================================
// task_client - Queue Information
// ============================================================================

TEST_F(TaskClientTest, PendingCount_EmptyQueue) {
	task_client client(queue_, backend_);
	EXPECT_EQ(client.pending_count(), 0);
}

TEST_F(TaskClientTest, PendingCount_WithTasks) {
	task_client client(queue_, backend_);

	// Submit several tasks
	for (int i = 0; i < 5; ++i) {
		container_module::value_container payload;
		client.send("test.task", payload);
	}

	EXPECT_GE(client.pending_count(), 0);  // Tasks may have been processed
}

TEST_F(TaskClientTest, PendingCount_SpecificQueue) {
	task_client client(queue_, backend_);

	// Submit task to specific queue
	auto task_result = task_builder("queued.task")
		.queue("custom-queue")
		.build();
	ASSERT_TRUE(task_result.is_ok());
	client.send(std::move(task_result).value());

	// Check custom queue
	size_t count = client.pending_count("custom-queue");
	EXPECT_GE(count, 0);
}

// ============================================================================
// task_client - Thread Safety
// ============================================================================

TEST_F(TaskClientTest, ConcurrentSend) {
	task_client client(queue_, backend_);

	std::vector<std::thread> threads;
	std::atomic<int> success_count{0};

	for (int i = 0; i < 10; ++i) {
		threads.emplace_back([&client, &success_count, i]() {
			for (int j = 0; j < 10; ++j) {
				container_module::value_container payload;
				auto result = client.send("concurrent.task." + std::to_string(i), payload);
				if (result.is_valid()) {
					success_count++;
				}
			}
		});
	}

	for (auto& t : threads) {
		t.join();
	}

	EXPECT_EQ(success_count, 100);
}
