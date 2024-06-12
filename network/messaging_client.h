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

#include "constexpr_string.h"
#include "container.h"
#include "data_handling.h"
#include "session_types.h"
#include "thread_pool.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "asio.hpp"
#include <thread>

namespace network
{

	class messaging_client
		: public std::enable_shared_from_this<messaging_client>,
		  data_handling
	{
	public:
		messaging_client(const std::string& source_id,
						 const unsigned char& start_code_value = 231,
						 const unsigned char& end_code_value = 67);
		~messaging_client(void);

	public:
		std::shared_ptr<messaging_client> get_ptr(void);

	public:
		std::string source_id(void) const;
		std::string source_sub_id(void) const;

	public:
		void set_auto_echo(const bool& auto_echo,
						   const unsigned short& echo_interval);
		void set_bridge_line(const bool& bridge_line);
		void set_encrypt_mode(const bool& encrypt_mode);
		void set_compress_mode(const bool& compress_mode);
		void set_compress_block_size(const unsigned short& compress_block_size);
		void set_session_types(const session_types& session_type);
		void set_connection_key(const std::string& connection_key);
		void set_snipping_targets(
			const std::vector<std::string>& snipping_targets);

	public:
		void set_connection_notification(
			const std::function<void(const std::string&,
									 const std::string&,
									 const bool&)>& notification);
		void set_message_notification(
			const std::function<void(
				std::shared_ptr<container::value_container>)>& notification);
		void set_file_notification(
			const std::function<void(const std::string&,
									 const std::string&,
									 const std::string&,
									 const std::string&)>& notification);
		void set_binary_notification(
			const std::function<void(const std::string&,
									 const std::string&,
									 const std::string&,
									 const std::string&,
									 const std::vector<uint8_t>&)>&
				notification);
		void set_specific_compress_sequence(
			const std::function<
				std::vector<uint8_t>(const std::vector<uint8_t>&, const bool&)>&
				specific_compress_sequence);
		void set_specific_encrypt_sequence(
			const std::function<
				std::vector<uint8_t>(const std::vector<uint8_t>&, const bool&)>&
				specific_encrypt_sequence);

	public:
		connection_conditions get_confirm_status(void) const;
		void start(const std::string& ip,
				   const unsigned short& port,
				   const unsigned short& high_priority = 8,
				   const unsigned short& normal_priority = 8,
				   const unsigned short& low_priority = 8);
		void stop(void);

	public:
		bool echo(void);
		bool send(const container::value_container& message);
		bool send(std::shared_ptr<container::value_container> message);
		bool send_files(const container::value_container& message);
		bool send_files(std::shared_ptr<container::value_container> message);
		bool send_binary(const std::string& target_id,
						 const std::string& target_sub_id,
						 const std::vector<uint8_t>& data);

	protected:
		void send_connection(void);
		void disconnected(void) override;

	private:
		void send_packet(const std::vector<uint8_t>& data) override;
		void send_file_packet(const std::vector<uint8_t>& data) override;
		void send_binary_packet(const std::vector<uint8_t>& data) override;

	private:
		void normal_message(
			std::shared_ptr<container::value_container> message) override;
		void confirm_message(
			std::shared_ptr<container::value_container> message);
		void request_files(std::shared_ptr<container::value_container> message);
		void echo_message(std::shared_ptr<container::value_container> message);

	private:
		void connection_notification(const bool& condition);

	private:
		bool create_socket(const std::string& ip, const unsigned short& port);
		void run(void);
		void create_thread_pool(const unsigned short& high_priority,
								const unsigned short& normal_priority,
								const unsigned short& low_priority);

	private:
		bool auto_echo_;
		bool bridge_line_;
		session_types session_type_;
		std::string source_id_;
		std::string source_sub_id_;
		std::string target_id_;
		std::string target_sub_id_;
		std::string connection_key_;
		unsigned short auto_echo_interval_seconds_;
		std::vector<std::string> snipping_targets_;

	private:
		std::function<void(const std::string&, const std::string&, const bool&)>
			connection_;

	private:
		std::shared_ptr<std::thread> thread_;
		std::shared_ptr<asio::ip::tcp::socket> socket_;
		std::shared_ptr<asio::io_context> io_context_;
	};
} // namespace network
