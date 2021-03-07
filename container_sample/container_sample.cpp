#include "logging.h"

#include "container.h"
#include "values/bool_value.h"
#include "values/float_value.h"
#include "values/double_value.h"

#include "fmt/format.h"

#include <memory>
#include <iostream>

using namespace logging;
using namespace container;

int main()
{
	logger::handle().start();

	auto start = logger::handle().chrono_start();

	values data;
	data.add(std::make_shared<bool_value>(L"false_value", false));
	data.add(std::make_shared<bool_value>(L"true_value", true));
	data.add(std::make_shared<float_value>(L"float_value", (float)1.234567890123456789));
	data.add(std::make_shared<double_value>(L"double_value", (double)1.234567890123456789));
	logger::handle().write(logging::logging_level::information, fmt::format(L"data serialize: {}", data.serialize()), start);

	logger::handle().stop();

    return 0;
}