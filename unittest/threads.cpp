#include "gtest/gtest.h"

#include "converting.h"
#include "job.h"
#include "thread_pool.h"

#include <memory>
#include <string>
#include <vector>

using namespace threads;
using namespace converting;

constexpr auto TEST = "test";

bool test_function(const std::vector<uint8_t>& data)
{
	std::string temp = converter::to_string(data);

	return !temp.empty();
}

bool test_function2(void) { return test_function(converter::to_array(TEST)); }

TEST(threads, test)
{
	thread_pool manager("thread_test");
	manager.append(std::make_shared<thread_worker>(priorities::high));
	manager.append(std::make_shared<thread_worker>(priorities::normal));
	manager.append(std::make_shared<thread_worker>(priorities::low));

	for (unsigned int i = 0; i < 1000; ++i)
	{
		manager.push(std::make_shared<job>(
			priorities::high, converter::to_array(TEST), &test_function));
		manager.push(
			std::make_shared<job>(priorities::normal, &test_function2));
	}

	manager.start();
	manager.stop(false);
}