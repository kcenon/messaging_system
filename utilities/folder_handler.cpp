/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "folder_handler.h"

#include <algorithm>
#include <filesystem>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace folder_handler
{
	wstring folder::get_temporary_folder(void)
	{
		return filesystem::temp_directory_path().wstring();
	}

	bool folder::create_folder(const wstring& root, const wstring& target)
	{
		if (root.empty())
		{
			return false;
		}

		if (!filesystem::exists(root))
		{
			filesystem::create_directories(root);
		}

		if (target.empty())
		{
			return true;
		}

		return filesystem::create_directory(fmt::format(L"{}/{}", root, target));
	}

	void folder::delete_folder(const wstring& target)
	{
		if (!filesystem::exists(target))
		{
			return;
		}

		filesystem::remove_all(target);
	}

	void folder::delete_folders(const vector<wstring>& targets)
	{
		for (auto& target : targets)
		{
			delete_folder(target);
		}
	}

	vector<wstring> folder::get_folders(const wstring& target_folder)
	{
		vector<wstring> result;

		if (target_folder.empty())
		{
			return result;
		}

		wstring extension;
		filesystem::path targetDir(target_folder);

		if (filesystem::exists(targetDir) != true)
		{
			return result;
		}

		filesystem::directory_iterator iterator(targetDir), endItr;

		for (; iterator != endItr; ++iterator)
		{
			if (!filesystem::is_directory(iterator->path()))
			{
				continue;
			}

			result.push_back(iterator->path().wstring());
		}

		return result;
	}

	vector<wstring> folder::get_files(const wstring& target_folder, const bool& search_sub_folder, const vector<wstring>& extensions)
	{
		vector<wstring> result;

		if (target_folder.empty())
		{
			return result;
		}

		wstring extension;
		filesystem::path targetDir(target_folder);

		if (filesystem::exists(targetDir) != true)
		{
			return result;
		}

		filesystem::directory_iterator iterator(targetDir), endItr;

		for (; iterator != endItr; ++iterator)
		{
			if (filesystem::is_regular_file(iterator->path()) == true)
			{
				if (extensions.size() == 0)
				{
					result.push_back(iterator->path().wstring());
					continue;
				}

				extension = iterator->path().extension().wstring();
				vector<wstring>::const_iterator target_extension = find_if(extensions.begin(), extensions.end(),
					[&extension](const wstring& item)
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

			if (filesystem::is_directory(iterator->path()) == true && search_sub_folder == true)
			{
				vector<wstring> innerFiles = get_files(iterator->path().wstring(), search_sub_folder, extensions);
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
