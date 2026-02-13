#include <kcenon/messaging/task/memory_result_backend.h>
#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <thread>
#include <vector>

namespace tsk = kcenon::messaging::task;
using tsk::memory_result_backend;
using tsk::result_backend_interface;
using tsk::task_state;
using tsk::progress_data;
using tsk::error_data;

// ============================================================================
// memory_result_backend - Basic Operations
// ============================================================================

TEST(MemoryResultBackendTest, DefaultConstruction) {
	memory_result_backend backend;
	EXPECT_EQ(backend.size(), 0);
}

TEST(MemoryResultBackendTest, StoreAndGetState) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Store state
	auto store_result = backend.store_state(task_id, task_state::running);
	EXPECT_TRUE(store_result.is_ok());

	// Get state
	auto get_result = backend.get_state(task_id);
	ASSERT_TRUE(get_result.is_ok());
	EXPECT_EQ(get_result.value(), task_state::running);
}

TEST(MemoryResultBackendTest, StoreAndGetResult) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Create and store an empty result container
	container_module::value_container result;

	// Store result
	auto store_res = backend.store_result(task_id, result);
	EXPECT_TRUE(store_res.is_ok());

	// Get result - verify it can be retrieved
	auto get_res = backend.get_result(task_id);
	ASSERT_TRUE(get_res.is_ok());
}

TEST(MemoryResultBackendTest, StoreAndGetError) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Store error
	auto store_res = backend.store_error(task_id, "Task failed", "at line 42");
	EXPECT_TRUE(store_res.is_ok());

	// Get error
	auto get_res = backend.get_error(task_id);
	ASSERT_TRUE(get_res.is_ok());
	EXPECT_EQ(get_res.value().message, "Task failed");
	EXPECT_EQ(get_res.value().traceback, "at line 42");
}

TEST(MemoryResultBackendTest, StoreAndGetProgress) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Store progress
	auto store_res = backend.store_progress(task_id, 0.5, "Halfway done");
	EXPECT_TRUE(store_res.is_ok());

	// Get progress
	auto get_res = backend.get_progress(task_id);
	ASSERT_TRUE(get_res.is_ok());
	EXPECT_DOUBLE_EQ(get_res.value().progress, 0.5);
	EXPECT_EQ(get_res.value().message, "Halfway done");
}

TEST(MemoryResultBackendTest, ProgressClamping) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Progress clamped to [0, 1]
	backend.store_progress(task_id, -0.5, "Negative");
	auto res1 = backend.get_progress(task_id);
	ASSERT_TRUE(res1.is_ok());
	EXPECT_DOUBLE_EQ(res1.value().progress, 0.0);

	backend.store_progress(task_id, 1.5, "Over 100%");
	auto res2 = backend.get_progress(task_id);
	ASSERT_TRUE(res2.is_ok());
	EXPECT_DOUBLE_EQ(res2.value().progress, 1.0);
}

// ============================================================================
// memory_result_backend - Error Cases
// ============================================================================

TEST(MemoryResultBackendTest, GetStateNonExistent) {
	memory_result_backend backend;

	auto result = backend.get_state("nonexistent");
	EXPECT_TRUE(result.is_err());
}

TEST(MemoryResultBackendTest, GetResultNonExistent) {
	memory_result_backend backend;

	auto result = backend.get_result("nonexistent");
	EXPECT_TRUE(result.is_err());
}

TEST(MemoryResultBackendTest, GetResultNotAvailable) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Store state but not result
	backend.store_state(task_id, task_state::running);

	auto result = backend.get_result(task_id);
	EXPECT_TRUE(result.is_err());
}

TEST(MemoryResultBackendTest, GetErrorNotAvailable) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Store state but not error
	backend.store_state(task_id, task_state::running);

	auto result = backend.get_error(task_id);
	EXPECT_TRUE(result.is_err());
}

// ============================================================================
// memory_result_backend - Exists and Remove
// ============================================================================

TEST(MemoryResultBackendTest, ExistsCheck) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	EXPECT_FALSE(backend.exists(task_id));

	backend.store_state(task_id, task_state::pending);
	EXPECT_TRUE(backend.exists(task_id));
}

TEST(MemoryResultBackendTest, Remove) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	backend.store_state(task_id, task_state::running);
	EXPECT_TRUE(backend.exists(task_id));

	auto remove_res = backend.remove(task_id);
	EXPECT_TRUE(remove_res.is_ok());
	EXPECT_FALSE(backend.exists(task_id));
}

TEST(MemoryResultBackendTest, Clear) {
	memory_result_backend backend;

	backend.store_state("task-1", task_state::running);
	backend.store_state("task-2", task_state::pending);
	backend.store_state("task-3", task_state::succeeded);
	EXPECT_EQ(backend.size(), 3);

	backend.clear();
	EXPECT_EQ(backend.size(), 0);
}

// ============================================================================
// memory_result_backend - Wait For Result
// ============================================================================

TEST(MemoryResultBackendTest, WaitForResultSuccess) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Start async task completion
	auto future = std::async(std::launch::async, [&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		container_module::value_container result;
		backend.store_result(task_id, result);
		backend.store_state(task_id, task_state::succeeded);
	});

	// Wait for result - just verify we get a successful result
	auto wait_result = backend.wait_for_result(task_id, std::chrono::seconds(1));
	ASSERT_TRUE(wait_result.is_ok());

	future.get();
}

TEST(MemoryResultBackendTest, WaitForResultFailure) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Start async task failure
	auto future = std::async(std::launch::async, [&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		backend.store_error(task_id, "Something went wrong", "traceback info");
		backend.store_state(task_id, task_state::failed);
	});

	// Wait for result
	auto wait_result = backend.wait_for_result(task_id, std::chrono::seconds(1));
	EXPECT_TRUE(wait_result.is_err());
	EXPECT_NE(wait_result.error().message.find("Task execution failed"), std::string::npos);

	future.get();
}

TEST(MemoryResultBackendTest, WaitForResultTimeout) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Create task but don't complete it
	backend.store_state(task_id, task_state::running);

	// Wait with short timeout
	auto wait_result = backend.wait_for_result(task_id, std::chrono::milliseconds(50));
	EXPECT_TRUE(wait_result.is_err());
	EXPECT_NE(wait_result.error().message.find("Task timeout"), std::string::npos);
}

TEST(MemoryResultBackendTest, WaitForResultCancelled) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Task gets cancelled
	auto future = std::async(std::launch::async, [&]() {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		backend.store_state(task_id, task_state::cancelled);
	});

	auto wait_result = backend.wait_for_result(task_id, std::chrono::seconds(1));
	EXPECT_TRUE(wait_result.is_err());
	EXPECT_NE(wait_result.error().message.find("cancelled"), std::string::npos);

	future.get();
}

// ============================================================================
// memory_result_backend - Cleanup
// ============================================================================

TEST(MemoryResultBackendTest, CleanupExpired) {
	memory_result_backend backend;

	// Create some tasks
	backend.store_state("task-1", task_state::succeeded);
	backend.store_state("task-2", task_state::failed);
	backend.store_state("task-3", task_state::running);  // Not terminal
	backend.store_state("task-4", task_state::pending);  // Not terminal

	EXPECT_EQ(backend.size(), 4);

	// Wait a bit
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Cleanup with very short max age
	auto cleanup_res = backend.cleanup_expired(std::chrono::milliseconds(10));
	EXPECT_TRUE(cleanup_res.is_ok());

	// Terminal tasks should be cleaned up, non-terminal should remain
	EXPECT_EQ(backend.size(), 2);
	EXPECT_FALSE(backend.exists("task-1"));  // succeeded - cleaned
	EXPECT_FALSE(backend.exists("task-2"));  // failed - cleaned
	EXPECT_TRUE(backend.exists("task-3"));   // running - not cleaned
	EXPECT_TRUE(backend.exists("task-4"));   // pending - not cleaned
}

// ============================================================================
// memory_result_backend - Thread Safety
// ============================================================================

TEST(MemoryResultBackendTest, ConcurrentWrites) {
	memory_result_backend backend;
	const int num_threads = 10;
	const int ops_per_thread = 100;

	std::vector<std::future<void>> futures;
	for (int t = 0; t < num_threads; ++t) {
		futures.push_back(std::async(std::launch::async, [&, t]() {
			for (int i = 0; i < ops_per_thread; ++i) {
				std::string task_id = "task-" + std::to_string(t) + "-" + std::to_string(i);
				backend.store_state(task_id, task_state::running);
				backend.store_progress(task_id, 0.5, "progress");
				backend.store_state(task_id, task_state::succeeded);
			}
		}));
	}

	for (auto& f : futures) {
		f.get();
	}

	EXPECT_EQ(backend.size(), num_threads * ops_per_thread);
}

TEST(MemoryResultBackendTest, ConcurrentReads) {
	memory_result_backend backend;
	const std::string task_id = "task-1";

	// Setup data
	backend.store_state(task_id, task_state::running);
	backend.store_progress(task_id, 0.5, "halfway");

	// Concurrent reads
	std::vector<std::future<void>> futures;
	for (int i = 0; i < 10; ++i) {
		futures.push_back(std::async(std::launch::async, [&]() {
			for (int j = 0; j < 100; ++j) {
				auto state = backend.get_state(task_id);
				EXPECT_TRUE(state.is_ok());
				auto progress = backend.get_progress(task_id);
				EXPECT_TRUE(progress.is_ok());
			}
		}));
	}

	for (auto& f : futures) {
		f.get();
	}
}

// ============================================================================
// memory_result_backend - Interface Compliance
// ============================================================================

TEST(MemoryResultBackendTest, InterfaceCompliance) {
	// Verify memory_result_backend implements result_backend_interface
	std::unique_ptr<result_backend_interface> backend =
		std::make_unique<memory_result_backend>();

	const std::string task_id = "task-1";

	// Test through interface
	EXPECT_TRUE(backend->store_state(task_id, task_state::running).is_ok());
	EXPECT_TRUE(backend->exists(task_id));

	auto state = backend->get_state(task_id);
	ASSERT_TRUE(state.is_ok());
	EXPECT_EQ(state.value(), task_state::running);
}
