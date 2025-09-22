#pragma once

#include <string>
#include <chrono>
#include <optional>
#include <unordered_map>

namespace monitoring_system {

/**
 * @brief Context metadata for thread-specific information
 */
struct context_metadata {
    std::string request_id;
    std::string correlation_id;
    std::string user_id;
    std::unordered_map<std::string, std::string> tags;

    explicit context_metadata(std::string req_id = "")
        : request_id(std::move(req_id)) {}

    bool empty() const {
        return request_id.empty() && correlation_id.empty() && user_id.empty() && tags.empty();
    }

    void set_tag(const std::string& key, const std::string& value) {
        tags[key] = value;
    }

    std::string get_tag(const std::string& key) const {
        auto it = tags.find(key);
        return it != tags.end() ? it->second : "";
    }
};

/**
 * @brief Enhanced thread context for comprehensive tracking
 */
struct thread_context_data {
    std::string request_id;
    std::string correlation_id;
    std::string user_id;
    std::string span_id;
    std::string trace_id;
    std::chrono::steady_clock::time_point start_time;
    std::optional<std::string> parent_span_id;
    std::unordered_map<std::string, std::string> tags;

    thread_context_data() : start_time(std::chrono::steady_clock::now()) {}

    explicit thread_context_data(std::string req_id)
        : request_id(std::move(req_id))
        , start_time(std::chrono::steady_clock::now()) {}

    void add_tag(const std::string& key, const std::string& value) {
        tags[key] = value;
    }

    std::string get_tag(const std::string& key) const {
        auto it = tags.find(key);
        return it != tags.end() ? it->second : "";
    }
};

/**
 * @brief Thread-local context management
 */
class thread_context {
public:
    // Create new context
    static thread_context_data& create(const std::string& request_id = "");

    // Get current context
    static thread_context_data* current();

    // Check if context exists
    static bool has_context();

    // Clear current context
    static void clear();

    // ID generation
    static std::string generate_request_id();
    static std::string generate_correlation_id();

    // Copy from another context
    static bool copy_from(const thread_context_data& source);

private:
    static thread_local std::unique_ptr<thread_context_data> current_context_;
};

/**
 * @brief Thread-local context storage (legacy compatibility)
 */
class thread_context_manager {
public:
    static void set_context(const thread_context_data& context);
    static std::optional<thread_context_data> get_context();
    static void clear_context();
    static std::string generate_request_id();
    static std::string generate_correlation_id();

private:
    static thread_local std::optional<thread_context_data> current_context_;
};

} // namespace monitoring_system