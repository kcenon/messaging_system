#include "compressing.h"

#include "logging.h"
#include "converting.h"
#include "file_handler.h"
#include "folder_handler.h"

#include "lz4.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <algorithm>
#include <filesystem>

constexpr auto FILE_HEADE = "PCAI[1000]";

namespace compressing
{
	using namespace logging;
	using namespace converting;
	using namespace file_handler;
	using namespace folder_handler;

	vector<unsigned char> compressor::compression(const vector<unsigned char>& original_data, const unsigned short& block_bytes)
	{
		if (original_data.empty())
		{
			return original_data;
		}

		LZ4_stream_t lz4Stream_body;
		size_t read_index = 0;
		int source_buffer_index = 0;

		vector<vector<char>> source_buffer;
		source_buffer.push_back(vector<char>());
		source_buffer.push_back(vector<char>());
		source_buffer[0].reserve(block_bytes);
		source_buffer[1].reserve(block_bytes);

		int compress_size = LZ4_COMPRESSBOUND(block_bytes);
		vector<char> compress_buffer;
		compress_buffer.reserve(compress_size);
		vector<unsigned char> compressed_data;

		char compress_size_data[4];

		LZ4_resetStream(&lz4Stream_body);

		while (true) {
			char* const source_buffer_pointer = source_buffer[source_buffer_index].data();
			memset(source_buffer_pointer, 0, sizeof(char) * block_bytes);

			const size_t inpBytes = ((original_data.size() - read_index) > block_bytes) ? block_bytes : (original_data.size() - read_index);
			if (0 == inpBytes) {
				break;
			}

			memcpy(source_buffer_pointer, original_data.data() + read_index, sizeof(char) * inpBytes);

			{
				char* const compress_buffer_pointer = compress_buffer.data();
				memset(compress_buffer_pointer, 0, sizeof(char) * compress_size);

				const int compressed_size = LZ4_compress_fast_continue(&lz4Stream_body, (const char*)source_buffer_pointer, (char*)compress_buffer_pointer, (int)inpBytes, compress_size, 1);
				if (compressed_size <= 0) {
					break;
				}

				memcpy(compress_size_data, &compressed_size, sizeof(int));

				compressed_data.insert(compressed_data.end(), compress_size_data, compress_size_data + sizeof(int));
				compressed_data.insert(compressed_data.end(), compress_buffer_pointer, compress_buffer_pointer + compressed_size);
			}

			read_index += inpBytes;
			source_buffer_index = (source_buffer_index + 1) % 2;
		}

		source_buffer[0].clear();
		source_buffer[1].clear();
		source_buffer.clear();
		
		if (compressed_data.size() == 0)
		{
			logger::handle().write(logging_level::error, L"cannot complete to compress data");

			return vector<unsigned char>();
		}
		
		logger::handle().write(logging_level::sequence, fmt::format(L"compressing(buffer {}): ({} -> {} : {:.2f} %)", 
			block_bytes, original_data.size(), compressed_data.size(), (((double)compressed_data.size() / (double)original_data.size()) * 100)));

		return compressed_data;
	}

	vector<unsigned char> compressor::decompression(const vector<unsigned char>& compressed_data, const unsigned short& block_bytes)
	{
		if (compressed_data.empty())
		{
			return compressed_data;
		}

		LZ4_streamDecode_t lz4StreamDecode_body;

		int read_index = 0;
		int compressed_size = 0;

		int target_buffer_index = 0;

		vector<vector<char>> target_buffer;
		target_buffer.push_back(vector<char>());
		target_buffer.push_back(vector<char>());
		target_buffer[0].reserve(block_bytes);
		target_buffer[1].reserve(block_bytes);

		int compress_size = LZ4_COMPRESSBOUND(block_bytes);
		vector<char> compress_buffer;
		compress_buffer.reserve(compress_size);
		vector<unsigned char> decompressed_data;

		LZ4_setStreamDecode(&lz4StreamDecode_body, NULL, 0);

		while (true) {
			char* const compress_buffer_pointer = compress_buffer.data();

			memset(compress_buffer_pointer, 0, sizeof(char) * compress_size);
			if ((compressed_data.size() - read_index) < 1) {
				break;
			}

			memcpy(&compressed_size, compressed_data.data() + read_index, sizeof(int));
			if (0 >= compressed_size) {
				break;
			}

			read_index += sizeof(int);
			memcpy(compress_buffer_pointer, compressed_data.data() + read_index, sizeof(char) * compressed_size);
			read_index += compressed_size;
			
			char* const target_buffer_pointer = target_buffer[target_buffer_index].data();
			const int decompressed_size = LZ4_decompress_safe_continue(&lz4StreamDecode_body, (const char*)compress_buffer_pointer, 
				(char*)target_buffer_pointer, compressed_size, block_bytes);
			if (decompressed_size <= 0) {
				break;
			}

			decompressed_data.insert(decompressed_data.end(), target_buffer_pointer, target_buffer_pointer + decompressed_size);

			target_buffer_index = (target_buffer_index + 1) % 2;
		}

		target_buffer[0].clear();
		target_buffer[1].clear();
		target_buffer.clear();
		
		if (decompressed_data.size() == 0)
		{
			logger::handle().write(logging_level::error, L"cannot complete to decompress data");

			return vector<unsigned char>();
		}

		logger::handle().write(logging_level::sequence, fmt::format(L"decompressing(buffer {}): ({} -> {} : {:.2f} %)",
			block_bytes, compressed_data.size(), decompressed_data.size(), (((double)compressed_data.size() / (double)decompressed_data.size()) * 100)));

		return decompressed_data;
	}
	
	bool compressor::compression_folder(const wstring& target_file, const wstring& root_path, const wstring& folder_path, 
		const bool& contain_sub_folder, const unsigned short& block_bytes)
	{
		if(target_file.empty())
		{
			return false;
		}
		
		if(!filesystem::exists(root_path))
		{
			return false;
		}

		wstring temp;
		vector<unsigned char> result;

		if(root_path == folder_path)
		{
			auto header = converter::to_array(FILE_HEADE);
			result.insert(result.end(), header.begin(), header.end());
		}

		auto files = folder::get_files(folder_path, false);
		for (auto& file : files)
		{
			temp = file;
			converter::replace(temp, root_path, L"");
#ifdef _WIN32
			converter::replace(temp, L"\\", L"/");
#endif
			vector<unsigned char> temp_buffer;
			append_binary(temp_buffer, converter::to_array(temp));
			append_binary(temp_buffer, file::load(file));
			append_binary(result, compression(temp_buffer, block_bytes));
		}

		if (!result.empty())
		{
			file::append(target_file, result);
		}

		if (!contain_sub_folder)
		{
			return true;
		}

		auto folders = folder::get_folders(folder_path);
		for (auto& folder : folders)
		{
			compression_folder(target_file, root_path, folder, contain_sub_folder);
		}

		return true;
	}
	
	bool compressor::decompression_folder(const wstring& source_path, const wstring& target_path, const unsigned short& block_bytes)
	{
		if (!folder::create_folder(target_path))
		{
			return false;
		}

		auto source = file::load(source_path);
		if (source.empty())
		{
			return false;
		}

		auto header = converter::to_array(FILE_HEADE);
		if (!equal(source.begin(), source.begin() + header.size(), header.begin()))
		{
			return false;
		}

		size_t index = header.size();
		size_t index2 = 0;
		size_t count = source.size();
		while (index < count)
		{
			vector<unsigned char> temp;
			temp = devide_binary(source, index);
			temp = decompression(temp, block_bytes);

			index2 = 0;
			auto file_path = fmt::format(L"{}{}", target_path, converter::to_wstring(devide_binary(temp, index2)));
			auto file_data = devide_binary(temp, index2);

			file::save(file_path, file_data);
		}

		return true;
	}

	void compressor::append_binary(vector<unsigned char>& result, const vector<unsigned char>& source)
	{
		size_t temp;
		const int size = sizeof(size_t);
		char temp_size[size];

		temp = source.size();
		memcpy(temp_size, (char*)&temp, size);
		result.insert(result.end(), temp_size, temp_size + size);
		if (temp == 0)
		{
			return;
		}

		result.insert(result.end(), source.begin(), source.end());
	}

	vector<unsigned char> compressor::devide_binary(const vector<unsigned char>& source, size_t& index)
	{
		if (source.empty())
		{
			return vector<unsigned char>();
		}

		size_t temp;
		const int size = sizeof(size_t);

		if (source.size() < index + size)
		{
			return vector<unsigned char>();
		}

		memcpy(&temp, source.data() + index, size);
		index += size;

		if (temp == 0 || source.size() < index + temp)
		{
			return vector<unsigned char>();
		}

		vector<unsigned char> result;
		result.insert(result.end(), source.begin() + index, source.begin() + index + temp);
		index += temp;

		return result;
	}
}
