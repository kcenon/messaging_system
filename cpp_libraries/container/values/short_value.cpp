#include "short_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	short_value::short_value(void)
		: value()
	{
		_type = value_types::short_value;
	}

	short_value::short_value(const wstring& name, const short& value)
		: value(name, (const unsigned char*)&value, sizeof(short), value_types::short_value)
	{
	}

	short_value::~short_value(void)
	{
	}

	short short_value::to_short(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short short_value::to_ushort(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int short_value::to_int(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int short_value::to_uint(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long short_value::to_long(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long short_value::to_ulong(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long short_value::to_llong(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long short_value::to_ullong(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float short_value::to_float(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double short_value::to_double(void) const
	{
		short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring short_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_short());
	}
}