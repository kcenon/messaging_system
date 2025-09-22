/**
 * @file mock_di_container.hpp
 * @brief Mock DI container implementation for testing
 * @date 2025-09-09
 *
 * BSD 3-Clause License
 * Copyright (c) 2025, kcenon
 * All rights reserved.
 */

#pragma once

#include "../../sources/logger/di/di_container_interface.h"
#include "../../sources/logger/error_codes.h"
#include "mock_writer.hpp"
#include <unordered_map>
#include <functional>
#include <atomic>

namespace logger_system::testing {

using namespace logger_module;

/**
 * @brief Mock DI container for unit testing
 * 
 * Provides controllable dependency injection behavior for testing
 * DI integration and component resolution scenarios.
 */
class mock_di_container : public di_container_interface<base_writer> {
private:
    using factory_func = std::function<std::shared_ptr<base_writer>()>;
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, factory_func> factories_;
    std::unordered_map<std::string, std::shared_ptr<base_writer>> singletons_;
    mutable std::unordered_map<std::string, size_t> resolution_counts_;
    std::atomic<bool> should_fail_{false};
    std::atomic<bool> use_singletons_{false};
    logger_error_code failure_error_{logger_error_code::component_not_found};

public:
    mock_di_container() = default;
    ~mock_di_container() override = default;

    // di_container_interface implementation
    result<std::shared_ptr<base_writer>> resolve(const std::string& name) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        resolution_counts_[name]++;

        if (should_fail_.load()) {
            return make_logger_error<std::shared_ptr<base_writer>>(failure_error_);
        }

        // Check singletons first if enabled
        if (use_singletons_.load()) {
            auto singleton_it = singletons_.find(name);
            if (singleton_it != singletons_.end()) {
                return singleton_it->second;
            }
        }

        // Try factory
        auto factory_it = factories_.find(name);
        if (factory_it == factories_.end()) {
            return make_logger_error<std::shared_ptr<base_writer>>(logger_error_code::component_not_found);
        }

        auto instance = factory_it->second();
        
        // Store as singleton if enabled
        if (use_singletons_.load()) {
            singletons_[name] = instance;
        }

        return instance;
    }

    result_void register_factory(const std::string& name,
                                std::function<std::shared_ptr<base_writer>()> factory) {
        if (should_fail_.load()) {
            return make_logger_error(failure_error_);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        factories_[name] = factory;
        return {};
    }

    // Missing pure virtual methods from di_container_interface
    result_void register_singleton(const std::string& name,
                                  std::shared_ptr<base_writer> instance) override {
        if (should_fail_.load()) {
            return make_logger_error(failure_error_);
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        singletons_[name] = instance;
        return {};
    }
    
    bool is_registered(const std::string& name) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return factories_.find(name) != factories_.end() ||
               singletons_.find(name) != singletons_.end();
    }
    
    result_void clear() override {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_.clear();
        singletons_.clear();
        return {};
    }
    
    size_t size() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return factories_.size() + singletons_.size();
    }
    
    // Mock-specific methods
    result_void register_instance(const std::string& name,
                                 std::shared_ptr<base_writer> instance) {
        if (should_fail_.load()) {
            return make_logger_error(failure_error_);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        singletons_[name] = instance;
        return {};
    }

    // Control methods
    void set_should_fail(bool fail, logger_error_code error = logger_error_code::component_not_found) {
        should_fail_.store(fail);
        failure_error_ = error;
    }

    void set_use_singletons(bool use) {
        use_singletons_.store(use);
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        factories_.clear();
        singletons_.clear();
        resolution_counts_.clear();
        should_fail_.store(false);
        use_singletons_.store(false);
    }

    // Inspection methods
    size_t get_resolution_count(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = resolution_counts_.find(name);
        return it != resolution_counts_.end() ? it->second : 0;
    }

    std::vector<std::string> get_registered_names() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        for (const auto& [name, _] : factories_) {
            names.push_back(name);
        }
        return names;
    }

    bool has_component(const std::string& name) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return factories_.find(name) != factories_.end() ||
               singletons_.find(name) != singletons_.end();
    }
};

/**
 * @brief Test helper for DI container scenarios
 */
class mock_di_scenario {
private:
    std::shared_ptr<mock_di_container> container_;
    std::vector<std::shared_ptr<mock_writer>> mock_writers_;

public:
    mock_di_scenario() 
        : container_(std::make_shared<mock_di_container>()) {}

    std::shared_ptr<mock_di_container> get_container() {
        return container_;
    }

    void setup_default_writers() {
        container_->register_factory("console", [this]() {
            auto writer = std::make_shared<mock_writer>();
            mock_writers_.push_back(writer);
            return writer;
        });

        container_->register_factory("file", [this]() {
            auto writer = std::make_shared<mock_writer>();
            mock_writers_.push_back(writer);
            return writer;
        });

        container_->register_factory("async", [this]() {
            auto writer = std::make_shared<mock_writer>();
            mock_writers_.push_back(writer);
            return writer;
        });
    }

    void setup_failing_writer(const std::string& name) {
        container_->register_factory(name, [this]() {
            auto writer = std::make_shared<mock_writer>();
            writer->set_should_fail(true);
            mock_writers_.push_back(writer);
            return writer;
        });
    }

    void setup_slow_writer(const std::string& name, 
                          std::chrono::milliseconds delay) {
        container_->register_factory(name, [this, delay]() {
            auto writer = std::make_shared<mock_writer>();
            writer->set_write_delay(delay);
            mock_writers_.push_back(writer);
            return writer;
        });
    }

    void reset() {
        container_->reset();
        mock_writers_.clear();
    }

    std::vector<std::shared_ptr<mock_writer>> get_created_writers() const {
        return mock_writers_;
    }
};

} // namespace logger_system::testing