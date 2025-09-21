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

// ASIO compatibility header for handling cases where ASIO is not available

#ifdef NO_ASIO_AVAILABLE
    // Provide stub implementations when ASIO is not available
    #warning "ASIO not available - network functionality will be limited"

    namespace asio {
        namespace ip {
            class tcp {
            public:
                class socket {};
                class acceptor {};
                class endpoint {};
                class resolver {};
            };
        }

        class io_context {};
        class strand {};

        namespace error {
            const int eof = 1;
            const int connection_reset = 2;
        }

        template<typename T>
        class buffer {
        public:
            buffer(T* data, size_t size) {}
        };
    }

    // Stub error_code
    namespace std {
        class error_code {
        public:
            error_code() : val_(0) {}
            explicit operator bool() const { return val_ != 0; }
            int value() const { return val_; }
            std::string message() const { return "ASIO not available"; }
        private:
            int val_;
        };
    }
#else
    // Include real ASIO headers
    #include <asio.hpp>
#endif