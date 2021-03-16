﻿#include "compressing.h"

#include "logging.h"

#include "lz4.h"

#include "fmt/format.h"

namespace compressing
{
	using namespace logging;

	unsigned short compressor::_block_bytes = 1024;

	std::vector<char> compressor::compression(const std::vector<char>& original_data)
	{
		if (original_data.empty())
		{
			return original_data;
		}

		auto start = logger::handle().chrono_start();

		LZ4_stream_t lz4Stream_body;
		size_t read_index = 0;
		int source_buffer_index = 0;

		std::vector<std::vector<char>> source_buffer;
		source_buffer.push_back(std::vector<char>());
		source_buffer.push_back(std::vector<char>());
		source_buffer[0].reserve(_block_bytes);
		source_buffer[1].reserve(_block_bytes);

		int compress_size = LZ4_COMPRESSBOUND(_block_bytes);
		std::vector<char> compress_buffer;
		compress_buffer.reserve(compress_size);
		std::vector<char> compressed_data;

		char compress_size_data[4];

		LZ4_resetStream(&lz4Stream_body);

		while (true) {
			char* const source_buffer_pointer = source_buffer[source_buffer_index].data();
			memset(source_buffer_pointer, 0, sizeof(char) * _block_bytes);

			const size_t inpBytes = ((original_data.size() - read_index) > _block_bytes) ? _block_bytes : (original_data.size() - read_index);
			if (0 == inpBytes) {
				break;
			}

			memcpy(source_buffer_pointer, original_data.data() + read_index, sizeof(char) * inpBytes);

			{
				char* const compress_buffer_pointer = compress_buffer.data();
				memset(compress_buffer_pointer, 0, sizeof(char) * compress_size);

				const int compressed_size = LZ4_compress_fast_continue(&lz4Stream_body, source_buffer_pointer, compress_buffer_pointer, (int)inpBytes, compress_size, 1);
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

		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> diff = end - start;

		if (original_data.size() < compressed_data.size())
		{
			logger::handle().write(logging::logging_level::information, L"this file does not need to use compress because compressed result is bigger than original data", start);

			return original_data;
		}

		logger::handle().write(logging::logging_level::information, fmt::format(L"compressing(buffer {}): ({} -> {} : {:.2f} %)", 
			_block_bytes, original_data.size(), compressed_data.size(), (((double)compressed_data.size() / (double)original_data.size()) * 100)), start);

		return compressed_data;
	}

	std::vector<char> compressor::decompression(const std::vector<char>& compressed_data)
	{
		if (compressed_data.empty())
		{
			return compressed_data;
		}

		auto start = std::chrono::steady_clock::now();

		LZ4_streamDecode_t lz4StreamDecode_body;

		int read_index = 0;
		int compressed_size = 0;

		int target_buffer_index = 0;

		std::vector<std::vector<char>> target_buffer;
		target_buffer.push_back(std::vector<char>());
		target_buffer.push_back(std::vector<char>());
		target_buffer[0].reserve(_block_bytes);
		target_buffer[1].reserve(_block_bytes);

		int compress_size = LZ4_COMPRESSBOUND(_block_bytes);
		std::vector<char> compress_buffer;
		compress_buffer.reserve(compress_size);
		std::vector<char> decompressed_data;

		LZ4_setStreamDecode(&lz4StreamDecode_body, NULL, 0);

		while (true) {
			char* const compress_buffer_pointer = compress_buffer.data();

			memset(compress_buffer_pointer, 0, sizeof(char) * compress_size);

			{
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
			}

			{
				char* const target_buffer_pointer = target_buffer[target_buffer_index].data();
				const int decompressed_size = LZ4_decompress_safe_continue(&lz4StreamDecode_body, compress_buffer_pointer, target_buffer_pointer, compressed_size, _block_bytes);
				if (decompressed_size <= 0) {
					break;
				}

				decompressed_data.insert(decompressed_data.end(), target_buffer_pointer, target_buffer_pointer + decompressed_size);
			}

			target_buffer_index = (target_buffer_index + 1) % 2;
		}

		target_buffer[0].clear();
		target_buffer[1].clear();
		target_buffer.clear();

		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> diff = end - start;

		if (decompressed_data.size() < compressed_data.size())
		{
			logger::handle().write(logging::logging_level::information, L"this file does not need to use decompress because it is not compressed data", start);

			return compressed_data;
		}

		logger::handle().write(logging::logging_level::information, fmt::format(L"decompressing(buffer {}): ({} -> {} : {:.2f} %)",
			_block_bytes, compressed_data.size(), decompressed_data.size(), (((double)compressed_data.size() / (double)decompressed_data.size()) * 100)), start);

		return decompressed_data;
	}

	void compressor::set_block_bytes(const unsigned short& block_bytes)
	{
		_block_bytes = block_bytes;
	}

	unsigned short compressor::get_block_bytes(void)
	{
		return _block_bytes;
	}
}