#pragma once

#include "../value.h"

namespace container
{
	class string_value : public value
	{
	public:
		string_value(void);
		string_value(const std::wstring& name, const std::wstring& value);
		~string_value(void);

	public:
		std::wstring to_string(const bool& original = true) const override;
	};
}
