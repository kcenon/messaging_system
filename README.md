## What is?
Normally, it is pretty hard to implement a TCP server and client with a proper thread system in the C++ language. To support newbie's desire to make their own TCP communication system, I planned its project.

So, it will contain several systems like below,
1. File log system
2. Concurrency control by the thread pool system
3. Serializable data packet container
4. Multi-session TCP server
5. TCP Client

And, it will provide functions like below,
1. Callback functions for each sequence such as connection, receiving data and receiving file
2. Send packet to specific target client
3. Send packet to all connected clients
4. Send files between main server and middle server
5. Packet compress and encrypt

## How to build
Before building this project, you have to download and build vcpkg(https://github.com/Microsoft/vcpkg).
Secondly, should install libraries like below followed vcpkg install rule,

1. ASIO(https://github.com/chriskohlhoff/asio/) or Boost(https://github.com/boostorg) library
2. fmt(https://github.com/fmtlib/fmt) library
3. cryptopp(https://www.cryptopp.com/) library
4. lz4(https://github.com/lz4/lz4) library

* If you want to use Boost library instead of Asio library, you have to remove 'ASIO_STANDALONE' on preprocessor definition on network library.

After all installations, you can build this project with Visual Studio 2019.

## How to use

### Sample: Thread safe logging

``` C++
#include "logging.h"
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
                    logger::handle().write(logging::logging_level::information, fmt::format(L"테스트_in_thread_{}: {}", thread_index, log_index), start);
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

### Sample: Container

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
    logger::handle().write(logging::logging_level::information, fmt::format(L"data serialize:\n{}", data.serialize()), start);

    start = logger::handle().chrono_start();
    value_container data2(data);
    data2.add(std::make_shared<long_value>(L"long_value", LONG_MAX));
    data2.add(std::make_shared<ulong_value>(L"ulong_value", ULONG_MAX));
    data2.add(std::make_shared<llong_value>(L"llong_value", LLONG_MAX));
    data2.add(std::make_shared<ullong_value>(L"ullong_value", ULLONG_MAX));
    logger::handle().write(logging::logging_level::information, fmt::format(L"data serialize:\n{}", data2.serialize()), start);

    start = logger::handle().chrono_start();
    value_container data3(data2);
    data3.remove(L"false_value");
    data3.remove(L"true_value");
    data3.remove(L"float_value");
    data3.remove(L"double_value");
    logger::handle().write(logging::logging_level::information, fmt::format(L"data serialize:\n{}", data3.serialize()), start);

    logger::handle().stop();

    return 0;
}
```

### Sample: Thread

``` C++
#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "argument_parsing.h"

#include "fmt/format.h"

constexpr auto PROGRAM_NAME = L"thread_sample";

using namespace logging;
using namespace converting;
using namespace threads;

bool write_console = false;
logging_level log_level = logging_level::information;

bool write_data(const std::vector<unsigned char>& data)
{
    auto start = logger::handle().chrono_start();
    logger::handle().write(logging_level::information, converter::to_wstring(data), start);

    return true;
}

bool write_high(void)
{
    return write_data(converter::to_array(L"테스트2_high_in_thread"));
}

bool write_normal(void)
{
    return write_data(converter::to_array(L"테스트2_normal_in_thread"));
}

bool write_low(void)
{
    return write_data(converter::to_array(L"테스트2_low_in_thread"));
}

class test_job : public job
{
public:
    test_job(const priorities& priority, const std::vector<unsigned char>& data) : job(priority)
    {
        _data = data;
    }

protected:
    bool working(const priorities& worker_priority) override
    {
        auto start = logger::handle().chrono_start();
        logger::handle().write(logging_level::information, converter::to_wstring(_data), start);

        return true;
    }

private:
    std::vector<unsigned char> _data;
};

class test2_job : public job
{
public:
    test2_job(const priorities& priority) : job(priority)
    {
    }

protected:
    bool working(const priorities& worker_priority) override
    {
        auto start = logger::handle().chrono_start();

        switch (priority())
        {
        case priorities::high: 
            logger::handle().write(logging_level::information, L"테스트4_high_in_thread", start);
            break;
	case priorities::normal:
            logger::handle().write(logging_level::information, L"테스트4_normal_in_thread", start);
            break;
        case priorities::low:
            logger::handle().write(logging_level::information, L"테스트4_low_in_thread", start);
            break;
        }		

        return true;
    }
};

int main(int argc, char* argv[])
{
    logger::handle().set_write_console(write_console);
    logger::handle().set_target_level(log_level);
    logger::handle().start(PROGRAM_NAME);

    thread_pool manager;
    manager.append(std::make_shared<thread_worker>(priorities::high));
    manager.append(std::make_shared<thread_worker>(priorities::high));
    manager.append(std::make_shared<thread_worker>(priorities::high));
    manager.append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
    manager.append(std::make_shared<thread_worker>(priorities::normal, std::vector<priorities> { priorities::high }));
    manager.append(std::make_shared<thread_worker>(priorities::low, std::vector<priorities> { priorities::high, priorities::normal }));
	
    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<job>(priorities::high, converter::to_array(L"테스트_high_in_thread"), &write_data));
        manager.push(std::make_shared<job>(priorities::normal, converter::to_array(L"테스트_normal_in_thread"), &write_data));
        manager.push(std::make_shared<job>(priorities::low, converter::to_array(L"테스트_low_in_thread"), &write_data));
    }

    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<job>(priorities::high, &write_high));
        manager.push(std::make_shared<job>(priorities::normal, &write_normal));
        manager.push(std::make_shared<job>(priorities::low, &write_low));
    }

    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<test_job>(priorities::high, converter::to_array(L"테스트3_high_in_thread")));
        manager.push(std::make_shared<test_job>(priorities::normal, converter::to_array(L"테스트3_normal_in_thread")));
        manager.push(std::make_shared<test_job>(priorities::low, converter::to_array(L"테스트3_low_in_thread")));
    }

    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<test2_job>(priorities::high));
        manager.push(std::make_shared<test2_job>(priorities::normal));
        manager.push(std::make_shared<test2_job>(priorities::low));
    }

    manager.start();
    manager.stop(false);

    logger::handle().stop();

    return 0;
}
```

### Sample: Download file

If you want to test this sample, you have to run two programs such as main_server and middle_server on build/micro_service before.
The next thing is that fill out two factors such as source_folder and target_folder.

``` C++
#include <iostream>

#include "logging.h"
#include "converting.h"
#include "messaging_client.h"
#include "folder_handling.h"

#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"

#include "fmt/format.h"

#include <future>

constexpr auto PROGRAM_NAME = L"download_sample";

using namespace logging;
using namespace network;
using namespace converting;
using namespace folder_handling;

bool write_console = false;
bool encrypt_mode = false;
bool compress_mode = false;
logging_level log_level = logging_level::information;
std::wstring source_folder = L"";
std::wstring target_folder = L"";
std::wstring connection_key = L"middle_connection_key";
std::wstring server_ip = L"127.0.0.1";
unsigned short server_port = 8642;
unsigned short high_priority_count = 1;
unsigned short normal_priority_count = 2;
unsigned short low_priority_count = 3;

std::promise<bool> _promise_status;
std::future<bool> _future_status;

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
    logger::handle().write(logging::logging_level::information,
        fmt::format(L"a client on main server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

void received_message(std::shared_ptr<container::value_container> container)
{
    if (container == nullptr)
    {
        return;
    }

    if (container->message_type() == L"transfer_condition")
    {
        if (container->get_value(L"percentage")->to_ushort() == 0)
        {
            logger::handle().write(logging::logging_level::information,
                fmt::format(L"started download: [{}]", container->get_value(L"indication_id")->to_string()));

                return;
        }

        logger::handle().write(logging::logging_level::information,
            fmt::format(L"received percentage: [{}] {}%", container->get_value(L"indication_id")->to_string(), container->get_value(L"percentage")->to_ushort()));

        if (container->get_value(L"completed")->to_boolean())
        {
            logger::handle().write(logging::logging_level::information,
                fmt::format(L"completed download: [{}] success-{}, fail-{}", 
		    container->get_value(L"indication_id")->to_string(), 
		    container->get_value(L"completed_count")->to_ushort(), 
		    container->get_value(L"failed_count")->to_ushort()));

            _promise_status.set_value(false);
        }
        else if (container->get_value(L"percentage")->to_ushort() == 100)
        {
            logger::handle().write(logging::logging_level::information,
                fmt::format(L"completed download: [{}]", container->get_value(L"indication_id")->to_string()));

            _promise_status.set_value(true);
        }

        return;
    }

    logger::handle().write(logging::logging_level::information,
        fmt::format(L"received message: {}", container->serialize()));
}

int main(int argc, char* argv[])
{
    std::vector<std::wstring> sources = folder_handler::get_files(source_folder);
    if (sources.empty())
    {
        return 0;
    }

    logger::handle().set_write_console(write_console);
    logger::handle().set_target_level(log_level);
    logger::handle().start(PROGRAM_NAME);

    std::shared_ptr<messaging_client> client = std::make_shared<messaging_client>(PROGRAM_NAME);
    client->set_compress_mode(compress_mode);
    client->set_connection_key(connection_key);
    client->set_connection_notification(&connection);
    client->set_message_notification(&received_message);
    client->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);

    std::vector<std::shared_ptr<container::value>> files;

    files.push_back(std::make_shared<container::string_value>(L"indication_id", L"download_test"));
    for (auto& source : sources)
    {
        files.push_back(std::make_shared<container::container_value>(L"file", std::vector<std::shared_ptr<container::value>> {
			std::make_shared<container::string_value>(L"source", source),
			std::make_shared<container::string_value>(L"target", converter::replace2(source, source_folder, target_folder))
        }));
    }

    _future_status = _promise_status.get_future();

    std::shared_ptr<container::value_container> container = 
        std::make_shared<container::value_container>(L"main_server", L"", L"download_files", files);
    client->send(container);

    _future_status.wait_for(std::chrono::seconds(100));

    client->stop();

    logger::handle().stop();

    return 0;
}
```

### Sample: Upload file

If you want to test this sample, you have to run two programs such as main_server and middle_server on build/micro_service before.
The next thing is that fill out two factors such as source_folder and target_folder.

``` C++
#include <iostream>

#include "logging.h"
#include "converting.h"
#include "messaging_client.h"
#include "folder_handling.h"

#include "container.h"
#include "values/string_value.h"
#include "values/container_value.h"

#include "fmt/format.h"

#include <future>

constexpr auto PROGRAM_NAME = L"upload_sample";

using namespace logging;
using namespace network;
using namespace converting;
using namespace folder_handling;

bool write_console = false;
bool encrypt_mode = false;
bool compress_mode = false;
logging_level log_level = logging_level::information;
std::wstring source_folder = L"";
std::wstring target_folder = L"";
std::wstring connection_key = L"middle_connection_key";
std::wstring server_ip = L"127.0.0.1";
unsigned short server_port = 8642;
unsigned short high_priority_count = 1;
unsigned short normal_priority_count = 2;
unsigned short low_priority_count = 3;

std::promise<bool> _promise_status;
std::future<bool> _future_status;

void connection(const std::wstring& target_id, const std::wstring& target_sub_id, const bool& condition)
{
    logger::handle().write(logging::logging_level::information,
        fmt::format(L"a client on main server: {}[{}] is {}", target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
}

void received_message(std::shared_ptr<container::value_container> container)
{
    if (container == nullptr)
    {
        return;
    }

    if (container->message_type() == L"transfer_condition")
    {
        if (container->get_value(L"percentage")->to_ushort() == 0)
        {
            logger::handle().write(logging::logging_level::information,
                fmt::format(L"started upload: [{}]", container->get_value(L"indication_id")->to_string()));

            return;
        }

        logger::handle().write(logging::logging_level::information,
            fmt::format(L"received percentage: [{}] {}%", container->get_value(L"indication_id")->to_string(), container->get_value(L"percentage")->to_ushort()));

        if (container->get_value(L"completed")->to_boolean())
        {
            logger::handle().write(logging::logging_level::information,
                fmt::format(L"completed download: [{}] success-{}, fail-{}", 
		    container->get_value(L"indication_id")->to_string(), 
		    container->get_value(L"completed_count")->to_ushort(), 
		    container->get_value(L"failed_count")->to_ushort()));

            _promise_status.set_value(false);
        }
        else if (container->get_value(L"percentage")->to_ushort() == 100)
        {
            logger::handle().write(logging::logging_level::information,
            fmt::format(L"completed upload: [{}]", container->get_value(L"indication_id")->to_string()));

            _promise_status.set_value(true);
        }

        return;
    }

    logger::handle().write(logging::logging_level::information,
        fmt::format(L"received message: {}", container->serialize()));
}

int main(int argc, char* argv[])
{
    std::vector<std::wstring> sources = folder_handler::get_files(source_folder);
    if (sources.empty())
    {
        return 0;
    }

    logger::handle().set_write_console(write_console);
    logger::handle().set_target_level(log_level);
    logger::handle().start(PROGRAM_NAME);

    std::shared_ptr<messaging_client> client = std::make_shared<messaging_client>(PROGRAM_NAME);
    client->set_compress_mode(compress_mode);
    client->set_connection_key(connection_key);
    client->set_connection_notification(&connection);
    client->set_message_notification(&received_message);
    client->start(server_ip, server_port, high_priority_count, normal_priority_count, low_priority_count);

    std::vector<std::shared_ptr<container::value>> files;

    files.push_back(std::make_shared<container::string_value>(L"indication_id", L"upload_test"));
    for (auto& source : sources)
    {
        files.push_back(std::make_shared<container::container_value>(L"file", std::vector<std::shared_ptr<container::value>> {
			std::make_shared<container::string_value>(L"source", source),
			std::make_shared<container::string_value>(L"target", converter::replace2(source, source_folder, target_folder))
		}));
    }

    _future_status = _promise_status.get_future();

    std::shared_ptr<container::value_container> container = 
        std::make_shared<container::value_container>(L"main_server", L"", L"upload_files", files);
    client->send(container);

    _future_status.wait_for(std::chrono::seconds(100));

    client->stop();

    logger::handle().stop();

    return 0;
}
```

## License

Note: This license has also been called the "New BSD License" or "Modified BSD License". See also the 2-clause BSD License.

Copyright 2021 🍀☀🌕🌥 🌊

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## Contact
Please report issues or questions here: https://github.com/kcenon/messaging_system/issues
