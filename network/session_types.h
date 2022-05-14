#pragma once

namespace network
{
	enum class session_types : short
	{
		message_line = 1,
		file_line = 2,
		binary_line = 3
	};

	enum class session_conditions : short
	{
		waiting = 1,
		expired = 2,
		confirmed = 3
	};
}
