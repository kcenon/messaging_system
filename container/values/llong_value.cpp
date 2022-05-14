#include "llong_value.h"

#ifdef __USE_TYPE_CONTAINER__

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	llong_value::llong_value(void)
		: value()
	{
		_type = value_types::llong_value;
	}

	llong_value::llong_value(const wstring& name, const long long& value)
		: value(name, (const unsigned char*)&value, sizeof(long long), value_types::llong_value)
	{
	}

	llong_value::~llong_value(void)
	{
	}

	short llong_value::to_short(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short llong_value::to_ushort(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int llong_value::to_int(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int llong_value::to_uint(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long llong_value::to_long(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long llong_value::to_ulong(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long llong_value::to_llong(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long llong_value::to_ullong(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float llong_value::to_float(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double llong_value::to_double(void) const
	{
		long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring llong_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_llong());
	}
}

#endif