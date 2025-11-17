#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/event_streaming.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <chrono>
#include <thread>
#include <atomic>

using namespace kcenon;
using namespace kcenon::messaging;
using namespace kcenon::messaging::patterns;

class EventStreamingTest : public ::testing::Test {
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
// Event Stream Construction Tests
// ============================================================================

TEST_F(EventStreamingTest, EventStreamConstruction) {
	event_stream_config config;
	config.max_buffer_size = 100;
	config.enable_replay = true;

	event_stream stream(bus_, "events.test", config);
	EXPECT_EQ(stream.get_stream_topic(), "events.test");
	EXPECT_EQ(stream.event_count(), 0);
}

TEST_F(EventStreamingTest, EventStreamPublish) {
	event_stream stream(bus_, "events.test");

	// Create and publish an event
	message event("events.test", message_type::event);
	auto result = stream.publish_event(std::move(event));
	ASSERT_TRUE(result.is_ok());

	// Allow time for event to be buffered
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Check that event was buffered
	EXPECT_EQ(stream.event_count(), 1);
}

TEST_F(EventStreamingTest, EventStreamSubscribe) {
	event_stream stream(bus_, "events.test");

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& msg) -> common::VoidResult {
		event_count++;
		return common::ok();
	};

	// Subscribe to stream
	auto sub_result = stream.subscribe(callback);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish events
	for (int i = 0; i < 5; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}

	// Wait for events to be processed
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(event_count.load(), 5);
}

TEST_F(EventStreamingTest, EventStreamUnsubscribe) {
	event_stream stream(bus_, "events.test");

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& msg) -> common::VoidResult {
		event_count++;
		return common::ok();
	};

	// Subscribe and get subscription ID
	auto sub_result = stream.subscribe(callback);
	ASSERT_TRUE(sub_result.is_ok());
	uint64_t sub_id = sub_result.unwrap();

	// Publish one event
	message event1("events.test", message_type::event);
	stream.publish_event(std::move(event1));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Unsubscribe
	auto unsub_result = stream.unsubscribe(sub_id);
	ASSERT_TRUE(unsub_result.is_ok());

	// Publish another event - should not be received
	message event2("events.test", message_type::event);
	stream.publish_event(std::move(event2));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Should only have received one event
	EXPECT_EQ(event_count.load(), 1);
}

// ============================================================================
// Event Replay Tests
// ============================================================================

TEST_F(EventStreamingTest, EventReplayDisabled) {
	event_stream_config config;
	config.enable_replay = false;
	event_stream stream(bus_, "events.test", config);

	// Publish events before subscribing
	for (int i = 0; i < 3; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& msg) -> common::VoidResult {
		event_count++;
		return common::ok();
	};

	// Subscribe with replay disabled
	auto sub_result = stream.subscribe(callback, false);
	ASSERT_TRUE(sub_result.is_ok());

	// Wait and check - should not receive past events
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	EXPECT_EQ(event_count.load(), 0);

	// Publish new event - should be received
	message new_event("events.test", message_type::event);
	stream.publish_event(std::move(new_event));
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	EXPECT_EQ(event_count.load(), 1);
}

TEST_F(EventStreamingTest, EventReplayEnabled) {
	event_stream_config config;
	config.enable_replay = true;
	config.max_buffer_size = 100;
	event_stream stream(bus_, "events.test", config);

	// Publish events before subscribing
	for (int i = 0; i < 5; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& msg) -> common::VoidResult {
		event_count++;
		return common::ok();
	};

	// Subscribe with replay enabled
	auto sub_result = stream.subscribe(callback, true);
	ASSERT_TRUE(sub_result.is_ok());

	// Wait for replay to complete
	std::this_thread::sleep_for(std::chrono::milliseconds(150));

	// Should receive all 5 past events
	EXPECT_EQ(event_count.load(), 5);
}

TEST_F(EventStreamingTest, EventReplayWithFilter) {
	event_stream stream(bus_, "events.test");

	// Publish events with different priorities
	for (int i = 0; i < 10; ++i) {
		message event("events.test", message_type::event);
		if (i % 2 == 0) {
			event.metadata().priority = message_priority::high;
		} else {
			event.metadata().priority = message_priority::low;
		}
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::atomic<int> high_priority_count{0};
	auto callback = [&high_priority_count](const message& msg) -> common::VoidResult {
		if (msg.metadata().priority == message_priority::high) {
			high_priority_count++;
		}
		return common::ok();
	};

	// Filter for high priority events only
	auto filter = [](const message& msg) {
		return msg.metadata().priority == message_priority::high;
	};

	// Subscribe with filter and replay
	auto sub_result = stream.subscribe(callback, filter, true);
	ASSERT_TRUE(sub_result.is_ok());

	std::this_thread::sleep_for(std::chrono::milliseconds(150));

	// Should only receive 5 high-priority events
	EXPECT_EQ(high_priority_count.load(), 5);
}

TEST_F(EventStreamingTest, ManualReplay) {
	event_stream stream(bus_, "events.test");

	// Publish events
	for (int i = 0; i < 7; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::atomic<int> replayed_count{0};
	auto callback = [&replayed_count](const message& msg) -> common::VoidResult {
		replayed_count++;
		return common::ok();
	};

	// Manually replay events
	auto replay_result = stream.replay(callback);
	ASSERT_TRUE(replay_result.is_ok());

	EXPECT_EQ(replayed_count.load(), 7);
}

// ============================================================================
// Event Buffer Tests
// ============================================================================

TEST_F(EventStreamingTest, EventBufferSize) {
	event_stream_config config;
	config.max_buffer_size = 5;
	event_stream stream(bus_, "events.test", config);

	// Publish more events than buffer size
	for (int i = 0; i < 10; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Buffer should only contain last 5 events
	EXPECT_LE(stream.event_count(), 5);
}

TEST_F(EventStreamingTest, GetEvents) {
	event_stream stream(bus_, "events.test");

	// Publish events
	for (int i = 0; i < 5; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Get all events
	auto events = stream.get_events();
	EXPECT_EQ(events.size(), 5);
}

TEST_F(EventStreamingTest, GetEventsWithFilter) {
	event_stream stream(bus_, "events.test");

	// Publish events with different types
	for (int i = 0; i < 10; ++i) {
		message event("events.test");
		if (i % 3 == 0) {
			event.metadata().type = message_type::event;
		} else {
			event.metadata().type = message_type::command;
		}
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Get only event type messages
	auto filter = [](const message& msg) {
		return msg.metadata().type == message_type::event;
	};
	auto events = stream.get_events(filter);

	// Should have 4 events (indices 0, 3, 6, 9)
	EXPECT_EQ(events.size(), 4);
}

TEST_F(EventStreamingTest, ClearBuffer) {
	event_stream stream(bus_, "events.test");

	// Publish events
	for (int i = 0; i < 5; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	EXPECT_EQ(stream.event_count(), 5);

	// Clear buffer
	stream.clear_buffer();
	EXPECT_EQ(stream.event_count(), 0);
}

// ============================================================================
// Batch Processor Tests
// ============================================================================

TEST_F(EventStreamingTest, BatchProcessorConstruction) {
	std::atomic<int> batch_count{0};
	auto batch_callback = [&batch_count](const std::vector<message>& batch) -> common::VoidResult {
		batch_count++;
		return common::ok();
	};

	event_batch_processor processor(bus_, "events.batch", batch_callback, 5);
	EXPECT_FALSE(processor.is_running());
}

TEST_F(EventStreamingTest, BatchProcessorStart) {
	std::atomic<int> batch_count{0};
	auto batch_callback = [&batch_count](const std::vector<message>& batch) -> common::VoidResult {
		batch_count++;
		return common::ok();
	};

	event_batch_processor processor(bus_, "events.batch", batch_callback, 5);

	auto start_result = processor.start();
	ASSERT_TRUE(start_result.is_ok());
	EXPECT_TRUE(processor.is_running());

	auto stop_result = processor.stop();
	ASSERT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(processor.is_running());
}

TEST_F(EventStreamingTest, BatchProcessorBatchSize) {
	std::atomic<int> batch_count{0};
	std::atomic<int> total_events{0};

	auto batch_callback = [&batch_count, &total_events](
		const std::vector<message>& batch
	) -> common::VoidResult {
		batch_count++;
		total_events += static_cast<int>(batch.size());
		return common::ok();
	};

	event_batch_processor processor(
		bus_, "events.batch", batch_callback, 3, std::chrono::milliseconds{1000}
	);
	processor.start();

	// Publish 9 events - should trigger 3 batches of 3
	for (int i = 0; i < 9; ++i) {
		message event("events.batch", message_type::event);
		bus_->publish(std::move(event));
	}

	// Wait for batches to process
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	processor.stop();

	EXPECT_GE(batch_count.load(), 3);
	EXPECT_GE(total_events.load(), 9);
}

TEST_F(EventStreamingTest, BatchProcessorFlush) {
	std::atomic<int> batch_count{0};
	std::atomic<int> last_batch_size{0};

	auto batch_callback = [&batch_count, &last_batch_size](
		const std::vector<message>& batch
	) -> common::VoidResult {
		batch_count++;
		last_batch_size = static_cast<int>(batch.size());
		return common::ok();
	};

	event_batch_processor processor(
		bus_, "events.batch", batch_callback, 10, std::chrono::milliseconds{10000}
	);
	processor.start();

	// Publish 5 events - not enough to trigger batch automatically
	for (int i = 0; i < 5; ++i) {
		message event("events.batch", message_type::event);
		bus_->publish(std::move(event));
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	// Manually flush
	auto flush_result = processor.flush();
	ASSERT_TRUE(flush_result.is_ok());

	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	processor.stop();

	EXPECT_GE(batch_count.load(), 1);
	EXPECT_EQ(last_batch_size.load(), 5);
}
