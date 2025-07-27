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

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <vector>

#ifdef USE_STD_SPAN
#include <span>
#endif

namespace utility_module
{
#ifdef USE_STD_SPAN
    /**
     * @brief Using the standard library implementation of span when available.
     * 
     * This provides a non-owning view into a contiguous sequence of objects,
     * conforming to C++20's std::span.
     */
    template <typename T, size_t Extent = std::dynamic_extent>
    using span = std::span<T, Extent>;
#else
    /**
     * @brief A fallback span implementation for C++17 and earlier compilers.
     * 
     * Provides a view over a contiguous sequence of objects, similar to C++20's std::span.
     * This implementation offers a compatible subset of the std::span interface.
     * 
     * @tparam T The type of elements in the span.
     * @tparam Extent The extent (size) of the span, or dynamic_extent if determined at runtime.
     */
    template <typename T, size_t Extent = size_t(-1)>
    class span
    {
    public:
        // Member types
        using element_type = T;
        using value_type = std::remove_cv_t<T>;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        // Dynamic extent constant
        static constexpr size_t dynamic_extent = size_t(-1);

        // Constructors
        constexpr span() noexcept : data_(nullptr), size_(0) 
        {
            static_assert(Extent == dynamic_extent || Extent == 0, 
                        "Default-constructed span must have dynamic extent or zero extent");
        }

        constexpr span(pointer ptr, size_type count) noexcept : data_(ptr), size_(count) 
        {
            static_assert(Extent == dynamic_extent || Extent == count, 
                        "Size mismatch in span construction");
        }

        constexpr span(pointer first, pointer last) noexcept : data_(first), size_(last - first) 
        {
            static_assert(Extent == dynamic_extent || Extent == size_, 
                        "Size mismatch in span construction");
        }

        // Constructor from std::array
        template <size_t N>
        constexpr span(std::array<value_type, N>& arr) noexcept
            : data_(arr.data()), size_(N)
        {
            static_assert(Extent == dynamic_extent || Extent == N, 
                        "Size mismatch in span construction from array");
        }

        template <size_t N>
        constexpr span(const std::array<value_type, N>& arr) noexcept
            : data_(arr.data()), size_(N)
        {
            static_assert(Extent == dynamic_extent || Extent == N, 
                        "Size mismatch in span construction from const array");
        }

        // Constructor from std::vector
        template <typename Allocator>
        constexpr span(std::vector<value_type, Allocator>& vec) noexcept
            : data_(vec.data()), size_(vec.size())
        {
            static_assert(Extent == dynamic_extent || Extent == 0, 
                        "Non-dynamic extent not supported for std::vector constructor");
        }

        template <typename Allocator>
        constexpr span(const std::vector<value_type, Allocator>& vec) noexcept
            : data_(vec.data()), size_(vec.size())
        {
            static_assert(Extent == dynamic_extent || Extent == 0, 
                        "Non-dynamic extent not supported for std::vector constructor");
        }

        // Constructors from C-style arrays
        template <size_t N>
        constexpr span(element_type (&arr)[N]) noexcept
            : data_(arr), size_(N)
        {
            static_assert(Extent == dynamic_extent || Extent == N, 
                        "Size mismatch in span construction from array");
        }

        // Assignment operators
        constexpr span& operator=(const span& other) noexcept = default;

        // Observer methods
        [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
        [[nodiscard]] constexpr size_type size_bytes() const noexcept { return size_ * sizeof(element_type); }
        [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

        // Element access
        [[nodiscard]] constexpr reference operator[](size_type idx) const noexcept
        {
            return data_[idx];
        }

        [[nodiscard]] constexpr reference front() const noexcept
        {
            return data_[0];
        }

        [[nodiscard]] constexpr reference back() const noexcept
        {
            return data_[size_ - 1];
        }

        [[nodiscard]] constexpr pointer data() const noexcept { return data_; }

        // Iterator support
        [[nodiscard]] constexpr iterator begin() const noexcept { return data_; }
        [[nodiscard]] constexpr iterator end() const noexcept { return data_ + size_; }
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data_; }
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return data_ + size_; }
        [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
        [[nodiscard]] constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

        // Subviews
        template<size_t Count>
        [[nodiscard]] constexpr span<element_type, Count> first() const noexcept
        {
            return span<element_type, Count>(data_, Count);
        }

        [[nodiscard]] constexpr span<element_type, dynamic_extent> first(size_type count) const noexcept
        {
            return span<element_type, dynamic_extent>(data_, count);
        }

        template<size_t Count>
        [[nodiscard]] constexpr span<element_type, Count> last() const noexcept
        {
            return span<element_type, Count>(data_ + (size_ - Count), Count);
        }

        [[nodiscard]] constexpr span<element_type, dynamic_extent> last(size_type count) const noexcept
        {
            return span<element_type, dynamic_extent>(data_ + (size_ - count), count);
        }

        template<size_t Offset, size_t Count = dynamic_extent>
        [[nodiscard]] constexpr auto subspan() const noexcept 
        {
            constexpr size_t new_extent = Count != dynamic_extent ? Count : 
                                        (Extent != dynamic_extent ? Extent - Offset : dynamic_extent);
            return span<element_type, new_extent>(
                data_ + Offset, 
                Count != dynamic_extent ? Count : (size_ - Offset));
        }

        [[nodiscard]] constexpr span<element_type, dynamic_extent> 
        subspan(size_type offset, size_type count = dynamic_extent) const noexcept
        {
            size_type new_count = count == dynamic_extent ? size_ - offset : count;
            return span<element_type, dynamic_extent>(data_ + offset, new_count);
        }

    private:
        pointer data_;
        size_type size_;
    };

    // Deduction guides for span
    template <class T, size_t N>
    span(T (&)[N]) -> span<T, N>;

    template <class T, size_t N>
    span(std::array<T, N>&) -> span<T, N>;

    template <class T, size_t N>
    span(const std::array<T, N>&) -> span<const T, N>;

    template <class Container>
    span(Container&) -> span<typename Container::value_type>;

    template <class Container>
    span(const Container&) -> span<const typename Container::value_type>;
#endif
} // namespace utility_module