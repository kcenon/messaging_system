#pragma once

#include "../core/message_types.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <queue>
#include <condition_variable>
#include <thread>

namespace kcenon::messaging::persistence {

    // Message storage interface
    class message_storage {
    public:
        virtual ~message_storage() = default;
        virtual bool store_message(const std::string& message_id, const core::message& msg) = 0;
        virtual std::unique_ptr<core::message> retrieve_message(const std::string& message_id) = 0;
        virtual bool delete_message(const std::string& message_id) = 0;
        virtual std::vector<std::string> list_messages(const std::string& topic_filter = "") = 0;
        virtual size_t get_storage_size() const = 0;
        virtual void cleanup_old_messages(std::chrono::hours max_age) = 0;
        virtual std::string get_storage_info() const = 0;
    };

    // File-based message storage
    class file_message_storage : public message_storage {
    private:
        std::filesystem::path storage_path_;
        mutable std::shared_mutex storage_mutex_;
        std::atomic<size_t> stored_count_{0};

        static constexpr size_t MAX_FILENAME_LENGTH = 255;

        std::string sanitize_filename(const std::string& message_id) const {
            std::string sanitized = message_id;
            // Replace invalid characters with underscores
            for (char& c : sanitized) {
                if (c == '/' || c == '\\' || c == ':' || c == '*' ||
                    c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
                    c = '_';
                }
            }

            if (sanitized.length() > MAX_FILENAME_LENGTH) {
                sanitized = sanitized.substr(0, MAX_FILENAME_LENGTH);
            }

            return sanitized + ".msg";
        }

        std::string serialize_message(const core::message& msg) const {
            // Simple JSON-like serialization
            std::string result = "{\n";
            result += "  \"timestamp\": " + std::to_string(msg.metadata.timestamp.time_since_epoch().count()) + ",\n";
            result += "  \"priority\": " + std::to_string(static_cast<int>(msg.metadata.priority)) + ",\n";
            result += "  \"sender\": \"" + msg.metadata.sender + "\",\n";
            result += "  \"topic\": \"" + msg.payload.topic + "\",\n";
            result += "  \"data\": {\n";

            for (auto it = msg.payload.data.begin(); it != msg.payload.data.end(); ++it) {
                if (it != msg.payload.data.begin()) result += ",\n";
                result += "    \"" + it->first + "\": ";

                std::visit([&result](const auto& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        result += "\"" + value + "\"";
                    } else if constexpr (std::is_same_v<T, int64_t>) {
                        result += std::to_string(value);
                    } else if constexpr (std::is_same_v<T, double>) {
                        result += std::to_string(value);
                    } else if constexpr (std::is_same_v<T, bool>) {
                        result += value ? "true" : "false";
                    }
                }, it->second);
            }

            result += "\n  }\n}";
            return result;
        }

    public:
        explicit file_message_storage(const std::filesystem::path& path)
            : storage_path_(path) {
            std::filesystem::create_directories(storage_path_);
        }

        bool store_message(const std::string& message_id, const core::message& msg) override {
            std::unique_lock<std::shared_mutex> lock(storage_mutex_);

            try {
                auto filename = sanitize_filename(message_id);
                auto filepath = storage_path_ / filename;

                std::ofstream file(filepath);
                if (!file.is_open()) return false;

                file << serialize_message(msg);
                file.close();

                stored_count_++;
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }

        std::unique_ptr<core::message> retrieve_message(const std::string& message_id) override {
            std::shared_lock<std::shared_mutex> lock(storage_mutex_);

            try {
                auto filename = sanitize_filename(message_id);
                auto filepath = storage_path_ / filename;

                if (!std::filesystem::exists(filepath)) return nullptr;

                std::ifstream file(filepath);
                if (!file.is_open()) return nullptr;

                // For now, return a basic message indicating successful retrieval
                // In a real implementation, this would deserialize the JSON
                auto msg = std::make_unique<core::message>();
                msg->metadata.sender = "persisted";
                msg->payload.topic = "recovered";

                return msg;
            } catch (const std::exception&) {
                return nullptr;
            }
        }

        bool delete_message(const std::string& message_id) override {
            std::unique_lock<std::shared_mutex> lock(storage_mutex_);

            try {
                auto filename = sanitize_filename(message_id);
                auto filepath = storage_path_ / filename;

                if (std::filesystem::exists(filepath)) {
                    std::filesystem::remove(filepath);
                    stored_count_--;
                    return true;
                }
                return false;
            } catch (const std::exception&) {
                return false;
            }
        }

        std::vector<std::string> list_messages(const std::string& topic_filter = "") override {
            std::shared_lock<std::shared_mutex> lock(storage_mutex_);
            std::vector<std::string> messages;

            try {
                for (const auto& entry : std::filesystem::directory_iterator(storage_path_)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".msg") {
                        auto filename = entry.path().stem().string();
                        if (topic_filter.empty() || filename.find(topic_filter) != std::string::npos) {
                            messages.push_back(filename);
                        }
                    }
                }
            } catch (const std::exception&) {
                // Return empty vector on error
            }

            return messages;
        }

        size_t get_storage_size() const override {
            std::shared_lock<std::shared_mutex> lock(storage_mutex_);
            size_t total_size = 0;

            try {
                for (const auto& entry : std::filesystem::directory_iterator(storage_path_)) {
                    if (entry.is_regular_file()) {
                        total_size += entry.file_size();
                    }
                }
            } catch (const std::exception&) {
                // Return 0 on error
            }

            return total_size;
        }

        void cleanup_old_messages(std::chrono::hours max_age) override {
            std::unique_lock<std::shared_mutex> lock(storage_mutex_);

            auto cutoff_time = std::filesystem::file_time_type::clock::now() - max_age;

            try {
                for (const auto& entry : std::filesystem::directory_iterator(storage_path_)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".msg") {
                        if (entry.last_write_time() < cutoff_time) {
                            std::filesystem::remove(entry.path());
                            stored_count_--;
                        }
                    }
                }
            } catch (const std::exception&) {
                // Continue on error
            }
        }

        std::string get_storage_info() const override {
            return "FileStorage: " + storage_path_.string() +
                   " (Messages: " + std::to_string(stored_count_) +
                   ", Size: " + std::to_string(get_storage_size()) + " bytes)";
        }
    };

    // Recovery state tracking
    enum class recovery_state {
        idle,
        scanning,
        recovering,
        completed,
        failed
    };

    // Message recovery system
    class message_recovery_system {
    private:
        std::unique_ptr<message_storage> storage_;
        std::function<void(const core::message&)> recovery_handler_;

        mutable std::mutex recovery_mutex_;
        std::atomic<recovery_state> state_{recovery_state::idle};
        std::atomic<size_t> messages_recovered_{0};
        std::atomic<size_t> recovery_errors_{0};

        // Background recovery
        std::atomic<bool> auto_recovery_enabled_{false};
        std::thread recovery_thread_;
        std::condition_variable recovery_cv_;
        std::chrono::minutes recovery_interval_{5};

    public:
        explicit message_recovery_system(std::unique_ptr<message_storage> storage)
            : storage_(std::move(storage)) {}

        ~message_recovery_system() {
            stop_auto_recovery();
        }

        void set_recovery_handler(std::function<void(const core::message&)> handler) {
            recovery_handler_ = std::move(handler);
        }

        void enable_auto_recovery(std::chrono::minutes interval) {
            recovery_interval_ = interval;

            if (!auto_recovery_enabled_) {
                auto_recovery_enabled_ = true;
                recovery_thread_ = std::thread([this]() { auto_recovery_loop(); });
            }
        }

        void stop_auto_recovery() {
            if (auto_recovery_enabled_) {
                auto_recovery_enabled_ = false;
                recovery_cv_.notify_all();
                if (recovery_thread_.joinable()) {
                    recovery_thread_.join();
                }
            }
        }

        bool recover_messages(const std::string& topic_filter = "") {
            std::lock_guard<std::mutex> lock(recovery_mutex_);

            if (state_ == recovery_state::recovering) {
                return false; // Already recovering
            }

            state_ = recovery_state::scanning;
            messages_recovered_ = 0;
            recovery_errors_ = 0;

            try {
                auto message_ids = storage_->list_messages(topic_filter);
                state_ = recovery_state::recovering;

                for (const auto& id : message_ids) {
                    auto message = storage_->retrieve_message(id);
                    if (message && recovery_handler_) {
                        try {
                            recovery_handler_(*message);
                            messages_recovered_++;
                        } catch (const std::exception&) {
                            recovery_errors_++;
                        }
                    } else {
                        recovery_errors_++;
                    }
                }

                state_ = recovery_state::completed;
                return true;
            } catch (const std::exception&) {
                state_ = recovery_state::failed;
                return false;
            }
        }

        struct recovery_statistics {
            recovery_state current_state;
            size_t messages_recovered;
            size_t recovery_errors;
            size_t total_stored_messages;
            size_t storage_size_bytes;
            std::string storage_info;
            bool auto_recovery_enabled;
            std::chrono::minutes auto_recovery_interval;
        };

        recovery_statistics get_statistics() const {
            recovery_statistics stats;
            stats.current_state = state_;
            stats.messages_recovered = messages_recovered_;
            stats.recovery_errors = recovery_errors_;
            stats.total_stored_messages = storage_->list_messages().size();
            stats.storage_size_bytes = storage_->get_storage_size();
            stats.storage_info = storage_->get_storage_info();
            stats.auto_recovery_enabled = auto_recovery_enabled_;
            stats.auto_recovery_interval = recovery_interval_;
            return stats;
        }

        message_storage* get_storage() const { return storage_.get(); }

    private:
        void auto_recovery_loop() {
            while (auto_recovery_enabled_) {
                std::unique_lock<std::mutex> lock(recovery_mutex_);
                if (recovery_cv_.wait_for(lock, recovery_interval_,
                    [this] { return !auto_recovery_enabled_; })) {
                    break;
                }

                // Perform automatic recovery
                if (state_ == recovery_state::idle || state_ == recovery_state::completed) {
                    lock.unlock();
                    recover_messages();
                }
            }
        }
    };

    // Persistent message queue
    class persistent_message_queue {
    private:
        std::unique_ptr<message_storage> storage_;
        std::queue<std::string> pending_messages_;
        mutable std::mutex queue_mutex_;
        std::atomic<uint64_t> message_counter_{0};

        std::string generate_message_id() {
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();

            return "msg_" + std::to_string(timestamp) + "_" + std::to_string(message_counter_++);
        }

    public:
        explicit persistent_message_queue(std::unique_ptr<message_storage> storage)
            : storage_(std::move(storage)) {}

        std::string enqueue_message(const core::message& msg) {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            auto message_id = generate_message_id();
            if (storage_->store_message(message_id, msg)) {
                pending_messages_.push(message_id);
                return message_id;
            }

            return "";  // Failed to store
        }

        std::unique_ptr<core::message> dequeue_message() {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            if (pending_messages_.empty()) {
                return nullptr;
            }

            auto message_id = pending_messages_.front();
            pending_messages_.pop();

            auto message = storage_->retrieve_message(message_id);
            // Keep message in storage until explicitly deleted

            return message;
        }

        bool acknowledge_message(const std::string& message_id) {
            return storage_->delete_message(message_id);
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            return pending_messages_.size();
        }

        void cleanup_old_messages(std::chrono::hours max_age) {
            storage_->cleanup_old_messages(max_age);
        }

        std::string get_queue_info() const {
            return "PersistentQueue: " + std::to_string(size()) + " pending, " +
                   storage_->get_storage_info();
        }
    };

    // Persistence manager factory
    class persistence_manager_factory {
    public:
        static std::unique_ptr<message_storage> create_file_storage(
            const std::string& storage_path) {
            return std::make_unique<file_message_storage>(storage_path);
        }

        static std::unique_ptr<message_recovery_system> create_recovery_system(
            std::unique_ptr<message_storage> storage) {
            return std::make_unique<message_recovery_system>(std::move(storage));
        }

        static std::unique_ptr<persistent_message_queue> create_persistent_queue(
            std::unique_ptr<message_storage> storage) {
            return std::make_unique<persistent_message_queue>(std::move(storage));
        }

        static std::unique_ptr<message_recovery_system> create_file_recovery_system(
            const std::string& storage_path) {
            auto storage = create_file_storage(storage_path);
            return create_recovery_system(std::move(storage));
        }

        static std::unique_ptr<persistent_message_queue> create_file_persistent_queue(
            const std::string& storage_path) {
            auto storage = create_file_storage(storage_path);
            return create_persistent_queue(std::move(storage));
        }
    };

} // namespace kcenon::messaging::persistence