#include "file_handling.h"

#include <io.h>
#include <fcntl.h>
#include <filesystem>

namespace file_handling
{
	std::vector<char> file_handler::load(const std::wstring& path)
	{
		if (!std::filesystem::exists(path))
		{
			return std::vector<char>();
		}

		int file;
		errno_t err = _wsopen_s(&file, path.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYRD, _S_IREAD);
		if (err != 0)
		{
			return std::vector<char>();
		}

		size_t file_size = _lseek(file, 0, SEEK_END);
		_lseek(file, 0, SEEK_SET);

		char* temp = new char[file_size];
		memset(temp, 0, file_size);

		file_size = _read(file, temp, (unsigned int)file_size);

		std::vector<char> target;
		target.reserve(file_size);
		target.insert(target.begin(), temp, temp + file_size);

		_close(file);

		delete[] temp;
		temp = nullptr;

		return target;
	}

	bool file_handler::save(const std::wstring& path, const std::vector<char>& data)
	{
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

	bool file_handler::append(const std::wstring& source, const std::vector<char>& data)
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