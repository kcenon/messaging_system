#pragma once

/**
 * @file database_types.h
 * @brief Defines the enumeration of supported database types.
 */

#include <cstdint>

namespace database
{
	/**
	 * @enum database_types
	 * @brief Represents various database backends or modes.
	 *
	 * This enumeration is used to specify which database type is being
	 * targeted for operations like connection, querying, and so forth.
	 */
	enum class database_types : uint8_t {
		/**
		 * @brief No specific database type is set.
		 */
		none = 0,

		/**
		 * @brief Indicates a PostgreSQL database.
		 */
		postgres = 1,

		/**
		 * @brief Indicates a MySQL/MariaDB database.
		 */
		mysql = 2,

		/**
		 * @brief Indicates a SQLite database.
		 */
		sqlite = 3,

		/**
		 * @brief Indicates an Oracle database (future implementation).
		 */
		oracle = 4,

		/**
		 * @brief Indicates a MongoDB database (future implementation).
		 */
		mongodb = 5,

		/**
		 * @brief Indicates a Redis database (future implementation).
		 */
		redis = 6
	};

	/**
	 * @brief Converts database_types enum to string representation.
	 * @param type The database type to convert.
	 * @return String representation of the database type.
	 */
	constexpr const char* to_string(database_types type) noexcept
	{
		switch (type) {
		case database_types::none: return "none";
		case database_types::postgres: return "postgres";
		case database_types::mysql: return "mysql";
		case database_types::sqlite: return "sqlite";
		case database_types::oracle: return "oracle";
		case database_types::mongodb: return "mongodb";
		case database_types::redis: return "redis";
		default: return "unknown";
		}
	}
} // namespace database
