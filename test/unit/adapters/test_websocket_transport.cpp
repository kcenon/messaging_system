// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/adapters/websocket_transport.h>
#include <kcenon/messaging/config/feature_flags.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <atomic>

using namespace kcenon::messaging::adapters;
using namespace kcenon::messaging;

#if KCENON_WITH_NETWORK_SYSTEM

// ============================================================================
// Configuration Tests
// ============================================================================

TEST(WebSocketTransportConfigTest, DefaultValues) {
	websocket_transport_config config;

	EXPECT_TRUE(config.host.empty());
	EXPECT_EQ(config.port, 0);
	EXPECT_EQ(config.path, "/ws");
	EXPECT_FALSE(config.use_ssl);
	EXPECT_EQ(config.ping_interval.count(), 30000);
	EXPECT_TRUE(config.auto_pong);
	EXPECT_EQ(config.max_message_size, 10 * 1024 * 1024);
	EXPECT_EQ(config.reconnect_delay.count(), 1000);
	EXPECT_DOUBLE_EQ(config.reconnect_backoff_multiplier, 2.0);
	EXPECT_EQ(config.max_reconnect_delay.count(), 30000);
}

TEST(WebSocketTransportConfigTest, InheritedDefaults) {
	websocket_transport_config config;

	// From transport_config
	EXPECT_EQ(config.connect_timeout.count(), 10000);
	EXPECT_EQ(config.request_timeout.count(), 30000);
	EXPECT_FALSE(config.auto_reconnect);
	EXPECT_EQ(config.max_retries, 3);
	EXPECT_EQ(config.retry_delay.count(), 1000);
}

// ============================================================================
// Transport Construction Tests
// ============================================================================

TEST(WebSocketTransportTest, Construction) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	EXPECT_NO_THROW({
		websocket_transport transport(config);
	});
}

TEST(WebSocketTransportTest, InitialState) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	EXPECT_EQ(transport.get_state(), transport_state::disconnected);
	EXPECT_FALSE(transport.is_connected());
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST(WebSocketTransportTest, InitialStatistics) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);
	auto stats = transport.get_statistics();

	EXPECT_EQ(stats.messages_sent, 0);
	EXPECT_EQ(stats.messages_received, 0);
	EXPECT_EQ(stats.bytes_sent, 0);
	EXPECT_EQ(stats.bytes_received, 0);
	EXPECT_EQ(stats.errors, 0);
}

TEST(WebSocketTransportTest, ResetStatistics) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	// Statistics start at zero
	auto stats_before = transport.get_statistics();
	EXPECT_EQ(stats_before.messages_sent, 0);

	// Reset should not change anything when already zero
	transport.reset_statistics();
	auto stats_after = transport.get_statistics();
	EXPECT_EQ(stats_after.messages_sent, 0);
}

// ============================================================================
// Subscription Tests (without connection)
// ============================================================================

TEST(WebSocketTransportTest, SubscriptionsInitiallyEmpty) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);
	auto subs = transport.get_subscriptions();

	EXPECT_TRUE(subs.empty());
}

// ============================================================================
// Error Handling Tests (not connected)
// ============================================================================

TEST(WebSocketTransportTest, SendWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	message msg("test.topic");
	auto result = transport.send(msg);

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportTest, SendBinaryWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	std::vector<uint8_t> data = {0x01, 0x02, 0x03};
	auto result = transport.send_binary(data);

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportTest, SendTextWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	auto result = transport.send_text("Hello");

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportTest, PingWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	auto result = transport.ping();

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportTest, SubscribeWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	auto result = transport.subscribe("events.*");

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportTest, UnsubscribeWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	auto result = transport.unsubscribe("events.*");

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportTest, UnsubscribeAllWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	// Should succeed even when not connected (just clears local state)
	auto result = transport.unsubscribe_all();

	EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Handler Tests
// ============================================================================

TEST(WebSocketTransportTest, SetHandlers) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	// These should not throw
	EXPECT_NO_THROW({
		transport.set_message_handler([](const message&) {});
		transport.set_binary_handler([](const std::vector<uint8_t>&) {});
		transport.set_state_handler([](transport_state) {});
		transport.set_error_handler([](const std::string&) {});
		transport.set_disconnect_handler([](uint16_t, const std::string&) {});
	});
}

// ============================================================================
// Disconnect Tests
// ============================================================================

TEST(WebSocketTransportTest, DisconnectWhenNotConnected) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	// Disconnecting when not connected should succeed
	auto result = transport.disconnect();

	EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// State Transition Tests
// ============================================================================

TEST(WebSocketTransportTest, StateHandlerCalled) {
	websocket_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	websocket_transport transport(config);

	std::atomic<bool> handler_called{false};
	transport_state received_state = transport_state::disconnected;

	transport.set_state_handler([&](transport_state state) {
		handler_called = true;
		received_state = state;
	});

	// Try to connect (will fail but should trigger state change)
	auto result = transport.connect();

	// Allow some time for async state change
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// State handler should have been called at least once
	EXPECT_TRUE(handler_called.load());
}

#else  // !KCENON_WITH_NETWORK_SYSTEM

// ============================================================================
// Stub Implementation Tests
// ============================================================================

TEST(WebSocketTransportStubTest, ConnectReturnsNotSupported) {
	websocket_transport_config config;
	websocket_transport transport(config);

	auto result = transport.connect();

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportStubTest, SendReturnsNotSupported) {
	websocket_transport_config config;
	websocket_transport transport(config);

	message msg("test.topic");
	auto result = transport.send(msg);

	EXPECT_TRUE(result.is_err());
}

TEST(WebSocketTransportStubTest, IsNotConnected) {
	websocket_transport_config config;
	websocket_transport transport(config);

	EXPECT_FALSE(transport.is_connected());
	EXPECT_EQ(transport.get_state(), transport_state::disconnected);
}

TEST(WebSocketTransportStubTest, SubscriptionsEmpty) {
	websocket_transport_config config;
	websocket_transport transport(config);

	auto subs = transport.get_subscriptions();
	EXPECT_TRUE(subs.empty());
}

#endif  // KCENON_WITH_NETWORK_SYSTEM
