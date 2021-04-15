#pragma once

#include <string>
#include <vector>

namespace folder_handling
{
	class folder_handler
	{
	public:
		static std::vector<std::wstring> get_files(const std::wstring& target_folder, const bool& search_sub_folder = true, const std::vector<std::wstring> extensions = {});
	};
}

