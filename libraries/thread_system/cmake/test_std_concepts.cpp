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

// Test for std::concepts support
#include <concepts>
#include <type_traits>

// Define a custom concept
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// Function using concept
template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// Class template with concept constraint
template<typename T>
    requires std::copyable<T> && std::default_initializable<T>
class Container {
    T value;
public:
    Container() = default;
    Container(T v) : value(v) {}
};

int main() {
    // Test built-in concepts
    static_assert(std::integral<int>);
    static_assert(std::floating_point<double>);
    static_assert(std::copyable<int>);
    static_assert(std::movable<std::string>);
    
    // Test custom concept
    static_assert(Numeric<int>);
    static_assert(Numeric<double>);
    static_assert(!Numeric<std::string>);
    
    // Use constrained function
    auto result = add(1, 2);
    auto result2 = add(1.5, 2.5);
    
    // Use constrained class
    Container<int> c1;
    Container<double> c2(3.14);
    
    return 0;
}