#pragma once

#include <string>
#include <vector>

using namespace std;

namespace encrypting
{
	class encryptor
	{
	public:
		static pair<wstring, wstring> create_key(void);
		static vector<unsigned char> encryption(const vector<unsigned char>& original_data, const wstring& key, const wstring& iv);
		static vector<unsigned char> decryption(const vector<unsigned char>& encrypted_data, const wstring& key, const wstring& iv);
	};
}
