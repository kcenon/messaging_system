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

#include "query_builder.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace database
{
	// query_condition implementation
	query_condition::query_condition(const std::string& field, const std::string& op, const database_value& value)
		: field_(field), operator_(op), value_(value)
	{
	}

	query_condition::query_condition(const std::string& raw_condition)
		: raw_condition_(raw_condition)
	{
	}

	std::string query_condition::to_sql() const
	{
		if (!raw_condition_.empty()) {
			return raw_condition_;
		}

		if (!sub_conditions_.empty()) {
			std::ostringstream oss;
			oss << "(";
			for (size_t i = 0; i < sub_conditions_.size(); ++i) {
				if (i > 0) {
					oss << " " << logical_operator_ << " ";
				}
				oss << sub_conditions_[i].to_sql();
			}
			oss << ")";
			return oss.str();
		}

		std::ostringstream oss;
		oss << field_ << " " << operator_ << " ";

		std::visit([&oss](const auto& val) {
			using T = std::decay_t<decltype(val)>;
			if constexpr (std::is_same_v<T, std::string>) {
				oss << "'" << val << "'";
			} else if constexpr (std::is_same_v<T, int64_t>) {
				oss << val;
			} else if constexpr (std::is_same_v<T, double>) {
				oss << val;
			} else if constexpr (std::is_same_v<T, bool>) {
				oss << (val ? "TRUE" : "FALSE");
			} else if constexpr (std::is_same_v<T, std::monostate>) {
				oss << "NULL";
			}
		}, value_);

		return oss.str();
	}

	std::string query_condition::to_mongodb() const
	{
		if (!raw_condition_.empty()) {
			return raw_condition_;
		}

		if (!sub_conditions_.empty()) {
			std::ostringstream oss;
			std::string mongo_op = (logical_operator_ == "AND") ? "$and" : "$or";
			oss << "{ \"" << mongo_op << "\": [";
			for (size_t i = 0; i < sub_conditions_.size(); ++i) {
				if (i > 0) oss << ", ";
				oss << sub_conditions_[i].to_mongodb();
			}
			oss << "] }";
			return oss.str();
		}

		std::ostringstream oss;
		oss << "{ \"" << field_ << "\": ";

		if (operator_ == "=") {
			std::visit([&oss](const auto& val) {
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, std::string>) {
					oss << "\"" << val << "\"";
				} else if constexpr (std::is_same_v<T, int64_t>) {
					oss << val;
				} else if constexpr (std::is_same_v<T, double>) {
					oss << val;
				} else if constexpr (std::is_same_v<T, bool>) {
					oss << (val ? "true" : "false");
				} else if constexpr (std::is_same_v<T, std::monostate>) {
					oss << "null";
				}
			}, value_);
		} else {
			std::string mongo_op;
			if (operator_ == ">") mongo_op = "$gt";
			else if (operator_ == ">=") mongo_op = "$gte";
			else if (operator_ == "<") mongo_op = "$lt";
			else if (operator_ == "<=") mongo_op = "$lte";
			else if (operator_ == "!=") mongo_op = "$ne";
			else mongo_op = "$eq";

			oss << "{ \"" << mongo_op << "\": ";
			std::visit([&oss](const auto& val) {
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, std::string>) {
					oss << "\"" << val << "\"";
				} else if constexpr (std::is_same_v<T, int64_t>) {
					oss << val;
				} else if constexpr (std::is_same_v<T, double>) {
					oss << val;
				} else if constexpr (std::is_same_v<T, bool>) {
					oss << (val ? "true" : "false");
				} else if constexpr (std::is_same_v<T, std::monostate>) {
					oss << "null";
				}
			}, value_);
			oss << " }";
		}

		oss << " }";
		return oss.str();
	}

	std::string query_condition::to_redis() const
	{
		return raw_condition_;
	}

	query_condition query_condition::operator&&(const query_condition& other) const
	{
		query_condition result("");
		result.sub_conditions_.push_back(*this);
		result.sub_conditions_.push_back(other);
		result.logical_operator_ = "AND";
		return result;
	}

	query_condition query_condition::operator||(const query_condition& other) const
	{
		query_condition result("");
		result.sub_conditions_.push_back(*this);
		result.sub_conditions_.push_back(other);
		result.logical_operator_ = "OR";
		return result;
	}

	// sql_query_builder implementation
	sql_query_builder::sql_query_builder()
		: type_(query_type::none)
		, limit_count_(0)
		, offset_count_(0)
	{
	}

	sql_query_builder& sql_query_builder::select(const std::vector<std::string>& columns)
	{
		type_ = query_type::select;
		select_columns_ = columns;
		return *this;
	}

	sql_query_builder& sql_query_builder::select(const std::string& column)
	{
		type_ = query_type::select;
		select_columns_ = {column};
		return *this;
	}

	sql_query_builder& sql_query_builder::select_raw(const std::string& raw_select)
	{
		type_ = query_type::select;
		select_columns_.clear();
		select_columns_.push_back(raw_select);
		return *this;
	}

	sql_query_builder& sql_query_builder::from(const std::string& table)
	{
		from_table_ = table;
		return *this;
	}

	sql_query_builder& sql_query_builder::where(const std::string& field, const std::string& op, const database_value& value)
	{
		where_conditions_.emplace_back(field, op, value);
		return *this;
	}

	sql_query_builder& sql_query_builder::where(const query_condition& condition)
	{
		where_conditions_.push_back(condition);
		return *this;
	}

	sql_query_builder& sql_query_builder::where_raw(const std::string& raw_where)
	{
		where_conditions_.emplace_back(raw_where);
		return *this;
	}

	sql_query_builder& sql_query_builder::or_where(const std::string& field, const std::string& op, const database_value& value)
	{
		if (!where_conditions_.empty()) {
			auto new_condition = query_condition(field, op, value);
			where_conditions_.back() = where_conditions_.back() || new_condition;
		} else {
			where_conditions_.emplace_back(field, op, value);
		}
		return *this;
	}

	sql_query_builder& sql_query_builder::join(const std::string& table, const std::string& condition, join_type type)
	{
		std::ostringstream oss;
		oss << join_type_to_string(type) << " JOIN " << table << " ON " << condition;
		joins_.push_back(oss.str());
		return *this;
	}

	sql_query_builder& sql_query_builder::left_join(const std::string& table, const std::string& condition)
	{
		return join(table, condition, join_type::left);
	}

	sql_query_builder& sql_query_builder::right_join(const std::string& table, const std::string& condition)
	{
		return join(table, condition, join_type::right);
	}

	sql_query_builder& sql_query_builder::group_by(const std::vector<std::string>& columns)
	{
		group_by_columns_ = columns;
		return *this;
	}

	sql_query_builder& sql_query_builder::group_by(const std::string& column)
	{
		group_by_columns_ = {column};
		return *this;
	}

	sql_query_builder& sql_query_builder::having(const std::string& condition)
	{
		having_clause_ = condition;
		return *this;
	}

	sql_query_builder& sql_query_builder::order_by(const std::string& column, sort_order order)
	{
		std::ostringstream oss;
		oss << column << " " << (order == sort_order::asc ? "ASC" : "DESC");
		order_by_clauses_.push_back(oss.str());
		return *this;
	}

	sql_query_builder& sql_query_builder::order_by_raw(const std::string& raw_order)
	{
		order_by_clauses_.push_back(raw_order);
		return *this;
	}

	sql_query_builder& sql_query_builder::limit(size_t count)
	{
		limit_count_ = count;
		return *this;
	}

	sql_query_builder& sql_query_builder::offset(size_t count)
	{
		offset_count_ = count;
		return *this;
	}

	sql_query_builder& sql_query_builder::insert_into(const std::string& table)
	{
		type_ = query_type::insert;
		target_table_ = table;
		return *this;
	}

	sql_query_builder& sql_query_builder::values(const std::map<std::string, database_value>& data)
	{
		set_data_ = data;
		return *this;
	}

	sql_query_builder& sql_query_builder::values(const std::vector<std::map<std::string, database_value>>& rows)
	{
		insert_rows_ = rows;
		return *this;
	}

	sql_query_builder& sql_query_builder::update(const std::string& table)
	{
		type_ = query_type::update;
		target_table_ = table;
		return *this;
	}

	sql_query_builder& sql_query_builder::set(const std::string& field, const database_value& value)
	{
		set_data_[field] = value;
		return *this;
	}

	sql_query_builder& sql_query_builder::set(const std::map<std::string, database_value>& data)
	{
		set_data_ = data;
		return *this;
	}

	sql_query_builder& sql_query_builder::delete_from(const std::string& table)
	{
		type_ = query_type::delete_query;
		target_table_ = table;
		return *this;
	}

	std::string sql_query_builder::build() const
	{
		return build_for_database(database_types::postgres);
	}

	std::string sql_query_builder::build_for_database(database_types db_type) const
	{
		std::ostringstream oss;

		switch (type_) {
			case query_type::select:
				oss << "SELECT ";
				if (select_columns_.empty()) {
					oss << "*";
				} else {
					for (size_t i = 0; i < select_columns_.size(); ++i) {
						if (i > 0) oss << ", ";
						oss << escape_identifier(select_columns_[i], db_type);
					}
				}
				if (!from_table_.empty()) {
					oss << " FROM " << escape_identifier(from_table_, db_type);
				}
				break;

			case query_type::insert:
				oss << "INSERT INTO " << escape_identifier(target_table_, db_type);
				if (!insert_rows_.empty()) {
					auto& first_row = insert_rows_[0];
					oss << " (";
					bool first = true;
					for (const auto& pair : first_row) {
						if (!first) oss << ", ";
						oss << escape_identifier(pair.first, db_type);
						first = false;
					}
					oss << ") VALUES ";
					for (size_t i = 0; i < insert_rows_.size(); ++i) {
						if (i > 0) oss << ", ";
						oss << "(";
						bool first_val = true;
						for (const auto& pair : insert_rows_[i]) {
							if (!first_val) oss << ", ";
							oss << format_value(pair.second, db_type);
							first_val = false;
						}
						oss << ")";
					}
				} else if (!set_data_.empty()) {
					oss << " (";
					bool first = true;
					for (const auto& pair : set_data_) {
						if (!first) oss << ", ";
						oss << escape_identifier(pair.first, db_type);
						first = false;
					}
					oss << ") VALUES (";
					first = true;
					for (const auto& pair : set_data_) {
						if (!first) oss << ", ";
						oss << format_value(pair.second, db_type);
						first = false;
					}
					oss << ")";
				}
				break;

			case query_type::update:
				oss << "UPDATE " << escape_identifier(target_table_, db_type) << " SET ";
				{
					bool first = true;
					for (const auto& pair : set_data_) {
						if (!first) oss << ", ";
						oss << escape_identifier(pair.first, db_type) << " = " << format_value(pair.second, db_type);
						first = false;
					}
				}
				break;

			case query_type::delete_query:
				oss << "DELETE FROM " << escape_identifier(target_table_, db_type);
				break;

			default:
				throw std::runtime_error("Invalid query type");
		}

		// Add JOINs
		for (const auto& join : joins_) {
			oss << " " << join;
		}

		// Add WHERE clause
		if (!where_conditions_.empty()) {
			oss << " WHERE ";
			for (size_t i = 0; i < where_conditions_.size(); ++i) {
				if (i > 0) oss << " AND ";
				oss << where_conditions_[i].to_sql();
			}
		}

		// Add GROUP BY
		if (!group_by_columns_.empty()) {
			oss << " GROUP BY ";
			for (size_t i = 0; i < group_by_columns_.size(); ++i) {
				if (i > 0) oss << ", ";
				oss << escape_identifier(group_by_columns_[i], db_type);
			}
		}

		// Add HAVING
		if (!having_clause_.empty()) {
			oss << " HAVING " << having_clause_;
		}

		// Add ORDER BY
		if (!order_by_clauses_.empty()) {
			oss << " ORDER BY ";
			for (size_t i = 0; i < order_by_clauses_.size(); ++i) {
				if (i > 0) oss << ", ";
				oss << order_by_clauses_[i];
			}
		}

		// Add LIMIT and OFFSET
		if (limit_count_ > 0) {
			oss << " LIMIT " << limit_count_;
		}
		if (offset_count_ > 0) {
			oss << " OFFSET " << offset_count_;
		}

		return oss.str();
	}

	void sql_query_builder::reset()
	{
		type_ = query_type::none;
		select_columns_.clear();
		from_table_.clear();
		where_conditions_.clear();
		joins_.clear();
		group_by_columns_.clear();
		having_clause_.clear();
		order_by_clauses_.clear();
		limit_count_ = 0;
		offset_count_ = 0;
		target_table_.clear();
		set_data_.clear();
		insert_rows_.clear();
	}

	std::string sql_query_builder::escape_identifier(const std::string& identifier, database_types db_type) const
	{
		switch (db_type) {
			case database_types::mysql:
				return "`" + identifier + "`";
			case database_types::postgres:
				return "\"" + identifier + "\"";
			case database_types::sqlite:
				return "[" + identifier + "]";
			default:
				return identifier;
		}
	}

	std::string sql_query_builder::format_value(const database_value& value, database_types db_type) const
	{
		std::ostringstream oss;
		std::visit([&oss](const auto& val) {
			using T = std::decay_t<decltype(val)>;
			if constexpr (std::is_same_v<T, std::string>) {
				oss << "'" << val << "'";
			} else if constexpr (std::is_same_v<T, int64_t>) {
				oss << val;
			} else if constexpr (std::is_same_v<T, double>) {
				oss << val;
			} else if constexpr (std::is_same_v<T, bool>) {
				oss << (val ? "TRUE" : "FALSE");
			} else if constexpr (std::is_same_v<T, std::monostate>) {
				oss << "NULL";
			}
		}, value);
		return oss.str();
	}

	std::string sql_query_builder::join_type_to_string(join_type type) const
	{
		switch (type) {
			case join_type::inner: return "INNER";
			case join_type::left: return "LEFT";
			case join_type::right: return "RIGHT";
			case join_type::full_outer: return "FULL OUTER";
			case join_type::cross: return "CROSS";
			default: return "INNER";
		}
	}

	// mongodb_query_builder implementation
	mongodb_query_builder::mongodb_query_builder()
		: type_(operation_type::none)
		, limit_count_(0)
		, skip_count_(0)
	{
	}

	mongodb_query_builder& mongodb_query_builder::collection(const std::string& name)
	{
		collection_name_ = name;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::find(const std::map<std::string, database_value>& filter)
	{
		type_ = operation_type::find;
		filter_ = filter;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::find_one(const std::map<std::string, database_value>& filter)
	{
		type_ = operation_type::find;
		filter_ = filter;
		limit_count_ = 1;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::project(const std::vector<std::string>& fields)
	{
		projection_.clear();
		for (const auto& field : fields) {
			projection_[field] = database_value{int64_t(1)};
		}
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::exclude(const std::vector<std::string>& fields)
	{
		for (const auto& field : fields) {
			projection_[field] = database_value{int64_t(0)};
		}
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::sort(const std::map<std::string, int>& sort_spec)
	{
		sort_spec_ = sort_spec;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::sort(const std::string& field, int direction)
	{
		sort_spec_[field] = direction;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::limit(size_t count)
	{
		limit_count_ = count;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::skip(size_t count)
	{
		skip_count_ = count;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::insert_one(const std::map<std::string, database_value>& document)
	{
		type_ = operation_type::insert;
		document_ = document;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::insert_many(const std::vector<std::map<std::string, database_value>>& documents)
	{
		type_ = operation_type::insert;
		documents_ = documents;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::update_one(const std::map<std::string, database_value>& filter,
														   const std::map<std::string, database_value>& update)
	{
		type_ = operation_type::update;
		filter_ = filter;
		update_spec_ = update;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::update_many(const std::map<std::string, database_value>& filter,
															const std::map<std::string, database_value>& update)
	{
		type_ = operation_type::update;
		filter_ = filter;
		update_spec_ = update;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::delete_one(const std::map<std::string, database_value>& filter)
	{
		type_ = operation_type::delete_op;
		filter_ = filter;
		limit_count_ = 1;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::delete_many(const std::map<std::string, database_value>& filter)
	{
		type_ = operation_type::delete_op;
		filter_ = filter;
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::match(const std::map<std::string, database_value>& conditions)
	{
		type_ = operation_type::aggregate;
		std::map<std::string, database_value> match_stage;
		match_stage["$match"] = database_value{std::string(to_json(conditions))};
		pipeline_.push_back(match_stage);
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::group(const std::map<std::string, database_value>& group_spec)
	{
		if (type_ != operation_type::aggregate) {
			type_ = operation_type::aggregate;
			pipeline_.clear();
		}
		std::map<std::string, database_value> group_stage;
		group_stage["$group"] = database_value{std::string(to_json(group_spec))};
		pipeline_.push_back(group_stage);
		return *this;
	}

	mongodb_query_builder& mongodb_query_builder::unwind(const std::string& field)
	{
		if (type_ != operation_type::aggregate) {
			type_ = operation_type::aggregate;
			pipeline_.clear();
		}
		std::map<std::string, database_value> unwind_stage;
		unwind_stage["$unwind"] = database_value{std::string("$" + field)};
		pipeline_.push_back(unwind_stage);
		return *this;
	}

	std::string mongodb_query_builder::build() const
	{
		std::ostringstream oss;

		switch (type_) {
			case operation_type::find:
				oss << "db." << collection_name_ << ".find(";
				oss << to_json(filter_);
				if (!projection_.empty()) {
					oss << ", " << to_json(projection_);
				}
				oss << ")";
				if (!sort_spec_.empty()) {
					oss << ".sort({";
					bool first = true;
					for (const auto& pair : sort_spec_) {
						if (!first) oss << ", ";
						oss << "\"" << pair.first << "\": " << pair.second;
						first = false;
					}
					oss << "})";
				}
				if (skip_count_ > 0) {
					oss << ".skip(" << skip_count_ << ")";
				}
				if (limit_count_ > 0) {
					oss << ".limit(" << limit_count_ << ")";
				}
				break;

			case operation_type::insert:
				if (documents_.empty()) {
					oss << "db." << collection_name_ << ".insertOne(" << to_json(document_) << ")";
				} else {
					oss << "db." << collection_name_ << ".insertMany([";
					for (size_t i = 0; i < documents_.size(); ++i) {
						if (i > 0) oss << ", ";
						oss << to_json(documents_[i]);
					}
					oss << "])";
				}
				break;

			case operation_type::update:
				oss << "db." << collection_name_ << ".updateOne(";
				oss << to_json(filter_) << ", { \"$set\": " << to_json(update_spec_) << " })";
				break;

			case operation_type::delete_op:
				if (limit_count_ == 1) {
					oss << "db." << collection_name_ << ".deleteOne(" << to_json(filter_) << ")";
				} else {
					oss << "db." << collection_name_ << ".deleteMany(" << to_json(filter_) << ")";
				}
				break;

			case operation_type::aggregate:
				oss << "db." << collection_name_ << ".aggregate([";
				for (size_t i = 0; i < pipeline_.size(); ++i) {
					if (i > 0) oss << ", ";
					oss << to_json(pipeline_[i]);
				}
				oss << "])";
				break;

			default:
				throw std::runtime_error("Invalid MongoDB operation type");
		}

		return oss.str();
	}

	std::string mongodb_query_builder::build_json() const
	{
		return build();
	}

	void mongodb_query_builder::reset()
	{
		type_ = operation_type::none;
		collection_name_.clear();
		filter_.clear();
		projection_.clear();
		sort_spec_.clear();
		limit_count_ = 0;
		skip_count_ = 0;
		document_.clear();
		documents_.clear();
		update_spec_.clear();
		pipeline_.clear();
	}

	std::string mongodb_query_builder::to_json(const std::map<std::string, database_value>& data) const
	{
		if (data.empty()) {
			return "{}";
		}

		std::ostringstream oss;
		oss << "{ ";
		bool first = true;
		for (const auto& pair : data) {
			if (!first) oss << ", ";
			oss << "\"" << pair.first << "\": " << value_to_json(pair.second);
			first = false;
		}
		oss << " }";
		return oss.str();
	}

	std::string mongodb_query_builder::value_to_json(const database_value& value) const
	{
		std::ostringstream oss;
		std::visit([&oss](const auto& val) {
			using T = std::decay_t<decltype(val)>;
			if constexpr (std::is_same_v<T, std::string>) {
				oss << "\"" << val << "\"";
			} else if constexpr (std::is_same_v<T, int64_t>) {
				oss << val;
			} else if constexpr (std::is_same_v<T, double>) {
				oss << val;
			} else if constexpr (std::is_same_v<T, bool>) {
				oss << (val ? "true" : "false");
			} else if constexpr (std::is_same_v<T, std::monostate>) {
				oss << "null";
			}
		}, value);
		return oss.str();
	}

	// redis_query_builder implementation
	redis_query_builder::redis_query_builder()
	{
	}

	redis_query_builder& redis_query_builder::set(const std::string& key, const std::string& value)
	{
		command_ = "SET";
		args_ = {key, value};
		return *this;
	}

	redis_query_builder& redis_query_builder::get(const std::string& key)
	{
		command_ = "GET";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::del(const std::string& key)
	{
		command_ = "DEL";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::exists(const std::string& key)
	{
		command_ = "EXISTS";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::hset(const std::string& key, const std::string& field, const std::string& value)
	{
		command_ = "HSET";
		args_ = {key, field, value};
		return *this;
	}

	redis_query_builder& redis_query_builder::hget(const std::string& key, const std::string& field)
	{
		command_ = "HGET";
		args_ = {key, field};
		return *this;
	}

	redis_query_builder& redis_query_builder::hdel(const std::string& key, const std::string& field)
	{
		command_ = "HDEL";
		args_ = {key, field};
		return *this;
	}

	redis_query_builder& redis_query_builder::hgetall(const std::string& key)
	{
		command_ = "HGETALL";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::lpush(const std::string& key, const std::string& value)
	{
		command_ = "LPUSH";
		args_ = {key, value};
		return *this;
	}

	redis_query_builder& redis_query_builder::rpush(const std::string& key, const std::string& value)
	{
		command_ = "RPUSH";
		args_ = {key, value};
		return *this;
	}

	redis_query_builder& redis_query_builder::lpop(const std::string& key)
	{
		command_ = "LPOP";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::rpop(const std::string& key)
	{
		command_ = "RPOP";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::lrange(const std::string& key, int start, int stop)
	{
		command_ = "LRANGE";
		args_ = {key, std::to_string(start), std::to_string(stop)};
		return *this;
	}

	redis_query_builder& redis_query_builder::sadd(const std::string& key, const std::string& member)
	{
		command_ = "SADD";
		args_ = {key, member};
		return *this;
	}

	redis_query_builder& redis_query_builder::srem(const std::string& key, const std::string& member)
	{
		command_ = "SREM";
		args_ = {key, member};
		return *this;
	}

	redis_query_builder& redis_query_builder::sismember(const std::string& key, const std::string& member)
	{
		command_ = "SISMEMBER";
		args_ = {key, member};
		return *this;
	}

	redis_query_builder& redis_query_builder::smembers(const std::string& key)
	{
		command_ = "SMEMBERS";
		args_ = {key};
		return *this;
	}

	redis_query_builder& redis_query_builder::expire(const std::string& key, int seconds)
	{
		command_ = "EXPIRE";
		args_ = {key, std::to_string(seconds)};
		return *this;
	}

	redis_query_builder& redis_query_builder::ttl(const std::string& key)
	{
		command_ = "TTL";
		args_ = {key};
		return *this;
	}

	std::string redis_query_builder::build() const
	{
		std::ostringstream oss;
		oss << command_;
		for (const auto& arg : args_) {
			oss << " " << arg;
		}
		return oss.str();
	}

	std::vector<std::string> redis_query_builder::build_args() const
	{
		std::vector<std::string> result;
		result.push_back(command_);
		result.insert(result.end(), args_.begin(), args_.end());
		return result;
	}

	void redis_query_builder::reset()
	{
		command_.clear();
		args_.clear();
	}

	// query_builder implementation
	query_builder::query_builder(database_types db_type)
		: db_type_(db_type)
	{
		ensure_builder();
	}

	query_builder& query_builder::for_database(database_types db_type)
	{
		db_type_ = db_type;
		ensure_builder();
		return *this;
	}

	query_builder& query_builder::select(const std::vector<std::string>& columns)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->select(columns);
		}
		return *this;
	}

	query_builder& query_builder::from(const std::string& table)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->from(table);
		}
		return *this;
	}

	query_builder& query_builder::where(const std::string& field, const std::string& op, const database_value& value)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->where(field, op, value);
		}
		return *this;
	}

	query_builder& query_builder::join(const std::string& table, const std::string& condition)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->join(table, condition);
		}
		return *this;
	}

	query_builder& query_builder::order_by(const std::string& column, sort_order order)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->order_by(column, order);
		}
		return *this;
	}

	query_builder& query_builder::limit(size_t count)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->limit(count);
		} else if (mongo_builder_) {
			mongo_builder_->limit(count);
		}
		return *this;
	}

	query_builder& query_builder::collection(const std::string& name)
	{
		ensure_builder();
		if (mongo_builder_) {
			mongo_builder_->collection(name);
		}
		return *this;
	}

	query_builder& query_builder::key(const std::string& key)
	{
		ensure_builder();
		if (redis_builder_) {
			redis_builder_->get(key);
		}
		return *this;
	}

	query_builder& query_builder::insert(const std::map<std::string, database_value>& data)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->values(data);
		} else if (mongo_builder_) {
			mongo_builder_->insert_one(data);
		}
		return *this;
	}

	query_builder& query_builder::update(const std::map<std::string, database_value>& data)
	{
		ensure_builder();
		if (sql_builder_) {
			sql_builder_->set(data);
		} else if (mongo_builder_) {
			mongo_builder_->update_one({}, data);
		}
		return *this;
	}

	query_builder& query_builder::remove()
	{
		ensure_builder();
		if (sql_builder_) {
			// DELETE command should be set via delete_from method
		} else if (mongo_builder_) {
			mongo_builder_->delete_many({});
		}
		return *this;
	}

	std::string query_builder::build() const
	{
		if (sql_builder_) {
			return sql_builder_->build_for_database(db_type_);
		} else if (mongo_builder_) {
			return mongo_builder_->build();
		} else if (redis_builder_) {
			return redis_builder_->build();
		}
		return "";
	}

	database_result query_builder::execute(database_base* db) const
	{
		if (!db) {
			return {};
		}

		std::string query = build();
		if (query.empty()) {
			return {};
		}

		return db->select_query(query);
	}

	void query_builder::reset()
	{
		if (sql_builder_) {
			sql_builder_->reset();
		}
		if (mongo_builder_) {
			mongo_builder_->reset();
		}
		if (redis_builder_) {
			redis_builder_->reset();
		}
	}

	void query_builder::ensure_builder()
	{
		switch (db_type_) {
			case database_types::postgres:
			case database_types::mysql:
			case database_types::sqlite:
				if (!sql_builder_) {
					sql_builder_ = std::make_unique<sql_query_builder>();
				}
				break;

			case database_types::mongodb:
				if (!mongo_builder_) {
					mongo_builder_ = std::make_unique<mongodb_query_builder>();
				}
				break;

			case database_types::redis:
				if (!redis_builder_) {
					redis_builder_ = std::make_unique<redis_query_builder>();
				}
				break;

			default:
				break;
		}
	}

} // namespace database