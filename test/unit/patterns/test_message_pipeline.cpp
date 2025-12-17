#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/message_pipeline.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

using namespace kcenon;
using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;

namespace {

template<typename Predicate>
bool wait_for_condition(Predicate&& pred, std::chrono::milliseconds timeout = std::chrono::milliseconds{1000}) {
	if (pred()) {
		return true;
	}

	std::mutex mtx;
	std::condition_variable cv;
	std::unique_lock<std::mutex> lock(mtx);

	auto deadline = std::chrono::steady_clock::now() + timeout;

	while (!pred()) {
		auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
			deadline - std::chrono::steady_clock::now());
		if (remaining <= std::chrono::milliseconds::zero()) {
			return false;
		}

		auto wait_time = std::min(remaining, std::chrono::milliseconds{50});
		cv.wait_for(lock, wait_time);
	}

	return true;
}

}  // namespace

class MessagePipelineTest : public ::testing::Test {
protected:
	std::shared_ptr<standalone_backend> backend_;
	std::shared_ptr<message_bus> bus_;

	void SetUp() override {
		// Create standalone backend
		backend_ = std::make_shared<standalone_backend>(2);

		// Create message bus
		message_bus_config config;
		config.worker_threads = 2;
		config.queue_capacity = 100;
		bus_ = std::make_shared<message_bus>(backend_, config);

		// Start the bus
		auto start_result = bus_->start();
		ASSERT_TRUE(start_result.is_ok()) << "Failed to start message bus";
	}

	void TearDown() override {
		if (bus_ && bus_->is_running()) {
			bus_->stop();
		}
	}
};

// ============================================================================
// Pipeline Construction Tests
// ============================================================================

TEST_F(MessagePipelineTest, PipelineConstruction) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");
	EXPECT_FALSE(pipeline.is_running());
	EXPECT_EQ(pipeline.stage_count(), 0);
}

TEST_F(MessagePipelineTest, PipelineAddStage) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	auto stage1 = [](const message& msg) -> common::Result<message> {
		return common::ok(message(msg));
	};

	pipeline.add_stage("stage1", stage1);
	EXPECT_EQ(pipeline.stage_count(), 1);

	auto names = pipeline.get_stage_names();
	ASSERT_EQ(names.size(), 1);
	EXPECT_EQ(names[0], "stage1");
}

TEST_F(MessagePipelineTest, PipelineAddMultipleStages) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	auto stage1 = [](const message& msg) -> common::Result<message> {
		return common::ok(message(msg));
	};

	auto stage2 = [](const message& msg) -> common::Result<message> {
		return common::ok(message(msg));
	};

	pipeline.add_stage("stage1", stage1)
		.add_stage("stage2", stage2);

	EXPECT_EQ(pipeline.stage_count(), 2);
}

TEST_F(MessagePipelineTest, PipelineRemoveStage) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	auto stage = [](const message& msg) -> common::Result<message> {
		return common::ok(message(msg));
	};

	pipeline.add_stage("test_stage", stage);
	EXPECT_EQ(pipeline.stage_count(), 1);

	auto remove_result = pipeline.remove_stage("test_stage");
	ASSERT_TRUE(remove_result.is_ok());
	EXPECT_EQ(pipeline.stage_count(), 0);
}

TEST_F(MessagePipelineTest, PipelineRemoveNonexistentStage) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	auto remove_result = pipeline.remove_stage("nonexistent");
	EXPECT_TRUE(remove_result.is_err());
}

// ============================================================================
// Pipeline Processing Tests
// ============================================================================

TEST_F(MessagePipelineTest, PipelineProcessMessage) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	std::atomic<int> process_count{0};
	auto stage = [&process_count](const message& msg) -> common::Result<message> {
		process_count++;
		return common::ok(message(msg));
	};

	pipeline.add_stage("counter", stage);

	message input("test.topic");
	auto result = pipeline.process(std::move(input));
	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(process_count.load(), 1);
}

TEST_F(MessagePipelineTest, PipelineMultiStageProcessing) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	std::atomic<int> stage1_count{0};
	std::atomic<int> stage2_count{0};
	std::atomic<int> stage3_count{0};

	pipeline.add_stage("stage1", [&stage1_count](const message& msg) -> common::Result<message> {
		stage1_count++;
		return common::ok(message(msg));
	});

	pipeline.add_stage("stage2", [&stage2_count](const message& msg) -> common::Result<message> {
		stage2_count++;
		return common::ok(message(msg));
	});

	pipeline.add_stage("stage3", [&stage3_count](const message& msg) -> common::Result<message> {
		stage3_count++;
		return common::ok(message(msg));
	});

	message input("test.topic");
	auto result = pipeline.process(std::move(input));
	ASSERT_TRUE(result.is_ok());

	EXPECT_EQ(stage1_count.load(), 1);
	EXPECT_EQ(stage2_count.load(), 1);
	EXPECT_EQ(stage3_count.load(), 1);
}

TEST_F(MessagePipelineTest, PipelineMessageTransformation) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	// Stage that modifies message priority
	pipeline.add_stage("priority_booster", [](const message& msg) -> common::Result<message> {
		message transformed(msg);
		transformed.metadata().priority = message_priority::high;
		return common::ok(std::move(transformed));
	});

	message input("test.topic");
	input.metadata().priority = message_priority::normal;

	auto result = pipeline.process(std::move(input));
	ASSERT_TRUE(result.is_ok());

	auto output = result.unwrap();
	EXPECT_EQ(output.metadata().priority, message_priority::high);
}

TEST_F(MessagePipelineTest, PipelineStageFailure) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	// Stage that fails
	pipeline.add_stage("failing_stage", [](const message& /* msg */) -> common::Result<message> {
		return common::make_error<message>(-1, "Stage failed");
	});

	message input("test.topic");
	auto result = pipeline.process(std::move(input));
	EXPECT_TRUE(result.is_err());
}

TEST_F(MessagePipelineTest, PipelineOptionalStageFailure) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	std::atomic<int> final_stage_count{0};

	// Optional failing stage
	pipeline.add_stage("optional_fail", [](const message& /* msg */) -> common::Result<message> {
		return common::make_error<message>(-1, "Optional stage failed");
	}, true);  // Mark as optional

	// Final stage that should still execute
	pipeline.add_stage("final_stage", [&final_stage_count](
		const message& msg
	) -> common::Result<message> {
		final_stage_count++;
		return common::ok(message(msg));
	});

	message input("test.topic");
	auto result = pipeline.process(std::move(input));

	// Pipeline should continue despite optional stage failure
	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(final_stage_count.load(), 1);
}

// ============================================================================
// Pipeline Runtime Tests
// ============================================================================

TEST_F(MessagePipelineTest, PipelineStartStop) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	auto start_result = pipeline.start();
	ASSERT_TRUE(start_result.is_ok());
	EXPECT_TRUE(pipeline.is_running());

	auto stop_result = pipeline.stop();
	ASSERT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(pipeline.is_running());
}

TEST_F(MessagePipelineTest, PipelineAutoProcessing) {
	message_pipeline pipeline(bus_, "pipeline.input", "pipeline.output");

	std::atomic<int> process_count{0};
	pipeline.add_stage("counter", [&process_count](const message& msg) -> common::Result<message> {
		process_count++;
		return common::ok(message(msg));
	});

	pipeline.start();

	// Subscribe to output topic to verify messages are processed
	std::atomic<int> output_count{0};
	auto sub_result = bus_->subscribe("pipeline.output", [&output_count](
		const message& /* msg */
	) -> common::VoidResult {
		output_count++;
		return common::ok();
	});
	ASSERT_TRUE(sub_result.is_ok());

	// Publish messages to input topic
	for (int i = 0; i < 5; ++i) {
		message msg("pipeline.input");
		bus_->publish(std::move(msg));
	}

	// Wait for processing
	ASSERT_TRUE(wait_for_condition([&output_count]() { return output_count.load() >= 5; }, std::chrono::milliseconds(500)));

	pipeline.stop();

	EXPECT_EQ(process_count.load(), 5);
	EXPECT_EQ(output_count.load(), 5);
}

// ============================================================================
// Pipeline Statistics Tests
// ============================================================================

TEST_F(MessagePipelineTest, PipelineStatistics) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	pipeline.add_stage("test_stage", [](const message& msg) -> common::Result<message> {
		return common::ok(message(msg));
	});

	// Process some messages
	for (int i = 0; i < 3; ++i) {
		message msg("test.topic");
		pipeline.process(std::move(msg));
	}

	auto stats = pipeline.get_statistics();
	EXPECT_EQ(stats.messages_processed, 3);
	EXPECT_EQ(stats.messages_succeeded, 3);
	EXPECT_EQ(stats.messages_failed, 0);
}

TEST_F(MessagePipelineTest, PipelineStatisticsWithFailures) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	std::atomic<int> counter{0};
	pipeline.add_stage("conditional", [&counter](
		const message& msg
	) -> common::Result<message> {
		int count = counter++;
		if (count % 2 == 0) {
			return common::ok(message(msg));
		}
		return common::make_error<message>(-1, "Even numbers only");
	});

	// Process 4 messages - 2 should succeed, 2 should fail
	for (int i = 0; i < 4; ++i) {
		message msg("test.topic");
		pipeline.process(std::move(msg));
	}

	auto stats = pipeline.get_statistics();
	EXPECT_EQ(stats.messages_processed, 4);
	EXPECT_EQ(stats.messages_succeeded, 2);
	EXPECT_EQ(stats.messages_failed, 2);
}

TEST_F(MessagePipelineTest, PipelineStatisticsReset) {
	message_pipeline pipeline(bus_, "input.topic", "output.topic");

	pipeline.add_stage("test", [](const message& msg) -> common::Result<message> {
		return common::ok(message(msg));  // msg is used here
	});

	// Process messages
	for (int i = 0; i < 5; ++i) {
		message msg("test.topic");
		pipeline.process(std::move(msg));
	}

	pipeline.reset_statistics();
	auto stats = pipeline.get_statistics();

	EXPECT_EQ(stats.messages_processed, 0);
	EXPECT_EQ(stats.messages_succeeded, 0);
	EXPECT_EQ(stats.messages_failed, 0);
}

// ============================================================================
// Pipeline Builder Tests
// ============================================================================

TEST_F(MessagePipelineTest, PipelineBuilderConstruction) {
	pipeline_builder builder(bus_);

	auto result = builder
		.from("input.topic")
		.to("output.topic")
		.build();

	ASSERT_TRUE(result.is_ok());
	auto pipeline = std::move(result.unwrap());
	EXPECT_FALSE(pipeline->is_running());
	EXPECT_EQ(pipeline->stage_count(), 0);
}

TEST_F(MessagePipelineTest, PipelineBuilderWithStages) {
	pipeline_builder builder(bus_);

	std::atomic<int> stage_count{0};

	auto result = builder
		.from("input.topic")
		.to("output.topic")
		.add_stage("stage1", [&stage_count](const message& msg) -> common::Result<message> {
			stage_count++;
			return common::ok(message(msg));
		})
		.add_stage("stage2", [&stage_count](const message& msg) -> common::Result<message> {
			stage_count++;
			return common::ok(message(msg));
		})
		.build();

	ASSERT_TRUE(result.is_ok());
	auto pipeline = std::move(result.unwrap());
	EXPECT_EQ(pipeline->stage_count(), 2);

	// Test processing
	message msg("test.topic");
	pipeline->process(std::move(msg));
	EXPECT_EQ(stage_count.load(), 2);
}

TEST_F(MessagePipelineTest, PipelineBuilderWithFilter) {
	pipeline_builder builder(bus_);

	std::atomic<int> processed{0};

	auto result = builder
		.from("input.topic")
		.to("output.topic")
		.add_filter("high_priority_only", [](const message& msg) {
			return msg.metadata().priority == message_priority::high;
		})
		.add_stage("processor", [&processed](const message& msg) -> common::Result<message> {
			processed++;
			return common::ok(message(msg));
		})
		.build();

	ASSERT_TRUE(result.is_ok());
	auto pipeline = std::move(result.unwrap());

	// Process low priority message - should be filtered out
	message low_msg("test.topic");
	low_msg.metadata().priority = message_priority::low;
	pipeline->process(std::move(low_msg));

	// Process high priority message - should pass through
	message high_msg("test.topic");
	high_msg.metadata().priority = message_priority::high;
	pipeline->process(std::move(high_msg));

	// Only high priority message should be processed
	EXPECT_EQ(processed.load(), 1);
}

TEST_F(MessagePipelineTest, PipelineBuilderWithTransformer) {
	pipeline_builder builder(bus_);

	auto result = builder
		.from("input.topic")
		.to("output.topic")
		.add_transformer("set_high_priority", [](const message& msg) {
			message transformed(msg);
			transformed.metadata().priority = message_priority::high;
			return transformed;
		})
		.build();

	ASSERT_TRUE(result.is_ok());
	auto pipeline = std::move(result.unwrap());

	message input("test.topic");
	input.metadata().priority = message_priority::low;

	auto process_result = pipeline->process(std::move(input));
	ASSERT_TRUE(process_result.is_ok());

	auto output = process_result.unwrap();
	EXPECT_EQ(output.metadata().priority, message_priority::high);
}

// ============================================================================
// Pipeline Stages Tests
// ============================================================================

TEST_F(MessagePipelineTest, ValidationStage) {
	auto validator = [](const message& msg) {
		return !msg.metadata().topic.empty();
	};

	auto validation_stage = pipeline_stages::create_validation_stage(validator);

	// Valid message
	message valid_msg("test.topic");
	auto valid_result = validation_stage(valid_msg);
	ASSERT_TRUE(valid_result.is_ok());

	// Invalid message (empty topic)
	message invalid_msg("");
	auto invalid_result = validation_stage(invalid_msg);
	EXPECT_TRUE(invalid_result.is_err());
}

TEST_F(MessagePipelineTest, EnrichmentStage) {
	auto enricher = [](message& msg) {
		msg.metadata().source = "test_source";
	};

	auto enrichment_stage = pipeline_stages::create_enrichment_stage(enricher);

	message msg("test.topic");
	auto result = enrichment_stage(msg);
	ASSERT_TRUE(result.is_ok());

	auto enriched = result.unwrap();
	EXPECT_EQ(enriched.metadata().source, "test_source");
}

TEST_F(MessagePipelineTest, RetryStage) {
	std::atomic<int> attempt_count{0};

	auto flaky_processor = [&attempt_count](const message& msg) -> common::Result<message> {
		int count = attempt_count++;
		if (count < 2) {
			return common::make_error<message>(-1, "Temporary failure");
		}
		return common::ok(message(msg));
	};

	auto retry_stage = pipeline_stages::create_retry_stage(
		flaky_processor, 3, std::chrono::milliseconds{10}
	);

	message msg("test.topic");
	auto result = retry_stage(msg);

	// Should succeed after 2 retries
	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(attempt_count.load(), 3);
}

TEST_F(MessagePipelineTest, RetryStageMaxAttemptsExceeded) {
	std::atomic<int> attempt_count{0};

	auto always_fail = [&attempt_count](const message& /* msg */) -> common::Result<message> {
		attempt_count++;
		return common::make_error<message>(-1, "Always fails");
	};

	auto retry_stage = pipeline_stages::create_retry_stage(
		always_fail, 2, std::chrono::milliseconds{10}
	);

	message msg("test.topic");
	auto result = retry_stage(msg);

	// Should fail after max retries
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(attempt_count.load(), 3);  // Initial + 2 retries
}
