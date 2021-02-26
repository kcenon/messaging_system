#pragma once

#include <vector>

namespace compressor
{
	class util
	{
	public:
		static std::vector<unsigned char> compression(const std::vector<unsigned char>& original_data);
		static std::vector<unsigned char> decompression(const std::vector<unsigned char>& compressed_data);

	public:
		static void set_block_bytes(const unsigned short& block_bytes);
		static unsigned short get_block_bytes(void);

	private:
		static unsigned short _block_bytes;
	}
};

