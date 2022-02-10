#pragma once

#include <vector>

using namespace std;

namespace compressing
{
	class compressor
	{
	public:
		static vector<unsigned char> compression(const vector<unsigned char>& original_data);
		static vector<unsigned char> decompression(const vector<unsigned char>& compressed_data);

	public:
		static void set_block_bytes(const unsigned short& block_bytes);
		static unsigned short get_block_bytes(void);

	private:
		static unsigned short _block_bytes;
	};
}

