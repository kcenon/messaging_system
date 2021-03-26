#pragma once

#include "../value.h"

namespace container
{
	class bytes_value : public value
	{
	public:
		bytes_value(void);
		bytes_value(const std::wstring& name, const std::vector<unsigned char>& data);
		bytes_value(const std::wstring& name, const unsigned char* data, const size_t& size);
		~bytes_value(void);

	public:
		std::wstring to_string(const bool& original = true) const override;
	};
}
