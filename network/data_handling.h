#pragma once

#include "data_modes.h"
#include "data_lengths.h"

#include <vector>
#include <system_error>

#ifdef ASIO_STANDALONE
#include "asio.hpp"
#else
#include <boost/asio.hpp>
#endif

namespace network
{
	class data_handling
	{
	public:
		data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value);
		~data_handling(void);

	protected:
#ifdef ASIO_STANDALONE
		void read_start_code(std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_packet_code(std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_length_code(const data_modes& packet_mode, std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_data(const data_modes& packet_mode, const size_t& remained_length, std::weak_ptr<asio::ip::tcp::socket> socket);
		void read_end_code(const data_modes& packet_mode, std::weak_ptr<asio::ip::tcp::socket> socket);
#else
		void read_start_code(std::weak_ptr<boost::asio::ip::tcp::socket> socket);
		void read_packet_code(std::weak_ptr<boost::asio::ip::tcp::socket> socket);
		void read_length_code(const data_modes& packet_mode, std::weak_ptr<boost::asio::ip::tcp::socket> socket);
		void read_data(const data_modes& packet_mode, const size_t& remained_length, std::weak_ptr<boost::asio::ip::tcp::socket> socket);
		void read_end_code(const data_modes& packet_mode, std::weak_ptr<boost::asio::ip::tcp::socket> socket);
#endif

	protected:
#ifdef ASIO_STANDALONE
		bool send_on_tcp(std::weak_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const std::vector<unsigned char>& data);
#else
		bool send_on_tcp(std::weak_ptr<boost::asio::ip::tcp::socket> socket, const data_modes& data_mode, const std::vector<unsigned char>& data);
#endif
		virtual void receive_on_tcp(const data_modes& data_mode, const std::vector<unsigned char>& data) = 0;

	protected:
		virtual void disconnected(void) = 0;

	protected:
		void append_binary_on_packet(std::vector<unsigned char>& result, const std::vector<unsigned char>& source);
		std::vector<unsigned char> devide_binary_on_packet(const std::vector<unsigned char>& source, size_t& index);

	private:
		char _start_code_tag[start_code];
		char _end_code_tag[end_code];
		char _receiving_buffer[buffer_size];
		std::vector<unsigned char> _received_data;
	};
}