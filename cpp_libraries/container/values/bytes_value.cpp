#include "bytes_value.h"

#ifdef __USE_TYPE_CONTAINER__

#include "converting.h"

namespace container
{
	using namespace converting;

	bytes_value::bytes_value(void)
		: value()
	{
		_type = value_types::bytes_value;
	}

	bytes_value::bytes_value(const wstring& name, const vector<unsigned char>& data)
		: value(name, data.data(), data.size(), value_types::bytes_value)
	{
	}

	bytes_value::bytes_value(const wstring& name, const unsigned char* data, const size_t& size)
		: value(name, data, size, value_types::bytes_value)
	{
	}

	bytes_value::~bytes_value(void)
	{
	}

	wstring bytes_value::to_string(const bool&) const
	{
		return converter::to_base64(_data);
	}
}

#endif