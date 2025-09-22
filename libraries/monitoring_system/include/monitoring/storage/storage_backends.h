#pragma once

// Storage backends implementation - stub for compatibility
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace monitoring_system {

/**
 * @brief Storage backend types
 */
enum class storage_backend_type {
    memory,
    file_json,
    file_binary,
    database,
    time_series
};

/**
 * @brief Compression algorithms
 */
enum class compression_algorithm {
    none,
    gzip,
    lz4,
    zstd
};

/**
 * @brief Storage configuration
 */
struct storage_config {
    storage_backend_type type{storage_backend_type::memory};
    std::string path;
    std::string data_directory;
    compression_algorithm compression{compression_algorithm::none};
    size_t max_size_mb{100};
    bool auto_flush{true};
    std::chrono::milliseconds flush_interval{std::chrono::milliseconds(5000)};
};

// Use monitoring_system result types
#include "monitoring/core/result_types.h"

/**
 * @brief Basic key-value storage interface - stub
 */
class kv_storage_backend {
public:
    virtual ~kv_storage_backend() = default;
    virtual bool store(const std::string& key, const std::string& value) = 0;
    virtual std::string retrieve(const std::string& key) = 0;
    virtual bool remove(const std::string& key) = 0;
    virtual result<bool> flush() { return make_success(true); }
};

/**
 * @brief In-memory storage backend - basic implementation
 */
class memory_storage_backend : public kv_storage_backend {
public:
    bool store(const std::string& key, const std::string& value) override {
        data_[key] = value;
        return true;
    }

    std::string retrieve(const std::string& key) override {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "";
    }

    bool remove(const std::string& key) override {
        return data_.erase(key) > 0;
    }

private:
    std::unordered_map<std::string, std::string> data_;
};

/**
 * @brief File storage backend - stub implementation
 */
class file_storage_backend : public kv_storage_backend {
public:
    file_storage_backend() = default;
    explicit file_storage_backend(const storage_config& config) : config_(config) {}

    bool store(const std::string& key, const std::string& value) override {
        data_[key] = value;
        return true;
    }

    std::string retrieve(const std::string& key) override {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "";
    }

    bool remove(const std::string& key) override {
        return data_.erase(key) > 0;
    }

    result<bool> flush() override {
        // Stub implementation - just return success
        return make_success(true);
    }

private:
    storage_config config_;
    std::unordered_map<std::string, std::string> data_;
};

} // namespace monitoring_system