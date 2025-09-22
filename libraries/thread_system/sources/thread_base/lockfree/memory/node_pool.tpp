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

namespace thread_module
{
	template<typename T>
	node_pool<T>::node_pool(size_t initial_chunks, size_t chunk_size)
		: chunk_size_(std::max(std::min(chunk_size, MAX_CHUNK_SIZE), MIN_CHUNK_SIZE))
	{
		reserve(initial_chunks);
	}
	
	template<typename T>
	node_pool<T>::~node_pool()
	{
		// Clean up all chunks
		auto* chunk = current_chunk_.load(std::memory_order_acquire);
		while (chunk) {
			auto* next = chunk->next;
			delete chunk;
			chunk = next;
		}
	}
	
	template<typename T>
	auto node_pool<T>::allocate() -> T*
	{
		// Try global free list first
		if (T* node = pop_from_free_list()) {
			return node;
		}
		
		// Allocate from current chunk
		auto* chunk = current_chunk_.load(std::memory_order_acquire);
		while (chunk) {
			if (T* node = allocate_from_chunk(chunk)) {
				allocated_nodes_.fetch_add(1, std::memory_order_relaxed);
				return node;
			}
			
			// Current chunk is full, try to allocate new chunk
			auto* new_chunk = allocate_new_chunk();
			new_chunk->next = chunk;
			
			// Try to update current chunk
			if (current_chunk_.compare_exchange_weak(chunk, new_chunk,
													  std::memory_order_release,
													  std::memory_order_acquire)) {
				chunk = new_chunk;
			} else {
				// Another thread already allocated a new chunk
				delete new_chunk;
				chunk = current_chunk_.load(std::memory_order_acquire);
			}
		}
		
		// Should never reach here if allocation succeeded
		throw std::bad_alloc();
	}
	
	template<typename T>
	auto node_pool<T>::deallocate(T* node) -> void
	{
		if (!node) return;
		
		// Push to global free list
		push_to_free_list(node);
		allocated_nodes_.fetch_sub(1, std::memory_order_relaxed);
	}
	
	template<typename T>
	auto node_pool<T>::get_statistics() const -> statistics
	{
		statistics stats;
		stats.total_chunks = total_chunks_.load(std::memory_order_relaxed);
		stats.total_nodes = total_nodes_.load(std::memory_order_relaxed);
		stats.allocated_nodes = allocated_nodes_.load(std::memory_order_relaxed);
		stats.free_list_size = free_list_size_.load(std::memory_order_relaxed);
		
		return stats;
	}
	
	template<typename T>
	auto node_pool<T>::reserve(size_t num_chunks) -> void
	{
		for (size_t i = 0; i < num_chunks; ++i) {
			auto* new_chunk = allocate_new_chunk();
			
			// Add to chunk list
			auto* current = current_chunk_.load(std::memory_order_acquire);
			do {
				new_chunk->next = current;
			} while (!current_chunk_.compare_exchange_weak(current, new_chunk,
														   std::memory_order_release,
														   std::memory_order_acquire));
		}
	}
	
	template<typename T>
	auto node_pool<T>::allocate_new_chunk() -> PoolChunk*
	{
		auto* chunk = new PoolChunk(chunk_size_);
		total_chunks_.fetch_add(1, std::memory_order_relaxed);
		total_nodes_.fetch_add(chunk_size_, std::memory_order_relaxed);
		return chunk;
	}
	
	template<typename T>
	auto node_pool<T>::allocate_from_chunk(PoolChunk* chunk) -> T*
	{
		size_t index = chunk->allocation_index.fetch_add(1, std::memory_order_acq_rel);
		if (index < chunk->capacity) {
			return &chunk->nodes[index];
		}
		return nullptr;
	}
	
	template<typename T>
	auto node_pool<T>::push_to_free_list(T* node) -> void
	{
		auto* free_node = to_free_node(node);
		auto* head = free_list_.load(std::memory_order_acquire);
		
		do {
			free_node->next.store(head, std::memory_order_relaxed);
		} while (!free_list_.compare_exchange_weak(head, free_node,
												   std::memory_order_release,
												   std::memory_order_acquire));
		
		free_list_size_.fetch_add(1, std::memory_order_relaxed);
	}
	
	template<typename T>
	auto node_pool<T>::pop_from_free_list() -> T*
	{
		auto* head = free_list_.load(std::memory_order_acquire);
		
		while (head) {
			auto* next = head->next.load(std::memory_order_acquire);
			if (free_list_.compare_exchange_weak(head, next,
												 std::memory_order_release,
												 std::memory_order_acquire)) {
				free_list_size_.fetch_sub(1, std::memory_order_relaxed);
				return from_free_node(head);
			}
		}
		
		return nullptr;
	}
	
	template<typename T>
	auto node_pool<T>::to_free_node(T* node) -> FreeNode*
	{
		// Use placement new to convert T* to FreeNode*
		// This is safe because FreeNode only contains an atomic pointer
		return reinterpret_cast<FreeNode*>(node);
	}
	
	template<typename T>
	auto node_pool<T>::from_free_node(FreeNode* free_node) -> T*
	{
		// Convert back from FreeNode* to T*
		return reinterpret_cast<T*>(free_node);
	}

} // namespace thread_module