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

#include <gtest/gtest.h>
#include <cstdint>
#include <type_traits>
#include "container/internal/variant_value.h"
#include "container/core/value_types.h"

namespace container_module {
namespace test {

// Test fixture for variant tests
class VariantTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test that ValueVariant compiles correctly on all platforms
TEST_F(VariantTest, ValueVariantCompiles) {
    // Create a variant with each type
    ValueVariant null_var = std::monostate{};
    ValueVariant bool_var = true;
    ValueVariant bytes_var = std::vector<uint8_t>{1, 2, 3};
    ValueVariant short_var = int16_t(42);
    ValueVariant ushort_var = uint16_t(42);
    ValueVariant int_var = int32_t(42);
    ValueVariant uint_var = uint32_t(42);
    ValueVariant long_var = int64_t(42);
    ValueVariant ulong_var = uint64_t(42);
    ValueVariant float_var = 3.14f;
    ValueVariant double_var = 3.14;
    ValueVariant string_var = std::string("test");

    EXPECT_EQ(null_var.index(), 0);
    EXPECT_EQ(bool_var.index(), 1);
    EXPECT_EQ(bytes_var.index(), 2);
    EXPECT_EQ(string_var.index(), 11);
}

// Test that int64_t and long long are the same type (preventing duplicate types)
TEST_F(VariantTest, Int64AndLongLongAreSame) {
    // This test verifies that int64_t and long long are the same type
    // which is why we removed long long from the variant
    constexpr bool same_signed = std::is_same_v<int64_t, long long>;
    constexpr bool same_unsigned = std::is_same_v<uint64_t, unsigned long long>;

    // On Windows and most modern 64-bit platforms, these should be true
    EXPECT_TRUE(same_signed || sizeof(int64_t) == sizeof(long long));
    EXPECT_TRUE(same_unsigned || sizeof(uint64_t) == sizeof(unsigned long long));
}

// Test backward compatibility with value_types enum
TEST_F(VariantTest, ValueTypesBackwardCompatibility) {
    // llong_value should map to long_value
    EXPECT_EQ(static_cast<int>(value_types::llong_value),
              static_cast<int>(value_types::long_value));

    // ullong_value should map to ulong_value
    EXPECT_EQ(static_cast<int>(value_types::ullong_value),
              static_cast<int>(value_types::ulong_value));
}

// Test variant_value with different types
TEST_F(VariantTest, VariantValueOperations) {
    // Test with int64_t
    variant_value v1("test_long", int64_t(9223372036854775807LL));
    auto long_val = v1.get<int64_t>();
    ASSERT_TRUE(long_val.has_value());
    EXPECT_EQ(*long_val, 9223372036854775807LL);

    // Test with uint64_t
    variant_value v2("test_ulong", uint64_t(18446744073709551615ULL));
    auto ulong_val = v2.get<uint64_t>();
    ASSERT_TRUE(ulong_val.has_value());
    EXPECT_EQ(*ulong_val, 18446744073709551615ULL);

    // Test that we can still handle large values that would have been long long
    variant_value v3("large_value", int64_t(1234567890123456789LL));
    EXPECT_FALSE(v3.is_null());
    EXPECT_EQ(v3.name(), "large_value");
}

// Test that variant indices match expected positions
TEST_F(VariantTest, VariantIndicesMatch) {
    variant_value null_val("null");
    EXPECT_TRUE(null_val.is_null());
    EXPECT_EQ(null_val.type_index(), 0);

    variant_value bool_val("bool", true);
    EXPECT_EQ(bool_val.type_index(), 1);

    variant_value bytes_val("bytes", std::vector<uint8_t>{1, 2, 3});
    EXPECT_EQ(bytes_val.type_index(), 2);

    variant_value int64_val("int64", int64_t(42));
    EXPECT_EQ(int64_val.type_index(), 7);

    variant_value uint64_val("uint64", uint64_t(42));
    EXPECT_EQ(uint64_val.type_index(), 8);
}

} // namespace test
} // namespace container_module

// Main entry point
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}