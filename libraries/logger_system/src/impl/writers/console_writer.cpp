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

#include <kcenon/logger/writers/console_writer.h>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace kcenon::logger {

console_writer::console_writer(bool use_stderr, bool auto_detect_color)
    : use_stderr_(use_stderr) {
    if (auto_detect_color) {
        set_use_color(is_color_supported());
    }
}

console_writer::~console_writer() {
    flush();
}

result_void console_writer::write(logger_system::log_level level,
                                  const std::string& message,
                                  const std::string& file,
                                  int line,
                                  const std::string& function,
                                  const std::chrono::system_clock::time_point& timestamp) {
    std::lock_guard<std::mutex> lock(write_mutex_);
    
    auto& stream = (use_stderr_ || level <= logger_system::log_level::error) 
                   ? std::cerr : std::cout;
    
    if (use_color()) {
        stream << level_to_color(level);
    }
    
    stream << format_log_entry(level, message, file, line, function, timestamp);
    
    if (use_color()) {
        stream << "\033[0m"; // Reset color
    }
    
    stream << std::endl;
    
    // Check for stream failure
    if (stream.fail()) {
        return make_logger_error(logger_error_code::processing_failed, 
                                "Console write failed");
    }
    
    return {}; // Success
}

result_void console_writer::flush() {
    std::lock_guard<std::mutex> lock(write_mutex_);
    std::cout.flush();
    std::cerr.flush();
    
    if (std::cout.fail() || std::cerr.fail()) {
        return make_logger_error(logger_error_code::flush_timeout,
                                "Console flush failed");
    }
    
    return {}; // Success
}

void console_writer::set_use_stderr(bool use_stderr) {
    use_stderr_ = use_stderr;
}

bool console_writer::is_color_supported() const {
#ifdef _WIN32
    // Check if running in Windows Terminal or if ANSI is enabled
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return false;
    }
    
    // Check for ENABLE_VIRTUAL_TERMINAL_PROCESSING flag
    return (dwMode & 0x0004) != 0;
#else
    // Check if output is to a terminal and TERM is set
    const char* term = std::getenv("TERM");
    return isatty(STDOUT_FILENO) && term && std::string(term) != "dumb";
#endif
}

// base_writer implementation
std::string base_writer::format_log_entry(logger_system::log_level level,
                                         const std::string& message,
                                         const std::string& file,
                                         int line,
                                         const std::string& function,
                                         const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    oss << "[" << level_to_string(level) << "] ";
    
    if (!file.empty()) {
        // Extract filename from path
        size_t pos = file.find_last_of("/\\");
        std::string filename = (pos != std::string::npos) ? file.substr(pos + 1) : file;
        oss << filename << ":" << line << " (" << function << ") ";
    }
    
    oss << message;
    
    return oss.str();
}

std::string base_writer::level_to_string(logger_system::log_level level) const {
    switch (level) {
        case logger_system::log_level::fatal: return "CRITICAL";
        case logger_system::log_level::error:    return "ERROR";
        case logger_system::log_level::warn:  return "WARNING";
        case logger_system::log_level::info:     return "INFO";
        case logger_system::log_level::debug:    return "DEBUG";
        case logger_system::log_level::trace:    return "TRACE";
        case logger_system::log_level::off:      return "OFF";
    }
    return "UNKNOWN";
}

std::string base_writer::level_to_color(logger_system::log_level level) const {
    if (!use_color()) return "";
    
    switch (level) {
        case logger_system::log_level::fatal: return "\033[1;35m"; // Bright Magenta
        case logger_system::log_level::error:    return "\033[1;31m"; // Bright Red
        case logger_system::log_level::warn:  return "\033[1;33m"; // Bright Yellow
        case logger_system::log_level::info:     return "\033[1;32m"; // Bright Green
        case logger_system::log_level::debug:    return "\033[1;36m"; // Bright Cyan
        case logger_system::log_level::trace:    return "\033[1;37m"; // Bright White
        case logger_system::log_level::off:      return ""; // No color for off
    }
    return "";
}

} // namespace kcenon::logger