#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

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
		auto remaining = deadline - std::chrono::steady_clock::now();
		if (remaining <= std::chrono::milliseconds::zero()) {
			return false;
		}

		auto wait_time = std::min(remaining, std::chrono::milliseconds{50});
		cv.wait_for(lock, wait_time);
	}

	return true;
}

}  // namespace

class PubSubTest : public ::testing::Test {
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
// Publisher Tests
// ============================================================================

TEST_F(PubSubTest, PublisherConstruction) {
	// Test constructor without default topic
	publisher pub1(bus_);
	EXPECT_TRUE(pub1.get_default_topic().empty());
	EXPECT_TRUE(pub1.is_ready());

	// Test constructor with default topic
	publisher pub2(bus_, "test.topic");
	EXPECT_EQ(pub2.get_default_topic(), "test.topic");
	EXPECT_TRUE(pub2.is_ready());
}

TEST_F(PubSubTest, PublisherSetDefaultTopic) {
	publisher pub(bus_);
	EXPECT_TRUE(pub.get_default_topic().empty());

	pub.set_default_topic("new.topic");
	EXPECT_EQ(pub.get_default_topic(), "new.topic");
}

TEST_F(PubSubTest, PublisherIsReady) {
	publisher pub(bus_, "test.topic");
	EXPECT_TRUE(pub.is_ready());

	// Stop bus
	bus_->stop();
	EXPECT_FALSE(pub.is_ready());
}

TEST_F(PubSubTest, PublisherPublishToDefaultTopic) {
	publisher pub(bus_, "test.default");

	// Subscribe to the default topic
	bool received = false;
	std::string received_topic;
	auto sub_result = bus_->subscribe("test.default", [&](const message& msg) {
		received = true;
		received_topic = msg.metadata().topic;
		return common::ok();
	});
	ASSERT_TRUE(sub_result.is_ok());

	// Publish message to default topic
	message msg("test.default");
	auto pub_result = pub.publish(std::move(msg));
	EXPECT_TRUE(pub_result.is_ok());

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&received]() { return received; }, std::chrono::milliseconds(200)));

	EXPECT_TRUE(received);
	EXPECT_EQ(received_topic, "test.default");
}

TEST_F(PubSubTest, PublisherPublishToSpecificTopic) {
	publisher pub(bus_, "test.default");

	// Subscribe to a different topic
	bool received = false;
	std::string received_topic;
	auto sub_result = bus_->subscribe("test.specific", [&](const message& msg) {
		received = true;
		received_topic = msg.metadata().topic;
		return common::ok();
	});
	ASSERT_TRUE(sub_result.is_ok());

	// Publish message to specific topic (not default)
	message msg("test.specific");
	auto pub_result = pub.publish("test.specific", std::move(msg));
	EXPECT_TRUE(pub_result.is_ok());

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&received]() { return received; }, std::chrono::milliseconds(200)));

	EXPECT_TRUE(received);
	EXPECT_EQ(received_topic, "test.specific");
}

TEST_F(PubSubTest, PublisherPublishWithoutDefaultTopic) {
	publisher pub(bus_);  // No default topic

	// This should work because we specify the topic in the message
	message msg("test.topic");
	auto pub_result = pub.publish(std::move(msg));
	EXPECT_TRUE(pub_result.is_ok());
}

// ============================================================================
// Subscriber Tests
// ============================================================================

TEST_F(PubSubTest, SubscriberConstruction) {
	subscriber sub(bus_);
	EXPECT_EQ(sub.subscription_count(), 0);
	EXPECT_FALSE(sub.has_subscriptions());
}

TEST_F(PubSubTest, SubscriberSubscribe) {
	subscriber sub(bus_);

	bool received = false;
	auto result = sub.subscribe("test.topic", [&](const message& msg) {
		received = true;
		return common::ok();
	});

	ASSERT_TRUE(result.is_ok());
	EXPECT_GT(result.value(), 0);  // Valid subscription ID
	EXPECT_EQ(sub.subscription_count(), 1);
	EXPECT_TRUE(sub.has_subscriptions());

	// Publish a message
	message msg("test.topic");
	auto pub_result = bus_->publish(std::move(msg));
	ASSERT_TRUE(pub_result.is_ok());

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&received]() { return received; }, std::chrono::milliseconds(200)));
	EXPECT_TRUE(received);
}

TEST_F(PubSubTest, SubscriberMultipleSubscriptions) {
	subscriber sub(bus_);

	int count = 0;
	auto result1 = sub.subscribe("test.topic1", [&](const message&) {
		count++;
		return common::ok();
	});
	auto result2 = sub.subscribe("test.topic2", [&](const message&) {
		count++;
		return common::ok();
	});

	ASSERT_TRUE(result1.is_ok());
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(sub.subscription_count(), 2);
	EXPECT_TRUE(sub.has_subscriptions());

	// Publish to both topics
	message msg1("test.topic1");
	message msg2("test.topic2");
	bus_->publish(std::move(msg1));
	bus_->publish(std::move(msg2));

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&count]() { return count >= 2; }, std::chrono::milliseconds(200)));
	EXPECT_EQ(count, 2);
}

TEST_F(PubSubTest, SubscriberUnsubscribe) {
	subscriber sub(bus_);

	bool received = false;
	auto result = sub.subscribe("test.topic", [&](const message&) {
		received = true;
		return common::ok();
	});
	ASSERT_TRUE(result.is_ok());

	uint64_t sub_id = result.value();
	EXPECT_EQ(sub.subscription_count(), 1);

	// Unsubscribe
	auto unsub_result = sub.unsubscribe(sub_id);
	EXPECT_TRUE(unsub_result.is_ok());
	EXPECT_EQ(sub.subscription_count(), 0);
	EXPECT_FALSE(sub.has_subscriptions());

	// Publish message - should not be received
	message msg("test.topic");
	bus_->publish(std::move(msg));

	// Verify message is NOT received after unsubscribe
	bool unexpected = wait_for_condition([&received]() { return received; }, std::chrono::milliseconds(100));
	EXPECT_FALSE(unexpected) << "Message received after unsubscribe";
	EXPECT_FALSE(received);
}

TEST_F(PubSubTest, SubscriberUnsubscribeAll) {
	subscriber sub(bus_);

	int count = 0;
	sub.subscribe("test.topic1", [&](const message&) {
		count++;
		return common::ok();
	});
	sub.subscribe("test.topic2", [&](const message&) {
		count++;
		return common::ok();
	});
	sub.subscribe("test.topic3", [&](const message&) {
		count++;
		return common::ok();
	});

	EXPECT_EQ(sub.subscription_count(), 3);

	// Unsubscribe all
	auto result = sub.unsubscribe_all();
	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(sub.subscription_count(), 0);
	EXPECT_FALSE(sub.has_subscriptions());

	// Publish messages - should not be received
	message msg1("test.topic1");
	message msg2("test.topic2");
	message msg3("test.topic3");
	bus_->publish(std::move(msg1));
	bus_->publish(std::move(msg2));
	bus_->publish(std::move(msg3));

	// Verify messages are NOT received after unsubscribe_all
	bool unexpected = wait_for_condition([&count]() { return count > 0; }, std::chrono::milliseconds(100));
	EXPECT_FALSE(unexpected) << "Messages received after unsubscribe_all";
	EXPECT_EQ(count, 0);
}

TEST_F(PubSubTest, SubscriberAutoUnsubscribeOnDestruction) {
	bool received = false;

	{
		subscriber sub(bus_);
		auto result = sub.subscribe("test.topic", [&](const message&) {
			received = true;
			return common::ok();
		});
		ASSERT_TRUE(result.is_ok());
		EXPECT_EQ(sub.subscription_count(), 1);

		// Subscriber goes out of scope here - should auto-unsubscribe
	}

	// Publish message - should not be received
	message msg("test.topic");
	bus_->publish(std::move(msg));

	// Verify message is NOT received after subscriber destruction
	bool unexpected = wait_for_condition([&received]() { return received; }, std::chrono::milliseconds(100));
	EXPECT_FALSE(unexpected) << "Message received after subscriber destruction";
	EXPECT_FALSE(received);
}

TEST_F(PubSubTest, SubscriberWithFilter) {
	subscriber sub(bus_);

	int high_priority_count = 0;
	int total_count = 0;

	// Subscribe with filter - only high priority messages
	auto result = sub.subscribe(
		"test.topic",
		[&](const message&) {
			high_priority_count++;
			return common::ok();
		},
		[](const message& msg) {
			return msg.metadata().priority == message_priority::high;
		}
	);
	ASSERT_TRUE(result.is_ok());

	// Also subscribe without filter to count total
	auto result2 = sub.subscribe("test.topic", [&](const message&) {
		total_count++;
		return common::ok();
	});
	ASSERT_TRUE(result2.is_ok());

	// Publish messages with different priorities
	message msg1("test.topic");
	msg1.metadata().priority = message_priority::normal;
	bus_->publish(std::move(msg1));

	message msg2("test.topic");
	msg2.metadata().priority = message_priority::high;
	bus_->publish(std::move(msg2));

	message msg3("test.topic");
	msg3.metadata().priority = message_priority::low;
	bus_->publish(std::move(msg3));

	// Wait for all messages to be delivered
	ASSERT_TRUE(wait_for_condition([&total_count]() { return total_count >= 3; }, std::chrono::milliseconds(200)));

	EXPECT_EQ(high_priority_count, 1);  // Only high priority message
	EXPECT_EQ(total_count, 3);          // All messages
}

TEST_F(PubSubTest, SubscriberWithPriority) {
	subscriber sub(bus_);

	std::vector<int> execution_order;

	// Subscribe with different priorities
	auto result1 = sub.subscribe(
		"test.topic",
		[&](const message&) {
			execution_order.push_back(1);
			return common::ok();
		},
		nullptr,
		1  // Low priority
	);

	auto result2 = sub.subscribe(
		"test.topic",
		[&](const message&) {
			execution_order.push_back(2);
			return common::ok();
		},
		nullptr,
		10  // High priority
	);

	auto result3 = sub.subscribe(
		"test.topic",
		[&](const message&) {
			execution_order.push_back(3);
			return common::ok();
		},
		nullptr,
		5  // Medium priority
	);

	ASSERT_TRUE(result1.is_ok());
	ASSERT_TRUE(result2.is_ok());
	ASSERT_TRUE(result3.is_ok());

	// Publish message
	message msg("test.topic");
	bus_->publish(std::move(msg));

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&execution_order]() { return execution_order.size() >= 3; }, std::chrono::milliseconds(200)));

	// Verify execution order (highest priority first)
	ASSERT_EQ(execution_order.size(), 3);
	EXPECT_EQ(execution_order[0], 2);  // Priority 10
	EXPECT_EQ(execution_order[1], 3);  // Priority 5
	EXPECT_EQ(execution_order[2], 1);  // Priority 1
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(PubSubTest, PublisherSubscriberIntegration) {
	publisher pub(bus_, "test.integration");
	subscriber sub(bus_);

	bool received = false;
	std::string received_data;

	auto sub_result = sub.subscribe("test.integration", [&](const message& msg) {
		received = true;
		received_data = msg.metadata().topic;
		return common::ok();
	});
	ASSERT_TRUE(sub_result.is_ok());

	// Publish message
	message msg("test.integration");
	auto pub_result = pub.publish(std::move(msg));
	EXPECT_TRUE(pub_result.is_ok());

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&received]() { return received; }, std::chrono::milliseconds(200)));

	EXPECT_TRUE(received);
	EXPECT_EQ(received_data, "test.integration");
}

TEST_F(PubSubTest, MultiplePublishersAndSubscribers) {
	publisher pub1(bus_, "topic.1");
	publisher pub2(bus_, "topic.2");

	subscriber sub1(bus_);
	subscriber sub2(bus_);

	std::atomic<int> count1{0};
	std::atomic<int> count2{0};

	// sub1 subscribes to topic.1
	auto result1 = sub1.subscribe("topic.1", [&](const message&) {
		count1++;
		return common::ok();
	});
	ASSERT_TRUE(result1.is_ok());

	// sub2 subscribes to topic.2
	auto result2 = sub2.subscribe("topic.2", [&](const message&) {
		count2++;
		return common::ok();
	});
	ASSERT_TRUE(result2.is_ok());

	// Publish to both topics
	for (int i = 0; i < 5; i++) {
		message msg1("topic.1");
		pub1.publish(std::move(msg1));

		message msg2("topic.2");
		pub2.publish(std::move(msg2));
	}

	// Wait for delivery
	ASSERT_TRUE(wait_for_condition([&count1, &count2]() {
		return count1.load() >= 5 && count2.load() >= 5;
	}, std::chrono::milliseconds(500)));

	EXPECT_EQ(count1, 5);
	EXPECT_EQ(count2, 5);
}

TEST_F(PubSubTest, WildcardSubscription) {
	publisher pub(bus_);
	subscriber sub(bus_);

	int count = 0;

	// Subscribe with wildcard
	auto result = sub.subscribe("user.*", [&](const message&) {
		count++;
		return common::ok();
	});
	ASSERT_TRUE(result.is_ok());

	// Publish to matching topics
	message msg1("user.created");
	message msg2("user.updated");
	message msg3("user.deleted");
	message msg4("order.created");  // Should not match

	pub.publish("user.created", std::move(msg1));
	pub.publish("user.updated", std::move(msg2));
	pub.publish("user.deleted", std::move(msg3));
	pub.publish("order.created", std::move(msg4));

	// Wait for delivery - should receive 3 matching messages (user.*)
	ASSERT_TRUE(wait_for_condition([&count]() { return count >= 3; }, std::chrono::milliseconds(200)));

	EXPECT_EQ(count, 3);  // Only user.* topics
}
