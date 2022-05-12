#pragma once

#include <string>
#include <vector>
#include <functional>

using namespace std;

namespace compressing
{
	class compressor
	{
	public:
		static vector<uint8_t> compression(const vector<uint8_t>& original_data, 
			const unsigned short& block_bytes = 1024);
		static vector<uint8_t> decompression(const vector<uint8_t>& compressed_data, 
			const unsigned short& block_bytes = 1024);

	public:
		static bool compression_folder(const wstring& target_file, const wstring& root_path, 
			const wstring& folder_path, const bool& contain_sub_folder = false, const unsigned short& block_bytes = 1024, 
			const function<void(vector<uint8_t>&, const wstring&, const vector<uint8_t>&)>& combination_rule = nullptr);
		static bool decompression_folder(const wstring& source_path, const wstring& target_path, const unsigned short& block_bytes = 1024,
			const function<void(const vector<uint8_t>&, wstring&, vector<uint8_t>&)>& combination_rule = nullptr);

	protected:
		static void append_binary(vector<uint8_t>& result, const vector<uint8_t>& source);
		static vector<uint8_t> devide_binary(const vector<uint8_t>& source, size_t& index);
	};
}

