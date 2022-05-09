#pragma once

#include <string>
#include <vector>

using namespace std;

namespace compressing
{
	class compressor
	{
	public:
		static vector<unsigned char> compression(const vector<unsigned char>& original_data, 
			const unsigned short& block_bytes = 1024);
		static vector<unsigned char> decompression(const vector<unsigned char>& compressed_data, 
			const unsigned short& block_bytes = 1024);

	public:
		static vector<unsigned char> compression_folder(const wstring& root_path, const wstring& folder_path,
			const bool& contain_sub_folder = false, const unsigned short& block_bytes = 1024);
		static bool decompression_folder(const wstring& source_path, const wstring& target_path,
			const unsigned short& block_bytes = 1024);

	protected:
		static void append_binary(vector<unsigned char>& result, const vector<unsigned char>& source);
		static vector<unsigned char> devide_binary(const vector<unsigned char>& source, size_t& index);
	};
}

