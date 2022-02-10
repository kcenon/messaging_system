#pragma once

#include "container.h"

#include <map>
#include <string>
#include <vector>
#include <memory>

using namespace std;

class file_manager
{
public:
	file_manager(void);
	~file_manager(void);

public:
	bool set(const wstring& indication_id, const vector<wstring> &file_list);
	shared_ptr<container::value_container> received(const wstring& target_id, const wstring& target_sub_id, const wstring& indication_id, const wstring& file_path);

private:
	map<wstring, unsigned short> _transferred_percentage;
	map<wstring, vector<wstring>> _transferring_list;
	map<wstring, vector<wstring>> _transferred_list;
	map<wstring, vector<wstring>> _failed_list;
};

