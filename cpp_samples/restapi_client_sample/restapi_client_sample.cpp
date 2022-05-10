#include <iostream>

#include "logging.h"

#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"
#include "folder_handler.h"
#include "argument_parser.h"
#include "constexpr_string.h"
#include "cpprest/json.h"
#include "cpprest/http_client.h"

#include "fmt/xchar.h"
#include "fmt/format.h"

#include <future>
#include <vector>

constexpr auto PROGRAM_NAME = L"restapi_client_sample";

using namespace std;
using namespace logging;
using namespace threads;
using namespace converting;
using namespace folder_handler;
using namespace argument_parser;

using namespace web;
using namespace web::http;
using namespace web::http::client;

bool write_console = false;

#ifdef _DEBUG
logging_level log_level = logging_level::parameter;
#else
logging_level log_level = logging_level::information;
#endif
wstring source_folder = L"";
wstring target_folder = L"";
unsigned short server_port = 7654;

shared_ptr<thread_pool> _thread_pool;
shared_ptr<http_client> _rest_client;

promise<bool> _promise_status;
future<bool> _future_status;

void get_request(void);
void post_request(const vector<unsigned char>& data);
bool parse_arguments(const map<wstring, wstring>& arguments);
void display_help(void);

int main(int argc, char* argv[])
{
	if (!parse_arguments(argument::parse(argc, argv)))
	{
		return 0;
	}

	logger::handle().set_write_console(write_console);
	logger::handle().set_target_level(log_level);
	logger::handle().start(PROGRAM_NAME);

	vector<wstring> sources = folder::get_files(source_folder);
	if (sources.empty())
	{
		logger::handle().stop();

		display_help();

		return 0;
	}

	_thread_pool = make_shared<threads::thread_pool>();
	_thread_pool->append(make_shared<thread_worker>(priorities::high, vector<priorities> { priorities::normal, priorities::low }), true);
	_thread_pool->append(make_shared<thread_worker>(priorities::normal, vector<priorities> { priorities::high, priorities::low }), true);
	_thread_pool->append(make_shared<thread_worker>(priorities::low, vector<priorities> { priorities::high, priorities::normal }), true);

#ifdef _WIN32
	_rest_client = make_shared<http_client>(fmt::format(L"http://localhost:{}/restapi", server_port));
#else
	_rest_client = make_shared<http_client>(fmt::format("http://localhost:{}/restapi", server_port));
#endif

	_future_status = _promise_status.get_future();
	
	json::value container = json::value::object(true);

#ifdef _WIN32
	container[MESSAGE_TYPE] = json::value::string(L"download_files");
	container[INDICATION_ID] = json::value::string(L"download_test");
#else
	container[MESSAGE_TYPE] = json::value::string("download_files");
	container[INDICATION_ID] = json::value::string("download_test");
#endif

	int index = 0;
#ifdef _WIN32
	container[FILES] = json::value::array();
	for (auto& source : sources)
	{
		container[FILES][index][SOURCE] = json::value::string(source);
		container[FILES][index][TARGET] = json::value::string(converter::replace2(source, source_folder, target_folder));
		index++;
	}
#else
	container[FILES] = json::value::array();
	for (auto& source : sources)
	{
		container[FILES][index][SOURCE] = json::value::string(converter::to_string(source));
		container[FILES][index][TARGET] = json::value::string(converter::to_string(converter::replace2(source, source_folder, target_folder)));
		index++;
	}
#endif

	_thread_pool->push(make_shared<job>(priorities::high, converter::to_array(container.serialize()), &post_request));
	_thread_pool->push(make_shared<job>(priorities::low, &get_request));

	_future_status.wait();

	_thread_pool->stop();
	_thread_pool.reset();

	logger::handle().stop();

	return 0;
}

void get_request(void)
{
	http_request request(methods::GET);

#ifdef _WIN32
	request.headers().add(L"previous_message", L"clear");
	request.headers().add(INDICATION_ID, L"download_test");
#else
	request.headers().add("previous_message", "clear");
	request.headers().add("indication_id", "download_test");
#endif
	_rest_client->request(request)
		.then([](http_response response)
			{
				if (response.status_code() != status_codes::OK)
				{
					this_thread::sleep_for(chrono::seconds(1));

					_thread_pool->push(make_shared<job>(priorities::low, &get_request));

					return;
				}

				auto answer = response.extract_json().get();
				if (answer.is_null())
				{
					return;
				}

#ifdef _WIN32
				auto& messages = answer[L"messages"].as_array();
				for (auto& message : messages)
				{
					if (message[L"percentage"].as_integer() == 0)
					{
						logger::handle().write(logging_level::information,
							fmt::format(L"started {}: [{}]", message[MESSAGE_TYPE].as_string(), 
								message[INDICATION_ID].as_string()));

						continue;
					}

					logger::handle().write(logging_level::information,
						fmt::format(L"received percentage: [{}] {}%", message[INDICATION_ID].as_string(),
							message[L"percentage"].as_integer()));

					if (message[L"percentage"].as_integer() != 100)
					{
						continue;
					}

					if (message[L"completed"].as_bool())
					{
						logger::handle().write(logging_level::information,
							fmt::format(L"completed {}: [{}]", message[MESSAGE_TYPE].as_string(), 
								message[INDICATION_ID].as_string()));

						_promise_status.set_value(true);

						return;
					}

					logger::handle().write(logging_level::information,
						fmt::format(L"cannot complete {}: [{}]", message[MESSAGE_TYPE].as_string(), 
							message[INDICATION_ID].as_string()));

					_promise_status.set_value(false);

					return;
				}
#else
				auto& messages = answer["messages"].as_array();
				for (auto& message : messages)
				{
					if (message["percentage"].as_integer() == 0)
					{
						logger::handle().write(logging_level::information,
							converter::to_wstring(fmt::format("started {}: [{}]", message[MESSAGE_TYPE].as_string(),
								message[INDICATION_ID].as_string())));
						
						continue;
					}

					logger::handle().write(logging_level::information,
						converter::to_wstring(fmt::format("received percentage: [{}] {}%", message[INDICATION_ID].as_string(),
							message["percentage"].as_integer())));

					if (message["percentage"].as_integer() != 100)
					{
						continue;
					}

					if (message["completed"].as_bool())
					{
						logger::handle().write(logging_level::information,
							converter::to_wstring(fmt::format("completed {}: [{}]", message[MESSAGE_TYPE].as_string(),
								message[INDICATION_ID].as_string())));
					
						_promise_status.set_value(true);

						return;
					}

					logger::handle().write(logging_level::information,
						converter::to_wstring(fmt::format("cannot complete {}: [{}]", message[MESSAGE_TYPE].as_string(),
							message[INDICATION_ID].as_string())));

					_promise_status.set_value(false);

					return;
				}
#endif

				_thread_pool->push(make_shared<job>(priorities::low, &get_request));
			})
		.wait();
}

void post_request(const vector<unsigned char>& data)
{
#ifdef _WIN32
	auto request_value = json::value::parse(converter::to_wstring(data));
#else
	auto request_value = json::value::parse(converter::to_string(data));
#endif

#ifdef _WIN32
	_rest_client->request(methods::POST, L"", request_value)
#else
	_rest_client->request(methods::POST, "", request_value)
#endif
		.then([](http_response response)
			{
				if (response.status_code() == status_codes::OK)
				{
#ifdef _WIN32
					logger::handle().write(logging_level::information, response.extract_string().get());
#else
					logger::handle().write(logging_level::information, converter::to_wstring(response.extract_string().get()));
#endif
				}
			})
		.wait();

	_thread_pool->push(make_shared<job>(priorities::low, &get_request));
}

bool parse_arguments(const map<wstring, wstring>& arguments)
{
	wstring temp;

	auto target = arguments.find(L"--help");
	if (target != arguments.end())
	{
		display_help();

		return false;
	}

	target = arguments.find(L"--server_port");
	if (target != arguments.end())
	{
		server_port = (unsigned short)atoi(converter::to_string(target->second).c_str());
	}

	target = arguments.find(L"--source_folder");
	if (target != arguments.end())
	{
		source_folder = target->second;
	}

	target = arguments.find(L"--target_folder");
	if (target != arguments.end())
	{
		target_folder = target->second;
	}

	target = arguments.find(L"--write_console_mode");
	if (target != arguments.end())
	{
		temp = target->second;
		transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

		if (temp.compare(L"true") == 0)
		{
			write_console = true;
		}
		else
		{
			write_console = false;
		}
	}

	target = arguments.find(L"--logging_level");
	if (target != arguments.end())
	{
		log_level = (logging_level)atoi(converter::to_string(target->second).c_str());
	}

	return true;
}

void display_help(void)
{
	wcout << L"restapi client sample options:" << endl << endl;
	wcout << L"--server_port [value]" << endl;
	wcout << L"\tIf you want to change a port number for the connection to the main server must be appended\n\t'--server_port [port number]'." << endl << endl;
	wcout << L"--source_folder [path]" << endl;
	wcout << L"\tIf you want to download folder on middle server on computer must be appended '--source_folder [path]'." << endl << endl;
	wcout << L"--target_folder [path]" << endl;
	wcout << L"\tIf you want to download on your computer must be appended '--target_folder [path]'." << endl << endl;
	wcout << L"--write_console_mode [value] " << endl;
	wcout << L"\tThe write_console_mode on/off. If you want to display log on console must be appended '--write_console_mode true'.\n\tInitialize value is --write_console_mode off." << endl << endl;
	wcout << L"--logging_level [value]" << endl;
	wcout << L"\tIf you want to change log level must be appended '--logging_level [level]'." << endl;
}
