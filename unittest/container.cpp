#include "gtest/gtest.h"

#include <limits.h>
#include <string>

// #include "converting.h" - Module not available yet

#include "container.h"
#include "values/bool_value.h"
#include "values/container_value.h"
#include "values/numeric_value.h"

using namespace container;
// using namespace converting; - Namespace not available yet

// Temporary replacement for converter functions
namespace test_utils {
    std::string to_string(const std::string& str) {
        return str;
    }
    
    std::string to_string(const std::vector<uint8_t>& data) {
        return std::string(data.begin(), data.end());
    }
}

TEST(container, test)
{
	value_container data;
	data.add(bool_value("false_value", false));
	data.add(bool_value("true_value", true));
	data.add(float_value("float_value", (float)1.234567890123456789));
	data.add(double_value("double_value", (double)1.234567890123456789));

	value_container data2(data);

	EXPECT_STREQ(test_utils::to_string(data.serialize()).c_str(),
				 test_utils::to_string(data2.serialize()).c_str());

	data2.add(std::make_shared<long_value>("long_value", LONG_MAX));
	data2.add(std::make_shared<ulong_value>("ulong_value", ULONG_MAX));
	data2.add(std::make_shared<llong_value>("llong_value", LLONG_MAX));
	data2.add(std::make_shared<ullong_value>("ullong_value", ULLONG_MAX));

	EXPECT_STRNE(test_utils::to_string(data.serialize()).c_str(),
				 test_utils::to_string(data2.serialize()).c_str());

	value_container data3(data2);
	data3.remove("long_value");
	data3.remove("ulong_value");
	data3.remove("llong_value");
	data3.remove("ullong_value");

	EXPECT_STREQ(test_utils::to_string(data.serialize()).c_str(),
				 test_utils::to_string(data3.serialize()).c_str());
}