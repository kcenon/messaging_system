/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include <iostream>

#include "logger/core/logger.h"
#include "utilities/core/formatter.h"
#include "thread_pool/core/thread_pool.h"

using namespace utility_module;
using namespace thread_pool_module;

bool use_backup_ = false;
uint32_t max_lines_ = 0;
uint16_t wait_interval_ = 100;
uint32_t test_line_count_ = 1000000;
log_module::log_types file_target_ = log_module::log_types::None;
log_module::log_types console_target_ = log_module::log_types::Information;
log_module::log_types callback_target_ = log_module::log_types::None;

uint16_t thread_counts_ = 10;

auto initialize_logger() -> std::optional<std::string>
{
	log_module::set_title("thread_pool_sample");
	log_module::set_use_backup(use_backup_);
	log_module::set_max_lines(max_lines_);
	log_module::file_target(file_target_);
	log_module::console_target(console_target_);
	log_module::callback_target(callback_target_);
	// Note: This demonstrates the logger callback feature - std::cout is intentionally used here
	log_module::message_callback(
		[](const log_module::log_types& type, const std::string& datetime,
		   const std::string& message)
		{ std::cout << formatter::format("[{}][{}] {}\n", datetime, type, message); });
	if (wait_interval_ > 0)
	{
		log_module::set_wake_interval(std::chrono::milliseconds(wait_interval_));
	}

	return log_module::start();
}

auto create_default(const uint16_t& worker_counts)
	-> std::tuple<std::shared_ptr<thread_pool>, std::optional<std::string>>
{
	std::shared_ptr<thread_pool> pool;

	try
	{
		pool = std::make_shared<thread_pool>();
	}
	catch (const std::bad_alloc& e)
	{
		return { nullptr, std::string(e.what()) };
	}

	std::optional<std::string> error_message = std::nullopt;

	std::vector<std::unique_ptr<thread_worker>> workers;
	workers.reserve(worker_counts);
	for (uint16_t i = 0; i < worker_counts; ++i)
	{
		workers.push_back(std::make_unique<thread_worker>());
	}

	error_message = pool->enqueue_batch(std::move(workers));
	if (error_message.has_value())
	{
		return { nullptr, formatter::format("cannot enqueue to workers: {}",
											error_message.value_or("unknown error")) };
	}

	return { pool, std::nullopt };
}

auto store_job(std::shared_ptr<thread_pool> thread_pool) -> std::optional<std::string>
{
	std::optional<std::string> error_message = std::nullopt;

	std::vector<std::unique_ptr<job>> jobs;
	jobs.reserve(test_line_count_);

	for (auto index = 0; index < test_line_count_; ++index)
	{
		jobs.push_back(std::make_unique<callback_job>(
			[index](void) -> result_void
			{
				log_module::write_debug("Hello, World!: {}", index);
				return result_void();
			}));
	}

	error_message = thread_pool->enqueue_batch(std::move(jobs));
	if (error_message.has_value())
	{
		return formatter::format("error enqueuing jobs: {}",
								 error_message.value_or("unknown error"));
	}

	log_module::write_sequence("enqueued jobs: {}", test_line_count_);

	return std::nullopt;
}

auto main() -> int
{
	auto error_message = initialize_logger();
	if (error_message.has_value())
	{
		std::cerr << formatter::format("error starting logger: {}\n",
									   error_message.value_or("unknown error"));
		return 0;
	}

	std::shared_ptr<thread_pool> thread_pool = nullptr;
	std::tie(thread_pool, error_message) = create_default(thread_counts_);
	if (error_message.has_value())
	{
		log_module::write_error("error creating thread pool: {}",
								error_message.value_or("unknown error"));

		return 0;
	}

	log_module::write_information("created {}", thread_pool->to_string());

	error_message = store_job(thread_pool);
	if (error_message.has_value())
	{
		log_module::write_error("error storing job: {}", error_message.value_or("unknown error"));

		thread_pool.reset();

		return 0;
	}

	error_message = thread_pool->start();
	if (error_message.has_value())
	{
		log_module::write_error("error starting thread pool: {}",
								error_message.value_or("unknown error"));

		thread_pool.reset();

		return 0;
	}

	log_module::write_information("started {}", thread_pool->to_string());

	thread_pool->stop();

	log_module::write_information("stopped {}", thread_pool->to_string());

	thread_pool.reset();

	log_module::stop();

	return 0;
}