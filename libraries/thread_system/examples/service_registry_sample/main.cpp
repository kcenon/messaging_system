/*****************************************************************************
BSD 3-Clause License
*****************************************************************************/

#include <iostream>
#include <memory>
#include <thread>
#include <atomic>

#include <kcenon/thread/core/service_registry.h>
#include <kcenon/thread/core/thread_pool.h>
#include <kcenon/thread/core/thread_worker.h>
#include <kcenon/thread/core/callback_job.h>

using namespace kcenon::thread;
using namespace kcenon::thread;

struct demo_service {
    std::string name;
};

int main() {
    // Register and get a simple service
    auto svc = std::make_shared<demo_service>();
    svc->name = "demo";
    service_registry::register_service<demo_service>(svc);
    auto got = service_registry::get_service<demo_service>();
    std::cout << "service name = " << (got ? got->name : "<null>") << "\n";

    // Use executor_interface via thread_pool
    auto pool = std::make_shared<thread_pool>("svc_pool");
    std::vector<std::unique_ptr<thread_worker>> workers;
    workers.push_back(std::make_unique<thread_worker>(false));
    if (auto r = pool->enqueue_batch(std::move(workers)); r.has_error()) {
        std::cerr << r.get_error().to_string() << "\n";
        return 1;
    }
    if (auto r = pool->start(); r.has_error()) {
        std::cerr << r.get_error().to_string() << "\n";
        return 1;
    }

    std::atomic<int> count{0};
    if (auto r = pool->execute(std::make_unique<callback_job>([&count]() -> result_void {
            count.fetch_add(1);
            return result_void();
        })); r.has_error()) {
        std::cerr << r.get_error().to_string() << "\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "executed jobs = " << count.load() << "\n";
    pool->shutdown();
    return 0;
}
