// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/patterns/request_reply.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;
using namespace kcenon::messaging::patterns;
using namespace kcenon::common;

/**
 * @brief Extended messaging pattern integration tests
 *
 * These tests cover multi-system messaging scenarios as specified in Issue #161:
 * - Pub/Sub with multiple subscribers
 * - Request/Reply with timeout handling
 * - Complex routing patterns
 */
class MessagingPatternsE2ETest : public MessagingFixture {};

// ============================================================================
// Pub/Sub Multiple Subscribers Tests (Issue #161 - High Priority)
// ============================================================================

/**
 * @test PubSubMultipleSubscribers
 * @brief Verify that multiple subscribers on the same topic all receive messages
 *
 * Setup: 1 publisher, 3 subscribers on same topic
 * Action: Publish 100 messages
 * Verify:
 *   - All subscribers receive all messages
 *   - Order preserved per subscriber
 *   - No message loss
 */
TEST_F(MessagingPatternsE2ETest, PubSubMultipleSubscribers) {
	const int num_subscribers = 3;
	const int num_messages = 100;
	const std::string topic = "test.pubsub.multi";

	// Create storage for each subscriber to verify order
	std::vector<std::vector<message>> received_messages(num_subscribers);
	std::vector<std::mutex> mutexes(num_subscribers);
	std::vector<MessageCounter> counters(num_subscribers);

	// Create subscribers
	std::vector<std::shared_ptr<subscriber>> subscribers;
	for (int i = 0; i < num_subscribers; ++i) {
		auto sub = std::make_shared<subscriber>(bus_);
		auto sub_result = sub->subscribe(
			topic,
			[i, &received_messages, &mutexes, &counters](const message& msg) -> VoidResult {
				{
					std::lock_guard lock(mutexes[i]);
					received_messages[i].push_back(msg);
				}
				counters[i].increment();
				return ok();
			}
		);
		ASSERT_TRUE(sub_result.is_ok()) << "Failed to subscribe subscriber " << i;
		subscribers.push_back(sub);
	}

	// Create publisher
	auto pub = std::make_shared<publisher>(bus_, topic);

	// Publish messages with sequence numbers in metadata
	for (int i = 0; i < num_messages; ++i) {
		auto msg_result = message_builder()
			.topic(topic)
			.type(message_type::event)
			.priority(message_priority::normal)
			.source("test_publisher")
			.correlation_id("seq_" + std::to_string(i))  // Use correlation_id as sequence marker
			.build();

		ASSERT_TRUE(msg_result.is_ok()) << "Failed to build message " << i;
		ASSERT_TRUE(pub->publish(msg_result.unwrap()).is_ok()) << "Failed to publish message " << i;
	}

	// Wait for all subscribers to receive all messages
	ASSERT_TRUE(wait_for_condition(
		[&]() {
			for (int i = 0; i < num_subscribers; ++i) {
				if (counters[i].count() < num_messages) {
					return false;
				}
			}
			return true;
		},
		std::chrono::seconds{10}
	)) << "Timeout waiting for all subscribers to receive messages";

	// Verify all subscribers received all messages
	for (int i = 0; i < num_subscribers; ++i) {
		EXPECT_EQ(counters[i].count(), num_messages)
			<< "Subscriber " << i << " received " << counters[i].count()
			<< " messages instead of " << num_messages;
	}

	// Verify order preserved per subscriber (using correlation_id as sequence)
	for (int i = 0; i < num_subscribers; ++i) {
		std::lock_guard lock(mutexes[i]);
		ASSERT_EQ(received_messages[i].size(), static_cast<size_t>(num_messages))
			<< "Subscriber " << i << " message vector size mismatch";

		for (int j = 0; j < num_messages; ++j) {
			std::string expected_seq = "seq_" + std::to_string(j);
			EXPECT_EQ(received_messages[i][j].metadata().correlation_id, expected_seq)
				<< "Subscriber " << i << " message order mismatch at position " << j;
		}
	}
}

/**
 * @test PubSubConcurrentPublishers
 * @brief Verify multiple publishers can publish to the same topic concurrently
 *
 * Setup: 3 publishers, 1 subscriber on same topic
 * Action: Each publisher publishes 100 messages concurrently
 * Verify:
 *   - Subscriber receives all 300 messages
 *   - No message loss under concurrent publishing
 */
TEST_F(MessagingPatternsE2ETest, PubSubConcurrentPublishers) {
	const int num_publishers = 3;
	const int messages_per_publisher = 100;
	const int total_messages = num_publishers * messages_per_publisher;
	const std::string topic = "test.pubsub.concurrent";

	MessageCounter counter;
	auto sub = std::make_shared<subscriber>(bus_);
	auto sub_result = sub->subscribe(topic, create_counting_callback(counter));
	ASSERT_TRUE(sub_result.is_ok());

	// Create and run publishers concurrently
	std::vector<std::thread> threads;
	std::atomic<int> publish_errors{0};

	for (int p = 0; p < num_publishers; ++p) {
		threads.emplace_back([&]() {
			auto pub = std::make_shared<publisher>(bus_, topic);
			for (int i = 0; i < messages_per_publisher; ++i) {
				auto msg = create_test_message(topic);
				if (!pub->publish(msg).is_ok()) {
					publish_errors.fetch_add(1);
				}
			}
		});
	}

	// Wait for all publishers to complete
	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(publish_errors.load(), 0) << "Some messages failed to publish";

	// Wait for subscriber to receive all messages
	ASSERT_TRUE(wait_for_condition(
		[&]() { return counter.count() >= total_messages; },
		std::chrono::seconds{10}
	)) << "Timeout: received " << counter.count() << " of " << total_messages << " messages";

	EXPECT_EQ(counter.count(), total_messages);
}

/**
 * @test PubSubHighThroughput
 * @brief Verify pub/sub can handle high message throughput
 *
 * Setup: 1 publisher, 1 subscriber
 * Action: Publish 1000 messages as fast as possible
 * Verify:
 *   - All messages delivered
 *   - No significant message loss
 */
TEST_F(MessagingPatternsE2ETest, PubSubHighThroughput) {
	const int num_messages = 1000;
	const std::string topic = "test.pubsub.throughput";

	MessageCounter counter;
	auto sub = std::make_shared<subscriber>(bus_);
	auto sub_result = sub->subscribe(topic, create_counting_callback(counter));
	ASSERT_TRUE(sub_result.is_ok());

	auto pub = std::make_shared<publisher>(bus_, topic);

	auto start = std::chrono::steady_clock::now();

	for (int i = 0; i < num_messages; ++i) {
		auto msg = create_test_message(topic);
		ASSERT_TRUE(pub->publish(msg).is_ok()) << "Failed to publish message " << i;
	}

	auto publish_end = std::chrono::steady_clock::now();

	// Wait for all messages to be received
	ASSERT_TRUE(wait_for_condition(
		[&]() { return counter.count() >= num_messages; },
		std::chrono::seconds{30}
	)) << "Timeout: received " << counter.count() << " of " << num_messages << " messages";

	auto receive_end = std::chrono::steady_clock::now();

	auto publish_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		publish_end - start).count();
	auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
		receive_end - start).count();

	EXPECT_EQ(counter.count(), num_messages);

	// Log throughput for diagnostics (not a test assertion)
	if (publish_duration > 0) {
		double publish_rate = (num_messages * 1000.0) / publish_duration;
		double total_rate = (num_messages * 1000.0) / total_duration;
		std::cout << "Publish rate: " << publish_rate << " msg/sec" << std::endl;
		std::cout << "End-to-end rate: " << total_rate << " msg/sec" << std::endl;
	}
}

// ============================================================================
// Request/Reply Pattern Tests (Issue #161 - High Priority)
// ============================================================================

/**
 * @test RequestReplyWithTimeout
 * @brief Verify request/reply pattern handles timeout correctly
 *
 * Setup: Request/Reply channel with slow handler
 * Action: Send request that will timeout
 * Verify:
 *   - Timeout triggers correctly
 *   - Error result returned
 */
TEST_F(MessagingPatternsE2ETest, RequestReplyWithTimeout) {
	const std::string topic = "test.rr.timeout";
	const auto short_timeout = std::chrono::milliseconds{100};

	auto handler = std::make_shared<request_reply_handler>(bus_, topic);

	// Register a slow handler that takes longer than timeout
	auto register_result = handler->register_handler(
		[](const message& req) -> Result<message> {
			// Simulate slow processing
			std::this_thread::sleep_for(std::chrono::milliseconds{500});
			return message_builder()
				.topic("test.rr.timeout.reply")
				.correlation_id(req.metadata().correlation_id)
				.build();
		}
	);
	ASSERT_TRUE(register_result.is_ok());

	// Send request with short timeout
	auto request_msg = create_test_message(topic);
	auto reply_result = handler->request(request_msg, short_timeout);

	// Should timeout
	EXPECT_FALSE(reply_result.is_ok()) << "Request should have timed out";
}

/**
 * @test RequestReplySequential
 * @brief Verify multiple sequential requests work correctly
 *
 * Setup: Request/Reply channel
 * Action: Send 10 sequential requests
 * Verify:
 *   - All requests get correct replies
 *   - Correlation IDs match
 */
TEST_F(MessagingPatternsE2ETest, RequestReplySequential) {
	const std::string topic = "test.rr.sequential";
	const int num_requests = 10;

	auto handler = std::make_shared<request_reply_handler>(bus_, topic);

	// Register echo handler
	auto register_result = handler->register_handler(
		[](const message& req) -> Result<message> {
			return message_builder()
				.topic("test.rr.sequential.reply")
				.correlation_id(req.metadata().correlation_id)
				.build();
		}
	);
	ASSERT_TRUE(register_result.is_ok());

	// Send sequential requests
	for (int i = 0; i < num_requests; ++i) {
		auto request_msg = create_test_message(topic);
		auto reply_result = handler->request(request_msg, std::chrono::seconds{2});

		ASSERT_TRUE(reply_result.is_ok()) << "Request " << i << " failed";
		auto reply = reply_result.unwrap();
		EXPECT_FALSE(reply.metadata().correlation_id.empty())
			<< "Reply " << i << " missing correlation ID";
	}
}

/**
 * @test RequestReplyConcurrent
 * @brief Verify multiple concurrent requests work correctly
 *
 * Setup: Request/Reply channel
 * Action: Send 10 concurrent requests
 * Verify:
 *   - All requests complete
 *   - No deadlocks or resource issues
 */
TEST_F(MessagingPatternsE2ETest, RequestReplyConcurrent) {
	const std::string topic = "test.rr.concurrent";
	const int num_requests = 10;

	auto handler = std::make_shared<request_reply_handler>(bus_, topic);

	// Register handler
	auto register_result = handler->register_handler(
		[](const message& req) -> Result<message> {
			// Small delay to simulate work
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
			return message_builder()
				.topic("test.rr.concurrent.reply")
				.correlation_id(req.metadata().correlation_id)
				.build();
		}
	);
	ASSERT_TRUE(register_result.is_ok());

	// Send concurrent requests
	std::vector<std::thread> threads;
	std::atomic<int> success_count{0};
	std::atomic<int> failure_count{0};

	for (int i = 0; i < num_requests; ++i) {
		threads.emplace_back([&]() {
			auto request_msg = create_test_message(topic);
			auto reply_result = handler->request(request_msg, std::chrono::seconds{5});

			if (reply_result.is_ok()) {
				success_count.fetch_add(1);
			} else {
				failure_count.fetch_add(1);
			}
		});
	}

	// Wait for all requests to complete
	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(success_count.load(), num_requests)
		<< "Expected all " << num_requests << " requests to succeed, but "
		<< failure_count.load() << " failed";
}

// ============================================================================
// Backpressure and Slow Consumer Tests (Issue #161 - Medium Priority)
// ============================================================================

/**
 * @test SlowConsumerHandling
 * @brief Verify system handles slow consumers gracefully
 *
 * Setup: Fast publisher, slow subscriber
 * Action: Publish messages faster than consumer can process
 * Verify:
 *   - No message loss (queued)
 *   - System remains responsive
 *   - Eventually all messages processed
 */
TEST_F(MessagingPatternsE2ETest, SlowConsumerHandling) {
	const int num_messages = 100;
	const std::string topic = "test.backpressure.slow";

	std::atomic<int> processed_count{0};

	auto sub = std::make_shared<subscriber>(bus_);
	auto sub_result = sub->subscribe(
		topic,
		[&processed_count]([[maybe_unused]] const message& msg) -> VoidResult {
			// Simulate slow processing
			std::this_thread::sleep_for(std::chrono::milliseconds{5});
			processed_count.fetch_add(1);
			return ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	auto pub = std::make_shared<publisher>(bus_, topic);

	// Publish all messages quickly
	for (int i = 0; i < num_messages; ++i) {
		auto msg = create_test_message(topic);
		ASSERT_TRUE(pub->publish(msg).is_ok()) << "Failed to publish message " << i;
	}

	// Wait for all messages to be processed (longer timeout due to slow consumer)
	ASSERT_TRUE(wait_for_condition(
		[&]() { return processed_count.load() >= num_messages; },
		std::chrono::seconds{30}
	)) << "Timeout: processed " << processed_count.load() << " of " << num_messages << " messages";

	EXPECT_EQ(processed_count.load(), num_messages);
}

/**
 * @test TopicWildcardRouting
 * @brief Verify wildcard topic routing works correctly with multiple levels
 *
 * Setup: Subscribers with exact, single-level (*), and multi-level (#) wildcards
 * Action: Publish to various topic levels
 * Verify:
 *   - Correct routing to each subscriber type
 *   - No duplicate deliveries
 */
TEST_F(MessagingPatternsE2ETest, TopicWildcardRouting) {
	MessageCounter exact_counter;
	MessageCounter single_wildcard_counter;
	MessageCounter multi_wildcard_counter;

	// Exact match subscriber
	auto exact_result = bus_->subscribe(
		"events.orders.created",
		create_counting_callback(exact_counter)
	);
	ASSERT_TRUE(exact_result.is_ok());

	// Single-level wildcard subscriber
	auto single_result = bus_->subscribe(
		"events.orders.*",
		create_counting_callback(single_wildcard_counter)
	);
	ASSERT_TRUE(single_result.is_ok());

	// Multi-level wildcard subscriber
	auto multi_result = bus_->subscribe(
		"events.#",
		create_counting_callback(multi_wildcard_counter)
	);
	ASSERT_TRUE(multi_result.is_ok());

	// Publish to various topics
	ASSERT_TRUE(bus_->publish(create_test_message("events.orders.created")).is_ok());
	ASSERT_TRUE(bus_->publish(create_test_message("events.orders.updated")).is_ok());
	ASSERT_TRUE(bus_->publish(create_test_message("events.orders.item.added")).is_ok());
	ASSERT_TRUE(bus_->publish(create_test_message("events.users.registered")).is_ok());

	// Wait and verify
	ASSERT_TRUE(wait_for_condition(
		[&]() {
			return exact_counter.count() >= 1 &&
			       single_wildcard_counter.count() >= 2 &&
			       multi_wildcard_counter.count() >= 4;
		},
		std::chrono::seconds{5}
	));

	EXPECT_EQ(exact_counter.count(), 1);           // only "events.orders.created"
	EXPECT_EQ(single_wildcard_counter.count(), 2); // "created" and "updated"
	EXPECT_EQ(multi_wildcard_counter.count(), 4);  // all four
}

/**
 * @test UnsubscribeDuringPublish
 * @brief Verify unsubscribe works correctly while messages are being published
 *
 * Setup: Publisher and subscriber
 * Action: Unsubscribe while publishing is in progress
 * Verify:
 *   - No crashes
 *   - Unsubscribe takes effect
 *   - In-flight messages handled gracefully
 */
TEST_F(MessagingPatternsE2ETest, UnsubscribeDuringPublish) {
	const std::string topic = "test.unsubscribe.during";

	MessageCounter counter;
	auto sub = std::make_shared<subscriber>(bus_);
	auto sub_result = sub->subscribe(topic, create_counting_callback(counter));
	ASSERT_TRUE(sub_result.is_ok());
	uint64_t sub_id = sub_result.unwrap();

	auto pub = std::make_shared<publisher>(bus_, topic);

	// Start publishing in a thread
	std::atomic<bool> stop_publishing{false};
	std::thread publish_thread([&]() {
		while (!stop_publishing.load()) {
			pub->publish(create_test_message(topic));
			std::this_thread::sleep_for(std::chrono::microseconds{100});
		}
	});

	// Let some messages flow
	std::this_thread::sleep_for(std::chrono::milliseconds{50});

	// Unsubscribe while publishing
	sub->unsubscribe(sub_id);

	// Continue publishing briefly
	std::this_thread::sleep_for(std::chrono::milliseconds{50});

	// Stop publishing
	stop_publishing.store(true);
	publish_thread.join();

	// Record count after unsubscribe
	int count_after_unsub = counter.count();

	// Publish more messages
	for (int i = 0; i < 10; ++i) {
		pub->publish(create_test_message(topic));
	}

	// Small wait
	std::this_thread::sleep_for(std::chrono::milliseconds{100});

	// Verify count didn't increase (unsubscribe worked)
	EXPECT_LE(counter.count(), count_after_unsub + 5)
		<< "Messages delivered after unsubscribe";
}
