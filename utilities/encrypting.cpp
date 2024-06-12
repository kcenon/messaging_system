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

#include "encrypting.h"

#include "converting.h"

#include "cryptopp/aes.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"

namespace encrypting
{
	using namespace converting;

	auto cryptor::create_key(void) -> std::tuple<std::string, std::string>
	{
		CryptoPP::AutoSeededRandomPool rng;

		std::vector<CryptoPP::byte> key(CryptoPP::AES::DEFAULT_KEYLENGTH);
		rng.GenerateBlock(key.data(), CryptoPP::AES::DEFAULT_KEYLENGTH);

		std::vector<CryptoPP::byte> iv(CryptoPP::AES::BLOCKSIZE);
		rng.GenerateBlock(iv.data(), CryptoPP::AES::BLOCKSIZE);

		return { converter::to_base64(key), converter::to_base64(iv) };
	}

	auto cryptor::encryption(const std::vector<uint8_t>& original_data,
							 const std::string& key_string,
							 const std::string& iv_string)
		-> std::vector<uint8_t>
	{
		if (original_data.empty() || key_string.empty() || iv_string.empty())
		{
			return original_data;
		}

		std::vector<uint8_t> key = converter::from_base64(key_string);
		std::vector<uint8_t> iv = converter::from_base64(iv_string);

		CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
		enc.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		std::vector<uint8_t> encrypted(original_data.size()
									   + CryptoPP::AES::BLOCKSIZE);

		CryptoPP::ArraySink cs(&encrypted[0], encrypted.size());
		CryptoPP::ArraySource source(original_data.data(), original_data.size(),
									 true,
									 new CryptoPP::StreamTransformationFilter(
										 enc, new CryptoPP::Redirector(cs)));

		encrypted.resize(cs.TotalPutLength());

		return encrypted;
	}

	auto cryptor::decryption(const std::vector<uint8_t>& encrypted_data,
							 const std::string& key_string,
							 const std::string& iv_string)
		-> std::vector<uint8_t>
	{
		if (encrypted_data.empty() || key_string.empty() || iv_string.empty())
		{
			return encrypted_data;
		}

		std::vector<uint8_t> key = converter::from_base64(key_string);
		std::vector<uint8_t> iv = converter::from_base64(iv_string);

		CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
		dec.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		std::vector<uint8_t> decrypted(encrypted_data.size());

		CryptoPP::ArraySink rs(&decrypted[0], decrypted.size());
		CryptoPP::ArraySource source(encrypted_data.data(),
									 encrypted_data.size(), true,
									 new CryptoPP::StreamTransformationFilter(
										 dec, new CryptoPP::Redirector(rs)));

		decrypted.resize(rs.TotalPutLength());

		return decrypted;
	}
} // namespace encrypting