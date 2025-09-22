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

#include <string>
#include <memory>
#include <functional>

#ifdef NO_ASIO_AVAILABLE
    // Provide stub implementations when ASIO is not available
    #ifdef _MSC_VER
        #pragma message("ASIO not available - network functionality will be limited")
    #else
        #warning "ASIO not available - network functionality will be limited"
    #endif

    #include <system_error>
    #include <thread>
    #include <future>
    #include <tuple>
    #if __has_include(<coroutine>)
        #include <coroutine>
    #endif

    namespace asio {
        // Forward declarations
        class io_context;
        using error_code = std::error_code;

        namespace ip {
            class tcp {
            public:
                // Forward declare endpoint before use
                class endpoint;
                class socket;
                class acceptor;
                class resolver;
                class socket {
                public:
                    socket() = default;
                    socket(io_context&) {}
                    socket(socket&&) = default;
                    socket& operator=(socket&&) = default;

                    void close() {}
                    void close(std::error_code& ec) { ec = std::error_code(); }
                    bool is_open() const { return false; }

                    // Executor type for compatibility
                    struct executor_type {
                        bool operator==(const executor_type&) const { return true; }
                        bool operator!=(const executor_type&) const { return false; }
                    };

                    executor_type get_executor() const { return executor_type{}; }

                    template<typename Buffer, typename Handler>
                    void async_read_some(const Buffer&, Handler handler) {
                        // Invoke handler with error
                        handler(std::make_error_code(std::errc::not_supported), 0);
                    }
                };

                class acceptor {
                public:
                    acceptor() = default;
                    acceptor(io_context&) {}
                    acceptor(io_context&, const endpoint&) {}

                    bool is_open() const { return false; }
                    void close() {}
                    void close(std::error_code& ec) { ec = std::error_code(); }
                    void listen(int = 0) {}

                    template<typename Handler>
                    void async_accept(Handler handler) {
                        // Invoke handler with error
                        handler(std::make_error_code(std::errc::not_supported), socket{});
                    }

                    template<typename Handler>
                    void async_accept(socket&, Handler handler) {
                        // Invoke handler with error
                        handler(std::make_error_code(std::errc::not_supported));
                    }
                };

                class endpoint {
                public:
                    endpoint() = default;
                    endpoint(const endpoint&, unsigned short) {}  // Constructor accepting protocol and port
                    endpoint(const std::string&, unsigned short) {}  // Constructor accepting address and port
                    unsigned short port() const { return 0; }
                };

                // Static factory methods for protocol versions
                static endpoint v4() { return endpoint{}; }
                static endpoint v6() { return endpoint{}; }

                class resolver {
                public:
                    resolver(io_context&) {}

                    class results_type {
                    public:
                        class iterator {
                        public:
                            endpoint operator*() const { return endpoint{}; }
                            iterator& operator++() { return *this; }
                            bool operator!=(const iterator&) const { return false; }
                        };

                        iterator begin() const { return iterator{}; }
                        iterator end() const { return iterator{}; }
                    };

                    template<typename Handler>
                    void async_resolve(const std::string&, const std::string&, Handler handler) {
                        handler(std::make_error_code(std::errc::not_supported), results_type{});
                    }
                };
            };
        }

        class io_context {
        public:
            using executor_type = ip::tcp::socket::executor_type;

            io_context() = default;

            std::size_t run() { return 0; }
            std::size_t run(std::error_code& ec) {
                ec = std::error_code();
                return 0;
            }

            void stop() {}

            executor_type get_executor() const { return executor_type{}; }
        };

        class strand {};

        namespace error {
            const int eof = 1;
            const int connection_reset = 2;
        }

        // Buffer types
        template<typename T>
        class mutable_buffer {
        public:
            mutable_buffer(T* data, size_t size) : data_(data), size_(size) {}
            T* data() const { return data_; }
            size_t size() const { return size_; }
        private:
            T* data_;
            size_t size_;
        };

        template<typename T>
        class const_buffer {
        public:
            const_buffer(const T* data, size_t size) : data_(data), size_(size) {}
            const T* data() const { return data_; }
            size_t size() const { return size_; }
        private:
            const T* data_;
            size_t size_;
        };

        template<typename T>
        auto buffer(T* data, size_t size) {
            return mutable_buffer<T>(data, size);
        }

        template<typename T>
        auto buffer(const T* data, size_t size) {
            return const_buffer<T>(data, size);
        }

        template<typename Container>
        auto buffer(const Container& c) {
            return const_buffer<typename Container::value_type>(c.data(), c.size());
        }

        template<typename Container>
        auto buffer(Container& c) {
            return mutable_buffer<typename Container::value_type>(c.data(), c.size());
        }

        // Async operations
        template<typename Socket, typename Endpoints, typename Handler>
        void async_connect(Socket& socket, const Endpoints& endpoints, Handler handler) {
            // Immediately invoke handler with error - pass an endpoint not an iterator
            handler(std::make_error_code(std::errc::not_supported), ip::tcp::endpoint{});
        }

        template<typename Socket, typename Buffer, typename Handler>
        void async_write(Socket& socket, const Buffer& buffer, Handler handler) {
            // Immediately invoke handler with error
            handler(std::make_error_code(std::errc::not_supported), 0);
        }

        // Awaitable support (minimal stubs)
        template<typename T = void>
        class awaitable {
        public:
            using value_type = T;

            #if __has_include(<coroutine>)
            // Minimal coroutine support for compilation
            struct promise_type {
                T value;

                awaitable get_return_object() { return {}; }
                std::suspend_never initial_suspend() noexcept { return {}; }
                std::suspend_never final_suspend() noexcept { return {}; }
                void return_value(T v) { value = v; }
                void unhandled_exception() {}
            };
            #endif
        };

        // Specialization for void
        template<>
        class awaitable<void> {
        public:
            using value_type = void;

            #if __has_include(<coroutine>)
            struct promise_type {
                awaitable get_return_object() { return {}; }
                std::suspend_never initial_suspend() noexcept { return {}; }
                std::suspend_never final_suspend() noexcept { return {}; }
                void return_void() {}
                void unhandled_exception() {}
            };
            #endif
        };

        struct use_awaitable_t {};
        constexpr use_awaitable_t use_awaitable{};

        namespace experimental {
            template<typename... Ts>
            struct as_tuple_t {
                template<typename T>
                struct awaitable_handler {
                    using result_type = std::tuple<std::error_code, size_t>;
                };
            };

            template<typename T>
            auto as_tuple(T) { return as_tuple_t<T>{}; }
        }

        // co_spawn stub
        template<typename Executor, typename F, typename Handler>
        void co_spawn(const Executor&, F f, Handler handler) {
            // Just invoke the handler with success
            handler(std::error_code{});
        }
    }

    // Use standard error_code from system_error header
    using asio::error_code;
#else
    // Include real ASIO headers
    #include <asio.hpp>
#endif