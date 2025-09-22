#pragma once

/**
 * @file database_types.h
 * @brief Defines the enumeration of supported database types.
 */

namespace database
{
	/**
	 * @enum database_types
	 * @brief Represents various database backends or modes.
	 *
	 * This enumeration is used to specify which database type is being
	 * targeted for operations like connection, querying, and so forth.
	 */
	enum class database_types {
		/**
		 * @brief No specific database type is set.
		 */
		none = 0,

		/**
		 * @brief Indicates a PostgreSQL database.
		 */
		postgres = 1
	};
} // namespace database
