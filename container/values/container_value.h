#pragma once

#include "../value.h"

namespace container
{
	class container_value : public value
	{
	public:
		container_value(void);
		container_value(const std::wstring& name, const long& reserved_count);
		container_value(const std::wstring& name, const std::vector<std::shared_ptr<value>>& units = {});
		~container_value(void);

	public:
		std::shared_ptr<value> add(std::shared_ptr<value> item, const bool& update_count = true) override;
		void add(const std::vector<std::shared_ptr<value>>& target_values, const bool& update_count = true) override;
		void remove(const std::wstring& target_name, const bool& update_count = true) override;
		void remove(std::shared_ptr<value> item, const bool& update_count = true) override;
		void remove_all(void) override;

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
		std::wstring to_string(const bool& original = true) const override;
	};
}
