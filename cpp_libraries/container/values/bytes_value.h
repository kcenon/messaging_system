#pragma once

#include "../value.h"

using namespace std;

namespace container
{
	class bytes_value : public value
	{
	public:
		bytes_value(void);
		bytes_value(const wstring& name, const vector<unsigned char>& data);
		bytes_value(const wstring& name, const unsigned char* data, const size_t& size);
		~bytes_value(void);

	public:
		wstring to_string(const bool& original = true) const override;
	};
}
