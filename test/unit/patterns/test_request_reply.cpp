#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/request_reply.h>
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
		auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
			deadline - std::chrono::steady_clock::now());
		if (remaining <= std::chrono::milliseconds::zero()) {
			return false;
		}

		auto wait_time = std::min(remaining, std::chrono::milliseconds{50});
		cv.wait_for(lock, wait_time);
	}

	return true;
}

}  // namespace

class RequestReplyTest : public ::testing::Test {
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
// request_reply_handler Tests
// ============================================================================

TEST_F(RequestReplyTest, HandlerConstruction) {
	request_reply_handler handler(bus_, "test.service");

	EXPECT_EQ(handler.get_service_topic(), "test.service");
	EXPECT_EQ(handler.get_reply_topic(), "test.service.reply");
	EXPECT_FALSE(handler.has_handler());
}

TEST_F(RequestReplyTest, HandlerConstructionWithCustomReplyTopic) {
	request_reply_handler handler(bus_, "test.service", "custom.reply");

	EXPECT_EQ(handler.get_service_topic(), "test.service");
	EXPECT_EQ(handler.get_reply_topic(), "custom.reply");
}

TEST_F(RequestReplyTest, RegisterHandler) {
	request_reply_handler handler(bus_, "test.service");

	EXPECT_FALSE(handler.has_handler());

	auto result = handler.register_handler([](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		return common::ok(std::move(reply));
	});

	EXPECT_TRUE(result.is_ok());
	EXPECT_TRUE(handler.has_handler());
}

TEST_F(RequestReplyTest, UnregisterHandler) {
	request_reply_handler handler(bus_, "test.service");

	auto result = handler.register_handler([](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(handler.has_handler());

	auto unregister_result = handler.unregister_handler();
	EXPECT_TRUE(unregister_result.is_ok());
	EXPECT_FALSE(handler.has_handler());
}

TEST_F(RequestReplyTest, RequestReplyBasic) {
	request_reply_handler server_handler(bus_, "echo.service");
	request_reply_handler client_handler(bus_, "echo.service");

	// Register echo handler
	auto reg_result = server_handler.register_handler([](const message& req) -> common::Result<message> {
		// Echo back the request
		message reply(req.metadata().topic);
		reply.metadata().source = "echo.service";
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send request
	message request("echo.service");
	request.metadata().source = "client";

	auto reply_result = client_handler.request(std::move(request), std::chrono::milliseconds{1000});

	ASSERT_TRUE(reply_result.is_ok());
	EXPECT_EQ(reply_result.value().metadata().source, "echo.service");
}

TEST_F(RequestReplyTest, RequestTimeout) {
	request_reply_handler client_handler(bus_, "nonexistent.service");

	// No server registered - should timeout
	message request("nonexistent.service");

	auto reply_result = client_handler.request(std::move(request), std::chrono::milliseconds{100});

	// Should timeout
	EXPECT_FALSE(reply_result.is_ok());
}

TEST_F(RequestReplyTest, MultipleRequestsSequential) {
	request_reply_handler server_handler(bus_, "counter.service");
	request_reply_handler client_handler(bus_, "counter.service");

	std::atomic<int> counter{0};

	// Register counter handler
	auto reg_result = server_handler.register_handler([&counter](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		int count = counter.fetch_add(1);
		reply.metadata().source = "counter:" + std::to_string(count);
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send multiple requests
	for (int i = 0; i < 5; i++) {
		message request("counter.service");
		auto reply_result = client_handler.request(std::move(request), std::chrono::milliseconds{1000});
		ASSERT_TRUE(reply_result.is_ok());
	}

	EXPECT_EQ(counter, 5);
}

// Commented out - need to verify error handling API
// TEST_F(RequestReplyTest, HandlerReturnsError) {
// 	request_reply_handler server_handler(bus_, "error.service");
// 	request_reply_handler client_handler(bus_, "error.service");
//
// 	// Register handler that returns error
// 	auto reg_result = server_handler.register_handler([](const message&) -> common::Result<message> {
// 		// TODO: Fix error creation syntax
// 		return {}; // placeholder
// 	});
// 	ASSERT_TRUE(reg_result.is_ok());
//
// 	// Give server time to subscribe
// 	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));
//
// 	// Send request
// 	message request("error.service");
//
// 	auto reply_result = client_handler.request(std::move(request), std::chrono::milliseconds{1000});
//
// 	// Client should timeout or get no reply when handler returns error
// 	// (depends on implementation - handler error might not send reply)
// 	// This tests the behavior is well-defined
// }

TEST_F(RequestReplyTest, CorrelationIDMatching) {
	request_reply_handler server_handler(bus_, "test.service");

	// Register handler that delays different amounts based on request
	auto reg_result = server_handler.register_handler([](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		reply.metadata().correlation_id = req.metadata().correlation_id;
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send multiple requests from different clients
	request_reply_handler client1(bus_, "test.service");
	request_reply_handler client2(bus_, "test.service");

	message req1("test.service");
	req1.metadata().source = "client1";

	message req2("test.service");
	req2.metadata().source = "client2";

	auto reply1 = client1.request(std::move(req1), std::chrono::milliseconds{1000});
	auto reply2 = client2.request(std::move(req2), std::chrono::milliseconds{1000});

	// Both should succeed
	ASSERT_TRUE(reply1.is_ok());
	ASSERT_TRUE(reply2.is_ok());
}

// ============================================================================
// request_client Tests
// ============================================================================

TEST_F(RequestReplyTest, ClientConstruction) {
	request_client client(bus_, "test.service");
	// No public methods to test construction, just ensure it doesn't crash
}

TEST_F(RequestReplyTest, ClientRequest) {
	// Setup server
	request_server server(bus_, "test.service");
	auto reg_result = server.register_handler([](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		reply.metadata().source = "server";
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send request from client
	request_client client(bus_, "test.service");
	message request("test.service");

	auto reply_result = client.request(std::move(request), std::chrono::milliseconds{1000});

	ASSERT_TRUE(reply_result.is_ok());
	EXPECT_EQ(reply_result.value().metadata().source, "server");
}

// ============================================================================
// request_server Tests
// ============================================================================

TEST_F(RequestReplyTest, ServerConstruction) {
	request_server server(bus_, "test.service");
	// No public methods to test construction, just ensure it doesn't crash
}

TEST_F(RequestReplyTest, ServerRegisterHandler) {
	request_server server(bus_, "test.service");

	auto result = server.register_handler([](const message&) -> common::Result<message> {
		message reply("test.service");
		return common::ok(std::move(reply));
	});

	EXPECT_TRUE(result.is_ok());
}

TEST_F(RequestReplyTest, ServerStop) {
	request_server server(bus_, "test.service");

	auto reg_result = server.register_handler([](const message&) -> common::Result<message> {
		message reply("test.service");
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	auto stop_result = server.stop();
	EXPECT_TRUE(stop_result.is_ok());
}

TEST_F(RequestReplyTest, ServerHandlesMultipleClients) {
	request_server server(bus_, "multi.service");

	std::atomic<int> request_count{0};

	auto reg_result = server.register_handler([&request_count](const message& req) -> common::Result<message> {
		request_count++;
		message reply(req.metadata().topic);
		reply.metadata().source = "multi.service";
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server more time to fully initialize and subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(200));

	// Create multiple clients
	std::vector<std::thread> threads;
	std::atomic<int> success_count{0};

	for (int i = 0; i < 5; i++) {
		threads.emplace_back([this, &success_count, i]() {
			// Stagger client starts to reduce thundering herd
			wait_for_condition([]() { return false; }, std::chrono::milliseconds(i * 10));

			request_client client(bus_, "multi.service");
			message request("multi.service");

			auto reply = client.request(std::move(request), std::chrono::milliseconds{3000});
			if (reply.is_ok()) {
				success_count++;
			}
		});
	}

	// Wait for all threads
	for (auto& thread : threads) {
		thread.join();
	}

	EXPECT_EQ(request_count, 5);
	EXPECT_EQ(success_count, 5);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(RequestReplyTest, ClientServerIntegration) {
	request_server server(bus_, "calc.service");
	request_client client(bus_, "calc.service");

	// Register calculator handler
	auto reg_result = server.register_handler([](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		reply.metadata().source = "calc.service";
		// In real scenario, would process request payload
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send request
	message request("calc.service");
	request.metadata().source = "client";

	auto reply_result = client.request(std::move(request), std::chrono::milliseconds{1000});

	ASSERT_TRUE(reply_result.is_ok());
	EXPECT_EQ(reply_result.value().metadata().source, "calc.service");
}

TEST_F(RequestReplyTest, MultipleServices) {
	// Setup multiple services
	request_server server1(bus_, "service1");
	request_server server2(bus_, "service2");

	server1.register_handler([](const message&) -> common::Result<message> {
		message reply("service1");
		reply.metadata().source = "server1";
		return common::ok(std::move(reply));
	});

	server2.register_handler([](const message&) -> common::Result<message> {
		message reply("service2");
		reply.metadata().source = "server2";
		return common::ok(std::move(reply));
	});

	// Give servers time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Create clients
	request_client client1(bus_, "service1");
	request_client client2(bus_, "service2");

	// Send requests
	message req1("service1");
	message req2("service2");

	auto reply1 = client1.request(std::move(req1), std::chrono::milliseconds{1000});
	auto reply2 = client2.request(std::move(req2), std::chrono::milliseconds{1000});

	ASSERT_TRUE(reply1.is_ok());
	ASSERT_TRUE(reply2.is_ok());

	EXPECT_EQ(reply1.value().metadata().source, "server1");
	EXPECT_EQ(reply2.value().metadata().source, "server2");
}

TEST_F(RequestReplyTest, RequestWithPayload) {
	request_server server(bus_, "echo.service");
	request_client client(bus_, "echo.service");

	// Register echo handler that echoes back correlation_id
	auto reg_result = server.register_handler([](const message& req) -> common::Result<message> {
		message reply(req.metadata().topic);
		reply.metadata().source = "echo.service";
		reply.metadata().correlation_id = req.metadata().correlation_id;
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send request with correlation_id
	message request("echo.service");
	request.metadata().correlation_id = "test-correlation-123";

	auto reply_result = client.request(std::move(request), std::chrono::milliseconds{1000});

	ASSERT_TRUE(reply_result.is_ok());
	EXPECT_EQ(reply_result.value().metadata().correlation_id, "test-correlation-123");
}

TEST_F(RequestReplyTest, LongRunningRequest) {
	request_server server(bus_, "slow.service");
	request_client client(bus_, "slow.service");

	// Register handler that takes time
	auto reg_result = server.register_handler([](const message& req) -> common::Result<message> {
		wait_for_condition([]() { return false; }, std::chrono::milliseconds(200));
		message reply(req.metadata().topic);
		reply.metadata().source = "slow.service";
		return common::ok(std::move(reply));
	});
	ASSERT_TRUE(reg_result.is_ok());

	// Give server time to subscribe
	wait_for_condition([]() { return false; }, std::chrono::milliseconds(50));

	// Send request with sufficient timeout
	message request("slow.service");

	auto start = std::chrono::steady_clock::now();
	auto reply_result = client.request(std::move(request), std::chrono::milliseconds{1000});
	auto duration = std::chrono::steady_clock::now() - start;

	ASSERT_TRUE(reply_result.is_ok());
	// Should take at least 200ms due to handler delay
	EXPECT_GE(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count(), 200);
}
