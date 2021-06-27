#include "value_types.h"

#include <map>

namespace container
{
	std::map<std::wstring, value_types> value_type_map =
	{
		{ L"1", value_types::bool_value },
		{ L"2", value_types::short_value },
		{ L"3", value_types::ushort_value },
		{ L"4", value_types::int_value },
		{ L"5", value_types::uint_value },
		{ L"6", value_types::long_value },
		{ L"7", value_types::ulong_value },
		{ L"8", value_types::llong_value },
		{ L"9", value_types::ullong_value },
		{ L"a", value_types::float_value },
		{ L"b", value_types::double_value },
		{ L"c", value_types::bytes_value },
		{ L"d", value_types::string_value },
		{ L"e", value_types::container_value }
	};

	const value_types convert_value_type(const std::wstring& target)
	{
		std::map<std::wstring, value_types>::iterator iterator = value_type_map.find(target);
		if (iterator == value_type_map.end())
		{
			return value_types::null_value;
		}

		return iterator->second;
	}

	const std::wstring convert_value_type(const value_types& target)
	{
		std::wstring result;

		switch (target)
		{
		case value_types::bool_value: result = L"1"; break;
		case value_types::short_value: result = L"2"; break;
		case value_types::ushort_value: result = L"3"; break;
		case value_types::int_value: result = L"4"; break;
		case value_types::uint_value: result = L"5"; break;
		case value_types::long_value: result = L"6"; break;
		case value_types::ulong_value: result = L"7"; break;
		case value_types::llong_value: result = L"8"; break;
		case value_types::ullong_value: result = L"9"; break;
		case value_types::float_value: result = L"a"; break;
		case value_types::double_value: result = L"b"; break;
		case value_types::bytes_value: result = L"c"; break;
		case value_types::string_value: result = L"d"; break;
		case value_types::container_value: result = L"e"; break;
		default: result = L"0"; break;
		}

		return result;
	}
}