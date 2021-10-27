#include "uint_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	uint_value::uint_value(void)
		: value()
	{
		_type = value_types::uint_value;
	}

	uint_value::uint_value(const std::wstring& name, const unsigned int& value)
		: value(name, (const unsigned char*)&value, sizeof(unsigned int), value_types::uint_value)
	{
	}

	uint_value::~uint_value(void)
	{
	}

	short uint_value::to_short(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short uint_value::to_ushort(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int uint_value::to_int(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int uint_value::to_uint(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long uint_value::to_long(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long uint_value::to_ulong(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long uint_value::to_llong(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long uint_value::to_ullong(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float uint_value::to_float(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double uint_value::to_double(void) const
	{
		unsigned int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	std::wstring uint_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_uint());
	}
}