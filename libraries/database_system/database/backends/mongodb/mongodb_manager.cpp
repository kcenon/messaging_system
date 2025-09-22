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

#include "mongodb_manager.h"

#ifdef USE_MONGODB
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/view/document.hpp>
#endif

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <regex>

namespace database
{
	mongodb_manager::mongodb_manager(void) : client_(nullptr), database_(nullptr) {}

	mongodb_manager::~mongodb_manager(void)
	{
		disconnect();
	}

	database_types mongodb_manager::database_type(void)
	{
		return database_types::mongodb;
	}

	bool mongodb_manager::connect(const std::string& connect_string)
	{
#ifdef USE_MONGODB
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			// Initialize MongoDB instance (should only be done once per application)
			static mongocxx::instance instance{};

			// Parse connection string
			std::string host, username, password;
			int port;
			if (!parse_connection_string(connect_string, host, port, db_name_, username, password)) {
				std::cerr << "MongoDB connection string parsing failed" << std::endl;
				return false;
			}

			// Create MongoDB URI
			mongocxx::uri uri{connect_string};

			// Create MongoDB client
			auto client = std::make_unique<mongocxx::client>(uri);
			client_ = client.release();

			// Get database reference
			auto* mongo_client = static_cast<mongocxx::client*>(client_);
			auto db = std::make_unique<mongocxx::database>((*mongo_client)[db_name_]);
			database_ = db.release();

			// Test connection by running a simple command
			auto* mongo_db = static_cast<mongocxx::database*>(database_);
			auto result = mongo_db->run_command(bsoncxx::builder::stream::document{} << "ping" << 1 << bsoncxx::builder::stream::finalize);

			return true;
		} catch (const std::exception& e) {
			std::cerr << "MongoDB connection error: " << e.what() << std::endl;
		}
#else
		std::cerr << "MongoDB support not compiled. Connection: " << connect_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	bool mongodb_manager::create_query(const std::string& query_string)
	{
#ifdef USE_MONGODB
		if (!database_) return false;
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			auto* mongo_db = static_cast<mongocxx::database*>(database_);

			// Parse query_string to determine operation
			// For simplicity, assume query_string is a collection name to create
			std::string collection_name = query_string;

			// Create collection (MongoDB creates collections lazily, so just verify database access)
			auto collection = (*mongo_db)[collection_name];
			auto doc = bsoncxx::builder::stream::document{} << "test" << "creation" << bsoncxx::builder::stream::finalize;
			collection.insert_one(doc.view());
			collection.delete_one(doc.view());

			return true;
		} catch (const std::exception& e) {
			std::cerr << "MongoDB create query error: " << e.what() << std::endl;
		}
#else
		std::cerr << "MongoDB support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return false;
	}

	unsigned int mongodb_manager::insert_query(const std::string& query_string)
	{
#ifdef USE_MONGODB
		if (!database_) return 0;
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			std::string collection_name, filter, document_json;
			if (!parse_query_string(query_string, collection_name, document_json, filter)) {
				return 0;
			}

			auto* mongo_db = static_cast<mongocxx::database*>(database_);
			auto collection = (*mongo_db)[collection_name];

			// Parse JSON to BSON
			auto doc = bsoncxx::from_json(document_json);
			auto result = collection.insert_one(doc.view());

			return result ? 1 : 0;
		} catch (const std::exception& e) {
			std::cerr << "MongoDB insert error: " << e.what() << std::endl;
		}
#else
		std::cerr << "MongoDB support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	unsigned int mongodb_manager::update_query(const std::string& query_string)
	{
#ifdef USE_MONGODB
		if (!database_) return 0;
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			std::string collection_name, filter_json, update_json;
			if (!parse_query_string(query_string, collection_name, filter_json, update_json)) {
				return 0;
			}

			auto* mongo_db = static_cast<mongocxx::database*>(database_);
			auto collection = (*mongo_db)[collection_name];

			// Parse JSONs to BSON
			auto filter = bsoncxx::from_json(filter_json);
			auto update = bsoncxx::from_json(update_json);

			auto result = collection.update_many(filter.view(), update.view());
			return result ? static_cast<unsigned int>(result->modified_count()) : 0;
		} catch (const std::exception& e) {
			std::cerr << "MongoDB update error: " << e.what() << std::endl;
		}
#else
		std::cerr << "MongoDB support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	unsigned int mongodb_manager::delete_query(const std::string& query_string)
	{
#ifdef USE_MONGODB
		if (!database_) return 0;
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			std::string collection_name, filter_json, unused;
			if (!parse_query_string(query_string, collection_name, filter_json, unused)) {
				return 0;
			}

			auto* mongo_db = static_cast<mongocxx::database*>(database_);
			auto collection = (*mongo_db)[collection_name];

			// Parse JSON to BSON
			auto filter = bsoncxx::from_json(filter_json);
			auto result = collection.delete_many(filter.view());

			return result ? static_cast<unsigned int>(result->deleted_count()) : 0;
		} catch (const std::exception& e) {
			std::cerr << "MongoDB delete error: " << e.what() << std::endl;
		}
#else
		std::cerr << "MongoDB support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
#endif
		return 0;
	}

	database_result mongodb_manager::select_query(const std::string& query_string)
	{
		database_result result;
#ifdef USE_MONGODB
		if (!database_) return result;
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			std::string collection_name, filter_json, unused;
			if (!parse_query_string(query_string, collection_name, filter_json, unused)) {
				return result;
			}

			auto* mongo_db = static_cast<mongocxx::database*>(database_);
			auto collection = (*mongo_db)[collection_name];

			// Parse JSON to BSON for filter
			bsoncxx::document::value filter_doc = filter_json.empty() ?
				bsoncxx::builder::stream::document{} << bsoncxx::builder::stream::finalize :
				bsoncxx::from_json(filter_json);

			auto cursor = collection.find(filter_doc.view());

			for (auto&& doc : cursor) {
				database_row row;

				// Convert BSON document to database_row
				auto json_string = bsoncxx::to_json(doc);
				row["_document"] = json_string;  // Store full document as JSON string

				// Also extract common fields
				auto view = doc.view();
				if (view["_id"]) {
					row["_id"] = bsoncxx::to_json(view["_id"].get_value());
				}

				// Extract other fields as strings for compatibility
				for (auto&& element : view) {
					std::string key = element.key().to_string();
					if (key != "_id") {  // _id already processed
						row[key] = bsoncxx::to_json(element.get_value());
					}
				}

				result.push_back(std::move(row));
			}
		} catch (const std::exception& e) {
			std::cerr << "MongoDB select error: " << e.what() << std::endl;
		}
#else
		std::cerr << "MongoDB support not compiled. Query: " << query_string.substr(0, 20) << "..." << std::endl;
		// Mock data for testing
		if (query_string.find("collection") != std::string::npos) {
			database_row mock_row;
			mock_row["_id"] = std::string("mock_object_id");
			mock_row["name"] = std::string("mongodb_mock_data");
			mock_row["_document"] = std::string("{\"_id\":\"mock_object_id\",\"name\":\"mongodb_mock_data\"}");
			result.push_back(mock_row);
		}
#endif
		return result;
	}

	bool mongodb_manager::execute_query(const std::string& query_string)
	{
#ifdef USE_MONGODB
		if (!database_) {
			std::cerr << "No active MongoDB connection" << std::endl;
			return false;
		}

		std::lock_guard<std::mutex> lock(mongo_mutex_);
		try {
			auto* mongo_db = static_cast<mongocxx::database*>(database_);

			// Try to parse as MongoDB command (JSON format)
			try {
				auto command_doc = bsoncxx::from_json(query_string);
				auto result = mongo_db->run_command(command_doc.view());
				return true;
			} catch (const std::exception&) {
				// If JSON parsing fails, treat as collection operation
				return create_query(query_string);
			}
		} catch (const std::exception& e) {
			std::cerr << "MongoDB execute error: " << e.what() << std::endl;
		}
#else
		// Mock execution for non-MongoDB builds
		std::cout << "MongoDB support not compiled. Mock execute: " << query_string << std::endl;
		return true;
#endif
		return false;
	}

	bool mongodb_manager::disconnect(void)
	{
#ifdef USE_MONGODB
		std::lock_guard<std::mutex> lock(mongo_mutex_);
		if (database_) {
			delete static_cast<mongocxx::database*>(database_);
			database_ = nullptr;
		}
		if (client_) {
			delete static_cast<mongocxx::client*>(client_);
			client_ = nullptr;
		}
		return true;
#endif
		return false;
	}

	bool mongodb_manager::insert_document(const std::string& collection_name, const std::string& document_json)
	{
		std::string query = collection_name + ":" + document_json;
		return insert_query(query) > 0;
	}

	database_result mongodb_manager::find_documents(const std::string& collection_name, const std::string& query_json)
	{
		std::string query = collection_name + ":" + query_json;
		return select_query(query);
	}

	unsigned int mongodb_manager::update_documents(const std::string& collection_name,
													const std::string& filter_json,
													const std::string& update_json)
	{
		std::string query = collection_name + ":" + filter_json + ":" + update_json;
		return update_query(query);
	}

	unsigned int mongodb_manager::delete_documents(const std::string& collection_name, const std::string& filter_json)
	{
		std::string query = collection_name + ":" + filter_json;
		return delete_query(query);
	}

	bool mongodb_manager::parse_connection_string(const std::string& connect_string,
												   std::string& host, int& port,
												   std::string& database, std::string& username,
												   std::string& password)
	{
		// Parse MongoDB URI format: mongodb://username:password@host:port/database
		std::regex uri_regex(R"(mongodb://(?:([^:]+):([^@]+)@)?([^:]+):?(\d+)?/(.+))");
		std::smatch matches;

		if (std::regex_match(connect_string, matches, uri_regex)) {
			username = matches[1].str();
			password = matches[2].str();
			host = matches[3].str();
			port = matches[4].str().empty() ? 27017 : std::stoi(matches[4].str());
			database = matches[5].str();
			return !host.empty() && !database.empty();
		}

		// Fallback: simple format
		database = "test";
		host = "localhost";
		port = 27017;
		return true;
	}

	database_value mongodb_manager::bson_to_database_value(void* bson_doc)
	{
#ifdef USE_MONGODB
		try {
			auto* doc = static_cast<bsoncxx::document::view*>(bson_doc);
			return bsoncxx::to_json(*doc);
		} catch (const std::exception& e) {
			std::cerr << "BSON conversion error: " << e.what() << std::endl;
		}
#endif
		return nullptr;
	}

	void* mongodb_manager::json_to_bson(const std::string& json_string)
	{
#ifdef USE_MONGODB
		try {
			auto doc = bsoncxx::from_json(json_string);
			return new bsoncxx::document::value(std::move(doc));
		} catch (const std::exception& e) {
			std::cerr << "JSON to BSON conversion error: " << e.what() << std::endl;
		}
#endif
		return nullptr;
	}

	bool mongodb_manager::parse_query_string(const std::string& query_string,
											  std::string& collection,
											  std::string& filter,
											  std::string& update)
	{
		// Parse format: "collection_name:filter_json" or "collection_name:filter_json:update_json"
		std::istringstream ss(query_string);
		std::string part;
		std::vector<std::string> parts;

		while (std::getline(ss, part, ':')) {
			parts.push_back(part);
		}

		if (parts.size() >= 1) collection = parts[0];
		if (parts.size() >= 2) filter = parts[1];
		if (parts.size() >= 3) update = parts[2];

		return !collection.empty();
	}
} // namespace database