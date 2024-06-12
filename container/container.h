/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
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

#include "value.h"

#include <memory>
#include <vector>

namespace container
{
	class value_container : public std::enable_shared_from_this<value_container>
	{
	public:
		value_container(void);
		value_container(const std::string& data_string,
						const bool& parse_only_header = true);
		value_container(const std::vector<uint8_t>& data_array,
						const bool& parse_only_header = true);
		value_container(const value_container& data_container,
						const bool& parse_only_header = true);
		value_container(std::shared_ptr<value_container> data_container,
						const bool& parse_only_header = true);
		value_container(const std::string& message_type,
						const std::vector<std::shared_ptr<value>>& units);
		value_container(const std::string& target_id,
						const std::string& target_sub_id,
						const std::string& message_type,
						const std::vector<std::shared_ptr<value>>& units);
		value_container(const std::string& source_id,
						const std::string& source_sub_id,
						const std::string& target_id,
						const std::string& target_sub_id,
						const std::string& message_type,
						const std::vector<std::shared_ptr<value>>& units);
		virtual ~value_container(void);

	public:
		std::shared_ptr<value_container> get_ptr(void);

	public:
		void set_source(const std::string& source_id,
						const std::string& source_sub_id);
		void set_target(const std::string& target_id,
						const std::string& target_sub_id = "");
		void set_message_type(const std::string& message_type);
		void set_units(const std::vector<std::shared_ptr<value>>& target_values,
					   const bool& update_immediately = false);

	public:
		void swap_header(void);
		void clear_value(void);
		std::shared_ptr<value_container> copy(const bool& containing_values
											  = true);

	public:
		std::string source_id(void) const;
		std::string source_sub_id(void) const;
		std::string target_id(void) const;
		std::string target_sub_id(void) const;
		std::string message_type(void) const;

	public:
		std::shared_ptr<value> add(const value& target_value,
								   const bool& update_immediately = false);
		std::shared_ptr<value> add(std::shared_ptr<value> target_value,
								   const bool& update_immediately = false);
		void remove(const std::string& target_name,
					const bool& update_immediately = false);
		void remove(std::shared_ptr<value> target_value,
					const bool& update_immediately = false);
		std::vector<std::shared_ptr<value>> value_array(
			const std::string& target_name);
		std::shared_ptr<value> get_value(const std::string& target_name,
										 const unsigned int& index = 0);

	public:
		void initialize(void);

	public:
		std::string serialize(void) const;
		std::vector<uint8_t> serialize_array(void) const;
		bool deserialize(const std::string& data_string,
						 const bool& parse_only_header = true);
		bool deserialize(const std::vector<uint8_t>& data_array,
						 const bool& parse_only_header = true);

	public:
		const std::string to_xml(void);
		const std::string to_json(void);

	public:
		std::string datas(void) const;

	public:
		void load_packet(const std::string& file_path);
		void save_packet(const std::string& file_path);

	public:
		std::vector<std::shared_ptr<value>> operator[](const std::string& key);

		friend value_container operator<<(value_container target_container,
										  value& other);
		friend value_container operator<<(value_container target_container,
										  std::shared_ptr<value> other);
		friend std::shared_ptr<value_container> operator<<(
			std::shared_ptr<value_container> target_container, value& other);
		friend std::shared_ptr<value_container> operator<<(
			std::shared_ptr<value_container> target_container,
			std::shared_ptr<value> other);

		friend std::ostream& operator<<(std::ostream& out,
										value_container& other);
		friend std::ostream& operator<<(std::ostream& out,
										std::shared_ptr<value_container> other);

		friend std::string& operator<<(std::string& out,
									   value_container& other);
		friend std::string& operator<<(std::string& out,
									   std::shared_ptr<value_container> other);

	protected:
		bool deserialize_values(const std::string& data,
								const bool& parse_only_header = true);
		void parsing(const std::string& source_name,
					 const std::string& target_name,
					 const std::string& target_value,
					 std::string& target_variable);

	private:
		std::shared_ptr<value> set_boolean(const std::string& name,
										   const std::string& data);
		std::shared_ptr<value> set_short(const std::string& name,
										 const std::string& data);
		std::shared_ptr<value> set_ushort(const std::string& name,
										  const std::string& data);
		std::shared_ptr<value> set_int(const std::string& name,
									   const std::string& data);
		std::shared_ptr<value> set_uint(const std::string& name,
										const std::string& data);
		std::shared_ptr<value> set_long(const std::string& name,
										const std::string& data);
		std::shared_ptr<value> set_ulong(const std::string& name,
										 const std::string& data);
		std::shared_ptr<value> set_llong(const std::string& name,
										 const std::string& data);
		std::shared_ptr<value> set_ullong(const std::string& name,
										  const std::string& data);
		std::shared_ptr<value> set_float(const std::string& name,
										 const std::string& data);
		std::shared_ptr<value> set_double(const std::string& name,
										  const std::string& data);
		std::shared_ptr<value> set_bytes(const std::string& name,
										 const std::string& data);
		std::shared_ptr<value> set_string(const std::string& name,
										  const std::string& data);
		std::shared_ptr<value> set_container(const std::string& name,
											 const std::string& data);

	private:
		bool parsed_data_;
		bool changed_data_;
		std::string data_string_;

	private:
		std::string source_id_;
		std::string source_sub_id_;
		std::string target_id_;
		std::string target_sub_id_;
		std::string message_type_;
		std::string version_;
		std::vector<std::shared_ptr<value>> units_;

	private:
		std::map<value_types,
				 std::function<std::shared_ptr<value>(const std::string&,
													  const std::string&)>>
			data_type_map_;
	};
} // namespace container