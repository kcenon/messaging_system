## How to use logging

Basically, this logging library is a thread-safe module. So, if you want to write log data into a file on a multi-thread architecture, it supports safely.

### Sample code

``` C++
#include "logging.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <iostream>

constexpr auto PROGRAM_NAME = L"logging_sample";

using namespace logging;

bool write_console = false;
logging_level log_level = logging_level::information;

int main(int argc, char* argv[])
{
    logger::handle().set_write_console(write_console);
    logger::handle().set_target_level(log_level);
    logger::handle().start(PROGRAM_NAME);

    std::vector<std::thread> threads;
    for (unsigned short thread_index = 0; thread_index < 10; ++thread_index)
    {
        threads.push_back(
            std::thread([](const unsigned short& thread_index)
            {
                for (unsigned int log_index = 0; log_index < 1000; ++log_index)
                {
                    auto start = logger::handle().chrono_start();
                    logger::handle().write(logging::logging_level::information, 
                        fmt::format(L"test_in_thread_{}: {}", thread_index, log_index), start);
                }
            }, thread_index)
        );
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    logger::handle().stop();

    return 0;
}
```