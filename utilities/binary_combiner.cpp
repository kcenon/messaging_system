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

#include <cstring>
#include <algorithm>

namespace binary_parser
{
    void combiner::append(vector<uint8_t>& result, const vector<uint8_t>& source)
    {
        size_t temp;
        const int size = sizeof(size_t);
        char temp_size[size];

        temp = source.size();
        memcpy(temp_size, (char*)&temp, size);
        result.insert(result.end(), temp_size, temp_size + size);
        if (temp == 0)
        {
            return;
        }

        result.insert(result.end(), source.rbegin(), source.rend());
    }

    vector<uint8_t> combiner::divide(const vector<uint8_t>& source, size_t& index)
    {
        if (source.empty())
        {
            return vector<uint8_t>();
        }

        size_t temp;
        const int size = sizeof(size_t);

        if (source.size() < index + size)
        {
            return vector<uint8_t>();
        }

        memcpy(&temp, source.data() + index, size);
        index += size;

        if (temp == 0 || source.size() < index + temp)
        {
            return vector<uint8_t>();
        }

        vector<uint8_t> result;
        result.insert(result.end(), source.begin() + index, source.begin() + index + temp);
        reverse(result.begin(), result.end());
        index += temp;

        return result;
    }
}