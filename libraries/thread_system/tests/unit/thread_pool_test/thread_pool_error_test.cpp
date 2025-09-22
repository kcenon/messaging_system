/*****************************************************************************
BSD 3-Clause License
*****************************************************************************/

#include "gtest/gtest.h"

#include "thread_pool.h"
#include "callback_job.h"
#include "error_handling.h"

using namespace kcenon::thread;
using namespace kcenon::thread;

TEST(thread_pool_error, start_without_workers)
{
    auto pool = std::make_shared<thread_pool>();
    auto r = pool->start();
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::invalid_argument);
}

TEST(thread_pool_error, enqueue_null_job)
{
    auto pool = std::make_shared<thread_pool>();
    std::unique_ptr<job> j{};
    auto r = pool->enqueue(std::move(j));
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::invalid_argument);
}

TEST(thread_pool_error, stop_when_not_started)
{
    auto pool = std::make_shared<thread_pool>();
    auto r = pool->stop(false);
    // stop is idempotent; consider success when not running
    EXPECT_FALSE(r.has_error());
}

