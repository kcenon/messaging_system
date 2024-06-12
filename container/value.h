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

#include "value_types.h"

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace container
{
	class value : public std::enable_shared_from_this<value>
	{
	public:
		value(void);
		value(std::shared_ptr<value> object);
		value(const std::string& name,
			  const std::vector<std::shared_ptr<value>>& units = {});
		value(const std::string& name,
			  const value_types& type,
			  const std::string& data);
		value(const std::string& name,
			  const unsigned char* data,
			  const size_t& size,
			  const value_types& type = value_types::null_value);
		virtual ~value(void);

	public:
		std::shared_ptr<value> get_ptr(void);

	public:
		void set_parent(std::shared_ptr<value> parent);
		void set_data(const unsigned char* data,
					  const size_t& size,
					  const value_types& type);
		void set_data(const std::string& name,
					  const value_types& type,
					  const std::string& data);

	public:
		std::string name(void) const;
		value_types type(void) const;
		std::string data(void) const;
		size_t size(void) const;
		std::shared_ptr<value> parent(void);
		size_t child_count(void) const;
		std::vector<std::shared_ptr<value>> children(const bool& only_container
													 = false);

	public:
		virtual std::shared_ptr<value> add(const value& item,
										   const bool& update_count = true)
		{
			throw std::exception(
				std::logic_error("cannot add value object on this object"));
		}
		virtual std::shared_ptr<value> add(std::shared_ptr<value> item,
										   const bool& update_count = true)
		{
			throw std::exception(
				std::logic_error("cannot add value object on this object"));
		}
		virtual void add(const std::vector<value>& target_values,
						 const bool& update_count = true)
		{
			throw std::exception(
				std::logic_error("cannot add value objects on this object"));
		}
		virtual void add(
			const std::vector<std::shared_ptr<value>>& target_values,
			const bool& update_count = true)
		{
			throw std::exception(
				std::logic_error("cannot add value objects on this object"));
		}
		virtual void remove(const std::string& target_name,
							const bool& update_count = true)
		{
			throw std::exception(
				std::logic_error("cannot remove value objects on this object"));
		}
		virtual void remove(std::shared_ptr<value> item,
							const bool& update_count = true)
		{
			throw std::exception(
				std::logic_error("cannot remove value object on this object"));
		}
		virtual void remove_all(void)
		{
			throw std::exception(
				std::logic_error("cannot remove value object on this object"));
		}
		std::vector<std::shared_ptr<value>> value_array(const std::string& key);

	public:
		const std::vector<uint8_t> to_bytes(void) const;

	public:
		bool is_null(void) const;
		bool is_bytes(void) const;
		bool is_boolean(void) const;
		bool is_numeric(void) const;
		bool is_string(void) const;
		bool is_container(void) const;

	public:
		const std::string to_xml(void);
		const std::string to_json(void);

	public:
		const std::string serialize(void);

	public:
		virtual bool to_boolean(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return false;
		}
		virtual short to_short(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual unsigned short to_ushort(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual int to_int(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual unsigned int to_uint(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual long to_long(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual unsigned long to_ulong(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual long long to_llong(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual unsigned long long to_ullong(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual float to_float(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual double to_double(void) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return 0;
		}
		virtual std::string to_string(const bool& original = true) const
		{
			if (type_ == value_types::null_value)
				throw std::exception(std::logic_error("Not implemented yet!"));
			else
				return "";
		}

	public:
		std::shared_ptr<value> operator[](const std::string& key);

		friend std::shared_ptr<value> operator<<(
			std::shared_ptr<value> container, std::shared_ptr<value> other);

		friend std::ostream& operator<<(std::ostream& out,
										std::shared_ptr<value> other);
		friend std::string& operator<<(std::string& out,
									   std::shared_ptr<value> other);

	protected:
		std::string convert_specific_string(
			const std::vector<uint8_t>& data) const;
		std::vector<uint8_t> convert_specific_string(std::string data) const;

	protected:
		template <typename T> void set_data(T data);
		void set_byte_string(const std::string& data);
		void set_string(const std::string& data);
		void set_boolean(const std::string& data);

	private:
		void set_short(const std::string& data);
		void set_ushort(const std::string& data);
		void set_int(const std::string& data);
		void set_uint(const std::string& data);
		void set_long(const std::string& data);
		void set_ulong(const std::string& data);
		void set_llong(const std::string& data);
		void set_ullong(const std::string& data);
		void set_float(const std::string& data);
		void set_double(const std::string& data);

	protected:
		size_t size_;
		value_types type_;
		std::string name_;
		std::vector<uint8_t> data_;

	protected:
		std::weak_ptr<value> parent_;
		std::vector<std::shared_ptr<value>> units_;

	private:
		std::map<value_types, std::function<void(const std::string&)>>
			data_type_map_;
	};
} // namespace container