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

#include "binary_combiner.h"

#include <algorithm>
#include <cstring>

namespace binary_parser
{
  void combiner::append(std::vector<uint8_t> &result, const std::vector<uint8_t> &source)
  {
    size_t temp = source.size();
    const auto *temp_size = reinterpret_cast<const uint8_t *>(&temp);

    result.insert(result.end(), temp_size, temp_size + sizeof(size_t));
    if (temp == 0)
    {
      return;
    }

    result.insert(result.end(), source.begin(), source.end());
  }

  std::vector<uint8_t> combiner::divide(const std::vector<uint8_t> &source, size_t &index)
  {
    if (source.empty() || source.size() < index + sizeof(size_t))
    {
      return {};
    }

    size_t temp;
    std::copy(source.begin() + index, source.begin() + index + sizeof(size_t), reinterpret_cast<uint8_t *>(&temp));
    index += sizeof(size_t);

    if (temp == 0 || source.size() < index + temp)
    {
      return {};
    }

    std::vector<uint8_t> result(source.begin() + index, source.begin() + index + temp);
    index += temp;

    return result;
  }
} // namespace binary_parser