#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/event_streaming.h>
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

/**
 * @brief Wait for a condition with timeout using condition variable
 */
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

	// Wait for event to be buffered
	ASSERT_TRUE(wait_for_condition(
		[&stream]() { return stream.event_count() >= 1; },
		std::chrono::milliseconds(100)
	));

	// Check that event was buffered
	EXPECT_EQ(stream.event_count(), 1);
}

TEST_F(EventStreamingTest, EventStreamSubscribe) {
	event_stream stream(bus_, "events.test");

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& /* msg */) -> common::VoidResult {
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
	ASSERT_TRUE(wait_for_condition(
		[&event_count]() { return event_count.load() >= 5; },
		std::chrono::milliseconds(200)
	));

	EXPECT_EQ(event_count.load(), 5);
}

TEST_F(EventStreamingTest, EventStreamUnsubscribe) {
	event_stream stream(bus_, "events.test");

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& /* msg */) -> common::VoidResult {
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

	// Wait for first event to be received
	ASSERT_TRUE(wait_for_condition(
		[&event_count]() { return event_count.load() >= 1; },
		std::chrono::milliseconds(100)
	));

	// Unsubscribe
	auto unsub_result = stream.unsubscribe(sub_id);
	ASSERT_TRUE(unsub_result.is_ok());

	// Publish another event - should not be received
	message event2("events.test", message_type::event);
	stream.publish_event(std::move(event2));

	// Verify no additional events are received (wait and check count doesn't increase)
	bool unexpected_event = wait_for_condition(
		[&event_count]() { return event_count.load() > 1; },
		std::chrono::milliseconds(100)
	);
	EXPECT_FALSE(unexpected_event) << "Event received after unsubscribe";

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

	// Wait for events to be buffered
	wait_for_condition(
		[&stream]() { return stream.event_count() >= 3; },
		std::chrono::milliseconds(100)
	);

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& /* msg */) -> common::VoidResult {
		event_count++;
		return common::ok();
	};

	// Subscribe with replay disabled
	auto sub_result = stream.subscribe(callback, false);
	ASSERT_TRUE(sub_result.is_ok());

	// Verify no past events are received (wait and check count stays at 0)
	bool received_past_events = wait_for_condition(
		[&event_count]() { return event_count.load() > 0; },
		std::chrono::milliseconds(100)
	);
	EXPECT_FALSE(received_past_events) << "Past events received when replay disabled";
	EXPECT_EQ(event_count.load(), 0);

	// Publish new event - should be received
	message new_event("events.test", message_type::event);
	stream.publish_event(std::move(new_event));

	// Wait for new event to be received
	ASSERT_TRUE(wait_for_condition(
		[&event_count]() { return event_count.load() >= 1; },
		std::chrono::milliseconds(100)
	));
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

	// Wait for events to be buffered
	wait_for_condition(
		[&stream]() { return stream.event_count() >= 5; },
		std::chrono::milliseconds(200)
	);

	std::atomic<int> event_count{0};
	auto callback = [&event_count](const message& /* msg */) -> common::VoidResult {
		event_count++;
		return common::ok();
	};

	// Subscribe with replay enabled
	auto sub_result = stream.subscribe(callback, true);
	ASSERT_TRUE(sub_result.is_ok());

	// Wait for replay to complete - should receive all 5 past events
	ASSERT_TRUE(wait_for_condition(
		[&event_count]() { return event_count.load() >= 5; },
		std::chrono::milliseconds(300)
	));

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

	// Wait for events to be buffered
	wait_for_condition(
		[&stream]() { return stream.event_count() >= 10; },
		std::chrono::milliseconds(200)
	);

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

	// Wait for filtered replay - should only receive 5 high-priority events
	ASSERT_TRUE(wait_for_condition(
		[&high_priority_count]() { return high_priority_count.load() >= 5; },
		std::chrono::milliseconds(300)
	));

	EXPECT_EQ(high_priority_count.load(), 5);
}

TEST_F(EventStreamingTest, ManualReplay) {
	event_stream stream(bus_, "events.test");

	// Publish events
	for (int i = 0; i < 7; ++i) {
		message event("events.test", message_type::event);
		stream.publish_event(std::move(event));
	}

	// Wait for events to be buffered
	wait_for_condition(
		[&stream]() { return stream.event_count() >= 7; },
		std::chrono::milliseconds(200)
	);

	std::atomic<int> replayed_count{0};
	auto callback = [&replayed_count](const message& /* msg */) -> common::VoidResult {
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

	// Wait for events to be processed (buffer will cap at max_buffer_size)
	wait_for_condition(
		[&stream]() { return stream.event_count() >= 5; },
		std::chrono::milliseconds(200)
	);

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

	// Wait for events to be buffered
	ASSERT_TRUE(wait_for_condition(
		[&stream]() { return stream.event_count() >= 5; },
		std::chrono::milliseconds(200)
	));

	// Get all events
	auto events = stream.get_events();
	EXPECT_EQ(events.size(), 5);
}

TEST_F(EventStreamingTest, GetEventsWithFilter) {
	event_stream stream(bus_, "events.test");

	// Publish events with different priorities
	for (int i = 0; i < 10; ++i) {
		message event("events.test");
		if (i % 3 == 0) {
			event.metadata().priority = message_priority::high;
		} else {
			event.metadata().priority = message_priority::normal;
		}
		stream.publish_event(std::move(event));
	}

	// Wait for events to be buffered
	wait_for_condition(
		[&stream]() { return stream.event_count() >= 10; },
		std::chrono::milliseconds(200)
	);

	// Get only high priority messages
	auto filter = [](const message& msg) {
		return msg.metadata().priority == message_priority::high;
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

	// Wait for events to be buffered
	ASSERT_TRUE(wait_for_condition(
		[&stream]() { return stream.event_count() >= 5; },
		std::chrono::milliseconds(200)
	));

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
	auto batch_callback = [&batch_count](const std::vector<message>& /* batch */) -> common::VoidResult {
		batch_count++;
		return common::ok();
	};

	event_batch_processor processor(bus_, "events.batch", batch_callback, 5);
	EXPECT_FALSE(processor.is_running());
}

TEST_F(EventStreamingTest, BatchProcessorStart) {
	std::atomic<int> batch_count{0};
	auto batch_callback = [&batch_count](const std::vector<message>& /* batch */) -> common::VoidResult {
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

	// Wait for batches to be processed
	ASSERT_TRUE(wait_for_condition(
		[&total_events]() { return total_events.load() >= 9; },
		std::chrono::milliseconds(500)
	));

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

	// Small delay to allow events to be queued
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Manually flush
	auto flush_result = processor.flush();
	ASSERT_TRUE(flush_result.is_ok());

	// Wait for flush to complete
	ASSERT_TRUE(wait_for_condition(
		[&batch_count]() { return batch_count.load() >= 1; },
		std::chrono::milliseconds(200)
	));

	processor.stop();

	EXPECT_GE(batch_count.load(), 1);
	EXPECT_EQ(last_batch_size.load(), 5);
}
