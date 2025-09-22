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

#pragma once

#include <kcenon/thread/core/job.h>
#include "job_types.h"

using namespace kcenon::thread;

namespace kcenon::thread
{
	template <typename job_type> class typed_job_queue_t;

	/**
	 * @class typed_job_t
	 * @brief Represents a job that carries a specific priority level.
	 *
	 * This class extends the base @c job interface to include a priority value
	 * that is used by a priority-based scheduling system. The class also
	 * keeps a weak reference to a @c typed_job_queue_t, which manages
	 * this job. By using a weak pointer, the queue avoids circular references
	 * that could prevent proper resource cleanup.
	 *
	 * @tparam job_type The data type used to represent the priority level.
	 *         Typically, an enum or an integral type.
	 */
	template <typename job_type> class typed_job_t : public job
	{
	public:
		/**
		 * @brief Constructs a new @c typed_job_t object with the given priority and name.
		 *
		 * @param priority The priority level for this job. RealTimeer values could
		 *                 indicate higher priority, depending on your scheduling logic.
		 * @param name     An optional name for the job, useful for debugging or logging.
		 *                 Defaults to "typed_job".
		 */
		typed_job_t(job_type priority, const std::string& name = "typed_job");

		/**
		 * @brief Destroys the @c typed_job_t object.
		 *
		 * The destructor is virtual to ensure proper cleanup in derived classes.
		 */
		~typed_job_t(void) override;

		/**
		 * @brief Retrieves the priority level of this job.
		 *
		 * @return The @c job_type value that indicates this job's priority.
		 */
		[[nodiscard]] auto priority() const -> job_type { return priority_; }

		/**
		 * @brief Associates this job with a particular job queue.
		 *
		 * Internally, this method stores the queue reference as a @c std::weak_ptr
		 * to avoid circular dependencies. Once set, the job can be scheduled and
		 * managed by the provided queue.
		 *
		 * @param job_queue A @c std::shared_ptr to the job queue that will manage
		 *                  this job.
		 */
		auto set_job_queue(const std::shared_ptr<job_queue>& queue) -> void override {
			job::set_job_queue(queue);
			auto typed_queue = std::dynamic_pointer_cast<typed_job_queue_t<job_type>>(queue);
			if (typed_queue) {
				job_queue_ = typed_queue;
			}
		}

		/**
		 * @brief Gets the job queue that currently manages this job, if any.
		 *
		 * Because the queue is stored as a weak pointer, the returned @c shared_ptr
		 * may be empty if the queue is no longer valid.
		 *
		 * @return A @c std::shared_ptr<job_queue> pointing to the job's managing queue,
		 *         or an empty pointer if the queue has expired or was never set.
		 */
		[[nodiscard]] auto get_job_queue(void) const -> std::shared_ptr<job_queue> override {
			auto typed_queue = job_queue_.lock();
			return std::static_pointer_cast<job_queue>(typed_queue);
		}

	private:
		/**
		 * @brief The priority level assigned to this job.
		 */
		job_type priority_;

		/**
		 * @brief A weak pointer to the priority job queue managing this job.
		 *
		 * This prevents a cyclic reference between the job and the queue.
		 */
		std::weak_ptr<typed_job_queue_t<job_type>> job_queue_;
	};

	/**
	 * @typedef typed_job
	 * @brief A convenient alias for @c typed_job_t using the @c job_types type.
	 */
	using typed_job = typed_job_t<job_types>;

} // namespace kcenon::thread

// Note: Template implementation moved inline or will be provided by explicit instantiation
