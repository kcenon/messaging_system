#pragma once

#include "data_modes.h"
#include "data_lengths.h"

#include <vector>
#include <system_error>

#include "asio.hpp"

namespace network
{
	class data_handling
	{
	public:
		data_handling(const unsigned char& start_code_value, const unsigned char& end_code_value);
		~data_handling(void);

	protected:
		void read_start_code(std::shared_ptr<asio::ip::tcp::socket> socket);
		void read_packet_code(std::shared_ptr<asio::ip::tcp::socket> socket);
		void read_length_code(const data_modes& packet_mode, std::shared_ptr<asio::ip::tcp::socket> socket);
		void read_data(const data_modes& packet_mode, const unsigned int& remained_length, std::shared_ptr<asio::ip::tcp::socket> socket);
		void read_end_code(const data_modes& packet_mode, std::shared_ptr<asio::ip::tcp::socket> socket);

	protected:
		bool send_on_tcp(std::shared_ptr<asio::ip::tcp::socket> socket, const data_modes& data_mode, const std::vector<char>& data);
		virtual void receive_on_tcp(const data_modes& data_mode, const std::vector<char>& data) = 0;

	private:
		char _start_code_tag[start_code];
		char _end_code_tag[end_code];
		char _receiving_buffer[buffer_size];
		std::vector<char> _received_data;
	};
}