#pragma once

#include <vector>

namespace compressing
{
	class compressor
	{
	public:
		static std::vector<char> compression(const std::vector<char>& original_data);
		static std::vector<char> decompression(const std::vector<char>& compressed_data);

	public:
		static void set_block_bytes(const unsigned short& block_bytes);
		static unsigned short get_block_bytes(void);

	private:
		static unsigned short _block_bytes;
	};
}

