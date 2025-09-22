/*****************************************************************************
BSD 3-Clause License
*****************************************************************************/

#include <gtest/gtest.h>

#include <kcenon/thread/interfaces/executor_interface.h>
#include <kcenon/thread/interfaces/scheduler_interface.h>
#include <kcenon/thread/interfaces/monitorable_interface.h>

#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/job_queue.h>
#include <kcenon/thread/core/callback_job.h>

#include <kcenon/thread/core/service_registry.h>

using namespace kcenon::thread;

TEST(interfaces_test, scheduler_interface_job_queue)
{
    job_queue queue;

    std::atomic<int> count{0};
    auto r1 = queue.schedule(std::make_unique<callback_job>([&count]() -> result_void {
        count.fetch_add(1);
        return result_void();
    }));
    EXPECT_TRUE(r1);

    auto j = queue.get_next_job();
    ASSERT_TRUE(j.has_value());
    auto res = j.value()->do_work();
    (void)res;
    EXPECT_EQ(count.load(), 1);
}

TEST(interfaces_test, executor_interface_thread_pool)
{
    auto pool = std::make_shared<thread_pool>("ifx_pool");
    // add one worker
    std::vector<std::unique_ptr<thread_worker>> workers;
    {
        auto w = std::make_unique<thread_worker>(false);
        w->set_wake_interval(std::chrono::milliseconds(10));
        workers.push_back(std::move(w));
    }
    ASSERT_TRUE(pool->enqueue_batch(std::move(workers)));

    std::atomic<int> count{0};
    executor_interface* exec = pool.get();
    auto r = exec->execute(std::make_unique<callback_job>([&count]() -> result_void {
        count.fetch_add(1);
        return result_void();
    }));
    ASSERT_TRUE(r);

    // Start after enqueue so worker picks up existing job
    ASSERT_TRUE(pool->start());

    auto start = std::chrono::steady_clock::now();
    while (count.load() < 1 && std::chrono::steady_clock::now() - start < std::chrono::seconds(2)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(count.load(), 1);
    EXPECT_TRUE(exec->shutdown());
}

namespace {
class dummy_monitorable : public monitorable_interface {
public:
    auto get_metrics() -> ::monitoring_interface::metrics_snapshot override {
        return snapshot_;
    }
    void reset_metrics() override { snapshot_ = {}; }
private:
    ::monitoring_interface::metrics_snapshot snapshot_{};
};
}

TEST(interfaces_test, monitorable_interface_mock)
{
    dummy_monitorable m;
    auto s = m.get_metrics();
    (void)s;
    m.reset_metrics();
    SUCCEED();
}

TEST(interfaces_test, service_registry_basic)
{
    struct foo { int v{0}; };
    auto f = std::make_shared<foo>();
    f->v = 42;
    service_registry::register_service<foo>(f);
    auto r = service_registry::get_service<foo>();
    ASSERT_TRUE(r != nullptr);
    EXPECT_EQ(r->v, 42);
}
