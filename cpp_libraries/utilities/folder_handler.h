#pragma once

#include <string>
#include <vector>

namespace folder_handler
{
	class folder
	{
	public:
		static std::wstring get_temporary_folder(void);
		static std::vector<std::wstring> get_files(const std::wstring& target_folder, const bool& search_sub_folder = true, const std::vector<std::wstring> extensions = {});
	};
}

