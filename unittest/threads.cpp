#ifdef __USE_UNIT_TEST__

#include "gtest/gtest.h"

#include "job.h"
#include "converting.h"
#include "thread_pool.h"

#include <memory>
#include <vector>
#include <string>

using namespace std;
using namespace threads;
using namespace converting;

constexpr auto TEST = L"test";

bool test_function(const vector<uint8_t>& data)
{
    wstring temp = converter::to_wstring(data);
    
    return !temp.empty();
}

bool test_function2(void)
{
    return test_function(converter::to_array(TEST));
}

TEST(threads, test)
{
    thread_pool manager(L"thread_test");
    manager.append(make_shared<thread_worker>(priorities::high));
    manager.append(make_shared<thread_worker>(priorities::normal));
    manager.append(make_shared<thread_worker>(priorities::low));

    for (unsigned int i = 0; i < 1000; ++i)
    {
        manager.push(make_shared<job>(priorities::high, converter::to_array(TEST), &test_function));
        manager.push(make_shared<job>(priorities::normal, &test_function2));
    }

    manager.start();
    manager.stop(false);
}

#endif