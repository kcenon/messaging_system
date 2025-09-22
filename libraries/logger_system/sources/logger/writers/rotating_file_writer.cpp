/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "rotating_file_writer.h"
#include <filesystem>
#include <algorithm>
#include <regex>
#include <iostream>

namespace logger_module {

rotating_file_writer::rotating_file_writer(const std::string& filename,
                                         size_t max_size,
                                         size_t max_files)
    : file_writer(filename, true)
    , rotation_type_(rotation_type::size)
    , max_size_(max_size)
    , max_files_(max_files)
    , last_rotation_time_(std::chrono::system_clock::now())
    , current_period_start_(std::chrono::system_clock::now()) {
    
    // Extract base filename and extension
    std::filesystem::path path(filename);
    base_filename_ = path.stem().string();
    file_extension_ = path.extension().string();
    if (file_extension_.empty()) {
        file_extension_ = ".log";
    }
}

rotating_file_writer::rotating_file_writer(const std::string& filename,
                                         rotation_type type,
                                         size_t max_files)
    : file_writer(filename, true)
    , rotation_type_(type)
    , max_size_(0)
    , max_files_(max_files)
    , last_rotation_time_(std::chrono::system_clock::now())
    , current_period_start_(std::chrono::system_clock::now()) {
    
    // Extract base filename and extension
    std::filesystem::path path(filename);
    base_filename_ = path.stem().string();
    file_extension_ = path.extension().string();
    if (file_extension_.empty()) {
        file_extension_ = ".log";
    }
}

rotating_file_writer::rotating_file_writer(const std::string& filename,
                                         rotation_type type,
                                         size_t max_size,
                                         size_t max_files)
    : file_writer(filename, true)
    , rotation_type_(type)
    , max_size_(max_size)
    , max_files_(max_files)
    , last_rotation_time_(std::chrono::system_clock::now())
    , current_period_start_(std::chrono::system_clock::now()) {
    
    if (type != rotation_type::size_and_time) {
        throw std::invalid_argument("This constructor is only for size_and_time rotation");
    }
    
    // Extract base filename and extension
    std::filesystem::path path(filename);
    base_filename_ = path.stem().string();
    file_extension_ = path.extension().string();
    if (file_extension_.empty()) {
        file_extension_ = ".log";
    }
}

bool rotating_file_writer::write(thread_module::log_level level,
                                const std::string& message,
                                const std::string& file,
                                int line,
                                const std::string& function,
                                const std::chrono::system_clock::time_point& timestamp) {
    // Check if rotation is needed before writing
    if (should_rotate()) {
        rotate();
    }
    
    return file_writer::write(level, message, file, line, function, timestamp);
}

void rotating_file_writer::rotate() {
    std::lock_guard<std::mutex> lock(write_mutex_);
    perform_rotation();
}

bool rotating_file_writer::should_rotate() const {
    switch (rotation_type_) {
        case rotation_type::size:
            return get_file_size() >= max_size_;
            
        case rotation_type::daily:
        case rotation_type::hourly:
            return should_rotate_by_time();
            
        case rotation_type::size_and_time:
            return get_file_size() >= max_size_ || should_rotate_by_time();
            
        default:
            return false;
    }
}

void rotating_file_writer::perform_rotation() {
    // Close current file
    close();
    
    // Generate new filename for the current log
    std::string rotated_name = generate_rotated_filename();
    
    // Rename current file
    try {
        if (std::filesystem::exists(filename_)) {
            std::filesystem::rename(filename_, rotated_name);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
    }
    
    // Clean up old files
    cleanup_old_files();
    
    // Open new file
    open();
    
    // Update rotation time
    last_rotation_time_ = std::chrono::system_clock::now();
    current_period_start_ = last_rotation_time_;
}

std::string rotating_file_writer::generate_rotated_filename(int index) const {
    std::ostringstream oss;
    std::filesystem::path dir = std::filesystem::path(filename_).parent_path();
    
    if (!dir.empty()) {
        oss << dir.string() << "/";
    }
    
    oss << base_filename_;
    
    // Add timestamp or index
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    switch (rotation_type_) {
        case rotation_type::size:
            if (index >= 0) {
                oss << "." << index;
            } else {
                // Find next available index
                int next_index = 1;
                while (std::filesystem::exists(oss.str() + "." + std::to_string(next_index) + file_extension_)) {
                    next_index++;
                }
                oss << "." << next_index;
            }
            break;
            
        case rotation_type::daily:
            oss << "." << std::put_time(std::localtime(&time_t), "%Y%m%d");
            break;
            
        case rotation_type::hourly:
            oss << "." << std::put_time(std::localtime(&time_t), "%Y%m%d_%H");
            break;
            
        case rotation_type::size_and_time:
            oss << "." << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
            break;
    }
    
    oss << file_extension_;
    return oss.str();
}

void rotating_file_writer::cleanup_old_files() {
    auto backup_files = get_backup_files();
    
    if (backup_files.size() > max_files_) {
        // Sort by modification time (oldest first)
        std::sort(backup_files.begin(), backup_files.end(),
                 [](const std::string& a, const std::string& b) {
                     return std::filesystem::last_write_time(a) <
                            std::filesystem::last_write_time(b);
                 });
        
        // Remove oldest files
        size_t files_to_remove = backup_files.size() - max_files_;
        for (size_t i = 0; i < files_to_remove; ++i) {
            try {
                std::filesystem::remove(backup_files[i]);
            } catch (const std::exception& e) {
                std::cerr << "Failed to remove old log file: " << e.what() << std::endl;
            }
        }
    }
}

std::vector<std::string> rotating_file_writer::get_backup_files() const {
    std::vector<std::string> files;
    std::filesystem::path dir = std::filesystem::path(filename_).parent_path();
    if (dir.empty()) {
        dir = ".";
    }
    
    // Create regex pattern for backup files
    std::string pattern = base_filename_ + R"(\.(\d+|\\d{8}|\\d{8}_\\d{2}|\\d{8}_\\d{6}))" +
                         std::regex_replace(file_extension_, std::regex(R"(\.)"), R"(\\.)");
    std::regex backup_regex(pattern);
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (std::regex_match(filename, backup_regex)) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error listing backup files: " << e.what() << std::endl;
    }
    
    return files;
}

bool rotating_file_writer::should_rotate_by_time() const {
    auto now = std::chrono::system_clock::now();
    
    switch (rotation_type_) {
        case rotation_type::daily:
        case rotation_type::size_and_time: {
            // Check if we're in a new day
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            auto start_time_t = std::chrono::system_clock::to_time_t(current_period_start_);
            
            std::tm now_tm = *std::localtime(&now_time_t);
            std::tm start_tm = *std::localtime(&start_time_t);
            
            return now_tm.tm_year != start_tm.tm_year ||
                   now_tm.tm_mon != start_tm.tm_mon ||
                   now_tm.tm_mday != start_tm.tm_mday;
        }
        
        case rotation_type::hourly: {
            // Check if we're in a new hour
            auto duration = now - current_period_start_;
            return duration >= std::chrono::hours(1);
        }
        
        default:
            return false;
    }
}

} // namespace logger_module