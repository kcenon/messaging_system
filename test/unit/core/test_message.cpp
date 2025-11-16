#include <kcenon/messaging/core/message.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

using namespace kcenon::messaging;

// Test message construction
TEST(MessageTest, DefaultConstruction) {
	message msg;

	EXPECT_FALSE(msg.metadata().id.empty());
	EXPECT_EQ(msg.metadata().type, message_type::event);
	EXPECT_EQ(msg.metadata().priority, message_priority::normal);
	EXPECT_TRUE(msg.metadata().topic.empty());
}

TEST(MessageTest, TopicConstruction) {
	message msg("test.topic");

	EXPECT_EQ(msg.metadata().topic, "test.topic");
	EXPECT_EQ(msg.metadata().type, message_type::event);
	EXPECT_FALSE(msg.metadata().id.empty());
}

TEST(MessageTest, TypeConstruction) {
	message msg("test.topic", message_type::command);

	EXPECT_EQ(msg.metadata().topic, "test.topic");
	EXPECT_EQ(msg.metadata().type, message_type::command);
}

// Test message expiration
TEST(MessageTest, NoExpiration) {
	message msg;
	EXPECT_FALSE(msg.is_expired());
}

TEST(MessageTest, NotExpiredYet) {
	message msg;
	msg.metadata().ttl = std::chrono::milliseconds(1000);
	EXPECT_FALSE(msg.is_expired());
}

TEST(MessageTest, ExpiredMessage) {
	message msg;
	msg.metadata().ttl = std::chrono::milliseconds(10);
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	EXPECT_TRUE(msg.is_expired());
}

// Test message age
TEST(MessageTest, MessageAge) {
	message msg;
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	auto age = msg.age();
	EXPECT_GE(age.count(), 10);
}

// Test message builder
TEST(MessageBuilderTest, BasicBuild) {
	auto result = message_builder()
					  .topic("test.topic")
					  .type(message_type::command)
					  .priority(message_priority::high)
					  .build();

	ASSERT_FALSE(result.is_err());
	auto msg = result.unwrap();

	EXPECT_EQ(msg.metadata().topic, "test.topic");
	EXPECT_EQ(msg.metadata().type, message_type::command);
	EXPECT_EQ(msg.metadata().priority, message_priority::high);
}

TEST(MessageBuilderTest, WithMetadata) {
	auto result = message_builder()
					  .topic("test.topic")
					  .source("service.a")
					  .target("service.b")
					  .correlation_id("corr-123")
					  .trace_id("trace-456")
					  .ttl(std::chrono::milliseconds(5000))
					  .build();

	ASSERT_FALSE(result.is_err());
	auto msg = result.unwrap();

	EXPECT_EQ(msg.metadata().source, "service.a");
	EXPECT_EQ(msg.metadata().target, "service.b");
	EXPECT_EQ(msg.metadata().correlation_id, "corr-123");
	EXPECT_EQ(msg.metadata().trace_id, "trace-456");
	ASSERT_TRUE(msg.metadata().ttl.has_value());
	EXPECT_EQ(msg.metadata().ttl.value().count(), 5000);
}

TEST(MessageBuilderTest, WithHeaders) {
	auto result = message_builder()
					  .topic("test.topic")
					  .header("key1", "value1")
					  .header("key2", "value2")
					  .build();

	ASSERT_FALSE(result.is_err());
	auto msg = result.unwrap();

	EXPECT_EQ(msg.metadata().headers.size(), 2);
	EXPECT_EQ(msg.metadata().headers.at("key1"), "value1");
	EXPECT_EQ(msg.metadata().headers.at("key2"), "value2");
}

TEST(MessageBuilderTest, EmptyTopicFails) {
	auto result = message_builder()
					  .type(message_type::command)
					  .build();

	EXPECT_TRUE(result.is_err());
}

// Test message priority levels
TEST(MessageTest, PriorityLevels) {
	EXPECT_LT(static_cast<uint8_t>(message_priority::lowest),
			  static_cast<uint8_t>(message_priority::low));
	EXPECT_LT(static_cast<uint8_t>(message_priority::low),
			  static_cast<uint8_t>(message_priority::normal));
	EXPECT_LT(static_cast<uint8_t>(message_priority::normal),
			  static_cast<uint8_t>(message_priority::high));
	EXPECT_LT(static_cast<uint8_t>(message_priority::high),
			  static_cast<uint8_t>(message_priority::highest));
	EXPECT_LT(static_cast<uint8_t>(message_priority::highest),
			  static_cast<uint8_t>(message_priority::critical));
}

// Test serialization/deserialization
TEST(MessageTest, SerializeDeserialize) {
	auto build_result = message_builder()
							.topic("test.topic")
							.source("service.a")
							.type(message_type::command)
							.build();

	ASSERT_FALSE(build_result.is_err());
	auto original = build_result.unwrap();

	auto serialize_result = original.serialize();
	ASSERT_FALSE(serialize_result.is_err());

	auto data = serialize_result.unwrap();

	// Deserialization is not fully implemented yet, but test the API
	auto deserialize_result = message::deserialize(data);
	ASSERT_FALSE(deserialize_result.is_err());
}

TEST(MessageTest, DeserializeEmptyDataFails) {
	std::vector<uint8_t> empty_data;
	auto result = message::deserialize(empty_data);
	EXPECT_TRUE(result.is_err());
}
