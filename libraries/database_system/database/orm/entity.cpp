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

#include "entity.h"
#include <sstream>
#include <algorithm>
#include <iostream>

namespace database::orm
{
	// field_metadata implementation
	field_metadata::field_metadata(const std::string& name,
	                               const std::string& type_name,
	                               field_constraint constraints,
	                               const std::string& index_name,
	                               const std::string& foreign_table,
	                               const std::string& foreign_field)
		: name_(name)
		, type_name_(type_name)
		, constraints_(constraints)
		, index_name_(index_name)
		, foreign_table_(foreign_table)
		, foreign_field_(foreign_field)
	{
	}

	std::string field_metadata::to_sql_definition() const
	{
		std::ostringstream oss;
		oss << name_ << " ";

		// Map C++ types to SQL types
		if (type_name_ == "int32_t" || type_name_ == "int") {
			oss << "INTEGER";
		} else if (type_name_ == "int64_t") {
			oss << "BIGINT";
		} else if (type_name_ == "double") {
			oss << "DOUBLE PRECISION";
		} else if (type_name_ == "std::string") {
			oss << "VARCHAR(255)";
		} else if (type_name_ == "bool") {
			oss << "BOOLEAN";
		} else if (type_name_.find("time_point") != std::string::npos) {
			oss << "TIMESTAMP";
		} else {
			oss << "TEXT";
		}

		// Add constraints
		if (is_primary_key()) {
			oss << " PRIMARY KEY";
		}
		if (is_auto_increment()) {
			oss << " AUTO_INCREMENT";
		}
		if (is_not_null() && !is_primary_key()) {
			oss << " NOT NULL";
		}
		if (is_unique() && !is_primary_key()) {
			oss << " UNIQUE";
		}
		if (has_default_now()) {
			oss << " DEFAULT CURRENT_TIMESTAMP";
		}

		return oss.str();
	}

	// entity_metadata implementation
	entity_metadata::entity_metadata(const std::string& table_name)
		: table_name_(table_name)
	{
	}

	void entity_metadata::add_field(const field_metadata& field)
	{
		fields_.push_back(field);
	}

	const field_metadata* entity_metadata::get_primary_key() const
	{
		auto it = std::find_if(fields_.begin(), fields_.end(),
			[](const field_metadata& field) {
				return field.is_primary_key();
			});
		return (it != fields_.end()) ? &(*it) : nullptr;
	}

	std::vector<const field_metadata*> entity_metadata::get_indexes() const
	{
		std::vector<const field_metadata*> indexes;
		for (const auto& field : fields_) {
			if (field.has_index()) {
				indexes.push_back(&field);
			}
		}
		return indexes;
	}

	std::vector<const field_metadata*> entity_metadata::get_foreign_keys() const
	{
		std::vector<const field_metadata*> foreign_keys;
		for (const auto& field : fields_) {
			if (field.is_foreign_key()) {
				foreign_keys.push_back(&field);
			}
		}
		return foreign_keys;
	}

	std::string entity_metadata::create_table_sql() const
	{
		std::ostringstream oss;
		oss << "CREATE TABLE IF NOT EXISTS " << table_name_ << " (\n";

		bool first = true;
		for (const auto& field : fields_) {
			if (!first) oss << ",\n";
			oss << "  " << field.to_sql_definition();
			first = false;
		}

		// Add foreign key constraints
		auto foreign_keys = get_foreign_keys();
		for (const auto* fk : foreign_keys) {
			oss << ",\n  FOREIGN KEY (" << fk->name() << ") REFERENCES "
			    << fk->foreign_table() << "(" << fk->foreign_field() << ")";
		}

		oss << "\n)";
		return oss.str();
	}

	std::string entity_metadata::create_indexes_sql() const
	{
		std::ostringstream oss;
		auto indexes = get_indexes();

		for (const auto* index : indexes) {
			if (!index->index_name().empty()) {
				oss << "CREATE INDEX IF NOT EXISTS " << index->index_name()
				    << " ON " << table_name_ << "(" << index->name() << ");\n";
			} else {
				oss << "CREATE INDEX IF NOT EXISTS idx_" << table_name_
				    << "_" << index->name() << " ON " << table_name_
				    << "(" << index->name() << ");\n";
			}
		}

		return oss.str();
	}

	// entity_manager implementation
	entity_manager& entity_manager::instance()
	{
		static entity_manager instance;
		return instance;
	}

	bool entity_manager::create_tables(std::shared_ptr<database_base> db)
	{
		if (!db) {
			std::cerr << "Database connection is null" << std::endl;
			return false;
		}

		try {
			for (const auto& [name, metadata] : metadata_cache_) {
				// Create table
				std::string create_sql = metadata->create_table_sql();
				if (!db->execute_query(create_sql)) {
					std::cerr << "Failed to create table: " << name << std::endl;
					return false;
				}

				// Create indexes
				std::string index_sql = metadata->create_indexes_sql();
				if (!index_sql.empty() && !db->execute_query(index_sql)) {
					std::cerr << "Failed to create indexes for table: " << name << std::endl;
					return false;
				}
			}
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Exception during table creation: " << e.what() << std::endl;
			return false;
		}
	}

	bool entity_manager::drop_tables(std::shared_ptr<database_base> db)
	{
		if (!db) {
			std::cerr << "Database connection is null" << std::endl;
			return false;
		}

		try {
			for (const auto& [name, metadata] : metadata_cache_) {
				std::string drop_sql = "DROP TABLE IF EXISTS " + metadata->table_name();
				if (!db->execute_query(drop_sql)) {
					std::cerr << "Failed to drop table: " << name << std::endl;
					return false;
				}
			}
			return true;
		} catch (const std::exception& e) {
			std::cerr << "Exception during table dropping: " << e.what() << std::endl;
			return false;
		}
	}

	bool entity_manager::sync_schema(std::shared_ptr<database_base> db)
	{
		if (!db) {
			std::cerr << "Database connection is null" << std::endl;
			return false;
		}

		try {
			// For now, just recreate all tables
			// In a real implementation, this would do schema diffing
			if (!drop_tables(db)) {
				std::cerr << "Failed to drop existing tables" << std::endl;
				return false;
			}

			if (!create_tables(db)) {
				std::cerr << "Failed to create new tables" << std::endl;
				return false;
			}

			return true;
		} catch (const std::exception& e) {
			std::cerr << "Exception during schema sync: " << e.what() << std::endl;
			return false;
		}
	}

	// Note: Template implementations moved to header file to avoid
	// template instantiation issues. Only non-template methods implemented here.

} // namespace database::orm