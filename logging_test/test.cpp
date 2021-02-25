#pragma once

#include "gtest/gtest.h"

#include "limits.h"
#include "logging.h"

TEST(Logging, Start) 
{
	logging::util::handle();

	EXPECT_TRUE(logging::util::handle().start());
}

TEST(Logging, Write)
{
	for (unsigned int index = 0; index < UINT_MAX; ++index)
	{
		logging::util::handle().write(logging::logging_level::information, L"test");
	}

	EXPECT_TRUE(true);
}

TEST(Logging, Stop)
{
	EXPECT_TRUE(logging::util::handle().stop());
}