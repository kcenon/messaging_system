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

#pragma once

#include "../database_types.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <mutex>
#include <functional>
#include <optional>

namespace database::security
{
	/**
	 * @enum encryption_type
	 * @brief Types of encryption supported.
	 */
	enum class encryption_type {
		none,
		tls,
		ssl,
		aes256
	};

	/**
	 * @enum authentication_method
	 * @brief Authentication methods supported.
	 */
	enum class authentication_method {
		password,
		certificate,
		kerberos,
		oauth2,
		jwt
	};

	/**
	 * @struct security_credentials
	 * @brief Secure credential storage.
	 */
	struct security_credentials
	{
		std::string username;
		std::string password_hash;
		std::string certificate_path;
		std::string private_key_path;
		std::string ca_cert_path;
		authentication_method auth_method = authentication_method::password;
		encryption_type encryption = encryption_type::tls;

		// OAuth2/JWT specific
		std::string client_id;
		std::string client_secret;
		std::string token;
		std::chrono::system_clock::time_point token_expiry;

		// Additional security options
		bool verify_certificate = true;
		bool mutual_authentication = false;
		std::string allowed_ciphers;
	};

	/**
	 * @struct audit_log_entry
	 * @brief Audit log entry for security events.
	 */
	struct audit_log_entry
	{
		std::chrono::system_clock::time_point timestamp;
		std::string user_id;
		std::string session_id;
		std::string operation;
		std::string table_name;
		std::string query_hash;
		bool success;
		std::string error_message;
		std::string client_ip;
		std::string user_agent;
	};

	/**
	 * @class credential_manager
	 * @brief Manages encrypted credential storage and retrieval.
	 */
	class credential_manager
	{
	public:
		static credential_manager& instance();

		// Credential management
		bool store_credentials(const std::string& connection_id, const security_credentials& credentials);
		std::optional<security_credentials> get_credentials(const std::string& connection_id) const;
		bool remove_credentials(const std::string& connection_id);

		// Encryption key management
		void set_master_key(const std::string& key);
		bool rotate_encryption_keys();

		// Password utilities
		std::string hash_password(const std::string& password) const;
		bool verify_password(const std::string& password, const std::string& hash) const;

	private:
		credential_manager() = default;

		std::string encrypt_data(const std::string& data) const;
		std::string decrypt_data(const std::string& encrypted_data) const;

		mutable std::mutex credentials_mutex_;
		std::unordered_map<std::string, std::string> encrypted_credentials_;
		std::string master_key_;
	};

	/**
	 * @class connection_security
	 * @brief Handles secure database connections.
	 */
	class connection_security
	{
	public:
		connection_security(const security_credentials& credentials);

		// Connection security
		bool establish_secure_connection(const std::string& host, int port);
		bool verify_server_certificate(const std::string& certificate) const;
		bool perform_mutual_authentication();

		// TLS/SSL configuration
		bool configure_tls(const std::string& cert_file, const std::string& key_file, const std::string& ca_file);
		bool set_cipher_suite(const std::string& ciphers);

		// Connection string encryption
		std::string encrypt_connection_string(const std::string& connection_string) const;
		std::string decrypt_connection_string(const std::string& encrypted_string) const;

	private:
		security_credentials credentials_;
		bool tls_configured_ = false;
	};

	/**
	 * @class query_security
	 * @brief SQL injection prevention and query security.
	 */
	class query_security
	{
	public:
		// SQL injection prevention
		static bool is_query_safe(const std::string& query);
		static std::string sanitize_input(const std::string& input);
		static std::string escape_sql_string(const std::string& value);

		// Query pattern analysis
		static bool detect_suspicious_patterns(const std::string& query);
		static std::vector<std::string> extract_table_names(const std::string& query);
		static bool validate_table_access(const std::string& table, const std::string& operation, const std::string& user);

		// Prepared statement support
		static std::string convert_to_prepared_statement(const std::string& query, const std::vector<database_value>& parameters);

	private:
		static const std::vector<std::string> dangerous_keywords_;
		static const std::vector<std::string> injection_patterns_;
	};

	/**
	 * @class access_control
	 * @brief Role-based access control (RBAC) system.
	 */
	class access_control
	{
	public:
		enum class permission {
			select = 1,
			insert = 2,
			update = 4,
			delete_record = 8,
			create = 16,
			drop = 32,
			alter = 64,
			admin = 128
		};

		struct role {
			std::string name;
			std::vector<permission> permissions;
			std::vector<std::string> allowed_tables;
			std::vector<std::string> denied_tables;
			std::chrono::system_clock::time_point created_at;
			bool active = true;
		};

		struct user_session {
			std::string user_id;
			std::string session_id;
			std::vector<std::string> roles;
			std::chrono::system_clock::time_point login_time;
			std::chrono::system_clock::time_point last_activity;
			std::string client_ip;
			bool active = true;
		};

		static access_control& instance();

		// Role management
		bool create_role(const role& new_role);
		bool assign_role_to_user(const std::string& user_id, const std::string& role_name);
		bool revoke_role_from_user(const std::string& user_id, const std::string& role_name);
		std::vector<role> get_user_roles(const std::string& user_id) const;

		// Permission checking
		bool check_permission(const std::string& user_id, const std::string& table,
		                     const std::string& operation) const;
		bool check_table_access(const std::string& user_id, const std::string& table) const;

		// Session management
		std::string create_session(const std::string& user_id, const std::string& client_ip);
		bool validate_session(const std::string& session_id) const;
		bool terminate_session(const std::string& session_id);
		void cleanup_expired_sessions();

	private:
		access_control() = default;

		mutable std::mutex access_mutex_;
		std::unordered_map<std::string, role> roles_;
		std::unordered_map<std::string, std::vector<std::string>> user_roles_;
		std::unordered_map<std::string, user_session> active_sessions_;
	};

	/**
	 * @class audit_logger
	 * @brief Security audit logging system.
	 */
	class audit_logger
	{
	public:
		static audit_logger& instance();

		// Audit logging
		void log_database_access(const std::string& user_id, const std::string& session_id,
		                        const std::string& operation, const std::string& table,
		                        const std::string& query_hash, bool success,
		                        const std::string& error_message = "");

		void log_authentication_event(const std::string& user_id, const std::string& client_ip,
		                             bool success, const std::string& method);

		void log_authorization_failure(const std::string& user_id, const std::string& operation,
		                             const std::string& table, const std::string& reason);

		// Audit retrieval
		std::vector<audit_log_entry> get_audit_logs(std::chrono::hours window) const;
		std::vector<audit_log_entry> get_user_audit_logs(const std::string& user_id,
		                                                 std::chrono::hours window) const;

		// Security reporting
		std::string generate_security_report(std::chrono::hours window) const;
		std::vector<std::string> detect_suspicious_activity(std::chrono::hours window) const;

		// Log management
		void set_log_retention_period(std::chrono::hours retention);
		void cleanup_old_logs();
		bool export_logs_to_file(const std::string& filename) const;

	private:
		audit_logger() = default;

		mutable std::mutex audit_mutex_;
		std::vector<audit_log_entry> audit_logs_;
		std::chrono::hours retention_period_{24 * 30}; // 30 days
	};

	/**
	 * @class security_monitor
	 * @brief Real-time security monitoring and alerting.
	 */
	class security_monitor
	{
	public:
		enum class threat_level {
			low,
			medium,
			high,
			critical
		};

		struct security_alert {
			threat_level level;
			std::string type;
			std::string description;
			std::string user_id;
			std::string session_id;
			std::chrono::system_clock::time_point timestamp;
		};

		static security_monitor& instance();

		// Threat detection
		void analyze_query_patterns(const std::string& user_id, const std::string& query);
		void detect_brute_force_attempts(const std::string& client_ip);
		void monitor_privilege_escalation(const std::string& user_id, const std::string& operation);

		// Alert management
		void register_security_handler(std::function<void(const security_alert&)> handler);
		std::vector<security_alert> get_recent_alerts(std::chrono::hours window) const;

		// Security metrics
		size_t get_failed_login_count(std::chrono::hours window) const;
		size_t get_suspicious_query_count(std::chrono::hours window) const;
		double calculate_security_score() const;

	private:
		security_monitor() = default;

		void emit_security_alert(threat_level level, const std::string& type,
		                        const std::string& description, const std::string& user_id = "");

		mutable std::mutex monitor_mutex_;
		std::vector<security_alert> security_alerts_;
		std::vector<std::function<void(const security_alert&)>> alert_handlers_;

		// Attack pattern tracking
		std::unordered_map<std::string, size_t> failed_login_attempts_;
		std::unordered_map<std::string, std::vector<std::string>> user_query_patterns_;
	};

	/**
	 * @class encryption_manager
	 * @brief Data encryption and key management.
	 */
	class encryption_manager
	{
	public:
		static encryption_manager& instance();

		// Data encryption
		std::string encrypt_field_data(const std::string& data, const std::string& field_name) const;
		std::string decrypt_field_data(const std::string& encrypted_data, const std::string& field_name) const;

		// Key management
		bool generate_field_key(const std::string& field_name);
		bool rotate_field_key(const std::string& field_name);
		void set_master_encryption_key(const std::string& key);

		// Column-level encryption
		bool configure_encrypted_column(const std::string& table, const std::string& column,
		                               encryption_type type);
		bool is_column_encrypted(const std::string& table, const std::string& column) const;

	private:
		encryption_manager() = default;

		std::string derive_key(const std::string& field_name) const;

		mutable std::mutex encryption_mutex_;
		std::string master_key_;
		std::unordered_map<std::string, std::string> field_keys_;
		std::unordered_map<std::string, encryption_type> encrypted_columns_;
	};

	// Utility functions for permission checking
	inline access_control::permission operator|(access_control::permission a, access_control::permission b) {
		return static_cast<access_control::permission>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline bool has_permission(access_control::permission permissions, access_control::permission check) {
		return (static_cast<int>(permissions) & static_cast<int>(check)) != 0;
	}

	// Helper macros for security logging
	#define AUDIT_LOG_ACCESS(user, session, op, table, query, success, error) \
		database::security::audit_logger::instance().log_database_access(user, session, op, table, query, success, error)

	#define AUDIT_LOG_AUTH(user, ip, success, method) \
		database::security::audit_logger::instance().log_authentication_event(user, ip, success, method)

	#define CHECK_PERMISSION(user, table, op) \
		database::security::access_control::instance().check_permission(user, table, op)

} // namespace database::security