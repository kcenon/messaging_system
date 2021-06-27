#pragma once

namespace threads
{
	enum class priorities : unsigned short
	{
		low = 0,
		normal = 1,
		high = 2,
		top = 3
	};
}