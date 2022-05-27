/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

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
			const wstring& file_header = L"CF100", const function<void(vector<uint8_t>&, const wstring&, const vector<uint8_t>&)>& compression_rule = nullptr);
		static bool decompression_folder(const wstring& source_path, const wstring& target_path, const unsigned short& block_bytes = 1024, 
			const wstring& file_header =L"CF100", const function<void(const vector<uint8_t>&, const wstring&, wstring&, vector<uint8_t>&)>& decompression_rule = nullptr);

	protected:
		static void append_binary(vector<uint8_t>& result, const vector<uint8_t>& source);
		static vector<uint8_t> devide_binary(const vector<uint8_t>& source, size_t& index);
	};
}

