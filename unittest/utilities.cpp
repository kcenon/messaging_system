#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "argument_parser.h"
#include "binary_combiner.h"
#include "compressing.h"
#include "converting.h"
#include "encrypting.h"

using namespace encrypting;
using namespace converting;
using namespace compressing;
using namespace binary_parser;
using namespace argument_parser;

TEST(argument, test)
{
	char* test1[] = { (char*)"test.exe", (char*)"--version", (char*)"1.000" };

	argument_manager manager1(3, test1);

	EXPECT_EQ(manager1.to_string("--version"), "1.000");
}

TEST(conbiner, test)
{
	std::vector<uint8_t> data1, data2;
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			data1.push_back((uint8_t)j);
			data2.push_back((uint8_t)j);
		}
	}

	std::vector<uint8_t> container_buffer;

	combiner::append(container_buffer, data1);
	combiner::append(container_buffer, data2);

	size_t index = 0;
	auto result1 = combiner::divide(container_buffer, index);
	auto result2 = combiner::divide(container_buffer, index);

	EXPECT_EQ(data1, result1);
	EXPECT_EQ(data2, result2);
}

TEST(compressor, test)
{
	std::vector<uint8_t> original;
	for (int i = 0; i < 256; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			original.push_back((uint8_t)j);
		}
	}

	auto [compressed, compressed_message] = compressor::compression(original);

	EXPECT_TRUE(compressed.has_value());
	EXPECT_TRUE(original != compressed.value());

	auto [decompressed, decompressed_message]
		= compressor::decompression(compressed.value());

	EXPECT_TRUE(decompressed.has_value());
	EXPECT_EQ(original, decompressed.value());
}

TEST(converter, test)
{
	std::string original = "Itestamtestatestprogrammer";
	std::string source = original;
	std::string token = "test";
	std::string target = " ";

	converter::replace(source, token, target);

	EXPECT_TRUE(source.compare("I am a programmer") == 0);

	source = converter::replace2(original, token, target);

	EXPECT_TRUE(source.compare("I am a programmer") == 0);

	EXPECT_TRUE(
		converter::to_string("test has passed").compare("test has passed")
		== 0);
	EXPECT_TRUE(
		converter::to_string("test has passed").compare("test has passed")
		== 0);
	EXPECT_TRUE(converter::to_string(converter::to_array("test has passed"))
					.compare("test has passed")
				== 0);
	EXPECT_TRUE(converter::to_string(converter::to_array("test has passed"))
					.compare("test has passed")
				== 0);
}

TEST(cryptor, test)
{
	auto [key, iv] = cryptor::create_key();

	EXPECT_TRUE(!key.empty());
	EXPECT_TRUE(!iv.empty());

	auto encrypted = cryptor::encryption(
		converter::to_array("I am a programmer"), key, iv);
	auto decrypted
		= converter::to_string(cryptor::decryption(encrypted, key, iv));

	EXPECT_TRUE(decrypted.compare("I am a programmer") == 0);
}