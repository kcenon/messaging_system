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

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <stdexcept>
#include <ostream>

#include "value_types.h"

namespace container
{
	/**
	 * @brief The base class for all values stored in the container system.
	 */
	class value : public std::enable_shared_from_this<value>
	{
	public:
		value();
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
		virtual ~value();

		std::shared_ptr<value> get_ptr();

		void set_parent(std::shared_ptr<value> parent);

		/**
		 * @brief Set raw data with a known type.
		 * @param data Raw pointer.
		 * @param size Byte size.
		 * @param type The value_types enum to apply.
		 */
		void set_data(const unsigned char* data,
					  const size_t& size,
					  const value_types& type);

		/**
		 * @brief Set this value from a string form, specifying a type.
		 * @param name Value name.
		 * @param type The enum type.
		 * @param data The string to parse and store.
		 */
		void set_data(const std::string& name,
					  const value_types& type,
					  const std::string& data);

		std::string name() const;
		value_types type() const;
		std::string data() const;
		size_t size() const;
		std::shared_ptr<value> parent();
		size_t child_count() const;
		std::vector<std::shared_ptr<value>> children(const bool& only_container
													 = false);
		std::vector<std::shared_ptr<value>> value_array(const std::string& key);

		const std::vector<uint8_t> to_bytes() const;

		bool is_null() const;
		bool is_bytes() const;
		bool is_boolean() const;
		bool is_numeric() const;
		bool is_string() const;
		bool is_container() const;

		const std::string to_xml();
		const std::string to_json();
		const std::string serialize();

		/**
		 * @brief Virtual numeric/string conversion methods.
		 */
		virtual bool to_boolean() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to boolean.");
			return false;
		}
		virtual short to_short() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to short.");
			return 0;
		}
		virtual unsigned short to_ushort() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to ushort.");
			return 0;
		}
		virtual int to_int() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to int.");
			return 0;
		}
		virtual unsigned int to_uint() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to uint.");
			return 0;
		}
		virtual long to_long() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to long.");
			return 0;
		}
		virtual unsigned long to_ulong() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to ulong.");
			return 0;
		}
		virtual long long to_llong() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to llong.");
			return 0;
		}
		virtual unsigned long long to_ullong() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to ullong.");
			return 0;
		}
		virtual float to_float() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to float.");
			return 0.0f;
		}
		virtual double to_double() const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to double.");
			return 0.0;
		}
		virtual std::string to_string(const bool& original = true) const
		{
			if (type_ == value_types::null_value)
				throw std::logic_error("Cannot convert null_value to string.");
			return "";
		}

		/**
		 * @brief Virtual methods for adding/removing child values. By default,
		 * these throw logic_error.
		 */
		virtual std::shared_ptr<value> add(const value& item,
										   bool update_count = true)
		{
			throw std::logic_error("Cannot add on this base value.");
		}
		virtual std::shared_ptr<value> add(std::shared_ptr<value> item,
										   bool update_count = true)
		{
			throw std::logic_error("Cannot add on this base value.");
		}
		virtual void add(const std::vector<value>& items,
						 bool update_count = true)
		{
			throw std::logic_error("Cannot add multiple on this base value.");
		}
		virtual void add(const std::vector<std::shared_ptr<value>>& items,
						 bool update_count = true)
		{
			throw std::logic_error("Cannot add multiple on this base value.");
		}
		virtual void remove(const std::string& name, bool update_count = true)
		{
			throw std::logic_error("Cannot remove from this base value.");
		}
		virtual void remove(std::shared_ptr<value> item,
							bool update_count = true)
		{
			throw std::logic_error("Cannot remove from this base value.");
		}
		virtual void remove_all()
		{
			throw std::logic_error("Cannot remove all from this base value.");
		}

		/**
		 * @brief An index operator to get the first matching child by name (if
		 * any).
		 */
		std::shared_ptr<value> operator[](const std::string& key);

		friend std::shared_ptr<value> operator<<(
			std::shared_ptr<value> container, std::shared_ptr<value> other);

		friend std::ostream& operator<<(std::ostream& out,
										std::shared_ptr<value> other);
		friend std::string& operator<<(std::string& out,
									   std::shared_ptr<value> other);

	protected:
		template <typename T> void set_data(T data)
		{
			char* data_ptr = reinterpret_cast<char*>(&data);
			size_ = sizeof(T);
			data_ = std::vector<uint8_t>(data_ptr, data_ptr + size_);
		}

		std::string convert_specific_string(
			const std::vector<uint8_t>& data) const;
		std::vector<uint8_t> convert_specific_string(std::string data) const;

		void set_byte_string(const std::string& data);
		void set_string(const std::string& data);
		void set_boolean(const std::string& data);
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
		std::weak_ptr<value> parent_;
		std::vector<std::shared_ptr<value>> units_;

		std::map<value_types, std::function<void(const std::string&)>>
			data_type_map_;
	};
} // namespace container
