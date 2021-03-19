#pragma once

namespace network
{
	enum class data_modes : unsigned char
	{
		binary_mode = 1,
		packet_mode = 2,
		file_mode = 3
	};
}