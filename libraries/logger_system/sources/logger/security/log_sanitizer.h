#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "../filters/log_filter.h"
#include <regex>
#include <unordered_map>
#include <functional>

namespace logger_module {

/**
 * @class log_sanitizer
 * @brief Sanitizes sensitive information from log messages
 */
class log_sanitizer {
public:
    /**
     * @struct sanitization_rule
     * @brief Rule for sanitizing specific patterns
     */
    struct sanitization_rule {
        std::string name;
        std::regex pattern;
        std::function<std::string(const std::smatch&)> replacer;
    };
    
    /**
     * @brief Constructor
     */
    log_sanitizer();
    
    /**
     * @brief Add default sanitization rules
     */
    void add_default_rules();
    
    /**
     * @brief Add custom sanitization rule
     * @param rule The sanitization rule
     */
    void add_rule(const sanitization_rule& rule);
    
    /**
     * @brief Remove a sanitization rule
     * @param name Rule name
     */
    void remove_rule(const std::string& name);
    
    /**
     * @brief Sanitize a message
     * @param message The message to sanitize
     * @return Sanitized message
     */
    std::string sanitize(const std::string& message) const;
    
    /**
     * @brief Enable/disable specific rules
     * @param name Rule name
     * @param enabled Whether to enable the rule
     */
    void set_rule_enabled(const std::string& name, bool enabled);
    
    /**
     * @brief Check if sanitization changed the message
     * @param original Original message
     * @param sanitized Sanitized message
     * @return true if message was modified
     */
    static bool was_sanitized(const std::string& original, const std::string& sanitized);
    
    // Predefined sanitizers
    static std::string mask_credit_card(const std::smatch& match);
    static std::string mask_ssn(const std::smatch& match);
    static std::string mask_email(const std::smatch& match);
    static std::string mask_ip_address(const std::smatch& match);
    static std::string mask_api_key(const std::smatch& match);
    static std::string mask_password(const std::smatch& match);
    
private:
    std::vector<sanitization_rule> rules_;
    std::unordered_map<std::string, bool> rule_enabled_;
    mutable std::mutex rules_mutex_;
};

/**
 * @class sanitizing_filter
 * @brief Log filter that sanitizes messages before logging
 */
class sanitizing_filter : public log_filter {
public:
    /**
     * @brief Constructor
     * @param sanitizer The sanitizer to use
     * @param wrapped_filter Optional wrapped filter
     */
    explicit sanitizing_filter(std::shared_ptr<log_sanitizer> sanitizer,
                              std::unique_ptr<log_filter> wrapped_filter = nullptr);
    
    /**
     * @brief Check if log should be processed (and sanitize message)
     */
    bool should_log(thread_module::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override;
    
    /**
     * @brief Get the sanitized message
     * @return Last sanitized message
     */
    std::string get_sanitized_message() const {
        std::lock_guard<std::mutex> lock(message_mutex_);
        return sanitized_message_;
    }
    
private:
    std::shared_ptr<log_sanitizer> sanitizer_;
    std::unique_ptr<log_filter> wrapped_filter_;
    mutable std::string sanitized_message_;
    mutable std::mutex message_mutex_;
};

/**
 * @class access_control_filter
 * @brief Filter that controls access based on log level and source
 */
class access_control_filter : public log_filter {
public:
    /**
     * @brief Permission levels
     */
    enum class permission_level {
        none = 0,
        read_only = 1,
        write_info = 2,
        write_all = 3,
        admin = 4
    };
    
    /**
     * @brief Constructor
     * @param default_permission Default permission level
     */
    explicit access_control_filter(permission_level default_permission = permission_level::write_all);
    
    /**
     * @brief Set permission for a specific source file
     * @param file_pattern File pattern (regex)
     * @param permission Permission level
     */
    void set_file_permission(const std::string& file_pattern, permission_level permission);
    
    /**
     * @brief Set current user context
     * @param user_id User identifier
     * @param permission User's permission level
     */
    void set_user_context(const std::string& user_id, permission_level permission);
    
    /**
     * @brief Check if log is allowed
     */
    bool should_log(thread_module::log_level level,
                   const std::string& message,
                   const std::string& file,
                   int line,
                   const std::string& function) const override;
    
private:
    permission_level default_permission_;
    std::unordered_map<std::string, permission_level> file_permissions_;
    std::vector<std::pair<std::regex, permission_level>> file_patterns_;
    
    // Current user context
    std::string current_user_;
    permission_level current_user_permission_;
    mutable std::mutex permissions_mutex_;
    
    // Check if level is allowed for permission
    bool is_level_allowed(thread_module::log_level level, permission_level permission) const;
};

} // namespace logger_module