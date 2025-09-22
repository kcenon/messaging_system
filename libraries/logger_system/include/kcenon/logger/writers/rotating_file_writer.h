#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "file_writer.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace kcenon::logger {

/**
 * @class rotating_file_writer
 * @brief File writer with rotation support based on size or time
 */
class rotating_file_writer : public file_writer {
public:
    /**
     * @enum rotation_type
     * @brief Type of rotation strategy
     */
    enum class rotation_type {
        size,       // Rotate based on file size
        daily,      // Rotate daily
        hourly,     // Rotate hourly
        size_and_time  // Rotate on size or time, whichever comes first
    };
    
    /**
     * @brief Constructor for size-based rotation
     * @param filename Base filename (will be appended with number/date)
     * @param max_size Maximum file size in bytes before rotation
     * @param max_files Maximum number of backup files to keep
     */
    rotating_file_writer(const std::string& filename,
                        size_t max_size,
                        size_t max_files = 10);
    
    /**
     * @brief Constructor for time-based rotation
     * @param filename Base filename (will be appended with date/time)
     * @param type Rotation type (daily or hourly)
     * @param max_files Maximum number of backup files to keep
     */
    rotating_file_writer(const std::string& filename,
                        rotation_type type,
                        size_t max_files = 10);
    
    /**
     * @brief Constructor for combined rotation
     * @param filename Base filename
     * @param type Must be size_and_time
     * @param max_size Maximum file size
     * @param max_files Maximum number of backup files
     */
    rotating_file_writer(const std::string& filename,
                        rotation_type type,
                        size_t max_size,
                        size_t max_files = 10);
    
    /**
     * @brief Destructor
     */
    ~rotating_file_writer() override = default;
    
    /**
     * @brief Write with rotation check
     */
    result_void write(logger_system::log_level level,
                      const std::string& message,
                      const std::string& file,
                      int line,
                      const std::string& function,
                      const std::chrono::system_clock::time_point& timestamp) override;
    
    /**
     * @brief Get writer name
     */
    std::string get_name() const override { return "rotating_file"; }
    
    /**
     * @brief Force rotation
     */
    void rotate();
    
protected:
    /**
     * @brief Check if rotation is needed
     */
    bool should_rotate() const;
    
    /**
     * @brief Perform rotation
     */
    void perform_rotation();
    
    /**
     * @brief Generate rotated filename
     */
    std::string generate_rotated_filename(int index = -1) const;
    
    /**
     * @brief Clean up old files
     */
    void cleanup_old_files();
    
    /**
     * @brief Get list of existing backup files
     */
    std::vector<std::string> get_backup_files() const;
    
    /**
     * @brief Check if time-based rotation is needed
     */
    bool should_rotate_by_time() const;
    
private:
    rotation_type rotation_type_;
    size_t max_size_;
    size_t max_files_;
    
    // For time-based rotation
    mutable std::chrono::system_clock::time_point last_rotation_time_;
    mutable std::chrono::system_clock::time_point current_period_start_;
    
    // Base filename without extension
    std::string base_filename_;
    std::string file_extension_;
};

} // namespace kcenon::logger