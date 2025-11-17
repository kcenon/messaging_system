#include "framework/messaging_fixture.h"
#include "framework/test_helpers.h"
#include <gtest/gtest.h>
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/patterns/request_reply.h>

using namespace kcenon::messaging;
using namespace kcenon::messaging::testing;
using namespace kcenon::messaging::patterns;
using namespace kcenon::common;

/**
 * @brief Full stack integration tests
 */
class FullIntegrationTest : public MessagingFixture {};

TEST_F(FullIntegrationTest, EndToEndPubSub) {
    // Create publisher and subscriber
    auto pub = std::make_shared<publisher>(bus_, "events");
    auto sub = std::make_shared<subscriber>(bus_);

    MessageCounter counter;
    auto sub_result = sub->subscribe("events", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish messages
    for (int i = 0; i < 10; ++i) {
        auto msg = create_test_message("events");
        ASSERT_TRUE(pub->publish(msg).is_ok());
    }

    // Verify delivery
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= 10; },
        std::chrono::seconds{2}
    ));

    EXPECT_EQ(counter.count(), 10);
}

TEST_F(FullIntegrationTest, RequestReplyPattern) {
    // Create request/reply handler
    auto handler = std::make_shared<request_reply_handler>(bus_, "service.echo");

    // Register echo handler
    auto register_result = handler->register_handler(
        [](const message& req) -> Result<message> {
            auto reply = message_builder()
                .topic("service.echo.reply")
                .correlation_id(req.metadata().correlation_id)
                .build();

            return reply;
        }
    );
    ASSERT_TRUE(register_result.is_ok());

    // Send request
    auto request_msg = create_test_message("service.echo");
    auto reply_result = handler->request(request_msg, std::chrono::seconds{2});

    ASSERT_TRUE(reply_result.is_ok());
    auto reply = reply_result.unwrap();
    EXPECT_EQ(reply.metadata().correlation_id, request_msg.metadata().correlation_id);
}

TEST_F(FullIntegrationTest, MultiplePatternsConcurrently) {
    // Setup pub/sub
    auto pub = std::make_shared<publisher>(bus_, "events");
    auto sub = std::make_shared<subscriber>(bus_);

    MessageCounter event_counter;
    auto sub_result = sub->subscribe("events", create_counting_callback(event_counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Setup request/reply
    auto handler = std::make_shared<request_reply_handler>(bus_, "service.test");
    auto register_result = handler->register_handler(
        [](const message& req) -> Result<message> {
            return message_builder()
                .topic("service.test.reply")
                .correlation_id(req.metadata().correlation_id)
                .build();
        }
    );
    ASSERT_TRUE(register_result.is_ok());

    // Execute both patterns concurrently
    std::atomic<bool> pubsub_done{false};
    std::atomic<bool> reqrep_done{false};

    // Pub/sub task
    std::thread pubsub_thread([&]() {
        for (int i = 0; i < 50; ++i) {
            pub->publish(create_test_message("events"));
        }
        pubsub_done.store(true);
    });

    // Request/reply task
    std::thread reqrep_thread([&]() {
        for (int i = 0; i < 10; ++i) {
            auto req = create_test_message("service.test");
            handler->request(req, std::chrono::seconds{1});
        }
        reqrep_done.store(true);
    });

    // Wait for completion
    pubsub_thread.join();
    reqrep_thread.join();

    EXPECT_TRUE(pubsub_done.load());
    EXPECT_TRUE(reqrep_done.load());

    // Verify pub/sub worked
    ASSERT_TRUE(wait_for_condition(
        [&]() { return event_counter.count() >= 50; },
        std::chrono::seconds{3}
    ));
}

TEST_F(FullIntegrationTest, ComplexRoutingScenario) {
    // Create multiple subscribers with different patterns
    MessageCounter exact_counter;
    MessageCounter wildcard_counter;
    MessageCounter multilevel_counter;

    auto sub1 = bus_->subscribe("orders.created", create_counting_callback(exact_counter));
    auto sub2 = bus_->subscribe("orders.*", create_counting_callback(wildcard_counter));
    auto sub3 = bus_->subscribe("orders.#", create_counting_callback(multilevel_counter));

    ASSERT_TRUE(sub1.is_ok() && sub2.is_ok() && sub3.is_ok());

    // Publish to exact topic
    ASSERT_TRUE(bus_->publish(create_test_message("orders.created")).is_ok());

    // Publish to other matching topics
    ASSERT_TRUE(bus_->publish(create_test_message("orders.updated")).is_ok());
    ASSERT_TRUE(bus_->publish(create_test_message("orders.item.added")).is_ok());

    // Wait and verify routing
    ASSERT_TRUE(wait_for_condition([&]() {
        return exact_counter.count() >= 1 &&
               wildcard_counter.count() >= 2 &&
               multilevel_counter.count() >= 3;
    }, std::chrono::seconds{2}));

    EXPECT_EQ(exact_counter.count(), 1);        // only "orders.created"
    EXPECT_EQ(wildcard_counter.count(), 2);     // "orders.created" and "orders.updated"
    EXPECT_EQ(multilevel_counter.count(), 3);   // all three
}

TEST_F(FullIntegrationTest, StressTestFullStack) {
    // Create multiple publishers and subscribers
    const int num_topics = 10;
    const int messages_per_topic = 100;

    std::vector<std::shared_ptr<subscriber>> subscribers;
    std::vector<MessageCounter> counters(num_topics);

    // Setup subscribers
    for (int i = 0; i < num_topics; ++i) {
        auto sub = std::make_shared<subscriber>(bus_);
        std::string topic = "stress.topic" + std::to_string(i);

        auto sub_result = sub->subscribe(topic, create_counting_callback(counters[i]));
        ASSERT_TRUE(sub_result.is_ok());

        subscribers.push_back(sub);
    }

    // Publish messages concurrently
    std::vector<std::thread> threads;
    for (int i = 0; i < num_topics; ++i) {
        threads.emplace_back([&, i]() {
            auto pub = std::make_shared<publisher>(
                bus_,
                "stress.topic" + std::to_string(i)
            );

            for (int j = 0; j < messages_per_topic; ++j) {
                pub->publish(create_test_message("stress.topic" + std::to_string(i)));
            }
        });
    }

    // Wait for all publishing threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all messages delivered
    for (int i = 0; i < num_topics; ++i) {
        ASSERT_TRUE(wait_for_condition(
            [&, i]() { return counters[i].count() >= messages_per_topic; },
            std::chrono::seconds{10}
        )) << "Topic " << i << " did not receive all messages";
    }
}

TEST_F(FullIntegrationTest, MessageStatistics) {
    MessageCounter counter;
    auto sub_result = bus_->subscribe("stats.test", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Get initial stats
    auto initial_stats = bus_->get_statistics();

    // Publish messages
    const int count = 50;
    for (int i = 0; i < count; ++i) {
        ASSERT_TRUE(bus_->publish(create_test_message("stats.test")).is_ok());
    }

    // Wait for processing
    ASSERT_TRUE(wait_for_condition(
        [&]() { return counter.count() >= count; },
        std::chrono::seconds{3}
    ));

    // Get final stats
    auto final_stats = bus_->get_statistics();

    // Verify statistics
    EXPECT_EQ(final_stats.messages_published - initial_stats.messages_published, count);
    EXPECT_EQ(final_stats.messages_processed - initial_stats.messages_processed, count);
    EXPECT_EQ(final_stats.messages_failed, initial_stats.messages_failed);
}

TEST_F(FullIntegrationTest, GracefulShutdown) {
    MessageCounter counter;
    auto sub_result = bus_->subscribe("shutdown.test", create_counting_callback(counter));
    ASSERT_TRUE(sub_result.is_ok());

    // Publish some messages
    for (int i = 0; i < 100; ++i) {
        bus_->publish(create_test_message("shutdown.test"));
    }

    // Stop the bus
    ASSERT_TRUE(bus_->stop().is_ok());

    // Verify bus is not running
    EXPECT_FALSE(bus_->is_running());

    // Stats should still be available
    auto stats = bus_->get_statistics();
    EXPECT_GT(stats.messages_published, 0u);
}
