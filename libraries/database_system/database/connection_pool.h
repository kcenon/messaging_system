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

#include "database_base.h"
#include "database_types.h"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

namespace database
{
	/**
	 * @struct connection_pool_config
	 * @brief Configuration parameters for connection pools.
	 */
	struct connection_pool_config
	{
		size_t min_connections = 2;        ///< Minimum number of connections to maintain
		size_t max_connections = 20;       ///< Maximum number of connections allowed
		std::chrono::milliseconds acquire_timeout{5000}; ///< Timeout for acquiring connections
		std::chrono::milliseconds idle_timeout{30000};   ///< Timeout for idle connections
		std::chrono::milliseconds health_check_interval{60000}; ///< Health check interval
		bool enable_health_checks = true;  ///< Enable periodic health checks
		std::string connection_string;     ///< Database connection string
	};

	/**
	 * @struct connection_stats
	 * @brief Statistics for connection pool monitoring.
	 */
	struct connection_stats
	{
		size_t total_connections = 0;      ///< Total connections created
		size_t active_connections = 0;     ///< Currently active connections
		size_t available_connections = 0;  ///< Available connections in pool
		size_t failed_acquisitions = 0;    ///< Number of failed connection acquisitions
		size_t successful_acquisitions = 0; ///< Number of successful acquisitions
		std::chrono::steady_clock::time_point last_health_check; ///< Last health check time
	};

	/**
	 * @class connection_wrapper
	 * @brief Wrapper for database connections with metadata.
	 */
	class connection_wrapper
	{
	public:
		connection_wrapper(std::unique_ptr<database_base> conn);
		~connection_wrapper();

		database_base* get() const;
		database_base* operator->() const;
		database_base& operator*() const;

		bool is_healthy() const;
		void mark_unhealthy();
		void update_last_used();
		std::chrono::steady_clock::time_point last_used() const;
		bool is_idle_timeout_exceeded(std::chrono::milliseconds timeout) const;

	private:
		std::unique_ptr<database_base> connection_;
		std::atomic<bool> is_healthy_;
		std::chrono::steady_clock::time_point last_used_;
		mutable std::mutex metadata_mutex_;
	};

	/**
	 * @class connection_pool_base
	 * @brief Abstract base class for database connection pools.
	 */
	class connection_pool_base
	{
	public:
		virtual ~connection_pool_base() = default;

		/**
		 * @brief Acquires a connection from the pool.
		 * @return Shared pointer to a connection wrapper, nullptr if timeout
		 */
		virtual std::shared_ptr<connection_wrapper> acquire_connection() = 0;

		/**
		 * @brief Returns a connection to the pool.
		 * @param connection The connection to return
		 */
		virtual void release_connection(std::shared_ptr<connection_wrapper> connection) = 0;

		/**
		 * @brief Gets the number of active connections.
		 * @return Number of active connections
		 */
		virtual size_t active_connections() const = 0;

		/**
		 * @brief Gets the number of available connections.
		 * @return Number of available connections
		 */
		virtual size_t available_connections() const = 0;

		/**
		 * @brief Gets connection pool statistics.
		 * @return Connection statistics
		 */
		virtual connection_stats get_stats() const = 0;

		/**
		 * @brief Shuts down the connection pool.
		 */
		virtual void shutdown() = 0;
	};

	/**
	 * @class connection_pool
	 * @brief Generic connection pool implementation.
	 */
	class connection_pool : public connection_pool_base
	{
	public:
		/**
		 * @brief Constructs a connection pool.
		 * @param db_type Database type for this pool
		 * @param config Pool configuration
		 * @param factory Function to create new database connections
		 */
		connection_pool(database_types db_type,
						const connection_pool_config& config,
						std::function<std::unique_ptr<database_base>()> factory);

		/**
		 * @brief Destructor.
		 */
		virtual ~connection_pool();

		// Inherited from connection_pool_base
		std::shared_ptr<connection_wrapper> acquire_connection() override;
		void release_connection(std::shared_ptr<connection_wrapper> connection) override;
		size_t active_connections() const override;
		size_t available_connections() const override;
		connection_stats get_stats() const override;
		void shutdown() override;

		/**
		 * @brief Initializes the connection pool.
		 * @return true if initialization successful, false otherwise
		 */
		bool initialize();

		/**
		 * @brief Performs health check on all connections.
		 */
		void health_check();

	private:
		/**
		 * @brief Creates a new database connection.
		 * @return Unique pointer to new connection, nullptr on failure
		 */
		std::unique_ptr<database_base> create_connection();

		/**
		 * @brief Background thread for maintenance tasks.
		 */
		void maintenance_thread();

		/**
		 * @brief Validates a connection's health.
		 * @param connection Connection to validate
		 * @return true if connection is healthy, false otherwise
		 */
		bool validate_connection(connection_wrapper* connection);

		/**
		 * @brief Removes idle connections exceeding timeout.
		 */
		void cleanup_idle_connections();

	private:
		database_types db_type_;
		connection_pool_config config_;
		std::function<std::unique_ptr<database_base>()> connection_factory_;

		mutable std::mutex pool_mutex_;
		std::condition_variable pool_condition_;
		std::queue<std::shared_ptr<connection_wrapper>> available_connections_;

		mutable connection_stats stats_;
		std::atomic<bool> shutdown_requested_;
		std::thread maintenance_thread_;

		std::atomic<size_t> active_count_;
		std::atomic<size_t> total_created_;
	};

	/**
	 * @class connection_pool_manager
	 * @brief Manages multiple connection pools for different database types.
	 */
	class connection_pool_manager
	{
	public:
		/**
		 * @brief Gets the singleton instance.
		 * @return Reference to the connection pool manager
		 */
		static connection_pool_manager& instance();

		/**
		 * @brief Creates a connection pool for a database type.
		 * @param db_type Database type
		 * @param config Pool configuration
		 * @return true if pool created successfully, false otherwise
		 */
		bool create_pool(database_types db_type, const connection_pool_config& config);

		/**
		 * @brief Gets a connection pool for a database type.
		 * @param db_type Database type
		 * @return Shared pointer to connection pool, nullptr if not found
		 */
		std::shared_ptr<connection_pool_base> get_pool(database_types db_type);

		/**
		 * @brief Removes a connection pool.
		 * @param db_type Database type
		 */
		void remove_pool(database_types db_type);

		/**
		 * @brief Shuts down all connection pools.
		 */
		void shutdown_all();

		/**
		 * @brief Gets statistics for all pools.
		 * @return Map of database type to connection stats
		 */
		std::map<database_types, connection_stats> get_all_stats() const;

	private:
		connection_pool_manager() = default;
		~connection_pool_manager();

		/**
		 * @brief Creates a connection factory for a database type.
		 * @param db_type Database type
		 * @param connection_string Connection string
		 * @return Function that creates database connections
		 */
		std::function<std::unique_ptr<database_base>()> create_factory(
			database_types db_type,
			const std::string& connection_string);

	private:
		mutable std::mutex pools_mutex_;
		std::map<database_types, std::shared_ptr<connection_pool>> pools_;
	};

} // namespace database