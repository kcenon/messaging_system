#pragma once

#include <string>

namespace container
{
	enum class value_types
	{
		null_value,
		bool_value,
		short_value,
		ushort_value,
		int_value,
		uint_value,
		long_value,
		ulong_value,
		llong_value,
		ullong_value,
		float_value,
		double_value,
		bytes_value,
		string_value,
		container_value
	};

	const value_types convert_value_type(const std::wstring& target);
	const std::wstring convert_value_type(const value_types& target);
}