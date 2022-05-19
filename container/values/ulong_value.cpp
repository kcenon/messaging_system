#include "ulong_value.h"

#ifdef __USE_TYPE_CONTAINER__

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	ulong_value::ulong_value(void)
		: value()
	{
		_type = value_types::ulong_value;
	}

	ulong_value::ulong_value(const wstring& name, const unsigned long& value)
		: ulong_value()
	{
		_name = name;
		set_data((const unsigned char*)&value, sizeof(unsigned long), value_types::ulong_value);
	}

	ulong_value::~ulong_value(void)
	{
	}

	short ulong_value::to_short(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short ulong_value::to_ushort(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int ulong_value::to_int(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int ulong_value::to_uint(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long ulong_value::to_long(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long ulong_value::to_ulong(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long ulong_value::to_llong(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long ulong_value::to_ullong(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float ulong_value::to_float(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double ulong_value::to_double(void) const
	{
		unsigned long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring ulong_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_ulong());
	}
}

#endif