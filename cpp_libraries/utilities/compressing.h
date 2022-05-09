#pragma once

#include <vector>

using namespace std;

namespace compressing
{
	class compressor
	{
	public:
		static vector<unsigned char> compression(const vector<unsigned char>& original_data, const unsigned short& _block_bytes = 1024);
		static vector<unsigned char> decompression(const vector<unsigned char>& compressed_data, const unsigned short& _block_bytes = 1024);
	};
}

