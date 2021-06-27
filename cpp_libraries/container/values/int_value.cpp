#include "int_value.h"

#include "fmt/format.h"

namespace container
{
	int_value::int_value(void)
		: value()
	{
		_type = value_types::int_value;
	}

	int_value::int_value(const std::wstring& name, const int& value)
		: value(name, (const unsigned char*)&value, sizeof(int), value_types::int_value)
	{
	}

	int_value::~int_value(void)
	{
	}

	short int_value::to_short(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short int_value::to_ushort(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int int_value::to_int(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int int_value::to_uint(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long int_value::to_long(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long int_value::to_ulong(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long int_value::to_llong(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long int_value::to_ullong(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float int_value::to_float(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double int_value::to_double(void) const
	{
		int temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	std::wstring int_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_int());
	}
}