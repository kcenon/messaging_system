#pragma once

#include <any>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <kcenon/thread/core/event_bus.h>

namespace kcenon::thread {

/**
 * @brief Configuration value type
 */
using config_value = std::variant<
    bool,
    int,
    double,
    std::string,
    std::vector<std::string>,
    std::unordered_map<std::string, std::any>
>;

/**
 * @brief Configuration validation result
 */
struct validation_result {
    bool is_valid{true};
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    void add_error(const std::string& error) {
        errors.push_back(error);
        is_valid = false;
    }

    void add_warning(const std::string& warning) {
        warnings.push_back(warning);
    }
};

/**
 * @brief Configuration manager for unified system configuration
 */
class configuration_manager {
public:
    /**
     * @brief Configuration change callback
     */
    using change_callback = std::function<void(const std::string&, const config_value&)>;

    /**
     * @brief Configuration validator
     */
    using validator_func = std::function<validation_result(const std::string&, const config_value&)>;

    /**
     * @brief Constructor
     * @param event_bus Optional event bus for change notifications
     */
    explicit configuration_manager(std::shared_ptr<event_bus> bus = nullptr)
        : event_bus_(bus) {
        if (!event_bus_) {
            event_bus_ = std::make_shared<event_bus>();
        }
    }

    /**
     * @brief Load configuration from file
     * @param config_file Path to configuration file
     * @return True if successful
     */
    bool load_from_file(const std::filesystem::path& config_file) {
        if (!std::filesystem::exists(config_file)) {
            return false;
        }

        try {
            std::ifstream file(config_file);
            if (!file.is_open()) {
                return false;
            }

            // Simple key=value parser for demonstration
            // In production, use JSON/YAML parser
            std::string line;
            while (std::getline(file, line)) {
                parse_config_line(line);
            }

            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Save configuration to file
     * @param config_file Path to configuration file
     * @return True if successful
     */
    bool save_to_file(const std::filesystem::path& config_file) const {
        try {
            std::ofstream file(config_file);
            if (!file.is_open()) {
                return false;
            }

            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& [key, value] : config_) {
                file << key << "=" << value_to_string(value) << "\n";
            }

            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Set a configuration value
     * @param path Configuration path (e.g., "thread_system.pool_size")
     * @param value Configuration value
     * @return True if successful
     */
    bool set(const std::string& path, const config_value& value) {
        // Validate if validator exists
        if (auto it = validators_.find(path); it != validators_.end()) {
            auto result = it->second(path, value);
            if (!result.is_valid) {
                return false;
            }
        }

        config_value old_value;
        bool had_value = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (auto it = config_.find(path); it != config_.end()) {
                old_value = it->second;
                had_value = true;
            }
            config_[path] = value;
        }

        // Notify callbacks
        notify_change(path, value);

        // Publish event if value changed
        if (event_bus_ && (!had_value || !values_equal(old_value, value))) {
            event_bus_->publish(config_changed_event(path, old_value, value));
        }

        return true;
    }

    /**
     * @brief Get a configuration value
     * @tparam T Value type
     * @param path Configuration path
     * @param default_value Default value if not found
     * @return Configuration value
     */
    template<typename T>
    T get(const std::string& path, const T& default_value = T{}) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = config_.find(path);
        if (it != config_.end()) {
            try {
                return std::get<T>(it->second);
            } catch (const std::bad_variant_access&) {
                // Type mismatch
            }
        }
        return default_value;
    }

    /**
     * @brief Get a configuration value as optional
     * @tparam T Value type
     * @param path Configuration path
     * @return Optional configuration value
     */
    template<typename T>
    std::optional<T> get_optional(const std::string& path) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = config_.find(path);
        if (it != config_.end()) {
            try {
                return std::get<T>(it->second);
            } catch (const std::bad_variant_access&) {
                // Type mismatch
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Check if configuration exists
     * @param path Configuration path
     * @return True if exists
     */
    bool has(const std::string& path) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return config_.find(path) != config_.end();
    }

    /**
     * @brief Remove a configuration value
     * @param path Configuration path
     * @return True if removed
     */
    bool remove(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        return config_.erase(path) > 0;
    }

    /**
     * @brief Register a change callback
     * @param path Configuration path (empty for all changes)
     * @param callback Callback function
     * @return Callback ID for unregistration
     */
    std::size_t on_change(const std::string& path, change_callback callback) {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        auto id = next_callback_id_++;
        callbacks_[path][id] = std::move(callback);
        return id;
    }

    /**
     * @brief Unregister a change callback
     * @param path Configuration path
     * @param id Callback ID
     */
    void remove_callback(const std::string& path, std::size_t id) {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (auto it = callbacks_.find(path); it != callbacks_.end()) {
            it->second.erase(id);
            if (it->second.empty()) {
                callbacks_.erase(it);
            }
        }
    }

    /**
     * @brief Register a configuration validator
     * @param path Configuration path
     * @param validator Validator function
     */
    void add_validator(const std::string& path, validator_func validator) {
        std::lock_guard<std::mutex> lock(mutex_);
        validators_[path] = std::move(validator);
    }

    /**
     * @brief Validate all configuration
     * @return Validation result
     */
    validation_result validate_all() const {
        validation_result result;
        std::lock_guard<std::mutex> lock(mutex_);

        for (const auto& [path, value] : config_) {
            if (auto it = validators_.find(path); it != validators_.end()) {
                auto path_result = it->second(path, value);
                if (!path_result.is_valid) {
                    result.is_valid = false;
                    for (const auto& error : path_result.errors) {
                        result.add_error(path + ": " + error);
                    }
                }
                for (const auto& warning : path_result.warnings) {
                    result.add_warning(path + ": " + warning);
                }
            }
        }

        return result;
    }

    /**
     * @brief Apply configuration for a specific system
     * @param system_name System name (e.g., "thread_system")
     * @param config Configuration map
     */
    void apply_system_config(const std::string& system_name,
                            const std::unordered_map<std::string, config_value>& config) {
        for (const auto& [key, value] : config) {
            set(system_name + "." + key, value);
        }
    }

    /**
     * @brief Get configuration for a specific system
     * @param system_name System name
     * @return Configuration map
     */
    std::unordered_map<std::string, config_value> get_system_config(
        const std::string& system_name) const {
        std::unordered_map<std::string, config_value> result;
        std::string prefix = system_name + ".";

        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [key, value] : config_) {
            if (key.starts_with(prefix)) {
                result[key.substr(prefix.length())] = value;
            }
        }

        return result;
    }

    /**
     * @brief Clear all configuration
     */
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.clear();
    }

    /**
     * @brief Get singleton instance
     * @return Configuration manager instance
     */
    static configuration_manager& instance() {
        static configuration_manager instance;
        return instance;
    }

private:
    /**
     * @brief Parse a configuration line
     */
    void parse_config_line(const std::string& line) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            return; // Skip comments and empty lines
        }

        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value_str = line.substr(pos + 1);

            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value_str.erase(0, value_str.find_first_not_of(" \t"));
            value_str.erase(value_str.find_last_not_of(" \t") + 1);

            // Parse value type
            config_value value;
            if (value_str == "true" || value_str == "false") {
                value = (value_str == "true");
            } else if (std::all_of(value_str.begin(), value_str.end(), ::isdigit)) {
                value = std::stoi(value_str);
            } else {
                try {
                    value = std::stod(value_str);
                } catch (...) {
                    value = value_str; // Default to string
                }
            }

            set(key, value);
        }
    }

    /**
     * @brief Convert value to string
     */
    std::string value_to_string(const config_value& value) const {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, std::string>) {
                return v;
            } else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(v);
            } else {
                return ""; // Complex types need custom serialization
            }
        }, value);
    }

    /**
     * @brief Check if two values are equal
     */
    bool values_equal(const config_value& a, const config_value& b) const {
        return a == b;
    }

    /**
     * @brief Notify change callbacks
     */
    void notify_change(const std::string& path, const config_value& value) {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);

        // Notify specific path callbacks
        if (auto it = callbacks_.find(path); it != callbacks_.end()) {
            for (const auto& [id, callback] : it->second) {
                callback(path, value);
            }
        }

        // Notify global callbacks
        if (auto it = callbacks_.find(""); it != callbacks_.end()) {
            for (const auto& [id, callback] : it->second) {
                callback(path, value);
            }
        }
    }

    mutable std::mutex mutex_;
    mutable std::mutex callbacks_mutex_;
    std::unordered_map<std::string, config_value> config_;
    std::unordered_map<std::string, validator_func> validators_;
    std::unordered_map<std::string, std::unordered_map<std::size_t, change_callback>> callbacks_;
    std::shared_ptr<event_bus> event_bus_;
    std::size_t next_callback_id_{1};
};

} // namespace kcenon::thread