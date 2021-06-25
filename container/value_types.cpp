#include "value_types.h"

#include <map>

namespace container
{
	std::map<std::wstring, value_types> value_type_map =
	{
		{ L"bool", value_types::bool_value },
		{ L"short", value_types::short_value },
		{ L"ushort", value_types::ushort_value },
		{ L"int", value_types::int_value },
		{ L"uint", value_types::uint_value },
		{ L"long", value_types::long_value },
		{ L"ulong", value_types::ulong_value },
		{ L"llong", value_types::llong_value },
		{ L"ullong", value_types::ullong_value },
		{ L"float", value_types::float_value },
		{ L"double", value_types::double_value },
		{ L"bytes", value_types::bytes_value },
		{ L"string", value_types::string_value },
		{ L"container", value_types::container_value }
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
		case value_types::bool_value: result = L"bool"; break;
		case value_types::short_value: result = L"short"; break;
		case value_types::ushort_value: result = L"ushort"; break;
		case value_types::int_value: result = L"int"; break;
		case value_types::uint_value: result = L"uint"; break;
		case value_types::long_value: result = L"long"; break;
		case value_types::ulong_value: result = L"ulong"; break;
		case value_types::llong_value: result = L"llong"; break;
		case value_types::ullong_value: result = L"ullong"; break;
		case value_types::float_value: result = L"float"; break;
		case value_types::double_value: result = L"double"; break;
		case value_types::bytes_value: result = L"bytes"; break;
		case value_types::string_value: result = L"string"; break;
		case value_types::container_value: result = L"container"; break;
		default: result = L"null"; break;
		}

		return result;
	}
}