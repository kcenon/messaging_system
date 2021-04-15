#include "folder_handling.h"

#include <filesystem>

namespace folder_handling
{
	std::vector<std::wstring> folder_handler::get_files(const std::wstring& target_folder, const bool& search_sub_folder, const std::vector<std::wstring> extensions)
	{
		std::vector<std::wstring> result;

		if (target_folder.empty())
		{
			return result;
		}

		std::wstring extension;
		std::filesystem::path targetDir(target_folder);

		if (std::filesystem::exists(targetDir) != true)
		{
			return result;
		}

		std::filesystem::directory_iterator iterator(targetDir), endItr;

		for (; iterator != endItr; ++iterator)
		{
			if (std::filesystem::is_regular_file(iterator->path()) == true)
			{
				if (extensions.size() == 0)
				{
					result.push_back(iterator->path().wstring());
					continue;
				}

				extension = iterator->path().extension().wstring();
				std::vector<std::wstring>::const_iterator target_extension = std::find_if(extensions.begin(), extensions.end(),
					[&extension](std::wstring item)
					{
						return item == extension;
					});

				if (target_extension == extensions.end())
				{
					continue;
				}

				result.push_back(iterator->path().wstring());
				continue;
			}

			if (std::filesystem::is_directory(iterator->path()) == true && search_sub_folder == true)
			{
				std::vector<std::wstring> innerFiles = get_files(iterator->path().wstring(), search_sub_folder, extensions);
				if (innerFiles.empty() == true)
				{
					continue;
				}

				result.insert(result.end(), innerFiles.begin(), innerFiles.end());
				continue;
			}
		}

		return result;
	}
}