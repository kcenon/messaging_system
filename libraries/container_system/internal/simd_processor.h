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

#pragma once

#include "container/internal/variant_value.h"
#include "container/internal/thread_safe_container.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <cstring>

// Platform-specific SIMD headers
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #define HAS_X86_SIMD 1
    #if defined(__AVX2__) || defined(HAS_AVX2)
        #ifndef HAS_AVX2
            #define HAS_AVX2 1
        #endif
        #include <immintrin.h>
    #elif defined(__SSE4_2__) || defined(HAS_SSE42)
        #ifndef HAS_SSE42
            #define HAS_SSE42 1
        #endif
        #include <nmmintrin.h>  // SSE4.2
        #include <smmintrin.h>  // SSE4.1
        #include <tmmintrin.h>  // SSSE3
    #elif defined(__SSE2__)
        #define HAS_SSE2 1
        #include <emmintrin.h>  // SSE2
    #endif
    // Basic SSE headers for all x86
    #if defined(HAS_SSE42) || defined(HAS_SSE2)
        #include <xmmintrin.h>  // SSE
        #include <emmintrin.h>  // SSE2
        #include <pmmintrin.h>  // SSE3
    #elif defined(__SSE2__)
        #define HAS_SSE2 1
        #include <emmintrin.h>
    #endif
#elif defined(__ARM_NEON) || defined(__aarch64__)
    #define HAS_ARM_NEON 1
    #include <arm_neon.h>
#endif

namespace container_module
{
namespace simd
{
    /**
     * @brief SIMD width detection
     */
    #if defined(HAS_AVX2) && defined(__x86_64__)
        constexpr size_t float_simd_width = 8;  // 256-bit / 32-bit
        constexpr size_t double_simd_width = 4; // 256-bit / 64-bit
        using float_simd = __m256;
        using double_simd = __m256d;
        using int32_simd = __m256i;
    #elif (defined(HAS_SSE42) || defined(HAS_SSE2)) && (defined(__x86_64__) || defined(__i386__))
        constexpr size_t float_simd_width = 4;  // 128-bit / 32-bit
        constexpr size_t double_simd_width = 2; // 128-bit / 64-bit
        using float_simd = __m128;
        using double_simd = __m128d;
        using int32_simd = __m128i;
    #elif defined(HAS_ARM_NEON) || defined(__ARM_NEON) || defined(__ARM_NEON__)
        constexpr size_t float_simd_width = 4;  // 128-bit / 32-bit
        constexpr size_t double_simd_width = 2; // 128-bit / 64-bit
        using float_simd = float32x4_t;
        using double_simd = float64x2_t;
        using int32_simd = int32x4_t;
    #else
        constexpr size_t float_simd_width = 1;  // No SIMD
        constexpr size_t double_simd_width = 1;
    #endif

    /**
     * @brief SIMD processor for vectorized operations on container values
     */
    class simd_processor
    {
    public:
        /**
         * @brief Sum all float values in a container using SIMD
         */
        static float sum_floats(const std::vector<variant_value>& values);

        /**
         * @brief Sum all double values in a container using SIMD
         */
        static double sum_doubles(const std::vector<variant_value>& values);

        /**
         * @brief Find minimum float value using SIMD
         */
        static std::optional<float> min_float(const std::vector<variant_value>& values);

        /**
         * @brief Find maximum float value using SIMD
         */
        static std::optional<float> max_float(const std::vector<variant_value>& values);

        /**
         * @brief Compute average of numeric values
         */
        template<typename T>
        static std::optional<double> average(const std::vector<variant_value>& values);

        /**
         * @brief Vectorized comparison - find all values equal to target
         */
        static std::vector<size_t> find_equal_floats(
            const std::vector<variant_value>& values,
            float target);

        /**
         * @brief Vectorized string search using SIMD
         */
        static std::vector<size_t> find_string_pattern(
            const std::vector<variant_value>& values,
            std::string_view pattern);

        /**
         * @brief Transform all numeric values by applying a function
         */
        template<typename T, typename Func>
        static void transform_numeric(std::vector<variant_value>& values, Func&& func);

        /**
         * @brief Parallel dot product of two float arrays
         */
        static std::optional<float> dot_product_floats(
            const std::vector<variant_value>& a,
            const std::vector<variant_value>& b);

        /**
         * @brief Fast memory copy using SIMD
         */
        static void fast_copy(const void* src, void* dst, size_t size);

        /**
         * @brief Fast memory comparison using SIMD
         */
        static bool fast_compare(const void* a, const void* b, size_t size);

        /**
         * @brief Serialize multiple values in parallel
         */
        static std::vector<std::vector<uint8_t>> parallel_serialize(
            const std::vector<variant_value>& values);

        /**
         * @brief Compute hash of data using SIMD
         */
        static uint64_t simd_hash(const void* data, size_t size);

    private:
        // Platform-specific implementations
        #if defined(HAS_AVX2)
        static float sum_floats_avx2(const float* data, size_t count);
        static float min_float_avx2(const float* data, size_t count);
        static float max_float_avx2(const float* data, size_t count);
        #endif

        #if defined(HAS_SSE42) || defined(HAS_SSE2)
        static float sum_floats_sse(const float* data, size_t count);
        static float min_float_sse(const float* data, size_t count);
        static float max_float_sse(const float* data, size_t count);
        #endif

        #if defined(HAS_ARM_NEON)
        static float sum_floats_neon(const float* data, size_t count);
        static float min_float_neon(const float* data, size_t count);
        static float max_float_neon(const float* data, size_t count);
        #endif

        // Scalar fallbacks
        static float sum_floats_scalar(const float* data, size_t count);
        static float min_float_scalar(const float* data, size_t count);
        static float max_float_scalar(const float* data, size_t count);
    };

    /**
     * @brief SIMD-accelerated data compressor
     */
    class simd_compressor
    {
    public:
        /**
         * @brief Compress data using SIMD-accelerated algorithm
         */
        static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);

        /**
         * @brief Decompress data using SIMD-accelerated algorithm
         */
        static std::vector<uint8_t> decompress(const std::vector<uint8_t>& compressed);

        /**
         * @brief Check if data is compressible (entropy estimation)
         */
        static bool is_compressible(const std::vector<uint8_t>& data);
    };

    /**
     * @brief Utility to check SIMD support at runtime
     */
    class simd_support
    {
    public:
        static bool has_sse2();
        static bool has_sse42();
        static bool has_avx2();
        static bool has_neon();
        
        /**
         * @brief Get a string describing available SIMD features
         */
        static std::string get_simd_info();
        
        /**
         * @brief Get the optimal SIMD width for current platform
         */
        static size_t get_optimal_width() {
            #if defined(HAS_AVX2)
                return 8;
            #elif defined(HAS_SSE42) || defined(HAS_SSE2) || defined(HAS_ARM_NEON)
                return 4;
            #else
                return 1;
            #endif
        }
    };

    /**
     * @brief Template for SIMD operations on different types
     */
    template<typename T>
    struct simd_traits {
        static constexpr size_t width = 1;
        static constexpr bool supported = false;
    };

    #if defined(HAS_X86_SIMD) || defined(HAS_ARM_NEON)
    template<>
    struct simd_traits<float> {
        static constexpr size_t width = float_simd_width;
        static constexpr bool supported = true;
    };

    template<>
    struct simd_traits<double> {
        static constexpr size_t width = double_simd_width;
        static constexpr bool supported = true;
    };

    template<>
    struct simd_traits<int32_t> {
        static constexpr size_t width = float_simd_width; // Same as float
        static constexpr bool supported = true;
    };
    #endif

} // namespace simd
} // namespace container_module