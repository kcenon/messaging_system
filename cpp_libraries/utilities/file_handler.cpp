#include "file_handler.h"

#include <io.h>
#include <fcntl.h>
#include <filesystem>

namespace file_handler
{
	bool file::remove(const std::wstring& path)
	{
		if (!std::filesystem::exists(path))
		{
			return false;
		}

		return std::filesystem::remove(path);
	}

	std::vector<unsigned char> file::load(const std::wstring& path)
	{
		if (!std::filesystem::exists(path))
		{
			return std::vector<unsigned char>();
		}

		size_t file_size = std::filesystem::file_size(path);

		int file;
		errno_t err = _wsopen_s(&file, path.c_str(), _O_RDONLY | _O_BINARY | _O_SEQUENTIAL, _SH_DENYRD, _S_IREAD);
		if (err != 0)
		{
			return std::vector<unsigned char>();
		}

		char* temp = new char[file_size];
		memset(temp, 0, file_size);

		file_size = _read(file, temp, (unsigned int)file_size);

		std::vector<unsigned char> target;
		target.reserve(file_size);
		target.insert(target.begin(), temp, temp + file_size);

		_close(file);

		delete[] temp;
		temp = nullptr;

		return target;
	}

	bool file::save(const std::wstring& path, const std::vector<unsigned char>& data)
	{
		std::filesystem::path target_path(path);
		if (target_path.parent_path().empty() != true)
		{
			std::filesystem::create_directories(target_path.parent_path());
		}

		int file;
		errno_t err = _wsopen_s(&file, path.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		if (err != 0)
		{
			return false;
		}

		_write(file, data.data(), (unsigned int)data.size());
		_close(file);

		return true;
	}

	bool file::append(const std::wstring& source, const std::vector<unsigned char>& data)
	{
		int file;
		errno_t err = _wsopen_s(&file, source.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY, _SH_DENYWR, _S_IWRITE);
		if (err != 0)
		{
			return false;
		}

		_write(file, data.data(), (unsigned int)data.size());
		_close(file);

		std::filesystem::remove(source);

		return true;
	}
}