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

#include "compressing.h"

#include "converting.h"
#include "file_handler.h"
#include "folder_handler.h"

#include "lz4.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <algorithm>
#include <filesystem>

namespace compressing
{
  using namespace converting;
  using namespace file_handler;
  using namespace folder_handler;

  std::tuple<std::vector<uint8_t>, std::wstring> compressor::compression(const std::vector<uint8_t> &original_data,
                                                                         const unsigned short &block_bytes)
  {
    if (original_data.empty())
    {
      return { {}, L"original data is empty" };
    }

    LZ4_stream_t lz4Stream_body;
    LZ4_resetStream(&lz4Stream_body);

    size_t read_index = 0;
    std::vector<std::vector<char> > source_buffer(2, std::vector<char>(block_bytes));

    int compress_size = LZ4_COMPRESSBOUND(block_bytes);
    std::vector<char> compress_buffer(compress_size);
    std::vector<uint8_t> compressed_data;

    while (read_index < original_data.size())
    {
      const size_t inpBytes = std::min(static_cast<size_t>(block_bytes), original_data.size() - read_index);
      std::copy_n(original_data.begin() + read_index, inpBytes, source_buffer[read_index % 2].begin());

      int compressed_size
          = LZ4_compress_fast_continue(&lz4Stream_body, source_buffer[read_index % 2].data(), compress_buffer.data(),
                                       static_cast<int>(inpBytes), compress_size, 1);

      if (compressed_size <= 0)
      {
        // Handle error appropriately, perhaps throw an exception
        break;
      }

      compressed_data.insert(compressed_data.end(), reinterpret_cast<char *>(&compressed_size),
                             reinterpret_cast<char *>(&compressed_size) + sizeof(int));
      compressed_data.insert(compressed_data.end(), compress_buffer.begin(), compress_buffer.begin() + compressed_size);

      read_index += inpBytes;
    }

    if (compressed_data.size() == 0)
    {
      return { {}, L"cannot complete to compress data" };
    }

    return { compressed_data, fmt::format(L"compressing(buffer {}): ({} -> {} : {:.2f} %)", block_bytes,
                                          original_data.size(), compressed_data.size(),
                                          (((double)compressed_data.size() / (double)original_data.size()) * 100)) };
  }

  std::tuple<std::vector<uint8_t>, std::wstring> compressor::decompression(const std::vector<uint8_t> &compressed_data,
                                                                           const unsigned short &block_bytes)
  {
    if (compressed_data.empty())
    {
      return { {}, L"original data is empty" };
    }

    LZ4_streamDecode_t lz4StreamDecode_body;
    LZ4_setStreamDecode(&lz4StreamDecode_body, nullptr, 0);

    size_t read_index = 0;
    std::vector<std::vector<char> > target_buffer(2, std::vector<char>(block_bytes));
    std::vector<uint8_t> decompressed_data;

    while (read_index < compressed_data.size())
    {
      if (compressed_data.size() - read_index < sizeof(int))
      {
        // Handle error: incomplete data for size
        break;
      }

      int compressed_size;
      std::memcpy(&compressed_size, compressed_data.data() + read_index, sizeof(int));
      read_index += sizeof(int);

      if (compressed_size <= 0 || compressed_data.size() - read_index < static_cast<size_t>(compressed_size))
      {
        // Handle error: invalid compressed size or insufficient data
        break;
      }

      int decompressed_size = LZ4_decompress_safe_continue(
          &lz4StreamDecode_body, reinterpret_cast<const char *>(compressed_data.data()) + read_index,
          target_buffer[read_index % 2].data(), compressed_size, block_bytes);

      if (decompressed_size <= 0)
      {
        // Handle error: decompression failed
        break;
      }

      decompressed_data.insert(decompressed_data.end(), target_buffer[read_index % 2].begin(),
                               target_buffer[read_index % 2].begin() + decompressed_size);

      read_index += compressed_size;
    }

    if (decompressed_data.size() == 0)
    {
      return { {}, L"cannot complete to decompress data" };
    }

    return { decompressed_data,
             fmt::format(L"decompressing(buffer {}): ({} -> {} : {:.2f} %)", block_bytes, compressed_data.size(),
                         decompressed_data.size(),
                         (((double)compressed_data.size() / (double)decompressed_data.size()) * 100)) };
  }
} // namespace compressing
