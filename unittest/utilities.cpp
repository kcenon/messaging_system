#include "gtest/gtest.h"

#include <string>
#include <vector>

#include "argument_parser.h"
#include "binary_combiner.h"
#include "compressing.h"
#include "converting.h"
#include "encrypting.h"

using namespace std;
using namespace encrypting;
using namespace converting;
using namespace compressing;
using namespace binary_parser;
using namespace argument_parser;

TEST(argument, test) {
  char *test1[] = {(char *)"test.exe", (char *)"--version", (char *)"1.000"};
  wchar_t *test2[] = {(wchar_t *)L"test.exe", (wchar_t *)L"--version",
                      (wchar_t *)L"1.000"};

  argument_manager manager1(3, test1);
  argument_manager manager2(3, test2);

  EXPECT_EQ(manager1.to_string(L"--version"), manager2.to_string(L"--version"));
}

TEST(conbiner, test) {
  vector<uint8_t> data1, data2;
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < 256; ++j) {
      data1.push_back((uint8_t)j);
      data2.push_back((uint8_t)j);
    }
  }

  vector<uint8_t> container_buffer;

  combiner::append(container_buffer, data1);
  combiner::append(container_buffer, data2);

  size_t index = 0;
  auto result1 = combiner::divide(container_buffer, index);
  auto result2 = combiner::divide(container_buffer, index);

  EXPECT_EQ(data1, result1);
  EXPECT_EQ(data2, result2);
}

TEST(compressor, test) {
  vector<uint8_t> original;
  for (int i = 0; i < 256; ++i) {
    for (int j = 0; j < 256; ++j) {
      original.push_back((uint8_t)j);
    }
  }

  vector<uint8_t> compressed = compressor::compression(original);

  EXPECT_TRUE(original != compressed);

  vector<uint8_t> decompressed = compressor::decompression(compressed);

  EXPECT_EQ(original, decompressed);
}

TEST(converter, test) {
  wstring original = L"Itestamtestatestprogrammer";
  wstring source = original;
  wstring token = L"test";
  wstring target = L" ";

  converter::replace(source, token, target);

  EXPECT_TRUE(source.compare(L"I am a programmer") == 0);

  source = converter::replace2(original, token, target);

  EXPECT_TRUE(source.compare(L"I am a programmer") == 0);

  EXPECT_TRUE(
      converter::to_string(L"test has passed").compare("test has passed") == 0);
  EXPECT_TRUE(
      converter::to_wstring("test has passed").compare(L"test has passed") ==
      0);
  EXPECT_TRUE(converter::to_string(converter::to_array("test has passed"))
                  .compare("test has passed") == 0);
  EXPECT_TRUE(converter::to_wstring(converter::to_array(L"test has passed"))
                  .compare(L"test has passed") == 0);
}

TEST(cryptor, test) {
  auto key = cryptor::create_key();

  EXPECT_TRUE(!key.first.empty());
  EXPECT_TRUE(!key.second.empty());

  auto encrypted = cryptor::encryption(
      converter::to_array(L"I am a programmer"), key.first, key.second);
  auto decrypted = converter::to_wstring(
      cryptor::decryption(encrypted, key.first, key.second));

  EXPECT_TRUE(decrypted.compare(L"I am a programmer") == 0);
}