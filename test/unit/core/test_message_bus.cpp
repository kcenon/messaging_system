#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/error/error_codes.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace kcenon::messaging;
using namespace kcenon::common;

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
// Use namespace alias to avoid ambiguity with common::error
namespace msg_error = kcenon::messaging::error;

class MessageBusTest : public ::testing::Test {
protected:
	void SetUp() override {
		backend_ = std::make_shared<standalone_backend>(2);

		message_bus_config config;
		config.queue_capacity = 100;
		config.worker_threads = 2;
		config.enable_priority_queue = true;
		config.enable_dead_letter_queue = true;
		config.enable_metrics = true;

		bus_ = std::make_unique<message_bus>(backend_, config);
	}

	void TearDown() override {
		if (bus_ && bus_->is_running()) {
			bus_->stop();
		}
	}

	std::shared_ptr<standalone_backend> backend_;
	std::unique_ptr<message_bus> bus_;
};

// Lifecycle tests
TEST_F(MessageBusTest, InitiallyNotRunning) {
	EXPECT_FALSE(bus_->is_running());
}

TEST_F(MessageBusTest, StartStop) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());
	EXPECT_TRUE(bus_->is_running());

	auto stop_result = bus_->stop();
	ASSERT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(bus_->is_running());
}

TEST_F(MessageBusTest, CannotStartTwice) {
	auto result1 = bus_->start();
	ASSERT_TRUE(result1.is_ok());

	auto result2 = bus_->start();
	EXPECT_TRUE(result2.is_err());
	EXPECT_EQ(result2.error().code, msg_error::already_running);

	bus_->stop();
}

TEST_F(MessageBusTest, CannotStopWhenNotRunning) {
	auto result = bus_->stop();
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(result.error().code, msg_error::not_running);
}

// Publishing tests
TEST_F(MessageBusTest, PublishWhenNotRunning) {
	message msg("test.topic");
	auto result = bus_->publish(msg);
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(result.error().code, msg_error::not_running);
}

TEST_F(MessageBusTest, PublishSuccess) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	message msg("test.topic");
	auto result = bus_->publish(msg);
	EXPECT_TRUE(result.is_ok());

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);

	bus_->stop();
}

TEST_F(MessageBusTest, PublishWithTopicParameter) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	message msg;
	auto result = bus_->publish("test.topic", std::move(msg));
	EXPECT_TRUE(result.is_ok());

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);

	bus_->stop();
}

// Subscription tests
TEST_F(MessageBusTest, SubscribeSuccess) {
	auto result = bus_->subscribe(
		"test.topic",
		[](const message& /* msg */) { return ::kcenon::common::ok(); }
	);
	EXPECT_TRUE(result.is_ok());
}

TEST_F(MessageBusTest, UnsubscribeSuccess) {
	auto sub_result = bus_->subscribe(
		"test.topic",
		[](const message& /* msg */) { return ::kcenon::common::ok(); }
	);
	ASSERT_TRUE(sub_result.is_ok());

	uint64_t sub_id = sub_result.unwrap();
	auto unsub_result = bus_->unsubscribe(sub_id);
	EXPECT_TRUE(unsub_result.is_ok());
}

TEST_F(MessageBusTest, UnsubscribeInvalidId) {
	auto result = bus_->unsubscribe(9999);
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(result.error().code, msg_error::subscription_not_found);
}

// Pub/Sub integration tests
TEST_F(MessageBusTest, PubSubBasic) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish message
	message msg("test.topic");
	auto pub_result = bus_->publish(msg);
	ASSERT_TRUE(pub_result.is_ok());

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 1; },
		std::chrono::milliseconds(200)
	));

	EXPECT_EQ(received_count.load(), 1);

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);
	EXPECT_EQ(stats.messages_processed, 1);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubMultipleMessages) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish multiple messages
	for (int i = 0; i < 10; ++i) {
		message msg("test.topic");
		auto pub_result = bus_->publish(msg);
		ASSERT_TRUE(pub_result.is_ok());
	}

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 10; },
		std::chrono::milliseconds(500)
	));

	EXPECT_EQ(received_count.load(), 10);

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 10);
	EXPECT_EQ(stats.messages_processed, 10);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubMultipleSubscribers) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> subscriber1_count{0};
	std::atomic<int> subscriber2_count{0};

	auto sub1_result = bus_->subscribe(
		"test.topic",
		[&subscriber1_count](const message& /* msg */) {
			subscriber1_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub1_result.is_ok());

	auto sub2_result = bus_->subscribe(
		"test.topic",
		[&subscriber2_count](const message& /* msg */) {
			subscriber2_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub2_result.is_ok());

	// Publish message
	message msg("test.topic");
	auto pub_result = bus_->publish(msg);
	ASSERT_TRUE(pub_result.is_ok());

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&subscriber1_count, &subscriber2_count]() {
			return subscriber1_count.load() >= 1 && subscriber2_count.load() >= 1;
		},
		std::chrono::milliseconds(200)
	));

	EXPECT_EQ(subscriber1_count.load(), 1);
	EXPECT_EQ(subscriber2_count.load(), 1);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubWithWildcard) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.*",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish messages with different topics
	message msg1("test.topic1");
	bus_->publish(msg1);

	message msg2("test.topic2");
	bus_->publish(msg2);

	message msg3("other.topic");
	bus_->publish(msg3);

	// Wait for processing - should receive 2 messages matching "test.*"
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 2; },
		std::chrono::milliseconds(300)
	));

	// Should receive only messages matching "test.*"
	EXPECT_EQ(received_count.load(), 2);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubWithFilter) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		},
		[](const message& msg) {
			// Only accept high priority messages
			return msg.metadata().priority == message_priority::high;
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish normal priority message
	message msg1("test.topic");
	msg1.metadata().priority = message_priority::normal;
	bus_->publish(msg1);

	// Publish high priority message
	message msg2("test.topic");
	msg2.metadata().priority = message_priority::high;
	bus_->publish(msg2);

	// Wait for processing - should receive 1 high priority message
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 1; },
		std::chrono::milliseconds(300)
	));

	// Should receive only high priority message
	EXPECT_EQ(received_count.load(), 1);

	bus_->stop();
}

// Statistics tests
TEST_F(MessageBusTest, Statistics) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	auto stats_before = bus_->get_statistics();
	EXPECT_EQ(stats_before.messages_published, 0);
	EXPECT_EQ(stats_before.messages_processed, 0);
	EXPECT_EQ(stats_before.messages_failed, 0);
	EXPECT_EQ(stats_before.messages_dropped, 0);

	// Subscribe
	auto sub_result = bus_->subscribe(
		"test.topic",
		[](const message& /* msg */) { return ::kcenon::common::ok(); }
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish
	message msg("test.topic");
	bus_->publish(msg);

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[this]() { return bus_->get_statistics().messages_processed >= 1; },
		std::chrono::milliseconds(200)
	));

	auto stats_after = bus_->get_statistics();
	EXPECT_EQ(stats_after.messages_published, 1);
	EXPECT_EQ(stats_after.messages_processed, 1);

	bus_->stop();
}

TEST_F(MessageBusTest, ResetStatistics) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	// Publish some messages
	for (int i = 0; i < 5; ++i) {
		message msg("test.topic");
		bus_->publish(msg);
	}

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 5);

	// Reset
	bus_->reset_statistics();

	auto stats_after = bus_->get_statistics();
	EXPECT_EQ(stats_after.messages_published, 0);
	EXPECT_EQ(stats_after.messages_processed, 0);
	EXPECT_EQ(stats_after.messages_failed, 0);
	EXPECT_EQ(stats_after.messages_dropped, 0);

	bus_->stop();
}
