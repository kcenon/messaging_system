/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "log_sanitizer.h"
#include <algorithm>

#include <iostream> // 명시적으로 포함

namespace logger_module {

log_sanitizer::log_sanitizer() {
    add_default_rules();
}

void log_sanitizer::add_default_rules() {
    // Credit card patterns
    add_rule({
        "credit_card",
        std::regex("\\b(?:\\d[ -]*?){13,19}\\b"),
        &log_sanitizer::mask_credit_card
    });
    
    // SSN pattern
    add_rule({
        "ssn",
        std::regex("\\b\\d{3}-\\d{2}-\\d{4}\\b"),
        &log_sanitizer::mask_ssn
    });
    
    // Email pattern
    add_rule({
        "email",
        std::regex("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Z|a-z]{2,}\\b"),
        &log_sanitizer::mask_email
    });
    
    // IP address pattern
    add_rule({
        "ip_address",
        std::regex("\\b\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\b"),
        &log_sanitizer::mask_ip_address
    });
    
    // API key patterns (common formats)
    add_rule({
        "api_key",
        std::regex("(api[_-]?key|apikey|key)\\s*[:=]\\s*['\"]?([A-Za-z0-9_\\-]{20,})['\"]?",
                  std::regex_constants::icase),
        &log_sanitizer::mask_api_key
    });
    
    // Password patterns
    add_rule({
        "password",
        std::regex("(password|passwd|pwd)\\s*[:=]\\s*['\"]?([^'\"\\s]+)['\"]?",
                  std::regex_constants::icase),
        &log_sanitizer::mask_password
    });
}

void log_sanitizer::add_rule(const sanitization_rule& rule) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    rules_.push_back(rule);
    rule_enabled_[rule.name] = true;
}

void log_sanitizer::remove_rule(const std::string& name) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    rules_.erase(
        std::remove_if(rules_.begin(), rules_.end(),
                      [&name](const sanitization_rule& rule) {
                          return rule.name == name;
                      }),
        rules_.end()
    );
    rule_enabled_.erase(name);
}

std::string log_sanitizer::sanitize(const std::string& message) const {
    std::string result = message;
    
    std::lock_guard<std::mutex> lock(rules_mutex_);
    
    for (const auto& rule : rules_) {
        // Check if rule is enabled
        auto it = rule_enabled_.find(rule.name);
        if (it != rule_enabled_.end() && !it->second) {
            continue;
        }
        
        // Apply rule
        std::string temp;
        std::sregex_iterator current(result.begin(), result.end(), rule.pattern);
        std::sregex_iterator end;
        
        if (current == end) {
            continue;  // No matches
        }
        
        size_t last_pos = 0;
        for (; current != end; ++current) {
            const std::smatch& match = *current;
            
            // Add text before match
            temp += result.substr(last_pos, match.position() - last_pos);
            
            // Add sanitized match
            temp += rule.replacer(match);
            
            last_pos = match.position() + match.length();
        }
        
        // Add remaining text
        temp += result.substr(last_pos);
        
        result = temp;
    }
    
    return result;
}

void log_sanitizer::set_rule_enabled(const std::string& name, bool enabled) {
    std::lock_guard<std::mutex> lock(rules_mutex_);
    rule_enabled_[name] = enabled;
}

bool log_sanitizer::was_sanitized(const std::string& original, const std::string& sanitized) {
    return original != sanitized;
}

// Predefined sanitizers
std::string log_sanitizer::mask_credit_card(const std::smatch& match) {
    std::string card = match.str();
    std::string digits;
    
    // Extract only digits
    for (char c : card) {
        if (std::isdigit(c)) {
            digits += c;
        }
    }
    
    if (digits.length() < 8) {
        return "****";
    }
    
    // Show first 4 and last 4 digits
    std::string masked = digits.substr(0, 4);
    for (size_t i = 4; i < digits.length() - 4; ++i) {
        masked += '*';
    }
    masked += digits.substr(digits.length() - 4);
    
    return masked;
}

std::string log_sanitizer::mask_ssn(const std::smatch& match) {
    // Format: XXX-XX-1234 (show last 4)
    std::string ssn = match.str();
    if (ssn.length() >= 4) {
        return "***-**-" + ssn.substr(ssn.length() - 4);
    }
    return "***-**-****";
}

std::string log_sanitizer::mask_email(const std::smatch& match) {
    std::string email = match.str();
    size_t at_pos = email.find('@');
    
    if (at_pos == std::string::npos || at_pos < 3) {
        return "****@****";
    }
    
    // Show first and last character of username
    std::string masked;
    masked += email[0];
    for (size_t i = 1; i < at_pos - 1; ++i) {
        masked += '*';
    }
    masked += email[at_pos - 1];
    masked += email.substr(at_pos);  // Keep domain
    
    return masked;
}

std::string log_sanitizer::mask_ip_address(const std::smatch& match) {
    // Mask last two octets: 192.168.*.* 
    std::string ip = match.str();
    size_t first_dot = ip.find('.');
    size_t second_dot = ip.find('.', first_dot + 1);
    
    if (second_dot != std::string::npos) {
        return ip.substr(0, second_dot) + ".*.*";
    }
    
    return "*.*.*.*";
}

std::string log_sanitizer::mask_api_key(const std::smatch& match) {
    // match[0] is full match, match[1] is key name, match[2] is key value
    if (match.size() >= 3) {
        std::string key_name = match[1];
        std::string key_value = match[2];
        
        if (key_value.length() > 8) {
            // Show first 4 and last 4 characters
            std::string masked = key_value.substr(0, 4);
            for (size_t i = 4; i < key_value.length() - 4; ++i) {
                masked += '*';
            }
            masked += key_value.substr(key_value.length() - 4);
            return key_name + "=" + masked;
        }
    }
    
    return match[1].str() + "=****";
}

std::string log_sanitizer::mask_password(const std::smatch& match) {
    // match[0] is full match, match[1] is password field name
    if (match.size() >= 2) {
        return match[1].str() + "=********";
    }
    return "password=********";
}

// Sanitizing filter implementation
sanitizing_filter::sanitizing_filter(std::shared_ptr<log_sanitizer> sanitizer,
                                   std::unique_ptr<log_filter> wrapped_filter)
    : sanitizer_(std::move(sanitizer))
    , wrapped_filter_(std::move(wrapped_filter)) {
}

bool sanitizing_filter::should_log(thread_module::log_level level,
                                  const std::string& message,
                                  const std::string& file,
                                  int line,
                                  const std::string& function) const {
    // Suppress unused parameter warnings
    (void)level;
    (void)file;
    (void)line;
    (void)function;
    
    // First check wrapped filter if present
    if (wrapped_filter_ && !wrapped_filter_->should_log(level, message, file, line, function)) {
        return false;
    }
    
    // Sanitize the message
    {
        std::lock_guard<std::mutex> lock(message_mutex_);
        sanitized_message_ = sanitizer_->sanitize(message);
    }
    
    return true;
}

// Access control filter implementation
access_control_filter::access_control_filter(permission_level default_permission)
    : default_permission_(default_permission)
    , current_user_("system")
    , current_user_permission_(permission_level::admin) {
}

void access_control_filter::set_file_permission(const std::string& file_pattern, 
                                               permission_level permission) {
    std::lock_guard<std::mutex> lock(permissions_mutex_);
    file_permissions_[file_pattern] = permission;
    file_patterns_.emplace_back(std::regex(file_pattern), permission);
}

void access_control_filter::set_user_context(const std::string& user_id, 
                                            permission_level permission) {
    std::lock_guard<std::mutex> lock(permissions_mutex_);
    current_user_ = user_id;
    current_user_permission_ = permission;
}

bool access_control_filter::should_log(thread_module::log_level level,
                                      const std::string& message,
                                      const std::string& file,
                                      int line,
                                      const std::string& function) const {
    // Suppress unused parameter warnings
    (void)message;
    (void)line;
    (void)function;
    
    std::lock_guard<std::mutex> lock(permissions_mutex_);
    
    // Check file-specific permissions
    permission_level required_permission = default_permission_;
    
    for (const auto& [pattern, permission] : file_patterns_) {
        if (std::regex_search(file, pattern)) {
            required_permission = permission;
            break;
        }
    }
    
    // Check if user has required permission
    if (current_user_permission_ < required_permission) {
        return false;
    }
    
    // Check if log level is allowed
    return is_level_allowed(level, current_user_permission_);
}

bool access_control_filter::is_level_allowed(thread_module::log_level level, 
                                            permission_level permission) const {
    switch (permission) {
        case permission_level::none:
            return false;
            
        case permission_level::read_only:
            // Can only read, not write logs
            return false;
            
        case permission_level::write_info:
            // Can write info and below
            return level <= thread_module::log_level::info;
            
        case permission_level::write_all:
        case permission_level::admin:
            // Can write all levels
            return true;
            
        default:
            return false;
    }
}

} // namespace logger_module