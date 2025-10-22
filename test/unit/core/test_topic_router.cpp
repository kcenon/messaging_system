// Unit tests for TopicRouter pattern matching

#include "messaging_system/core/topic_router.h"
#include "messaging_system/core/messaging_container.h"
#include <kcenon/common/interfaces/executor_interface.h>

#include <iostream>
#include <future>
#include <cassert>
#include <atomic>
#include <thread>
#include <chrono>

using namespace messaging;

// Simple mock executor for testing
class MockExecutor : public common::interfaces::IExecutor {
public:
    MockExecutor() : running_(true) {}

    // Function-based execution (legacy)
    std::future<void> submit(std::function<void()> task) override {
        // Execute immediately for testing
        task();
        std::promise<void> promise;
        promise.set_value();
        return promise.get_future();
    }

    std::future<void> submit_delayed(std::function<void()> task, std::chrono::milliseconds) override {
        return submit(task);
    }

    // Job-based execution (Phase 2)
    common::Result<std::future<void>> execute(std::unique_ptr<common::interfaces::IJob>&& job) override {
        auto result = job->execute();
        if (result.is_err()) {
            auto err = result.unwrap_err();
            return common::Result<std::future<void>>(err);
        }
        std::promise<void> promise;
        promise.set_value();
        return common::Result<std::future<void>>::ok(promise.get_future());
    }

    common::Result<std::future<void>> execute_delayed(
        std::unique_ptr<common::interfaces::IJob>&& job,
        std::chrono::milliseconds delay) override {
        return execute(std::move(job));
    }

    // Status and control
    size_t worker_count() const override { return 1; }
    bool is_running() const override { return running_; }
    size_t pending_tasks() const override { return 0; }

    void shutdown(bool wait_for_completion = true) override {
        running_ = false;
    }

private:
    std::atomic<bool> running_;
};

void test_exact_topic_match() {
    std::cout << "Test: Exact topic match..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> call_count{0};

    auto callback = [&](const MessagingContainer& msg) -> common::VoidResult {
        call_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    auto sub_result = router.subscribe("user.created", callback);
    assert(sub_result.is_ok() && "Should subscribe successfully");

    auto msg = MessagingContainer::create("test", "test", "user.created").value();
    router.route(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(call_count == 1 && "Should receive exactly one message");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_single_wildcard_match() {
    std::cout << "Test: Single-level wildcard (*)..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> call_count{0};

    auto callback = [&](const MessagingContainer& msg) -> common::VoidResult {
        call_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    // Subscribe to user.*
    auto sub_result = router.subscribe("user.*", callback);
    assert(sub_result.is_ok() && "Should subscribe successfully");

    // Should match
    auto msg1 = MessagingContainer::create("test", "test", "user.created").value();
    router.route(msg1);

    auto msg2 = MessagingContainer::create("test", "test", "user.deleted").value();
    router.route(msg2);

    // Should NOT match (multi-level)
    auto msg3 = MessagingContainer::create("test", "test", "user.admin.created").value();
    router.route(msg3);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(call_count == 2 && "Should receive exactly 2 messages (not 3)");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_multilevel_wildcard_match() {
    std::cout << "Test: Multi-level wildcard (#)..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> call_count{0};

    auto callback = [&](const MessagingContainer& msg) -> common::VoidResult {
        call_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    // Subscribe to order.#
    auto sub_result = router.subscribe("order.#", callback);
    assert(sub_result.is_ok() && "Should subscribe successfully");

    // All should match
    auto msg1 = MessagingContainer::create("test", "test", "order.placed").value();
    router.route(msg1);

    auto msg2 = MessagingContainer::create("test", "test", "order.placed.confirmed").value();
    router.route(msg2);

    auto msg3 = MessagingContainer::create("test", "test", "order.shipped.tracking.updated").value();
    router.route(msg3);

    // Should NOT match
    auto msg4 = MessagingContainer::create("test", "test", "user.created").value();
    router.route(msg4);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(call_count == 3 && "Should receive exactly 3 messages (not 4)");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_multiple_subscribers_same_topic() {
    std::cout << "Test: Multiple subscribers on same topic..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> sub1_count{0};
    std::atomic<int> sub2_count{0};

    auto callback1 = [&](const MessagingContainer& msg) -> common::VoidResult {
        sub1_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    auto callback2 = [&](const MessagingContainer& msg) -> common::VoidResult {
        sub2_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    router.subscribe("event.test", callback1);
    router.subscribe("event.test", callback2);

    auto msg = MessagingContainer::create("test", "test", "event.test").value();
    router.route(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(sub1_count == 1 && "Subscriber 1 should receive message");
    assert(sub2_count == 1 && "Subscriber 2 should receive message");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_unsubscribe() {
    std::cout << "Test: Unsubscribe functionality..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> call_count{0};

    auto callback = [&](const MessagingContainer& msg) -> common::VoidResult {
        call_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    auto sub_result = router.subscribe("test.topic", callback);
    assert(sub_result.is_ok() && "Should subscribe successfully");
    auto sub_id = sub_result.value();

    // Send message - should receive
    auto msg1 = MessagingContainer::create("test", "test", "test.topic").value();
    router.route(msg1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(call_count == 1 && "Should receive first message");

    // Unsubscribe
    router.unsubscribe(sub_id);

    // Send message - should NOT receive
    auto msg2 = MessagingContainer::create("test", "test", "test.topic").value();
    router.route(msg2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(call_count == 1 && "Should not receive second message after unsubscribe");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_complex_wildcard_patterns() {
    std::cout << "Test: Complex wildcard patterns..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> pattern1_count{0};
    std::atomic<int> pattern2_count{0};

    auto callback1 = [&](const MessagingContainer& msg) -> common::VoidResult {
        pattern1_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    auto callback2 = [&](const MessagingContainer& msg) -> common::VoidResult {
        pattern2_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    // Pattern 1: event.*.created
    router.subscribe("event.*.created", callback1);

    // Pattern 2: event.#
    router.subscribe("event.#", callback2);

    // Test messages
    auto msg1 = MessagingContainer::create("test", "test", "event.user.created").value();
    router.route(msg1); // Both should match

    auto msg2 = MessagingContainer::create("test", "test", "event.order.created").value();
    router.route(msg2); // Both should match

    auto msg3 = MessagingContainer::create("test", "test", "event.user.deleted").value();
    router.route(msg3); // Only pattern2 should match

    auto msg4 = MessagingContainer::create("test", "test", "event.system.startup.complete").value();
    router.route(msg4); // Only pattern2 should match

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(pattern1_count == 2 && "Pattern 1 should match 2 messages");
    assert(pattern2_count == 4 && "Pattern 2 should match all 4 messages");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_no_match() {
    std::cout << "Test: No matching subscribers..." << std::endl;

    auto executor = std::make_shared<MockExecutor>();
    TopicRouter router(executor);

    std::atomic<int> call_count{0};

    auto callback = [&](const MessagingContainer& msg) -> common::VoidResult {
        call_count++;
        return common::VoidResult::ok(std::monostate{});
    };

    router.subscribe("user.created", callback);

    // Send unmatched message
    auto msg = MessagingContainer::create("test", "test", "order.placed").value();
    auto result = router.route(msg);

    // Route should succeed even with no subscribers
    assert(result.is_ok() && "Route should succeed even with no subscribers");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(call_count == 0 && "Should not receive any messages");

    std::cout << "  ✓ Passed" << std::endl;
}

int main() {
    std::cout << "=== TopicRouter Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        test_exact_topic_match();
        test_single_wildcard_match();
        test_multilevel_wildcard_match();
        test_multiple_subscribers_same_topic();
        test_unsubscribe();
        test_complex_wildcard_patterns();
        test_no_match();

        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
