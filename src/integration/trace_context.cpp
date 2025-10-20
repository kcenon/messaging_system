#include "messaging_system/integration/trace_context.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace messaging {

// Thread-local storage for trace ID
thread_local std::string TraceContext::current_trace_id_;

void TraceContext::set_trace_id(const std::string& id) {
    current_trace_id_ = id;
}

std::string TraceContext::get_trace_id() {
    if (current_trace_id_.empty()) {
        return generate_trace_id();
    }
    return current_trace_id_;
}

std::string TraceContext::generate_trace_id() {
    // Generate a simple trace ID: timestamp-random
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    std::stringstream ss;
    ss << std::hex << std::setfill('0')
       << std::setw(12) << timestamp
       << "-"
       << std::setw(8) << dis(gen);

    return ss.str();
}

void TraceContext::clear() {
    current_trace_id_.clear();
}

// ScopedTrace implementation
ScopedTrace::ScopedTrace(const std::string& trace_id)
    : previous_trace_id_(TraceContext::get_trace_id())
{
    TraceContext::set_trace_id(trace_id);
}

ScopedTrace::~ScopedTrace() {
    if (previous_trace_id_.empty()) {
        TraceContext::clear();
    } else {
        TraceContext::set_trace_id(previous_trace_id_);
    }
}

} // namespace messaging
