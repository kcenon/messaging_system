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
		memset(_receiving_buffer, 0, buffer_size);
	}

	data_handling::~data_handling(void)
	{
	}

#ifdef ASIO_STANDALONE
	void data_handling::read_start_code(std::weak_ptr<asio::ip::tcp::socket> socket)
#else
	void data_handling::read_start_code(std::weak_ptr<boost::asio::ip::tcp::socket> socket)
#endif
	{
#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> current_socket = socket.lock();
#endif
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		_received_data.clear();

#ifdef ASIO_STANDALONE
		current_socket->async_receive(asio::buffer(_receiving_buffer, start_code),
			[this, socket](std::error_code ec, std::size_t length)
#else
		current_socket->async_receive(boost::asio::buffer(_receiving_buffer, start_code),
			[this, socket](boost::system::error_code ec, std::size_t length)
#endif
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

#ifdef ASIO_STANDALONE
	void data_handling::read_packet_code(std::weak_ptr<asio::ip::tcp::socket> socket)
#else
	void data_handling::read_packet_code(std::weak_ptr<boost::asio::ip::tcp::socket> socket)
#endif
	{
#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> current_socket = socket.lock();
#endif
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

#ifdef ASIO_STANDALONE
		current_socket->async_receive(asio::buffer(_receiving_buffer, mode_code),
			[this, socket](std::error_code ec, std::size_t length)
#else
		current_socket->async_receive(boost::asio::buffer(_receiving_buffer, mode_code),
			[this, socket](boost::system::error_code ec, std::size_t length)
#endif
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

#ifdef ASIO_STANDALONE
	void data_handling::read_length_code(const data_modes& packet_mode, std::weak_ptr<asio::ip::tcp::socket> socket)
#else
	void data_handling::read_length_code(const data_modes& packet_mode, std::weak_ptr<boost::asio::ip::tcp::socket> socket)
#endif
	{
#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> current_socket = socket.lock();
#endif
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

#ifdef ASIO_STANDALONE
		current_socket->async_receive(asio::buffer(_receiving_buffer, length_code),
			[this, packet_mode, socket](std::error_code ec, std::size_t length)
#else
		current_socket->async_receive(boost::asio::buffer(_receiving_buffer, length_code),
			[this, packet_mode, socket](boost::system::error_code ec, std::size_t length)
#endif
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

#ifdef ASIO_STANDALONE
	void data_handling::read_data(const data_modes& packet_mode, const unsigned int& remained_length, std::weak_ptr<asio::ip::tcp::socket> socket)
#else
	void data_handling::read_data(const data_modes& packet_mode, const unsigned int& remained_length, std::weak_ptr<boost::asio::ip::tcp::socket> socket)
#endif
	{
		if (remained_length == 0)
		{
			read_end_code(packet_mode, socket);

			return;
		}

#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> current_socket = socket.lock();
#endif
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

		if (remained_length >= buffer_size)
		{
#ifdef ASIO_STANDALONE
			current_socket->async_receive(asio::buffer(_receiving_buffer, buffer_size),
				[this, packet_mode, remained_length, socket](std::error_code ec, std::size_t length)
#else
			current_socket->async_receive(boost::asio::buffer(_receiving_buffer, buffer_size),
				[this, packet_mode, remained_length, socket](boost::system::error_code ec, std::size_t length)
#endif
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

#ifdef ASIO_STANDALONE
		current_socket->async_receive(asio::buffer(_receiving_buffer, remained_length),
			[this, packet_mode, socket](std::error_code ec, std::size_t length)
#else
		current_socket->async_receive(boost::asio::buffer(_receiving_buffer, remained_length),
			[this, packet_mode, socket](boost::system::error_code ec, std::size_t length)
#endif
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

#ifdef ASIO_STANDALONE
	void data_handling::read_end_code(const data_modes& packet_mode, std::weak_ptr<asio::ip::tcp::socket> socket)
#else
	void data_handling::read_end_code(const data_modes& packet_mode, std::weak_ptr<boost::asio::ip::tcp::socket> socket)
#endif
	{
#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> current_socket = socket.lock();
#endif
		if (current_socket == nullptr)
		{
			disconnected();

			return;
		}

		memset(_receiving_buffer, 0, buffer_size);

#ifdef ASIO_STANDALONE
		current_socket->async_receive(asio::buffer(_receiving_buffer, end_code),
			[this, packet_mode, socket](std::error_code ec, std::size_t length)
#else
		current_socket->async_receive(boost::asio::buffer(_receiving_buffer, end_code),
			[this, packet_mode, socket](boost::system::error_code ec, std::size_t length)
#endif
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

#ifdef ASIO_STANDALONE
	bool data_handling::send_on_tcp(std::weak_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const std::vector<unsigned char>& data)
#else
	bool data_handling::send_on_tcp(std::weak_ptr<boost::asio::ip::tcp::socket> socket, const data_modes& data_mode, const std::vector<unsigned char>& data)
#endif
	{
		if (data.empty())
		{
			return false;
		}

#ifdef ASIO_STANDALONE
		std::shared_ptr<asio::ip::tcp::socket> current_socket = socket.lock();
#else
		std::shared_ptr<boost::asio::ip::tcp::socket> current_socket = socket.lock();
#endif
		if (current_socket == nullptr)
		{
			disconnected();

			return false;
		}

		size_t sended_size;
#ifdef ASIO_STANDALONE
		sended_size = current_socket->send(asio::buffer(_start_code_tag, start_code));
#else
		sended_size = current_socket->send(boost::asio::buffer(_start_code_tag, start_code));
#endif
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send start code");

			current_socket.reset();
			return false;
		}

#ifdef ASIO_STANDALONE
		sended_size = current_socket->send(asio::buffer(&data_mode, mode_code));
#else
		sended_size = current_socket->send(boost::asio::buffer(&data_mode, mode_code));
#endif
		if (sended_size != sizeof(unsigned char))
		{
			logger::handle().write(logging_level::error, L"cannot send data type code");

			current_socket.reset();
			return false;
		}

		unsigned int length = (unsigned int)data.size();
#ifdef ASIO_STANDALONE
		sended_size = current_socket->send(asio::buffer(&length, length_code));
#else
		sended_size = current_socket->send(boost::asio::buffer(&length, length_code));
#endif
		if (sended_size != sizeof(unsigned int))
		{
			logger::handle().write(logging_level::error, L"cannot send length code");

			current_socket.reset();
			return false;
		}

#ifdef ASIO_STANDALONE
		sended_size = current_socket->send(asio::buffer(data.data(), data.size()));
#else
		sended_size = current_socket->send(boost::asio::buffer(data.data(), data.size()));
#endif
		if (sended_size != data.size())
		{
			logger::handle().write(logging_level::error, L"cannot send data");

			current_socket.reset();
			return false;
		}

#ifdef ASIO_STANDALONE
		sended_size = current_socket->send(asio::buffer(_end_code_tag, end_code));
#else
		sended_size = current_socket->send(boost::asio::buffer(_end_code_tag, end_code));
#endif
		if (sended_size != 4)
		{
			logger::handle().write(logging_level::error, L"cannot send end code");

			current_socket.reset();
			return false;
		}

		current_socket.reset();

		return true;
	}

	void data_handling::append_data_on_file_packet(std::vector<unsigned char>& result, const std::vector<unsigned char>& source)
	{
		size_t temp;
		const int size = sizeof(size_t);
		char temp_size[size];

		temp = source.size();
		memcpy(temp_size, &temp, size);
		result.insert(result.end(), temp_size, temp_size + size);
		result.insert(result.end(), source.begin(), source.end());
	}

	std::vector<unsigned char> data_handling::devide_data_on_file_packet(const std::vector<unsigned char>& source, size_t& index)
	{
		if (source.empty())
		{
			return std::vector<unsigned char>();
		}

		size_t temp;
		const int size = sizeof(size_t);

		if (source.size() < index + size)
		{
			return std::vector<unsigned char>();
		}

		memcpy(&temp, source.data() + index, size);
		index += size;

		if (source.size() < index + temp)
		{
			return std::vector<unsigned char>();
		}

		std::vector<unsigned char> result;
		result.insert(result.end(), source.begin() + index, source.begin() + index + temp);

		return result;
	}
}