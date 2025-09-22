/**
 * @file test_distributed_tracing.cpp
 * @brief Unit tests for distributed tracing functionality
 */

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <unordered_map>
// Note: distributed_tracer.h does not exist in include directory
// #include <kcenon/monitoring/tracing/distributed_tracer.h>

using namespace monitoring_system;

class DistributedTracingTest : public ::testing::Test {
protected:
    distributed_tracer tracer;
    
    void SetUp() override {
        // Reset global tracer state if needed
    }
    
    void TearDown() override {
        // Cleanup
    }
};

TEST_F(DistributedTracingTest, CreateRootSpan) {
    auto span_result = tracer.start_span("test_operation", "test_service");
    ASSERT_TRUE(span_result.has_value());
    
    auto span = span_result.value();
    EXPECT_FALSE(span->trace_id.empty());
    EXPECT_FALSE(span->span_id.empty());
    EXPECT_TRUE(span->parent_span_id.empty());
    EXPECT_EQ(span->operation_name, "test_operation");
    EXPECT_EQ(span->service_name, "test_service");
    EXPECT_FALSE(span->is_finished());
}

TEST_F(DistributedTracingTest, CreateChildSpan) {
    auto parent_result = tracer.start_span("parent_operation");
    ASSERT_TRUE(parent_result.has_value());
    auto parent = parent_result.value();
    
    auto child_result = tracer.start_child_span(*parent, "child_operation");
    ASSERT_TRUE(child_result.has_value());
    auto child = child_result.value();
    
    EXPECT_EQ(child->trace_id, parent->trace_id);
    EXPECT_NE(child->span_id, parent->span_id);
    EXPECT_EQ(child->parent_span_id, parent->span_id);
    EXPECT_EQ(child->operation_name, "child_operation");
}

TEST_F(DistributedTracingTest, FinishSpan) {
    auto span_result = tracer.start_span("test_operation");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    // Add some delay to have measurable duration
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto finish_result = tracer.finish_span(span);
    ASSERT_TRUE(finish_result.has_value());
    
    EXPECT_TRUE(span->is_finished());
    EXPECT_GT(span->duration.count(), 0);
    EXPECT_EQ(span->status, trace_span::status_code::ok);
}

TEST_F(DistributedTracingTest, CannotFinishSpanTwice) {
    auto span_result = tracer.start_span("test_operation");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    auto first_finish = tracer.finish_span(span);
    ASSERT_TRUE(first_finish.has_value());
    
    auto second_finish = tracer.finish_span(span);
    ASSERT_FALSE(second_finish.has_value());
    EXPECT_EQ(second_finish.get_error().code, monitoring_error_code::already_exists);
}

TEST_F(DistributedTracingTest, TraceContextPropagation) {
    auto span_result = tracer.start_span("test_operation");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    // Add baggage
    span->baggage["user_id"] = "12345";
    span->baggage["request_type"] = "api";
    
    // Extract context - explicitly use const trace_span&
    const trace_span& span_ref = *span;
    auto context = tracer.extract_context(span_ref);
    EXPECT_EQ(context.trace_id, span->trace_id);
    EXPECT_EQ(context.span_id, span->span_id);
    EXPECT_EQ(context.baggage["user_id"], "12345");
    EXPECT_EQ(context.baggage["request_type"], "api");
}

TEST_F(DistributedTracingTest, W3CTraceContextFormat) {
    trace_context ctx;
    ctx.trace_id = "0af7651916cd43dd8448eb211c80319c";
    ctx.span_id = "b7ad6b7169203331";
    ctx.trace_flags = "01";
    
    auto header = ctx.to_w3c_traceparent();
    EXPECT_EQ(header, "00-0af7651916cd43dd8448eb211c80319c-b7ad6b7169203331-01");
    
    auto parsed_result = trace_context::from_w3c_traceparent(header);
    ASSERT_TRUE(parsed_result.has_value());
    auto parsed = parsed_result.value();
    
    EXPECT_EQ(parsed.trace_id, ctx.trace_id);
    EXPECT_EQ(parsed.span_id, ctx.span_id);
    EXPECT_EQ(parsed.trace_flags, ctx.trace_flags);
}

TEST_F(DistributedTracingTest, InjectExtractContext) {
    auto span_result = tracer.start_span("test_operation");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    span->baggage["test_key"] = "test_value";
    
    // Inject into carrier (simulating HTTP headers)
    std::unordered_map<std::string, std::string> headers;
    auto context = tracer.extract_context(*span);
    tracer.inject_context(context, headers);
    
    EXPECT_TRUE(headers.contains("traceparent"));
    EXPECT_TRUE(headers.contains("baggage-test_key"));
    
    // Extract from carrier
    auto extracted_result = tracer.extract_context_from_carrier(headers);
    ASSERT_TRUE(extracted_result.has_value());
    auto extracted = extracted_result.value();
    
    EXPECT_EQ(extracted.trace_id, span->trace_id);
    EXPECT_EQ(extracted.span_id, span->span_id);
    EXPECT_EQ(extracted.baggage["test_key"], "test_value");
}

TEST_F(DistributedTracingTest, StartSpanFromContext) {
    // Simulate incoming request with trace context
    trace_context incoming_ctx;
    incoming_ctx.trace_id = "0af7651916cd43dd8448eb211c80319c";
    incoming_ctx.span_id = "b7ad6b7169203331";
    incoming_ctx.baggage["user_id"] = "67890";
    
    auto span_result = tracer.start_span_from_context(incoming_ctx, "handle_request");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    EXPECT_EQ(span->trace_id, incoming_ctx.trace_id);
    EXPECT_NE(span->span_id, incoming_ctx.span_id);  // New span ID
    EXPECT_EQ(span->parent_span_id, incoming_ctx.span_id);
    EXPECT_EQ(span->baggage["user_id"], "67890");
}

TEST_F(DistributedTracingTest, CurrentSpanManagement) {
    EXPECT_EQ(tracer.get_current_span(), nullptr);
    
    auto span_result = tracer.start_span("test_operation");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    tracer.set_current_span(span);
    EXPECT_EQ(tracer.get_current_span(), span);
    
    // Different thread should have different current span
    std::thread other_thread([&]() {
        EXPECT_EQ(tracer.get_current_span(), nullptr);
        
        auto other_span_result = tracer.start_span("other_operation");
        ASSERT_TRUE(other_span_result.has_value());
        auto other_span = other_span_result.value();
        
        tracer.set_current_span(other_span);
        EXPECT_EQ(tracer.get_current_span(), other_span);
    });
    other_thread.join();
    
    // Original thread should still have its span
    EXPECT_EQ(tracer.get_current_span(), span);
}

TEST_F(DistributedTracingTest, ScopedSpan) {
    {
        auto span_result = tracer.start_span("scoped_operation");
        ASSERT_TRUE(span_result.has_value());
        scoped_span scoped(span_result.value(), &tracer);
        
        EXPECT_EQ(tracer.get_current_span(), span_result.value());
        EXPECT_FALSE(scoped->is_finished());
        
        // Span should be accessible via scoped object
        scoped->tags["custom_tag"] = "custom_value";
    }
    // Span should be finished after leaving scope
    // Note: We can't directly check as the span is finished and stored
}

TEST_F(DistributedTracingTest, GetTrace) {
    auto span1_result = tracer.start_span("operation1");
    ASSERT_TRUE(span1_result.has_value());
    auto span1 = span1_result.value();
    
    auto span2_result = tracer.start_child_span(*span1, "operation2");
    ASSERT_TRUE(span2_result.has_value());
    auto span2 = span2_result.value();
    
    auto span3_result = tracer.start_child_span(*span2, "operation3");
    ASSERT_TRUE(span3_result.has_value());
    auto span3 = span3_result.value();
    
    // Finish all spans
    tracer.finish_span(span1);
    tracer.finish_span(span2);
    tracer.finish_span(span3);
    
    // Get all spans for the trace
    auto trace_result = tracer.get_trace(span1->trace_id);
    ASSERT_TRUE(trace_result.has_value());
    auto trace = trace_result.value();
    
    EXPECT_EQ(trace.size(), 3);
    
    // All spans should have the same trace ID
    for (const auto& span : trace) {
        EXPECT_EQ(span.trace_id, span1->trace_id);
        EXPECT_TRUE(span.is_finished());
    }
}

TEST_F(DistributedTracingTest, SpanTags) {
    auto span_result = tracer.start_span("tagged_operation", "my_service");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    // Check default tags
    EXPECT_EQ(span->tags["span.kind"], "internal");
    EXPECT_EQ(span->tags["service.name"], "my_service");
    
    // Add custom tags
    span->tags["http.method"] = "GET";
    span->tags["http.status_code"] = "200";
    span->tags["user.id"] = "user123";
    
    EXPECT_EQ(span->tags["http.method"], "GET");
    EXPECT_EQ(span->tags["http.status_code"], "200");
    EXPECT_EQ(span->tags["user.id"], "user123");
}

TEST_F(DistributedTracingTest, SpanStatus) {
    auto span_result = tracer.start_span("status_operation");
    ASSERT_TRUE(span_result.has_value());
    auto span = span_result.value();
    
    EXPECT_EQ(span->status, trace_span::status_code::unset);
    
    // Set error status
    span->status = trace_span::status_code::error;
    span->status_message = "Operation failed due to timeout";
    
    tracer.finish_span(span);
    
    EXPECT_EQ(span->status, trace_span::status_code::error);
    EXPECT_EQ(span->status_message, "Operation failed due to timeout");
}

TEST_F(DistributedTracingTest, BaggagePropagation) {
    auto parent_result = tracer.start_span("parent");
    ASSERT_TRUE(parent_result.has_value());
    auto parent = parent_result.value();
    
    parent->baggage["session_id"] = "abc123";
    parent->baggage["feature_flag"] = "enabled";
    
    auto child_result = tracer.start_child_span(*parent, "child");
    ASSERT_TRUE(child_result.has_value());
    auto child = child_result.value();
    
    // Child should inherit baggage
    EXPECT_EQ(child->baggage["session_id"], "abc123");
    EXPECT_EQ(child->baggage["feature_flag"], "enabled");
    
    // Child can add its own baggage
    child->baggage["child_data"] = "xyz";
    
    auto grandchild_result = tracer.start_child_span(*child, "grandchild");
    ASSERT_TRUE(grandchild_result.has_value());
    auto grandchild = grandchild_result.value();
    
    // Grandchild should have all baggage
    EXPECT_EQ(grandchild->baggage["session_id"], "abc123");
    EXPECT_EQ(grandchild->baggage["feature_flag"], "enabled");
    EXPECT_EQ(grandchild->baggage["child_data"], "xyz");
}

TEST_F(DistributedTracingTest, ExportSpans) {
    std::vector<trace_span> spans_to_export;
    
    // Create and finish spans
    for (int i = 0; i < 5; ++i) {
        auto span_result = tracer.start_span("operation_" + std::to_string(i));
        ASSERT_TRUE(span_result.has_value());
        auto span = span_result.value();
        
        tracer.finish_span(span);
        spans_to_export.push_back(*span);
    }
    
    auto export_result = tracer.export_spans(spans_to_export);
    ASSERT_TRUE(export_result.has_value());
    
    // Verify spans were stored
    auto trace_result = tracer.get_trace(spans_to_export[0].trace_id);
    ASSERT_TRUE(trace_result.has_value());
    auto trace = trace_result.value();
    
    // Note: In this test, all spans have different trace IDs
    // In a real scenario, they might be part of the same trace
}

// Test with macros
TEST_F(DistributedTracingTest, TraceMacros) {
    {
        TRACE_SPAN("macro_operation");
        
        // The span should be active
        auto current = global_tracer().get_current_span();
        ASSERT_NE(current, nullptr);
        EXPECT_EQ(current->operation_name, "macro_operation");
        
        // Nested span
        {
            TRACE_CHILD_SPAN(*current, "nested_operation");
            auto nested = global_tracer().get_current_span();
            ASSERT_NE(nested, nullptr);
            EXPECT_EQ(nested->operation_name, "nested_operation");
            EXPECT_EQ(nested->parent_span_id, current->span_id);
        }
    }
    // Spans should be finished after leaving scope
}