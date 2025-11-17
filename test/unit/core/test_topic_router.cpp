#include <gtest/gtest.h>

#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/core/topic_router.h>

#include <atomic>
#include <set>
#include <thread>

using namespace kcenon::messaging;
using namespace kcenon;

class TopicRouterTest : public ::testing::Test {
protected:
	void SetUp() override { router_ = std::make_unique<topic_router>(); }

	void TearDown() override {
		if (router_) {
			router_->clear();
		}
	}

	std::unique_ptr<topic_router> router_;
};

// Basic subscription tests
TEST_F(TopicRouterTest, SubscribeSuccess) {
	int call_count = 0;
	auto result = router_->subscribe("test.topic",
									 [&call_count](const message& msg) {
										 call_count++;
										 return common::ok();
									 });

	ASSERT_TRUE(result.is_ok());
	EXPECT_GT(result.unwrap(), 0);
}

TEST_F(TopicRouterTest, SubscribeInvalidCallback) {
	auto result = router_->subscribe("test.topic", nullptr);

	EXPECT_TRUE(result.is_err());
}

TEST_F(TopicRouterTest, SubscribeEmptyPattern) {
	auto result =
		router_->subscribe("", [](const message& msg) { return common::ok(); });

	EXPECT_TRUE(result.is_err());
}

TEST_F(TopicRouterTest, SubscribeInvalidPriority) {
	auto result = router_->subscribe(
		"test.topic", [](const message& msg) { return common::ok(); }, nullptr,
		-1);

	EXPECT_TRUE(result.is_err());

	auto result2 = router_->subscribe(
		"test.topic", [](const message& msg) { return common::ok(); }, nullptr,
		11);

	EXPECT_TRUE(result2.is_err());
}

TEST_F(TopicRouterTest, UnsubscribeSuccess) {
	auto sub_result = router_->subscribe(
		"test.topic", [](const message& msg) { return common::ok(); });

	ASSERT_TRUE(sub_result.is_ok());
	uint64_t sub_id = sub_result.unwrap();

	auto unsub_result = router_->unsubscribe(sub_id);
	EXPECT_TRUE(unsub_result.is_ok());
}

TEST_F(TopicRouterTest, UnsubscribeNotFound) {
	auto result = router_->unsubscribe(999);

	EXPECT_TRUE(result.is_err());
}

// Pattern matching tests
TEST_F(TopicRouterTest, ExactMatch) {
	int call_count = 0;
	router_->subscribe("user.created",
					   [&call_count](const message& msg) {
						   call_count++;
						   return common::ok();
					   });

	message msg("user.created");
	auto result = router_->route(msg);

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(call_count, 1);
}

TEST_F(TopicRouterTest, ExactMatchNoSubscribers) {
	message msg("user.deleted");
	auto result = router_->route(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(TopicRouterTest, SingleLevelWildcard) {
	int call_count = 0;
	router_->subscribe("user.*", [&call_count](const message& msg) {
		call_count++;
		return common::ok();
	});

	message msg1("user.created");
	auto result1 = router_->route(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	message msg2("user.updated");
	auto result2 = router_->route(msg2);
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(call_count, 2);

	// Should not match multi-level
	message msg3("user.profile.updated");
	auto result3 = router_->route(msg3);
	EXPECT_TRUE(result3.is_err());
	EXPECT_EQ(call_count, 2);
}

TEST_F(TopicRouterTest, MultiLevelWildcard) {
	int call_count = 0;
	router_->subscribe("user.#", [&call_count](const message& msg) {
		call_count++;
		return common::ok();
	});

	message msg1("user.created");
	auto result1 = router_->route(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	message msg2("user.profile.updated");
	auto result2 = router_->route(msg2);
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(call_count, 2);

	message msg3("user.profile.settings.changed");
	auto result3 = router_->route(msg3);
	ASSERT_TRUE(result3.is_ok());
	EXPECT_EQ(call_count, 3);
}

TEST_F(TopicRouterTest, WildcardAtEnd) {
	int call_count = 0;
	router_->subscribe("*.created", [&call_count](const message& msg) {
		call_count++;
		return common::ok();
	});

	message msg1("user.created");
	auto result1 = router_->route(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	message msg2("order.created");
	auto result2 = router_->route(msg2);
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(call_count, 2);

	message msg3("user.updated");
	auto result3 = router_->route(msg3);
	EXPECT_TRUE(result3.is_err());
	EXPECT_EQ(call_count, 2);
}

TEST_F(TopicRouterTest, ComplexPatterns) {
	std::set<std::string> received_topics;

	router_->subscribe("user.*.created",
					   [&received_topics](const message& msg) {
						   received_topics.insert(msg.metadata().topic);
						   return common::ok();
					   });

	message msg1("user.profile.created");
	router_->route(msg1);

	message msg2("user.account.created");
	router_->route(msg2);

	EXPECT_EQ(received_topics.size(), 2);
	EXPECT_TRUE(received_topics.count("user.profile.created") > 0);
	EXPECT_TRUE(received_topics.count("user.account.created") > 0);
}

// Priority tests
TEST_F(TopicRouterTest, PriorityOrdering) {
	std::vector<int> execution_order;

	router_->subscribe(
		"test.topic",
		[&execution_order](const message& msg) {
			execution_order.push_back(1);
			return common::ok();
		},
		nullptr, 1);

	router_->subscribe(
		"test.topic",
		[&execution_order](const message& msg) {
			execution_order.push_back(5);
			return common::ok();
		},
		nullptr, 5);

	router_->subscribe(
		"test.topic",
		[&execution_order](const message& msg) {
			execution_order.push_back(3);
			return common::ok();
		},
		nullptr, 3);

	message msg("test.topic");
	auto result = router_->route(msg);

	ASSERT_TRUE(result.is_ok());
	ASSERT_EQ(execution_order.size(), 3);
	EXPECT_EQ(execution_order[0], 5);  // Highest priority first
	EXPECT_EQ(execution_order[1], 3);
	EXPECT_EQ(execution_order[2], 1);  // Lowest priority last
}

// Filter tests
TEST_F(TopicRouterTest, FilterAccept) {
	int call_count = 0;

	auto filter = [](const message& msg) {
		return msg.metadata().type == message_type::event;
	};

	router_->subscribe(
		"test.topic",
		[&call_count](const message& msg) {
			call_count++;
			return common::ok();
		},
		filter);

	message event_msg("test.topic", message_type::event);
	auto result1 = router_->route(event_msg);
	EXPECT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	message command_msg("test.topic", message_type::command);
	auto result2 = router_->route(command_msg);
	EXPECT_TRUE(result2.is_err());	// No matching subscribers after filter
	EXPECT_EQ(call_count, 1);
}

TEST_F(TopicRouterTest, FilterReject) {
	int call_count = 0;

	auto filter = [](const message& msg) {
		return msg.metadata().priority == message_priority::high;
	};

	router_->subscribe(
		"test.topic",
		[&call_count](const message& msg) {
			call_count++;
			return common::ok();
		},
		filter);

	message normal_msg("test.topic");
	normal_msg.metadata().priority = message_priority::normal;
	auto result = router_->route(normal_msg);

	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(call_count, 0);
}

// Multiple subscribers tests
TEST_F(TopicRouterTest, MultipleSubscribers) {
	int call_count1 = 0;
	int call_count2 = 0;

	router_->subscribe("test.topic", [&call_count1](const message& msg) {
		call_count1++;
		return common::ok();
	});

	router_->subscribe("test.topic", [&call_count2](const message& msg) {
		call_count2++;
		return common::ok();
	});

	message msg("test.topic");
	auto result = router_->route(msg);

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(call_count1, 1);
	EXPECT_EQ(call_count2, 1);
}

TEST_F(TopicRouterTest, PartialFailure) {
	router_->subscribe("test.topic", [](const message& msg) {
		return common::error_info(
			common::error::codes::common_errors::internal_error,
			"Subscriber 1 failed");
	});

	router_->subscribe("test.topic",
					   [](const message& msg) { return common::ok(); });

	message msg("test.topic");
	auto result = router_->route(msg);

	// Should succeed because at least one subscriber succeeded
	EXPECT_TRUE(result.is_ok());
}

TEST_F(TopicRouterTest, AllSubscribersFail) {
	router_->subscribe("test.topic", [](const message& msg) {
		return common::error_info(
			common::error::codes::common_errors::internal_error,
			"Subscriber 1 failed");
	});

	router_->subscribe("test.topic", [](const message& msg) {
		return common::error_info(
			common::error::codes::common_errors::internal_error,
			"Subscriber 2 failed");
	});

	message msg("test.topic");
	auto result = router_->route(msg);

	EXPECT_TRUE(result.is_err());
}

// Utility methods tests
TEST_F(TopicRouterTest, SubscriberCount) {
	router_->subscribe("user.created",
					   [](const message& msg) { return common::ok(); });
	router_->subscribe("user.created",
					   [](const message& msg) { return common::ok(); });
	router_->subscribe("user.*",
					   [](const message& msg) { return common::ok(); });

	EXPECT_EQ(router_->subscriber_count("user.created"), 3);
	EXPECT_EQ(router_->subscriber_count("user.updated"), 1);
	EXPECT_EQ(router_->subscriber_count("order.created"), 0);
}

TEST_F(TopicRouterTest, GetTopics) {
	router_->subscribe("user.created",
					   [](const message& msg) { return common::ok(); });
	router_->subscribe("user.updated",
					   [](const message& msg) { return common::ok(); });
	router_->subscribe("order.*",
					   [](const message& msg) { return common::ok(); });

	auto topics = router_->get_topics();

	EXPECT_EQ(topics.size(), 3);
	EXPECT_TRUE(std::find(topics.begin(), topics.end(), "user.created") !=
				topics.end());
	EXPECT_TRUE(std::find(topics.begin(), topics.end(), "user.updated") !=
				topics.end());
	EXPECT_TRUE(std::find(topics.begin(), topics.end(), "order.*") !=
				topics.end());
}

TEST_F(TopicRouterTest, Clear) {
	router_->subscribe("user.created",
					   [](const message& msg) { return common::ok(); });
	router_->subscribe("user.updated",
					   [](const message& msg) { return common::ok(); });

	EXPECT_EQ(router_->get_topics().size(), 2);

	router_->clear();

	EXPECT_EQ(router_->get_topics().size(), 0);

	message msg("user.created");
	auto result = router_->route(msg);
	EXPECT_TRUE(result.is_err());
}

// Threading tests
TEST_F(TopicRouterTest, ConcurrentSubscribe) {
	const int num_threads = 4;
	const int subscriptions_per_thread = 25;

	std::vector<std::thread> threads;
	for (int t = 0; t < num_threads; ++t) {
		threads.emplace_back([this, t, subscriptions_per_thread]() {
			for (int i = 0; i < subscriptions_per_thread; ++i) {
				router_->subscribe(
					"thread." + std::to_string(t) + ".msg." +
						std::to_string(i),
					[](const message& msg) { return common::ok(); });
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(router_->get_topics().size(),
			  num_threads * subscriptions_per_thread);
}

TEST_F(TopicRouterTest, ConcurrentRoute) {
	std::atomic<int> total_calls{0};

	router_->subscribe("test.topic", [&total_calls](const message& msg) {
		total_calls++;
		return common::ok();
	});

	const int num_threads = 4;
	const int messages_per_thread = 25;

	std::vector<std::thread> threads;
	for (int t = 0; t < num_threads; ++t) {
		threads.emplace_back([this, messages_per_thread]() {
			for (int i = 0; i < messages_per_thread; ++i) {
				message msg("test.topic");
				router_->route(msg);
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(total_calls, num_threads * messages_per_thread);
}

// Skip this test on Windows due to CI timeout issues
#ifndef _WIN32
TEST_F(TopicRouterTest, ConcurrentSubscribeUnsubscribe) {
	std::atomic<bool> running{true};
	std::atomic<int> route_success{0};

	// Subscriber thread
	std::thread subscriber_thread([this, &running]() {
		while (running) {
			auto result = router_->subscribe(
				"test.topic", [](const message& msg) { return common::ok(); });
			if (result.is_ok()) {
				router_->unsubscribe(result.unwrap());
			}
			std::this_thread::yield();
		}
	});

	// Router thread
	std::thread router_thread([this, &running, &route_success]() {
		while (running) {
			message msg("test.topic");
			auto result = router_->route(msg);
			if (result.is_ok()) {
				route_success++;
			}
			std::this_thread::yield();
		}
	});

	// Reduced time for CI compatibility
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	running = false;

	subscriber_thread.join();
	router_thread.join();

	// Test completes without crashes
	SUCCEED();
}
#endif

// Edge cases
TEST_F(TopicRouterTest, EmptyTopic) {
	router_->subscribe("test.topic",
					   [](const message& msg) { return common::ok(); });

	message msg("");
	auto result = router_->route(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(TopicRouterTest, TopicWithOnlyDots) {
	router_->subscribe("...", [](const message& msg) { return common::ok(); });

	message msg("...");
	auto result = router_->route(msg);

	ASSERT_TRUE(result.is_ok());
}

TEST_F(TopicRouterTest, VeryLongTopic) {
	std::string long_topic;
	for (int i = 0; i < 100; ++i) {
		if (i > 0)
			long_topic += ".";
		long_topic += "segment" + std::to_string(i);
	}

	router_->subscribe(long_topic,
					   [](const message& msg) { return common::ok(); });

	message msg(long_topic);
	auto result = router_->route(msg);

	ASSERT_TRUE(result.is_ok());
}
