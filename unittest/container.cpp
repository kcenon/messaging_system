#include "gtest/gtest.h"

#include <limits.h>

#include "converting.h"

#include "container.h"
#include "values/bool_value.h"
#include "values/container_value.h"
#include "values/double_value.h"
#include "values/float_value.h"
#include "values/llong_value.h"
#include "values/long_value.h"
#include "values/ullong_value.h"
#include "values/ulong_value.h"

using namespace container;
using namespace converting;

TEST(container, test)
{
	value_container data;
	data.add(bool_value(L"false_value", false));
	data.add(bool_value(L"true_value", true));
	data.add(float_value(L"float_value", (float)1.234567890123456789));
	data.add(double_value(L"double_value", (double)1.234567890123456789));

	value_container data2(data);

	EXPECT_STREQ(converter::to_string(data.serialize()).c_str(),
				 converter::to_string(data2.serialize()).c_str());

	data2.add(std::make_shared<long_value>(L"long_value", LONG_MAX));
	data2.add(std::make_shared<ulong_value>(L"ulong_value", ULONG_MAX));
	data2.add(std::make_shared<llong_value>(L"llong_value", LLONG_MAX));
	data2.add(std::make_shared<ullong_value>(L"ullong_value", ULLONG_MAX));

	EXPECT_STRNE(converter::to_string(data.serialize()).c_str(),
				 converter::to_string(data2.serialize()).c_str());

	value_container data3(data2);
	data3.remove(L"long_value");
	data3.remove(L"ulong_value");
	data3.remove(L"llong_value");
	data3.remove(L"ullong_value");

	EXPECT_STREQ(converter::to_string(data.serialize()).c_str(),
				 converter::to_string(data3.serialize()).c_str());
}