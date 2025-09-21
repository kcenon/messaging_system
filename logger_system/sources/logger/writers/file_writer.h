#pragma once

/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.
*****************************************************************************/

#include "base_writer.h"
#include <fstream>
#include <mutex>
#include <atomic>

namespace logger_module {

/**
 * @class file_writer
 * @brief Basic file writer for logging to files
 */
class file_writer : public base_writer {
public:
    /**
     * @brief Constructor
     * @param filename Path to the log file
     * @param append Whether to append to existing file (default: true)
     * @param buffer_size Output buffer size in bytes (default: 8192)
     */
    explicit file_writer(const std::string& filename,
                        bool append = true,
                        size_t buffer_size = 8192);
    
    /**
     * @brief Destructor
     */
    ~file_writer() override;
    
    /**
     * @brief Write log entry to file
     */
    bool write(thread_module::log_level level,
               const std::string& message,
               const std::string& file,
               int line,
               const std::string& function,
               const std::chrono::system_clock::time_point& timestamp) override;
    
    /**
     * @brief Flush file buffer
     */
    void flush() override;
    
    /**
     * @brief Get writer name
     */
    std::string get_name() const override { return "file"; }
    
    /**
     * @brief Check if file is open
     */
    bool is_open() const { return file_stream_.is_open(); }
    
    /**
     * @brief Get current file size
     */
    size_t get_file_size() const { return bytes_written_.load(); }
    
    /**
     * @brief Reopen the file
     */
    bool reopen();
    
protected:
    /**
     * @brief Close the current file
     */
    void close();
    
    /**
     * @brief Open the file
     */
    bool open();
    
protected:
    std::string filename_;
    bool append_mode_;
    size_t buffer_size_;
    
    std::ofstream file_stream_;
    std::unique_ptr<char[]> buffer_;
    mutable std::mutex write_mutex_;
    
    std::atomic<size_t> bytes_written_{0};
};

} // namespace logger_module