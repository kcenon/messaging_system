#include "logging.h"

#include "limits.h"

#include "fmt/format.h"

#include <iostream>

int main()
{
	logging::util::handle().start();
	for (unsigned int index = 0; index < UINT_MAX; ++index)
	{
		logging::util::handle().write(logging::logging_level::information, fmt::format(L"test_{}", index));
	}
	logging::util::handle().stop();

    return 0;
}