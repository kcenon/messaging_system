#include <kcenon/messaging/task/task.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

namespace msg = kcenon::messaging;
namespace tsk = kcenon::messaging::task;
using tsk::task_state;
using tsk::task_config;
using tsk::task_builder;
using tsk::to_string;
using tsk::task_state_from_string;
using task_t = tsk::task;
using msg::message_priority;

// ============================================================================
// task_state tests
// ============================================================================

TEST(TaskStateTest, ToStringConversion) {
	EXPECT_EQ(to_string(task_state::pending), "pending");
	EXPECT_EQ(to_string(task_state::queued), "queued");
	EXPECT_EQ(to_string(task_state::running), "running");
	EXPECT_EQ(to_string(task_state::succeeded), "succeeded");
	EXPECT_EQ(to_string(task_state::failed), "failed");
	EXPECT_EQ(to_string(task_state::retrying), "retrying");
	EXPECT_EQ(to_string(task_state::cancelled), "cancelled");
	EXPECT_EQ(to_string(task_state::expired), "expired");
}

TEST(TaskStateTest, FromStringConversion) {
	EXPECT_EQ(task_state_from_string("pending"), task_state::pending);
	EXPECT_EQ(task_state_from_string("queued"), task_state::queued);
	EXPECT_EQ(task_state_from_string("running"), task_state::running);
	EXPECT_EQ(task_state_from_string("succeeded"), task_state::succeeded);
	EXPECT_EQ(task_state_from_string("failed"), task_state::failed);
	EXPECT_EQ(task_state_from_string("retrying"), task_state::retrying);
	EXPECT_EQ(task_state_from_string("cancelled"), task_state::cancelled);
	EXPECT_EQ(task_state_from_string("expired"), task_state::expired);
	EXPECT_EQ(task_state_from_string("unknown"), task_state::pending);  // default
}

// ============================================================================
// task_config tests
// ============================================================================

TEST(TaskConfigTest, DefaultValues) {
	task_config config;

	EXPECT_EQ(config.timeout.count(), 300000);  // 5 minutes
	EXPECT_EQ(config.max_retries, 3);
	EXPECT_EQ(config.retry_delay.count(), 1000);  // 1 second
	EXPECT_DOUBLE_EQ(config.retry_backoff_multiplier, 2.0);
	EXPECT_EQ(config.priority, message_priority::normal);
	EXPECT_FALSE(config.eta.has_value());
	EXPECT_FALSE(config.expires.has_value());
	EXPECT_EQ(config.queue_name, "default");
	EXPECT_TRUE(config.tags.empty());
}

// ============================================================================
// task tests - Construction
// ============================================================================

TEST(TaskTest, DefaultConstruction) {
	task_t t;

	EXPECT_FALSE(t.task_id().empty());
	EXPECT_TRUE(t.task_name().empty());
	EXPECT_EQ(t.state(), task_state::pending);
	EXPECT_EQ(t.attempt_count(), 0);
	EXPECT_DOUBLE_EQ(t.progress(), 0.0);
	EXPECT_FALSE(t.has_result());
	EXPECT_FALSE(t.has_error());
}

TEST(TaskTest, NamedConstruction) {
	task_t t("email.send");

	EXPECT_FALSE(t.task_id().empty());
	EXPECT_EQ(t.task_name(), "email.send");
	EXPECT_EQ(t.state(), task_state::pending);
}

TEST(TaskTest, UniqueTaskIds) {
	task_t t1("task1");
	task_t t2("task2");

	EXPECT_NE(t1.task_id(), t2.task_id());
}

// ============================================================================
// task tests - State Management
// ============================================================================

TEST(TaskTest, StateTransitions) {
	task_t t("test.task");

	EXPECT_EQ(t.state(), task_state::pending);

	t.set_state(task_state::queued);
	EXPECT_EQ(t.state(), task_state::queued);

	t.set_state(task_state::running);
	EXPECT_EQ(t.state(), task_state::running);

	t.set_state(task_state::succeeded);
	EXPECT_EQ(t.state(), task_state::succeeded);
}

TEST(TaskTest, IsTerminalState) {
	task_t t("test.task");

	EXPECT_FALSE(t.is_terminal_state());

	t.set_state(task_state::running);
	EXPECT_FALSE(t.is_terminal_state());

	t.set_state(task_state::succeeded);
	EXPECT_TRUE(t.is_terminal_state());

	task_t t2("test.task2");
	t2.set_state(task_state::failed);
	EXPECT_TRUE(t2.is_terminal_state());

	task_t t3("test.task3");
	t3.set_state(task_state::cancelled);
	EXPECT_TRUE(t3.is_terminal_state());

	task_t t4("test.task4");
	t4.set_state(task_state::expired);
	EXPECT_TRUE(t4.is_terminal_state());
}

// ============================================================================
// task tests - Progress Tracking
// ============================================================================

TEST(TaskTest, ProgressTracking) {
	task_t t("test.task");

	EXPECT_DOUBLE_EQ(t.progress(), 0.0);

	t.set_progress(0.5);
	EXPECT_DOUBLE_EQ(t.progress(), 0.5);

	t.set_progress(1.0);
	EXPECT_DOUBLE_EQ(t.progress(), 1.0);
}

TEST(TaskTest, ProgressClamping) {
	task_t t("test.task");

	t.set_progress(-0.5);
	EXPECT_DOUBLE_EQ(t.progress(), 0.0);

	t.set_progress(1.5);
	EXPECT_DOUBLE_EQ(t.progress(), 1.0);
}

TEST(TaskTest, ProgressMessage) {
	task_t t("test.task");

	EXPECT_TRUE(t.progress_message().empty());

	t.set_progress_message("Processing step 1");
	EXPECT_EQ(t.progress_message(), "Processing step 1");
}

// ============================================================================
// task tests - Attempt Tracking
// ============================================================================

TEST(TaskTest, AttemptTracking) {
	task_t t("test.task");

	EXPECT_EQ(t.attempt_count(), 0);

	t.increment_attempt();
	EXPECT_EQ(t.attempt_count(), 1);

	t.increment_attempt();
	EXPECT_EQ(t.attempt_count(), 2);
}

TEST(TaskTest, ShouldRetry) {
	task_t t("test.task");
	t.config().max_retries = 3;

	// Not failed yet, shouldn't retry
	EXPECT_FALSE(t.should_retry());

	// Failed but no attempts yet
	t.set_state(task_state::failed);
	EXPECT_TRUE(t.should_retry());

	// After max retries
	t.increment_attempt();
	t.increment_attempt();
	t.increment_attempt();
	EXPECT_FALSE(t.should_retry());
}

TEST(TaskTest, RetryDelay) {
	task_t t("test.task");
	t.config().retry_delay = std::chrono::milliseconds(1000);
	t.config().retry_backoff_multiplier = 2.0;

	// First retry: base delay
	auto delay = t.get_next_retry_delay();
	EXPECT_EQ(delay.count(), 1000);

	// After first attempt: 1000 * 2 = 2000
	t.increment_attempt();
	delay = t.get_next_retry_delay();
	EXPECT_EQ(delay.count(), 2000);

	// After second attempt: 1000 * 4 = 4000
	t.increment_attempt();
	delay = t.get_next_retry_delay();
	EXPECT_EQ(delay.count(), 4000);
}

// ============================================================================
// task tests - Execution Tracking
// ============================================================================

TEST(TaskTest, ExecutionTiming) {
	task_t t("test.task");

	auto now = std::chrono::system_clock::now();
	t.set_started_at(now);
	EXPECT_EQ(t.started_at(), now);

	auto later = now + std::chrono::seconds(10);
	t.set_completed_at(later);
	EXPECT_EQ(t.completed_at(), later);
}

// ============================================================================
// task tests - Result/Error Storage
// ============================================================================

TEST(TaskTest, ResultStorage) {
	task_t t("test.task");

	EXPECT_FALSE(t.has_result());

	auto result = std::make_shared<container_module::value_container>();
	t.set_result(result);
	EXPECT_TRUE(t.has_result());
}

TEST(TaskTest, ErrorStorage) {
	task_t t("test.task");

	EXPECT_FALSE(t.has_error());
	EXPECT_TRUE(t.error_message().empty());
	EXPECT_TRUE(t.error_traceback().empty());

	t.set_error("Something went wrong", "stack trace here");
	EXPECT_TRUE(t.has_error());
	EXPECT_EQ(t.error_message(), "Something went wrong");
	EXPECT_EQ(t.error_traceback(), "stack trace here");
}

// ============================================================================
// task tests - Expiration
// ============================================================================

TEST(TaskTest, NoExpiration) {
	task_t t("test.task");
	EXPECT_FALSE(t.is_expired());
}

TEST(TaskTest, NotExpiredYet) {
	task_t t("test.task");
	t.config().expires = std::chrono::milliseconds(1000);
	EXPECT_FALSE(t.is_expired());
}

TEST(TaskTest, ExpiredTask) {
	task_t t("test.task");
	t.config().expires = std::chrono::milliseconds(10);
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	EXPECT_TRUE(t.is_expired());
}

// ============================================================================
// task tests - Copy and Move
// ============================================================================

TEST(TaskTest, CopyConstruction) {
	task_t original("email.send");
	original.set_state(task_state::running);
	original.set_progress(0.5);
	original.increment_attempt();

	task_t copy(original);

	EXPECT_EQ(copy.task_id(), original.task_id());
	EXPECT_EQ(copy.task_name(), original.task_name());
	EXPECT_EQ(copy.state(), original.state());
	EXPECT_DOUBLE_EQ(copy.progress(), original.progress());
	EXPECT_EQ(copy.attempt_count(), original.attempt_count());
}

TEST(TaskTest, MoveConstruction) {
	task_t original("email.send");
	std::string original_id = original.task_id();
	original.set_state(task_state::running);

	task_t moved(std::move(original));

	EXPECT_EQ(moved.task_id(), original_id);
	EXPECT_EQ(moved.task_name(), "email.send");
	EXPECT_EQ(moved.state(), task_state::running);
}

// ============================================================================
// task_builder tests
// ============================================================================

TEST(TaskBuilderTest, BasicBuild) {
	auto result = task_builder("email.send").build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.task_name(), "email.send");
	EXPECT_EQ(t.state(), task_state::pending);
}

TEST(TaskBuilderTest, EmptyNameFails) {
	auto result = task_builder("").build();

	EXPECT_TRUE(result.is_err());
}

TEST(TaskBuilderTest, WithPriority) {
	auto result = task_builder("test.task")
					  .priority(message_priority::high)
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.config().priority, message_priority::high);
}

TEST(TaskBuilderTest, WithTimeout) {
	auto result = task_builder("test.task")
					  .timeout(std::chrono::milliseconds(60000))
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.config().timeout.count(), 60000);
}

TEST(TaskBuilderTest, WithRetries) {
	auto result = task_builder("test.task")
					  .retries(5)
					  .retry_delay(std::chrono::milliseconds(2000))
					  .retry_backoff(3.0)
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.config().max_retries, 5);
	EXPECT_EQ(t.config().retry_delay.count(), 2000);
	EXPECT_DOUBLE_EQ(t.config().retry_backoff_multiplier, 3.0);
}

TEST(TaskBuilderTest, WithQueue) {
	auto result = task_builder("test.task")
					  .queue("high-priority")
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.config().queue_name, "high-priority");
}

TEST(TaskBuilderTest, WithEta) {
	auto eta = std::chrono::system_clock::now() + std::chrono::hours(1);

	auto result = task_builder("test.task")
					  .eta(eta)
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	ASSERT_TRUE(t.config().eta.has_value());
	EXPECT_EQ(t.config().eta.value(), eta);
}

TEST(TaskBuilderTest, WithCountdown) {
	auto before = std::chrono::system_clock::now();

	auto result = task_builder("test.task")
					  .countdown(std::chrono::milliseconds(5000))
					  .build();

	auto after = std::chrono::system_clock::now();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	ASSERT_TRUE(t.config().eta.has_value());
	auto eta = t.config().eta.value();
	EXPECT_GE(eta, before + std::chrono::milliseconds(5000));
	EXPECT_LE(eta, after + std::chrono::milliseconds(5000));
}

TEST(TaskBuilderTest, WithExpires) {
	auto result = task_builder("test.task")
					  .expires(std::chrono::milliseconds(60000))
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	ASSERT_TRUE(t.config().expires.has_value());
	EXPECT_EQ(t.config().expires.value().count(), 60000);
}

TEST(TaskBuilderTest, WithTags) {
	auto result = task_builder("test.task")
					  .tag("important")
					  .tag("batch")
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.config().tags.size(), 2);
	EXPECT_EQ(t.config().tags[0], "important");
	EXPECT_EQ(t.config().tags[1], "batch");
}

TEST(TaskBuilderTest, WithTagsVector) {
	auto result = task_builder("test.task")
					  .tags({"tag1", "tag2", "tag3"})
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.config().tags.size(), 3);
}

TEST(TaskBuilderTest, FullConfiguration) {
	auto result = task_builder("email.send")
					  .priority(message_priority::high)
					  .timeout(std::chrono::milliseconds(120000))
					  .retries(5)
					  .retry_delay(std::chrono::milliseconds(2000))
					  .retry_backoff(1.5)
					  .queue("email-queue")
					  .expires(std::chrono::milliseconds(3600000))
					  .tag("notification")
					  .build();

	ASSERT_FALSE(result.is_err());
	auto t = result.unwrap();

	EXPECT_EQ(t.task_name(), "email.send");
	EXPECT_EQ(t.config().priority, message_priority::high);
	EXPECT_EQ(t.config().timeout.count(), 120000);
	EXPECT_EQ(t.config().max_retries, 5);
	EXPECT_EQ(t.config().retry_delay.count(), 2000);
	EXPECT_DOUBLE_EQ(t.config().retry_backoff_multiplier, 1.5);
	EXPECT_EQ(t.config().queue_name, "email-queue");
	ASSERT_TRUE(t.config().expires.has_value());
	EXPECT_EQ(t.config().expires.value().count(), 3600000);
	EXPECT_EQ(t.config().tags.size(), 1);
}

// ============================================================================
// task tests - Serialization
// ============================================================================

TEST(TaskTest, SerializeDeserialize) {
	auto build_result = task_builder("email.send").build();
	ASSERT_FALSE(build_result.is_err());

	auto original = build_result.unwrap();
	original.set_state(task_state::running);

	auto serialize_result = original.serialize();
	ASSERT_FALSE(serialize_result.is_err());

	auto data = serialize_result.unwrap();
	EXPECT_FALSE(data.empty());

	auto deserialize_result = task_t::deserialize(data);
	ASSERT_FALSE(deserialize_result.is_err());

	auto restored = deserialize_result.unwrap();
	EXPECT_EQ(restored.task_id(), original.task_id());
	EXPECT_EQ(restored.task_name(), original.task_name());
	EXPECT_EQ(restored.state(), original.state());
}

TEST(TaskTest, DeserializeEmptyDataFails) {
	std::vector<uint8_t> empty_data;
	auto result = task_t::deserialize(empty_data);
	EXPECT_TRUE(result.is_err());
}

TEST(TaskTest, DeserializeInvalidVersionFails) {
	std::vector<uint8_t> bad_data = {0x99};  // Invalid version
	auto result = task_t::deserialize(bad_data);
	EXPECT_TRUE(result.is_err());
}
