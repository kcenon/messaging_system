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
#include "binary_combiner.h"
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
	using namespace binary_parser;

	data_handling::data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value)
		: _key(L""), _iv(L""), _compress_mode(false), _encrypt_mode(false), _received_message(nullptr),
		_thread_pool(nullptr), _received_file(nullptr), _received_data(nullptr), _compress_block_size(1024), 
		_confirm(connection_conditions::waiting)
	{
		memset(_start_code_tag, start_code_value, start_code);
		memset(_end_code_tag, end_code_value, end_code);
	}

	data_handling::~data_handling(void)
	{
	}
	
	void data_handling::read_start_code(weak_ptr<asio::ip::tcp::socket> socket, const unsigned short& matched_code)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		if (matched_code == 4)
		{
#ifdef _DEBUG
			logger::handle().write(logging_level::packet, fmt::format(L"read start code: {} bytes", start_code));
#endif

			read_packet_code(socket);
			
			return;
		}

		_received_data_vector.clear();

		memset(_receiving_buffer, 0, buffer_size);
		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, 1), asio::transfer_exactly(1),
			[this, socket, matched_code](error_code ec, size_t length)
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

				if (_receiving_buffer[0] != _start_code_tag[matched_code])
				{
					read_start_code(socket);

					return;
				}

				read_start_code(socket, matched_code + 1);
			});
	}

	void data_handling::read_packet_code(weak_ptr<asio::ip::tcp::socket> socket)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);
		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, mode_code), asio::transfer_exactly(mode_code),
			[this, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != mode_code)
				{
					logger::handle().write(logging_level::error, 
						L"drop read data: not matched packet code");

					read_start_code(socket);

					return;
				}
				
#ifdef _DEBUG
				logger::handle().write(logging_level::packet, fmt::format(L"read packet code: {} bytes", mode_code));
#endif

				data_modes mode = (data_modes)_receiving_buffer[0];

				read_length_code(mode, socket);
			});
	}

	void data_handling::read_length_code(const data_modes& packet_mode, weak_ptr<asio::ip::tcp::socket> socket)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);
		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, length_code), asio::transfer_exactly(length_code),
			[this, packet_mode, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != length_code)
				{
					logger::handle().write(logging_level::error, 
						L"drop read data: not matched length code");

					read_start_code(socket);

					return;
				}
				
#ifdef _DEBUG
				logger::handle().write(logging_level::packet, fmt::format(L"read length code: {} bytes", length_code));
#endif

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
			return;
		}

		if (remained_length >= buffer_size)
		{
			memset(_receiving_buffer, 0, buffer_size);
			asio::async_read(*current_socket, asio::buffer(_receiving_buffer, buffer_size), asio::transfer_exactly(buffer_size),
				[this, packet_mode, remained_length, socket](error_code ec, size_t length)
				{
					if (ec)
					{
						disconnected();

						return;
					}

					if (length != buffer_size)
					{
						logger::handle().write(logging_level::error, 
							L"drop read data: not matched data length");

						read_start_code(socket);

						return;
					}

					_received_data_vector.insert(_received_data_vector.end(), _receiving_buffer, _receiving_buffer + length);
					memset(_receiving_buffer, 0, buffer_size);

					read_data(packet_mode, remained_length - length, socket);
				});

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);
		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, remained_length), asio::transfer_exactly(remained_length),
			[this, packet_mode, socket](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}
				
				_received_data_vector.insert(_received_data_vector.end(), _receiving_buffer, _receiving_buffer + length);
				memset(_receiving_buffer, 0, buffer_size);

#ifdef _DEBUG
				logger::handle().write(logging_level::packet, fmt::format(L"read data: {} bytes", _received_data_vector.size()));
#endif

				read_data(packet_mode, 0, socket);
			});
	}

	void data_handling::read_end_code(const data_modes& packet_mode, weak_ptr<asio::ip::tcp::socket> socket, const unsigned short& matched_code)
	{
		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return;
		}
		
		if (matched_code == 4)
		{
#ifdef _DEBUG
			logger::handle().write(logging_level::packet, fmt::format(L"read end code: {} bytes", end_code));
#endif

			receive_on_tcp(packet_mode, _received_data_vector);
			_received_data_vector.clear();

			read_start_code(socket);
			
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		asio::async_read(*current_socket, asio::buffer(_receiving_buffer, 1), asio::transfer_exactly(1),
			[this, packet_mode, socket, matched_code](error_code ec, size_t length)
			{
				if (ec)
				{
					disconnected();

					return;
				}

				if (length != 1 || _receiving_buffer[0] != _end_code_tag[matched_code])
				{
					logger::handle().write(logging_level::error, 
						L"drop read data: not matched end code");

					read_start_code(socket);

					return;
				}
				
				read_end_code(packet_mode, socket, matched_code + 1);
			});
	}

	bool data_handling::send_on_tcp(weak_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return false;
		}

		shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
		if (current_socket == nullptr)
		{
			return false;
		}
		
		logger::handle().write(logging_level::parameter, fmt::format(L"attempt to send message: data mode [{}]", (unsigned short)data_mode));

		size_t sended_size;
		sended_size = current_socket->send(asio::buffer(_start_code_tag, start_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, fmt::format(L"cannot send start code: {} bytes", start_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(logging_level::packet, fmt::format(L"sent start code: {} bytes", start_code));
#endif

		sended_size = current_socket->send(asio::buffer(&data_mode, mode_code));
		if (sended_size != sizeof(unsigned char))
		{
			logger::handle().write(logging_level::error, fmt::format(L"cannot send data type code: {} bytes", mode_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(logging_level::packet, fmt::format(L"sent data type code: {} bytes", mode_code));
#endif

		unsigned int length = (unsigned int)data.size();
		sended_size = current_socket->send(asio::buffer(&length, length_code));
		if (sended_size != sizeof(unsigned int))
		{
			logger::handle().write(logging_level::error, fmt::format(L"cannot send length code: {} bytes", length_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(logging_level::packet, fmt::format(L"sent length code: {} bytes", length_code));
#endif

		size_t temp = 0;
		size_t count = data.size();
		for(size_t index = 0; index < count; index += _compress_block_size)
		{
			temp = count - index;
			if (temp > _compress_block_size)
			{
				temp = _compress_block_size;
			}

			sended_size = current_socket->send(asio::buffer(data.data() + index, temp));
			if (sended_size != temp)
			{
				logger::handle().write(logging_level::error, fmt::format(L"cannot send data: {} bytes", data.size()));

				current_socket.reset();
				return false;
			}
		}

#ifdef _DEBUG
		logger::handle().write(logging_level::packet, fmt::format(L"sent data: {} bytes", data.size()));
#endif

		sended_size = current_socket->send(asio::buffer(_end_code_tag, end_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, fmt::format(L"cannot send end code: {} bytes", end_code));

			current_socket.reset();
			return false;
		}

#ifdef _DEBUG
		logger::handle().write(logging_level::packet, fmt::format(L"sent end code: {} bytes", end_code));
#endif

		logger::handle().write(logging_level::parameter, fmt::format(L"completed to send message: data mode [{}]", (unsigned short)data_mode));

		current_socket.reset();

		return true;
	}
	
	void data_handling::receive_on_tcp(const data_modes& data_mode, const vector<uint8_t>& data)
	{
		logger::handle().write(logging_level::parameter, fmt::format(L"received message: data mode [{}]", (unsigned short)data_mode));

		switch (data_mode)
		{
		case data_modes::packet_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::decompress_packet, this, placeholders::_1)));
			break;
		case data_modes::file_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::decompress_file_packet, this, placeholders::_1)));
			break;
		case data_modes::binary_mode:
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::decompress_binary_packet, this, placeholders::_1)));
			break;
		default:
			break;
		}
	}

	void data_handling::send_packet_job(const vector<uint8_t>& data)
	{
		if (_confirm == connection_conditions::confirmed)
		{
			_thread_pool->push(make_shared<job>(priorities::high, data, 
					bind(&data_handling::encrypt_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, data, 
				bind(&data_handling::compress_packet, this, placeholders::_1)));
	}

	void data_handling::send_file_job(const vector<uint8_t>& data)
	{
		_thread_pool->push(make_shared<job>(priorities::low, data, 
				bind(&data_handling::load_file_packet, this, placeholders::_1)));
	}

	void data_handling::send_binary_job(const vector<uint8_t>& data)
	{
		_thread_pool->push(make_shared<job>(priorities::top, data, 
				bind(&data_handling::encrypt_binary_packet, this, placeholders::_1)));
	}

	void data_handling::compress_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::top, data, bind(&data_handling::send_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to compress a packet");

		if (_specific_compress_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::top, _specific_compress_sequence(data, true), bind(&data_handling::send_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data, _compress_block_size), bind(&data_handling::send_packet, this, placeholders::_1)));
	}

	void data_handling::encrypt_packet(const vector<uint8_t>& data)
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

		if (_specific_encrypt_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, _specific_encrypt_sequence(data, true), bind(&data_handling::compress_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, cryptor::encryption(data, _key, _iv), bind(&data_handling::compress_packet, this, placeholders::_1)));
	}

	void data_handling::decompress_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::decrypt_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decompress a packet");
		
		if (_specific_compress_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, _specific_compress_sequence(data, false), bind(&data_handling::decrypt_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data, _compress_block_size), bind(&data_handling::decrypt_packet, this, placeholders::_1)));
	}

	void data_handling::decrypt_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_encrypt_mode || _confirm != connection_conditions::confirmed)
		{
			_thread_pool->push(make_shared<job>(priorities::low, data, bind(&data_handling::receive_packet, this, placeholders::_1)));	

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decrypt a packet");

		if (_specific_encrypt_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::low, _specific_encrypt_sequence(data, false), bind(&data_handling::receive_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::low, cryptor::decryption(data, _key, _iv), bind(&data_handling::receive_packet, this, placeholders::_1)));		
	}

	void data_handling::receive_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		shared_ptr<container::value_container> message = make_shared<container::value_container>(data, true);
		if (message == nullptr)
		{
			return;
		}

		if (message->message_type() != REQUEST_CONNECTION &&
			message->message_type() != CONFIRM_CONNECTION)
		{
			logger::handle().write(logging_level::packet, fmt::format(L"received: {}", message->serialize()));
		}

		auto target = _message_handlers.find(message->message_type());
		if (target == _message_handlers.end())
		{
			return normal_message(message);
		}

		target->second(message);
	}

	void data_handling::load_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}
		
		shared_ptr<container::value_container> message = make_shared<container::value_container>(data);
		if (message == nullptr)
		{
			return;
		}

		vector<uint8_t> result;
		combiner::append(result, converter::to_array(message->get_value(L"indication_id")->to_string()));
		combiner::append(result, converter::to_array(message->source_id()));
		combiner::append(result, converter::to_array(message->source_sub_id()));
		combiner::append(result, converter::to_array(message->target_id()));
		combiner::append(result, converter::to_array(message->target_sub_id()));
		combiner::append(result, converter::to_array(message->get_value(L"source")->to_string()));
		combiner::append(result, converter::to_array(message->get_value(L"target")->to_string()));
		combiner::append(result, file::load(message->get_value(L"source")->to_string()));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"load_file_packet: [{}] => [{}:{}] -> [{}:{}]", message->get_value(L"indication_id")->to_string(),
				message->source_id(), message->source_sub_id(), message->target_id(), message->target_sub_id()));

		_thread_pool->push(make_shared<job>(priorities::normal, result, bind(&data_handling::encrypt_file_packet, this, placeholders::_1)));
	}

	void data_handling::compress_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::top, data, bind(&data_handling::send_file_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to compress a file packet");

		if (_specific_compress_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::top, _specific_compress_sequence(data, true), bind(&data_handling::send_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data, _compress_block_size), bind(&data_handling::send_file_packet, this, placeholders::_1)));
	}

	void data_handling::encrypt_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::high, data, bind(&data_handling::compress_file_packet, this, placeholders::_1)));
			
			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to encrypt a file packet");	

		if (_specific_encrypt_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::high, _specific_encrypt_sequence(data, true), bind(&data_handling::compress_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::high, cryptor::encryption(data, _key, _iv), bind(&data_handling::compress_file_packet, this, placeholders::_1)));
	}

	void data_handling::decompress_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, data, bind(&data_handling::decrypt_file_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decompress a file packet");
		
		if (_specific_compress_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::low, _specific_compress_sequence(data, false), bind(&data_handling::decrypt_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::low, compressor::decompression(data, _compress_block_size), bind(&data_handling::decrypt_file_packet, this, placeholders::_1)));
	}

	void data_handling::decrypt_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::receive_file_packet, this, placeholders::_1)));	

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decrypt a packet");

		if (_specific_encrypt_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, _specific_encrypt_sequence(data, false), bind(&data_handling::receive_file_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, cryptor::decryption(data, _key, _iv), bind(&data_handling::receive_file_packet, this, placeholders::_1)));
	}

	void data_handling::receive_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		wstring indication_id = converter::to_wstring(combiner::divide(data, index));
		wstring source_id = converter::to_wstring(combiner::divide(data, index));
		wstring source_sub_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_sub_id = converter::to_wstring(combiner::divide(data, index));
		wstring source_path = converter::to_wstring(combiner::divide(data, index));
		wstring target_path = converter::to_wstring(combiner::divide(data, index));

		logger::handle().write(logging_level::parameter,
			fmt::format(L"receive_file_packet: [{}] => [{}:{}] -> [{}:{}]", source_path, source_id, source_sub_id, target_id, target_sub_id));

		vector<uint8_t> result;
		combiner::append(result, converter::to_array(indication_id));
		combiner::append(result, converter::to_array(target_id));
		combiner::append(result, converter::to_array(target_sub_id));
		if (file::save(target_path, combiner::divide(data, index)))
		{
			combiner::append(result, converter::to_array(target_path));
		}
		else
		{
			combiner::append(result, converter::to_array(L""));
		}

		_thread_pool->push(make_shared<job>(priorities::high, result, bind(&data_handling::notify_file_packet, this, placeholders::_1)));
	}

	void data_handling::notify_file_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		wstring indication_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_sub_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_path = converter::to_wstring(combiner::divide(data, index));

		logger::handle().write(logging_level::packet, fmt::format(L"received file packet: {}", target_path));

		if (_received_file)
		{
			logger::handle().write(logging_level::parameter, L"attempt to transfer a file packet");

			_received_file(target_id, target_sub_id, indication_id, target_path);
		}
	}

	void data_handling::compress_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::top, data, bind(&data_handling::send_binary_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to compress a packet");

		if (_specific_compress_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::top, _specific_compress_sequence(data, true), bind(&data_handling::send_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::top, compressor::compression(data, _compress_block_size), bind(&data_handling::send_binary_packet, this, placeholders::_1)));
	}

	void data_handling::encrypt_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}
			
		if (!_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::compress_binary_packet, this, placeholders::_1)));
			
			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to encrypt a packet");	

		if (_specific_encrypt_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, _specific_encrypt_sequence(data, true), bind(&data_handling::compress_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::normal, cryptor::encryption(data, _key, _iv), bind(&data_handling::compress_binary_packet, this, placeholders::_1)));
	}

	void data_handling::decompress_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_compress_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, data, bind(&data_handling::decrypt_binary_packet, this, placeholders::_1)));

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decompress a binary packet");

		if (_specific_compress_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::normal, _specific_compress_sequence(data, false), bind(&data_handling::decrypt_binary_packet, this, placeholders::_1)));

			return;
		}
			
		_thread_pool->push(make_shared<job>(priorities::normal, compressor::decompression(data, _compress_block_size), bind(&data_handling::decrypt_binary_packet, this, placeholders::_1)));
	}

	void data_handling::decrypt_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		if (!_encrypt_mode)
		{
			_thread_pool->push(make_shared<job>(priorities::low, data, bind(&data_handling::receive_binary_packet, this, placeholders::_1)));	

			return;
		}

		logger::handle().write(logging_level::parameter, L"attempt to decrypt a binary packet");

		if (_specific_encrypt_sequence)
		{
			_thread_pool->push(make_shared<job>(priorities::low, _specific_encrypt_sequence(data, false), bind(&data_handling::receive_binary_packet, this, placeholders::_1)));

			return;
		}

		_thread_pool->push(make_shared<job>(priorities::low, cryptor::decryption(data, _key, _iv), bind(&data_handling::receive_binary_packet, this, placeholders::_1)));
	}

	void data_handling::receive_binary_packet(const vector<uint8_t>& data)
	{
		if (data.empty())
		{
			return;
		}

		size_t index = 0;
		wstring source_id = converter::to_wstring(combiner::divide(data, index));
		wstring source_sub_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_id = converter::to_wstring(combiner::divide(data, index));
		wstring target_sub_id = converter::to_wstring(combiner::divide(data, index));
		vector<uint8_t> target_data = combiner::divide(data, index);

		logger::handle().write(logging_level::packet, fmt::format(L"received binary packet: {} bytes", target_data.size()));

		if (_received_data)
		{
			_received_data(source_id, source_sub_id, target_id, target_sub_id, target_data);
		}
	}
}