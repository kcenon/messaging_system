// Basic unit test for MessagingContainer
// Note: This is a minimal test. Full GTest integration pending Phase 4.

#include "messaging_system/core/messaging_container.h"
#include <iostream>
#include <cassert>

using namespace messaging;

void test_create_valid_message() {
    std::cout << "Test: Create valid message..." << std::endl;

    auto result = MessagingContainer::create("source1", "target1", "user.created");
    assert(result.is_ok() && "Should create valid message");

    auto msg = result.value();
    assert(msg.source() == "source1" && "Source should match");
    assert(msg.target() == "target1" && "Target should match");
    assert(msg.topic() == "user.created" && "Topic should match");
    assert(!msg.trace_id().empty() && "Trace ID should be generated");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_create_invalid_message() {
    std::cout << "Test: Create invalid message (empty topic)..." << std::endl;

    auto result = MessagingContainer::create("source1", "target1", "");
    assert(result.is_error() && "Should fail with empty topic");
    assert(result.error().code == error::INVALID_MESSAGE && "Error code should be INVALID_MESSAGE");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_serialize_deserialize() {
    std::cout << "Test: Serialize and deserialize..." << std::endl;

    auto create_result = MessagingContainer::create("source1", "target1", "test.topic");
    assert(create_result.is_ok() && "Should create message");
    auto original = create_result.value();

    // Add some data
    original.container().set_value("key1", "value1");
    original.container().set_value("key2", 42);

    // Serialize
    auto serialize_result = original.serialize();
    assert(serialize_result.is_ok() && "Should serialize");
    auto bytes = serialize_result.value();
    assert(!bytes.empty() && "Serialized data should not be empty");

    // Deserialize
    auto deserialize_result = MessagingContainer::deserialize(bytes);
    assert(deserialize_result.is_ok() && "Should deserialize");
    auto restored = deserialize_result.value();

    assert(restored.source() == original.source() && "Source should match");
    assert(restored.target() == original.target() && "Target should match");
    assert(restored.topic() == original.topic() && "Topic should match");
    assert(restored.trace_id() == original.trace_id() && "Trace ID should match");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_builder_pattern() {
    std::cout << "Test: Builder pattern..." << std::endl;

    auto result = MessagingContainerBuilder()
        .source("src")
        .target("tgt")
        .topic("user.login")
        .add_value("user_id", "12345")
        .add_value("timestamp", 1234567890)
        .build();

    assert(result.is_ok() && "Should build message");
    auto msg = result.value();

    assert(msg.source() == "src" && "Source should match");
    assert(msg.target() == "tgt" && "Target should match");
    assert(msg.topic() == "user.login" && "Topic should match");

    std::cout << "  ✓ Passed" << std::endl;
}

int main() {
    std::cout << "=== MessagingContainer Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        test_create_valid_message();
        test_create_invalid_message();
        test_serialize_deserialize();
        test_builder_pattern();

        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
