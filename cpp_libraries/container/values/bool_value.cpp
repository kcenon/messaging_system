#include "bool_value.h"

#nclude <cstring>

ifdef __USE_TYPE_CONTAINER__

namespace container
{
	bool_value::bool_value(void)
		: value()
	{
		_type = value_types::bool_value;
	}

	bool_value::bool_value(const wstring& name, const bool& value)
		: value(name, value_types::bool_value, value ? L"true" : L"false")
	{
	}

	bool_value::bool_value(const wstring& name, const wstring& value)
		: value(name, value_types::bool_value, value)
	{
	}

	bool_value::~bool_value(void)
	{
	}

	bool bool_value::to_boolean(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return temp;
	}

	short bool_value::to_short(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short bool_value::to_ushort(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int bool_value::to_int(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int bool_value::to_uint(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long bool_value::to_long(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long bool_value::to_ulong(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long bool_value::to_llong(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long bool_value::to_ullong(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float bool_value::to_float(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double bool_value::to_double(void) const
	{
		bool temp = false;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring bool_value::to_string(const bool&) const
	{
		return (to_boolean() ? L"true" : L"false");
	}
}

#endif
