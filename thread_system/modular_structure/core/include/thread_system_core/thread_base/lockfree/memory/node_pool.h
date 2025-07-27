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

#include <atomic>
#include <vector>
#include <memory>
#include <array>
#include <type_traits>
#include <new>
#include <mutex>
#include <unordered_set>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#endif

namespace thread_module
{
	/**
	 * @class node_pool
	 * @brief High-performance memory pool for lock-free data structures
	 * 
	 * This class provides a thread-safe memory pool optimized for allocating
	 * and deallocating nodes in lock-free data structures. It uses:
	 * - Lock-free free list for global pool (no thread-local storage)
	 * - Chunk-based allocation for better locality
	 * - Cache-line aligned structures to reduce false sharing
	 * 
	 * @tparam T Type of objects to pool (must be default constructible)
	 */
	template<typename T>
	class node_pool
	{
		static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
		
	public:
		/**
		 * @brief Constructor
		 * @param initial_chunks Number of chunks to pre-allocate (default: 1)
		 * @param chunk_size Number of nodes per chunk (default: 256)
		 */
		explicit node_pool(size_t initial_chunks = 1, size_t chunk_size = 256);
		
		/**
		 * @brief Destructor
		 */
		~node_pool();
		
		// Delete copy operations
		node_pool(const node_pool&) = delete;
		node_pool& operator=(const node_pool&) = delete;
		
		// Delete move operations
		node_pool(node_pool&&) = delete;
		node_pool& operator=(node_pool&&) = delete;
		
		/**
		 * @brief Allocate a node from the pool
		 * @return Pointer to allocated node (never null)
		 * @throws std::bad_alloc if allocation fails
		 */
		[[nodiscard]] auto allocate() -> T*;
		
		/**
		 * @brief Deallocate a node back to the pool
		 * @param node Pointer to deallocate (can be null)
		 */
		auto deallocate(T* node) -> void;
		
		/**
		 * @brief Get pool statistics
		 */
		struct statistics
		{
			size_t total_chunks;
			size_t total_nodes;
			size_t allocated_nodes;
			size_t free_list_size;
		};
		
		[[nodiscard]] auto get_statistics() const -> statistics;
		
		/**
		 * @brief Pre-allocate additional chunks
		 * @param num_chunks Number of chunks to allocate
		 */
		auto reserve(size_t num_chunks) -> void;
		
	private:
		static constexpr size_t CACHE_LINE_SIZE = 64;
		static constexpr size_t MIN_CHUNK_SIZE = 64;
		static constexpr size_t MAX_CHUNK_SIZE = 8192;
		static constexpr size_t DEFAULT_CHUNK_SIZE = 256;
		static constexpr size_t DEFAULT_INITIAL_CHUNKS = 1;
		
		/**
		 * @struct PoolChunk
		 * @brief A chunk of pre-allocated nodes
		 */
		struct alignas(CACHE_LINE_SIZE) PoolChunk
		{
			std::unique_ptr<T[]> nodes;
			std::atomic<size_t> allocation_index{0};
			PoolChunk* next{nullptr};
			const size_t capacity;
			
			explicit PoolChunk(size_t size) 
				: nodes(std::make_unique<T[]>(size)), capacity(size) {}
		};
		
		/**
		 * @struct FreeNode
		 * @brief Node in the free list
		 */
		struct FreeNode
		{
			std::atomic<FreeNode*> next{nullptr};
		};
		
		// Configuration
		const size_t chunk_size_;
		
		// Global free list (lock-free stack)
		alignas(CACHE_LINE_SIZE) std::atomic<FreeNode*> free_list_{nullptr};
		
		// Chunk management
		alignas(CACHE_LINE_SIZE) std::atomic<PoolChunk*> current_chunk_{nullptr};
		alignas(CACHE_LINE_SIZE) std::atomic<size_t> total_chunks_{0};
		alignas(CACHE_LINE_SIZE) std::atomic<size_t> total_nodes_{0};
		alignas(CACHE_LINE_SIZE) std::atomic<size_t> allocated_nodes_{0};
		
		// Free list size tracking for statistics
		alignas(CACHE_LINE_SIZE) std::atomic<size_t> free_list_size_{0};
		
		// Internal methods
		auto allocate_new_chunk() -> PoolChunk*;
		auto allocate_from_chunk(PoolChunk* chunk) -> T*;
		auto push_to_free_list(T* node) -> void;
		auto pop_from_free_list() -> T*;
		auto to_free_node(T* node) -> FreeNode*;
		auto from_free_node(FreeNode* free_node) -> T*;
	};

} // namespace thread_module

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// Include template implementation
#include "thread_system_core/thread_base/lockfree/memory/node_pool.tpp"