#include "file_handler.h"

#include "converting.h"

#include <fcntl.h>
#include <fstream>
#include <filesystem>

#include <string.h>

namespace file_handler
{
	using namespace converting;

	bool file::remove(const wstring& path)
	{
		if (!filesystem::exists(path))
		{
			return false;
		}

		return filesystem::remove(path);
	}

	vector<unsigned char> file::load(const wstring& path)
	{
		if (!filesystem::exists(path))
		{
			return vector<unsigned char>();
		}

		size_t file_size = filesystem::file_size(path);

#ifdef _WIN32
		fstream stream(path, ios::in | ios::binary);
#else
		fstream stream(converter::to_string(path), ios::in | ios::binary);
#endif
		if (!stream.is_open())
		{
			return vector<unsigned char>();
		}

		char* temp = new char[file_size];
		memset(temp, 0, file_size);

		stream.seekg(0, ios::beg);
		stream.read(temp, file_size);
		stream.close();

		vector<unsigned char> target;
		target.reserve(file_size);
		target.insert(target.begin(), temp, temp + file_size);

		delete[] temp;
		temp = nullptr;

		return target;
	}

	bool file::save(const wstring& path, const vector<unsigned char>& data)
	{
		filesystem::path target_path(path);
		if (target_path.parent_path().empty() != true)
		{
			filesystem::create_directories(target_path.parent_path());
		}

#ifdef _WIN32
		fstream stream(path, ios::out | ios::binary | ios::trunc);
#else
		fstream stream(converter::to_string(path), ios::out | ios::binary | ios::trunc);
#endif
		if (!stream.is_open())
		{
			return false;
		}

		stream.write((char *)data.data(), (unsigned int)data.size());
		stream.close();

		return true;
	}

	bool file::append(const wstring& source, const vector<unsigned char>& data)
	{
#ifdef _WIN32
		fstream stream(source, ios::out | ios::binary | ios::app);
#else
		fstream stream(converter::to_string(source), ios::out | ios::binary | ios::app);
#endif
		if (!stream.is_open())
		{
			return false;
		}

		stream.write((char*)data.data(), (unsigned int)data.size());
		stream.close();

		return true;
	}
}
