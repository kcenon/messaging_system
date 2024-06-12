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

#include "data_handling.h"

#include "binary_combiner.h"
#include "compressing.h"
#include "constexpr_string.h"
#include "converting.h"
#include "encrypting.h"
#include "file_handler.h"
#include "job.h"
#include "logging.h"

#include <utility>

#include "fmt/format.h"
#include "fmt/xchar.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handler;
	using namespace binary_parser;

	data_handling::data_handling(const unsigned char& start_code_value,
								 const unsigned char& end_code_value)
		: key_("")
		, iv_("")
		, compress_mode_(false)
		, encrypt_mode_(false)
		, received_message_(nullptr)
		, _thread_pool(nullptr)
		, received_file_(nullptr)
		, received_data_(nullptr)
		, compress_block_size_(1024)
		, confirm_(connection_conditions::waiting)
	{
		memset(start_code_tag_, start_code_value, start_code);
		memset(end_code_tag_, end_code_value, end_code);
		memset(receiving_buffer_, 0, buffer_size);
	}

	data_handling::~data_handling(void) {}

	void data_handling::read_start_code(
		std::weak_ptr<asio::ip::tcp::socket> socket,
		const unsigned short& matched_code)
	{
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		if (matched_code == 4)
		{
#ifdef _DEBUG
			logger::handle().write(
				logging_level::packet,
				fmt::format("read start code: {} bytes", start_code));
#endif

			read_packet_code(socket);

			return;
		}

		received_data_vector_.clear();

		memset(receiving_buffer_, 0, buffer_size);
		asio::async_read(
			*current_socket, asio::buffer(receiving_buffer_, 1),
			asio::transfer_exactly(1),
			[this, socket, matched_code](std::error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != 1)
				{
					read_start_code(socket);

					return;
				}

				if (receiving_buffer_[0] != start_code_tag_[matched_code])
				{
					read_start_code(socket);

					return;
				}

				read_start_code(socket, matched_code + 1);
			});
	}

	void data_handling::read_packet_code(
		std::weak_ptr<asio::ip::tcp::socket> socket)
	{
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		memset(receiving_buffer_, 0, buffer_size);
		asio::async_read(
			*current_socket, asio::buffer(receiving_buffer_, mode_code),
			asio::transfer_exactly(mode_code),
			[this, socket](std::error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != mode_code)
				{
					logger::handle().write(
						logging_level::error,
						"drop read data: not matched packet code");

					read_start_code(socket);

					return;
				}

#ifdef _DEBUG
				logger::handle().write(
					logging_level::packet,
					fmt::format("read packet code: {} bytes", mode_code));
#endif

				data_modes mode = (data_modes)receiving_buffer_[0];

				read_length_code(mode, socket);
			});
	}

	void data_handling::read_length_code(
		const data_modes& packet_mode,
		std::weak_ptr<asio::ip::tcp::socket> socket)
	{
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		memset(receiving_buffer_, 0, buffer_size);
		asio::async_read(
			*current_socket, asio::buffer(receiving_buffer_, length_code),
			asio::transfer_exactly(length_code),
			[this, packet_mode, socket](std::error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != length_code)
				{
					logger::handle().write(
						logging_level::error,
						"drop read data: not matched length code");

					read_start_code(socket);

					return;
				}

#ifdef _DEBUG
				logger::handle().write(
					logging_level::packet,
					fmt::format("read length code: {} bytes", length_code));
#endif

				unsigned int target_length = 0;
				memcpy(&target_length, receiving_buffer_, length);

				memset(receiving_buffer_, 0, buffer_size);

				read_data(packet_mode, target_length, socket);
			});
	}

	void data_handling::read_data(const data_modes& packet_mode,
								  const size_t& remained_length,
								  std::weak_ptr<asio::ip::tcp::socket> socket)
	{
		if (remained_length == 0)
		{
			read_end_code(packet_mode, socket);

			return;
		}

		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		if (remained_length >= buffer_size)
		{
			memset(receiving_buffer_, 0, buffer_size);
			asio::async_read(
				*current_socket, asio::buffer(receiving_buffer_, buffer_size),
				asio::transfer_exactly(buffer_size),
				[this, packet_mode, remained_length, socket](std::error_code ec,
															 size_t length)
				{
					if (ec)
					{
						disconnected();

						return;
					}

					if (length != buffer_size)
					{
						logger::handle().write(
							logging_level::error,
							"drop read data: not matched data length");

						read_start_code(socket);

						return;
					}

					received_data_vector_.insert(received_data_vector_.end(),
												 receiving_buffer_,
												 receiving_buffer_ + length);
					memset(receiving_buffer_, 0, buffer_size);

					read_data(packet_mode, remained_length - length, socket);
				});

			return;
		}

		memset(receiving_buffer_, 0, buffer_size);
		asio::async_read(
			*current_socket, asio::buffer(receiving_buffer_, remained_length),
			asio::transfer_exactly(remained_length),
			[this, packet_mode, socket](std::error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				received_data_vector_.insert(received_data_vector_.end(),
											 receiving_buffer_,
											 receiving_buffer_ + length);
				memset(receiving_buffer_, 0, buffer_size);

#ifdef _DEBUG
				logger::handle().write(
					logging_level::packet,
					fmt::format("read data: {} bytes",
								received_data_vector_.size()));
#endif

				read_data(packet_mode, 0, socket);
			});
	}

	void data_handling::read_end_code(
		const data_modes& packet_mode,
		std::weak_ptr<asio::ip::tcp::socket> socket,
		const unsigned short& matched_code)
	{
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		if (matched_code == 4)
		{
#ifdef _DEBUG
			logger::handle().write(
				logging_level::packet,
				fmt::format("read end code: {} bytes", end_code));
#endif

			receive_on_tcp(packet_mode, received_data_vector_);
			received_data_vector_.clear();

			read_start_code(socket);

			return;
		}

		memset(receiving_buffer_, 0, buffer_size);

		asio::async_read(
			*current_socket, asio::buffer(receiving_buffer_, 1),
			asio::transfer_exactly(1),
			[this, packet_mode, socket, matched_code](std::error_code ec,
													  size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != 1
					|| receiving_buffer_[0] != end_code_tag_[matched_code])
				{
					logger::handle().write(
						logging_level::error,
						"drop read data: not matched end code");

					read_start_code(socket);

					return;
				}

				read_end_code(packet_mode, socket, matched_code + 1);
			});
	}

	bool data_handling::send_on_tcp(std::weak_ptr<asio::ip::tcp::socket> socket,
									const data_modes& data_mode,
									const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return false;
		}

		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return false;
		}

		logger::handle().write(
			logging_level::parameter,
			fmt::format("attempt to send message: data mode [{}]",
						(unsigned short)data_mode));

		size_t sended_size;
		sended_size
			= current_socket->send(asio::buffer(start_code_tag_, start_code));
		if (sended_size != 4)
		{
			logger::handle().write(
				logging_level::error,
				fmt::format("cannot send start code: {} bytes", start_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(
			logging_level::packet,
			fmt::format("sent start code: {} bytes", start_code));
#endif

		sended_size = current_socket->send(asio::buffer(&data_mode, mode_code));
		if (sended_size != sizeof(unsigned char))
		{
			logger::handle().write(
				logging_level::error,
				fmt::format("cannot send data type code: {} bytes", mode_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(
			logging_level::packet,
			fmt::format("sent data type code: {} bytes", mode_code));
#endif

		unsigned int length = (unsigned int)data.size();
		sended_size = current_socket->send(asio::buffer(&length, length_code));
		if (sended_size != sizeof(unsigned int))
		{
			logger::handle().write(
				logging_level::error,
				fmt::format("cannot send length code: {} bytes", length_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(
			logging_level::packet,
			fmt::format("sent length code: {} bytes", length_code));
#endif

		size_t temp = 0;
		size_t count = data.size();
		for (size_t index = 0; index < count;)
		{
			temp = count - index;
			if (temp > compress_block_size_)
			{
				temp = compress_block_size_;
			}

			std::vector<uint8_t> temp_buffer(data.begin() + index,
											 data.begin() + index + temp);
			temp = current_socket->send(asio::buffer(temp_buffer.data(), temp));
			if (temp == 0)
			{
				logger::handle().write(
					logging_level::error,
					fmt::format("cannot send data: sent [{}] / total[{}] bytes",
								index, count));

				current_socket.reset();
				return false;
			}

			index += temp;
		}

#ifdef _DEBUG
		logger::handle().write(logging_level::packet,
							   fmt::format("sent data: {} bytes", data.size()));
#endif

		sended_size
			= current_socket->send(asio::buffer(end_code_tag_, end_code));
		if (sended_size != 4)
		{
			logger::handle().write(
				logging_level::error,
				fmt::format("cannot send end code: {} bytes", end_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(
			logging_level::packet,
			fmt::format("sent end code: {} bytes", end_code));
#endif

		logger::handle().write(
			logging_level::parameter,
			fmt::format("completed to send message: data mode [{}]",
						(unsigned short)data_mode));

		current_socket.reset();

		return true;
	}

	void data_handling::receive_on_tcp(const data_modes& data_mode,
									   const std::vector<uint8_t>& data)
	{
		logger::handle().write(logging_level::parameter,
							   fmt::format("received message: data mode [{}]",
										   (unsigned short)data_mode));

		switch (data_mode)
		{
		case data_modes::packet_mode:
			_thread_pool->push(std::make_shared<job>(
				priorities::high, data,
				std::bind(&data_handling::decompress_packet, this,
						  std::placeholders::_1)));
			break;
		case data_modes::file_mode:
			_thread_pool->push(std::make_shared<job>(
				priorities::high, data,
				std::bind(&data_handling::decompress_file_packet, this,
						  std::placeholders::_1)));
			break;
		case data_modes::binary_mode:
			_thread_pool->push(std::make_shared<job>(
				priorities::high, data,
				std::bind(&data_handling::decompress_binary_packet, this,
						  std::placeholders::_1)));
			break;
		default:
			break;
		}
	}

	void data_handling::send_packet_job(const std::vector<uint8_t>& data)
	{
		if (_thread_pool == nullptr)
		{
			return;
		}

		if (confirm_ == connection_conditions::confirmed)
		{
			_thread_pool->push(
				std::make_shared<job>(priorities::high, data,
									  std::bind(&data_handling::encrypt_packet,
												this, std::placeholders::_1)));

			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::normal, data,
								  std::bind(&data_handling::compress_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::send_file_job(const std::vector<uint8_t>& data)
	{
		if (_thread_pool == nullptr)
		{
			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::low, data,
								  std::bind(&data_handling::load_file_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::send_binary_job(const std::vector<uint8_t>& data)
	{
		if (_thread_pool == nullptr)
		{
			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::top, data,
			std::bind(&data_handling::encrypt_binary_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::compress_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!compress_mode_)
		{
			_thread_pool->push(
				std::make_shared<job>(priorities::top, data,
									  std::bind(&data_handling::send_packet,
												this, std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to compress a packet");

		if (specific_compress_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::top, specific_compress_sequence_(data, true),
				std::bind(&data_handling::send_packet, this,
						  std::placeholders::_1)));

			return;
		}

		auto [compressed_data, message]
			= compressor::compression(data, compress_block_size_);
		if (!compressed_data.has_value())
		{
			logger::handle().write(logging_level::error, message);

			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::top, compressed_data.value(),
								  std::bind(&data_handling::send_packet, this,
											std::placeholders::_1)));
	}

	void data_handling::encrypt_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!encrypt_mode_)
		{
			_thread_pool->push(
				std::make_shared<job>(priorities::normal, data,
									  std::bind(&data_handling::compress_packet,
												this, std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to encrypt a packet");

		if (specific_encrypt_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, specific_encrypt_sequence_(data, true),
				std::bind(&data_handling::compress_packet, this,
						  std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::normal, cryptor::encryption(data, key_, iv_),
			std::bind(&data_handling::compress_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::decompress_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!compress_mode_)
		{
			_thread_pool->push(
				std::make_shared<job>(priorities::normal, data,
									  std::bind(&data_handling::decrypt_packet,
												this, std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to decompress a packet");

		if (specific_compress_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, specific_compress_sequence_(data, false),
				std::bind(&data_handling::decrypt_packet, this,
						  std::placeholders::_1)));

			return;
		}

		auto [decompressed_data, message]
			= compressor::decompression(data, compress_block_size_);
		if (!decompressed_data.has_value())
		{
			logger::handle().write(logging_level::error, message);

			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::normal, decompressed_data.value(),
								  std::bind(&data_handling::decrypt_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::decrypt_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!encrypt_mode_ || confirm_ != connection_conditions::confirmed)
		{
			_thread_pool->push(
				std::make_shared<job>(priorities::low, data,
									  std::bind(&data_handling::receive_packet,
												this, std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to decrypt a packet");

		if (specific_encrypt_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low, specific_encrypt_sequence_(data, false),
				std::bind(&data_handling::receive_packet, this,
						  std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::low, cryptor::decryption(data, key_, iv_),
			std::bind(&data_handling::receive_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::receive_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		std::shared_ptr<container::value_container> message
			= std::make_shared<container::value_container>(data, true);
		if (message == nullptr)
		{
			return;
		}

#ifndef _DEBUG
		if (message->message_type() != REQUEST_CONNECTION
			&& message->message_type() != CONFIRM_CONNECTION)
		{
#endif
			logger::handle().write(
				logging_level::packet,
				fmt::format("received: {}", message->serialize()));
#ifndef _DEBUG
		}
#endif

		auto target = message_handlers_.find(message->message_type());
		if (target == message_handlers_.end())
		{
			return normal_message(message);
		}

		target->second(message);
	}

	void data_handling::load_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		std::shared_ptr<container::value_container> message
			= std::make_shared<container::value_container>(data);
		if (message == nullptr)
		{
			return;
		}

		std::vector<uint8_t> result;
		combiner::append(result,
						 converter::to_array(
							 message->get_value("indication_id")->to_string()));
		combiner::append(result, converter::to_array(message->source_id()));
		combiner::append(result, converter::to_array(message->source_sub_id()));
		combiner::append(result, converter::to_array(message->target_id()));
		combiner::append(result, converter::to_array(message->target_sub_id()));
		combiner::append(
			result,
			converter::to_array(message->get_value("source")->to_string()));
		combiner::append(
			result,
			converter::to_array(message->get_value("target")->to_string()));
		combiner::append(result,
						 file::load(message->get_value("source")->to_string()));

		logger::handle().write(
			logging_level::packet,
			fmt::format(
				"send file packet: source[{}:{}] -> target[{}:{}] => {}",
				message->source_id(), message->source_sub_id(),
				message->target_id(), message->target_sub_id(),
				message->get_value("source")->to_string()));

		_thread_pool->push(
			std::make_shared<job>(priorities::normal, result,
								  std::bind(&data_handling::encrypt_file_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::compress_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!compress_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::top, data,
				std::bind(&data_handling::send_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to compress a file packet");

		if (specific_compress_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::top, specific_compress_sequence_(data, true),
				std::bind(&data_handling::send_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		auto [compressed_data, message]
			= compressor::compression(data, compress_block_size_);
		if (!compressed_data.has_value())
		{
			logger::handle().write(logging_level::error, message);

			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::top, compressed_data.value(),
								  std::bind(&data_handling::send_file_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::encrypt_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!encrypt_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::high, data,
				std::bind(&data_handling::compress_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to encrypt a file packet");

		if (specific_encrypt_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::high, specific_encrypt_sequence_(data, true),
				std::bind(&data_handling::compress_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::high, cryptor::encryption(data, key_, iv_),
			std::bind(&data_handling::compress_file_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::decompress_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!compress_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low, data,
				std::bind(&data_handling::decrypt_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to decompress a file packet");

		if (specific_compress_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low, specific_compress_sequence_(data, false),
				std::bind(&data_handling::decrypt_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		auto [decompressed_data, message]
			= compressor::decompression(data, compress_block_size_);
		if (!decompressed_data.has_value())
		{
			logger::handle().write(logging_level::error, message);

			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::low, decompressed_data.value(),
								  std::bind(&data_handling::decrypt_file_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::decrypt_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!encrypt_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, data,
				std::bind(&data_handling::receive_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to decrypt a packet");

		if (specific_encrypt_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, specific_encrypt_sequence_(data, false),
				std::bind(&data_handling::receive_file_packet, this,
						  std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::normal, cryptor::decryption(data, key_, iv_),
			std::bind(&data_handling::receive_file_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::receive_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		std::string indication_id
			= converter::to_string(combiner::divide(data, index));
		std::string source_id
			= converter::to_string(combiner::divide(data, index));
		std::string source_sub_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_sub_id
			= converter::to_string(combiner::divide(data, index));
		std::string source_path
			= converter::to_string(combiner::divide(data, index));
		std::string target_path
			= converter::to_string(combiner::divide(data, index));

		logger::handle().write(
			logging_level::parameter,
			fmt::format("receive_file_packet: [{}] => [{}:{}] -> [{}:{}]",
						source_path, source_id, source_sub_id, target_id,
						target_sub_id));

		std::vector<uint8_t> result;
		combiner::append(result, converter::to_array(indication_id));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		if (file::save(target_path, combiner::divide(data, index)))
		{
			combiner::append(result, converter::to_array(target_path));
		}
		else
		{
			combiner::append(result, converter::to_array(""));
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::high, result,
								  std::bind(&data_handling::notify_file_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::notify_file_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		std::string indication_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_sub_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_path
			= converter::to_string(combiner::divide(data, index));

		logger::handle().write(
			logging_level::packet,
			fmt::format("received file packet: target[{}:{}], "
						"indication_id[{}], target_path[{}]",
						target_id, target_sub_id, indication_id, target_path));

		if (received_file_)
		{
			logger::handle().write(logging_level::parameter,
								   "attempt to transfer a file packet");

			received_file_(target_id, target_sub_id, indication_id,
						   target_path);
		}
	}

	void data_handling::compress_binary_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!compress_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::top, data,
				std::bind(&data_handling::send_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to compress a packet");

		if (specific_compress_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::top, specific_compress_sequence_(data, true),
				std::bind(&data_handling::send_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		auto [compressed_data, message]
			= compressor::compression(data, compress_block_size_);
		if (!compressed_data.has_value())
		{
			logger::handle().write(logging_level::error, message);

			return;
		}

		_thread_pool->push(
			std::make_shared<job>(priorities::top, compressed_data.value(),
								  std::bind(&data_handling::send_binary_packet,
											this, std::placeholders::_1)));
	}

	void data_handling::encrypt_binary_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!encrypt_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, data,
				std::bind(&data_handling::compress_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to encrypt a packet");

		if (specific_encrypt_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, specific_encrypt_sequence_(data, true),
				std::bind(&data_handling::compress_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::normal, cryptor::encryption(data, key_, iv_),
			std::bind(&data_handling::compress_binary_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::decompress_binary_packet(
		const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_thread_pool == nullptr)
		{
			return;
		}

		if (!compress_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, data,
				std::bind(&data_handling::decrypt_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to decompress a binary packet");

		if (specific_compress_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::normal, specific_compress_sequence_(data, false),
				std::bind(&data_handling::decrypt_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		auto [decompressed_data, message]
			= compressor::decompression(data, compress_block_size_);
		if (!decompressed_data.has_value())
		{
			logger::handle().write(logging_level::error, message);

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::normal, decompressed_data.value(),
			std::bind(&data_handling::decrypt_binary_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::decrypt_binary_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!encrypt_mode_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low, data,
				std::bind(&data_handling::receive_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter,
							   "attempt to decrypt a binary packet");

		if (specific_encrypt_sequence_)
		{
			_thread_pool->push(std::make_shared<job>(
				priorities::low, specific_encrypt_sequence_(data, false),
				std::bind(&data_handling::receive_binary_packet, this,
						  std::placeholders::_1)));

			return;
		}

		_thread_pool->push(std::make_shared<job>(
			priorities::low, cryptor::decryption(data, key_, iv_),
			std::bind(&data_handling::receive_binary_packet, this,
					  std::placeholders::_1)));
	}

	void data_handling::receive_binary_packet(const std::vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		std::string source_id
			= converter::to_string(combiner::divide(data, index));
		std::string source_sub_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_id
			= converter::to_string(combiner::divide(data, index));
		std::string target_sub_id
			= converter::to_string(combiner::divide(data, index));
		std::vector<uint8_t> target_data = combiner::divide(data, index);

		logger::handle().write(
			logging_level::packet,
			fmt::format(
				"received binary packet: target_id[{}], target_sub_id[{}], "
				"target_data[{} bytes]",
				target_id, target_sub_id, target_data.size()));

		if (received_data_)
		{
			received_data_(source_id, source_sub_id, target_id, target_sub_id,
						   target_data);
		}
	}
} // namespace network