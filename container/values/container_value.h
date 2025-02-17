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

#include "../value.h"
#include <functional>
#include <map>

namespace container
{
	/**
	 * @class container_value
	 * @brief A specialized value that can hold child values, i.e. a
	 * hierarchical container.
	 */
	class container_value : public value
	{
	public:
		container_value();
		container_value(const std::string& name, long reserved_count);
		container_value(const std::string& name,
						const std::vector<std::shared_ptr<value>>& units = {});
		~container_value() override;

		/**
		 * @brief Override the base add(...) and remove(...) methods.
		 */
		std::shared_ptr<value> add(const value& item,
								   bool update_count = true) override;
		std::shared_ptr<value> add(std::shared_ptr<value> item,
								   bool update_count = true) override;
		void add(const std::vector<value>& target_values,
				 bool update_count = true) override;
		void add(const std::vector<std::shared_ptr<value>>& target_values,
				 bool update_count = true) override;
		void remove(const std::string& target_name,
					bool update_count = true) override;
		void remove(std::shared_ptr<value> item,
					bool update_count = true) override;
		void remove_all() override;

		long to_long() const override;
		std::string to_string(const bool& original = true) const override;

	private:
		std::map<value_types,
				 std::function<std::shared_ptr<value>(const std::string&,
													  const std::string&)>>
			data_type_map_;

		std::shared_ptr<value> set_boolean(const std::string& name,
										   const std::string& dataStr);
		std::shared_ptr<value> set_short(const std::string& name,
										 const std::string& dataStr);
		std::shared_ptr<value> set_ushort(const std::string& name,
										  const std::string& dataStr);
		std::shared_ptr<value> set_int(const std::string& name,
									   const std::string& dataStr);
		std::shared_ptr<value> set_uint(const std::string& name,
										const std::string& dataStr);
		std::shared_ptr<value> set_long(const std::string& name,
										const std::string& dataStr);
		std::shared_ptr<value> set_ulong(const std::string& name,
										 const std::string& dataStr);
		std::shared_ptr<value> set_llong(const std::string& name,
										 const std::string& dataStr);
		std::shared_ptr<value> set_ullong(const std::string& name,
										  const std::string& dataStr);
		std::shared_ptr<value> set_float(const std::string& name,
										 const std::string& dataStr);
		std::shared_ptr<value> set_double(const std::string& name,
										  const std::string& dataStr);
		std::shared_ptr<value> set_byte_string(const std::string& name,
											   const std::string& dataStr);
		std::shared_ptr<value> set_string(const std::string& name,
										  const std::string& dataStr);
		std::shared_ptr<value> set_container(const std::string& name,
											 const std::string& dataStr);
	};
} // namespace container
