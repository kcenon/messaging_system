#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include <string>
#include <chrono>
#include <kcenon/logger/core/error_codes.h>

namespace kcenon::logger {

// Forward declaration
struct log_entry;

/**
 * @interface log_writer_interface
 * @brief Interface for log writers
 * 
 * This interface defines the contract for all log writers.
 * Follows the Single Responsibility Principle - only responsible for writing logs.
 */
class log_writer_interface {
public:
    virtual ~log_writer_interface() = default;
    
    /**
     * @brief Write a log entry
     * @param entry The log entry to write
     * @return result_void indicating success or failure
     */
    virtual result_void write(const log_entry& entry) = 0;
    
    /**
     * @brief Flush any buffered data
     * @return result_void indicating success or failure
     */
    virtual result_void flush() = 0;
    
    /**
     * @brief Get the name of this writer
     * @return Writer name for identification
     */
    virtual std::string get_name() const = 0;
    
    /**
     * @brief Check if the writer is healthy
     * @return true if the writer is operational
     */
    virtual bool is_healthy() const = 0;
};

} // namespace kcenon::logger