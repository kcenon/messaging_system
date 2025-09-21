#pragma once

#include "../core/message_types.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <random>
#include <algorithm>
#include <functional>

namespace kcenon::messaging::security {

    // Security levels
    enum class security_level {
        none = 0,
        basic = 1,
        standard = 2,
        high = 3,
        maximum = 4
    };

    // Authentication result
    enum class auth_result {
        success,
        invalid_credentials,
        account_locked,
        token_expired,
        permission_denied,
        rate_limited
    };

    // User credentials
    struct user_credentials {
        std::string user_id;
        std::string password_hash;
        std::unordered_set<std::string> permissions;
        std::chrono::system_clock::time_point created_at;
        std::chrono::system_clock::time_point last_login;
        std::atomic<uint32_t> failed_attempts{0};
        bool is_locked = false;
        security_level access_level = security_level::basic;
    };

    // Authentication token
    struct auth_token {
        std::string token_id;
        std::string user_id;
        std::chrono::system_clock::time_point issued_at;
        std::chrono::system_clock::time_point expires_at;
        std::unordered_set<std::string> scopes;
        security_level level;

        bool is_valid() const {
            return std::chrono::system_clock::now() < expires_at;
        }

        bool has_scope(const std::string& scope) const {
            return scopes.find(scope) != scopes.end();
        }
    };

    // Message encryption interface
    class message_encryptor {
    public:
        virtual ~message_encryptor() = default;
        virtual std::string encrypt(const std::string& plaintext, const std::string& key) = 0;
        virtual std::string decrypt(const std::string& ciphertext, const std::string& key) = 0;
        virtual std::string generate_key() = 0;
        virtual std::string get_algorithm_name() const = 0;
    };

    // Simple XOR encryptor (for demonstration - not production ready)
    class xor_encryptor : public message_encryptor {
    private:
        std::mt19937 rng_;

    public:
        xor_encryptor() : rng_(std::random_device{}()) {}

        std::string encrypt(const std::string& plaintext, const std::string& key) override {
            std::string result = plaintext;
            for (size_t i = 0; i < result.size(); ++i) {
                result[i] ^= key[i % key.size()];
            }
            return result;
        }

        std::string decrypt(const std::string& ciphertext, const std::string& key) override {
            // XOR is symmetric
            return encrypt(ciphertext, key);
        }

        std::string generate_key() override {
            std::string key;
            key.reserve(32);

            std::uniform_int_distribution<int> dist(33, 126); // Printable ASCII
            for (int i = 0; i < 32; ++i) {
                key += static_cast<char>(dist(rng_));
            }
            return key;
        }

        std::string get_algorithm_name() const override {
            return "XOR-32";
        }
    };

    // Authentication manager
    class authentication_manager {
    private:
        mutable std::shared_mutex auth_mutex_;
        std::unordered_map<std::string, std::unique_ptr<user_credentials>> users_;
        std::unordered_map<std::string, std::unique_ptr<auth_token>> active_tokens_;

        // Security settings
        static constexpr uint32_t MAX_FAILED_ATTEMPTS = 5;
        static constexpr std::chrono::minutes TOKEN_LIFETIME{60};
        static constexpr std::chrono::minutes LOCKOUT_DURATION{15};

        std::mt19937 token_generator_;
        std::atomic<uint64_t> total_authentications_{0};
        std::atomic<uint64_t> failed_authentications_{0};

        std::string generate_token_id() {
            std::uniform_int_distribution<uint64_t> dist;
            return "token_" + std::to_string(dist(token_generator_));
        }

        std::string hash_password(const std::string& password) const {
            // Simple hash (in production, use bcrypt or similar)
            std::hash<std::string> hasher;
            return std::to_string(hasher(password + "salt"));
        }

    public:
        authentication_manager() : token_generator_(std::random_device{}()) {}

        bool create_user(const std::string& user_id, const std::string& password,
                        const std::unordered_set<std::string>& permissions = {},
                        security_level level = security_level::basic) {
            std::unique_lock<std::shared_mutex> lock(auth_mutex_);

            if (users_.find(user_id) != users_.end()) {
                return false; // User already exists
            }

            auto credentials = std::make_unique<user_credentials>();
            credentials->user_id = user_id;
            credentials->password_hash = hash_password(password);
            credentials->permissions = permissions;
            credentials->created_at = std::chrono::system_clock::now();
            credentials->access_level = level;

            users_[user_id] = std::move(credentials);
            return true;
        }

        auth_result authenticate(const std::string& user_id, const std::string& password,
                               std::string& token_out) {
            std::unique_lock<std::shared_mutex> lock(auth_mutex_);
            total_authentications_++;

            auto user_it = users_.find(user_id);
            if (user_it == users_.end()) {
                failed_authentications_++;
                return auth_result::invalid_credentials;
            }

            auto& user = *user_it->second;

            // Check if account is locked
            if (user.is_locked) {
                auto now = std::chrono::system_clock::now();
                if (now - user.last_login < LOCKOUT_DURATION) {
                    return auth_result::account_locked;
                } else {
                    // Unlock account after lockout period
                    user.is_locked = false;
                    user.failed_attempts = 0;
                }
            }

            // Verify password
            if (user.password_hash != hash_password(password)) {
                user.failed_attempts++;
                if (user.failed_attempts >= MAX_FAILED_ATTEMPTS) {
                    user.is_locked = true;
                }
                failed_authentications_++;
                return auth_result::invalid_credentials;
            }

            // Create token
            auto token = std::make_unique<auth_token>();
            token->token_id = generate_token_id();
            token->user_id = user_id;
            token->issued_at = std::chrono::system_clock::now();
            token->expires_at = token->issued_at + TOKEN_LIFETIME;
            token->scopes = user.permissions;
            token->level = user.access_level;

            token_out = token->token_id;
            active_tokens_[token->token_id] = std::move(token);

            // Update user login info
            user.last_login = std::chrono::system_clock::now();
            user.failed_attempts = 0;

            return auth_result::success;
        }

        auth_result verify_token(const std::string& token_id, const std::string& required_scope = "") {
            std::shared_lock<std::shared_mutex> lock(auth_mutex_);

            auto token_it = active_tokens_.find(token_id);
            if (token_it == active_tokens_.end()) {
                return auth_result::invalid_credentials;
            }

            auto& token = *token_it->second;
            if (!token.is_valid()) {
                return auth_result::token_expired;
            }

            if (!required_scope.empty() && !token.has_scope(required_scope)) {
                return auth_result::permission_denied;
            }

            return auth_result::success;
        }

        bool revoke_token(const std::string& token_id) {
            std::unique_lock<std::shared_mutex> lock(auth_mutex_);
            return active_tokens_.erase(token_id) > 0;
        }

        void cleanup_expired_tokens() {
            std::unique_lock<std::shared_mutex> lock(auth_mutex_);
            auto now = std::chrono::system_clock::now();

            for (auto it = active_tokens_.begin(); it != active_tokens_.end();) {
                if (it->second->expires_at <= now) {
                    it = active_tokens_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        struct auth_statistics {
            size_t total_users;
            size_t active_tokens;
            uint64_t total_authentications;
            uint64_t failed_authentications;
            double success_rate;
        };

        auth_statistics get_statistics() const {
            std::shared_lock<std::shared_mutex> lock(auth_mutex_);
            auth_statistics stats;
            stats.total_users = users_.size();
            stats.active_tokens = active_tokens_.size();
            stats.total_authentications = total_authentications_;
            stats.failed_authentications = failed_authentications_;

            if (stats.total_authentications > 0) {
                stats.success_rate = 1.0 - static_cast<double>(stats.failed_authentications) /
                                   static_cast<double>(stats.total_authentications);
            } else {
                stats.success_rate = 0.0;
            }

            return stats;
        }
    };

    // Message access control
    class message_access_controller {
    private:
        std::unordered_map<std::string, std::unordered_set<std::string>> topic_permissions_;
        std::unordered_map<std::string, security_level> topic_security_levels_;
        mutable std::shared_mutex acl_mutex_;

    public:
        void set_topic_permission(const std::string& topic, const std::string& permission) {
            std::unique_lock<std::shared_mutex> lock(acl_mutex_);
            topic_permissions_[topic].insert(permission);
        }

        void set_topic_security_level(const std::string& topic, security_level level) {
            std::unique_lock<std::shared_mutex> lock(acl_mutex_);
            topic_security_levels_[topic] = level;
        }

        bool check_access(const std::string& topic, const auth_token& token,
                         const std::string& operation) const {
            std::shared_lock<std::shared_mutex> lock(acl_mutex_);

            // Check if token has required scope for operation
            std::string required_scope = operation + ":" + topic;
            if (!token.has_scope(required_scope) && !token.has_scope("admin")) {
                return false;
            }

            // Check security level requirements
            auto level_it = topic_security_levels_.find(topic);
            if (level_it != topic_security_levels_.end()) {
                if (token.level < level_it->second) {
                    return false;
                }
            }

            return true;
        }

        std::vector<std::string> get_accessible_topics(const auth_token& token) const {
            std::shared_lock<std::shared_mutex> lock(acl_mutex_);
            std::vector<std::string> accessible;

            for (const auto& [topic, permissions] : topic_permissions_) {
                if (check_access(topic, token, "read")) {
                    accessible.push_back(topic);
                }
            }

            return accessible;
        }
    };

    // Secure message wrapper
    struct secure_message {
        core::message original_message;
        std::string encrypted_payload;
        std::string encryption_key_id;
        std::string sender_token;
        std::chrono::system_clock::time_point encrypted_at;
        security_level required_level;

        bool is_encrypted() const {
            return !encrypted_payload.empty();
        }
    };

    // Security manager - orchestrates all security components
    class security_manager {
    private:
        std::unique_ptr<authentication_manager> auth_manager_;
        std::unique_ptr<message_access_controller> access_controller_;
        std::unique_ptr<message_encryptor> encryptor_;
        std::unordered_map<std::string, std::string> encryption_keys_;
        mutable std::mutex keys_mutex_;

        security_level default_security_level_ = security_level::basic;

    public:
        security_manager()
            : auth_manager_(std::make_unique<authentication_manager>()),
              access_controller_(std::make_unique<message_access_controller>()),
              encryptor_(std::make_unique<xor_encryptor>()) {}

        // User management
        bool create_user(const std::string& user_id, const std::string& password,
                        const std::unordered_set<std::string>& permissions = {},
                        security_level level = security_level::basic) {
            return auth_manager_->create_user(user_id, password, permissions, level);
        }

        auth_result authenticate_user(const std::string& user_id, const std::string& password,
                                    std::string& token_out) {
            return auth_manager_->authenticate(user_id, password, token_out);
        }

        // Message security
        secure_message encrypt_message(const core::message& msg, const std::string& token,
                                     security_level level = security_level::standard) {
            secure_message secure_msg;
            secure_msg.original_message = msg;
            secure_msg.sender_token = token;
            secure_msg.encrypted_at = std::chrono::system_clock::now();
            secure_msg.required_level = level;

            if (level >= security_level::standard) {
                // Generate or get encryption key
                std::string key_id = "key_" + msg.payload.topic;
                std::lock_guard<std::mutex> lock(keys_mutex_);

                if (encryption_keys_.find(key_id) == encryption_keys_.end()) {
                    encryption_keys_[key_id] = encryptor_->generate_key();
                }

                // Serialize and encrypt message payload
                std::string serialized = msg.payload.topic + "|" +
                    (msg.payload.data.empty() ? "" : "data_present");

                secure_msg.encrypted_payload = encryptor_->encrypt(serialized, encryption_keys_[key_id]);
                secure_msg.encryption_key_id = key_id;
            }

            return secure_msg;
        }

        bool decrypt_message(const secure_message& secure_msg, core::message& decrypted_msg) {
            if (!secure_msg.is_encrypted()) {
                decrypted_msg = secure_msg.original_message;
                return true;
            }

            std::lock_guard<std::mutex> lock(keys_mutex_);
            auto key_it = encryption_keys_.find(secure_msg.encryption_key_id);
            if (key_it == encryption_keys_.end()) {
                return false;
            }

            try {
                std::string decrypted = encryptor_->decrypt(secure_msg.encrypted_payload, key_it->second);
                // Simple deserialization (in production, use proper serialization)
                decrypted_msg = secure_msg.original_message;
                return true;
            } catch (const std::exception&) {
                return false;
            }
        }

        // Access control
        void configure_topic_security(const std::string& topic, security_level level,
                                     const std::unordered_set<std::string>& required_permissions) {
            access_controller_->set_topic_security_level(topic, level);
            for (const auto& permission : required_permissions) {
                access_controller_->set_topic_permission(topic, permission);
            }
        }

        bool authorize_message_access(const std::string& topic, const std::string& token,
                                     const std::string& operation) {
            if (auth_manager_->verify_token(token) != auth_result::success) {
                return false;
            }

            // For demonstration, we'll create a simple token lookup
            // In production, this would involve proper token parsing
            auth_token dummy_token;
            dummy_token.token_id = token;
            dummy_token.scopes.insert(operation + ":" + topic);
            dummy_token.level = default_security_level_;

            return access_controller_->check_access(topic, dummy_token, operation);
        }

        // System management
        void cleanup_expired_tokens() {
            auth_manager_->cleanup_expired_tokens();
        }

        struct security_statistics {
            authentication_manager::auth_statistics auth_stats;
            size_t encrypted_messages_count;
            size_t active_encryption_keys;
            std::string encryption_algorithm;
        };

        security_statistics get_statistics() const {
            security_statistics stats;
            stats.auth_stats = auth_manager_->get_statistics();

            std::lock_guard<std::mutex> lock(keys_mutex_);
            stats.active_encryption_keys = encryption_keys_.size();
            stats.encryption_algorithm = encryptor_->get_algorithm_name();

            return stats;
        }

        authentication_manager* get_auth_manager() const { return auth_manager_.get(); }
        message_access_controller* get_access_controller() const { return access_controller_.get(); }
    };

    // Security policy builder
    class security_policy_builder {
    private:
        std::unique_ptr<security_manager> manager_;

    public:
        security_policy_builder() : manager_(std::make_unique<security_manager>()) {}

        security_policy_builder& create_admin_user(const std::string& user_id,
                                                   const std::string& password) {
            std::unordered_set<std::string> admin_permissions = {
                "admin", "read:*", "write:*", "delete:*"
            };
            manager_->create_user(user_id, password, admin_permissions, security_level::maximum);
            return *this;
        }

        security_policy_builder& create_regular_user(const std::string& user_id,
                                                     const std::string& password,
                                                     const std::vector<std::string>& topics) {
            std::unordered_set<std::string> permissions;
            for (const auto& topic : topics) {
                permissions.insert("read:" + topic);
                permissions.insert("write:" + topic);
            }
            manager_->create_user(user_id, password, permissions, security_level::standard);
            return *this;
        }

        security_policy_builder& secure_topic(const std::string& topic, security_level level) {
            std::unordered_set<std::string> permissions = {"read:" + topic, "write:" + topic};
            manager_->configure_topic_security(topic, level, permissions);
            return *this;
        }

        std::unique_ptr<security_manager> build() {
            return std::move(manager_);
        }
    };

} // namespace kcenon::messaging::security