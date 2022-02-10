#pragma once

#include "../value.h"

using namespace std;

namespace container
{
	class string_value : public value
	{
	public:
		string_value(void);
		string_value(const wstring& name, const wstring& value);
		~string_value(void);

	public:
		wstring to_string(const bool& original = true) const override;
	};
}
