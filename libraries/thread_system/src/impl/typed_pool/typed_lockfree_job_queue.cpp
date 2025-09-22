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

#include "typed_lockfree_job_queue.h"

/**
 * @file typed_lockfree_job_queue.cpp
 * @brief Explicit template instantiation for typed lock-free job queue.
 *
 * This file provides explicit template instantiation for the typed_lockfree_job_queue_t
 * template class with the job_types enumeration. This approach separates template
 * compilation from header inclusion, reducing compile times and binary size.
 * 
 * Key Features:
 * - Explicit template instantiation for job_types enumeration
 * - Reduces template instantiation overhead in client code
 * - Centralizes template compilation for the typed job queue
 * - Maintains type safety while improving build performance
 * 
 * Template Instantiation Benefits:
 * - Faster compilation times for client code
 * - Reduced binary size through single instantiation
 * - Better error localization for template-related issues
 * - Cleaner separation of interface and implementation
 * 
 * Usage:
 * - Client code includes only the header file
 * - Template implementation is pre-compiled in this unit
 * - Linker resolves template instantiation automatically
 * - Type safety maintained through template parameter validation
 */

namespace kcenon::thread
{
	/**
	 * @brief Explicit template instantiation for job_types enumeration.
	 * 
	 * This instantiation creates a concrete implementation of the
	 * typed_lockfree_job_queue_t template for the job_types enum,
	 * enabling efficient job categorization and routing in the
	 * typed thread pool system.
	 * 
	 * Instantiated Features:
	 * - Lock-free MPMC queue operations
	 * - Type-safe job categorization using job_types
	 * - Hazard pointer-based memory management
	 * - High-performance concurrent access
	 */
	template class typed_lockfree_job_queue_t<job_types>;
}