#pragma once

// Compatibility header - enhanced stub implementation
#include <string>
#include <memory>
#include <unordered_map>
#include <map>
#include <optional>
#include <chrono>

namespace monitoring_system {

// Forward declarations
class trace_span;
class trace_context;
template<typename T> class result;

// Trace context for propagation
class trace_context {
public:
    std::string trace_id;
    std::string span_id;
    std::unordered_map<std::string, std::string> baggage;

    trace_context(std::string tid = "", std::string sid = "")
        : trace_id(std::move(tid)), span_id(std::move(sid)) {}
};

// Enhanced trace span with all required fields
class trace_span {
public:
    enum class status_code {
        ok = 0,
        cancelled = 1,
        unknown = 2,
        invalid_argument = 3,
        deadline_exceeded = 4,
        not_found = 5,
        already_exists = 6,
        permission_denied = 7,
        resource_exhausted = 8,
        failed_precondition = 9,
        aborted = 10,
        out_of_range = 11,
        unimplemented = 12,
        internal = 13,
        unavailable = 14,
        data_loss = 15,
        unauthenticated = 16,
        error = 13  // Alias for internal
    };

    std::string name;
    std::string operation_name;  // Alias for name
    std::string trace_id;
    std::string span_id;
    std::string parent_span_id;
    std::unordered_map<std::string, std::string> tags;
    std::unordered_map<std::string, std::string> baggage;  // For context propagation
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    status_code status{status_code::ok};
    std::string status_message{"OK"};

    explicit trace_span(std::string span_name)
        : name(span_name)
        , operation_name(std::move(span_name))
        , trace_id("trace-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))
        , span_id("span-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))
        , start_time(std::chrono::steady_clock::now()) {}

    void set_tag(const std::string& key, const std::string& value) {
        tags[key] = value;
    }

    void set_status(status_code code, const std::string& msg = "") {
        status = code;
        if (!msg.empty()) {
            status_message = msg;
        }
    }

    void finish() {
        end_time = std::chrono::steady_clock::now();
    }
};

// Basic result type for compatibility
template<typename T>
class result {
public:
    result(T value) : value_(std::move(value)), has_value_(true) {}
    result() : has_value_(false) {}

    explicit operator bool() const { return has_value_; }
    T& value() { return value_; }
    const T& value() const { return value_; }

private:
    T value_;
    bool has_value_;
};

// Enhanced distributed tracer with all required methods
class distributed_tracer {
public:
    static distributed_tracer& instance() {
        static distributed_tracer inst;
        return inst;
    }

    // Basic span creation
    std::shared_ptr<trace_span> start_span(const std::string& name) {
        return std::make_shared<trace_span>(name);
    }

    // Overloaded span creation with parent service
    result<std::shared_ptr<trace_span>> start_span(const std::string& name, const std::string& service) {
        auto span = std::make_shared<trace_span>(name);
        span->set_tag("service.name", service);
        return result<std::shared_ptr<trace_span>>(span);
    }

    // Context extraction from headers
    result<trace_context> extract_context_from_carrier(const std::map<std::string, std::string>& headers) {
        auto it = headers.find("trace-id");
        if (it != headers.end()) {
            trace_context ctx;
            ctx.trace_id = it->second;
            auto span_it = headers.find("span-id");
            if (span_it != headers.end()) {
                ctx.span_id = span_it->second;
            }
            return result<trace_context>(std::move(ctx));
        }
        return result<trace_context>(); // Empty result
    }

    // Start span from existing context
    result<std::shared_ptr<trace_span>> start_span_from_context(const trace_context& context, const std::string& name) {
        auto span = std::make_shared<trace_span>(name);
        span->trace_id = context.trace_id;
        span->parent_span_id = context.span_id;
        return result<std::shared_ptr<trace_span>>(span);
    }

    // Start child span
    result<std::shared_ptr<trace_span>> start_child_span(std::shared_ptr<trace_span> parent, const std::string& name) {
        auto child = std::make_shared<trace_span>(name);
        child->trace_id = parent->trace_id;
        child->parent_span_id = parent->span_id;
        return result<std::shared_ptr<trace_span>>(child);
    }

    // Finish span
    void finish_span(std::shared_ptr<trace_span> span) {
        if (span) {
            span->finish();
        }
    }

    // Inject context into headers
    void inject_context_into_carrier(const trace_context& context, std::map<std::string, std::string>& headers) {
        headers["trace-id"] = context.trace_id;
        headers["span-id"] = context.span_id;
    }

    // Get context from span
    trace_context get_context_from_span(std::shared_ptr<trace_span> span) {
        if (span) {
            return trace_context(span->trace_id, span->span_id);
        }
        return trace_context();
    }

    // Get trace information (returns vector of spans for a trace)
    result<std::vector<std::shared_ptr<trace_span>>> get_trace(const std::string& trace_id) {
        (void)trace_id; // Suppress unused parameter warning
        // Stub implementation - return empty vector
        std::vector<std::shared_ptr<trace_span>> spans;
        return result<std::vector<std::shared_ptr<trace_span>>>(spans);
    }

    // Get current span (stub implementation)
    std::shared_ptr<trace_span> get_current_span() {
        // Return null in stub implementation
        return nullptr;
    }
};

// Global tracer accessor function
inline distributed_tracer& global_tracer() {
    return distributed_tracer::instance();
}

} // namespace monitoring_system