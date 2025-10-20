#pragma once

#include <string>
#include <random>

namespace messaging {

class TraceContext {
    static thread_local std::string current_trace_id_;

public:
    static void set_trace_id(const std::string& id);
    static std::string get_trace_id();
    static std::string generate_trace_id();
    static void clear();
};

// RAII helper for scoped trace context
class ScopedTrace {
    std::string previous_trace_id_;

public:
    explicit ScopedTrace(const std::string& trace_id);
    ~ScopedTrace();
};

} // namespace messaging
