#include "encrypting.h"

#include "converting.h"

#include "cryptopp/aes.h"
#include "cryptopp/hex.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "cryptopp/filters.h"

namespace encrypting
{
	using namespace converting;

	pair<wstring, wstring> encryptor::create_key(void)
	{
		CryptoPP::AutoSeededRandomPool rng;

		CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
		memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
		rng.GenerateBlock(key, CryptoPP::AES::DEFAULT_KEYLENGTH);

		CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
		memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);
		rng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

		return {
			converter::to_base64(vector<unsigned char>(key, key + CryptoPP::AES::DEFAULT_KEYLENGTH)),
			converter::to_base64(vector<unsigned char>(iv, iv + CryptoPP::AES::BLOCKSIZE))
		};
	}

	vector<unsigned char> encryptor::encryption(const vector<unsigned char>& original_data, const wstring& key_string, const wstring& iv_string)
	{
		if (original_data.empty())
		{
			return original_data;
		}

		if (key_string.empty() || iv_string.empty())
		{
			return original_data;
		}

		vector<unsigned char> encrypted;
		vector<unsigned char> key = converter::from_base64(key_string);
		vector<unsigned char> iv = converter::from_base64(iv_string);

		CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc;
		enc.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		encrypted.resize(original_data.size() + CryptoPP::AES::BLOCKSIZE);
		CryptoPP::ArraySink cs(&encrypted[0], encrypted.size());

		CryptoPP::ArraySource(original_data.data(), original_data.size(), true,
			new CryptoPP::StreamTransformationFilter(enc, new CryptoPP::Redirector(cs)));

		encrypted.resize((size_t)cs.TotalPutLength());

		return encrypted;
	}

	vector<unsigned char> encryptor::decryption(const vector<unsigned char>& encrypted_data, const wstring& key_string, const wstring& iv_string)
	{
		if (encrypted_data.empty())
		{
			return encrypted_data;
		}

		if (key_string.empty() || iv_string.empty())
		{
			return encrypted_data;
		}

		vector<unsigned char> decrypted;
		vector<unsigned char> key = converter::from_base64(key_string);
		vector<unsigned char> iv = converter::from_base64(iv_string);

		CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec;
		dec.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		decrypted.resize(encrypted_data.size());
		CryptoPP::ArraySink rs((unsigned char*)&decrypted[0], decrypted.size());

		CryptoPP::ArraySource(encrypted_data.data(), encrypted_data.size(), true,
			new CryptoPP::StreamTransformationFilter(dec, new CryptoPP::Redirector(rs)));

		decrypted.resize((size_t)rs.TotalPutLength());

		return decrypted;
	}
}