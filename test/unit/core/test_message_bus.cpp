// Unit tests for MessageBus pub/sub functionality

#include "messaging_system/core/message_bus.h"
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

void test_start_stop() {
    std::cout << "Test: MessageBus start/stop..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);

    auto start_result = bus.start();
    assert(start_result.is_ok() && "Should start successfully");

    auto stop_result = bus.stop();
    assert(stop_result.is_ok() && "Should stop successfully");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_publish_subscribe_sync() {
    std::cout << "Test: Synchronous publish/subscribe..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> received_count{0};
    std::string received_topic;

    auto sub_result = bus.subscribe("test.message",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            received_count++;
            received_topic = msg.topic();
            return common::VoidResult::ok(std::monostate{});
        });

    assert(sub_result.is_ok() && "Should subscribe successfully");

    // Publish message
    auto msg = MessagingContainer::create("publisher", "subscriber", "test.message").value();
    auto pub_result = bus.publish_sync(msg);
    assert(pub_result.is_ok() && "Should publish successfully");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(received_count == 1 && "Should receive exactly one message");
    assert(received_topic == "test.message" && "Should receive correct topic");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_publish_subscribe_async() {
    std::cout << "Test: Asynchronous publish/subscribe..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> received_count{0};

    auto sub_result = bus.subscribe("async.test",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            received_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    assert(sub_result.is_ok() && "Should subscribe successfully");

    // Publish multiple messages asynchronously
    for (int i = 0; i < 5; ++i) {
        auto msg = MessagingContainer::create("publisher", "subscriber", "async.test").value();
        bus.publish_async(msg);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(received_count == 5 && "Should receive all 5 messages");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_multiple_subscribers() {
    std::cout << "Test: Multiple subscribers on same topic..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> sub1_count{0};
    std::atomic<int> sub2_count{0};
    std::atomic<int> sub3_count{0};

    bus.subscribe("broadcast.message",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            sub1_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    bus.subscribe("broadcast.message",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            sub2_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    bus.subscribe("broadcast.message",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            sub3_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    auto msg = MessagingContainer::create("publisher", "all", "broadcast.message").value();
    bus.publish_sync(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(sub1_count == 1 && "Subscriber 1 should receive message");
    assert(sub2_count == 1 && "Subscriber 2 should receive message");
    assert(sub3_count == 1 && "Subscriber 3 should receive message");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_wildcard_subscriptions() {
    std::cout << "Test: Wildcard subscriptions via MessageBus..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> wildcard_count{0};
    std::atomic<int> exact_count{0};

    // Wildcard subscription
    bus.subscribe("event.*",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            wildcard_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    // Exact subscription
    bus.subscribe("event.specific",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            exact_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    // Should match both
    auto msg1 = MessagingContainer::create("pub", "sub", "event.specific").value();
    bus.publish_sync(msg1);

    // Should match only wildcard
    auto msg2 = MessagingContainer::create("pub", "sub", "event.general").value();
    bus.publish_sync(msg2);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(wildcard_count == 2 && "Wildcard should match both messages");
    assert(exact_count == 1 && "Exact should match only specific message");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_unsubscribe_via_bus() {
    std::cout << "Test: Unsubscribe via MessageBus..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> received_count{0};

    auto sub_result = bus.subscribe("test.unsub",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            received_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    assert(sub_result.is_ok() && "Should subscribe successfully");
    auto sub_id = sub_result.value();

    // First message - should receive
    auto msg1 = MessagingContainer::create("pub", "sub", "test.unsub").value();
    bus.publish_sync(msg1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(received_count == 1 && "Should receive first message");

    // Unsubscribe
    bus.unsubscribe(sub_id);

    // Second message - should NOT receive
    auto msg2 = MessagingContainer::create("pub", "sub", "test.unsub").value();
    bus.publish_sync(msg2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(received_count == 1 && "Should not receive after unsubscribe");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_concurrent_publishing() {
    std::cout << "Test: Concurrent publishing from multiple threads..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> received_count{0};

    bus.subscribe("concurrent.test",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            received_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    const int num_threads = 4;
    const int messages_per_thread = 10;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                auto msg = MessagingContainer::create(
                    "publisher_" + std::to_string(t),
                    "subscriber",
                    "concurrent.test"
                ).value();
                bus.publish_async(msg);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    assert(received_count == num_threads * messages_per_thread &&
           "Should receive all messages from all threads");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_error_handling_in_callback() {
    std::cout << "Test: Error handling in subscriber callback..." << std::endl;

    auto io_executor = std::make_shared<MockExecutor>();
    auto work_executor = std::make_shared<MockExecutor>();
    auto router = std::make_shared<TopicRouter>(work_executor);

    MessageBus bus(io_executor, work_executor, router);
    bus.start();

    std::atomic<int> success_count{0};
    std::atomic<int> error_count{0};

    // Subscriber that fails
    bus.subscribe("test.error",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            error_count++;
            return common::VoidResult(
                common::error_info{-1, "Intentional error", "test", ""}
            );
        });

    // Subscriber that succeeds (should still receive message even if other fails)
    bus.subscribe("test.error",
        [&](const MessagingContainer& msg) -> common::VoidResult {
            success_count++;
            return common::VoidResult::ok(std::monostate{});
        });

    auto msg = MessagingContainer::create("pub", "sub", "test.error").value();
    bus.publish_sync(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(error_count == 1 && "Error callback should be invoked");
    assert(success_count == 1 && "Success callback should be invoked despite error in other callback");

    bus.stop();
    std::cout << "  ✓ Passed" << std::endl;
}

int main() {
    std::cout << "=== MessageBus Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        test_start_stop();
        test_publish_subscribe_sync();
        test_publish_subscribe_async();
        test_multiple_subscribers();
        test_wildcard_subscriptions();
        test_unsubscribe_via_bus();
        test_concurrent_publishing();
        test_error_handling_in_callback();

        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
