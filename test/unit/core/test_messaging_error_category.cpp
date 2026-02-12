#include <kcenon/messaging/error/messaging_error_category.h>
#include <kcenon/common/patterns/result_helpers.h>
#include <gtest/gtest.h>

namespace msg = kcenon::messaging;
namespace cmn = kcenon::common;

// =============================================================================
// Singleton Tests
// =============================================================================

TEST(MessagingErrorCategoryTest, SingletonIdentity) {
    const auto& cat1 = msg::messaging_error_category::instance();
    const auto& cat2 = msg::messaging_error_category::instance();

    EXPECT_EQ(&cat1, &cat2);
}

TEST(MessagingErrorCategoryTest, CategoryName) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.name(), "messaging");
}

// =============================================================================
// Message Lookup Tests
// =============================================================================

TEST(MessagingErrorCategoryTest, MessageErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::invalid_message), "Invalid message");
    EXPECT_EQ(cat.message(msg::error::message_too_large), "Message too large");
    EXPECT_EQ(cat.message(msg::error::message_expired), "Message expired");
    EXPECT_EQ(cat.message(msg::error::invalid_payload), "Invalid message payload");
    EXPECT_EQ(cat.message(msg::error::message_serialization_failed), "Message serialization failed");
    EXPECT_EQ(cat.message(msg::error::message_deserialization_failed), "Message deserialization failed");
}

TEST(MessagingErrorCategoryTest, TaskErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::task_not_found), "Task not found");
    EXPECT_EQ(cat.message(msg::error::task_already_running), "Task already running");
    EXPECT_EQ(cat.message(msg::error::task_timeout), "Task timeout");
    EXPECT_EQ(cat.message(msg::error::task_failed), "Task execution failed");
}

TEST(MessagingErrorCategoryTest, RoutingErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::routing_failed), "Message routing failed");
    EXPECT_EQ(cat.message(msg::error::unknown_topic), "Unknown topic");
    EXPECT_EQ(cat.message(msg::error::no_subscribers), "No subscribers for topic");
}

TEST(MessagingErrorCategoryTest, QueueErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::queue_full), "Message queue full");
    EXPECT_EQ(cat.message(msg::error::queue_empty), "Message queue empty");
    EXPECT_EQ(cat.message(msg::error::queue_stopped), "Message queue stopped");
    EXPECT_EQ(cat.message(msg::error::dlq_full), "Dead letter queue full");
    EXPECT_EQ(cat.message(msg::error::dlq_not_configured), "Dead letter queue not configured");
}

TEST(MessagingErrorCategoryTest, SubscriptionErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::subscription_failed), "Subscription failed");
    EXPECT_EQ(cat.message(msg::error::duplicate_subscription), "Duplicate subscription");
}

TEST(MessagingErrorCategoryTest, PublishingErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::publication_failed), "Publication failed");
    EXPECT_EQ(cat.message(msg::error::broker_unavailable), "Message broker unavailable");
    EXPECT_EQ(cat.message(msg::error::not_running), "Message bus not running");
    EXPECT_EQ(cat.message(msg::error::not_supported), "Feature not supported (requires optional dependency)");
}

TEST(MessagingErrorCategoryTest, TransportErrorCodes) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(msg::error::connection_failed), "Connection failed");
    EXPECT_EQ(cat.message(msg::error::send_timeout), "Send operation timed out");
    EXPECT_EQ(cat.message(msg::error::authentication_failed), "Authentication failed");
    EXPECT_EQ(cat.message(msg::error::not_connected), "Transport not connected");
}

TEST(MessagingErrorCategoryTest, UnknownCode) {
    const auto& cat = msg::messaging_error_category::instance();

    EXPECT_EQ(cat.message(0), "Unknown messaging error");
    EXPECT_EQ(cat.message(-999), "Unknown messaging error");
}

// =============================================================================
// typed_error_code Integration Tests
// =============================================================================

TEST(MessagingErrorCategoryTest, MakeMessagingErrorCode) {
    auto ec = msg::make_messaging_error_code(msg::error::queue_full);

    EXPECT_EQ(ec.value(), msg::error::queue_full);
    EXPECT_EQ(ec.category().name(), "messaging");
    EXPECT_EQ(ec.message(), "Message queue full");
    EXPECT_TRUE(static_cast<bool>(ec));
}

TEST(MessagingErrorCategoryTest, MakeTypedErrorCodeTemplate) {
    auto ec = cmn::make_typed_error_code<msg::messaging_error_category>(
        msg::error::routing_failed);

    EXPECT_EQ(ec.value(), msg::error::routing_failed);
    EXPECT_EQ(ec.category().name(), "messaging");
    EXPECT_EQ(ec.message(), "Message routing failed");
}

TEST(MessagingErrorCategoryTest, CategoryComparison) {
    auto messaging_ec = msg::make_messaging_error_code(msg::error::queue_full);
    auto common_ec = cmn::make_typed_error_code(cmn::common_error_category::timeout);

    // Different categories
    EXPECT_NE(messaging_ec.category(), common_ec.category());

    // Same category
    auto messaging_ec2 = msg::make_messaging_error_code(msg::error::queue_empty);
    EXPECT_EQ(messaging_ec.category(), messaging_ec2.category());
}

TEST(MessagingErrorCategoryTest, ErrorCodeEquality) {
    auto ec1 = msg::make_messaging_error_code(msg::error::queue_full);
    auto ec2 = msg::make_messaging_error_code(msg::error::queue_full);
    auto ec3 = msg::make_messaging_error_code(msg::error::queue_empty);

    EXPECT_EQ(ec1, ec2);
    EXPECT_NE(ec1, ec3);
}

TEST(MessagingErrorCategoryTest, ResultIntegration) {
    auto ec = msg::make_messaging_error_code(msg::error::subscription_failed);
    auto result = cmn::Result<int>::err(ec);

    EXPECT_FALSE(result.is_ok());
}
