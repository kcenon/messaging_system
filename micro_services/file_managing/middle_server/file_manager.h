#pragma once

#include "container.h"

#include <map>
#include <string>
#include <vector>
#include <memory>

class file_manager
{
public:
	file_manager(void);
	~file_manager(void);

public:
	bool set(const std::wstring& indication_id, const std::vector<std::wstring> &file_list);
	std::shared_ptr<container::value_container> received(const std::wstring& target_id, const std::wstring& target_sub_id, const std::wstring& indication_id, const std::wstring& file_path);

private:
	std::map<std::wstring, unsigned short> _transferred_percentage;
	std::map<std::wstring, std::vector<std::wstring>> _transferring_list;
	std::map<std::wstring, std::vector<std::wstring>> _transferred_list;
	std::map<std::wstring, std::vector<std::wstring>> _failed_list;
};

