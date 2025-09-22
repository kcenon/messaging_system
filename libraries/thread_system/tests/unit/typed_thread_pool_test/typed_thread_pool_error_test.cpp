/*****************************************************************************
BSD 3-Clause License
*****************************************************************************/

#include "gtest/gtest.h"

#include "typed_thread_pool.h"
#include "typed_thread_worker.h"
#include "error_handling.h"

using namespace kcenon::thread;
using namespace kcenon::thread;

TEST(typed_thread_pool_error, start_without_workers)
{
    auto pool = std::make_shared<typed_thread_pool>();
    auto r = pool->start();
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::thread_start_failure);
}

TEST(typed_thread_pool_error, enqueue_null_worker)
{
    auto pool = std::make_shared<typed_thread_pool>();
    std::unique_ptr<typed_thread_worker> w{};
    auto r = pool->enqueue(std::move(w));
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::invalid_argument);
}

