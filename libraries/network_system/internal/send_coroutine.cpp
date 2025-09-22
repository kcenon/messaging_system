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

#include "network_system/internal/send_coroutine.h"
#include "network_system/integration/logger_integration.h"

#include <future>
#include <thread>
#include <functional>
#include <type_traits>

// Use nested namespace definition (C++17)
namespace network_module
{
    auto prepare_data_async(const std::vector<uint8_t>& input_data,
                            const pipeline& pl,
                            bool use_compress,
                            bool use_encrypt)
        -> std::future<std::vector<uint8_t>>
    {
        return std::async(std::launch::async, [input_data = input_data, &pl, use_compress, use_encrypt]() {
            std::vector<uint8_t> processed_data = input_data;
            
            if constexpr (std::is_invocable_r_v<std::vector<uint8_t>, decltype(pl.compress), const std::vector<uint8_t>&>) {
                if (use_compress) {
                    processed_data = pl.compress(processed_data);
                }
            }
            
            if constexpr (std::is_invocable_r_v<std::vector<uint8_t>, decltype(pl.encrypt), const std::vector<uint8_t>&>) {
                if (use_encrypt) {
                    processed_data = pl.encrypt(processed_data);
                }
            }
            
            return processed_data;
        });
    }

#ifdef USE_STD_COROUTINE

    auto async_send_with_pipeline_co(std::shared_ptr<tcp_socket> sock,
                                     std::vector<uint8_t> data,
                                     const pipeline& pl,
                                     bool use_compress,
                                     bool use_encrypt)
        -> asio::awaitable<std::error_code>
    {
        // Process data with pipeline (compress/encrypt as needed)
        auto future_processed = prepare_data_async(data, pl, use_compress, use_encrypt);
        auto processed_data = future_processed.get(); // This blocks, but we're in a coroutine
        
        // Using structured binding directly with co_await
        auto [ec, bytes_transferred] = co_await asio::async_write(
            sock->socket(),
            asio::buffer(processed_data),
            asio::experimental::as_tuple(asio::use_awaitable));
        
        if (ec) {
            NETWORK_LOG_ERROR("[send_coroutine] Error sending data: " + ec.message());
        }
        
        co_return ec;
    }

#else // fallback

    auto async_send_with_pipeline_no_co(std::shared_ptr<tcp_socket> sock,
                                        std::vector<uint8_t> data,
                                        const pipeline& pl,
                                        bool use_compress,
                                        bool use_encrypt)
        -> std::future<std::error_code>
    {
        // Create a promise for the final result
        auto promise = std::make_shared<std::promise<std::error_code>>();
        auto future_result = promise->get_future();
        
        // Process data in a separate thread
        auto future_processed = prepare_data_async(std::move(data), pl, use_compress, use_encrypt);
        
        // When processing is done, send the data
        std::thread([sock, promise, future = std::move(future_processed)]() mutable {
            try {
                auto processed_data = future.get();
                
                // Perform async write and capture result in the promise
                sock->async_send(processed_data, 
                    [promise](std::error_code ec, std::size_t /*bytes_transferred*/) {
                        promise->set_value(ec);
                    });
            }
            catch (const std::exception& e) {
                NETWORK_LOG_ERROR("[send_coroutine] Exception processing data: "
                          + std::string(e.what()));
                promise->set_value(std::make_error_code(std::errc::io_error));
            }
        }).detach();
        
        return future_result;
    }

#endif

} // namespace network_module