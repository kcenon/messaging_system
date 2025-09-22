/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file test_thread_context_simple.cpp
 * @brief Simple unit tests for thread context and metadata
 */

#include <gtest/gtest.h>
#include <kcenon/monitoring/context/thread_context.h>

using namespace monitoring_system;

/**
 * Test context_metadata basic functionality
 */
TEST(ThreadContextSimpleTest, ContextMetadataBasics) {
    // Test constructor
    context_metadata metadata("test-request");
    EXPECT_EQ(metadata.request_id, "test-request");
    EXPECT_TRUE(metadata.correlation_id.empty());
    EXPECT_TRUE(metadata.user_id.empty());
    EXPECT_FALSE(metadata.empty()); // Has request_id, so not empty

    // Test set_tag and get_tag
    metadata.set_tag("environment", "test");
    metadata.set_tag("version", "1.0.0");

    EXPECT_EQ(metadata.get_tag("environment"), "test");
    EXPECT_EQ(metadata.get_tag("version"), "1.0.0");
    EXPECT_EQ(metadata.get_tag("nonexistent"), ""); // Returns empty string

    // Test setting other fields
    metadata.correlation_id = "corr-123";
    metadata.user_id = "user-456";

    EXPECT_EQ(metadata.correlation_id, "corr-123");
    EXPECT_EQ(metadata.user_id, "user-456");
}

/**
 * Test context_metadata empty functionality
 */
TEST(ThreadContextSimpleTest, ContextMetadataEmpty) {
    // Empty constructor
    context_metadata empty_metadata;
    EXPECT_TRUE(empty_metadata.empty());
    EXPECT_TRUE(empty_metadata.request_id.empty());
    EXPECT_TRUE(empty_metadata.correlation_id.empty());
    EXPECT_TRUE(empty_metadata.user_id.empty());
    EXPECT_TRUE(empty_metadata.tags.empty());

    // Add a tag - should not be empty anymore
    empty_metadata.set_tag("test", "value");
    EXPECT_FALSE(empty_metadata.empty());
}

/**
 * Test thread_context_data structure
 */
TEST(ThreadContextSimpleTest, ThreadContextData) {
    thread_context_data data;

    // Check default values
    EXPECT_TRUE(data.request_id.empty());
    EXPECT_TRUE(data.correlation_id.empty());
    EXPECT_TRUE(data.user_id.empty());
    EXPECT_TRUE(data.span_id.empty());
    EXPECT_TRUE(data.trace_id.empty());
    EXPECT_FALSE(data.parent_span_id.has_value());
    EXPECT_TRUE(data.tags.empty());

    // Set values
    data.request_id = "req-123";
    data.trace_id = "trace-456";
    data.span_id = "span-789";
    data.parent_span_id = "parent-span-101";
    data.tags["env"] = "test";

    EXPECT_EQ(data.request_id, "req-123");
    EXPECT_EQ(data.trace_id, "trace-456");
    EXPECT_EQ(data.span_id, "span-789");
    EXPECT_TRUE(data.parent_span_id.has_value());
    EXPECT_EQ(data.parent_span_id.value(), "parent-span-101");
    EXPECT_EQ(data.tags["env"], "test");
}