# Core Module

Foundational building blocks for all implementations:

- base: `thread_base`, `thread_conditions`, platform abstraction in `detail/thread_impl`
- jobs: `job`, `callback_job`, `job_queue` (mutex-based baseline)
- sync: error handling (`result<T>`, `error_code`), `cancellation_token`, sync primitives

Key headers (relative to repository root):
- `core/base/include/thread_base.h`
- `core/jobs/include/job.h`, `core/jobs/include/job_queue.h`
- `core/sync/include/error_handling.h`, `core/sync/include/cancellation_token.h`

Typical usage
```cpp
class my_worker : public thread_module::thread_base {
protected:
  auto before_start() -> thread_module::result_void override { return {}; }
  auto do_work() -> thread_module::result_void override { /* ... */ return {}; }
  auto after_stop() -> thread_module::result_void override { return {}; }
};
```

Notes
- `thread_base` supports both C++20 `std::jthread` and legacy `std::thread` via `detail/thread_impl`.
- Public APIs return `result_void`/`result<T>` with `error_code` for robust error handling.
