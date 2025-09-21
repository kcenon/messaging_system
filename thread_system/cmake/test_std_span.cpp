/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#include <span>
#include <vector>
#include <array>
#include <algorithm>
#include <numeric>

// Test basic span usage
int test_basic_span() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::span<int> s(vec);
    
    // Test size and empty
    if (s.size() != 5) return 1;
    if (s.empty()) return 1;
    
    // Test element access
    if (s[0] != 1) return 1;
    if (s.front() != 1) return 1;
    if (s.back() != 5) return 1;
    
    return 0;
}

// Test span with different containers
int test_container_span() {
    // Vector
    std::vector<int> vec = {1, 2, 3};
    std::span<int> s1(vec);
    
    // Array
    std::array<int, 3> arr = {4, 5, 6};
    std::span<int> s2(arr);
    
    // C-array
    int c_arr[] = {7, 8, 9};
    std::span<int> s3(c_arr);
    
    return (s1.size() + s2.size() + s3.size() == 9) ? 0 : 1;
}

// Test span subviews
int test_subspan() {
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::span<int> s(vec);
    
    // Test first/last
    auto first3 = s.first(3);
    auto last3 = s.last(3);
    
    if (first3.size() != 3 || first3[0] != 1) return 1;
    if (last3.size() != 3 || last3[2] != 10) return 1;
    
    // Test subspan
    auto middle = s.subspan(2, 5);
    if (middle.size() != 5 || middle[0] != 3) return 1;
    
    return 0;
}

// Test const span
int test_const_span() {
    const std::vector<int> vec = {1, 2, 3};
    std::span<const int> s(vec);
    
    int sum = 0;
    for (const auto& val : s) {
        sum += val;
    }
    
    return (sum == 6) ? 0 : 1;
}

// Test span with algorithms
int test_span_algorithms() {
    std::vector<int> vec = {5, 2, 8, 1, 9};
    std::span<int> s(vec);
    
    // Sort through span
    std::sort(s.begin(), s.end());
    
    // Check if sorted
    if (!std::is_sorted(s.begin(), s.end())) return 1;
    
    // Use accumulate
    int sum = std::accumulate(s.begin(), s.end(), 0);
    if (sum != 25) return 1;
    
    return 0;
}

// Test dynamic extent
int test_dynamic_extent() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // Dynamic extent span
    std::span<int> dynamic_span(vec);
    
    // Fixed extent span
    std::span<int, 5> fixed_span(vec);
    
    return (dynamic_span.size() == fixed_span.size()) ? 0 : 1;
}

int main() {
    if (test_basic_span() != 0) return 1;
    if (test_container_span() != 0) return 1;
    if (test_subspan() != 0) return 1;
    if (test_const_span() != 0) return 1;
    if (test_span_algorithms() != 0) return 1;
    if (test_dynamic_extent() != 0) return 1;
    
    return 0;
}