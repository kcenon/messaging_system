#include "double_value.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	double_value::double_value(void)
		: value()
	{
		_type = value_types::double_value;
	}

	double_value::double_value(const wstring& name, const double& value)
		: value(name, (const unsigned char*)&value, sizeof(double), value_types::double_value)
	{
	}

	double_value::~double_value(void)
	{
	}

	short double_value::to_short(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short double_value::to_ushort(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int double_value::to_int(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int double_value::to_uint(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long double_value::to_long(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long double_value::to_ulong(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long double_value::to_llong(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long double_value::to_ullong(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float double_value::to_float(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double double_value::to_double(void) const
	{
		double temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring double_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_double());
	}
}