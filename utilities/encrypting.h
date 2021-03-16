#pragma once

#include <string>
#include <vector>

namespace encrypting
{
	class encryptor
	{
	public:
		static std::pair<std::wstring, std::wstring> create_key(void);
		static std::vector<char> encryption(const std::vector<char>& original_data, const std::wstring& key, const std::wstring& iv);
		static std::vector<char> decryption(const std::vector<char>& encrypted_data, const std::wstring& key, const std::wstring& iv);
	};
}
