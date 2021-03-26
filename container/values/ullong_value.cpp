#include "ullong_value.h"

#include "fmt/format.h"

namespace container
{
	ullong_value::ullong_value(void)
		: value()
	{
		_type = value_types::ullong_value;
	}

	ullong_value::ullong_value(const std::wstring& name, const unsigned long long& value)
		: value(name, (const unsigned char*)&value, sizeof(unsigned long long), value_types::ullong_value)
	{
	}

	ullong_value::~ullong_value(void)
	{
	}

	short ullong_value::to_short(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short ullong_value::to_ushort(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int ullong_value::to_int(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int ullong_value::to_uint(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long ullong_value::to_long(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long ullong_value::to_ulong(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long ullong_value::to_llong(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long ullong_value::to_ullong(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float ullong_value::to_float(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double ullong_value::to_double(void) const
	{
		unsigned long long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	std::wstring ullong_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_ullong());
	}
}