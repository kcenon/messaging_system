// BSD 3-Clause License
// Copyright (c) 2025, kcenon
// See the LICENSE file in the project root for full license information.

#include <kcenon/messaging/adapters/http_transport.h>
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

TEST(HttpTransportConfigTest, DefaultValues) {
	http_transport_config config;

	EXPECT_TRUE(config.host.empty());
	EXPECT_EQ(config.port, 0);
	EXPECT_EQ(config.base_path, "/api/messages");
	EXPECT_EQ(config.content_type, http_content_type::json);
	EXPECT_FALSE(config.use_ssl);
	EXPECT_TRUE(config.default_headers.empty());
	EXPECT_EQ(config.publish_endpoint, "/publish");
	EXPECT_EQ(config.subscribe_endpoint, "/subscribe");
	EXPECT_EQ(config.request_endpoint, "/request");
}

TEST(HttpTransportConfigTest, InheritedDefaults) {
	http_transport_config config;

	// From transport_config
	EXPECT_EQ(config.connect_timeout.count(), 10000);
	EXPECT_EQ(config.request_timeout.count(), 30000);
	EXPECT_FALSE(config.auto_reconnect);
	EXPECT_EQ(config.max_retries, 3);
	EXPECT_EQ(config.retry_delay.count(), 1000);
}

TEST(HttpTransportConfigTest, ContentTypes) {
	http_transport_config json_config;
	json_config.content_type = http_content_type::json;

	http_transport_config binary_config;
	binary_config.content_type = http_content_type::binary;

	http_transport_config msgpack_config;
	msgpack_config.content_type = http_content_type::msgpack;

	EXPECT_EQ(json_config.content_type, http_content_type::json);
	EXPECT_EQ(binary_config.content_type, http_content_type::binary);
	EXPECT_EQ(msgpack_config.content_type, http_content_type::msgpack);
}

// ============================================================================
// Transport Construction Tests
// ============================================================================

TEST(HttpTransportTest, Construction) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	EXPECT_NO_THROW({
		http_transport transport(config);
	});
}

TEST(HttpTransportTest, InitialState) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	EXPECT_EQ(transport.get_state(), transport_state::disconnected);
	EXPECT_FALSE(transport.is_connected());
}

// ============================================================================
// Connection Tests
// ============================================================================

TEST(HttpTransportTest, ConnectWithEmptyHost) {
	http_transport_config config;
	// host is empty

	http_transport transport(config);
	auto result = transport.connect();

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportTest, ConnectWithValidConfig) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);
	auto result = transport.connect();

	// HTTP is stateless, connect should succeed with valid config
	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(transport.get_state(), transport_state::connected);
	EXPECT_TRUE(transport.is_connected());
}

TEST(HttpTransportTest, ConnectWithDefaultPort) {
	http_transport_config config;
	config.host = "localhost";
	// port = 0 (should default to 80)

	http_transport transport(config);
	auto result = transport.connect();

	EXPECT_TRUE(result.is_ok());
}

TEST(HttpTransportTest, ConnectWithSSLDefaultPort) {
	http_transport_config config;
	config.host = "localhost";
	config.use_ssl = true;
	// port = 0 (should default to 443)

	http_transport transport(config);
	auto result = transport.connect();

	EXPECT_TRUE(result.is_ok());
}

TEST(HttpTransportTest, DoubleConnect) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	auto result1 = transport.connect();
	EXPECT_TRUE(result1.is_ok());

	auto result2 = transport.connect();
	EXPECT_TRUE(result2.is_ok());  // Should succeed (already connected)
}

// ============================================================================
// Disconnect Tests
// ============================================================================

TEST(HttpTransportTest, DisconnectWhenNotConnected) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	auto result = transport.disconnect();

	EXPECT_TRUE(result.is_ok());
}

TEST(HttpTransportTest, DisconnectAfterConnect) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	transport.connect();
	auto result = transport.disconnect();

	EXPECT_TRUE(result.is_ok());
	EXPECT_EQ(transport.get_state(), transport_state::disconnected);
	EXPECT_FALSE(transport.is_connected());
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST(HttpTransportTest, InitialStatistics) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);
	auto stats = transport.get_statistics();

	EXPECT_EQ(stats.messages_sent, 0);
	EXPECT_EQ(stats.messages_received, 0);
	EXPECT_EQ(stats.bytes_sent, 0);
	EXPECT_EQ(stats.bytes_received, 0);
	EXPECT_EQ(stats.errors, 0);
}

TEST(HttpTransportTest, ResetStatistics) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	auto stats_before = transport.get_statistics();
	EXPECT_EQ(stats_before.messages_sent, 0);

	transport.reset_statistics();
	auto stats_after = transport.get_statistics();
	EXPECT_EQ(stats_after.messages_sent, 0);
}

// ============================================================================
// Error Handling Tests (not connected)
// ============================================================================

TEST(HttpTransportTest, SendWhenNotConnected) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	message msg("test.topic");
	auto result = transport.send(msg);

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportTest, SendBinaryWhenNotConnected) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	std::vector<uint8_t> data = {0x01, 0x02, 0x03};
	auto result = transport.send_binary(data);

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportTest, PostWhenNotConnected) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	message msg("test.topic");
	auto result = transport.post("/endpoint", msg);

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportTest, GetWhenNotConnected) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	auto result = transport.get("/endpoint");

	EXPECT_TRUE(result.is_err());
}

// ============================================================================
// Header Management Tests
// ============================================================================

TEST(HttpTransportTest, SetHeader) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	// Should not throw
	EXPECT_NO_THROW({
		transport.set_header("Authorization", "Bearer token123");
		transport.set_header("X-Custom-Header", "custom-value");
	});
}

TEST(HttpTransportTest, RemoveHeader) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	transport.set_header("Authorization", "Bearer token123");

	// Should not throw
	EXPECT_NO_THROW({
		transport.remove_header("Authorization");
		transport.remove_header("NonExistentHeader");  // Should not throw
	});
}

// ============================================================================
// Handler Tests
// ============================================================================

TEST(HttpTransportTest, SetHandlers) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	// These should not throw
	EXPECT_NO_THROW({
		transport.set_message_handler([](const message&) {});
		transport.set_binary_handler([](const std::vector<uint8_t>&) {});
		transport.set_state_handler([](transport_state) {});
		transport.set_error_handler([](const std::string&) {});
	});
}

// ============================================================================
// State Transition Tests
// ============================================================================

TEST(HttpTransportTest, StateHandlerCalledOnConnect) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);

	std::vector<transport_state> state_changes;
	transport.set_state_handler([&](transport_state state) {
		state_changes.push_back(state);
	});

	transport.connect();

	// Should have connecting and connected states
	EXPECT_FALSE(state_changes.empty());
	EXPECT_NE(std::find(state_changes.begin(), state_changes.end(),
						transport_state::connecting),
			  state_changes.end());
	EXPECT_NE(std::find(state_changes.begin(), state_changes.end(),
						transport_state::connected),
			  state_changes.end());
}

TEST(HttpTransportTest, StateHandlerCalledOnDisconnect) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;

	http_transport transport(config);
	transport.connect();

	bool disconnected_called = false;
	transport.set_state_handler([&](transport_state state) {
		if (state == transport_state::disconnected) {
			disconnected_called = true;
		}
	});

	transport.disconnect();

	// Should have disconnected state
	EXPECT_TRUE(disconnected_called);
}

// ============================================================================
// Default Headers Tests
// ============================================================================

TEST(HttpTransportTest, DefaultHeadersFromConfig) {
	http_transport_config config;
	config.host = "localhost";
	config.port = 8080;
	config.default_headers["Authorization"] = "Bearer default-token";
	config.default_headers["X-API-Version"] = "v1";

	http_transport transport(config);

	// Headers should be stored in transport
	// (Verification would require sending actual request)
	EXPECT_NO_THROW({
		transport.connect();
	});
}

#else  // !KCENON_WITH_NETWORK_SYSTEM

// ============================================================================
// Stub Implementation Tests
// ============================================================================

TEST(HttpTransportStubTest, ConnectReturnsNotSupported) {
	http_transport_config config;
	http_transport transport(config);

	auto result = transport.connect();

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportStubTest, SendReturnsNotSupported) {
	http_transport_config config;
	http_transport transport(config);

	message msg("test.topic");
	auto result = transport.send(msg);

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportStubTest, PostReturnsNotSupported) {
	http_transport_config config;
	http_transport transport(config);

	message msg("test.topic");
	auto result = transport.post("/endpoint", msg);

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportStubTest, GetReturnsNotSupported) {
	http_transport_config config;
	http_transport transport(config);

	auto result = transport.get("/endpoint");

	EXPECT_TRUE(result.is_err());
}

TEST(HttpTransportStubTest, IsNotConnected) {
	http_transport_config config;
	http_transport transport(config);

	EXPECT_FALSE(transport.is_connected());
	EXPECT_EQ(transport.get_state(), transport_state::disconnected);
}

TEST(HttpTransportStubTest, StatisticsEmpty) {
	http_transport_config config;
	http_transport transport(config);

	auto stats = transport.get_statistics();
	EXPECT_EQ(stats.messages_sent, 0);
	EXPECT_EQ(stats.messages_received, 0);
	EXPECT_EQ(stats.bytes_sent, 0);
	EXPECT_EQ(stats.bytes_received, 0);
	EXPECT_EQ(stats.errors, 0);
}

#endif  // KCENON_WITH_NETWORK_SYSTEM
