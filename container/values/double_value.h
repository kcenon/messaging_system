#pragma once

#ifdef __USE_TYPE_CONTAINER__

#include "../value.h"

using namespace std;

namespace container
{
	class double_value : public value
	{
	public:
		double_value(void);
		double_value(const wstring& name, const double& value);
		~double_value(void);

	public:
		short to_short(void) const override;
		unsigned short to_ushort(void) const override;
		int to_int(void) const override;
		unsigned int to_uint(void) const override;
		long to_long(void) const override;
		unsigned long to_ulong(void) const override;
		long long to_llong(void) const override;
		unsigned long long to_ullong(void) const override;
		float to_float(void) const override;
		double to_double(void) const override;
		wstring to_string(const bool& original = true) const override;
	};
}

#endif