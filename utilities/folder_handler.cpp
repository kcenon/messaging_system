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

#include "fmt/format.h"
#include "fmt/xchar.h"

namespace folder_handler
{
  std::wstring folder::get_temporary_folder(void) { return std::filesystem::temp_directory_path().wstring(); }

  bool folder::create_folder(const std::wstring &root, const std::wstring &target)
  {
    if (root.empty())
    {
      return false;
    }

    std::filesystem::path root_path(root);
    if (!std::filesystem::exists(root_path))
    {
      std::filesystem::create_directories(root_path);
    }

    if (target.empty())
    {
      return true;
    }

    return std::filesystem::create_directory(root_path / target);
  }

  void folder::delete_folder(const std::wstring &target)
  {
    std::filesystem::path target_path(target);
    if (!std::filesystem::exists(target_path))
    {
      return;
    }

    std::filesystem::remove_all(target_path);
  }

  void folder::delete_folders(const std::vector<std::wstring> &targets)
  {
    for (const auto &target : targets)
    {
      delete_folder(target);
    }
  }

  std::vector<std::wstring> folder::get_folders(const std::wstring &target_folder)
  {
    std::vector<std::wstring> result;

    if (target_folder.empty())
    {
      return result;
    }

    std::filesystem::path targetDir(target_folder);
    if (std::filesystem::exists(targetDir) != true)
    {
      return result;
    }

    std::filesystem::directory_iterator iterator(targetDir);
    for (const auto &entry : iterator)
    {
      if (!std::filesystem::is_directory(entry.path()))
      {
        continue;
      }

      result.push_back(entry.path().wstring());
    }

    return result;
  }

  std::vector<std::wstring> folder::get_files(const std::wstring &target_folder,
                                              const bool &search_sub_folder,
                                              const std::vector<std::wstring> &extensions)
  {
    std::vector<std::wstring> result;

    if (target_folder.empty())
    {
      return result;
    }

    std::filesystem::path targetDir(target_folder);
    if (std::filesystem::exists(targetDir) != true)
    {
      return result;
    }

    std::filesystem::directory_iterator iterator(targetDir);
    for (const auto &entry : iterator)
    {
      if (std::filesystem::is_directory(entry.path()) && search_sub_folder)
      {
        auto innerFiles = get_files(entry.path().wstring(), search_sub_folder, extensions);
        result.insert(result.end(), innerFiles.begin(), innerFiles.end());

        continue;
      }

      if (!std::filesystem::is_regular_file(entry.path()))
      {
        continue;
      }

      if (extensions.empty()
          || std::find(extensions.begin(), extensions.end(), entry.path().extension().wstring()) != extensions.end())
      {
        result.push_back(entry.path().wstring());
      }
    }

    return result;
  }
} // namespace folder_handler
