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
#include "../database_base.h"
#include <string>
#include <vector>
#include <memory>
#include <concepts>
#include <type_traits>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <optional>

namespace database::orm
{
	// Forward declarations
	class entity_base;
	class field_metadata;
	class entity_metadata;

	// C++20 concepts for type safety
	template<typename T>
	concept Entity = requires(T t) {
		typename T::primary_key_type;
		{ t.table_name() } -> std::convertible_to<std::string>;
		{ t.get_metadata() } -> std::same_as<const entity_metadata&>;
	};

	template<typename T>
	concept FieldType = std::is_same_v<T, int32_t> ||
	                   std::is_same_v<T, int64_t> ||
	                   std::is_same_v<T, double> ||
	                   std::is_same_v<T, std::string> ||
	                   std::is_same_v<T, bool> ||
	                   std::is_same_v<T, std::chrono::system_clock::time_point>;

	// Forward declaration with proper constraint
	template<Entity EntityType> class query_builder;

	// Field constraint types
	enum class field_constraint {
		none = 0,
		primary_key = 1,
		not_null = 2,
		unique = 4,
		auto_increment = 8,
		index = 16,
		foreign_key = 32,
		default_now = 64
	};

	inline field_constraint operator|(field_constraint a, field_constraint b) {
		return static_cast<field_constraint>(static_cast<int>(a) | static_cast<int>(b));
	}

	inline bool has_constraint(field_constraint constraints, field_constraint check) {
		return (static_cast<int>(constraints) & static_cast<int>(check)) != 0;
	}

	/**
	 * @class field_metadata
	 * @brief Metadata for entity fields including constraints and relationships.
	 */
	class field_metadata
	{
	public:
		field_metadata(const std::string& name,
		               const std::string& type_name,
		               field_constraint constraints = field_constraint::none,
		               const std::string& index_name = "",
		               const std::string& foreign_table = "",
		               const std::string& foreign_field = "");

		const std::string& name() const { return name_; }
		const std::string& type_name() const { return type_name_; }
		field_constraint constraints() const { return constraints_; }
		const std::string& index_name() const { return index_name_; }
		const std::string& foreign_table() const { return foreign_table_; }
		const std::string& foreign_field() const { return foreign_field_; }

		bool is_primary_key() const { return has_constraint(constraints_, field_constraint::primary_key); }
		bool is_not_null() const { return has_constraint(constraints_, field_constraint::not_null); }
		bool is_unique() const { return has_constraint(constraints_, field_constraint::unique); }
		bool is_auto_increment() const { return has_constraint(constraints_, field_constraint::auto_increment); }
		bool has_index() const { return has_constraint(constraints_, field_constraint::index); }
		bool is_foreign_key() const { return has_constraint(constraints_, field_constraint::foreign_key); }
		bool has_default_now() const { return has_constraint(constraints_, field_constraint::default_now); }

		std::string to_sql_definition() const;

	private:
		std::string name_;
		std::string type_name_;
		field_constraint constraints_;
		std::string index_name_;
		std::string foreign_table_;
		std::string foreign_field_;
	};

	/**
	 * @class entity_metadata
	 * @brief Metadata for entire entities including table mapping and relationships.
	 */
	class entity_metadata
	{
	public:
		entity_metadata(const std::string& table_name);

		void add_field(const field_metadata& field);
		const std::vector<field_metadata>& fields() const { return fields_; }
		const std::string& table_name() const { return table_name_; }

		const field_metadata* get_primary_key() const;
		std::vector<const field_metadata*> get_indexes() const;
		std::vector<const field_metadata*> get_foreign_keys() const;

		std::string create_table_sql() const;
		std::string create_indexes_sql() const;

	private:
		std::string table_name_;
		std::vector<field_metadata> fields_;
	};

	/**
	 * @class entity_base
	 * @brief Base class for all ORM entities.
	 */
	class entity_base
	{
	public:
		virtual ~entity_base() = default;
		virtual std::string table_name() const = 0;
		virtual const entity_metadata& get_metadata() const = 0;

		// CRUD operations
		virtual bool save() = 0;
		virtual bool load() = 0;
		virtual bool update() = 0;
		virtual bool remove() = 0;

	protected:
		entity_base() = default;
	};

	/**
	 * @class field_accessor
	 * @brief Template class for type-safe field access.
	 */
	template<FieldType T>
	class field_accessor
	{
	public:
		field_accessor(T& value, const field_metadata& metadata)
			: value_(value), metadata_(metadata) {}

		T& get() { return value_; }
		const T& get() const { return value_; }
		const field_metadata& metadata() const { return metadata_; }

		field_accessor& operator=(const T& value) {
			value_ = value;
			return *this;
		}

		operator T&() { return value_; }
		operator const T&() const { return value_; }

	private:
		T& value_;
		const field_metadata& metadata_;
	};

	/**
	 * @class query_builder
	 * @brief Template query builder for type-safe ORM queries.
	 */
	template<Entity EntityType>
	class query_builder
	{
	public:
		query_builder(std::shared_ptr<database_base> db);

		// Query building methods
		query_builder& where(const std::string& condition);
		query_builder& order_by(const std::string& field, bool ascending = true);
		query_builder& limit(size_t count);
		query_builder& offset(size_t count);

		// Join operations
		template<Entity OtherEntity>
		query_builder& join(const std::string& condition);

		template<Entity OtherEntity>
		query_builder& left_join(const std::string& condition);

		// Execution methods
		std::vector<EntityType> execute();
		std::optional<EntityType> first();
		size_t count();

		// Aggregation methods
		double sum(const std::string& field);
		double avg(const std::string& field);
		database_value min(const std::string& field);
		database_value max(const std::string& field);

	private:
		std::shared_ptr<database_base> db_;
		std::string where_clause_;
		std::string order_clause_;
		std::string join_clause_;
		size_t limit_count_ = 0;
		size_t offset_count_ = 0;

		std::string build_query() const;
		EntityType map_result_to_entity(const database_result& result, size_t row) const;
	};

	/**
	 * @class entity_manager
	 * @brief Manages entity metadata and provides factory methods.
	 */
	class entity_manager
	{
	public:
		static entity_manager& instance();

		template<Entity EntityType>
		void register_entity();

		template<Entity EntityType>
		const entity_metadata& get_metadata();

		template<Entity EntityType>
		query_builder<EntityType> query(std::shared_ptr<database_base> db);

		// Schema operations
		bool create_tables(std::shared_ptr<database_base> db);
		bool drop_tables(std::shared_ptr<database_base> db);
		bool sync_schema(std::shared_ptr<database_base> db);

	private:
		std::unordered_map<std::string, std::unique_ptr<entity_metadata>> metadata_cache_;
		entity_manager() = default;
	};

	// Helper macros for entity definition
	#define ENTITY_FIELD(type, name, ...) \
		private: \
			type name##_; \
			static inline field_metadata name##_metadata_{#name, #type, __VA_ARGS__}; \
		public: \
			field_accessor<type> name{name##_, name##_metadata_}; \
			static const field_metadata& name##_field() { return name##_metadata_; }

	#define ENTITY_TABLE(table_name) \
		public: \
			std::string table_name() const override { return table_name; } \
			using primary_key_type = decltype(id_); \
		private: \
			static inline entity_metadata metadata_{table_name};

	#define ENTITY_METADATA() \
		public: \
			const entity_metadata& get_metadata() const override { \
				static bool initialized = false; \
				if (!initialized) { \
					initialize_metadata(); \
					initialized = true; \
				} \
				return metadata_; \
			} \
		private: \
			static void initialize_metadata();

	// Constraint helper functions
	inline field_constraint primary_key() { return field_constraint::primary_key; }
	inline field_constraint not_null() { return field_constraint::not_null; }
	inline field_constraint unique() { return field_constraint::unique; }
	inline field_constraint auto_increment() { return field_constraint::auto_increment; }
	inline field_constraint default_now() { return field_constraint::default_now; }

	inline field_constraint index(const std::string& name = "") {
		return field_constraint::index;
	}

	inline field_constraint foreign_key(const std::string& table, const std::string& field) {
		return field_constraint::foreign_key;
	}

} // namespace database::orm