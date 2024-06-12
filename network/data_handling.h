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

#pragma once

#include "connection_conditions.h"
#include "data_lengths.h"
#include "data_modes.h"
#include "thread_pool.h"

#include <functional>
#include <map>
#include <memory>
#include <system_error>
#include <vector>

#include "asio.hpp"

#include "container.h"

namespace network
{

	class data_handling
	{
	public:
		data_handling(const unsigned char& start_code_value,
					  const unsigned char& end_code_value);
		~data_handling(void);

	protected:
		void read_start_code(std::weak_ptr<asio::ip::tcp::socket> socket,
							 const unsigned short& matched_code = 0);
		void read_packet_code(std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_length_code(const data_modes& packet_mode,
							  std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_data(const data_modes& packet_mode,
					   const size_t& remained_length,
					   std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_end_code(const data_modes& packet_mode,
						   std::weak_ptr<asio::ip::tcp::socket> socket,
						   const unsigned short& matched_code = 0);

	protected:
		bool send_on_tcp(std::weak_ptr<asio::ip::tcp::socket> socket,
						 const data_modes& data_mode,
						 const std::vector<uint8_t>& data);
		void receive_on_tcp(const data_modes& data_mode,
							const std::vector<uint8_t>& data);

	protected:
		virtual void disconnected(void) = 0;
		virtual void normal_message(
			std::shared_ptr<container::value_container> message)
			= 0;

	protected:
		void send_packet_job(const std::vector<uint8_t>& data);
		void send_file_job(const std::vector<uint8_t>& data);
		void send_binary_job(const std::vector<uint8_t>& data);

		// packet
	protected:
		void compress_packet(const std::vector<uint8_t>& data);
		void encrypt_packet(const std::vector<uint8_t>& data);
		virtual void send_packet(const std::vector<uint8_t>& data) = 0;

	protected:
		void decompress_packet(const std::vector<uint8_t>& data);
		void decrypt_packet(const std::vector<uint8_t>& data);
		void receive_packet(const std::vector<uint8_t>& data);

		// file
	protected:
		void load_file_packet(const std::vector<uint8_t>& data);
		void compress_file_packet(const std::vector<uint8_t>& data);
		void encrypt_file_packet(const std::vector<uint8_t>& data);
		virtual void send_file_packet(const std::vector<uint8_t>& data) = 0;

	protected:
		void decompress_file_packet(const std::vector<uint8_t>& data);
		void decrypt_file_packet(const std::vector<uint8_t>& data);
		void receive_file_packet(const std::vector<uint8_t>& data);
		void notify_file_packet(const std::vector<uint8_t>& data);

		// binary
	protected:
		void compress_binary_packet(const std::vector<uint8_t>& data);
		void encrypt_binary_packet(const std::vector<uint8_t>& data);
		virtual void send_binary_packet(const std::vector<uint8_t>& data) = 0;

	protected:
		void decompress_binary_packet(const std::vector<uint8_t>& data);
		void decrypt_binary_packet(const std::vector<uint8_t>& data);
		void receive_binary_packet(const std::vector<uint8_t>& data);

	protected:
		std::function<void(std::shared_ptr<container::value_container>)>
			received_message_;
		std::function<void(const std::string&,
						   const std::string&,
						   const std::string&,
						   const std::string&)>
			received_file_;
		std::function<void(const std::string&,
						   const std::string&,
						   const std::string&,
						   const std::string&,
						   const std::vector<uint8_t>&)>
			received_data_;

	protected:
		std::shared_ptr<threads::thread_pool> _thread_pool;
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
										   const bool&)>
			specific_compress_sequence_;
		std::function<std::vector<uint8_t>(const std::vector<uint8_t>&,
										   const bool&)>
			specific_encrypt_sequence_;
		std::map<
			std::string,
			std::function<void(std::shared_ptr<container::value_container>)>>
			message_handlers_;

	protected:
		connection_conditions confirm_;
		bool compress_mode_;
		unsigned short compress_block_size_;

		bool encrypt_mode_;
		std::string key_;
		std::string iv_;

	private:
		char start_code_tag_[start_code];
		char end_code_tag_[end_code];
		char receiving_buffer_[buffer_size];
		std::vector<uint8_t> received_data_vector_;
	};
} // namespace network