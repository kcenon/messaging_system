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
