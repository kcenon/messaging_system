/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2025, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

#include "../database_types.h"
#include "../database_base.h"
#include <future>
#include <memory>
#include <coroutine>
#include <functional>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <string>
#include <exception>
#include <vector>
#include <unordered_map>

namespace database::async
{
	// Forward declarations
	template<typename T> class async_result;
	class async_executor;
	class transaction_coordinator;

	/**
	 * @class async_result
	 * @brief Template class for asynchronous operation results.
	 */
	template<typename T>
	class async_result
	{
	public:
		async_result(std::future<T> future);

		// Blocking operations
		T get();
		T get_for(std::chrono::milliseconds timeout);

		// Non-blocking operations
		bool is_ready() const;
		std::future_status wait_for(std::chrono::milliseconds timeout) const;

		// Callback support
		void then(std::function<void(T)> callback);
		void on_error(std::function<void(const std::exception&)> error_handler);

	private:
		std::future<T> future_;
		std::function<void(T)> success_callback_;
		std::function<void(const std::exception&)> error_callback_;
	};

	/**
	 * @class coroutine_support
	 * @brief C++20 coroutine support for database operations.
	 */
	template<typename T>
	class database_awaitable
	{
	public:
		struct promise_type
		{
			T result_;
			std::exception_ptr exception_;

			database_awaitable get_return_object() {
				return database_awaitable{std::coroutine_handle<promise_type>::from_promise(*this)};
			}

			std::suspend_never initial_suspend() { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }

			void return_value(T value) { result_ = std::move(value); }
			void unhandled_exception() { exception_ = std::current_exception(); }
		};

		database_awaitable(std::coroutine_handle<promise_type> handle) : handle_(handle) {}
		~database_awaitable() { if (handle_) handle_.destroy(); }

		database_awaitable(const database_awaitable&) = delete;
		database_awaitable& operator=(const database_awaitable&) = delete;

		database_awaitable(database_awaitable&& other) noexcept : handle_(other.handle_) {
			other.handle_ = nullptr;
		}

		database_awaitable& operator=(database_awaitable&& other) noexcept {
			if (this != &other) {
				if (handle_) handle_.destroy();
				handle_ = other.handle_;
				other.handle_ = nullptr;
			}
			return *this;
		}

		bool await_ready() const { return handle_.done(); }
		void await_suspend(std::coroutine_handle<> waiting_coroutine) {
			// Resume waiting coroutine when this one completes
		}

		T await_resume() {
			if (handle_.promise().exception_) {
				std::rethrow_exception(handle_.promise().exception_);
			}
			return std::move(handle_.promise().result_);
		}

	private:
		std::coroutine_handle<promise_type> handle_;
	};

	/**
	 * @class async_database
	 * @brief Asynchronous database interface wrapper.
	 */
	class async_database
	{
	public:
		async_database(std::shared_ptr<database_base> db, std::shared_ptr<async_executor> executor);

		// Asynchronous query operations
		async_result<bool> execute_async(const std::string& query);
		async_result<database_result> select_async(const std::string& query);

		// Coroutine support
		database_awaitable<bool> execute_coro(const std::string& query);
		database_awaitable<database_result> select_coro(const std::string& query);

		// Batch operations
		async_result<std::vector<bool>> execute_batch_async(const std::vector<std::string>& queries);
		async_result<std::vector<database_result>> select_batch_async(const std::vector<std::string>& queries);

		// Transaction support
		async_result<bool> begin_transaction_async();
		async_result<bool> commit_transaction_async();
		async_result<bool> rollback_transaction_async();

		// Connection management
		async_result<bool> connect_async(const std::string& connection_string);
		async_result<bool> disconnect_async();

	private:
		std::shared_ptr<database_base> db_;
		std::shared_ptr<async_executor> executor_;
	};

	/**
	 * @class async_executor
	 * @brief Thread pool executor for asynchronous database operations.
	 */
	class async_executor
	{
	public:
		async_executor(size_t thread_count = std::thread::hardware_concurrency());
		~async_executor();

		// Task submission
		template<typename F, typename... Args>
		auto submit(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

		// Executor management
		void shutdown();
		void wait_for_completion();
		size_t pending_tasks() const;

	private:
		void worker_thread();

		std::vector<std::thread> workers_;
		std::queue<std::function<void()>> tasks_;
		std::mutex queue_mutex_;
		std::condition_variable condition_;
		std::atomic<bool> stop_{false};
	};

	/**
	 * @class stream_processor
	 * @brief Real-time data stream processing.
	 */
	class stream_processor
	{
	public:
		enum class stream_type {
			postgresql_notify,
			mongodb_change_stream,
			redis_pubsub,
			custom
		};

		struct stream_event {
			stream_type type;
			std::string channel;
			std::string payload;
			std::chrono::system_clock::time_point timestamp;
			std::unordered_map<std::string, std::string> metadata;
		};

		stream_processor(std::shared_ptr<database_base> db);
		~stream_processor();

		// Stream management
		bool start_stream(stream_type type, const std::string& channel);
		bool stop_stream(const std::string& channel);
		void stop_all_streams();

		// Event handling
		void register_event_handler(const std::string& channel,
		                           std::function<void(const stream_event&)> handler);
		void register_global_handler(std::function<void(const stream_event&)> handler);

		// Filter support
		void add_event_filter(const std::string& channel,
		                     std::function<bool(const stream_event&)> filter);

	private:
		void stream_thread(const std::string& channel, stream_type type);
		void process_event(const stream_event& event);

		std::shared_ptr<database_base> db_;
		std::unordered_map<std::string, std::thread> stream_threads_;
		std::unordered_map<std::string, std::function<void(const stream_event&)>> event_handlers_;
		std::vector<std::function<void(const stream_event&)>> global_handlers_;
		std::unordered_map<std::string, std::function<bool(const stream_event&)>> event_filters_;
		std::atomic<bool> running_{true};
		std::mutex handlers_mutex_;
	};

	/**
	 * @class transaction_coordinator
	 * @brief Distributed transaction coordination.
	 */
	class transaction_coordinator
	{
	public:
		enum class transaction_state {
			active,
			preparing,
			prepared,
			committing,
			committed,
			aborting,
			aborted
		};

		struct distributed_transaction {
			std::string transaction_id;
			std::vector<std::shared_ptr<database_base>> participants;
			transaction_state state;
			std::chrono::system_clock::time_point start_time;
			std::chrono::system_clock::time_point last_activity;
		};

		static transaction_coordinator& instance();

		// Transaction management
		std::string begin_distributed_transaction(const std::vector<std::shared_ptr<database_base>>& participants);
		async_result<bool> commit_distributed_transaction(const std::string& transaction_id);
		async_result<bool> rollback_distributed_transaction(const std::string& transaction_id);

		// Two-phase commit protocol
		async_result<bool> prepare_phase(const std::string& transaction_id);
		async_result<bool> commit_phase(const std::string& transaction_id);

		// Saga pattern support
		class saga_builder;
		saga_builder create_saga();

		// Transaction recovery
		void recover_transactions();
		std::vector<distributed_transaction> get_active_transactions() const;

	private:
		transaction_coordinator() = default;

		async_result<bool> two_phase_commit(const std::string& transaction_id);
		void cleanup_completed_transactions();

		mutable std::mutex transactions_mutex_;
		std::unordered_map<std::string, distributed_transaction> active_transactions_;
		std::shared_ptr<async_executor> executor_;
	};

	/**
	 * @class saga_builder
	 * @brief Builder for Saga pattern transactions.
	 */
	class saga_builder
	{
	public:
		saga_builder(transaction_coordinator& coordinator);

		// Saga step definition
		saga_builder& add_step(std::function<async_result<bool>()> action,
		                      std::function<async_result<bool>()> compensation);

		// Execution
		async_result<bool> execute();

	private:
		struct saga_step {
			std::function<async_result<bool>()> action;
			std::function<async_result<bool>()> compensation;
		};

		transaction_coordinator& coordinator_;
		std::vector<saga_step> steps_;
	};

	/**
	 * @class connection_pool_async
	 * @brief Asynchronous connection pool wrapper.
	 */
	class connection_pool_async
	{
	public:
		connection_pool_async(std::shared_ptr<class connection_pool_base> pool);

		// Asynchronous connection management
		async_result<std::shared_ptr<async_database>> acquire_connection_async();
		void release_connection_async(std::shared_ptr<async_database> connection);

		// Pool monitoring
		async_result<class connection_stats> get_stats_async() const;

	private:
		std::shared_ptr<class connection_pool_base> pool_;
		std::shared_ptr<async_executor> executor_;
	};

	// Template implementation for async_executor
	template<typename F, typename... Args>
	auto async_executor::submit(F&& func, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
	{
		using return_type = std::invoke_result_t<F, Args...>;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(func), std::forward<Args>(args)...));

		std::future<return_type> result = task->get_future();

		{
			std::unique_lock<std::mutex> lock(queue_mutex_);
			if (stop_) {
				throw std::runtime_error("Cannot submit task to stopped executor");
			}
			tasks_.emplace([task]() { (*task)(); });
		}

		condition_.notify_one();
		return result;
	}

	// Helper functions for async operations
	template<typename T>
	async_result<T> make_ready_result(T value) {
		std::promise<T> promise;
		promise.set_value(std::move(value));
		return async_result<T>(promise.get_future());
	}

	template<typename T>
	async_result<T> make_error_result(const std::exception& error) {
		std::promise<T> promise;
		promise.set_exception(std::make_exception_ptr(error));
		return async_result<T>(promise.get_future());
	}

	// Coroutine helpers
	inline auto when_all(std::vector<database_awaitable<bool>> awaitables) -> database_awaitable<std::vector<bool>> {
		std::vector<bool> results;
		for (auto& awaitable : awaitables) {
			results.push_back(co_await awaitable);
		}
		co_return results;
	}

	inline auto when_any(std::vector<database_awaitable<bool>> awaitables) -> database_awaitable<bool> {
		// In a real implementation, this would race the awaitables
		if (!awaitables.empty()) {
			co_return co_await awaitables[0];
		}
		co_return false;
	}

} // namespace database::async