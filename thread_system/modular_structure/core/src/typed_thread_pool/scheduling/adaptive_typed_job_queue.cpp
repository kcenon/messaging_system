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

#include "thread_system_core/typed_thread_pool/scheduling/adaptive_typed_job_queue.h"

/**
 * @file adaptive_typed_job_queue.cpp
 * @brief Explicit template instantiation for typed job queue with job_types enumeration.
 *
 * This file provides explicit template instantiation for the adaptive_typed_job_queue_t
 * template class and factory function with the job_types enumeration. This approach
 * separates template compilation from header inclusion, improving compilation times
 * and reducing binary size.
 * 
 * Template Instantiation Benefits:
 * - Faster compilation times for client code using typed queues
 * - Reduced binary size through single instantiation point
 * - Better error localization for template-related issues
 * - Cleaner separation between interface and implementation
 * - Improved build system dependency management
 * 
 * Instantiated Components:
 * - adaptive_typed_job_queue_t<job_types>: Main queue implementation
 * - create_typed_job_queue<job_types>: Factory function for queue creation
 * - All supported queue strategies and configurations
 * 
 * Usage Pattern:
 * - Client code includes only the header file
 * - Template implementation is pre-compiled in this unit
 * - Linker resolves template instantiation automatically
 * - Type safety maintained through template parameter validation
 */

namespace typed_thread_pool_module
{
	/**
	 * @brief Explicit instantiation of adaptive typed job queue for job_types.
	 * 
	 * This instantiation creates a concrete implementation of the adaptive
	 * typed job queue template for the job_types enumeration, enabling
	 * efficient job categorization and routing in typed thread pools.
	 * 
	 * Features instantiated:
	 * - Lock-free and mutex-based queue implementations
	 * - Adaptive strategy selection based on contention
	 * - Type-safe job categorization using job_types enum
	 * - High-performance concurrent access patterns
	 */
	template class adaptive_typed_job_queue_t<job_types>;
	
	/**
	 * @brief Explicit instantiation of typed job queue factory function.
	 * 
	 * This instantiation provides the factory function for creating
	 * typed job queues with specified strategies and configurations.
	 * The factory handles queue strategy selection and initialization.
	 * 
	 * Parameters instantiated:
	 * - queue_strategy: Strategy selection (lock-free, mutex, adaptive)
	 * - size_t: Maximum queue capacity or thread count
	 * 
	 * @return Shared pointer to initialized typed job queue
	 */
	template auto create_typed_job_queue<job_types>(
		adaptive_typed_job_queue_t<job_types>::queue_strategy, 
		size_t) -> std::shared_ptr<typed_job_queue_t<job_types>>;

} // namespace typed_thread_pool_module