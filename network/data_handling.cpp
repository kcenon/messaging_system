#include "data_handling.h"

#include "logging.h"

#include <utility>

namespace network
{
	using namespace logging;

	data_handling::data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value)
	{
		memset(_start_code_tag, start_code_value, start_code);
		memset(_end_code_tag, end_code_value, end_code);
	}

	data_handling::~data_handling(void)
	{
	}

	void data_handling::read_start_code(std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		if (socket == nullptr)
		{
			return;
		}

		_received_data.clear();
		memset(_receiving_buffer, 0, buffer_size);

		socket->async_receive(asio::buffer(_receiving_buffer, start_code),
			[this, socket](std::error_code ec, std::size_t length)
			{
				if (ec || length != start_code)
				{
					read_start_code(socket);

					return;
				}

				for (int index = 0; index < start_code; ++index)
				{
					if (_receiving_buffer[index] == _start_code_tag[index])
					{
						continue;
					}

					read_start_code(socket);

					return;
				}

				read_packet_code(socket);
			});
	}

	void data_handling::read_packet_code(std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		if (socket == nullptr)
		{
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		socket->async_receive(asio::buffer(_receiving_buffer, mode_code),
			[this, socket](std::error_code ec, std::size_t length)
			{
				if (ec || length != mode_code)
				{
					read_start_code(socket);

					return;
				}

				read_length_code((data_modes)_receiving_buffer[0], socket);
			});
	}

	void data_handling::read_length_code(const data_modes& packet_mode, std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		if (socket == nullptr)
		{
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		socket->async_receive(asio::buffer(_receiving_buffer, length_code),
			[this, packet_mode, socket](std::error_code ec, std::size_t length)
			{
				if (ec || length != length_code)
				{
					read_start_code(socket);

					return;
				}

				unsigned int target_length = 0;
				memcpy(&target_length, _receiving_buffer, length);

				read_data(packet_mode, target_length, socket);
			});
	}

	void data_handling::read_data(const data_modes& packet_mode, const unsigned int& remained_length, std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		if (socket == nullptr)
		{
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		if (remained_length == 0)
		{
			read_end_code(packet_mode, socket);

			return;
		}

		if (remained_length >= buffer_size)
		{
			socket->async_receive(asio::buffer(_receiving_buffer, buffer_size),
				[this, packet_mode, remained_length,socket](std::error_code ec, std::size_t length)
				{
					if (ec || length != buffer_size)
					{
						read_start_code(socket);

						return;
					}

					_received_data.insert(_received_data.end(), _receiving_buffer, _receiving_buffer + length);

					read_data(packet_mode, remained_length - length, socket);
				});

			return;
		}

		socket->async_receive(asio::buffer(_receiving_buffer, remained_length),
			[this, packet_mode, socket](std::error_code ec, std::size_t length)
			{
				if (ec)
				{
					read_start_code(socket);

					return;
				}

				_received_data.insert(_received_data.end(), _receiving_buffer, _receiving_buffer + length);

				read_data(packet_mode, 0, socket);
			});
	}

	void data_handling::read_end_code(const data_modes& packet_mode, std::shared_ptr<asio::ip::tcp::socket> socket)
	{
		if (socket == nullptr)
		{
			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		socket->async_receive(asio::buffer(_receiving_buffer, end_code),
			[this, packet_mode, socket](std::error_code ec, std::size_t length)
			{
				if (ec || length != end_code)
				{
					read_start_code(socket);

					return;
				}

				for (int index = 0; index < end_code; ++index)
				{
					if (_receiving_buffer[index] == _end_code_tag[index])
					{
						continue;
					}

					read_start_code(socket);

					return;
				}

				receive_on_tcp(packet_mode, _received_data);
				_received_data.clear();
			});
	}

	bool data_handling::send_on_tcp(std::shared_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const std::vector<char>& data)
	{
		if (socket == nullptr)
		{
			return false;
		}

		if (data.empty())
		{
			return false;
		}


		size_t sended_size;
		sended_size = socket->send(asio::buffer(_start_code_tag, start_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send start code");
			return false;
		}

		sended_size = socket->send(asio::buffer(&data_mode, mode_code));
		if (sended_size != sizeof(unsigned char))
		{
			logger::handle().write(logging_level::error, L"cannot send data type code");
			return false;
		}

		unsigned int length = (unsigned int)data.size();
		sended_size = socket->send(asio::buffer(&length, length_code));
		if (sended_size != sizeof(unsigned int))
		{
			logger::handle().write(logging_level::error, L"cannot send length code");
			return false;
		}

		sended_size = socket->send(asio::buffer(data.data(), data.size()));
		if (sended_size != data.size())
		{
			logger::handle().write(logging_level::error, L"cannot send data");
			return false;
		}

		sended_size = socket->send(asio::buffer(_end_code_tag, end_code));
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send end code");
			return false;
		}

		return true;
	}
}