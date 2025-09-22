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

#include <kcenon/logger/core/logger.h>
#include <kcenon/logger/writers/base_writer.h>
#include <kcenon/logger/interfaces/logger_types.h>
#include <iostream>
#include <vector>
#include <chrono>

namespace kcenon::logger {

#ifdef USE_THREAD_SYSTEM_INTEGRATION
// Helper function to convert kcenon::thread::log_level to logger_system::log_level
logger_system::log_level convert_log_level(kcenon::thread::log_level level) {
    switch (level) {
        case kcenon::thread::log_level::critical: return logger_system::log_level::fatal;
        case kcenon::thread::log_level::error: return logger_system::log_level::error;
        case kcenon::thread::log_level::warning: return logger_system::log_level::warn;
        case kcenon::thread::log_level::info: return logger_system::log_level::info;
        case kcenon::thread::log_level::debug: return logger_system::log_level::debug;
        case kcenon::thread::log_level::trace: return logger_system::log_level::trace;
        default: return logger_system::log_level::info;
    }
}
#else
// In standalone mode, no conversion needed - both types are the same
logger_system::log_level convert_log_level(logger_system::log_level level) {
    return level;
}
#endif

// Simple implementation class for logger PIMPL
class logger::impl {
public:
    bool async_mode_;
    std::size_t buffer_size_;
    bool running_;
#ifdef USE_THREAD_SYSTEM_INTEGRATION
    kcenon::thread::log_level min_level_;
#else
    logger_system::log_level min_level_;
#endif
    std::vector<std::unique_ptr<base_writer>> writers_;

    impl(bool async, std::size_t buffer_size)
#ifdef USE_THREAD_SYSTEM_INTEGRATION
        : async_mode_(async), buffer_size_(buffer_size), running_(false), min_level_(kcenon::thread::log_level::info) {
#else
        : async_mode_(async), buffer_size_(buffer_size), running_(false), min_level_(logger_system::log_level::info) {
#endif
    }
};

logger::logger(bool async, std::size_t buffer_size)
    : pimpl_(std::make_unique<impl>(async, buffer_size)) {
    // Initialize logger with configuration
}

logger::~logger() {
    if (pimpl_ && pimpl_->running_) {
        stop();
    }
}

result_void logger::start() {
    if (pimpl_ && !pimpl_->running_) {
        pimpl_->running_ = true;
        // Initialize async processing if needed
    }
    return result_void{};
}

result_void logger::stop() {
    if (pimpl_ && pimpl_->running_) {
        flush();
        pimpl_->running_ = false;
    }
    return result_void{};
}

bool logger::is_running() const {
    return pimpl_ && pimpl_->running_;
}

result_void logger::add_writer(std::unique_ptr<base_writer> writer) {
    if (pimpl_ && writer) {
        pimpl_->writers_.push_back(std::move(writer));
    }
    return result_void{};
}

#ifdef USE_THREAD_SYSTEM_INTEGRATION
void logger::set_min_level(kcenon::thread::log_level level) {
#else
void logger::set_min_level(logger_system::log_level level) {
#endif
    if (pimpl_) {
        pimpl_->min_level_ = level;
    }
}

#ifdef USE_THREAD_SYSTEM_INTEGRATION
kcenon::thread::log_level logger::get_min_level() const {
    return pimpl_ ? pimpl_->min_level_ : kcenon::thread::log_level::info;
#else
logger_system::log_level logger::get_min_level() const {
    return pimpl_ ? pimpl_->min_level_ : logger_system::log_level::info;
#endif
}

#ifdef USE_THREAD_SYSTEM_INTEGRATION
void logger::log(kcenon::thread::log_level level, const std::string& message) {
#else
void logger::log(logger_system::log_level level, const std::string& message) {
#endif
    if (pimpl_ && level <= pimpl_->min_level_) {
        for (auto& writer : pimpl_->writers_) {
            if (writer) {
                // Create a simple log entry and write it
                auto now = std::chrono::system_clock::now();
                writer->write(convert_log_level(level), message, "", 0, "", now);
            }
        }
    }
}

#ifdef USE_THREAD_SYSTEM_INTEGRATION
void logger::log(kcenon::thread::log_level level, const std::string& message,
                const std::string& file, int line, const std::string& function) {
#else
void logger::log(logger_system::log_level level, const std::string& message,
                const std::string& file, int line, const std::string& function) {
#endif
    if (pimpl_ && level <= pimpl_->min_level_) {
        for (auto& writer : pimpl_->writers_) {
            if (writer) {
                // Create a log entry with source location
                auto now = std::chrono::system_clock::now();
                writer->write(convert_log_level(level), message, file, line, function, now);
            }
        }
    }
}

#ifdef USE_THREAD_SYSTEM_INTEGRATION
bool logger::is_enabled(kcenon::thread::log_level level) const {
#else
bool logger::is_enabled(logger_system::log_level level) const {
#endif
    return pimpl_ && level <= pimpl_->min_level_;
}

void logger::flush() {
    if (pimpl_) {
        for (auto& writer : pimpl_->writers_) {
            if (writer) {
                writer->flush();
            }
        }
    }
}

} // namespace kcenon::logger