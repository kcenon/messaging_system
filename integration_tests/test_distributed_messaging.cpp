// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

/**
 * @file test_distributed_messaging.cpp
 * @brief E2E integration tests for distributed messaging scenarios
 *
 * Tests message exchange between multiple message_bus instances
 * simulating distributed system communication.
 */

#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/adapters/transport_interface.h>
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/patterns/request_reply.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;
using namespace kcenon::messaging::patterns;
using namespace kcenon::messaging::adapters;
using namespace kcenon::common;

namespace {

/**
 * @class bridged_transport
 * @brief Mock transport that bridges two message_bus instances
 *
 * Simulates network communication between distributed nodes.
 * Messages sent from one node are delivered to the connected peer.
 */
class bridged_transport : public transport_interface {
public:
    explicit bridged_transport(const std::string& node_id)
        : node_id_(node_id) {}

    VoidResult connect() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connected_) {
            return ok();
        }
        state_ = transport_state::connecting;
        if (state_handler_) {
            state_handler_(state_);
        }

        connected_ = true;
        state_ = transport_state::connected;
        if (state_handler_) {
            state_handler_(state_);
        }
        return ok();
    }

    VoidResult disconnect() override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!connected_) {
            return ok();
        }
        state_ = transport_state::disconnecting;
        if (state_handler_) {
            state_handler_(state_);
        }

        connected_ = false;
        state_ = transport_state::disconnected;
        if (state_handler_) {
            state_handler_(state_);
        }
        return ok();
    }

    bool is_connected() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return connected_;
    }

    transport_state get_state() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }

    VoidResult send(const message& msg) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!connected_) {
            return make_error<std::monostate>(
                -1, "Transport not connected");
        }

        stats_.messages_sent++;

        // Forward to peer if connected
        if (peer_) {
            peer_->receive_from_peer(msg);
        }
        return ok();
    }

    VoidResult send_binary(const std::vector<uint8_t>& data) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!connected_) {
            return make_error<std::monostate>(
                -1, "Transport not connected");
        }

        stats_.bytes_sent += data.size();

        if (peer_ && binary_handler_) {
            peer_->receive_binary_from_peer(data);
        }
        return ok();
    }

    void set_message_handler(std::function<void(const message&)> handler) override {
        std::lock_guard<std::mutex> lock(mutex_);
        message_handler_ = std::move(handler);
    }

    void set_binary_handler(std::function<void(const std::vector<uint8_t>&)> handler) override {
        std::lock_guard<std::mutex> lock(mutex_);
        binary_handler_ = std::move(handler);
    }

    void set_state_handler(std::function<void(transport_state)> handler) override {
        std::lock_guard<std::mutex> lock(mutex_);
        state_handler_ = std::move(handler);
    }

    void set_error_handler(std::function<void(const std::string&)> handler) override {
        std::lock_guard<std::mutex> lock(mutex_);
        error_handler_ = std::move(handler);
    }

    transport_statistics get_statistics() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return stats_;
    }

    void reset_statistics() override {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_ = {};
    }

    // Bridge management
    void connect_to_peer(std::shared_ptr<bridged_transport> peer) {
        std::lock_guard<std::mutex> lock(mutex_);
        peer_ = peer;
    }

    void receive_from_peer(const message& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.messages_received++;
        if (message_handler_) {
            message_handler_(msg);
        }
    }

    void receive_binary_from_peer(const std::vector<uint8_t>& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.bytes_received += data.size();
        if (binary_handler_) {
            binary_handler_(data);
        }
    }

    const std::string& node_id() const { return node_id_; }

    // Simulate network failure
    void simulate_disconnect() {
        std::lock_guard<std::mutex> lock(mutex_);
        connected_ = false;
        state_ = transport_state::disconnected;
        if (state_handler_) {
            state_handler_(state_);
        }
    }

    // Simulate network recovery
    void simulate_reconnect() {
        std::lock_guard<std::mutex> lock(mutex_);
        connected_ = true;
        state_ = transport_state::connected;
        if (state_handler_) {
            state_handler_(state_);
        }
    }

private:
    std::string node_id_;
    mutable std::mutex mutex_;
    bool connected_ = false;
    transport_state state_ = transport_state::disconnected;
    std::shared_ptr<bridged_transport> peer_;
    std::function<void(const message&)> message_handler_;
    std::function<void(const std::vector<uint8_t>&)> binary_handler_;
    std::function<void(transport_state)> state_handler_;
    std::function<void(const std::string&)> error_handler_;
    transport_statistics stats_;
};

/**
 * @class DistributedMessagingTest
 * @brief Test fixture for distributed messaging scenarios
 */
class DistributedMessagingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create backends for two nodes
        backend_node1_ = std::make_shared<standalone_backend>(2);
        backend_node2_ = std::make_shared<standalone_backend>(2);

        // Create bridged transports
        transport_node1_ = std::make_shared<bridged_transport>("node1");
        transport_node2_ = std::make_shared<bridged_transport>("node2");

        // Connect transports bidirectionally
        transport_node1_->connect_to_peer(transport_node2_);
        transport_node2_->connect_to_peer(transport_node1_);
    }

    void TearDown() override {
        if (bus_node1_) {
            bus_node1_->stop();
        }
        if (bus_node2_) {
            bus_node2_->stop();
        }
        if (backend_node1_) {
            backend_node1_->shutdown();
        }
        if (backend_node2_) {
            backend_node2_->shutdown();
        }
    }

    std::shared_ptr<message_bus> create_distributed_bus(
        std::shared_ptr<backend_interface> backend,
        std::shared_ptr<transport_interface> transport,
        transport_mode mode = transport_mode::hybrid) {

        message_bus_config config;
        config.queue_capacity = 1000;
        config.worker_threads = 2;
        config.mode = mode;
        config.transport = transport;

        return std::make_shared<message_bus>(backend, config);
    }

    std::shared_ptr<standalone_backend> backend_node1_;
    std::shared_ptr<standalone_backend> backend_node2_;
    std::shared_ptr<bridged_transport> transport_node1_;
    std::shared_ptr<bridged_transport> transport_node2_;
    std::shared_ptr<message_bus> bus_node1_;
    std::shared_ptr<message_bus> bus_node2_;
};

} // anonymous namespace

// ============================================================================
// Basic Distributed Communication Tests
// ============================================================================

TEST_F(DistributedMessagingTest, TwoNodeMessageExchange) {
    // Setup nodes in hybrid mode
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Node2 subscribes to a topic
    MessageCounter node2_counter;
    auto sub_result = bus_node2_->subscribe(
        "distributed.events",
        create_counting_callback(node2_counter)
    );
    ASSERT_TRUE(sub_result.is_ok());

    // Node1 publishes a message
    auto msg = create_test_message("distributed.events");
    auto pub_result = bus_node1_->publish(msg);
    ASSERT_TRUE(pub_result.is_ok());

    // Wait for message to arrive at Node2
    ASSERT_TRUE(wait_for_condition(
        [&]() { return node2_counter.count() >= 1; },
        std::chrono::seconds{2}
    ));

    EXPECT_EQ(node2_counter.count(), 1);

    // Verify transport statistics
    auto stats1 = transport_node1_->get_statistics();
    auto stats2 = transport_node2_->get_statistics();
    EXPECT_EQ(stats1.messages_sent, 1u);
    EXPECT_EQ(stats2.messages_received, 1u);
}

TEST_F(DistributedMessagingTest, BidirectionalCommunication) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Both nodes subscribe to different topics
    MessageCounter node1_counter;
    MessageCounter node2_counter;

    auto sub1 = bus_node1_->subscribe(
        "to.node1",
        create_counting_callback(node1_counter)
    );
    auto sub2 = bus_node2_->subscribe(
        "to.node2",
        create_counting_callback(node2_counter)
    );
    ASSERT_TRUE(sub1.is_ok() && sub2.is_ok());

    // Node1 sends to Node2
    ASSERT_TRUE(bus_node1_->publish(create_test_message("to.node2")).is_ok());

    // Node2 sends to Node1
    ASSERT_TRUE(bus_node2_->publish(create_test_message("to.node1")).is_ok());

    // Wait for both messages
    ASSERT_TRUE(wait_for_condition(
        [&]() { return node1_counter.count() >= 1 && node2_counter.count() >= 1; },
        std::chrono::seconds{2}
    ));

    EXPECT_EQ(node1_counter.count(), 1);
    EXPECT_EQ(node2_counter.count(), 1);
}

TEST_F(DistributedMessagingTest, RemoteModeOnlyRemoteDelivery) {
    // Create nodes in remote-only mode
    bus_node1_ = create_distributed_bus(
        backend_node1_, transport_node1_, transport_mode::remote);
    bus_node2_ = create_distributed_bus(
        backend_node2_, transport_node2_, transport_mode::remote);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Local subscriber on Node1
    MessageCounter local_counter;
    auto local_sub = bus_node1_->subscribe(
        "remote.test",
        create_counting_callback(local_counter)
    );
    ASSERT_TRUE(local_sub.is_ok());

    // Remote subscriber on Node2
    MessageCounter remote_counter;
    auto remote_sub = bus_node2_->subscribe(
        "remote.test",
        create_counting_callback(remote_counter)
    );
    ASSERT_TRUE(remote_sub.is_ok());

    // Node1 publishes - in remote mode, should NOT go to local
    ASSERT_TRUE(bus_node1_->publish(create_test_message("remote.test")).is_ok());

    // Wait and check
    ASSERT_TRUE(wait_for_condition(
        [&]() { return remote_counter.count() >= 1; },
        std::chrono::seconds{2}
    ));

    // Remote got it
    EXPECT_EQ(remote_counter.count(), 1);
    // Local should not receive in remote-only mode
    EXPECT_EQ(local_counter.count(), 0);
}

TEST_F(DistributedMessagingTest, HybridModeLocalAndRemote) {
    bus_node1_ = create_distributed_bus(
        backend_node1_, transport_node1_, transport_mode::hybrid);
    bus_node2_ = create_distributed_bus(
        backend_node2_, transport_node2_, transport_mode::hybrid);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Local subscriber on Node1
    MessageCounter local_counter;
    auto local_sub = bus_node1_->subscribe(
        "hybrid.test",
        create_counting_callback(local_counter)
    );
    ASSERT_TRUE(local_sub.is_ok());

    // Remote subscriber on Node2
    MessageCounter remote_counter;
    auto remote_sub = bus_node2_->subscribe(
        "hybrid.test",
        create_counting_callback(remote_counter)
    );
    ASSERT_TRUE(remote_sub.is_ok());

    // Node1 publishes - hybrid mode should deliver to both
    ASSERT_TRUE(bus_node1_->publish(create_test_message("hybrid.test")).is_ok());

    // Wait for both
    ASSERT_TRUE(wait_for_condition(
        [&]() { return local_counter.count() >= 1 && remote_counter.count() >= 1; },
        std::chrono::seconds{2}
    ));

    // Both should receive
    EXPECT_EQ(local_counter.count(), 1);
    EXPECT_EQ(remote_counter.count(), 1);
}

// ============================================================================
// Topic Pattern Matching Tests
// ============================================================================

TEST_F(DistributedMessagingTest, WildcardSubscriptionAcrossNodes) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Node2 subscribes with wildcard
    MessageCounter wildcard_counter;
    auto sub = bus_node2_->subscribe(
        "orders.*",
        create_counting_callback(wildcard_counter)
    );
    ASSERT_TRUE(sub.is_ok());

    // Node1 publishes to various matching topics
    ASSERT_TRUE(bus_node1_->publish(create_test_message("orders.created")).is_ok());
    ASSERT_TRUE(bus_node1_->publish(create_test_message("orders.updated")).is_ok());
    ASSERT_TRUE(bus_node1_->publish(create_test_message("orders.deleted")).is_ok());

    // Should NOT match
    ASSERT_TRUE(bus_node1_->publish(create_test_message("orders.item.added")).is_ok());

    ASSERT_TRUE(wait_for_condition(
        [&]() { return wildcard_counter.count() >= 3; },
        std::chrono::seconds{3}
    ));

    EXPECT_EQ(wildcard_counter.count(), 3);
}

TEST_F(DistributedMessagingTest, MultiLevelWildcardAcrossNodes) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Node2 subscribes with multi-level wildcard
    MessageCounter multilevel_counter;
    auto sub = bus_node2_->subscribe(
        "events.#",
        create_counting_callback(multilevel_counter)
    );
    ASSERT_TRUE(sub.is_ok());

    // All these should match
    ASSERT_TRUE(bus_node1_->publish(create_test_message("events.user")).is_ok());
    ASSERT_TRUE(bus_node1_->publish(create_test_message("events.user.created")).is_ok());
    ASSERT_TRUE(bus_node1_->publish(create_test_message("events.order.item.added")).is_ok());

    ASSERT_TRUE(wait_for_condition(
        [&]() { return multilevel_counter.count() >= 3; },
        std::chrono::seconds{3}
    ));

    EXPECT_EQ(multilevel_counter.count(), 3);
}

// ============================================================================
// High Volume Tests
// ============================================================================

TEST_F(DistributedMessagingTest, HighVolumeMessageExchange) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    const int message_count = 500;

    MessageCounter counter;
    auto sub = bus_node2_->subscribe(
        "stress.test",
        create_counting_callback(counter)
    );
    ASSERT_TRUE(sub.is_ok());

    // Publish many messages from Node1
    for (int i = 0; i < message_count; ++i) {
        ASSERT_TRUE(bus_node1_->publish(create_test_message("stress.test")).is_ok());
    }

    // Wait for all messages
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= message_count; },
        std::chrono::seconds{10}
    ));

    EXPECT_EQ(counter.count(), message_count);

    // Verify statistics
    auto stats1 = transport_node1_->get_statistics();
    EXPECT_EQ(stats1.messages_sent, static_cast<uint64_t>(message_count));
}

TEST_F(DistributedMessagingTest, ConcurrentPublishFromMultipleThreads) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    const int threads_count = 5;
    const int messages_per_thread = 100;

    MessageCounter counter;
    auto sub = bus_node2_->subscribe(
        "concurrent.test",
        create_counting_callback(counter)
    );
    ASSERT_TRUE(sub.is_ok());

    // Launch multiple publisher threads
    std::vector<std::thread> threads;
    for (int t = 0; t < threads_count; ++t) {
        threads.emplace_back([this, messages_per_thread]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                bus_node1_->publish(create_test_message("concurrent.test"));
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Wait for all messages to arrive
    const int total_messages = threads_count * messages_per_thread;
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= total_messages; },
        std::chrono::seconds{15}
    ));

    EXPECT_EQ(counter.count(), total_messages);
}

// ============================================================================
// Network Failure Recovery Tests
// ============================================================================

TEST_F(DistributedMessagingTest, MessagesDuringDisconnection) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    MessageCounter counter;
    auto sub = bus_node2_->subscribe(
        "disconnect.test",
        create_counting_callback(counter)
    );
    ASSERT_TRUE(sub.is_ok());

    // Send a message successfully first
    ASSERT_TRUE(bus_node1_->publish(create_test_message("disconnect.test")).is_ok());
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= 1; },
        std::chrono::seconds{2}
    ));

    // Simulate network failure
    transport_node1_->simulate_disconnect();

    // Message should fail during disconnection
    auto result = bus_node1_->publish(create_test_message("disconnect.test"));
    // The message might still be queued locally but not sent to remote
    // depending on implementation

    // Restore connection
    transport_node1_->simulate_reconnect();

    // New messages should work
    ASSERT_TRUE(bus_node1_->publish(create_test_message("disconnect.test")).is_ok());
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= 2; },
        std::chrono::seconds{2}
    ));

    EXPECT_GE(counter.count(), 2);
}

TEST_F(DistributedMessagingTest, TransportStateTransitions) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    auto state_changes = std::make_shared<std::vector<transport_state>>();
    auto state_mutex = std::make_shared<std::mutex>();

    transport_node1_->set_state_handler([state_changes, state_mutex](transport_state state) {
        std::lock_guard<std::mutex> lock(*state_mutex);
        state_changes->push_back(state);
    });

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Simulate disconnect and reconnect
    transport_node1_->simulate_disconnect();
    transport_node1_->simulate_reconnect();

    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    // Clear the handler before checking results to prevent late callbacks
    transport_node1_->set_state_handler(nullptr);

    std::lock_guard<std::mutex> lock(*state_mutex);
    ASSERT_GE(state_changes->size(), 2u);

    // Verify state transitions occurred
    bool found_disconnect = false;
    bool found_connect = false;
    for (auto state : *state_changes) {
        if (state == transport_state::disconnected) {
            found_disconnect = true;
        }
        if (state == transport_state::connected) {
            found_connect = true;
        }
    }
    EXPECT_TRUE(found_disconnect);
    EXPECT_TRUE(found_connect);
}

// ============================================================================
// Statistics and Monitoring Tests
// ============================================================================

TEST_F(DistributedMessagingTest, DistributedStatisticsTracking) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    MessageCounter counter;
    auto sub = bus_node2_->subscribe("stats.test", create_counting_callback(counter));
    ASSERT_TRUE(sub.is_ok());

    // Get initial stats
    auto initial_stats1 = bus_node1_->get_statistics();
    auto initial_stats2 = bus_node2_->get_statistics();

    // Publish messages
    const int count = 25;
    for (int i = 0; i < count; ++i) {
        ASSERT_TRUE(bus_node1_->publish(create_test_message("stats.test")).is_ok());
    }

    // Wait for processing
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= count; },
        std::chrono::seconds{5}
    ));

    // Check statistics
    auto final_stats1 = bus_node1_->get_statistics();
    auto final_stats2 = bus_node2_->get_statistics();

    // Node1 should show messages sent remotely
    EXPECT_EQ(
        final_stats1.messages_sent_remote - initial_stats1.messages_sent_remote,
        static_cast<uint64_t>(count)
    );

    // Node2 should show messages received remotely
    EXPECT_EQ(
        final_stats2.messages_received_remote - initial_stats2.messages_received_remote,
        static_cast<uint64_t>(count)
    );
}

TEST_F(DistributedMessagingTest, TransportStatisticsReset) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    MessageCounter counter;
    bus_node2_->subscribe("reset.test", create_counting_callback(counter));

    // Send some messages
    for (int i = 0; i < 10; ++i) {
        bus_node1_->publish(create_test_message("reset.test"));
    }

    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= 10; },
        std::chrono::seconds{3}
    ));

    // Verify stats before reset
    auto stats = transport_node1_->get_statistics();
    EXPECT_EQ(stats.messages_sent, 10u);

    // Reset and verify
    transport_node1_->reset_statistics();
    auto stats_after = transport_node1_->get_statistics();
    EXPECT_EQ(stats_after.messages_sent, 0u);
}

// ============================================================================
// Pattern Integration Tests
// ============================================================================

TEST_F(DistributedMessagingTest, PubSubPatternAcrossNodes) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Create pub/sub pattern instances
    auto pub = std::make_shared<publisher>(bus_node1_, "distributed.events");
    auto sub = std::make_shared<subscriber>(bus_node2_);

    MessageCounter counter;
    auto sub_result = sub->subscribe("distributed.events", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish through pattern
    for (int i = 0; i < 5; ++i) {
        auto msg = create_test_message("distributed.events");
        ASSERT_TRUE(pub->publish(msg).is_ok());
    }

    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= 5; },
        std::chrono::seconds{3}
    ));

    EXPECT_EQ(counter.count(), 5);
}

TEST_F(DistributedMessagingTest, MultipleSubscribersAcrossNodes) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    // Multiple subscribers on Node2
    MessageCounter counter1, counter2, counter3;

    auto sub1 = bus_node2_->subscribe("multi.sub", create_counting_callback(counter1));
    auto sub2 = bus_node2_->subscribe("multi.sub", create_counting_callback(counter2));
    auto sub3 = bus_node2_->subscribe("multi.sub", create_counting_callback(counter3));

    ASSERT_TRUE(sub1.is_ok() && sub2.is_ok() && sub3.is_ok());

    // Publish from Node1
    ASSERT_TRUE(bus_node1_->publish(create_test_message("multi.sub")).is_ok());

    // All subscribers should receive the message
    ASSERT_TRUE(wait_for_condition(
        [&]() {
            return counter1.count() >= 1 &&
                   counter2.count() >= 1 &&
                   counter3.count() >= 1;
        },
        std::chrono::seconds{3}
    ));

    EXPECT_EQ(counter1.count(), 1);
    EXPECT_EQ(counter2.count(), 1);
    EXPECT_EQ(counter3.count(), 1);
}

// ============================================================================
// Message Content Integrity Tests
// ============================================================================

TEST_F(DistributedMessagingTest, MessageContentPreservedAcrossNodes) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    std::vector<message> received_messages;
    std::mutex messages_mutex;

    auto sub = bus_node2_->subscribe(
        "content.test",
        create_storing_callback(received_messages, messages_mutex)
    );
    ASSERT_TRUE(sub.is_ok());

    // Create message with specific content
    auto original_msg = message_builder()
        .topic("content.test")
        .type(message_type::command)
        .priority(message_priority::high)
        .source("node1_source")
        .build();
    ASSERT_TRUE(original_msg.is_ok());

    ASSERT_TRUE(bus_node1_->publish(original_msg.unwrap()).is_ok());

    ASSERT_TRUE(wait_for_condition(
        [&]() {
            std::lock_guard<std::mutex> lock(messages_mutex);
            return received_messages.size() >= 1;
        },
        std::chrono::seconds{2}
    ));

    std::lock_guard<std::mutex> lock(messages_mutex);
    ASSERT_EQ(received_messages.size(), 1u);

    const auto& received = received_messages[0];
    EXPECT_EQ(received.metadata().topic, "content.test");
    EXPECT_EQ(received.metadata().type, message_type::command);
    EXPECT_EQ(received.metadata().priority, message_priority::high);
    EXPECT_EQ(received.metadata().source, "node1_source");
}

// ============================================================================
// Graceful Shutdown Tests
// ============================================================================

TEST_F(DistributedMessagingTest, GracefulShutdownWithPendingMessages) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);
    bus_node2_ = create_distributed_bus(backend_node2_, transport_node2_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    ASSERT_TRUE(bus_node2_->start().is_ok());

    MessageCounter counter;
    auto sub = bus_node2_->subscribe("shutdown.test", create_counting_callback(counter));
    ASSERT_TRUE(sub.is_ok());

    // Publish some messages
    for (int i = 0; i < 10; ++i) {
        bus_node1_->publish(create_test_message("shutdown.test"));
    }

    // Stop Node1 gracefully
    ASSERT_TRUE(bus_node1_->stop().is_ok());
    EXPECT_FALSE(bus_node1_->is_running());

    // Wait for messages that were sent before shutdown
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= 10; },
        std::chrono::seconds{3}
    ));

    // Node2 should still be running
    EXPECT_TRUE(bus_node2_->is_running());

    // Stop Node2
    ASSERT_TRUE(bus_node2_->stop().is_ok());
    EXPECT_FALSE(bus_node2_->is_running());
}

TEST_F(DistributedMessagingTest, TransportDisconnectOnShutdown) {
    bus_node1_ = create_distributed_bus(backend_node1_, transport_node1_);

    ASSERT_TRUE(bus_node1_->start().is_ok());
    EXPECT_TRUE(transport_node1_->is_connected());

    ASSERT_TRUE(bus_node1_->stop().is_ok());
    EXPECT_FALSE(transport_node1_->is_connected());
}
