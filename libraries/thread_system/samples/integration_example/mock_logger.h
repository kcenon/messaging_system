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

#include "../../sources/interfaces/logger_interface.h"
#include <iostream>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <chrono>

/**
 * @brief Mock logger implementation for demonstration
 * 
 * In a real application, this would be replaced with:
 * #include <logger_system/logger.h>
 * using logger_module::logger;
 */
class mock_logger : public thread_module::logger_interface {
public:
    mock_logger() : min_level_(thread_module::log_level::trace) {}
    
    void log(thread_module::log_level level, const std::string& message) override {
        if (level < min_level_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stream = (level <= thread_module::log_level::error) ? std::cerr : std::cout;
        
        stream << "[" << format_time() << "] "
               << "[" << level_to_string(level) << "] "
               << message << std::endl;
    }
    
    void log(thread_module::log_level level, const std::string& message,
             const std::string& file, int line, const std::string& function) override {
        if (level < min_level_) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        auto& stream = (level <= thread_module::log_level::error) ? std::cerr : std::cout;
        
        stream << "[" << format_time() << "] "
               << "[" << level_to_string(level) << "] ";
        
        if (!file.empty()) {
            size_t pos = file.find_last_of("/\\");
            std::string filename = (pos != std::string::npos) ? file.substr(pos + 1) : file;
            stream << filename << ":" << line << " (" << function << ") ";
        }
        
        stream << message << std::endl;
    }
    
    bool is_enabled(thread_module::log_level level) const override {
        return level >= min_level_;
    }
    
    void flush() override {
        std::cout.flush();
        std::cerr.flush();
    }
    
    void start() {
        std::cout << "[MockLogger] Started" << std::endl;
    }
    
    void stop() {
        flush();
        std::cout << "[MockLogger] Stopped" << std::endl;
    }
    
    void set_min_level(thread_module::log_level level) {
        min_level_ = level;
    }
    
private:
    std::string format_time() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        oss << "." << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }
    
    std::string level_to_string(thread_module::log_level level) const {
        switch (level) {
            case thread_module::log_level::critical: return "CRITICAL";
            case thread_module::log_level::error:    return "ERROR";
            case thread_module::log_level::warning:  return "WARNING";
            case thread_module::log_level::info:     return "INFO";
            case thread_module::log_level::debug:    return "DEBUG";
            case thread_module::log_level::trace:    return "TRACE";
        }
        return "UNKNOWN";
    }
    
private:
    thread_module::log_level min_level_;
    mutable std::mutex mutex_;
};