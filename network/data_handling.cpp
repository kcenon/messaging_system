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

#include "job.h"
#include "logging.h"
#include "converting.h"
#include "encrypting.h"
#include "compressing.h"
#include "file_handler.h"
#include "constexpr_string.h"

#include <utility>

#include "fmt/xchar.h"
#include "fmt/format.h"

namespace network
{
	using namespace logging;
	using namespace threads;
	using namespace converting;
	using namespace encrypting;
	using namespace compressing;
	using namespace file_handler;

	data_handling::data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value)
		: _key(L""), _iv(L""), _compress_mode(false), _encrypt_mode(false), _received_message(nullptr),
		_thread_pool(nullptr), _received_file(nullptr), _received_data(nullptr), _compress_block_size(1024), 
		_confirm(connection_conditions::waiting)
	{
		memset(_start_code_tag, start_code_value, start_code);
		memset(_end_code_tag, end_code_value, end_code);
		memset(_receiving_buffer, 0, buffer_size);
	}

	data_handling::~data_handling(void)
	{
	}

	void data_handling::read_start_code(weak_ptr<asio::ip::tcp::socket> socket)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		_received_data_vector.clear();

		logger::handle().write(logging_level::parameter, L"attempt to read start code");

		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, start_code),
			[this, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != start_code)
				{
					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}

				for (int index = 0; index < start_code; ++index)
				{
					if (_receiving_buffer[index] == _start_code_tag[index])
					{
						continue;
					}

					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}
				
				logger::handle().write(logging_level::parameter, L"read start code");

				memset(_receiving_buffer, 0, buffer_size);

				read_packet_code(socket);
			});
	}

	void data_handling::read_packet_code(weak_ptr<asio::ip::tcp::socket> socket)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, mode_code),
			[this, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != mode_code)
				{
					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}
				
				logger::handle().write(logging_level::parameter, L"read packet code");

				data_modes mode = (data_modes)_receiving_buffer[0];
				memset(_receiving_buffer, 0, buffer_size);

				read_length_code(mode, socket);
			});
	}

	void data_handling::read_length_code(const data_modes& packet_mode, weak_ptr<asio::ip::tcp::socket> socket)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, length_code),
			[this, packet_mode, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != length_code)
				{
					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}
				
				logger::handle().write(logging_level::parameter, L"read length code");

				unsigned int target_length = 0;
				memcpy(&target_length, _receiving_buffer, length);

				memset(_receiving_buffer, 0, buffer_size);

				read_data(packet_mode, target_length, socket);
			});
	}

	void data_handling::read_data(const data_modes& packet_mode, const size_t& remained_length, weak_ptr<asio::ip::tcp::socket> socket)
	{
		if (remained_length == 0)
		{
			read_end_code(packet_mode, socket);

			return;
		}

		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		if (remained_length >= buffer_size)
		{
			asio::async_read(*current_socket, asio::buffer(_receiving_buffer, buffer_size),
				[this, packet_mode, remained_length, socket](error_code ec, size_t length)
				{
					if (ec)
					{
						disconnected();

						return;
					}

					if (length != buffer_size)
					{
						memset(_receiving_buffer, 0, buffer_size);

						read_start_code(socket);

						return;
					}

					_received_data_vector.insert(_received_data_vector.end(), _receiving_buffer, _receiving_buffer + length);
					memset(_receiving_buffer, 0, buffer_size);

					read_data(packet_mode, remained_length - length, socket);
				});

			current_socket.reset();
			return;
		}

		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, remained_length),
			[this, packet_mode, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}
				
				logger::handle().write(logging_level::parameter, L"read data");

				_received_data_vector.insert(_received_data_vector.end(), _receiving_buffer, _receiving_buffer + length);
				memset(_receiving_buffer, 0, buffer_size);

				read_data(packet_mode, 0, socket);
			});
		current_socket.reset();
	}

	void data_handling::read_end_code(const data_modes& packet_mode, weak_ptr<asio::ip::tcp::socket> socket)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, end_code),
			[this, packet_mode, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != end_code)
				{
					logger::handle().write(logging_level::parameter, L"drop read data");

					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}

				for (int index = 0; index < end_code; ++index)
				{
					if (_receiving_buffer[index] == _end_code_tag[index])
					{
						continue;
					}
				
					logger::handle().write(logging_level::parameter, L"drop read data");

					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}
				
				logger::handle().write(logging_level::parameter, L"read end code");

				receive_on_tcp(packet_mode, _received_data_vector);
				_received_data_vector.clear();

				memset(_receiving_buffer, 0, buffer_size);

				read_start_code(socket);
			});
		current_socket.reset();
	}

	bool data_handling::send_on_tcp(weak_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return false;
		}

		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			disconnected();

			return false;
		}

		size_t sended_size;
		sended_size = current_socket->send(asio::buffer(_start_code_tag, start_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send start code");

			current_socket.reset();
			return false;
		}

		logger::handle().write(logging_level::parameter, L"sent start code");

		sended_size = current_socket->send(asio::buffer(&data_mode, mode_code));
		if (sended_size != sizeof(unsigned char))
		{
			logger::handle().write(logging_level::error, L"cannot send data type code");

			current_socket.reset();
			return false;
		}

		logger::handle().write(logging_level::parameter, L"sent data type code");

		unsigned int length = (unsigned int)data.size();
		sended_size = current_socket->send(asio::buffer(&length, length_code));
		if (sended_size != sizeof(unsigned int))
		{
			logger::handle().write(logging_level::error, L"cannot send length code");

			current_socket.reset();
			return false;
		}

		logger::handle().write(logging_level::parameter, L"sent length code");

		sended_size = current_socket->send(asio::buffer(data.data(), data.size()));
		if (sended_size != data.size())
		{
			logger::handle().write(logging_level::error, L"cannot send data");

			current_socket.reset();
			return false;
		}

		logger::handle().write(logging_level::parameter, L"sent data");

		sended_size = current_socket->send(asio::buffer(_end_code_tag, end_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send end code");

			current_socket.reset();
			return false;
		}

		logger::handle().write(logging_level::parameter, L"sent end code");

		current_socket.reset();

		return true;
	}
	
	void data_handling::receive_on_tcp(const data_modes& data_mode, const vector<unsigned char>& data)
	{
		switch (data_mode)
		{
		case data_modes::packet_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::decompress_packet, this, placeholders::_1)));
			break;
		case data_modes::file_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::decrypt_file_packet, this, placeholders::_1)));
			break;
		default:
			break;
		}
	}

	void data_handling::send_packet_job(const vector<unsigned char>& data)
	{
		if (_encrypt_mode && _confirm == connection_conditions::confirmed)
		{
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::encrypt_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::compress_packet, this, placeholders::_1)));
	}

	void data_handling::send_file_job(const vector<unsigned char>& data)
	{
		_thread_pool->push(make_shared<job>(priorities::low, data, 
				bind(&data_handling::load_file_packet, this, placeholders::_1)));
	}

	void data_handling::send_binary_job(const vector<unsigned char>& data)
	{
		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::compress_binary_packet, this, placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::encrypt_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, data, bind(&data_handling::send_binary_packet, this, placeholders::_1)));
	}

	void data_handling::compress_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, data, bind(&data_handling::send_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to compress a packet");

		_thread_pool->push(make_shared<job>(priorities::low, compressor::compression(data, _compress_block_size), bind(&data_handling::send_packet, this, placeholders::_1)));
	}

	void data_handling::encrypt_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::compress_packet, this, placeholders::_1)));
			
			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to encrypt a packet");	

		_thread_pool->push(make_shared<job>(priorities::normal, encryptor::encryption(data, _key, _iv), bind(&data_handling::compress_packet, this, placeholders::_1)));
	}

	void data_handling::decompress_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_compress_mode || _confirm != connection_conditions::confirmed)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::decrypt_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decompress a packet");
			
		_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data, _compress_block_size), bind(&data_handling::decrypt_packet, this, placeholders::_1)));
	}

	void data_handling::decrypt_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, data, bind(&data_handling::receive_packet, this, placeholders::_1)));	

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decrypt a packet");

		_thread_pool->push(make_shared<job>(priorities::low, encryptor::decryption(data, _key, _iv), bind(&data_handling::receive_packet, this, placeholders::_1)));		
	}

	void data_handling::receive_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_wstring(data)));
#else
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_string(data)));
#endif
#else
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data, true);
#endif
		if (message == nullptr)
		{
			return;
		}

#ifdef __USE_TYPE_CONTAINER__
		logger::handle().write(logging_level::packet, fmt::format(L"received: {}", message->serialize()));
#else
#ifdef _WIN32
		logger::handle().write(logging_level::packet, fmt::format(L"received: {}", message->serialize()));
#else
		logger::handle().write(logging_level::packet, converter::to_wstring(fmt::format("received: {}", message->serialize())));
#endif
#endif

#ifndef __USE_TYPE_CONTAINER__
#ifdef _WIN32
		auto target = _message_handlers.find((*message)[HEADER][MESSAGE_TYPE].as_string());
#else
		auto target = _message_handlers.find(converter::to_wstring((*message)[HEADER][MESSAGE_TYPE].as_string()));
#endif
#else
		auto target = _message_handlers.find(message->message_type());
#endif
		if (target == _message_handlers.end())
		{
			return normal_message(message);
		}

		target->second(message);
	}

	void data_handling::load_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

#ifndef __USE_TYPE_CONTAINER__
		shared_ptr<json::value> message = make_shared<json::value>(json::value::parse(converter::to_string(data)));
#else
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data);
#endif
		if (message == nullptr)
		{
			return;
		}

		vector<unsigned char> result;
#ifndef __USE_TYPE_CONTAINER__
		append_binary_on_packet(result, converter::to_array((*message)[DATA][INDICATION_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][SOURCE_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][SOURCE_SUB_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][TARGET_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[HEADER][TARGET_SUB_ID].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[DATA][SOURCE].as_string()));
		append_binary_on_packet(result, converter::to_array((*message)[DATA][TARGET].as_string()));
#ifdef _WIN32
		append_binary_on_packet(result, file::load((*message)[DATA][SOURCE].as_string()));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"load_file_packet: [{}] => [{}:{}] -> [{}:{}]", (*message)[DATA][INDICATION_ID].as_string(),
				(*message)[HEADER][SOURCE_ID].as_string(), (*message)[HEADER][SOURCE_SUB_ID].as_string(),
				(*message)[HEADER][TARGET_ID].as_string(), (*message)[HEADER][TARGET_SUB_ID].as_string()));
#else
		append_binary_on_packet(result, file::load(converter::to_wstring((*message)[DATA][SOURCE].as_string())));

		logger::handle().write(logging_level::parameter,
			converter::to_wstring(fmt::format("load_file_packet: [{}] => [{}:{}] -> [{}:{}]", (*message)[DATA][INDICATION_ID].as_string(),
				(*message)[HEADER][SOURCE_ID].as_string(), (*message)[HEADER][SOURCE_SUB_ID].as_string(),
				(*message)[HEADER][TARGET_ID].as_string(), (*message)[HEADER][TARGET_SUB_ID].as_string())));
#endif
#else
		append_binary_on_packet(result, converter::to_array(message->get_value(L"indication_id")->to_string()));
		append_binary_on_packet(result, converter::to_array(message->source_id()));
		append_binary_on_packet(result, converter::to_array(message->source_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->target_id()));
		append_binary_on_packet(result, converter::to_array(message->target_sub_id()));
		append_binary_on_packet(result, converter::to_array(message->get_value(L"source")->to_string()));
		append_binary_on_packet(result, converter::to_array(message->get_value(L"target")->to_string()));
		append_binary_on_packet(result, file::load(message->get_value(L"source")->to_string()));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"load_file_packet: [{}] => [{}:{}] -> [{}:{}]", message->get_value(L"indication_id")->to_string(),
				message->source_id(), message->source_sub_id(), message->target_id(), message->target_sub_id()));
#endif

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&data_handling::compress_file_packet, this, placeholders::_1)));

			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&data_handling::encrypt_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, result, bind(&data_handling::send_file_packet, this, placeholders::_1)));
	}

	void data_handling::compress_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::high, compressor::compression(data, _compress_block_size), bind(&data_handling::encrypt_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data, _compress_block_size), bind(&data_handling::send_file_packet, this, placeholders::_1)));
	}

	void data_handling::encrypt_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&data_handling::send_file_packet, this, placeholders::_1)));
	}

	void data_handling::decompress_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, compressor::decompression(data, _compress_block_size), bind(&data_handling::receive_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::low , data, bind(&data_handling::receive_file_packet, this, placeholders::_1)));
	}

	void data_handling::decrypt_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, encryptor::decryption(data, _key, _iv), bind(&data_handling::decompress_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::decompress_file_packet, this, placeholders::_1)));
	}

	void data_handling::receive_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_path = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"receive_file_packet: [{}] => [{}:{}] -> [{}:{}]", source_path, source_id, source_sub_id, target_id, target_sub_id));

		vector<unsigned char> result;
		append_binary_on_packet(result, converter::to_array(indication_id));
		append_binary_on_packet(result, converter::to_array(target_id));
		append_binary_on_packet(result, converter::to_array(target_sub_id));
		if (file::save(target_path, devide_binary_on_packet(data, index)))
		{
			append_binary_on_packet(result, converter::to_array(target_path));
		}
		else
		{
			append_binary_on_packet(result, converter::to_array(L""));
		}

		_thread_pool->push(make_shared<job>(priorities::high, result, bind(&data_handling::notify_file_packet, this, placeholders::_1)));
	}

	void data_handling::notify_file_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		wstring indication_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_path = converter::to_wstring(devide_binary_on_packet(data, index));

		if (_received_file)
		{
			_received_file(target_id, target_sub_id, indication_id, target_path);
		}
	}

	void data_handling::compress_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::compression(data, _compress_block_size), bind(&data_handling::encrypt_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data, _compress_block_size), bind(&data_handling::send_binary_packet, this, placeholders::_1)));
	}

	void data_handling::encrypt_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, encryptor::encryption(data, _key, _iv), bind(&data_handling::send_binary_packet, this, placeholders::_1)));
	}

	void data_handling::decompress_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data, _compress_block_size), bind(&data_handling::receive_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::receive_binary_packet, this, placeholders::_1)));
	}

	void data_handling::decrypt_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::high, encryptor::decryption(data, _key, _iv), bind(&data_handling::decompress_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::decompress_binary_packet, this, placeholders::_1)));
	}

	void data_handling::receive_binary_packet(const vector<unsigned char>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		wstring source_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring source_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_id = converter::to_wstring(devide_binary_on_packet(data, index));
		wstring target_sub_id = converter::to_wstring(devide_binary_on_packet(data, index));
		vector<unsigned char> target_data = devide_binary_on_packet(data, index);
		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, target_data);
		}
	}

	void data_handling::append_binary_on_packet(vector<unsigned char>& result, const vector<unsigned char>& source)
	{
		size_t temp;
		const int size = sizeof(size_t);
		char temp_size[size];

		temp = source.size();
		memcpy(temp_size, &temp, size);
		result.insert(result.end(), temp_size, temp_size + size);
		if (size == 0)
		{
			return;
		}

		result.insert(result.end(), source.begin(), source.end());
	}

	vector<unsigned char> data_handling::devide_binary_on_packet(const vector<unsigned char>& source, size_t& index)
	{
		if (source.empty())
		{
			return vector<unsigned char>();
		}

		size_t temp;
		const int size = sizeof(size_t);

		if (source.size() < index + size)
		{
			return vector<unsigned char>();
		}

		memcpy(&temp, source.data() + index, size);
		index += size;

		if (temp == 0 || source.size() < index + temp)
		{
			return vector<unsigned char>();
		}

		vector<unsigned char> result;
		result.insert(result.end(), source.begin() + index, source.begin() + index + temp);
		index += temp;

		return result;
	}
}