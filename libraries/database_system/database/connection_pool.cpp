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

#include "connection_pool.h"
#include "database_manager.h"
#include "postgres_manager.h"
#include "backends/mysql/mysql_manager.h"
#include "backends/sqlite/sqlite_manager.h"
#include "backends/mongodb/mongodb_manager.h"
#include "backends/redis/redis_manager.h"
#include <iostream>
#include <algorithm>

namespace database
{
	// connection_wrapper implementation
	connection_wrapper::connection_wrapper(std::unique_ptr<database_base> conn)
		: connection_(std::move(conn))
		, is_healthy_(true)
		, last_used_(std::chrono::steady_clock::now())
	{
	}

	connection_wrapper::~connection_wrapper()
	{
		if (connection_) {
			connection_->disconnect();
		}
	}

	database_base* connection_wrapper::get() const
	{
		return connection_.get();
	}

	database_base* connection_wrapper::operator->() const
	{
		return connection_.get();
	}

	database_base& connection_wrapper::operator*() const
	{
		return *connection_;
	}

	bool connection_wrapper::is_healthy() const
	{
		return is_healthy_.load();
	}

	void connection_wrapper::mark_unhealthy()
	{
		is_healthy_.store(false);
	}

	void connection_wrapper::update_last_used()
	{
		std::lock_guard<std::mutex> lock(metadata_mutex_);
		last_used_ = std::chrono::steady_clock::now();
	}

	std::chrono::steady_clock::time_point connection_wrapper::last_used() const
	{
		std::lock_guard<std::mutex> lock(metadata_mutex_);
		return last_used_;
	}

	bool connection_wrapper::is_idle_timeout_exceeded(std::chrono::milliseconds timeout) const
	{
		auto now = std::chrono::steady_clock::now();
		auto idle_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_used());
		return idle_time > timeout;
	}

	// connection_pool implementation
	connection_pool::connection_pool(database_types db_type,
									 const connection_pool_config& config,
									 std::function<std::unique_ptr<database_base>()> factory)
		: db_type_(db_type)
		, config_(config)
		, connection_factory_(std::move(factory))
		, shutdown_requested_(false)
		, active_count_(0)
		, total_created_(0)
	{
	}

	connection_pool::~connection_pool()
	{
		shutdown();
	}

	bool connection_pool::initialize()
	{
		try {
			// Create minimum number of connections
			std::lock_guard<std::mutex> lock(pool_mutex_);

			for (size_t i = 0; i < config_.min_connections; ++i) {
				auto connection = create_connection();
				if (!connection) {
					std::cerr << "Failed to create initial connection " << i << std::endl;
					return false;
				}

				auto wrapper = std::make_shared<connection_wrapper>(std::move(connection));
				available_connections_.push(wrapper);
				++total_created_;
			}

			stats_.total_connections = total_created_.load();
			stats_.available_connections = available_connections_.size();

			// Start maintenance thread
			maintenance_thread_ = std::thread(&connection_pool::maintenance_thread, this);

			return true;
		} catch (const std::exception& e) {
			std::cerr << "Connection pool initialization failed: " << e.what() << std::endl;
			return false;
		}
	}

	std::shared_ptr<connection_wrapper> connection_pool::acquire_connection()
	{
		std::unique_lock<std::mutex> lock(pool_mutex_);

		// Wait for available connection or timeout
		auto deadline = std::chrono::steady_clock::now() + config_.acquire_timeout;

		while (available_connections_.empty() && !shutdown_requested_) {
			// Try to create new connection if under limit
			if (active_count_ + available_connections_.size() < config_.max_connections) {
				lock.unlock();
				auto new_conn = create_connection();
				lock.lock();

				if (new_conn) {
					auto wrapper = std::make_shared<connection_wrapper>(std::move(new_conn));
					available_connections_.push(wrapper);
					++total_created_;
					break;
				}
			}

			// Wait for connection to become available
			if (pool_condition_.wait_until(lock, deadline) == std::cv_status::timeout) {
				++stats_.failed_acquisitions;
				return nullptr; // Timeout
			}
		}

		if (shutdown_requested_ || available_connections_.empty()) {
			++stats_.failed_acquisitions;
			return nullptr;
		}

		// Get connection from pool
		auto connection = available_connections_.front();
		available_connections_.pop();
		++active_count_;
		++stats_.successful_acquisitions;

		// Update statistics
		stats_.active_connections = active_count_.load();
		stats_.available_connections = available_connections_.size();

		connection->update_last_used();
		return connection;
	}

	void connection_pool::release_connection(std::shared_ptr<connection_wrapper> connection)
	{
		if (!connection) return;

		std::lock_guard<std::mutex> lock(pool_mutex_);

		if (shutdown_requested_) {
			--active_count_;
			return;
		}

		// Check if connection is healthy
		if (!connection->is_healthy() || !validate_connection(connection.get())) {
			--active_count_;
			// Connection is unhealthy, don't return to pool
			return;
		}

		// Return connection to pool
		available_connections_.push(connection);
		--active_count_;

		// Update statistics
		stats_.active_connections = active_count_.load();
		stats_.available_connections = available_connections_.size();

		// Notify waiting threads
		pool_condition_.notify_one();
	}

	size_t connection_pool::active_connections() const
	{
		return active_count_.load();
	}

	size_t connection_pool::available_connections() const
	{
		std::lock_guard<std::mutex> lock(pool_mutex_);
		return available_connections_.size();
	}

	connection_stats connection_pool::get_stats() const
	{
		std::lock_guard<std::mutex> lock(pool_mutex_);
		stats_.total_connections = total_created_.load();
		stats_.active_connections = active_count_.load();
		stats_.available_connections = available_connections_.size();
		return stats_;
	}

	void connection_pool::shutdown()
	{
		shutdown_requested_.store(true);

		{
			std::lock_guard<std::mutex> lock(pool_mutex_);
			pool_condition_.notify_all();
		}

		if (maintenance_thread_.joinable()) {
			maintenance_thread_.join();
		}

		// Clear all connections
		std::lock_guard<std::mutex> lock(pool_mutex_);
		while (!available_connections_.empty()) {
			available_connections_.pop();
		}
	}

	std::unique_ptr<database_base> connection_pool::create_connection()
	{
		try {
			auto connection = connection_factory_();
			if (connection) {
				// Test connection
				try {
					auto type = connection->database_type();
					if (type != database_types::none) {
						return connection;
					}
				} catch (const std::exception&) {
					// Connection validation failed
				}
			}
		} catch (const std::exception& e) {
			std::cerr << "Failed to create connection: " << e.what() << std::endl;
		}
		return nullptr;
	}

	void connection_pool::maintenance_thread()
	{
		while (!shutdown_requested_) {
			std::this_thread::sleep_for(config_.health_check_interval);

			if (shutdown_requested_) break;

			try {
				if (config_.enable_health_checks) {
					health_check();
				}
				cleanup_idle_connections();
			} catch (const std::exception& e) {
				std::cerr << "Maintenance thread error: " << e.what() << std::endl;
			}
		}
	}

	void connection_pool::health_check()
	{
		std::lock_guard<std::mutex> lock(pool_mutex_);

		std::queue<std::shared_ptr<connection_wrapper>> healthy_connections;

		while (!available_connections_.empty()) {
			auto conn = available_connections_.front();
			available_connections_.pop();

			if (validate_connection(conn.get())) {
				healthy_connections.push(conn);
			}
			// Unhealthy connections are discarded
		}

		available_connections_ = std::move(healthy_connections);
		stats_.last_health_check = std::chrono::steady_clock::now();
		stats_.available_connections = available_connections_.size();
	}

	bool connection_pool::validate_connection(connection_wrapper* connection)
	{
		if (!connection || !connection->is_healthy()) {
			return false;
		}

		try {
			// Perform basic connectivity test based on database type
			auto* db = connection->get();
			if (!db) return false;

			// Simple validation - try to get database type
			auto type = db->database_type();
			return type != database_types::none;
		} catch (const std::exception&) {
			connection->mark_unhealthy();
			return false;
		}
	}


	void connection_pool::cleanup_idle_connections()
	{
		std::lock_guard<std::mutex> lock(pool_mutex_);

		std::queue<std::shared_ptr<connection_wrapper>> active_connections;

		while (!available_connections_.empty()) {
			auto conn = available_connections_.front();
			available_connections_.pop();

			// Keep connection if not idle or if we're at minimum
			if (!conn->is_idle_timeout_exceeded(config_.idle_timeout) ||
				active_connections.size() < config_.min_connections) {
				active_connections.push(conn);
			}
			// Idle connections beyond minimum are discarded
		}

		available_connections_ = std::move(active_connections);
		stats_.available_connections = available_connections_.size();
	}

	// connection_pool_manager implementation
	connection_pool_manager& connection_pool_manager::instance()
	{
		static connection_pool_manager instance;
		return instance;
	}

	connection_pool_manager::~connection_pool_manager()
	{
		shutdown_all();
	}

	bool connection_pool_manager::create_pool(database_types db_type, const connection_pool_config& config)
	{
		std::lock_guard<std::mutex> lock(pools_mutex_);

		if (pools_.find(db_type) != pools_.end()) {
			std::cerr << "Pool for database type already exists" << std::endl;
			return false;
		}

		auto factory = create_factory(db_type, config.connection_string);
		if (!factory) {
			std::cerr << "Failed to create connection factory for database type" << std::endl;
			return false;
		}

		auto pool = std::make_shared<connection_pool>(db_type, config, factory);
		if (!pool->initialize()) {
			std::cerr << "Failed to initialize connection pool" << std::endl;
			return false;
		}

		pools_[db_type] = pool;
		return true;
	}

	std::shared_ptr<connection_pool_base> connection_pool_manager::get_pool(database_types db_type)
	{
		std::lock_guard<std::mutex> lock(pools_mutex_);
		auto it = pools_.find(db_type);
		return (it != pools_.end()) ? it->second : nullptr;
	}

	void connection_pool_manager::remove_pool(database_types db_type)
	{
		std::lock_guard<std::mutex> lock(pools_mutex_);
		auto it = pools_.find(db_type);
		if (it != pools_.end()) {
			it->second->shutdown();
			pools_.erase(it);
		}
	}

	void connection_pool_manager::shutdown_all()
	{
		std::lock_guard<std::mutex> lock(pools_mutex_);
		for (auto& pair : pools_) {
			pair.second->shutdown();
		}
		pools_.clear();
	}

	std::map<database_types, connection_stats> connection_pool_manager::get_all_stats() const
	{
		std::lock_guard<std::mutex> lock(pools_mutex_);
		std::map<database_types, connection_stats> all_stats;

		for (const auto& pair : pools_) {
			all_stats[pair.first] = pair.second->get_stats();
		}

		return all_stats;
	}

	std::function<std::unique_ptr<database_base>()> connection_pool_manager::create_factory(
		database_types db_type,
		const std::string& connection_string)
	{
		switch (db_type) {
			case database_types::postgres:
				return [connection_string]() -> std::unique_ptr<database_base> {
					auto conn = std::make_unique<postgres_manager>();
					if (conn->connect(connection_string)) {
						return conn;
					}
					return nullptr;
				};

			case database_types::mysql:
				return [connection_string]() -> std::unique_ptr<database_base> {
					auto conn = std::make_unique<mysql_manager>();
					if (conn->connect(connection_string)) {
						return conn;
					}
					return nullptr;
				};

			case database_types::sqlite:
				return [connection_string]() -> std::unique_ptr<database_base> {
					auto conn = std::make_unique<sqlite_manager>();
					if (conn->connect(connection_string)) {
						return conn;
					}
					return nullptr;
				};

			case database_types::mongodb:
				return [connection_string]() -> std::unique_ptr<database_base> {
					auto conn = std::make_unique<mongodb_manager>();
					if (conn->connect(connection_string)) {
						return conn;
					}
					return nullptr;
				};

			case database_types::redis:
				return [connection_string]() -> std::unique_ptr<database_base> {
					auto conn = std::make_unique<redis_manager>();
					if (conn->connect(connection_string)) {
						return conn;
					}
					return nullptr;
				};

			default:
				return nullptr;
		}
	}

} // namespace database