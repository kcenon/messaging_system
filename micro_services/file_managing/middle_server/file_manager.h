#pragma once

#ifndef __USE_TYPE_CONTAINER__
#include "cpprest/json.h"
#else
#include "container.h"
#endif

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <memory>

using namespace std;

#ifndef __USE_TYPE_CONTAINER__
using namespace web;
#endif

class file_manager
{
public:
	file_manager(void);
	~file_manager(void);

public:
	bool set(const wstring& indication_id, const vector<wstring> &file_list);

#ifndef __USE_TYPE_CONTAINER__
	shared_ptr<json::value> received(const wstring& target_id, const wstring& target_sub_id,
		const wstring& indication_id, const wstring& file_path);
#else
	shared_ptr<container::value_container> received(const wstring& target_id, const wstring& target_sub_id, 
		const wstring& indication_id, const wstring& file_path);
#endif

private:
	mutex _mutex;
	map<wstring, unsigned short> _transferred_percentage;
	map<wstring, vector<wstring>> _transferring_list;
	map<wstring, vector<wstring>> _transferred_list;
	map<wstring, vector<wstring>> _failed_list;
};

