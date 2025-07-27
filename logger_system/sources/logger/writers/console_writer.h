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

#include "base_writer.h"
#include <mutex>

namespace logger_module {

/**
 * @brief Console writer that outputs logs to stdout/stderr
 * 
 * Features:
 * - Color support for different log levels (if terminal supports it)
 * - Thread-safe console output
 * - Error levels go to stderr, others to stdout
 */
class console_writer : public base_writer {
public:
    /**
     * @brief Constructor
     * @param use_stderr If true, all output goes to stderr (default: false)
     * @param auto_detect_color Auto-detect terminal color support (default: true)
     */
    explicit console_writer(bool use_stderr = false, bool auto_detect_color = true);
    
    /**
     * @brief Destructor
     */
    ~console_writer() override;
    
    /**
     * @brief Write log entry to console
     */
    bool write(thread_module::log_level level,
               const std::string& message,
               const std::string& file,
               int line,
               const std::string& function,
               const std::chrono::system_clock::time_point& timestamp) override;
    
    /**
     * @brief Flush console output
     */
    void flush() override;
    
    /**
     * @brief Get writer name
     */
    std::string get_name() const override { return "console"; }
    
    /**
     * @brief Set whether to use stderr for all output
     * @param use_stderr Enable/disable stderr usage
     */
    void set_use_stderr(bool use_stderr);
    
private:
    bool use_stderr_;
    std::mutex write_mutex_;
    
    /**
     * @brief Check if terminal supports color
     * @return true if color is supported
     */
    bool is_color_supported() const;
};

} // namespace logger_module