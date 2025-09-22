/*****************************************************************************
BSD 3-Clause License
*****************************************************************************/

#include <gtest/gtest.h>

#include <kcenon/thread/core/job_queue.h>
#include <kcenon/thread/core/callback_job.h>
#include <kcenon/thread/core/error_handling.h>

using namespace kcenon::thread;

TEST(job_queue_error, enqueue_null)
{
    job_queue q;
    std::unique_ptr<job> j{};
    auto r = q.enqueue(std::move(j));
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::invalid_argument);
}

TEST(job_queue_error, enqueue_batch_empty)
{
    job_queue q;
    std::vector<std::unique_ptr<job>> batch;
    auto r = q.enqueue_batch(std::move(batch));
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::invalid_argument);
}

TEST(job_queue_error, enqueue_batch_contains_null)
{
    job_queue q;
    std::vector<std::unique_ptr<job>> batch;
    batch.push_back(std::unique_ptr<job>{});
    auto r = q.enqueue_batch(std::move(batch));
    ASSERT_TRUE(r.has_error());
    EXPECT_EQ(r.get_error().code(), error_code::invalid_argument);
}

TEST(job_queue_error, dequeue_after_stop)
{
    job_queue q;
    q.stop_waiting_dequeue();
    auto r = q.dequeue();
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.get_error().code(), error_code::queue_empty);
}
