#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <string>

namespace kcenon::logger {

// Forward declaration
struct log_entry;

/**
 * @interface log_formatter_interface
 * @brief Interface for log formatters
 * 
 * This interface defines the contract for formatting log entries.
 * Formatters convert log entries into string representations.
 */
class log_formatter_interface {
public:
    virtual ~log_formatter_interface() = default;
    
    /**
     * @brief Format a log entry into a string
     * @param entry The log entry to format
     * @return Formatted string representation
     */
    virtual std::string format(const log_entry& entry) const = 0;
    
    /**
     * @brief Get the format type name
     * @return Format type (e.g., "json", "plain", "xml")
     */
    virtual std::string get_format_type() const = 0;
};

} // namespace kcenon::logger