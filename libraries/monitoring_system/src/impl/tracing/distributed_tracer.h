/**
 * @file distributed_tracer.h
 * @brief Distributed tracing implementation for monitoring system
 * @date 2025
 * 
 * Provides distributed tracing capabilities for tracking requests
 * across multiple services and components.
 */

#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <optional>

#include <kcenon/monitoring/core/result_types.h>
#include <kcenon/monitoring/core/error_codes.h>
#include <kcenon/monitoring/context/thread_context.h>

namespace monitoring_system {

/**
 * @brief Trace span representing a unit of work in distributed tracing
 */
struct trace_span {
    std::string trace_id;           // Unique trace identifier
    std::string span_id;            // Unique span identifier
    std::string parent_span_id;     // Parent span ID (empty for root span)
    std::string operation_name;     // Name of the operation
    std::string service_name;       // Service executing this span
    
    // Timing information
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::chrono::microseconds duration{0};
    
    // Context and metadata
    std::unordered_map<std::string, std::string> tags;
    std::unordered_map<std::string, std::string> baggage;
    
    // Status information
    enum class status_code {
        unset,
        ok,
        error
    };
    status_code status{status_code::unset};
    std::string status_message;
    
    /**
     * @brief Check if span has finished
     */
    bool is_finished() const {
        return end_time != std::chrono::system_clock::time_point{};
    }
    
    /**
     * @brief Calculate duration if span is finished
     */
    void calculate_duration() {
        if (is_finished()) {
            duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end_time - start_time
            );
        }
    }
};

/**
 * @brief Trace context for propagation across service boundaries
 */
struct trace_context {
    std::string trace_id;
    std::string span_id;
    std::string trace_flags;
    std::string trace_state;
    std::unordered_map<std::string, std::string> baggage;
    
    /**
     * @brief Serialize to W3C Trace Context format
     */
    std::string to_w3c_traceparent() const {
        // Version-TraceId-SpanId-TraceFlags
        return "00-" + trace_id + "-" + span_id + "-" + trace_flags;
    }
    
    /**
     * @brief Parse from W3C Trace Context format
     */
    static monitoring_system::result<trace_context> from_w3c_traceparent(const std::string& header) {
        // Parse format: version-traceid-spanid-traceflags
        if (header.length() < 55) {  // Minimum valid length
            return monitoring_system::make_error<trace_context>(monitoring_system::monitoring_error_code::invalid_argument);
        }
        
        trace_context ctx;
        // Simple parsing - production code would be more robust
        size_t pos = 0;
        size_t dash1 = header.find('-', pos);
        size_t dash2 = header.find('-', dash1 + 1);
        size_t dash3 = header.find('-', dash2 + 1);
        
        if (dash1 == std::string::npos || dash2 == std::string::npos || dash3 == std::string::npos) {
            return monitoring_system::make_error<trace_context>(monitoring_system::monitoring_error_code::invalid_argument);
        }
        
        ctx.trace_id = header.substr(dash1 + 1, dash2 - dash1 - 1);
        ctx.span_id = header.substr(dash2 + 1, dash3 - dash2 - 1);
        ctx.trace_flags = header.substr(dash3 + 1);
        
        return ctx;
    }
};

/**
 * @brief Span builder for creating new spans
 */
class span_builder {
private:
    trace_span span_;
    
public:
    span_builder& with_trace_id(const std::string& id) {
        span_.trace_id = id;
        return *this;
    }
    
    span_builder& with_parent(const std::string& parent_id) {
        span_.parent_span_id = parent_id;
        return *this;
    }
    
    span_builder& with_operation(const std::string& name) {
        span_.operation_name = name;
        return *this;
    }
    
    span_builder& with_service(const std::string& name) {
        span_.service_name = name;
        return *this;
    }
    
    span_builder& with_tag(const std::string& key, const std::string& value) {
        span_.tags[key] = value;
        return *this;
    }
    
    span_builder& with_baggage(const std::string& key, const std::string& value) {
        span_.baggage[key] = value;
        return *this;
    }
    
    trace_span build() {
        if (span_.span_id.empty()) {
            span_.span_id = thread_context_manager::generate_request_id();
        }
        if (span_.trace_id.empty()) {
            span_.trace_id = thread_context_manager::generate_correlation_id();
        }
        span_.start_time = std::chrono::system_clock::now();
        return span_;
    }
};

/**
 * @brief Distributed tracer for managing spans and traces
 */
class distributed_tracer {
private:
    struct tracer_impl;
    std::unique_ptr<tracer_impl> impl_;
    
public:
    distributed_tracer();
    ~distributed_tracer();
    
    // Disable copy
    distributed_tracer(const distributed_tracer&) = delete;
    distributed_tracer& operator=(const distributed_tracer&) = delete;
    
    // Enable move
    distributed_tracer(distributed_tracer&&) noexcept;
    distributed_tracer& operator=(distributed_tracer&&) noexcept;
    
    /**
     * @brief Start a new root span
     */
    monitoring_system::result<std::shared_ptr<trace_span>> start_span(
        const std::string& operation_name,
        const std::string& service_name = "monitoring_system"
    );
    
    /**
     * @brief Start a child span
     */
    monitoring_system::result<std::shared_ptr<trace_span>> start_child_span(
        const trace_span& parent,
        const std::string& operation_name
    );
    
    /**
     * @brief Start a span from trace context (for incoming requests)
     */
    monitoring_system::result<std::shared_ptr<trace_span>> start_span_from_context(
        const trace_context& context,
        const std::string& operation_name
    );
    
    /**
     * @brief Finish a span
     */
    monitoring_system::result<bool> finish_span(std::shared_ptr<trace_span> span);
    
    /**
     * @brief Get current active span for this thread
     */
    std::shared_ptr<trace_span> get_current_span() const;
    
    /**
     * @brief Set current active span for this thread
     */
    void set_current_span(std::shared_ptr<trace_span> span);
    
    /**
     * @brief Extract trace context for propagation
     */
    trace_context extract_context(const trace_span& span) const;
    
    /**
     * @brief Inject trace context into carrier (e.g., HTTP headers)
     */
    template<typename Carrier>
    void inject_context(const trace_context& context, Carrier& carrier) {
        carrier["traceparent"] = context.to_w3c_traceparent();
        if (!context.trace_state.empty()) {
            carrier["tracestate"] = context.trace_state;
        }
        // Inject baggage
        for (const auto& [key, value] : context.baggage) {
            carrier["baggage-" + key] = value;
        }
    }
    
    /**
     * @brief Extract trace context from carrier
     */
    template<typename Carrier>
    monitoring_system::result<trace_context> extract_context_from_carrier(const Carrier& carrier) {
        auto traceparent_it = carrier.find("traceparent");
        if (traceparent_it == carrier.end()) {
            return monitoring_system::make_error<trace_context>(monitoring_system::monitoring_error_code::not_found);
        }
        
        auto ctx_result = trace_context::from_w3c_traceparent(traceparent_it->second);
        if (!ctx_result) {
            return monitoring_system::make_error<trace_context>(ctx_result.get_error().code);
        }
        
        auto ctx = ctx_result.value();
        
        // Extract tracestate if present
        auto tracestate_it = carrier.find("tracestate");
        if (tracestate_it != carrier.end()) {
            ctx.trace_state = tracestate_it->second;
        }
        
        // Extract baggage
        for (const auto& [key, value] : carrier) {
            if (key.find("baggage-") == 0) {  // C++17 compatible
                ctx.baggage[key.substr(8)] = value;
            }
        }
        
        return ctx;
    }
    
    /**
     * @brief Get all spans for a trace
     */
    monitoring_system::result<std::vector<trace_span>> get_trace(const std::string& trace_id) const;
    
    /**
     * @brief Export spans to external system
     */
    monitoring_system::result<bool> export_spans(std::vector<trace_span> spans);
};

/**
 * @brief Scoped span for RAII-style span management
 */
class scoped_span {
private:
    std::shared_ptr<trace_span> span_;
    distributed_tracer* tracer_;
    
public:
    scoped_span(std::shared_ptr<trace_span> span, distributed_tracer* tracer)
        : span_(span), tracer_(tracer) {
        if (tracer_) {
            tracer_->set_current_span(span_);
        }
    }
    
    ~scoped_span() {
        if (span_ && tracer_) {
            tracer_->finish_span(span_);
        }
    }
    
    // Disable copy
    scoped_span(const scoped_span&) = delete;
    scoped_span& operator=(const scoped_span&) = delete;
    
    // Enable move
    scoped_span(scoped_span&& other) noexcept
        : span_(std::move(other.span_)), tracer_(other.tracer_) {
        other.tracer_ = nullptr;
    }
    
    scoped_span& operator=(scoped_span&& other) noexcept {
        if (this != &other) {
            if (span_ && tracer_) {
                tracer_->finish_span(span_);
            }
            span_ = std::move(other.span_);
            tracer_ = other.tracer_;
            other.tracer_ = nullptr;
        }
        return *this;
    }
    
    trace_span* operator->() { return span_.get(); }
    const trace_span* operator->() const { return span_.get(); }
    trace_span& operator*() { return *span_; }
    const trace_span& operator*() const { return *span_; }
};

/**
 * @brief Global tracer instance
 */
distributed_tracer& global_tracer();

/**
 * @brief Helper macro for creating a scoped span
 */
#define TRACE_SPAN(operation_name) \
    auto _span_result = monitoring_system::global_tracer().start_span(operation_name); \
    monitoring_system::scoped_span _scoped_span( \
        _span_result ? _span_result.value() : nullptr, \
        &monitoring_system::global_tracer() \
    )

#define TRACE_CHILD_SPAN(parent, operation_name) \
    auto _child_span_result = monitoring_system::global_tracer().start_child_span(parent, operation_name); \
    monitoring_system::scoped_span _child_scoped_span( \
        _child_span_result ? _child_span_result.value() : nullptr, \
        &monitoring_system::global_tracer() \
    )

} // namespace monitoring_system