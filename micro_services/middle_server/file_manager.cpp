#include "file_manager.h"

#include "value.h"
#include "values/bool_value.h"
#include "values/ushort_value.h"
#include "values/string_value.h"

file_manager::file_manager(void)
{

}

file_manager::~file_manager(void)
{

}

bool file_manager::set(const std::wstring& indication_id, const std::vector<std::wstring>& file_list)
{
	auto target = _transferring_list.find(indication_id);
	if (target != _transferring_list.end())
	{
		return false;
	}

	_transferring_list.insert({ indication_id, file_list });
	_transferred_list.insert({ indication_id, std::vector<std::wstring>() });
	_failed_list.insert({ indication_id, std::vector<std::wstring>() });
	_transferred_percentage.insert({ indication_id, 0 });

	return true;
}

std::shared_ptr<container::value_container> file_manager::received(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& indication_id, const std::wstring& file_path)
{
	auto source = _transferring_list.find(indication_id);
	if (source == _transferring_list.end())
	{
		return nullptr;
	}

	auto target = _transferred_list.find(indication_id);
	if (target == _transferred_list.end())
	{
		return nullptr;
	}

	auto fail = _failed_list.find(indication_id);
	if (fail == _failed_list.end())
	{
		return nullptr;
	}

	auto percentage = _transferred_percentage.find(indication_id);
	if (percentage == _transferred_percentage.end())
	{
		return nullptr;
	}

	if (file_path.empty())
	{
		fail->second.push_back(file_path);
	}
	else
	{
		target->second.push_back(file_path);
	}

	unsigned short temp = ((double)target->second.size() / (double)source->second.size()) * 100;
	if (percentage->second != temp)
	{
		percentage->second = temp;

		if (temp == 100)
		{
			_transferring_list.erase(source);
			_transferred_list.erase(target);
			_failed_list.erase(fail);
			_transferred_percentage.erase(percentage);
		}

		return std::make_shared<container::value_container>(target_id, target_sub_id, L"transfer_condition",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::string_value>(L"indication_id", indication_id),
				std::make_shared<container::ushort_value>(L"percentage", temp)
		});
	}
	
	if (source->second.size() == (target->second.size() + fail->second.size()))
	{
		unsigned short completed = target->second.size();
		unsigned short failed = fail->second.size();

		_transferring_list.erase(source);
		_transferred_list.erase(target);
		_failed_list.erase(fail);
		_transferred_percentage.erase(percentage);

		return std::make_shared<container::value_container>(target_id, target_sub_id, L"transfer_condition",
			std::vector<std::shared_ptr<container::value>> {
				std::make_shared<container::string_value>(L"indication_id", indication_id),
				std::make_shared<container::ushort_value>(L"percentage", temp),
				std::make_shared<container::ushort_value>(L"completed_count", completed),
				std::make_shared<container::ushort_value>(L"failed_count", failed),
				std::make_shared<container::bool_value>(L"completed", true)
		});
	}

	return nullptr;
}