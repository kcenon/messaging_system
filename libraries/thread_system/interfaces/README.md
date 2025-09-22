# Interfaces Module

Public interfaces decoupling components and enabling dependency injection:

- `executor_interface` — submit work and shutdown (implemented by thread pools)
- `scheduler_interface` — enqueue/dequeue jobs (implemented by job queues)
- `logger_interface`/`logger_registry` — logging abstraction and global registry
- `monitoring_interface`/`monitorable_interface` — metrics reporting/query
- `thread_context` — unified access to optional services (logger/monitoring)
- `service_container` — thread-safe DI container (singleton/factory lifetimes)
- `error_handler` — pluggable error handling (with `default_error_handler`)
- `crash_handler` — process-wide crash safety hooks (optional)

See `docs/INTERFACES.md` for full API details and examples.

Usage
- Register services in `service_container::global()` and use `thread_context`
  in pools/workers to access them.
