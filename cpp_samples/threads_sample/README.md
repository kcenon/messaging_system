## How to use priority thread

Basically, this thread library can support thread handling with multi-workers to an inherited job and a callback function.

### Sample code

``` C++
#include <iostream>

#include "logging.h"
#include "thread_pool.h"
#include "thread_worker.h"
#include "job_pool.h"
#include "job.h"

#include "converting.h"

#include "argument_parsing.h"

#include "fmt/xchar.h"
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
    return write_data(converter::to_array(L"test2_high_in_thread"));
}

bool write_normal(void)
{
    return write_data(converter::to_array(L"test2_normal_in_thread"));
}

bool write_low(void)
{
    return write_data(converter::to_array(L"test2_low_in_thread"));
}

class saving_test_job : public job
{
public:
    saving_test_job(const priorities& priority, const std::vector<unsigned char>& data) : job(priority, data)
    {
		save();
    }

protected:
    bool working(const priorities& worker_priority) override
    {
        auto start = logger::handle().chrono_start();
        logger::handle().write(logging_level::information, converter::to_wstring(_data), start);

        return true;
    }
};

class test_job_without_data : public job
{
public:
    test_job_without_data(const priorities& priority) : job(priority)
    {
    }

protected:
    bool working(const priorities& worker_priority) override
    {
        auto start = logger::handle().chrono_start();

        switch (priority())
        {
        case priorities::high: 
            logger::handle().write(logging_level::information, 
                L"test4_high_in_thread", start);
            break;
        case priorities::normal:
            logger::handle().write(logging_level::information, 
                L"test4_normal_in_thread", start);
            break;
        case priorities::low:
            logger::handle().write(logging_level::information, 
                L"test4_low_in_thread", start);
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

    // create thread_pool
    thread_pool manager;
    manager.append(std::make_shared<thread_worker>(priorities::high));
    manager.append(std::make_shared<thread_worker>(priorities::high));
    manager.append(std::make_shared<thread_worker>(priorities::high));
    manager.append(std::make_shared<thread_worker>(priorities::normal, 
        std::vector<priorities> { priorities::high }));
    manager.append(std::make_shared<thread_worker>(priorities::normal, 
        std::vector<priorities> { priorities::high }));
    manager.append(std::make_shared<thread_worker>(priorities::low, 
        std::vector<priorities> { priorities::high, priorities::normal }));
	
	// unit job with callback and data
    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<job>(priorities::high, 
            converter::to_array(L"test_high_in_thread"), &write_data));
        manager.push(std::make_shared<job>(priorities::normal, 
            converter::to_array(L"test_normal_in_thread"), &write_data));
        manager.push(std::make_shared<job>(priorities::low, 
            converter::to_array(L"test_low_in_thread"), &write_data));
    }

	// unit job with callback
    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<job>(priorities::high, &write_high));
        manager.push(std::make_shared<job>(priorities::normal, &write_normal));
        manager.push(std::make_shared<job>(priorities::low, &write_low));
    }

	// derived job with data
    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<saving_test_job>(priorities::high, 
            converter::to_array(L"test3_high_in_thread")));
        manager.push(std::make_shared<saving_test_job>(priorities::normal, 
            converter::to_array(L"test3_normal_in_thread")));
        manager.push(std::make_shared<saving_test_job>(priorities::low, 
            converter::to_array(L"test3_low_in_thread")));
    }

	// derived job without data
    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<test_job_without_data>(priorities::high));
        manager.push(std::make_shared<test_job_without_data>(priorities::normal));
        manager.push(std::make_shared<test_job_without_data>(priorities::low));
    }

#ifdef __USE_CHAKRA_CORE__
    for (unsigned int log_index = 0; log_index < 1000; ++log_index)
    {
        manager.push(std::make_shared<job>(priorities::high, 
            converter::to_array(L"(()=>{return \'테스트5_high_in_thread\';})()")));
        manager.push(std::make_shared<job>(priorities::normal, 
            converter::to_array(L"(()=>{return \'테스트5_normal_in_thread\';})()")));
        manager.push(std::make_shared<job>(priorities::low, 
            converter::to_array(L"(()=>{return \'테스트5_low_in_thread\';})()")));
    }
#endif

    // If you want to check the thread-safe of priority job-pool, 
    // you can call the below function before appending jobs.
    manager.start();

    manager.stop(false);

    logger::handle().stop();

    return 0;
}
```