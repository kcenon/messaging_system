#include <kcenon/messaging/task/task_queue.h>
#include <kcenon/messaging/task/task.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

namespace tsk = kcenon::messaging::task;
using tsk::task_queue;
using tsk::task_queue_config;
using tsk::task_builder;
using task_t = tsk::task;

// ============================================================================
// task_queue_config tests
// ============================================================================

TEST(TaskQueueConfigTest, DefaultValues) {
	task_queue_config config;

	EXPECT_EQ(config.max_size, 100000);
	EXPECT_FALSE(config.enable_persistence);
	EXPECT_TRUE(config.persistence_path.empty());
	EXPECT_TRUE(config.enable_delayed_queue);
	EXPECT_EQ(config.delayed_poll_interval.count(), 1000);
}

// ============================================================================
// task_queue tests - Lifecycle
// ============================================================================

TEST(TaskQueueTest, Construction) {
	task_queue_config config;
	config.max_size = 1000;
	task_queue queue(config);

	EXPECT_FALSE(queue.is_running());
	EXPECT_EQ(queue.total_size(), 0);
}

TEST(TaskQueueTest, StartStop) {
	task_queue queue;

	auto start_result = queue.start();
	EXPECT_TRUE(start_result.is_ok());
	EXPECT_TRUE(queue.is_running());

	queue.stop();
	EXPECT_FALSE(queue.is_running());
}

TEST(TaskQueueTest, DoubleStartReturnsError) {
	task_queue queue;

	auto result1 = queue.start();
	EXPECT_TRUE(result1.is_ok());

	// Second start should fail since already running
	auto result2 = queue.start();
	EXPECT_TRUE(result2.is_err());

	queue.stop();
}

TEST(TaskQueueTest, MoveConstruction) {
	task_queue_config config;
	config.max_size = 500;
	task_queue queue1(config);

	// Move before starting (thread cannot be moved while running)
	task_queue queue2(std::move(queue1));

	// Can start the moved queue
	auto result = queue2.start();
	EXPECT_TRUE(result.is_ok());
	EXPECT_TRUE(queue2.is_running());

	queue2.stop();
}

// ============================================================================
// task_queue tests - Enqueue Operations
// ============================================================================

TEST(TaskQueueTest, EnqueueSingleTask) {
	task_queue queue;
	queue.start();

	auto task_result = task_builder("test.task").build();
	ASSERT_TRUE(task_result.is_ok());
	auto t = task_result.unwrap();

	auto enqueue_result = queue.enqueue(std::move(t));
	EXPECT_TRUE(enqueue_result.is_ok());
	EXPECT_FALSE(enqueue_result.unwrap().empty());

	queue.stop();
}

TEST(TaskQueueTest, EnqueueToNamedQueue) {
	task_queue queue;
	queue.start();

	auto task_result = task_builder("test.task")
		.queue("high-priority")
		.build();
	ASSERT_TRUE(task_result.is_ok());

	auto enqueue_result = queue.enqueue(task_result.unwrap());
	EXPECT_TRUE(enqueue_result.is_ok());

	EXPECT_EQ(queue.queue_size("high-priority"), 1);
	EXPECT_EQ(queue.queue_size("default"), 0);

	queue.stop();
}

TEST(TaskQueueTest, EnqueueBulk) {
	task_queue queue;
	queue.start();

	std::vector<task_t> tasks;
	for (int i = 0; i < 5; ++i) {
		auto result = task_builder("bulk.task").build();
		ASSERT_TRUE(result.is_ok());
		tasks.push_back(result.unwrap());
	}

	auto enqueue_result = queue.enqueue_bulk(std::move(tasks));
	EXPECT_TRUE(enqueue_result.is_ok());
	EXPECT_EQ(enqueue_result.unwrap().size(), 5);

	queue.stop();
}

TEST(TaskQueueTest, EnqueueMultipleQueues) {
	task_queue queue;
	queue.start();

	for (int i = 0; i < 3; ++i) {
		auto t1 = task_builder("task.a").queue("queue-a").build().unwrap();
		queue.enqueue(std::move(t1));
	}
	for (int i = 0; i < 2; ++i) {
		auto t2 = task_builder("task.b").queue("queue-b").build().unwrap();
		queue.enqueue(std::move(t2));
	}

	EXPECT_EQ(queue.queue_size("queue-a"), 3);
	EXPECT_EQ(queue.queue_size("queue-b"), 2);
	EXPECT_EQ(queue.total_size(), 5);

	queue.stop();
}

// ============================================================================
// task_queue tests - Dequeue Operations
// ============================================================================

TEST(TaskQueueTest, DequeueFromSingleQueue) {
	task_queue queue;
	queue.start();

	auto t = task_builder("dequeue.test")
		.queue("test-queue")
		.build()
		.unwrap();
	std::string task_id = t.task_id();

	queue.enqueue(std::move(t));

	auto dequeue_result = queue.dequeue(
		{"test-queue"},
		std::chrono::milliseconds(100)
	);
	EXPECT_TRUE(dequeue_result.is_ok());
	EXPECT_EQ(dequeue_result.unwrap().task_id(), task_id);

	queue.stop();
}

TEST(TaskQueueTest, DequeueFromMultipleQueues) {
	task_queue queue;
	queue.start();

	// Only add task to secondary queue
	auto t = task_builder("multi.test")
		.queue("secondary")
		.build()
		.unwrap();

	queue.enqueue(std::move(t));

	// Should find task in secondary queue when primary is empty
	auto result = queue.dequeue(
		{"primary", "secondary"},
		std::chrono::milliseconds(100)
	);
	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(result.unwrap().task_name(), "multi.test");

	queue.stop();
}

TEST(TaskQueueTest, TryDequeueEmpty) {
	task_queue queue;
	queue.start();

	auto result = queue.try_dequeue({"default"});
	EXPECT_TRUE(result.is_err());

	queue.stop();
}

TEST(TaskQueueTest, DequeueWithShortTimeout) {
	task_queue queue;
	queue.start();

	auto t = task_builder("try.test").build().unwrap();
	queue.enqueue(std::move(t));

	// Use short timeout instead of try_dequeue (which uses 0 timeout)
	auto result = queue.dequeue({"default"}, std::chrono::milliseconds(100));
	EXPECT_TRUE(result.is_ok());

	queue.stop();
}

TEST(TaskQueueTest, DequeueTimeout) {
	task_queue queue;
	queue.start();

	auto start = std::chrono::steady_clock::now();
	auto result = queue.dequeue(
		{"default"},
		std::chrono::milliseconds(100)
	);
	auto elapsed = std::chrono::steady_clock::now() - start;

	EXPECT_TRUE(result.is_err());
	EXPECT_GE(elapsed, std::chrono::milliseconds(90));

	queue.stop();
}

TEST(TaskQueueTest, DequeuePreservesOrder) {
	task_queue queue;
	queue.start();

	std::vector<std::string> task_ids;
	for (int i = 0; i < 3; ++i) {
		auto t = task_builder("order.test").build().unwrap();
		task_ids.push_back(t.task_id());
		queue.enqueue(std::move(t));
	}

	for (int i = 0; i < 3; ++i) {
		auto result = queue.dequeue({"default"}, std::chrono::milliseconds(100));
		ASSERT_TRUE(result.is_ok());
		EXPECT_EQ(result.unwrap().task_id(), task_ids[i]);
	}

	queue.stop();
}

// ============================================================================
// task_queue tests - Delayed Execution
// ============================================================================

TEST(TaskQueueTest, DelayedTaskNotImmediatelyAvailable) {
	task_queue_config config;
	config.enable_delayed_queue = true;
	config.delayed_poll_interval = std::chrono::milliseconds(50);
	task_queue queue(config);
	queue.start();

	auto eta = std::chrono::system_clock::now() + std::chrono::milliseconds(500);
	auto t = task_builder("delayed.task")
		.eta(eta)
		.build()
		.unwrap();

	queue.enqueue(std::move(t));

	// Should not be available immediately (use short timeout to check)
	auto result = queue.dequeue({"default"}, std::chrono::milliseconds(50));
	EXPECT_TRUE(result.is_err());

	// Wait for delayed task to become available
	std::this_thread::sleep_for(std::chrono::milliseconds(600));

	// Now it should be available
	result = queue.dequeue({"default"}, std::chrono::milliseconds(100));
	EXPECT_TRUE(result.is_ok());

	queue.stop();
}

TEST(TaskQueueTest, CountdownDelay) {
	task_queue_config config;
	config.enable_delayed_queue = true;
	config.delayed_poll_interval = std::chrono::milliseconds(50);
	task_queue queue(config);
	queue.start();

	auto t = task_builder("countdown.task")
		.countdown(std::chrono::milliseconds(300))
		.build()
		.unwrap();

	queue.enqueue(std::move(t));

	// Not available immediately
	auto result1 = queue.dequeue({"default"}, std::chrono::milliseconds(50));
	EXPECT_TRUE(result1.is_err());

	// Wait for countdown
	std::this_thread::sleep_for(std::chrono::milliseconds(400));

	auto result2 = queue.dequeue({"default"}, std::chrono::milliseconds(100));
	EXPECT_TRUE(result2.is_ok());

	queue.stop();
}

TEST(TaskQueueTest, DelayedQueueSize) {
	task_queue_config config;
	config.enable_delayed_queue = true;
	task_queue queue(config);
	queue.start();

	auto eta = std::chrono::system_clock::now() + std::chrono::seconds(60);
	for (int i = 0; i < 3; ++i) {
		auto t = task_builder("delayed.task").eta(eta).build().unwrap();
		queue.enqueue(std::move(t));
	}

	EXPECT_EQ(queue.delayed_size(), 3);
	EXPECT_EQ(queue.queue_size("default"), 0);

	queue.stop();
}

// ============================================================================
// task_queue tests - Cancellation
// ============================================================================

TEST(TaskQueueTest, CancelTask) {
	task_queue queue;
	queue.start();

	auto t = task_builder("cancel.test").build().unwrap();
	std::string task_id = t.task_id();

	queue.enqueue(std::move(t));

	auto cancel_result = queue.cancel(task_id);
	EXPECT_TRUE(cancel_result.is_ok());

	queue.stop();
}

TEST(TaskQueueTest, CancelNonexistentTask) {
	task_queue queue;
	queue.start();

	// cancel() always succeeds - it adds to cancelled set regardless
	auto result = queue.cancel("nonexistent-task-id");
	EXPECT_TRUE(result.is_ok());

	queue.stop();
}

TEST(TaskQueueTest, CancelByTag) {
	task_queue queue;
	queue.start();

	for (int i = 0; i < 3; ++i) {
		auto t = task_builder("tagged.task")
			.tag("batch-1")
			.build()
			.unwrap();
		queue.enqueue(std::move(t));
	}
	for (int i = 0; i < 2; ++i) {
		auto t = task_builder("other.task")
			.tag("batch-2")
			.build()
			.unwrap();
		queue.enqueue(std::move(t));
	}

	auto result = queue.cancel_by_tag("batch-1");
	EXPECT_TRUE(result.is_ok());

	queue.stop();
}

// ============================================================================
// task_queue tests - Query Operations
// ============================================================================

TEST(TaskQueueTest, GetTask) {
	task_queue queue;
	queue.start();

	auto t = task_builder("get.test").build().unwrap();
	std::string task_id = t.task_id();

	queue.enqueue(std::move(t));

	auto result = queue.get_task(task_id);
	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(result.unwrap().task_id(), task_id);

	queue.stop();
}

TEST(TaskQueueTest, GetNonexistentTask) {
	task_queue queue;
	queue.start();

	auto result = queue.get_task("nonexistent");
	EXPECT_TRUE(result.is_err());

	queue.stop();
}

TEST(TaskQueueTest, ListQueues) {
	task_queue queue;
	queue.start();

	queue.enqueue(task_builder("task").queue("queue-a").build().unwrap());
	queue.enqueue(task_builder("task").queue("queue-b").build().unwrap());
	queue.enqueue(task_builder("task").queue("queue-c").build().unwrap());

	auto queues = queue.list_queues();
	EXPECT_EQ(queues.size(), 3);

	std::set<std::string> queue_set(queues.begin(), queues.end());
	EXPECT_TRUE(queue_set.count("queue-a") > 0);
	EXPECT_TRUE(queue_set.count("queue-b") > 0);
	EXPECT_TRUE(queue_set.count("queue-c") > 0);

	queue.stop();
}

TEST(TaskQueueTest, HasQueue) {
	task_queue queue;
	queue.start();

	EXPECT_FALSE(queue.has_queue("new-queue"));

	queue.enqueue(task_builder("task").queue("new-queue").build().unwrap());

	EXPECT_TRUE(queue.has_queue("new-queue"));

	queue.stop();
}

TEST(TaskQueueTest, QueueSizeEmpty) {
	task_queue queue;
	queue.start();

	EXPECT_EQ(queue.queue_size("nonexistent"), 0);

	queue.stop();
}

// ============================================================================
// task_queue tests - Thread Safety
// ============================================================================

TEST(TaskQueueTest, ConcurrentEnqueue) {
	task_queue queue;
	queue.start();

	constexpr int NUM_THREADS = 4;
	constexpr int TASKS_PER_THREAD = 100;
	std::atomic<int> enqueued{0};

	std::vector<std::thread> threads;
	for (int t = 0; t < NUM_THREADS; ++t) {
		threads.emplace_back([&queue, &enqueued, t]() {
			for (int i = 0; i < TASKS_PER_THREAD; ++i) {
				auto task = task_builder("concurrent.task").build().unwrap();
				auto result = queue.enqueue(std::move(task));
				if (result.is_ok()) {
					enqueued.fetch_add(1);
				}
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(enqueued.load(), NUM_THREADS * TASKS_PER_THREAD);
	EXPECT_EQ(queue.total_size(), NUM_THREADS * TASKS_PER_THREAD);

	queue.stop();
}

TEST(TaskQueueTest, ConcurrentEnqueueDequeue) {
	task_queue queue;
	queue.start();

	constexpr int NUM_TASKS = 500;
	std::atomic<int> enqueued{0};
	std::atomic<int> dequeued{0};
	std::atomic<bool> producer_done{false};

	// Producer thread
	std::thread producer([&]() {
		for (int i = 0; i < NUM_TASKS; ++i) {
			auto task = task_builder("concurrent.task").build().unwrap();
			queue.enqueue(std::move(task));
			enqueued.fetch_add(1);
		}
		producer_done.store(true);
	});

	// Consumer threads
	std::vector<std::thread> consumers;
	for (int c = 0; c < 2; ++c) {
		consumers.emplace_back([&]() {
			while (!producer_done.load() || queue.total_size() > 0) {
				auto result = queue.dequeue(
					{"default"},
					std::chrono::milliseconds(10)
				);
				if (result.is_ok()) {
					dequeued.fetch_add(1);
				}
			}
		});
	}

	producer.join();
	for (auto& consumer : consumers) {
		consumer.join();
	}

	EXPECT_EQ(enqueued.load(), NUM_TASKS);
	EXPECT_EQ(dequeued.load(), NUM_TASKS);

	queue.stop();
}

// ============================================================================
// task_queue tests - Priority Ordering
// ============================================================================

TEST(TaskQueueTest, PriorityOrdering) {
	task_queue queue;
	queue.start();

	// Enqueue in reverse priority order
	auto low = task_builder("low.priority")
		.priority(kcenon::messaging::message_priority::low)
		.build()
		.unwrap();
	auto normal = task_builder("normal.priority")
		.priority(kcenon::messaging::message_priority::normal)
		.build()
		.unwrap();
	auto high = task_builder("high.priority")
		.priority(kcenon::messaging::message_priority::high)
		.build()
		.unwrap();

	queue.enqueue(std::move(low));
	queue.enqueue(std::move(normal));
	queue.enqueue(std::move(high));

	// High priority should come first
	auto result1 = queue.dequeue({"default"}, std::chrono::milliseconds(100));
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(result1.unwrap().task_name(), "high.priority");

	auto result2 = queue.dequeue({"default"}, std::chrono::milliseconds(100));
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(result2.unwrap().task_name(), "normal.priority");

	auto result3 = queue.dequeue({"default"}, std::chrono::milliseconds(100));
	ASSERT_TRUE(result3.is_ok());
	EXPECT_EQ(result3.unwrap().task_name(), "low.priority");

	queue.stop();
}
