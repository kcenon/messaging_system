/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
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

#include "thread_system_core/thread_base/core/thread_base.h"

/**
 * @file thread_base.cpp
 * @brief Implementation of the core thread base class.
 *
 * This file contains the implementation of the thread_base class, which serves
 * as the foundation for all worker thread types in the thread system.
 */

namespace thread_module
{
	/**
	 * @brief Constructs a new thread_base instance with the specified title.
	 * 
	 * Implementation details:
	 * - Initializes the worker_thread_ pointer to nullptr (thread not started)
	 * - Sets up thread control mechanisms based on configuration:
	 *   - In C++20 mode (USE_STD_JTHREAD), initializes stop_source_ to std::nullopt
	 *   - In legacy mode, initializes stop_requested_ to false
	 * - Sets wake_interval_ to std::nullopt (no periodic wake-ups by default)
	 * - Sets thread_title_ to the provided title
	 * - Sets initial thread_condition_ to Created
	 */
	thread_base::thread_base(const std::string& thread_title)
		: wake_interval_(std::nullopt)
		, worker_thread_(nullptr)
#ifdef USE_STD_JTHREAD
		, stop_source_(std::nullopt)
#else
		, stop_requested_(false)
#endif
		, thread_title_(thread_title)
		, thread_condition_(thread_conditions::Created)
	{
	}

	/**
	 * @brief Destroys the thread_base instance, stopping the thread if needed.
	 * 
	 * Implementation details:
	 * - Calls stop() to ensure the thread is properly terminated
	 * - The stop() method handles joining the thread and cleaning up resources
	 * - This ensures no thread resources are leaked when the object is destroyed
	 * 
	 * @note This destructor is virtual, allowing derived classes to perform
	 * their own cleanup operations in their destructors.
	 */
	thread_base::~thread_base(void) { stop(); }

	/**
	 * @brief Sets the wake interval for periodic thread wake-ups.
	 * 
	 * Implementation details:
	 * - Uses a dedicated mutex (wake_interval_mutex_) to ensure thread-safe access
	 * - The scoped_lock ensures automatic release when the function exits
	 * - This interval controls how often the thread wakes up even when idle
	 * - Setting std::nullopt disables periodic wake-ups (thread only wakes on signals)
	 * 
	 * Thread Safety:
	 * - Safe to call from any thread while the worker thread is running
	 * - The wake_interval_mutex_ protects against data races with get_wake_interval()
	 */
	auto thread_base::set_wake_interval(
		const std::optional<std::chrono::milliseconds>& wake_interval) -> void
	{
		// Use dedicated mutex for wake_interval to prevent data races
		std::scoped_lock<std::mutex> lock(wake_interval_mutex_);
		wake_interval_ = wake_interval;
	}

	/**
	 * @brief Gets the current wake interval setting.
	 * 
	 * Implementation details:
	 * - Uses the same mutex as set_wake_interval() for consistency
	 * - Returns a copy of the current wake_interval_ value
	 * - std::nullopt indicates no periodic wake-ups are configured
	 * 
	 * Thread Safety:
	 * - Safe to call from any thread concurrently with set_wake_interval()
	 * - The mutex ensures consistent reads even during concurrent modifications
	 * 
	 * @return Current wake interval or std::nullopt if disabled
	 */
	auto thread_base::get_wake_interval() const 
		-> std::optional<std::chrono::milliseconds>
	{
		// Thread-safe read of wake_interval
		std::scoped_lock<std::mutex> lock(wake_interval_mutex_);
		return wake_interval_;
	}

	/**
	 * @brief Starts the worker thread and begins execution loop.
	 * 
	 * Implementation details:
	 * - First checks if thread is already running using different methods:
	 *   - C++20: stop_source_.has_value() indicates active thread
	 *   - Legacy: worker_thread_->joinable() indicates active thread
	 * - Calls stop() first to ensure clean state (idempotent operation)
	 * - Initializes stop control mechanism:
	 *   - C++20: Creates new std::stop_source for cooperative cancellation
	 *   - Legacy: Resets atomic stop_requested_ flag to false
	 * - Creates worker thread that executes the main work loop
	 * 
	 * Main Work Loop Logic:
	 * 1. Calls before_start() hook for derived class initialization
	 * 2. Enters main loop while not stopped and has work to do
	 * 3. Sets thread_condition_ to Waiting before sleep
	 * 4. Waits on condition variable with optional timeout (wake_interval)
	 * 5. Sets thread_condition_ to Working before calling do_work()
	 * 6. Calls do_work() hook for actual work execution
	 * 7. Handles exceptions from do_work() gracefully
	 * 8. Calls after_stop() hook for cleanup when exiting
	 * 
	 * Error Handling:
	 * - Returns error if thread is already running
	 * - Catches std::bad_alloc during thread creation
	 * - Logs errors from hooks using std::cerr
	 * - Exception-safe cleanup in catch blocks
	 * 
	 * @return Empty result on success, error on failure
	 */
	auto thread_base::start(void) -> result_void
	{
		// Check if thread is already running using platform-specific method
#ifdef USE_STD_JTHREAD
		if (stop_source_.has_value())
#else
		if (worker_thread_ && worker_thread_->joinable())
#endif
		{
			return result_void{error{error_code::thread_already_running, "thread is already running"}};
		}

		// Ensure clean state by stopping any existing thread first
		stop();

		// Initialize stop control mechanism for the new thread
#ifdef USE_STD_JTHREAD
		stop_source_ = std::stop_source();
#else
		stop_requested_ = false;
#endif

		try
		{
			// Create the worker thread using platform-appropriate thread type
#ifdef USE_STD_JTHREAD
			worker_thread_ = std::make_unique<std::jthread>(
#else
			worker_thread_ = std::make_unique<std::thread>(
#endif
				[this](void)  // Capture 'this' to access member functions and variables
				{
#ifdef USE_STD_JTHREAD
					// Get stop token for cooperative cancellation in C++20
					auto stop_token = stop_source_.value().get_token();
#endif

					// Phase 1: Call derived class initialization hook
					auto work_result = before_start();
					if (work_result.has_error())
					{
						std::cerr << "error before start: " << work_result.get_error().to_string()
								  << std::endl;
					}

					// Phase 2: Main work loop - continues until stop requested and no more work
#ifdef USE_STD_JTHREAD
					while (!stop_token.stop_requested() || should_continue_work())
#else
					while (!stop_requested_ || should_continue_work())
#endif
					{
						// Update thread state to indicate it's waiting for work
						thread_condition_.store(thread_conditions::Waiting);

						// Get current wake interval with thread-safe access
						auto interval = get_wake_interval();
						
						// Use unique_lock for condition variable operations (required by std::condition_variable)
						std::unique_lock<std::mutex> lock(cv_mutex_);
						
						// Wait strategy depends on whether wake interval is configured
						if (interval.has_value())
						{
							// Timed wait: wake up after interval OR when condition is met
#ifdef USE_STD_JTHREAD
							worker_condition_.wait_for(
								lock, interval.value(), [this, &stop_token]()
								{ return stop_token.stop_requested() || should_continue_work(); });
#else
							worker_condition_.wait_for(
								lock, interval.value(),
								[this]() { return stop_requested_ || should_continue_work(); });
#endif
						}
						else
						{
							// Indefinite wait: only wake up when condition is met
#ifdef USE_STD_JTHREAD
							worker_condition_.wait(
								lock, [this, &stop_token]()
								{ return stop_token.stop_requested() || should_continue_work(); });
#else
							worker_condition_.wait(
								lock,
								[this]() { return stop_requested_ || should_continue_work(); });
#endif
						}

						// Check if we should exit the loop
#ifdef USE_STD_JTHREAD
						if (stop_token.stop_requested() && !should_continue_work())
#else
						if (stop_requested_ && !should_continue_work())
#endif
						{
							// Update state to indicate graceful shutdown in progress
							thread_condition_.store(thread_conditions::Stopping);
							break;
						}

						// Execute the actual work with exception protection
						try
						{
							// Update state to indicate active work is being performed
							thread_condition_.store(thread_conditions::Working);

							// Call derived class work implementation
							work_result = do_work();
							if (work_result.has_error())
							{
								std::cerr << "error doing work on " << thread_title_ << " : "
										  << work_result.get_error().to_string() << std::endl;
							}
						}
						catch (const std::exception& e)
						{
							// Log any unhandled exceptions from do_work() but continue running
							std::cerr << "exception in " << thread_title_ << ": " << e.what() << '\n';
						}
					}

					// Phase 3: Call derived class cleanup hook after main loop exits
					work_result = after_stop();
					if (work_result.has_error())
					{
						std::cerr << "error after stop: " << work_result.get_error().to_string()
								  << std::endl;
					}
				});  // End of lambda function passed to thread constructor
		}
		catch (const std::bad_alloc& e)
		{
			// Exception-safe cleanup: reset all resources if thread creation fails
#ifdef USE_STD_JTHREAD
			stop_source_.reset();
#else
			stop_requested_ = true;
#endif

			worker_thread_.reset();

			return result_void{error{error_code::resource_allocation_failed, e.what()}};
		}

		// Thread creation successful
		return {};
	}

	/**
	 * @brief Stops the worker thread and waits for it to complete.
	 * 
	 * Implementation details:
	 * - This method is idempotent - safe to call multiple times
	 * - First checks if there's actually a thread to stop
	 * - Uses platform-specific stop signaling:
	 *   - C++20: Uses std::stop_source for cooperative cancellation
	 *   - Legacy: Sets atomic flag stop_requested_ to true
	 * - Notifies condition variable to wake up waiting thread
	 * - Joins the thread to wait for complete shutdown
	 * - Cleans up all thread-related resources
	 * 
	 * Shutdown Sequence:
	 * 1. Signal stop request using appropriate mechanism
	 * 2. Notify condition variable to wake sleeping thread
	 * 3. Wait for thread to exit its main loop and complete after_stop()
	 * 4. Clean up thread object and stop control mechanism
	 * 5. Update thread_condition_ to Stopped state
	 * 
	 * Thread Safety:
	 * - Safe to call from any thread including the worker thread itself
	 * - Uses proper synchronization to avoid race conditions
	 * - join() ensures thread resources are properly released
	 * 
	 * @return Empty result on success, error if thread wasn't running
	 */
	auto thread_base::stop(void) -> result_void
	{
		// Early exit if no thread to stop (idempotent behavior)
		if (worker_thread_ == nullptr)
		{
			return result_void{error{error_code::thread_not_running, "thread is not running"}};
		}

		// Only attempt to stop if thread is actually joinable
		if (worker_thread_->joinable())
		{
			// Step 1: Signal the thread to stop using platform-specific mechanism
#ifdef USE_STD_JTHREAD
			if (stop_source_.has_value())
			{
				stop_source_.value().request_stop();  // Cooperative cancellation
			}
#else
			stop_requested_ = true;  // Atomic flag for legacy mode
#endif

			// Step 2: Wake up the thread if it's waiting on condition variable
			{
				std::scoped_lock<std::mutex> lock(cv_mutex_);
				worker_condition_.notify_all();  // Wake all waiting threads
			}

			// Step 3: Wait for the thread to complete its shutdown sequence
			worker_thread_->join();  // Blocks until thread exits
		}

		// Step 4: Clean up thread resources
#ifdef USE_STD_JTHREAD
		stop_source_.reset();  // Release stop_source
#endif
		worker_thread_.reset();  // Release thread object

		// Step 5: Update thread state to indicate complete shutdown
		thread_condition_.store(thread_conditions::Stopped);

		return {};
	}

	/**
	 * @brief Checks if the worker thread is currently active.
	 * 
	 * Implementation details:
	 * - Uses the atomic thread_condition_ member instead of checking thread pointers
	 * - This is more reliable as it reflects the actual thread state
	 * - Considers both Working and Waiting states as "running"
	 * - Thread-safe operation due to atomic load
	 * 
	 * Thread States Considered "Running":
	 * - Working: Thread is actively executing do_work()
	 * - Waiting: Thread is alive but waiting for work or timeout
	 * 
	 * Thread States NOT Considered "Running":
	 * - Created: Thread object created but not started
	 * - Stopping: Thread is in shutdown sequence
	 * - Stopped: Thread has completely finished
	 * 
	 * @return true if thread is actively running (Working or Waiting)
	 */
	auto thread_base::is_running(void) const -> bool
	{ 
		// Use the thread_condition_ atomic flag instead of checking the pointer
		auto condition = thread_condition_.load();
		return condition == thread_conditions::Working || 
			   condition == thread_conditions::Waiting;
	}

	/**
	 * @brief Provides a string representation of the thread's current state.
	 * 
	 * Implementation details:
	 * - Uses the formatter utility to create consistent output format
	 * - Includes both thread title and current condition
	 * - Useful for logging and debugging purposes
	 * - Thread-safe due to atomic load of thread_condition_
	 * 
	 * @return Formatted string showing thread title and current state
	 */
	auto thread_base::to_string(void) const -> std::string
	{
		return formatter::format("{} is {}", thread_title_, thread_condition_.load());
	}
} // namespace thread_module