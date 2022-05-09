#pragma once

#include <string>
#include <vector>

using namespace std;

namespace folder_handler
{
	class folder
	{
	public:
		static wstring get_temporary_folder(void);
		static bool create_folder(const wstring& root, const wstring& target = L"");
		static void delete_folder(const wstring& target);
		static void delete_folders(const vector<wstring>& targets);
		static vector<wstring> get_files(const wstring& target_folder, const bool& search_sub_folder = true, const vector<wstring> extensions = {});
	};
}

