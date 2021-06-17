## How to use logging

If you want to test this sample, you have to run two programs such as main_server and middle_server on build/micro_service before.
The next thing is that fill out two factors such as source_folder and target_folder.

### Sample code

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
        fmt::format(L"a client on main server: {}[{}] is {}", 
            target_id, target_sub_id, condition ? L"connected" : L"disconnected"));
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
                fmt::format(L"started download: [{}]", 
                    container->get_value(L"indication_id")->to_string()));

                return;
        }

        logger::handle().write(logging::logging_level::information,
            fmt::format(L"received percentage: [{}] {}%", 
                container->get_value(L"indication_id")->to_string(), 
                container->get_value(L"percentage")->to_ushort()));

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
                fmt::format(L"completed download: [{}]", 
                    container->get_value(L"indication_id")->to_string()));

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
        files.push_back(std::make_shared<container::container_value>(L"file", 
            std::vector<std::shared_ptr<container::value>> {
		    	std::make_shared<container::string_value>(L"source", source),
		    	std::make_shared<container::string_value>(L"target", 
                    converter::replace2(source, source_folder, target_folder))
            }
        ));
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