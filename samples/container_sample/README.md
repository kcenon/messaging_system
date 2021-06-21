## How to use data container

Basically, this data container can store all types on it, serialize, and deserialize.

### Sample code

``` C++
#include "logging.h"

#include "container.h"
#include "values/bool_value.h"
#include "values/float_value.h"
#include "values/double_value.h"
#include "values/long_value.h"
#include "values/ulong_value.h"
#include "values/llong_value.h"
#include "values/ullong_value.h"

#include "fmt/format.h"

#include <limits.h>
#include <memory>
#include <iostream>

constexpr auto PROGRAM_NAME = L"container_sample";

using namespace logging;
using namespace container;

bool write_console = false;
logging_level log_level = logging_level::information;

int main(int argc, char* argv[])
{
    logger::handle().set_write_console(write_console);
    logger::handle().set_target_level(log_level);
    logger::handle().start(PROGRAM_NAME);

    auto start = logger::handle().chrono_start();
    value_container data;
    data.add(bool_value(L"false_value", false));
    data.add(bool_value(L"true_value", true));
    data.add(float_value(L"float_value", (float)1.234567890123456789));
    data.add(double_value(L"double_value", (double)1.234567890123456789));

    // write serialized data
    logger::handle().write(logging::logging_level::information, 
        fmt::format(L"data serialize:\n{}", data.serialize()), start);

    start = logger::handle().chrono_start();
    value_container data2(data);
    data2.add(std::make_shared<long_value>(L"long_value", LONG_MAX));
    data2.add(std::make_shared<ulong_value>(L"ulong_value", ULONG_MAX));
    data2.add(std::make_shared<llong_value>(L"llong_value", LLONG_MAX));
    data2.add(std::make_shared<ullong_value>(L"ullong_value", ULLONG_MAX));

    // write serialized data
    logger::handle().write(logging::logging_level::information, 
        fmt::format(L"data serialize:\n{}", data2.serialize()), start);

    start = logger::handle().chrono_start();
    value_container data3(data2);
    data3.remove(L"false_value");
    data3.remove(L"true_value");
    data3.remove(L"float_value");
    data3.remove(L"double_value");

    // write serialized data
    logger::handle().write(logging::logging_level::information, 
        fmt::format(L"data serialize:\n{}", data3.serialize()), start);

    logger::handle().stop();

    return 0;
}
```