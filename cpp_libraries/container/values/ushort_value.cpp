#include "ushort_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	ushort_value::ushort_value(void)
		: value()
	{
		_type = value_types::ushort_value;
	}

	ushort_value::ushort_value(const std::wstring& name, const unsigned short& value)
		: value(name, (const unsigned char*)&value, sizeof(unsigned short), value_types::ushort_value)
	{
	}

	ushort_value::~ushort_value(void)
	{
	}

	short ushort_value::to_short(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short ushort_value::to_ushort(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int ushort_value::to_int(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int ushort_value::to_uint(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long ushort_value::to_long(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long ushort_value::to_ulong(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long ushort_value::to_llong(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long ushort_value::to_ullong(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float ushort_value::to_float(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double ushort_value::to_double(void) const
	{
		unsigned short temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	std::wstring ushort_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_ushort());
	}
}