/**
 * @file thread_context.cpp
 * @brief Thread context implementation
 */

#include <kcenon/monitoring/context/thread_context.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace monitoring_system {

// New thread_context implementation
thread_local std::unique_ptr<thread_context_data> thread_context::current_context_;

thread_context_data& thread_context::create(const std::string& request_id) {
    if (request_id.empty()) {
        current_context_ = std::make_unique<thread_context_data>(generate_request_id());
    } else {
        current_context_ = std::make_unique<thread_context_data>(request_id);
    }
    return *current_context_;
}

thread_context_data* thread_context::current() {
    return current_context_.get();
}

bool thread_context::has_context() {
    return current_context_ != nullptr;
}

void thread_context::clear() {
    current_context_.reset();
}

bool thread_context::copy_from(const thread_context_data& source) {
    current_context_ = std::make_unique<thread_context_data>(source);
    return true;
}

// Legacy thread_context_manager implementation
thread_local std::optional<thread_context_data> thread_context_manager::current_context_;

void thread_context_manager::set_context(const thread_context_data& context) {
    current_context_ = context;
}

std::optional<thread_context_data> thread_context_manager::get_context() {
    return current_context_;
}

void thread_context_manager::clear_context() {
    current_context_.reset();
}

std::string thread_context::generate_request_id() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<uint64_t> dis;
    auto id = dis(gen);
    std::stringstream ss;
    ss << std::hex << id;
    return ss.str();
}

std::string thread_context::generate_correlation_id() {
    return generate_request_id(); // Simple implementation for now
}

std::string thread_context_manager::generate_request_id() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<uint64_t> dis;

    auto id = dis(gen);
    std::stringstream ss;
    ss << std::hex << id;
    return ss.str();
}

std::string thread_context_manager::generate_correlation_id() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<uint64_t> dis;

    auto id1 = dis(gen);
    auto id2 = dis(gen);
    std::stringstream ss;
    ss << std::hex << id1 << "-" << std::hex << id2;
    return ss.str();
}

} // namespace monitoring_system