#pragma once

namespace network
{
	enum class session_types : short
	{
		message_line = 1,
		file_line = 2,
		binary_line = 3
	};
}
