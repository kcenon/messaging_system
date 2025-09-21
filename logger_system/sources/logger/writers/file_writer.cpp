/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "file_writer.h"
#include <filesystem>
#include <iostream>

namespace logger_module {

file_writer::file_writer(const std::string& filename, bool append, size_t buffer_size)
    : filename_(filename)
    , append_mode_(append)
    , buffer_size_(buffer_size)
    , buffer_(std::make_unique<char[]>(buffer_size)) {
    open();
}

file_writer::~file_writer() {
    close();
}

bool file_writer::write(thread_module::log_level level,
                       const std::string& message,
                       const std::string& file,
                       int line,
                       const std::string& function,
                       const std::chrono::system_clock::time_point& timestamp) {
    std::lock_guard<std::mutex> lock(write_mutex_);
    
    if (!file_stream_.is_open()) {
        return false;
    }
    
    std::string formatted = format_log_entry(level, message, file, line, function, timestamp);
    
    try {
        file_stream_ << formatted << std::endl;
        bytes_written_.fetch_add(formatted.size() + 1);  // +1 for newline
        return file_stream_.good();
    } catch (const std::exception& e) {
        std::cerr << "File write error: " << e.what() << std::endl;
        return false;
    }
}

void file_writer::flush() {
    std::lock_guard<std::mutex> lock(write_mutex_);
    if (file_stream_.is_open()) {
        file_stream_.flush();
    }
}

bool file_writer::reopen() {
    std::lock_guard<std::mutex> lock(write_mutex_);
    close();
    return open();
}

void file_writer::close() {
    if (file_stream_.is_open()) {
        file_stream_.flush();
        file_stream_.close();
    }
}

bool file_writer::open() {
    try {
        // Create directory if it doesn't exist
        std::filesystem::path file_path(filename_);
        std::filesystem::path dir = file_path.parent_path();
        if (!dir.empty() && !std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }
        
        // Open file
        auto mode = append_mode_ ? std::ios::app : std::ios::trunc;
        file_stream_.open(filename_, std::ios::out | mode);
        
        if (file_stream_.is_open()) {
            // Set buffer
            file_stream_.rdbuf()->pubsetbuf(buffer_.get(), buffer_size_);
            
            // Get current file size if appending
            if (append_mode_) {
                file_stream_.seekp(0, std::ios::end);
                bytes_written_ = file_stream_.tellp();
            } else {
                bytes_written_ = 0;
            }
            
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to open log file: " << e.what() << std::endl;
    }
    
    return false;
}

} // namespace logger_module