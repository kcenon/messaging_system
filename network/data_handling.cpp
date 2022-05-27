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

#include "logging.h"

#include <utility>

#include "fmt/format.h"

namespace network
{
	using namespace logging;

	data_handling::data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value)
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

		_received_data.clear();

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

					_received_data.insert(_received_data.end(), _receiving_buffer, _receiving_buffer + length);
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

				_received_data.insert(_received_data.end(), _receiving_buffer, _receiving_buffer + length);
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

					memset(_receiving_buffer, 0, buffer_size);

					read_start_code(socket);

					return;
				}

				receive_on_tcp(packet_mode, _received_data);
				_received_data.clear();

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

		sended_size = current_socket->send(asio::buffer(&data_mode, mode_code));
		if (sended_size != sizeof(unsigned char))
		{
			logger::handle().write(logging_level::error, L"cannot send data type code");

			current_socket.reset();
			return false;
		}

		unsigned int length = (unsigned int)data.size();
		sended_size = current_socket->send(asio::buffer(&length, length_code));
		if (sended_size != sizeof(unsigned int))
		{
			logger::handle().write(logging_level::error, L"cannot send length code");

			current_socket.reset();
			return false;
		}

		sended_size = current_socket->send(asio::buffer(data.data(), data.size()));
		if (sended_size != data.size())
		{
			logger::handle().write(logging_level::error, L"cannot send data");

			current_socket.reset();
			return false;
		}

		sended_size = current_socket->send(asio::buffer(_end_code_tag, end_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send end code");

			current_socket.reset();
			return false;
		}

		current_socket.reset();

		return true;
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