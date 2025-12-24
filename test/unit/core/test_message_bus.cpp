#include <gtest/gtest.h>
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message.h>
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/backends/integration_backend.h>
#include <kcenon/messaging/error/error_codes.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace kcenon::messaging;
using namespace kcenon::common;

namespace {

/**
 * @brief Wait for a condition with timeout using condition variable
 */
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
// Use namespace alias to avoid ambiguity with common::error
namespace msg_error = kcenon::messaging::error;

class MessageBusTest : public ::testing::Test {
protected:
	void SetUp() override {
		backend_ = std::make_shared<standalone_backend>(2);

		message_bus_config config;
		config.queue_capacity = 100;
		config.worker_threads = 2;
		config.enable_priority_queue = true;
		config.enable_dead_letter_queue = true;
		config.enable_metrics = true;

		bus_ = std::make_unique<message_bus>(backend_, config);
	}

	void TearDown() override {
		if (bus_ && bus_->is_running()) {
			bus_->stop();
		}
	}

	std::shared_ptr<standalone_backend> backend_;
	std::unique_ptr<message_bus> bus_;
};

// Lifecycle tests
TEST_F(MessageBusTest, InitiallyNotRunning) {
	EXPECT_FALSE(bus_->is_running());
}

TEST_F(MessageBusTest, StartStop) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());
	EXPECT_TRUE(bus_->is_running());

	auto stop_result = bus_->stop();
	ASSERT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(bus_->is_running());
}

TEST_F(MessageBusTest, CannotStartTwice) {
	auto result1 = bus_->start();
	ASSERT_TRUE(result1.is_ok());

	auto result2 = bus_->start();
	EXPECT_TRUE(result2.is_err());
	EXPECT_EQ(result2.error().code, msg_error::already_running);

	bus_->stop();
}

TEST_F(MessageBusTest, CannotStopWhenNotRunning) {
	auto result = bus_->stop();
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(result.error().code, msg_error::not_running);
}

// Publishing tests
TEST_F(MessageBusTest, PublishWhenNotRunning) {
	message msg("test.topic");
	auto result = bus_->publish(msg);
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(result.error().code, msg_error::not_running);
}

TEST_F(MessageBusTest, PublishSuccess) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	message msg("test.topic");
	auto result = bus_->publish(msg);
	EXPECT_TRUE(result.is_ok());

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);

	bus_->stop();
}

TEST_F(MessageBusTest, PublishWithTopicParameter) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	message msg;
	auto result = bus_->publish("test.topic", std::move(msg));
	EXPECT_TRUE(result.is_ok());

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);

	bus_->stop();
}

// Subscription tests
TEST_F(MessageBusTest, SubscribeSuccess) {
	auto result = bus_->subscribe(
		"test.topic",
		[](const message& /* msg */) { return ::kcenon::common::ok(); }
	);
	EXPECT_TRUE(result.is_ok());
}

TEST_F(MessageBusTest, UnsubscribeSuccess) {
	auto sub_result = bus_->subscribe(
		"test.topic",
		[](const message& /* msg */) { return ::kcenon::common::ok(); }
	);
	ASSERT_TRUE(sub_result.is_ok());

	uint64_t sub_id = sub_result.unwrap();
	auto unsub_result = bus_->unsubscribe(sub_id);
	EXPECT_TRUE(unsub_result.is_ok());
}

TEST_F(MessageBusTest, UnsubscribeInvalidId) {
	auto result = bus_->unsubscribe(9999);
	EXPECT_TRUE(result.is_err());
	EXPECT_EQ(result.error().code, msg_error::subscription_not_found);
}

// Pub/Sub integration tests
TEST_F(MessageBusTest, PubSubBasic) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish message
	message msg("test.topic");
	auto pub_result = bus_->publish(msg);
	ASSERT_TRUE(pub_result.is_ok());

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 1; },
		std::chrono::milliseconds(200)
	));

	EXPECT_EQ(received_count.load(), 1);

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);
	EXPECT_EQ(stats.messages_processed, 1);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubMultipleMessages) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish multiple messages
	for (int i = 0; i < 10; ++i) {
		message msg("test.topic");
		auto pub_result = bus_->publish(msg);
		ASSERT_TRUE(pub_result.is_ok());
	}

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 10; },
		std::chrono::milliseconds(500)
	));

	EXPECT_EQ(received_count.load(), 10);

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 10);
	EXPECT_EQ(stats.messages_processed, 10);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubMultipleSubscribers) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> subscriber1_count{0};
	std::atomic<int> subscriber2_count{0};

	auto sub1_result = bus_->subscribe(
		"test.topic",
		[&subscriber1_count](const message& /* msg */) {
			subscriber1_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub1_result.is_ok());

	auto sub2_result = bus_->subscribe(
		"test.topic",
		[&subscriber2_count](const message& /* msg */) {
			subscriber2_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub2_result.is_ok());

	// Publish message
	message msg("test.topic");
	auto pub_result = bus_->publish(msg);
	ASSERT_TRUE(pub_result.is_ok());

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&subscriber1_count, &subscriber2_count]() {
			return subscriber1_count.load() >= 1 && subscriber2_count.load() >= 1;
		},
		std::chrono::milliseconds(200)
	));

	EXPECT_EQ(subscriber1_count.load(), 1);
	EXPECT_EQ(subscriber2_count.load(), 1);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubWithWildcard) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.*",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish messages with different topics
	message msg1("test.topic1");
	bus_->publish(msg1);

	message msg2("test.topic2");
	bus_->publish(msg2);

	message msg3("other.topic");
	bus_->publish(msg3);

	// Wait for processing - should receive 2 messages matching "test.*"
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 2; },
		std::chrono::milliseconds(300)
	));

	// Should receive only messages matching "test.*"
	EXPECT_EQ(received_count.load(), 2);

	bus_->stop();
}

TEST_F(MessageBusTest, PubSubWithFilter) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		},
		[](const message& msg) {
			// Only accept high priority messages
			return msg.metadata().priority == message_priority::high;
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish normal priority message
	message msg1("test.topic");
	msg1.metadata().priority = message_priority::normal;
	bus_->publish(msg1);

	// Publish high priority message
	message msg2("test.topic");
	msg2.metadata().priority = message_priority::high;
	bus_->publish(msg2);

	// Wait for processing - should receive 1 high priority message
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 1; },
		std::chrono::milliseconds(300)
	));

	// Should receive only high priority message
	EXPECT_EQ(received_count.load(), 1);

	bus_->stop();
}

// Statistics tests
TEST_F(MessageBusTest, Statistics) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	auto stats_before = bus_->get_statistics();
	EXPECT_EQ(stats_before.messages_published, 0);
	EXPECT_EQ(stats_before.messages_processed, 0);
	EXPECT_EQ(stats_before.messages_failed, 0);
	EXPECT_EQ(stats_before.messages_dropped, 0);

	// Subscribe
	auto sub_result = bus_->subscribe(
		"test.topic",
		[](const message& /* msg */) { return ::kcenon::common::ok(); }
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish
	message msg("test.topic");
	bus_->publish(msg);

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[this]() { return bus_->get_statistics().messages_processed >= 1; },
		std::chrono::milliseconds(200)
	));

	auto stats_after = bus_->get_statistics();
	EXPECT_EQ(stats_after.messages_published, 1);
	EXPECT_EQ(stats_after.messages_processed, 1);

	bus_->stop();
}

TEST_F(MessageBusTest, ResetStatistics) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	// Publish some messages
	for (int i = 0; i < 5; ++i) {
		message msg("test.topic");
		bus_->publish(msg);
	}

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 5);

	// Reset
	bus_->reset_statistics();

	auto stats_after = bus_->get_statistics();
	EXPECT_EQ(stats_after.messages_published, 0);
	EXPECT_EQ(stats_after.messages_processed, 0);
	EXPECT_EQ(stats_after.messages_failed, 0);
	EXPECT_EQ(stats_after.messages_dropped, 0);

	bus_->stop();
}

// ============================================================================
// Transport Integration Tests
// ============================================================================

// Mock transport for testing
class mock_transport : public kcenon::messaging::adapters::transport_interface {
public:
	mock_transport() = default;

	kcenon::common::VoidResult connect() override {
		connected_ = true;
		state_ = kcenon::messaging::adapters::transport_state::connected;
		if (state_handler_) state_handler_(state_);
		return kcenon::common::ok();
	}

	kcenon::common::VoidResult disconnect() override {
		connected_ = false;
		state_ = kcenon::messaging::adapters::transport_state::disconnected;
		if (state_handler_) state_handler_(state_);
		return kcenon::common::ok();
	}

	bool is_connected() const override { return connected_; }

	kcenon::messaging::adapters::transport_state get_state() const override {
		return state_;
	}

	kcenon::common::VoidResult send(const message& msg) override {
		sent_messages_.push_back(msg);
		return kcenon::common::ok();
	}

	kcenon::common::VoidResult send_binary(const std::vector<uint8_t>& /* data */) override {
		return kcenon::common::ok();
	}

	void set_message_handler(std::function<void(const message&)> handler) override {
		message_handler_ = handler;
	}

	void set_binary_handler(std::function<void(const std::vector<uint8_t>&)> handler) override {
		binary_handler_ = handler;
	}

	void set_state_handler(std::function<void(kcenon::messaging::adapters::transport_state)> handler) override {
		state_handler_ = handler;
	}

	void set_error_handler(std::function<void(const std::string&)> handler) override {
		error_handler_ = handler;
	}

	kcenon::messaging::adapters::transport_statistics get_statistics() const override {
		return stats_;
	}

	void reset_statistics() override {
		stats_ = {};
	}

	// Test helpers
	void simulate_incoming_message(const message& msg) {
		if (message_handler_) {
			message_handler_(msg);
		}
	}

	const std::vector<message>& get_sent_messages() const { return sent_messages_; }

private:
	bool connected_ = false;
	kcenon::messaging::adapters::transport_state state_ = kcenon::messaging::adapters::transport_state::disconnected;
	std::function<void(const message&)> message_handler_;
	std::function<void(const std::vector<uint8_t>&)> binary_handler_;
	std::function<void(kcenon::messaging::adapters::transport_state)> state_handler_;
	std::function<void(const std::string&)> error_handler_;
	std::vector<message> sent_messages_;
	kcenon::messaging::adapters::transport_statistics stats_;
};

class MessageBusTransportTest : public ::testing::Test {
protected:
	void SetUp() override {
		backend_ = std::make_shared<standalone_backend>(2);
		mock_transport_ = std::make_shared<mock_transport>();
	}

	std::shared_ptr<standalone_backend> backend_;
	std::shared_ptr<mock_transport> mock_transport_;
};

TEST_F(MessageBusTransportTest, LocalModeDefault) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;

	auto bus = std::make_unique<message_bus>(backend_, config);
	EXPECT_EQ(bus->get_transport_mode(), transport_mode::local);
	EXPECT_FALSE(bus->has_transport());
}

TEST_F(MessageBusTransportTest, ConfigureWithTransport) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;
	config.mode = transport_mode::hybrid;
	config.transport = mock_transport_;

	auto bus = std::make_unique<message_bus>(backend_, config);
	EXPECT_EQ(bus->get_transport_mode(), transport_mode::hybrid);
	EXPECT_TRUE(bus->has_transport());
}

TEST_F(MessageBusTransportTest, TransportConnectOnStart) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;
	config.mode = transport_mode::remote;
	config.transport = mock_transport_;

	auto bus = std::make_unique<message_bus>(backend_, config);
	EXPECT_FALSE(bus->is_transport_connected());

	auto result = bus->start();
	ASSERT_TRUE(result.is_ok());
	EXPECT_TRUE(bus->is_transport_connected());

	bus->stop();
	EXPECT_FALSE(bus->is_transport_connected());
}

TEST_F(MessageBusTransportTest, RemoteModePublish) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;
	config.mode = transport_mode::remote;
	config.transport = mock_transport_;

	auto bus = std::make_unique<message_bus>(backend_, config);
	auto result = bus->start();
	ASSERT_TRUE(result.is_ok());

	message msg("test.topic");
	auto pub_result = bus->publish(msg);
	EXPECT_TRUE(pub_result.is_ok());

	// Check that message was sent via transport
	EXPECT_EQ(mock_transport_->get_sent_messages().size(), 1);

	auto stats = bus->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);
	EXPECT_EQ(stats.messages_sent_remote, 1);

	bus->stop();
}

TEST_F(MessageBusTransportTest, HybridModePublish) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;
	config.mode = transport_mode::hybrid;
	config.transport = mock_transport_;

	auto bus = std::make_unique<message_bus>(backend_, config);
	auto result = bus->start();
	ASSERT_TRUE(result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus->subscribe(
		"test.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	message msg("test.topic");
	auto pub_result = bus->publish(msg);
	EXPECT_TRUE(pub_result.is_ok());

	// Wait for local processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 1; },
		std::chrono::milliseconds(200)
	));

	// Check both local and remote
	EXPECT_EQ(received_count.load(), 1);
	EXPECT_EQ(mock_transport_->get_sent_messages().size(), 1);

	auto stats = bus->get_statistics();
	EXPECT_EQ(stats.messages_published, 1);
	EXPECT_EQ(stats.messages_sent_remote, 1);
	EXPECT_EQ(stats.messages_processed, 1);

	bus->stop();
}

TEST_F(MessageBusTransportTest, ReceiveRemoteMessage) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;
	config.mode = transport_mode::hybrid;
	config.transport = mock_transport_;

	auto bus = std::make_unique<message_bus>(backend_, config);
	auto result = bus->start();
	ASSERT_TRUE(result.is_ok());

	std::atomic<int> received_count{0};
	auto sub_result = bus->subscribe(
		"remote.topic",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Simulate incoming remote message
	message remote_msg("remote.topic");
	mock_transport_->simulate_incoming_message(remote_msg);

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 1; },
		std::chrono::milliseconds(200)
	));

	EXPECT_EQ(received_count.load(), 1);

	auto stats = bus->get_statistics();
	EXPECT_EQ(stats.messages_received_remote, 1);
	EXPECT_EQ(stats.messages_processed, 1);

	bus->stop();
}

TEST_F(MessageBusTransportTest, StatisticsIncludeRemote) {
	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;
	config.mode = transport_mode::remote;
	config.transport = mock_transport_;

	auto bus = std::make_unique<message_bus>(backend_, config);
	auto result = bus->start();
	ASSERT_TRUE(result.is_ok());

	for (int i = 0; i < 5; ++i) {
		message msg("test.topic");
		bus->publish(msg);
	}

	auto stats = bus->get_statistics();
	EXPECT_EQ(stats.messages_published, 5);
	EXPECT_EQ(stats.messages_sent_remote, 5);

	bus->reset_statistics();

	auto stats_after = bus->get_statistics();
	EXPECT_EQ(stats_after.messages_sent_remote, 0);
	EXPECT_EQ(stats_after.messages_received_remote, 0);

	bus->stop();
}

// ============================================================================
// Executor Integration Tests
// ============================================================================

class MessageBusExecutorTest : public ::testing::Test {
protected:
	void SetUp() override {
		// Use standalone_backend which provides executor via thread_pool
		backend_ = std::make_shared<standalone_backend>(4);

		message_bus_config config;
		config.queue_capacity = 100;
		config.worker_threads = 2;
		config.enable_priority_queue = true;
		config.enable_dead_letter_queue = true;
		config.enable_metrics = true;

		bus_ = std::make_unique<message_bus>(backend_, config);
	}

	void TearDown() override {
		if (bus_ && bus_->is_running()) {
			bus_->stop();
		}
	}

	std::shared_ptr<standalone_backend> backend_;
	std::unique_ptr<message_bus> bus_;
};

TEST_F(MessageBusExecutorTest, WorkersUseExecutorWhenAvailable) {
	// Start the message bus - workers should use executor
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());
	EXPECT_TRUE(bus_->is_running());

	// Publish and verify processing works with executor-based workers
	std::atomic<int> received_count{0};
	auto sub_result = bus_->subscribe(
		"executor.test",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);
	ASSERT_TRUE(sub_result.is_ok());

	// Publish multiple messages to verify executor workers process them
	for (int i = 0; i < 10; ++i) {
		message msg("executor.test");
		auto pub_result = bus_->publish(msg);
		ASSERT_TRUE(pub_result.is_ok());
	}

	// Wait for processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 10; },
		std::chrono::milliseconds(500)
	));

	EXPECT_EQ(received_count.load(), 10);

	auto stats = bus_->get_statistics();
	EXPECT_EQ(stats.messages_published, 10);
	EXPECT_EQ(stats.messages_processed, 10);

	bus_->stop();
}

TEST_F(MessageBusExecutorTest, GracefulShutdownWithExecutor) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	// Subscribe and publish messages
	std::atomic<int> received_count{0};
	bus_->subscribe(
		"shutdown.test",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);

	for (int i = 0; i < 5; ++i) {
		message msg("shutdown.test");
		bus_->publish(msg);
	}

	// Wait briefly for some processing
	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 5; },
		std::chrono::milliseconds(300)
	));

	// Stop should complete gracefully
	auto stop_result = bus_->stop();
	ASSERT_TRUE(stop_result.is_ok());
	EXPECT_FALSE(bus_->is_running());
}

TEST_F(MessageBusExecutorTest, ConcurrentProcessingWithExecutor) {
	auto start_result = bus_->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> total_received{0};
	std::atomic<int> topic1_count{0};
	std::atomic<int> topic2_count{0};

	// Subscribe to multiple topics
	bus_->subscribe(
		"concurrent.topic1",
		[&topic1_count, &total_received](const message& /* msg */) {
			topic1_count++;
			total_received++;
			return ::kcenon::common::ok();
		}
	);

	bus_->subscribe(
		"concurrent.topic2",
		[&topic2_count, &total_received](const message& /* msg */) {
			topic2_count++;
			total_received++;
			return ::kcenon::common::ok();
		}
	);

	// Publish to both topics concurrently
	for (int i = 0; i < 20; ++i) {
		message msg1("concurrent.topic1");
		message msg2("concurrent.topic2");
		bus_->publish(msg1);
		bus_->publish(msg2);
	}

	// Wait for all messages to be processed
	ASSERT_TRUE(wait_for_condition(
		[&total_received]() { return total_received.load() >= 40; },
		std::chrono::milliseconds(1000)
	));

	EXPECT_EQ(topic1_count.load(), 20);
	EXPECT_EQ(topic2_count.load(), 20);
	EXPECT_EQ(total_received.load(), 40);

	bus_->stop();
}

// Test with integration_backend to verify external executor support
TEST(MessageBusIntegrationBackendTest, WorksWithExternalExecutor) {
	// Create a standalone backend to get its executor
	auto standalone = std::make_shared<standalone_backend>(2);
	auto init_result = standalone->initialize();
	ASSERT_TRUE(init_result.is_ok());

	auto executor = standalone->get_executor();
	ASSERT_NE(executor, nullptr);

	// Create integration_backend with the executor
	auto integration = std::make_shared<kcenon::messaging::integration_backend>(executor);

	message_bus_config config;
	config.queue_capacity = 100;
	config.worker_threads = 2;

	auto bus = std::make_unique<message_bus>(integration, config);

	auto start_result = bus->start();
	ASSERT_TRUE(start_result.is_ok());

	std::atomic<int> received_count{0};
	bus->subscribe(
		"integration.test",
		[&received_count](const message& /* msg */) {
			received_count++;
			return ::kcenon::common::ok();
		}
	);

	for (int i = 0; i < 5; ++i) {
		message msg("integration.test");
		bus->publish(msg);
	}

	ASSERT_TRUE(wait_for_condition(
		[&received_count]() { return received_count.load() >= 5; },
		std::chrono::milliseconds(300)
	));

	EXPECT_EQ(received_count.load(), 5);

	bus->stop();
	standalone->shutdown();
}
