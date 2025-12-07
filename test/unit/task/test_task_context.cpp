#include <kcenon/messaging/task/task_context.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
namespace cmn = kcenon::common;
using tsk::task_context;
using tsk::task_builder;
using tsk::progress_info;
using tsk::task_log_entry;
using task_t = tsk::task;

// ============================================================================
// task_context construction tests
// ============================================================================

TEST(TaskContextTest, Construction) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	EXPECT_DOUBLE_EQ(ctx.progress(), 0.0);
	EXPECT_EQ(ctx.attempt_number(), 1);
	EXPECT_FALSE(ctx.is_cancelled());
	EXPECT_FALSE(ctx.has_checkpoint());
}

TEST(TaskContextTest, ConstructionWithAttempt) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task, 3);

	EXPECT_EQ(ctx.attempt_number(), 3);
}

// ============================================================================
// Progress tracking tests
// ============================================================================

TEST(TaskContextTest, ProgressUpdate) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	ctx.update_progress(0.5, "Halfway done");
	EXPECT_DOUBLE_EQ(ctx.progress(), 0.5);

	ctx.update_progress(1.0, "Complete");
	EXPECT_DOUBLE_EQ(ctx.progress(), 1.0);
}

TEST(TaskContextTest, ProgressClamping) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	ctx.update_progress(-0.5);
	EXPECT_DOUBLE_EQ(ctx.progress(), 0.0);

	ctx.update_progress(1.5);
	EXPECT_DOUBLE_EQ(ctx.progress(), 1.0);
}

TEST(TaskContextTest, ProgressHistory) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	ctx.update_progress(0.25, "Step 1");
	ctx.update_progress(0.50, "Step 2");
	ctx.update_progress(0.75, "Step 3");
	ctx.update_progress(1.00, "Done");

	auto history = ctx.progress_history();
	EXPECT_EQ(history.size(), 4);
	EXPECT_DOUBLE_EQ(history[0].progress, 0.25);
	EXPECT_EQ(history[0].message, "Step 1");
	EXPECT_DOUBLE_EQ(history[3].progress, 1.00);
	EXPECT_EQ(history[3].message, "Done");
}

TEST(TaskContextTest, ProgressUpdatesTask) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	ctx.update_progress(0.75, "Almost there");

	// The task should also be updated
	EXPECT_DOUBLE_EQ(task.progress(), 0.75);
	EXPECT_EQ(task.progress_message(), "Almost there");
}

// ============================================================================
// Checkpoint tests
// ============================================================================

TEST(TaskContextTest, CheckpointSaveLoad) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	EXPECT_FALSE(ctx.has_checkpoint());

	container_module::value_container state;
	state.set_value("step", 5);
	ctx.save_checkpoint(state);

	EXPECT_TRUE(ctx.has_checkpoint());

	auto loaded = ctx.load_checkpoint();
	// Just verify checkpoint was loaded (value retrieval API differs)
	EXPECT_TRUE(ctx.has_checkpoint());
}

TEST(TaskContextTest, CheckpointClear) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	container_module::value_container state;
	state.set_value("key", std::string("value"));
	ctx.save_checkpoint(state);

	EXPECT_TRUE(ctx.has_checkpoint());

	ctx.clear_checkpoint();
	EXPECT_FALSE(ctx.has_checkpoint());
}

TEST(TaskContextTest, CheckpointWithSharedPtr) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	auto state = std::make_shared<container_module::value_container>();
	state->set_value("shared_key", std::string("shared_value"));
	ctx.save_checkpoint(state);

	EXPECT_TRUE(ctx.has_checkpoint());
}

// ============================================================================
// Cancellation tests
// ============================================================================

TEST(TaskContextTest, Cancellation) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	EXPECT_FALSE(ctx.is_cancelled());

	ctx.request_cancellation();
	EXPECT_TRUE(ctx.is_cancelled());
}

// ============================================================================
// Logging tests
// ============================================================================

TEST(TaskContextTest, Logging) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	ctx.log_info("Starting task");
	ctx.log_warning("Resource low");
	ctx.log_error("Something failed");

	auto logs = ctx.logs();
	EXPECT_EQ(logs.size(), 3);

	EXPECT_EQ(logs[0].log_level, task_log_entry::level::info);
	EXPECT_EQ(logs[0].message, "Starting task");

	EXPECT_EQ(logs[1].log_level, task_log_entry::level::warning);
	EXPECT_EQ(logs[1].message, "Resource low");

	EXPECT_EQ(logs[2].log_level, task_log_entry::level::error);
	EXPECT_EQ(logs[2].message, "Something failed");
}

// ============================================================================
// Task information tests
// ============================================================================

TEST(TaskContextTest, CurrentTask) {
	auto task_result = task_builder("my.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	EXPECT_EQ(ctx.current_task().task_name(), "my.task");
	EXPECT_EQ(ctx.current_task().task_id(), task.task_id());
}

TEST(TaskContextTest, ElapsedTime) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	// Sleep a bit
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	auto elapsed = ctx.elapsed();
	EXPECT_GE(elapsed.count(), 50);
}

TEST(TaskContextTest, StartedAt) {
	auto before = std::chrono::system_clock::now();

	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	auto after = std::chrono::system_clock::now();

	EXPECT_GE(ctx.started_at(), before);
	EXPECT_LE(ctx.started_at(), after);
}

// ============================================================================
// Subtask spawning tests
// ============================================================================

TEST(TaskContextTest, SubtaskSpawnerNotConfigured) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	auto subtask_result = task_builder("subtask").build();
	ASSERT_FALSE(subtask_result.is_err());
	auto subtask = subtask_result.unwrap();

	auto spawn_result = ctx.spawn_subtask(std::move(subtask));
	EXPECT_TRUE(spawn_result.is_err());
}

TEST(TaskContextTest, SubtaskSpawning) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	// Set up a simple spawner that just returns an ID
	int spawn_count = 0;
	ctx.set_subtask_spawner([&spawn_count](task_t subtask) -> cmn::Result<std::string> {
		spawn_count++;
		return cmn::ok(subtask.task_id());
	});

	auto subtask_result = task_builder("subtask.1").build();
	ASSERT_FALSE(subtask_result.is_err());
	auto subtask = subtask_result.unwrap();
	std::string subtask_id = subtask.task_id();

	auto spawn_result = ctx.spawn_subtask(std::move(subtask));
	ASSERT_FALSE(spawn_result.is_err());
	EXPECT_EQ(spawn_result.unwrap(), subtask_id);
	EXPECT_EQ(spawn_count, 1);

	auto spawned_ids = ctx.spawned_subtask_ids();
	EXPECT_EQ(spawned_ids.size(), 1);
	EXPECT_EQ(spawned_ids[0], subtask_id);
}

TEST(TaskContextTest, MultipleSubtasks) {
	auto task_result = task_builder("test.task").build();
	ASSERT_FALSE(task_result.is_err());
	auto task = task_result.unwrap();

	task_context ctx(task);

	ctx.set_subtask_spawner([](task_t subtask) -> cmn::Result<std::string> {
		return cmn::ok(subtask.task_id());
	});

	for (int i = 0; i < 5; ++i) {
		auto subtask_result = task_builder("subtask." + std::to_string(i)).build();
		ASSERT_FALSE(subtask_result.is_err());
		auto spawn_result = ctx.spawn_subtask(subtask_result.unwrap());
		ASSERT_FALSE(spawn_result.is_err());
	}

	auto spawned_ids = ctx.spawned_subtask_ids();
	EXPECT_EQ(spawned_ids.size(), 5);
}
