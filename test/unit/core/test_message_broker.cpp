#include <gtest/gtest.h>

#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/core/message_broker.h>

#include <atomic>
#include <set>
#include <thread>

using namespace kcenon::messaging;
using namespace kcenon;

class MessageBrokerTest : public ::testing::Test {
protected:
	void SetUp() override {
		broker_ = std::make_unique<message_broker>();
	}

	void TearDown() override {
		if (broker_) {
			if (broker_->is_running()) {
				broker_->stop();
			}
			broker_->clear_routes();
		}
	}

	std::unique_ptr<message_broker> broker_;
};

// =============================================================================
// Lifecycle Tests
// =============================================================================

TEST_F(MessageBrokerTest, StartSuccess) {
	EXPECT_FALSE(broker_->is_running());

	auto result = broker_->start();

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(broker_->is_running());
}

TEST_F(MessageBrokerTest, StopSuccess) {
	broker_->start();
	EXPECT_TRUE(broker_->is_running());

	auto result = broker_->stop();

	ASSERT_TRUE(result.is_ok());
	EXPECT_FALSE(broker_->is_running());
}

TEST_F(MessageBrokerTest, StartAlreadyRunning) {
	broker_->start();

	auto result = broker_->start();

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, StopNotRunning) {
	auto result = broker_->stop();

	EXPECT_TRUE(result.is_err());
}

// =============================================================================
// Route Management Tests
// =============================================================================

TEST_F(MessageBrokerTest, AddRouteSuccess) {
	auto result = broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(broker_->has_route("test-route"));
	EXPECT_EQ(broker_->route_count(), 1);
}

TEST_F(MessageBrokerTest, AddRouteEmptyId) {
	auto result = broker_->add_route(
		"",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, AddRouteEmptyPattern) {
	auto result = broker_->add_route(
		"test-route",
		"",
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, AddRouteNullHandler) {
	auto result = broker_->add_route(
		"test-route",
		"test.topic",
		nullptr
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, AddRouteInvalidPriority) {
	auto result1 = broker_->add_route(
		"test-route-1",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); },
		-1
	);
	EXPECT_TRUE(result1.is_err());

	auto result2 = broker_->add_route(
		"test-route-2",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); },
		11
	);
	EXPECT_TRUE(result2.is_err());
}

TEST_F(MessageBrokerTest, AddRouteDuplicate) {
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	auto result = broker_->add_route(
		"test-route",
		"test.topic.2",
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, RemoveRouteSuccess) {
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	auto result = broker_->remove_route("test-route");

	ASSERT_TRUE(result.is_ok());
	EXPECT_FALSE(broker_->has_route("test-route"));
	EXPECT_EQ(broker_->route_count(), 0);
}

TEST_F(MessageBrokerTest, RemoveRouteNotFound) {
	auto result = broker_->remove_route("nonexistent-route");

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, EnableDisableRoute) {
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	// Disable route
	auto disable_result = broker_->disable_route("test-route");
	ASSERT_TRUE(disable_result.is_ok());

	auto route_info = broker_->get_route("test-route");
	ASSERT_TRUE(route_info.is_ok());
	EXPECT_FALSE(route_info.unwrap().active);

	// Enable route
	auto enable_result = broker_->enable_route("test-route");
	ASSERT_TRUE(enable_result.is_ok());

	route_info = broker_->get_route("test-route");
	ASSERT_TRUE(route_info.is_ok());
	EXPECT_TRUE(route_info.unwrap().active);
}

TEST_F(MessageBrokerTest, EnableDisableRouteNotFound) {
	auto enable_result = broker_->enable_route("nonexistent-route");
	EXPECT_TRUE(enable_result.is_err());

	auto disable_result = broker_->disable_route("nonexistent-route");
	EXPECT_TRUE(disable_result.is_err());
}

TEST_F(MessageBrokerTest, GetRouteSuccess) {
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); },
		8
	);

	auto result = broker_->get_route("test-route");

	ASSERT_TRUE(result.is_ok());
	auto info = result.unwrap();
	EXPECT_EQ(info.route_id, "test-route");
	EXPECT_EQ(info.topic_pattern, "test.topic");
	EXPECT_EQ(info.priority, 8);
	EXPECT_TRUE(info.active);
}

TEST_F(MessageBrokerTest, GetRouteNotFound) {
	auto result = broker_->get_route("nonexistent-route");

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, GetRoutes) {
	broker_->add_route("route-1", "topic.1", [](const message& /* msg */) { return common::ok(); });
	broker_->add_route("route-2", "topic.2", [](const message& /* msg */) { return common::ok(); });
	broker_->add_route("route-3", "topic.3", [](const message& /* msg */) { return common::ok(); });

	auto routes = broker_->get_routes();

	EXPECT_EQ(routes.size(), 3);
}

TEST_F(MessageBrokerTest, ClearRoutes) {
	broker_->add_route("route-1", "topic.1", [](const message& /* msg */) { return common::ok(); });
	broker_->add_route("route-2", "topic.2", [](const message& /* msg */) { return common::ok(); });

	EXPECT_EQ(broker_->route_count(), 2);

	broker_->clear_routes();

	EXPECT_EQ(broker_->route_count(), 0);
}

// =============================================================================
// Message Routing Tests
// =============================================================================

TEST_F(MessageBrokerTest, RouteSuccess) {
	broker_->start();

	int call_count = 0;
	broker_->add_route(
		"test-route",
		"test.topic",
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	message msg("test.topic");
	auto result = broker_->route(msg);

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, RouteNotRunning) {
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	message msg("test.topic");
	auto result = broker_->route(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, RouteNoMatchingRoute) {
	broker_->start();

	broker_->add_route(
		"test-route",
		"other.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	message msg("test.topic");
	auto result = broker_->route(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, RouteWildcardSingleLevel) {
	broker_->start();

	int call_count = 0;
	broker_->add_route(
		"test-route",
		"user.*",
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	message msg1("user.created");
	auto result1 = broker_->route(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	message msg2("user.updated");
	auto result2 = broker_->route(msg2);
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(call_count, 2);

	// Should not match multi-level
	message msg3("user.profile.updated");
	auto result3 = broker_->route(msg3);
	EXPECT_TRUE(result3.is_err());
	EXPECT_EQ(call_count, 2);
}

TEST_F(MessageBrokerTest, RouteWildcardMultiLevel) {
	broker_->start();

	int call_count = 0;
	broker_->add_route(
		"test-route",
		"user.#",
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	message msg1("user.created");
	auto result1 = broker_->route(msg1);
	ASSERT_TRUE(result1.is_ok());

	message msg2("user.profile.updated");
	auto result2 = broker_->route(msg2);
	ASSERT_TRUE(result2.is_ok());

	message msg3("user.profile.settings.changed");
	auto result3 = broker_->route(msg3);
	ASSERT_TRUE(result3.is_ok());

	EXPECT_EQ(call_count, 3);
}

TEST_F(MessageBrokerTest, RoutePriorityOrdering) {
	broker_->start();

	std::vector<int> execution_order;

	broker_->add_route(
		"low-priority",
		"test.topic",
		[&execution_order](const message& /* msg */) {
			execution_order.push_back(1);
			return common::ok();
		},
		1
	);

	broker_->add_route(
		"high-priority",
		"test.topic",
		[&execution_order](const message& /* msg */) {
			execution_order.push_back(10);
			return common::ok();
		},
		10
	);

	broker_->add_route(
		"medium-priority",
		"test.topic",
		[&execution_order](const message& /* msg */) {
			execution_order.push_back(5);
			return common::ok();
		},
		5
	);

	message msg("test.topic");
	auto result = broker_->route(msg);

	ASSERT_TRUE(result.is_ok());
	ASSERT_EQ(execution_order.size(), 3);
	EXPECT_EQ(execution_order[0], 10);  // Highest priority first
	EXPECT_EQ(execution_order[1], 5);
	EXPECT_EQ(execution_order[2], 1);   // Lowest priority last
}

TEST_F(MessageBrokerTest, RouteDisabledRoute) {
	broker_->start();

	int call_count = 0;
	broker_->add_route(
		"test-route",
		"test.topic",
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	broker_->disable_route("test-route");

	message msg("test.topic");
	auto result = broker_->route(msg);

	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(call_count, 0);
}

TEST_F(MessageBrokerTest, RouteMultipleRoutes) {
	broker_->start();

	int call_count1 = 0;
	int call_count2 = 0;

	broker_->add_route(
		"route-1",
		"test.topic",
		[&call_count1](const message& /* msg */) {
			call_count1++;
			return common::ok();
		}
	);

	broker_->add_route(
		"route-2",
		"test.topic",
		[&call_count2](const message& /* msg */) {
			call_count2++;
			return common::ok();
		}
	);

	message msg("test.topic");
	auto result = broker_->route(msg);

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(call_count1, 1);
	EXPECT_EQ(call_count2, 1);
}

// =============================================================================
// Statistics Tests
// =============================================================================

TEST_F(MessageBrokerTest, StatisticsInitial) {
	auto stats = broker_->get_statistics();

	EXPECT_EQ(stats.messages_routed, 0);
	EXPECT_EQ(stats.messages_delivered, 0);
	EXPECT_EQ(stats.messages_failed, 0);
	EXPECT_EQ(stats.messages_unrouted, 0);
	EXPECT_EQ(stats.active_routes, 0);
}

TEST_F(MessageBrokerTest, StatisticsAfterRouting) {
	broker_->start();

	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	message msg("test.topic");
	broker_->route(msg);
	broker_->route(msg);
	broker_->route(msg);

	auto stats = broker_->get_statistics();

	EXPECT_EQ(stats.messages_routed, 3);
	EXPECT_EQ(stats.messages_delivered, 3);
	EXPECT_EQ(stats.active_routes, 1);
}

TEST_F(MessageBrokerTest, StatisticsUnrouted) {
	broker_->start();

	message msg("nonexistent.topic");
	broker_->route(msg);

	auto stats = broker_->get_statistics();

	EXPECT_EQ(stats.messages_routed, 1);
	EXPECT_EQ(stats.messages_unrouted, 1);
	EXPECT_EQ(stats.messages_delivered, 0);
}

TEST_F(MessageBrokerTest, StatisticsReset) {
	broker_->start();

	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	message msg("test.topic");
	broker_->route(msg);
	broker_->route(msg);

	broker_->reset_statistics();

	auto stats = broker_->get_statistics();

	EXPECT_EQ(stats.messages_routed, 0);
	EXPECT_EQ(stats.messages_delivered, 0);
}

// =============================================================================
// Configuration Tests
// =============================================================================

TEST_F(MessageBrokerTest, CustomConfiguration) {
	broker_config config;
	config.max_routes = 5;
	config.enable_statistics = true;

	message_broker custom_broker(config);
	custom_broker.start();

	// Add routes up to the limit
	for (int i = 0; i < 5; ++i) {
		auto result = custom_broker.add_route(
			"route-" + std::to_string(i),
			"topic." + std::to_string(i),
			[](const message& /* msg */) { return common::ok(); }
		);
		EXPECT_TRUE(result.is_ok());
	}

	// Attempt to exceed the limit
	auto result = custom_broker.add_route(
		"route-5",
		"topic.5",
		[](const message& /* msg */) { return common::ok(); }
	);
	EXPECT_TRUE(result.is_err());

	custom_broker.stop();
}

// =============================================================================
// Threading Tests
// =============================================================================

TEST_F(MessageBrokerTest, ConcurrentRouting) {
	broker_->start();

	std::atomic<int> total_calls{0};

	broker_->add_route(
		"test-route",
		"test.topic",
		[&total_calls](const message& /* msg */) {
			total_calls++;
			return common::ok();
		}
	);

	const int num_threads = 4;
	const int messages_per_thread = 25;

	std::vector<std::thread> threads;
	for (int t = 0; t < num_threads; ++t) {
		threads.emplace_back([this, messages_per_thread]() {
			for (int i = 0; i < messages_per_thread; ++i) {
				message msg("test.topic");
				broker_->route(msg);
			}
		});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(total_calls, num_threads * messages_per_thread);
}

TEST_F(MessageBrokerTest, ConcurrentRouteManagement) {
	broker_->start();

	std::atomic<bool> running{true};

	// Route management thread
	std::thread management_thread([this, &running]() {
		int counter = 0;
		while (running) {
			std::string route_id = "route-" + std::to_string(counter % 10);

			if (broker_->has_route(route_id)) {
				broker_->remove_route(route_id);
			} else {
				broker_->add_route(
					route_id,
					"topic." + std::to_string(counter % 10),
					[](const message& /* msg */) { return common::ok(); }
				);
			}
			counter++;
			std::this_thread::yield();
		}
	});

	// Routing thread
	std::thread routing_thread([this, &running]() {
		while (running) {
			message msg("topic.0");
			broker_->route(msg);
			std::this_thread::yield();
		}
	});

	// Run for a fixed number of iterations
	const int test_iterations = 1000;
	int iteration_count = 0;
	while (iteration_count < test_iterations) {
		iteration_count++;
		std::this_thread::yield();
	}
	running = false;

	management_thread.join();
	routing_thread.join();

	// Test completes without crashes
	SUCCEED();
}

// =============================================================================
// Move Semantics Tests
// =============================================================================

TEST_F(MessageBrokerTest, MoveConstruction) {
	broker_->start();
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	message_broker moved_broker(std::move(*broker_));

	// Reset broker_ to prevent TearDown from accessing moved-from object
	broker_.reset();

	EXPECT_TRUE(moved_broker.is_running());
	EXPECT_TRUE(moved_broker.has_route("test-route"));

	moved_broker.stop();
}

TEST_F(MessageBrokerTest, MoveAssignment) {
	broker_->start();
	broker_->add_route(
		"test-route",
		"test.topic",
		[](const message& /* msg */) { return common::ok(); }
	);

	message_broker other_broker;
	other_broker = std::move(*broker_);

	// Reset broker_ to prevent TearDown from accessing moved-from object
	broker_.reset();

	EXPECT_TRUE(other_broker.is_running());
	EXPECT_TRUE(other_broker.has_route("test-route"));

	other_broker.stop();
}

// =============================================================================
// Dead Letter Queue Tests
// =============================================================================

TEST_F(MessageBrokerTest, DLQNotConfigured) {
	EXPECT_FALSE(broker_->is_dlq_configured());

	message msg("test.topic");
	auto result = broker_->move_to_dlq(msg, "test failure");

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, DLQConfiguration) {
	dlq_config config;
	config.max_size = 100;
	config.retention_period = std::chrono::seconds(3600);
	config.on_full = dlq_policy::drop_oldest;

	auto result = broker_->configure_dlq(config);

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(broker_->is_dlq_configured());
}

TEST_F(MessageBrokerTest, DLQMoveMessage) {
	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	message msg("test.topic");
	auto result = broker_->move_to_dlq(msg, "handler failed");

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(broker_->get_dlq_size(), 1);
}

TEST_F(MessageBrokerTest, DLQGetMessages) {
	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	// Add 3 messages
	for (int i = 0; i < 3; ++i) {
		message msg("test.topic." + std::to_string(i));
		broker_->move_to_dlq(msg, "failure " + std::to_string(i));
	}

	// Get all messages
	auto all_messages = broker_->get_dlq_messages();
	EXPECT_EQ(all_messages.size(), 3);

	// Get limited messages
	auto limited_messages = broker_->get_dlq_messages(2);
	EXPECT_EQ(limited_messages.size(), 2);
}

TEST_F(MessageBrokerTest, DLQReplayMessage) {
	broker_->start();

	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	int call_count = 0;
	broker_->add_route(
		"test-route",
		"test.topic",
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	message msg("test.topic");
	std::string msg_id = msg.metadata().id;

	// Move message to DLQ
	broker_->move_to_dlq(msg, "initial failure");
	EXPECT_EQ(broker_->get_dlq_size(), 1);
	EXPECT_EQ(call_count, 0);

	// Replay the message
	auto result = broker_->replay_dlq_message(msg_id);
	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(broker_->get_dlq_size(), 0);
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, DLQReplayMessageNotFound) {
	broker_->start();

	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	auto result = broker_->replay_dlq_message("nonexistent-id");

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, DLQReplayAll) {
	broker_->start();

	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	int call_count = 0;
	broker_->add_route(
		"test-route",
		"test.topic",
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Add 3 messages to DLQ
	for (int i = 0; i < 3; ++i) {
		message msg("test.topic");
		broker_->move_to_dlq(msg, "failure");
	}
	EXPECT_EQ(broker_->get_dlq_size(), 3);

	// Replay all
	std::size_t replayed = broker_->replay_all_dlq_messages();

	EXPECT_EQ(replayed, 3);
	EXPECT_EQ(broker_->get_dlq_size(), 0);
	EXPECT_EQ(call_count, 3);
}

TEST_F(MessageBrokerTest, DLQPurge) {
	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	// Add messages
	for (int i = 0; i < 5; ++i) {
		message msg("test.topic");
		broker_->move_to_dlq(msg, "failure");
	}
	EXPECT_EQ(broker_->get_dlq_size(), 5);

	// Purge all
	std::size_t purged = broker_->purge_dlq();

	EXPECT_EQ(purged, 5);
	EXPECT_EQ(broker_->get_dlq_size(), 0);
}

TEST_F(MessageBrokerTest, DLQPurgeOld) {
	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	// Add a message
	message msg("test.topic");
	broker_->move_to_dlq(msg, "failure");
	EXPECT_EQ(broker_->get_dlq_size(), 1);

	// Purge messages older than 1 hour - should not purge recent message
	std::size_t purged = broker_->purge_dlq_older_than(std::chrono::seconds(3600));
	EXPECT_EQ(purged, 0);
	EXPECT_EQ(broker_->get_dlq_size(), 1);

	// Purge messages older than 0 seconds - should purge all
	purged = broker_->purge_dlq_older_than(std::chrono::seconds(0));
	EXPECT_EQ(purged, 1);
	EXPECT_EQ(broker_->get_dlq_size(), 0);
}

TEST_F(MessageBrokerTest, DLQStatistics) {
	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	// Initial stats
	auto stats = broker_->get_dlq_statistics();
	EXPECT_EQ(stats.current_size, 0);
	EXPECT_EQ(stats.total_received, 0);

	// Add messages
	message msg1("test.topic");
	broker_->move_to_dlq(msg1, "reason1");

	message msg2("test.topic");
	broker_->move_to_dlq(msg2, "reason2");

	message msg3("test.topic");
	broker_->move_to_dlq(msg3, "reason1");

	stats = broker_->get_dlq_statistics();
	EXPECT_EQ(stats.current_size, 3);
	EXPECT_EQ(stats.total_received, 3);
	EXPECT_EQ(stats.failure_reasons["reason1"], 2);
	EXPECT_EQ(stats.failure_reasons["reason2"], 1);
	EXPECT_TRUE(stats.oldest_entry.has_value());
}

TEST_F(MessageBrokerTest, DLQOverflowDropOldest) {
	dlq_config config;
	config.max_size = 3;
	config.on_full = dlq_policy::drop_oldest;
	broker_->configure_dlq(config);

	// Add 5 messages (exceeds max_size of 3)
	for (int i = 0; i < 5; ++i) {
		message msg("test.topic." + std::to_string(i));
		broker_->move_to_dlq(msg, "failure");
	}

	// Should only have 3 messages (oldest dropped)
	EXPECT_EQ(broker_->get_dlq_size(), 3);

	// Verify statistics show dropped messages
	auto stats = broker_->get_dlq_statistics();
	EXPECT_EQ(stats.total_received, 5);
	EXPECT_EQ(stats.total_purged, 2);  // 2 messages were dropped
}

TEST_F(MessageBrokerTest, DLQOverflowDropNewest) {
	dlq_config config;
	config.max_size = 3;
	config.on_full = dlq_policy::drop_newest;
	broker_->configure_dlq(config);

	// Add 3 messages to fill DLQ
	for (int i = 0; i < 3; ++i) {
		message msg("test.topic." + std::to_string(i));
		auto result = broker_->move_to_dlq(msg, "failure");
		ASSERT_TRUE(result.is_ok());
	}

	// Try to add 4th message - should be rejected
	message msg4("test.topic.3");
	auto result = broker_->move_to_dlq(msg4, "failure");

	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(broker_->get_dlq_size(), 3);
}

TEST_F(MessageBrokerTest, DLQCallback) {
	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	int callback_count = 0;
	std::string last_reason;

	broker_->on_dlq_message([&callback_count, &last_reason](const dlq_entry& entry) {
		callback_count++;
		last_reason = entry.failure_reason;
	});

	message msg("test.topic");
	broker_->move_to_dlq(msg, "test reason");

	EXPECT_EQ(callback_count, 1);
	EXPECT_EQ(last_reason, "test reason");
}

TEST_F(MessageBrokerTest, DLQFullCallback) {
	dlq_config config;
	config.max_size = 2;
	config.on_full = dlq_policy::drop_oldest;
	broker_->configure_dlq(config);

	int full_callback_count = 0;
	std::size_t reported_size = 0;

	broker_->on_dlq_full([&full_callback_count, &reported_size](std::size_t size) {
		full_callback_count++;
		reported_size = size;
	});

	// Fill the DLQ
	for (int i = 0; i < 2; ++i) {
		message msg("test.topic");
		broker_->move_to_dlq(msg, "failure");
	}
	EXPECT_EQ(full_callback_count, 0);  // Not full yet when adding

	// Add one more - triggers full callback
	message msg("test.topic");
	broker_->move_to_dlq(msg, "failure");

	EXPECT_EQ(full_callback_count, 1);
	EXPECT_EQ(reported_size, 2);
}

TEST_F(MessageBrokerTest, DLQReplayFailureUpdatesRetryCount) {
	broker_->start();

	dlq_config config;
	config.max_size = 100;
	broker_->configure_dlq(config);

	// Add route that always fails
	broker_->add_route(
		"failing-route",
		"test.topic",
		[](const message& /* msg */) {
			return common::make_error<std::monostate>(
				common::error::codes::common_errors::internal_error,
				"Simulated failure"
			);
		}
	);

	message msg("test.topic");
	std::string msg_id = msg.metadata().id;
	broker_->move_to_dlq(msg, "initial failure");

	// Try to replay - should fail and increment retry count
	auto result = broker_->replay_dlq_message(msg_id);
	EXPECT_TRUE(result.is_err());

	// Message should still be in DLQ with updated retry count
	EXPECT_EQ(broker_->get_dlq_size(), 1);

	auto messages = broker_->get_dlq_messages();
	ASSERT_EQ(messages.size(), 1);
	EXPECT_EQ(messages[0].retry_count, 1);
	EXPECT_TRUE(messages[0].last_error.has_value());
}

// =============================================================================
// Content-Based Routing Tests
// =============================================================================

TEST_F(MessageBrokerTest, AddContentRouteSuccess) {
	auto result = broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(broker_->has_content_route("test-content-route"));
	EXPECT_EQ(broker_->content_route_count(), 1);
}

TEST_F(MessageBrokerTest, AddContentRouteEmptyId) {
	auto result = broker_->add_content_route(
		"",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, AddContentRouteNullFilter) {
	auto result = broker_->add_content_route(
		"test-content-route",
		nullptr,
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, AddContentRouteNullHandler) {
	auto result = broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		nullptr
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, AddContentRouteDuplicate) {
	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	auto result = broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return false; },
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, RemoveContentRouteSuccess) {
	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	auto result = broker_->remove_content_route("test-content-route");

	ASSERT_TRUE(result.is_ok());
	EXPECT_FALSE(broker_->has_content_route("test-content-route"));
	EXPECT_EQ(broker_->content_route_count(), 0);
}

TEST_F(MessageBrokerTest, RemoveContentRouteNotFound) {
	auto result = broker_->remove_content_route("nonexistent-route");

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, EnableDisableContentRoute) {
	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	// Disable route
	auto disable_result = broker_->disable_content_route("test-content-route");
	ASSERT_TRUE(disable_result.is_ok());

	auto route_info = broker_->get_content_route("test-content-route");
	ASSERT_TRUE(route_info.is_ok());
	EXPECT_FALSE(route_info.unwrap().active);

	// Enable route
	auto enable_result = broker_->enable_content_route("test-content-route");
	ASSERT_TRUE(enable_result.is_ok());

	route_info = broker_->get_content_route("test-content-route");
	ASSERT_TRUE(route_info.is_ok());
	EXPECT_TRUE(route_info.unwrap().active);
}

TEST_F(MessageBrokerTest, GetContentRoutes) {
	broker_->add_content_route(
		"route-1",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);
	broker_->add_content_route(
		"route-2",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	auto routes = broker_->get_content_routes();

	EXPECT_EQ(routes.size(), 2);
}

TEST_F(MessageBrokerTest, ClearContentRoutes) {
	broker_->add_content_route(
		"route-1",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);
	broker_->add_content_route(
		"route-2",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	EXPECT_EQ(broker_->content_route_count(), 2);

	broker_->clear_content_routes();

	EXPECT_EQ(broker_->content_route_count(), 0);
}

TEST_F(MessageBrokerTest, RouteByContentSuccess) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"test-content-route",
		[](const message& msg) {
			return msg.metadata().topic == "test.topic";
		},
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, RouteByContentNotRunning) {
	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, RouteByContentNoMatch) {
	broker_->start();

	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return false; },  // Never matches
		[](const message& /* msg */) { return common::ok(); }
	);

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, RouteByContentMultipleMatches) {
	broker_->start();

	int call_count1 = 0;
	int call_count2 = 0;

	broker_->add_content_route(
		"route-1",
		[](const message& /* msg */) { return true; },
		[&call_count1](const message& /* msg */) {
			call_count1++;
			return common::ok();
		}
	);

	broker_->add_content_route(
		"route-2",
		[](const message& /* msg */) { return true; },
		[&call_count2](const message& /* msg */) {
			call_count2++;
			return common::ok();
		}
	);

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	ASSERT_TRUE(result.is_ok());
	EXPECT_EQ(call_count1, 1);
	EXPECT_EQ(call_count2, 1);
}

TEST_F(MessageBrokerTest, RouteByContentPriorityOrdering) {
	broker_->start();

	std::vector<int> execution_order;

	broker_->add_content_route(
		"low-priority",
		[](const message& /* msg */) { return true; },
		[&execution_order](const message& /* msg */) {
			execution_order.push_back(1);
			return common::ok();
		},
		1
	);

	broker_->add_content_route(
		"high-priority",
		[](const message& /* msg */) { return true; },
		[&execution_order](const message& /* msg */) {
			execution_order.push_back(10);
			return common::ok();
		},
		10
	);

	broker_->add_content_route(
		"medium-priority",
		[](const message& /* msg */) { return true; },
		[&execution_order](const message& /* msg */) {
			execution_order.push_back(5);
			return common::ok();
		},
		5
	);

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	ASSERT_TRUE(result.is_ok());
	ASSERT_EQ(execution_order.size(), 3);
	EXPECT_EQ(execution_order[0], 10);  // Highest priority first
	EXPECT_EQ(execution_order[1], 5);
	EXPECT_EQ(execution_order[2], 1);   // Lowest priority last
}

TEST_F(MessageBrokerTest, RouteByContentDisabledRoute) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	broker_->disable_content_route("test-content-route");

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	EXPECT_TRUE(result.is_err());  // No active routes matched
	EXPECT_EQ(call_count, 0);
}

TEST_F(MessageBrokerTest, RouteByContentHandlerFailure) {
	broker_->start();

	broker_->add_content_route(
		"failing-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) {
			return common::make_error<std::monostate>(
				common::error::codes::common_errors::internal_error,
				"Simulated failure"
			);
		}
	);

	message msg("test.topic");
	auto result = broker_->route_by_content(msg);

	EXPECT_TRUE(result.is_err());
}

TEST_F(MessageBrokerTest, ContentFilterMetadataEquals) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"region-filter",
		content_filters::metadata_equals("region", "EU"),
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Message with matching header
	message msg1("test.topic");
	msg1.metadata().headers["region"] = "EU";
	auto result1 = broker_->route_by_content(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	// Message with non-matching header
	message msg2("test.topic");
	msg2.metadata().headers["region"] = "US";
	auto result2 = broker_->route_by_content(msg2);
	EXPECT_TRUE(result2.is_err());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, ContentFilterMessageType) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"event-filter",
		content_filters::message_type_is(message_type::event),
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Event message should match
	message msg1("test.topic", message_type::event);
	auto result1 = broker_->route_by_content(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	// Command message should not match
	message msg2("test.topic", message_type::command);
	auto result2 = broker_->route_by_content(msg2);
	EXPECT_TRUE(result2.is_err());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, ContentFilterPriorityAtLeast) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"high-priority-filter",
		content_filters::priority_at_least(message_priority::high),
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Build high priority message
	auto high_msg_result = message_builder()
		.topic("test.topic")
		.priority(message_priority::high)
		.build();
	ASSERT_TRUE(high_msg_result.is_ok());

	auto result1 = broker_->route_by_content(high_msg_result.unwrap());
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	// Build low priority message
	auto low_msg_result = message_builder()
		.topic("test.topic")
		.priority(message_priority::low)
		.build();
	ASSERT_TRUE(low_msg_result.is_ok());

	auto result2 = broker_->route_by_content(low_msg_result.unwrap());
	EXPECT_TRUE(result2.is_err());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, ContentFilterCombineAllOf) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"combined-filter",
		content_filters::all_of({
			content_filters::metadata_equals("region", "EU"),
			content_filters::message_type_is(message_type::event)
		}),
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Message with both conditions met
	message msg1("test.topic", message_type::event);
	msg1.metadata().headers["region"] = "EU";
	auto result1 = broker_->route_by_content(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	// Message with only one condition met
	message msg2("test.topic", message_type::command);
	msg2.metadata().headers["region"] = "EU";
	auto result2 = broker_->route_by_content(msg2);
	EXPECT_TRUE(result2.is_err());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, ContentFilterCombineAnyOf) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"combined-filter",
		content_filters::any_of({
			content_filters::metadata_equals("region", "EU"),
			content_filters::metadata_equals("region", "UK")
		}),
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Message matching first filter
	message msg1("test.topic");
	msg1.metadata().headers["region"] = "EU";
	auto result1 = broker_->route_by_content(msg1);
	ASSERT_TRUE(result1.is_ok());
	EXPECT_EQ(call_count, 1);

	// Message matching second filter
	message msg2("test.topic");
	msg2.metadata().headers["region"] = "UK";
	auto result2 = broker_->route_by_content(msg2);
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(call_count, 2);

	// Message matching neither filter
	message msg3("test.topic");
	msg3.metadata().headers["region"] = "US";
	auto result3 = broker_->route_by_content(msg3);
	EXPECT_TRUE(result3.is_err());
	EXPECT_EQ(call_count, 2);
}

TEST_F(MessageBrokerTest, ContentFilterNot) {
	broker_->start();

	int call_count = 0;
	broker_->add_content_route(
		"not-filter",
		content_filters::not_filter(
			content_filters::metadata_equals("region", "EU")
		),
		[&call_count](const message& /* msg */) {
			call_count++;
			return common::ok();
		}
	);

	// Message that would match the inner filter should NOT match
	message msg1("test.topic");
	msg1.metadata().headers["region"] = "EU";
	auto result1 = broker_->route_by_content(msg1);
	EXPECT_TRUE(result1.is_err());
	EXPECT_EQ(call_count, 0);

	// Message that would not match the inner filter SHOULD match
	message msg2("test.topic");
	msg2.metadata().headers["region"] = "US";
	auto result2 = broker_->route_by_content(msg2);
	ASSERT_TRUE(result2.is_ok());
	EXPECT_EQ(call_count, 1);
}

TEST_F(MessageBrokerTest, ContentRouteStatistics) {
	broker_->start();

	broker_->add_content_route(
		"test-content-route",
		[](const message& /* msg */) { return true; },
		[](const message& /* msg */) { return common::ok(); }
	);

	// Route some messages
	for (int i = 0; i < 5; ++i) {
		message msg("test.topic");
		broker_->route_by_content(msg);
	}

	auto route_info = broker_->get_content_route("test-content-route");
	ASSERT_TRUE(route_info.is_ok());
	EXPECT_EQ(route_info.unwrap().messages_processed, 5);
}
