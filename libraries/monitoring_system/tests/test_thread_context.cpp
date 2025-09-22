/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, monitoring_system contributors
All rights reserved.
*****************************************************************************/

/**
 * @file test_thread_context.cpp
 * @brief Unit tests for thread context and metadata
 */

#include <gtest/gtest.h>
#include <kcenon/monitoring/context/thread_context.h>
#include <thread>
#include <vector>
#include <set>
#include <future>

using namespace monitoring_system;

/**
 * Test fixture for thread context tests
 */
class ThreadContextTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state
        thread_context::clear();
    }
    
    void TearDown() override {
        // Clean up after each test
        thread_context::clear();
    }
};

/**
 * Test context_metadata basic operations
 */
TEST_F(ThreadContextTest, ContextMetadataBasicOperations) {
    context_metadata metadata("req-123");
    
    // Check initial state
    EXPECT_EQ(metadata.request_id, "req-123");
    EXPECT_TRUE(metadata.correlation_id.empty());
    EXPECT_TRUE(metadata.user_id.empty());
    EXPECT_FALSE(metadata.empty());
    
    // Set fields
    metadata.correlation_id = "corr-456";
    metadata.user_id = "user-789";

    // Add custom tags using set_tag method
    metadata.set_tag("environment", "production");
    metadata.set_tag("version", "1.2.3");

    // Verify tags using get_tag method (returns string, not optional)
    auto env = metadata.get_tag("environment");
    EXPECT_EQ(env, "production");

    auto ver = metadata.get_tag("version");
    EXPECT_EQ(ver, "1.2.3");

    auto missing = metadata.get_tag("nonexistent");
    EXPECT_EQ(missing, ""); // Returns empty string for missing tags

    // Verify tags map directly
    EXPECT_TRUE(metadata.tags.find("environment") != metadata.tags.end());
    EXPECT_TRUE(metadata.tags.find("version") != metadata.tags.end());

    // Verify empty check
    EXPECT_FALSE(metadata.empty()); // Should not be empty with data
}

/**
 * Test context_metadata copy and comparison
 */
TEST_F(ThreadContextTest, DISABLED_ContextMetadataMerge) {
    // Disabled until merge functionality is implemented
    context_metadata metadata1("req-1");
    metadata1.user_id = "user-1";
    metadata1.set_tag("tag1", "value1");

    context_metadata metadata2("req-2");
    metadata2.correlation_id = "corr-2";
    metadata2.user_id = "user-2";
    metadata2.set_tag("tag2", "value2");

    // Basic copy test instead of merge
    context_metadata copy = metadata1;
    EXPECT_EQ(copy.request_id, metadata1.request_id);
    EXPECT_EQ(copy.user_id, metadata1.user_id);

    // Verify tags are copied
    EXPECT_EQ(copy.get_tag("tag1"), "value1");
}

/**
 * Test thread_context basic operations
 */
TEST_F(ThreadContextTest, ThreadContextBasicOperations) {
    // Initially no context
    EXPECT_FALSE(thread_context::has_context());
    EXPECT_EQ(thread_context::current(), nullptr);
    
    // Create context
    auto& ctx = thread_context::create("test-request");
    EXPECT_TRUE(thread_context::has_context());
    EXPECT_NE(thread_context::current(), nullptr);
    EXPECT_EQ(ctx.request_id, "test-request");
    
    // Modify current context
    auto* current = thread_context::current();
    ASSERT_NE(current, nullptr);
    current->user_id = "test-user";
    current->add_tag("test", "value");
    
    // Verify modifications
    EXPECT_EQ(thread_context::current()->user_id, "test-user");
    auto tag = thread_context::current()->get_tag("test");
    ASSERT_TRUE(tag.has_value());
    EXPECT_EQ(tag.value(), "value");
    
    // Clear context
    thread_context::clear();
    EXPECT_FALSE(thread_context::has_context());
    EXPECT_EQ(thread_context::current(), nullptr);
}

/**
 * Test automatic request ID generation
 */
TEST_F(ThreadContextTest, RequestIdGeneration) {
    // Generate multiple IDs
    std::set<std::string> ids;
    for (int i = 0; i < 100; ++i) {
        ids.insert(thread_context::generate_request_id());
    }
    
    // All should be unique
    EXPECT_EQ(ids.size(), 100);
    
    // Create context without request ID
    auto& ctx = thread_context::create();
    EXPECT_FALSE(ctx.request_id.empty());
    
    // Generated ID should be unique
    auto generated_id = ctx.request_id;
    EXPECT_EQ(ids.find(generated_id), ids.end());
}

/**
 * Test correlation ID generation
 */
TEST_F(ThreadContextTest, CorrelationIdGeneration) {
    auto corr_id1 = thread_context::generate_correlation_id();
    auto corr_id2 = thread_context::generate_correlation_id();
    
    EXPECT_FALSE(corr_id1.empty());
    EXPECT_FALSE(corr_id2.empty());
    EXPECT_NE(corr_id1, corr_id2);
    
    // Should have "corr-" prefix
    EXPECT_EQ(corr_id1.substr(0, 5), "corr-");
    EXPECT_EQ(corr_id2.substr(0, 5), "corr-");
}

/**
 * Test context_scope RAII wrapper
 */
TEST_F(ThreadContextTest, ContextScope) {
    // No initial context
    EXPECT_FALSE(thread_context::has_context());
    
    {
        // Create scope with new context
        context_scope scope("scoped-request");
        
        EXPECT_TRUE(thread_context::has_context());
        EXPECT_EQ(thread_context::current()->request_id, "scoped-request");
        
        // Modify context within scope
        thread_context::current()->user_id = "scoped-user";
    }
    
    // Context should be cleared after scope
    EXPECT_FALSE(thread_context::has_context());
}

/**
 * Test context_scope with preservation
 */
TEST_F(ThreadContextTest, ContextScopeWithPreservation) {
    // Set initial context
    thread_context::create("original-request");
    thread_context::current()->user_id = "original-user";
    
    {
        // Create scope that preserves previous context
        auto new_metadata = std::make_unique<context_metadata>("scoped-request");
        new_metadata->user_id = "scoped-user";
        context_scope scope(std::move(new_metadata), true);
        
        // Should have new context
        EXPECT_EQ(thread_context::current()->request_id, "scoped-request");
        EXPECT_EQ(thread_context::current()->user_id, "scoped-user");
    }
    
    // Should restore original context
    EXPECT_TRUE(thread_context::has_context());
    EXPECT_EQ(thread_context::current()->request_id, "original-request");
    EXPECT_EQ(thread_context::current()->user_id, "original-user");
}

/**
 * Test context_propagator
 */
TEST_F(ThreadContextTest, ContextPropagator) {
    // Create context in main thread
    thread_context::create("main-request");
    thread_context::current()->user_id = "main-user";
    thread_context::current()->add_tag("source", "main");
    
    // Capture context
    context_propagator propagator;
    auto capture_result = propagator.capture();
    ASSERT_TRUE(capture_result);
    EXPECT_TRUE(propagator.has_captured());
    
    // Clear main thread context
    thread_context::clear();
    EXPECT_FALSE(thread_context::has_context());
    
    // Apply in same thread
    auto apply_result = propagator.apply();
    ASSERT_TRUE(apply_result);
    
    // Should have restored context
    EXPECT_TRUE(thread_context::has_context());
    EXPECT_EQ(thread_context::current()->request_id, "main-request");
    EXPECT_EQ(thread_context::current()->user_id, "main-user");
    auto tag = thread_context::current()->get_tag("source");
    ASSERT_TRUE(tag.has_value());
    EXPECT_EQ(tag.value(), "main");
}

/**
 * Test context propagation across threads
 */
TEST_F(ThreadContextTest, CrossThreadPropagation) {
    // Create context in main thread
    thread_context::create("main-thread-request");
    thread_context::current()->correlation_id = "main-correlation";
    thread_context::current()->add_tag("thread", "main");
    
    // Capture for propagation
    auto propagator = context_propagator::from_current();
    
    // Verify in another thread
    std::thread worker([propagator]() {
        // Initially no context in new thread
        EXPECT_FALSE(thread_context::has_context());
        
        // Apply captured context
        auto result = propagator.apply();
        EXPECT_TRUE(result);
        
        // Should have context from main thread
        EXPECT_TRUE(thread_context::has_context());
        EXPECT_EQ(thread_context::current()->request_id, "main-thread-request");
        EXPECT_EQ(thread_context::current()->correlation_id, "main-correlation");
        
        auto tag = thread_context::current()->get_tag("thread");
        ASSERT_TRUE(tag.has_value());
        EXPECT_EQ(tag.value(), "main");
        
        // Modify in worker thread
        thread_context::current()->add_tag("thread", "worker");
    });
    
    worker.join();
    
    // Main thread context should be unchanged
    auto main_tag = thread_context::current()->get_tag("thread");
    ASSERT_TRUE(main_tag.has_value());
    EXPECT_EQ(main_tag.value(), "main");
}

/**
 * Test context_aware_monitoring enrichment
 */
TEST_F(ThreadContextTest, ContextAwareEnrichment) {
    // Create test implementation
    class test_context_aware : public context_aware_monitoring {
    public:
        // Use default implementation
    };
    
    test_context_aware aware;
    
    // Create context
    thread_context::create("enrich-request");
    thread_context::current()->correlation_id = "enrich-corr";
    thread_context::current()->user_id = "enrich-user";
    thread_context::current()->add_tag("custom1", "value1");
    thread_context::current()->add_tag("custom2", "value2");
    
    // Create monitoring data
    monitoring_data data("test-component");
    data.add_metric("metric1", 100.0);
    
    // Enrich with context
    auto result = aware.enrich_with_context(data);
    ASSERT_TRUE(result);
    
    // Verify context was added as tags
    auto req_id = data.get_tag("request_id");
    ASSERT_TRUE(req_id.has_value());
    EXPECT_EQ(req_id.value(), "enrich-request");
    
    auto corr_id = data.get_tag("correlation_id");
    ASSERT_TRUE(corr_id.has_value());
    EXPECT_EQ(corr_id.value(), "enrich-corr");
    
    auto user_id = data.get_tag("user_id");
    ASSERT_TRUE(user_id.has_value());
    EXPECT_EQ(user_id.value(), "enrich-user");
    
    // Custom tags should have "ctx." prefix
    auto custom1 = data.get_tag("ctx.custom1");
    ASSERT_TRUE(custom1.has_value());
    EXPECT_EQ(custom1.value(), "value1");
    
    auto custom2 = data.get_tag("ctx.custom2");
    ASSERT_TRUE(custom2.has_value());
    EXPECT_EQ(custom2.value(), "value2");
    
    // Original metric should remain
    auto metric = data.get_metric("metric1");
    ASSERT_TRUE(metric.has_value());
    EXPECT_DOUBLE_EQ(metric.value(), 100.0);
}

/**
 * Test context_metrics_collector
 */
TEST_F(ThreadContextTest, ContextMetricsCollector) {
    // Create test collector
    class test_collector : public context_metrics_collector {
    public:
        explicit test_collector(const std::string& name)
            : context_metrics_collector(name) {}
        
        result<metrics_snapshot> collect() override {
            auto snapshot = create_snapshot_with_context();
            snapshot.add_metric("test_metric", 42.0);
            return make_success(std::move(snapshot));
        }
    };
    
    test_collector collector("test-collector");
    
    // Set thread context
    thread_context::create("collector-request");
    thread_context::current()->user_id = "collector-user";
    
    // Collect with context awareness enabled
    collector.set_context_aware(true);
    auto result1 = collector.collect();
    ASSERT_TRUE(result1);
    
    auto snapshot1 = result1.value();
    EXPECT_EQ(snapshot1.source_id, "test-collector");
    
    // Verify metric
    auto metric = snapshot1.get_metric("test_metric");
    ASSERT_TRUE(metric.has_value());
    EXPECT_DOUBLE_EQ(metric.value(), 42.0);
    
    // Collect with context awareness disabled
    collector.set_context_aware(false);
    auto result2 = collector.collect();
    ASSERT_TRUE(result2);
    
    // Should still work but without context
    auto snapshot2 = result2.value();
    EXPECT_EQ(snapshot2.source_id, "test-collector");
}

/**
 * Test thread isolation
 */
TEST_F(ThreadContextTest, ThreadIsolation) {
    const int thread_count = 10;
    std::vector<std::thread> threads;
    std::vector<std::future<std::string>> futures;
    
    // Create threads with their own contexts
    for (int i = 0; i < thread_count; ++i) {
        std::promise<std::string> promise;
        futures.push_back(promise.get_future());
        
        threads.emplace_back([i, p = std::move(promise)]() mutable {
            // Each thread creates its own context
            auto req_id = "thread-" + std::to_string(i);
            thread_context::create(req_id);
            thread_context::current()->user_id = "user-" + std::to_string(i);
            
            // Do some work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // Verify context is still correct
            auto* ctx = thread_context::current();
            if (ctx && ctx->request_id == req_id && 
                ctx->user_id == "user-" + std::to_string(i)) {
                p.set_value(req_id);
            } else {
                p.set_value("error");
            }
        });
    }
    
    // Collect results
    std::set<std::string> results;
    for (auto& future : futures) {
        results.insert(future.get());
    }
    
    // Wait for threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Each thread should have maintained its own context
    EXPECT_EQ(results.size(), thread_count);
    for (int i = 0; i < thread_count; ++i) {
        auto expected = "thread-" + std::to_string(i);
        EXPECT_NE(results.find(expected), results.end());
    }
}

/**
 * Test copy_from functionality
 */
TEST_F(ThreadContextTest, CopyFromContext) {
    // Create source context
    context_metadata source("source-request");
    source.correlation_id = "source-corr";
    source.user_id = "source-user";
    source.add_tag("tag1", "value1");
    
    // Copy to current thread
    auto result = thread_context::copy_from(source);
    ASSERT_TRUE(result);
    
    // Verify copy
    EXPECT_TRUE(thread_context::has_context());
    auto* current = thread_context::current();
    ASSERT_NE(current, nullptr);
    
    EXPECT_EQ(current->request_id, "source-request");
    EXPECT_EQ(current->correlation_id, "source-corr");
    EXPECT_EQ(current->user_id, "source-user");
    
    auto tag = current->get_tag("tag1");
    ASSERT_TRUE(tag.has_value());
    EXPECT_EQ(tag.value(), "value1");
    
    // Modifications shouldn't affect source
    current->user_id = "modified-user";
    EXPECT_EQ(source.user_id, "source-user");
}

// Main function provided by gtest_main