#include <kcenon/messaging/task/async_result.h>
#include <kcenon/messaging/task/memory_result_backend.h>
#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <thread>

namespace tsk = kcenon::messaging::task;
using tsk::async_result;
using tsk::memory_result_backend;
using tsk::task_state;

// ============================================================================
// async_result - Construction
// ============================================================================

TEST(AsyncResultTest, DefaultConstruction) {
	async_result result;
	EXPECT_FALSE(result.is_valid());
	EXPECT_TRUE(result.task_id().empty());
}

TEST(AsyncResultTest, ConstructWithTaskIdAndBackend) {
	auto backend = std::make_shared<memory_result_backend>();
	async_result result("task-123", backend);

	EXPECT_TRUE(result.is_valid());
	EXPECT_EQ(result.task_id(), "task-123");
}

TEST(AsyncResultTest, CopyConstruction) {
	auto backend = std::make_shared<memory_result_backend>();
	async_result original("task-123", backend);
	async_result copy(original);

	EXPECT_EQ(copy.task_id(), original.task_id());
	EXPECT_TRUE(copy.is_valid());
}

TEST(AsyncResultTest, MoveConstruction) {
	auto backend = std::make_shared<memory_result_backend>();
	async_result original("task-123", backend);
	async_result moved(std::move(original));

	EXPECT_EQ(moved.task_id(), "task-123");
	EXPECT_TRUE(moved.is_valid());
}

// ============================================================================
// async_result - State Queries
// ============================================================================

TEST(AsyncResultTest, StateQuery_Pending) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::pending);

	async_result result("task-1", backend);
	EXPECT_EQ(result.state(), task_state::pending);
	EXPECT_FALSE(result.is_ready());
	EXPECT_FALSE(result.is_successful());
	EXPECT_FALSE(result.is_failed());
}

TEST(AsyncResultTest, StateQuery_Running) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::running);

	async_result result("task-1", backend);
	EXPECT_EQ(result.state(), task_state::running);
	EXPECT_FALSE(result.is_ready());
}

TEST(AsyncResultTest, StateQuery_Succeeded) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::succeeded);

	async_result result("task-1", backend);
	EXPECT_EQ(result.state(), task_state::succeeded);
	EXPECT_TRUE(result.is_ready());
	EXPECT_TRUE(result.is_successful());
	EXPECT_FALSE(result.is_failed());
}

TEST(AsyncResultTest, StateQuery_Failed) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::failed);

	async_result result("task-1", backend);
	EXPECT_EQ(result.state(), task_state::failed);
	EXPECT_TRUE(result.is_ready());
	EXPECT_FALSE(result.is_successful());
	EXPECT_TRUE(result.is_failed());
}

TEST(AsyncResultTest, StateQuery_Cancelled) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::cancelled);

	async_result result("task-1", backend);
	EXPECT_TRUE(result.is_cancelled());
	EXPECT_TRUE(result.is_ready());
}

// ============================================================================
// async_result - Progress Queries
// ============================================================================

TEST(AsyncResultTest, ProgressQuery) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_progress("task-1", 0.5, "Halfway done");

	async_result result("task-1", backend);
	EXPECT_DOUBLE_EQ(result.progress(), 0.5);
	EXPECT_EQ(result.progress_message(), "Halfway done");
}

TEST(AsyncResultTest, ProgressQuery_NoProgress) {
	auto backend = std::make_shared<memory_result_backend>();
	async_result result("task-nonexistent", backend);

	EXPECT_DOUBLE_EQ(result.progress(), 0.0);
	EXPECT_EQ(result.progress_message(), "");
}

// ============================================================================
// async_result - Result Retrieval (Blocking)
// ============================================================================

TEST(AsyncResultTest, GetResult_Success) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::succeeded);

	container_module::value_container result_data;
	backend->store_result("task-1", result_data);

	async_result result("task-1", backend);
	auto get_result = result.get(std::chrono::milliseconds(100));
	EXPECT_TRUE(get_result.is_ok());
}

TEST(AsyncResultTest, GetResult_Failed) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::failed);
	backend->store_error("task-1", "Task execution failed", "Stack trace here");

	async_result result("task-1", backend);
	auto get_result = result.get(std::chrono::milliseconds(100));
	EXPECT_FALSE(get_result.is_ok());
}

TEST(AsyncResultTest, GetResult_InvalidHandle) {
	async_result result;
	auto get_result = result.get(std::chrono::milliseconds(100));
	EXPECT_FALSE(get_result.is_ok());
	EXPECT_EQ(get_result.error().message, "Invalid async_result handle");
}

// ============================================================================
// async_result - Wait
// ============================================================================

TEST(AsyncResultTest, Wait_AlreadyComplete) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::succeeded);

	async_result result("task-1", backend);
	bool completed = result.wait(std::chrono::milliseconds(100));
	EXPECT_TRUE(completed);
}

TEST(AsyncResultTest, Wait_Timeout) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::running);

	async_result result("task-1", backend);
	bool completed = result.wait(std::chrono::milliseconds(100));
	EXPECT_FALSE(completed);
}

TEST(AsyncResultTest, Wait_CompletesInTime) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::running);

	async_result result("task-1", backend);

	// Start a thread that will mark the task as succeeded
	std::thread completer([&backend]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		backend->store_state("task-1", task_state::succeeded);
	});

	bool completed = result.wait(std::chrono::milliseconds(500));
	completer.join();

	EXPECT_TRUE(completed);
}

// ============================================================================
// async_result - Callback-based Retrieval
// ============================================================================

TEST(AsyncResultTest, Then_AlreadySucceeded) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::succeeded);
	container_module::value_container result_data;
	backend->store_result("task-1", result_data);

	async_result result("task-1", backend);

	std::atomic<bool> success_called{false};
	std::atomic<bool> failure_called{false};

	result.then(
		[&success_called](const container_module::value_container&) {
			success_called = true;
		},
		[&failure_called](const std::string&) {
			failure_called = true;
		}
	);

	// Give time for callback to be invoked
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	EXPECT_TRUE(success_called);
	EXPECT_FALSE(failure_called);
}

TEST(AsyncResultTest, Then_AlreadyFailed) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::failed);
	backend->store_error("task-1", "Task failed", "");

	async_result result("task-1", backend);

	std::atomic<bool> success_called{false};
	std::atomic<bool> failure_called{false};

	result.then(
		[&success_called](const container_module::value_container&) {
			success_called = true;
		},
		[&failure_called](const std::string&) {
			failure_called = true;
		}
	);

	// Give time for callback to be invoked
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	EXPECT_FALSE(success_called);
	EXPECT_TRUE(failure_called);
}

// ============================================================================
// async_result - Task Control
// ============================================================================

TEST(AsyncResultTest, Revoke) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::running);

	async_result result("task-1", backend);
	auto revoke_result = result.revoke();

	EXPECT_TRUE(revoke_result.is_ok());
	EXPECT_TRUE(result.is_cancelled());
}

TEST(AsyncResultTest, Revoke_InvalidHandle) {
	async_result result;
	auto revoke_result = result.revoke();

	EXPECT_FALSE(revoke_result.is_ok());
}

// ============================================================================
// async_result - Child Task Management
// ============================================================================

TEST(AsyncResultTest, AddAndGetChildren) {
	auto backend = std::make_shared<memory_result_backend>();
	async_result result("parent-task", backend);

	result.add_child("child-1");
	result.add_child("child-2");

	auto children = result.children();
	EXPECT_EQ(children.size(), 2);
	EXPECT_EQ(children[0].task_id(), "child-1");
	EXPECT_EQ(children[1].task_id(), "child-2");
}

// ============================================================================
// async_result - Error Information
// ============================================================================

TEST(AsyncResultTest, ErrorMessage) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_error("task-1", "Something went wrong", "Detailed traceback");

	async_result result("task-1", backend);

	EXPECT_EQ(result.error_message(), "Something went wrong");
	EXPECT_EQ(result.error_traceback(), "Detailed traceback");
}

TEST(AsyncResultTest, ErrorMessage_NoError) {
	auto backend = std::make_shared<memory_result_backend>();
	async_result result("task-nonexistent", backend);

	EXPECT_EQ(result.error_message(), "");
	EXPECT_EQ(result.error_traceback(), "");
}

// ============================================================================
// async_result - Thread Safety
// ============================================================================

TEST(AsyncResultTest, ConcurrentStateQueries) {
	auto backend = std::make_shared<memory_result_backend>();
	backend->store_state("task-1", task_state::running);
	backend->store_progress("task-1", 0.0, "Starting");

	async_result result("task-1", backend);

	std::vector<std::thread> threads;
	std::atomic<int> query_count{0};

	// Start multiple threads querying state
	for (int i = 0; i < 10; ++i) {
		threads.emplace_back([&result, &query_count]() {
			for (int j = 0; j < 100; ++j) {
				result.state();
				result.progress();
				result.is_ready();
				query_count++;
			}
		});
	}

	// Meanwhile, update state
	for (int i = 0; i <= 10; ++i) {
		backend->store_progress("task-1", i / 10.0, "Progress " + std::to_string(i * 10) + "%");
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	for (auto& t : threads) {
		t.join();
	}

	EXPECT_EQ(query_count, 1000);  // All queries completed
}
