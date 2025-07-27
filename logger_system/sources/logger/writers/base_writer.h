#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <string>
#include <chrono>
#include "../logger_interface.h"

namespace logger_module {

/**
 * @brief Base class for all log writers
 * 
 * Writers are responsible for outputting log messages to various destinations
 * such as console, files, network, etc.
 */
class base_writer {
public:
    virtual ~base_writer() = default;
    
    /**
     * @brief Write a log entry
     * @param level Log level
     * @param message Log message
     * @param file Source file (optional)
     * @param line Source line (optional)
     * @param function Function name (optional)
     * @param timestamp Time of log entry
     * @return true if write was successful
     */
    virtual bool write(thread_module::log_level level,
                      const std::string& message,
                      const std::string& file,
                      int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) = 0;
    
    /**
     * @brief Flush any buffered data
     */
    virtual void flush() = 0;
    
    /**
     * @brief Set whether to use color output (if supported)
     * @param use_color Enable/disable color
     */
    virtual void set_use_color(bool use_color) {
        use_color_ = use_color;
    }
    
    /**
     * @brief Get color setting
     * @return true if color is enabled
     */
    bool use_color() const {
        return use_color_;
    }
    
    /**
     * @brief Get writer name
     * @return Name of the writer
     */
    virtual std::string get_name() const = 0;
    
protected:
    /**
     * @brief Format a log entry to string
     * @param level Log level
     * @param message Log message
     * @param file Source file
     * @param line Source line
     * @param function Function name
     * @param timestamp Time of log entry
     * @return Formatted log string
     */
    std::string format_log_entry(thread_module::log_level level,
                                const std::string& message,
                                const std::string& file,
                                int line,
                                const std::string& function,
                                const std::chrono::system_clock::time_point& timestamp);
    
    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return String representation
     */
    std::string level_to_string(thread_module::log_level level) const;
    
    /**
     * @brief Get ANSI color code for log level
     * @param level Log level
     * @return ANSI color code or empty string if color disabled
     */
    std::string level_to_color(thread_module::log_level level) const;
    
private:
    bool use_color_ = true;
};

} // namespace logger_module