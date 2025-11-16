#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_queue.h>
#include <kcenon/messaging/core/message.h>
#include <thread>
#include <chrono>
#include <set>

using namespace kcenon::messaging;

class MessageQueueTest : public ::testing::Test {
protected:
	void SetUp() override {
		queue_config config;
		config.max_size = 100;
		config.enable_priority = false;
		queue_ = std::make_unique<message_queue>(config);
	}

	void TearDown() override {
		if (queue_) {
			queue_->stop();
		}
	}

	std::unique_ptr<message_queue> queue_;
};

// Basic operations tests
TEST_F(MessageQueueTest, InitiallyEmpty) {
	EXPECT_TRUE(queue_->empty());
	EXPECT_EQ(queue_->size(), 0);
}

TEST_F(MessageQueueTest, EnqueueDequeue) {
	message msg("test.topic");

	auto enqueue_result = queue_->enqueue(std::move(msg));
	ASSERT_TRUE(enqueue_result.is_ok());

	EXPECT_FALSE(queue_->empty());
	EXPECT_EQ(queue_->size(), 1);

	auto dequeue_result = queue_->try_dequeue();
	ASSERT_TRUE(dequeue_result.is_ok());

	EXPECT_EQ(dequeue_result.unwrap().metadata().topic, "test.topic");
	EXPECT_TRUE(queue_->empty());
}

TEST_F(MessageQueueTest, MultipleMessages) {
	for (int i = 0; i < 10; ++i) {
		message msg("topic." + std::to_string(i));
		auto result = queue_->enqueue(std::move(msg));
		ASSERT_TRUE(result.is_ok());
	}

	EXPECT_EQ(queue_->size(), 10);

	for (int i = 0; i < 10; ++i) {
		auto result = queue_->try_dequeue();
		ASSERT_TRUE(result.is_ok());
		EXPECT_EQ(result.unwrap().metadata().topic, "topic." + std::to_string(i));
	}

	EXPECT_TRUE(queue_->empty());
}

TEST_F(MessageQueueTest, TryDequeueEmptyQueue) {
	auto result = queue_->try_dequeue();
	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageQueueTest, DequeueWithTimeout) {
	auto start = std::chrono::steady_clock::now();
	auto result = queue_->dequeue(std::chrono::milliseconds(100));
	auto duration = std::chrono::steady_clock::now() - start;

	EXPECT_TRUE(result.is_err());
	EXPECT_GE(duration, std::chrono::milliseconds(90));
}

TEST_F(MessageQueueTest, Clear) {
	for (int i = 0; i < 5; ++i) {
		message msg("topic." + std::to_string(i));
		queue_->enqueue(std::move(msg));
	}

	EXPECT_EQ(queue_->size(), 5);

	queue_->clear();

	EXPECT_TRUE(queue_->empty());
	EXPECT_EQ(queue_->size(), 0);
}

TEST_F(MessageQueueTest, Stop) {
	queue_->stop();

	message msg("test.topic");
	auto result = queue_->enqueue(std::move(msg));

	EXPECT_TRUE(result.is_err());
}

// Capacity tests
TEST_F(MessageQueueTest, MaxCapacity) {
	queue_config config;
	config.max_size = 5;
	config.drop_on_full = false;
	queue_ = std::make_unique<message_queue>(config);

	for (int i = 0; i < 5; ++i) {
		message msg("topic." + std::to_string(i));
		auto result = queue_->enqueue(std::move(msg));
		ASSERT_TRUE(result.is_ok());
	}

	// Should fail when full
	message overflow_msg("overflow.topic");
	auto result = queue_->enqueue(std::move(overflow_msg));
	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageQueueTest, DropOnFull) {
	queue_config config;
	config.max_size = 5;
	config.drop_on_full = true;
	queue_ = std::make_unique<message_queue>(config);

	for (int i = 0; i < 10; ++i) {
		message msg("topic." + std::to_string(i));
		auto result = queue_->enqueue(std::move(msg));
		EXPECT_TRUE(result.is_ok());
	}

	EXPECT_EQ(queue_->size(), 5);

	// First messages should be dropped
	auto result = queue_->try_dequeue();
	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(result.unwrap().metadata().topic, "topic.5");
}

// Threading tests
TEST_F(MessageQueueTest, ConcurrentEnqueue) {
	const int num_threads = 4;
	const int messages_per_thread = 25;

	std::vector<std::thread> threads;
	for (int t = 0; t < num_threads; ++t) {
		threads.emplace_back([this, t, messages_per_thread]() {
			for (int i = 0; i < messages_per_thread; ++i) {
				message msg("thread." + std::to_string(t) + ".msg." + std::to_string(i));
				queue_->enqueue(std::move(msg));
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(queue_->size(), num_threads * messages_per_thread);
}

TEST_F(MessageQueueTest, ProducerConsumer) {
	const int num_messages = 100;
	std::atomic<int> consumed{0};

	// Producer thread
	std::thread producer([this, num_messages]() {
		for (int i = 0; i < num_messages; ++i) {
			message msg("msg." + std::to_string(i));
			queue_->enqueue(std::move(msg));
			std::this_thread::sleep_for(std::chrono::microseconds(10));
		}
	});

	// Consumer thread
	std::thread consumer([this, &consumed, num_messages]() {
		while (consumed < num_messages) {
			auto result = queue_->dequeue(std::chrono::milliseconds(100));
			if (result.is_ok()) {
				consumed++;
			}
		}
	});

	producer.join();
	consumer.join();

	EXPECT_EQ(consumed, num_messages);
	EXPECT_TRUE(queue_->empty());
}

// Priority queue tests
class PriorityQueueTest : public ::testing::Test {
protected:
	void SetUp() override {
		queue_config config;
		config.max_size = 100;
		config.enable_priority = true;
		queue_ = std::make_unique<message_queue>(config);
	}

	void TearDown() override {
		if (queue_) {
			queue_->stop();
		}
	}

	std::unique_ptr<message_queue> queue_;
};

TEST_F(PriorityQueueTest, PriorityOrdering) {
	// Enqueue messages with different priorities
	message low("low");
	low.metadata().priority = message_priority::low;

	message normal("normal");
	normal.metadata().priority = message_priority::normal;

	message high("high");
	high.metadata().priority = message_priority::high;

	message critical("critical");
	critical.metadata().priority = message_priority::critical;

	// Enqueue in random order
	queue_->enqueue(std::move(normal));
	queue_->enqueue(std::move(critical));
	queue_->enqueue(std::move(low));
	queue_->enqueue(std::move(high));

	// Should dequeue in priority order
	auto result1 = queue_->try_dequeue();
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(result1.unwrap().metadata().topic, "critical");

	auto result2 = queue_->try_dequeue();
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(result2.unwrap().metadata().topic, "high");

	auto result3 = queue_->try_dequeue();
	ASSERT_TRUE(result3.is_ok());
	EXPECT_EQ(result3.unwrap().metadata().topic, "normal");

	auto result4 = queue_->try_dequeue();
	ASSERT_TRUE(result4.is_ok());
	EXPECT_EQ(result4.unwrap().metadata().topic, "low");
}

TEST_F(PriorityQueueTest, SamePriorityHandling) {
	// Messages with same priority are dequeued (order not guaranteed with std::priority_queue)
	std::set<std::string> expected_topics;
	for (int i = 0; i < 5; ++i) {
		message msg("msg." + std::to_string(i));
		msg.metadata().priority = message_priority::normal;
		queue_->enqueue(std::move(msg));
		expected_topics.insert("msg." + std::to_string(i));
	}

	std::set<std::string> received_topics;
	for (int i = 0; i < 5; ++i) {
		auto result = queue_->try_dequeue();
		ASSERT_TRUE(result.is_ok());
		received_topics.insert(result.unwrap().metadata().topic);
	}

	EXPECT_EQ(expected_topics, received_topics);
}
