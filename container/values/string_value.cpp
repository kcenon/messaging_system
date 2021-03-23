#include "string_value.h"

#include "converting.h"

namespace container
{
	using namespace converting;

	string_value::string_value(void)
		: value()
	{
		_type = value_types::string_value;
	}

	string_value::string_value(const std::wstring& name, const std::wstring& value)
		: value(name, nullptr, 0, value_types::null_value)
	{
		std::wstring temp = value;
		converter::replace_all(temp, L"\r", L"</0x0A;>");
		converter::replace_all(temp, L"\n", L"</0x0B;>");
		converter::replace_all(temp, L" ", L"</0x0C;>");
		converter::replace_all(temp, L"\t", L"</0x0D;>");

		std::vector<char> data = converter::to_array(temp);

		set_data(data.data(), data.size(), value_types::string_value);
	}

	string_value::~string_value(void)
	{
	}

	std::wstring string_value::to_string(const bool& original) const
	{
		if (!original)
		{
			return converter::to_wstring(_data);
		}

		std::wstring temp = converter::to_wstring(_data);
		converter::replace_all(temp, L"</0x0A;>", L"\r");
		converter::replace_all(temp, L"</0x0B;>", L"\n");
		converter::replace_all(temp, L"</0x0C;>", L" ");
		converter::replace_all(temp, L"</0x0D;>", L"\t");

		return temp;
	}
}