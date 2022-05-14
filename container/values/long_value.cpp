#include "long_value.h"

#ifdef __USE_TYPE_CONTAINER__

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace container
{
	long_value::long_value(void)
		: value()
	{
		_type = value_types::long_value;
	}

	long_value::long_value(const wstring& name, const long& value)
		: value(name, (const unsigned char*)&value, sizeof(long), value_types::long_value)
	{
	}

	long_value::~long_value(void)
	{
	}

	short long_value::to_short(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short long_value::to_ushort(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int long_value::to_int(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int long_value::to_uint(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long long_value::to_long(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long long_value::to_ulong(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long long_value::to_llong(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long long_value::to_ullong(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float long_value::to_float(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double long_value::to_double(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring long_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_long());
	}
}

#endif