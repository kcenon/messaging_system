#include "container_value.h"

#ifdef __USE_TYPE_CONTAINER__

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <algorithm>

namespace container
{
	container_value::container_value(void)
		: value()
	{
		_type = value_types::container_value;
	}

	container_value::container_value(const wstring& name, const long& reserved_count)
		: container_value()
	{
		_name = name;
		_type = value_types::container_value;
		_size = sizeof(long);

		char* data_ptr = (char*)&reserved_count;
		_data = vector<unsigned char>(data_ptr, data_ptr + _size);
	}

	container_value::container_value(const wstring& name, const vector<shared_ptr<value>>& units)
		: value(name, units)
	{
	}

	container_value::~container_value(void)
	{
		_data.clear();
		_units.clear();
	}

	shared_ptr<value> container_value::add(const value& item, const bool& update_count)
	{
		return add(value::generate_value(item.name(), convert_value_type(item.type()), item.to_string()), update_count);
	}

	shared_ptr<value> container_value::add(shared_ptr<value> item, const bool& update_count)
	{
		vector<shared_ptr<value>>::iterator target;
		target = find_if(_units.begin(), _units.end(),
			[&item](shared_ptr<value> current)
			{
				return current == item;
			});

		if (target != _units.end())
		{
			return nullptr;
		}

		_units.push_back(item);
		item->set_parent(get_ptr());

		if (update_count == true)
		{
			long size = static_cast<long>(_units.size());
			set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
		}

		return item;
	}

	void container_value::add(const vector<value>& target_values, const bool& update_count)
	{
		vector<shared_ptr<value>> temp_values;
		for (auto& target_value : target_values)
		{
			temp_values.push_back(value::generate_value(target_value.name(), convert_value_type(target_value.type()), target_value.to_string()));
		}

		add(temp_values, update_count);
	}

	void container_value::add(const vector<shared_ptr<value>>& target_values, const bool& update_count)
	{
		vector<shared_ptr<value>>::iterator target;
		for (auto& target_value : target_values)
		{
			target = find_if(_units.begin(), _units.end(),
				[&target_value](shared_ptr<value> item)
				{
					return item == target_value;
				});

			if (target != _units.end())
			{
				continue;
			}

			_units.push_back(target_value);
			target_value->set_parent(get_ptr());
		}

		if (update_count == true)
		{
			long size = static_cast<long>(_units.size());
			set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
		}
	}

	void container_value::remove(const wstring& target_name, const bool& update_count)
	{
		vector<shared_ptr<value>>::iterator target;

		while (true) {
			target = find_if(_units.begin(), _units.end(),
				[&target_name](shared_ptr<value> current)
				{
					return current->name() == target_name;
				});

			if (target == _units.end())
			{
				break;
			}

			_units.erase(target);
		}

		if (update_count == true)
		{
			long size = static_cast<long>(_units.size());
			set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
		}
	}

	void container_value::remove(shared_ptr<value> item, const bool& update_count)
	{
		vector<shared_ptr<value>>::iterator target;
		target = find_if(_units.begin(), _units.end(),
			[&item](shared_ptr<value> current)
			{
				return current == item;
			});

		if (target == _units.end())
		{
			return;
		}

		_units.erase(target);

		if (update_count == true)
		{
			long size = static_cast<long>(_units.size());
			set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
		}
	}

	void container_value::remove_all(void)
	{
		_units.clear();

		long size = static_cast<long>(_units.size());
		set_data((const unsigned char*)&size, sizeof(long), value_types::container_value);
	}

	short container_value::to_short(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<short>(temp);
	}

	unsigned short container_value::to_ushort(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned short>(temp);
	}

	int container_value::to_int(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<int>(temp);
	}

	unsigned int container_value::to_uint(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned int>(temp);
	}

	long container_value::to_long(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long>(temp);
	}

	unsigned long container_value::to_ulong(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long>(temp);
	}

	long long container_value::to_llong(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<long long>(temp);
	}

	unsigned long long container_value::to_ullong(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<unsigned long long>(temp);
	}

	float container_value::to_float(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<float>(temp);
	}

	double container_value::to_double(void) const
	{
		long temp = 0;
		memcpy(&temp, _data.data(), _size);

		return static_cast<double>(temp);
	}

	wstring container_value::to_string(const bool&) const
	{
		return fmt::format(L"{}", to_long());
	}
}

#endif