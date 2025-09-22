/**
 * @file distributed_tracer.cpp
 * @brief Implementation of distributed tracing functionality
 */

#include "distributed_tracer.h"
#include <thread>
#include <algorithm>
#include <shared_mutex>
#include <sstream>

namespace monitoring_system {

/**
 * @brief Private implementation of distributed tracer
 */
struct distributed_tracer::tracer_impl {
    // Thread-local storage for current span
    static thread_local std::shared_ptr<trace_span> current_span;
    
    // Storage for all spans
    mutable std::shared_mutex spans_mutex;
    std::unordered_map<std::string, std::vector<trace_span>> traces;  // trace_id -> spans
    
    // Configuration
    std::string default_service_name{"monitoring_system"};
    std::atomic<size_t> max_traces{10000};
    std::atomic<size_t> max_spans_per_trace{1000};
    
    /**
     * @brief Store a span
     */
    monitoring_system::result<bool> store_span(const trace_span& span) {
        std::unique_lock lock(spans_mutex);
        
        auto& trace_spans = traces[span.trace_id];
        if (trace_spans.size() >= max_spans_per_trace) {
            return monitoring_system::make_error<bool>(monitoring_system::monitoring_error_code::resource_exhausted);
        }
        
        trace_spans.push_back(span);
        
        // Cleanup old traces if we have too many
        if (traces.size() > max_traces) {
            // Find oldest trace (simple strategy - in production, use LRU or time-based)
            auto oldest = traces.begin();
            traces.erase(oldest);
        }
        
        return true;
    }
    
    /**
     * @brief Generate unique span ID
     */
    std::string generate_span_id() {
        return thread_context_manager::generate_request_id();
    }
    
    /**
     * @brief Generate unique trace ID
     */
    std::string generate_trace_id() {
        return thread_context_manager::generate_correlation_id();
    }
};

// Define thread-local storage
thread_local std::shared_ptr<trace_span> distributed_tracer::tracer_impl::current_span;

distributed_tracer::distributed_tracer() 
    : impl_(std::make_unique<tracer_impl>()) {
}

distributed_tracer::~distributed_tracer() = default;

distributed_tracer::distributed_tracer(distributed_tracer&&) noexcept = default;
distributed_tracer& distributed_tracer::operator=(distributed_tracer&&) noexcept = default;

monitoring_system::result<std::shared_ptr<trace_span>> distributed_tracer::start_span(
    const std::string& operation_name,
    const std::string& service_name) {
    
    auto span = std::make_shared<trace_span>();
    span->trace_id = impl_->generate_trace_id();
    span->span_id = impl_->generate_span_id();
    span->operation_name = operation_name;
    span->service_name = service_name.empty() ? impl_->default_service_name : service_name;
    span->start_time = std::chrono::system_clock::now();
    
    // Add default tags
    span->tags["span.kind"] = "internal";
    span->tags["service.name"] = span->service_name;
    
    // Get thread context if available
    auto ctx = thread_context_manager::get_context();
    if (ctx) {
        // Thread ID from current std::thread
        std::stringstream ss;
        ss << std::this_thread::get_id();
        span->tags["thread.id"] = ss.str();
        if (!ctx->correlation_id.empty()) {
            span->tags["correlation.id"] = ctx->correlation_id;
        }
    }
    
    return span;
}

monitoring_system::result<std::shared_ptr<trace_span>> distributed_tracer::start_child_span(
    const trace_span& parent,
    const std::string& operation_name) {
    
    auto span = std::make_shared<trace_span>();
    span->trace_id = parent.trace_id;
    span->span_id = impl_->generate_span_id();
    span->parent_span_id = parent.span_id;
    span->operation_name = operation_name;
    span->service_name = parent.service_name;
    span->start_time = std::chrono::system_clock::now();
    
    // Inherit baggage from parent
    span->baggage = parent.baggage;
    
    // Add default tags
    span->tags["span.kind"] = "internal";
    span->tags["service.name"] = span->service_name;
    span->tags["parent.span.id"] = parent.span_id;
    
    return span;
}

monitoring_system::result<std::shared_ptr<trace_span>> distributed_tracer::start_span_from_context(
    const trace_context& context,
    const std::string& operation_name) {
    
    auto span = std::make_shared<trace_span>();
    span->trace_id = context.trace_id;
    span->span_id = impl_->generate_span_id();
    span->parent_span_id = context.span_id;
    span->operation_name = operation_name;
    span->service_name = impl_->default_service_name;
    span->start_time = std::chrono::system_clock::now();
    
    // Copy baggage from context
    span->baggage = context.baggage;
    
    // Add tags
    span->tags["span.kind"] = "server";
    span->tags["service.name"] = span->service_name;
    span->tags["parent.span.id"] = context.span_id;
    
    return span;
}

monitoring_system::result<bool> distributed_tracer::finish_span(std::shared_ptr<trace_span> span) {
    if (!span) {
        return monitoring_system::make_error<bool>(monitoring_system::monitoring_error_code::invalid_argument);
    }
    
    if (span->is_finished()) {
        return monitoring_system::make_error<bool>(monitoring_system::monitoring_error_code::already_exists);
    }
    
    span->end_time = std::chrono::system_clock::now();
    span->calculate_duration();
    
    // Set status if not already set
    if (span->status == trace_span::status_code::unset) {
        span->status = trace_span::status_code::ok;
    }
    
    // Store the span
    return impl_->store_span(*span);
}

std::shared_ptr<trace_span> distributed_tracer::get_current_span() const {
    return impl_->current_span;
}

void distributed_tracer::set_current_span(std::shared_ptr<trace_span> span) {
    impl_->current_span = span;
}

trace_context distributed_tracer::extract_context(const trace_span& span) const {
    trace_context ctx;
    ctx.trace_id = span.trace_id;
    ctx.span_id = span.span_id;
    ctx.trace_flags = "01";  // Sampled
    ctx.baggage = span.baggage;
    return ctx;
}

monitoring_system::result<std::vector<trace_span>> distributed_tracer::get_trace(const std::string& trace_id) const {
    std::shared_lock lock(impl_->spans_mutex);
    
    auto it = impl_->traces.find(trace_id);
    if (it == impl_->traces.end()) {
        return monitoring_system::make_error<std::vector<trace_span>>(monitoring_system::monitoring_error_code::not_found);
    }
    
    return it->second;
}

monitoring_system::result<bool> distributed_tracer::export_spans(std::vector<trace_span> spans) {
    // In a real implementation, this would export to Jaeger, Zipkin, etc.
    // For now, just validate the spans
    for (const auto& span : spans) {
        if (!span.is_finished()) {
            return monitoring_system::make_error<bool>(monitoring_system::monitoring_error_code::invalid_state);
        }
    }
    
    // Store spans
    for (const auto& span : spans) {
        auto result = impl_->store_span(span);
        if (!result) {
            return monitoring_system::make_error<bool>(result.get_error().code);
        }
    }
    
    return true;
}

// Global tracer instance
distributed_tracer& global_tracer() {
    static distributed_tracer instance;
    return instance;
}

} // namespace monitoring_system