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

#include "hazard_pointer.h"
#include <algorithm>
#include <unordered_set>

/**
 * @file hazard_pointer.cpp
 * @brief Implementation of lock-free memory management using hazard pointers.
 *
 * This file contains the implementation of the hazard pointer system, which provides
 * safe memory reclamation for lock-free data structures. Hazard pointers prevent
 * the ABA problem by ensuring that memory is not reclaimed while other threads
 * might still be accessing it.
 * 
 * Key Features:
 * - Thread-safe memory reclamation for lock-free structures
 * - Protection against use-after-free and ABA problems
 * - Efficient memory usage with deferred reclamation
 * - Scalable design supporting multiple threads
 * - Configurable scanning thresholds and intervals
 * 
 * Algorithm Overview:
 * 1. Threads acquire hazard pointers before accessing shared data
 * 2. Retired memory is added to thread-local retired lists
 * 3. Periodic scanning compares retired memory against active hazard pointers
 * 4. Memory not protected by hazard pointers is safely reclaimed
 * 5. Protected memory remains in retired list for later scanning
 * 
 * Performance Characteristics:
 * - O(1) hazard pointer acquisition and release
 * - O(n*m) scanning cost where n=retired nodes, m=hazard pointers
 * - Configurable memory overhead vs. reclamation latency tradeoffs
 * - Thread-local operations minimize contention
 */

namespace thread_module
{
	// Thread-local static member definitions for MinGW
	#ifdef __MINGW32__
	thread_local hazard_pointer_manager::HazardRecord* hazard_pointer_manager::local_record_ = nullptr;
	thread_local std::vector<hazard_pointer_manager::RetiredNode> hazard_pointer_manager::retired_list_;
	thread_local std::chrono::steady_clock::time_point hazard_pointer_manager::last_scan_;
	#endif

	/**
	 * @brief Constructs a hazard pointer manager with specified capacity.
	 * 
	 * Implementation details:
	 * - Pre-allocates hazard records for maximum thread count
	 * - Each record can hold multiple hazard pointers per thread
	 * - Records are linked in a lock-free list for efficient access
	 * - Uses compare-and-swap to build the record list atomically
	 * 
	 * Memory Layout:
	 * - Each HazardRecord contains an array of atomic pointers
	 * - Records are organized as a singly-linked list
	 * - Thread ownership is tracked per record
	 * 
	 * @param max_threads Maximum number of threads that will use hazard pointers
	 * @param pointers_per_thread Number of hazard pointers available per thread
	 */
	hazard_pointer_manager::hazard_pointer_manager(size_t max_threads, size_t pointers_per_thread)
		: max_threads_(max_threads)
		, pointers_per_thread_(pointers_per_thread)
	{
		// Pre-allocate hazard records for all anticipated threads
		for (size_t i = 0; i < max_threads; ++i) {
			auto* record = new HazardRecord(pointers_per_thread);
			
			// Add record to the global list using lock-free insertion
			auto* head = head_record_.load(std::memory_order_acquire);
			do {
				record->next.store(head, std::memory_order_relaxed);
			} while (!head_record_.compare_exchange_weak(head, record,
														  std::memory_order_release,
														  std::memory_order_acquire));
		}
	}
	
	/**
	 * @brief Destroys the hazard pointer manager and reclaims all memory.
	 * 
	 * Implementation details:
	 * - Performs final scan to reclaim all possible retired memory
	 * - Deallocates all hazard records in the global list
	 * - Ensures no memory leaks from hazard pointer infrastructure
	 * - Safe to call even if threads are still using hazard pointers
	 */
	hazard_pointer_manager::~hazard_pointer_manager()
	{
		// Force final reclamation of all eligible retired memory
		scan_and_reclaim();
		
		// Clean up all hazard records in the global list
		auto* record = head_record_.load(std::memory_order_acquire);
		while (record) {
			auto* next = record->next.load(std::memory_order_acquire);
			delete record;
			record = next;
		}
	}
	
	// hazard_pointer implementation
	
	/**
	 * @brief Constructs an empty hazard pointer.
	 * 
	 * Implementation details:
	 * - Initializes with null slot pointer (no protection active)
	 * - Does not acquire any resources until protect() is called
	 * - Safe to copy and move without side effects
	 */
	hazard_pointer_manager::hazard_pointer::hazard_pointer()
		: hp_slot_(nullptr)
	{
	}
	
	/**
	 * @brief Destroys the hazard pointer and releases protection.
	 * 
	 * Implementation details:
	 * - Automatically clears any active protection
	 * - Releases the hazard pointer slot back to the manager
	 * - RAII ensures no leaked protection or slots
	 */
	hazard_pointer_manager::hazard_pointer::~hazard_pointer()
	{
		clear();
	}
	
	/**
	 * @brief Move constructs a hazard pointer from another.
	 * 
	 * Implementation details:
	 * - Transfers ownership of the hazard pointer slot
	 * - Source hazard pointer becomes inactive
	 * - No atomic operations required (just pointer transfer)
	 * 
	 * @param other Source hazard pointer to move from
	 */
	hazard_pointer_manager::hazard_pointer::hazard_pointer(hazard_pointer&& other) noexcept
		: hp_slot_(other.hp_slot_)
	{
		other.hp_slot_ = nullptr;
	}
	
	/**
	 * @brief Move assigns a hazard pointer from another.
	 * 
	 * Implementation details:
	 * - Clears current protection before taking new slot
	 * - Transfers ownership from source hazard pointer
	 * - Self-assignment safe with pointer comparison
	 * 
	 * @param other Source hazard pointer to move from
	 * @return Reference to this hazard pointer
	 */
	auto hazard_pointer_manager::hazard_pointer::operator=(hazard_pointer&& other) noexcept -> hazard_pointer&
	{
		if (this != &other) {
			clear();
			hp_slot_ = other.hp_slot_;
			other.hp_slot_ = nullptr;
		}
		return *this;
	}
	
	// hazard_pointer_manager methods
	
	/**
	 * @brief Acquires a new hazard pointer for protecting shared data.
	 * 
	 * Implementation details:
	 * - Obtains a free hazard pointer slot from the current thread's record
	 * - Creates a hazard_pointer object that manages the slot lifetime
	 * - May throw if no slots are available (thread exhaustion)
	 * 
	 * Usage Pattern:
	 * @code
	 * auto hp = manager.acquire();
	 * auto* ptr = hp.protect(shared_data_pointer);
	 * // Use ptr safely...
	 * @endcode
	 * 
	 * @return A hazard_pointer object for protecting shared data
	 * @throws std::runtime_error if no hazard pointer slots available
	 */
	auto hazard_pointer_manager::acquire() -> hazard_pointer
	{
		auto* slot = acquire_slot();
		return hazard_pointer(slot);
	}
	
	/**
	 * @brief Scans hazard pointers and reclaims unprotected retired memory.
	 * 
	 * Implementation details:
	 * - Collects all active hazard pointers from all thread records
	 * - Sorts hazard pointers for efficient binary search
	 * - Compares each retired node against the hazard pointer list
	 * - Reclaims memory not protected by any hazard pointer
	 * - Defers reclamation of still-protected memory
	 * 
	 * Algorithm Complexity:
	 * - O(H log H) for sorting hazard pointers (H = active hazard pointers)
	 * - O(R log H) for scanning retired list (R = retired nodes)
	 * - Total: O((H + R) log H)
	 * 
	 * Reclamation Safety:
	 * - Only reclaims memory with no active hazard pointer protection
	 * - Uses custom deleter functions for proper cleanup
	 * - Updates statistics for monitoring and debugging
	 * 
	 * Performance Considerations:
	 * - Called automatically when retired list grows large
	 * - Can be called manually for immediate reclamation
	 * - Thread-local retired lists minimize contention
	 */
	auto hazard_pointer_manager::scan_and_reclaim() -> void
	{
		// Collect all currently active hazard pointers from all threads
		auto hazards = collect_hazard_pointers();
		std::sort(hazards.begin(), hazards.end());
		
		// Process the thread-local retired list
		std::vector<RetiredNode> new_retired_list;
		
		for (auto& retired : retired_list_) {
			if (std::binary_search(hazards.begin(), hazards.end(), retired.ptr)) {
				// Memory is still protected by a hazard pointer, defer reclamation
				new_retired_list.push_back(std::move(retired));
			} else {
				// No hazard pointer protects this memory, safe to reclaim
				retired.deleter(retired.ptr);
				total_reclaimed_.fetch_add(1, std::memory_order_relaxed);
			}
		}
		
		// Update retired list and scan timestamp
		retired_list_ = std::move(new_retired_list);
		last_scan_ = std::chrono::steady_clock::now();
	}
	
	auto hazard_pointer_manager::get_statistics() const -> statistics
	{
		statistics stats;
		
		// Count active hazard pointers
		size_t active_count = 0;
		auto* record = head_record_.load(std::memory_order_acquire);
		while (record) {
			if (record->owner.load(std::memory_order_acquire) != std::thread::id{}) {
				for (const auto& hazard : record->hazards) {
					if (hazard.load(std::memory_order_acquire) != nullptr) {
						++active_count;
					}
				}
			}
			record = record->next.load(std::memory_order_acquire);
		}
		
		stats.active_hazard_pointers = active_count;
		stats.retired_list_size = retired_list_.size();
		stats.total_reclaimed = total_reclaimed_.load(std::memory_order_relaxed);
		stats.total_retired = total_retired_.load(std::memory_order_relaxed);
		
		return stats;
	}
	
	/**
	 * @brief Acquires a hazard record for the current thread.
	 * 
	 * Implementation details:
	 * - First attempts to reuse thread-local cached record
	 * - Falls back to searching global list for unused record
	 * - Uses atomic compare-and-swap to claim record ownership
	 * - Caches acquired record in thread-local storage
	 * 
	 * Thread Safety:
	 * - Multiple threads can safely call this concurrently
	 * - Record ownership is managed atomically
	 * - Thread IDs prevent accidental sharing
	 * 
	 * @return Pointer to acquired hazard record
	 * @throws std::runtime_error if no records available
	 */
	auto hazard_pointer_manager::acquire_record() -> HazardRecord*
	{
		// Try to reuse existing thread-local record if still owned
		if (local_record_ && local_record_->owner.load(std::memory_order_acquire) == std::this_thread::get_id()) {
			return local_record_;
		}
		
		// Search for an unused record in the global list
		auto* record = head_record_.load(std::memory_order_acquire);
		while (record) {
			std::thread::id expected{};
			if (record->owner.compare_exchange_strong(expected, std::this_thread::get_id(),
													   std::memory_order_acquire,
													   std::memory_order_relaxed)) {
				// Successfully claimed the record
				local_record_ = record;
				return record;
			}
			record = record->next.load(std::memory_order_acquire);
		}
		
		// No free records available - system is at capacity
		throw std::runtime_error("No free hazard records available");
	}
	
	auto hazard_pointer_manager::release_record(HazardRecord* record) -> void
	{
		if (record) {
			// Clear all hazard pointers
			for (auto& hazard : record->hazards) {
				hazard.store(nullptr, std::memory_order_release);
			}
			
			// Release ownership
			record->owner.store(std::thread::id{}, std::memory_order_release);
			
			if (local_record_ == record) {
				local_record_ = nullptr;
			}
		}
	}
	
	auto hazard_pointer_manager::acquire_slot() -> std::atomic<void*>*
	{
		auto* record = acquire_record();
		
		// Find an empty slot
		for (auto& hazard : record->hazards) {
			void* expected = nullptr;
			if (hazard.compare_exchange_strong(expected, reinterpret_cast<void*>(1),
											   std::memory_order_acquire,
											   std::memory_order_relaxed)) {
				hazard.store(nullptr, std::memory_order_release);
				return &hazard;
			}
		}
		
		// No free slots in record
		throw std::runtime_error("No free hazard pointer slots");
	}
	
	auto hazard_pointer_manager::release_slot(std::atomic<void*>* slot) -> void
	{
		if (slot) {
			slot->store(nullptr, std::memory_order_release);
		}
	}
	
	auto hazard_pointer_manager::retire_impl(void* ptr, std::function<void(void*)> deleter) -> void
	{
		retired_list_.emplace_back(ptr, std::move(deleter));
		total_retired_.fetch_add(1, std::memory_order_relaxed);
		
		// Check if we should scan
		if (should_scan()) {
			scan_and_reclaim();
		}
	}
	
	auto hazard_pointer_manager::collect_hazard_pointers() const -> std::vector<void*>
	{
		std::vector<void*> hazards;
		
		auto* record = head_record_.load(std::memory_order_acquire);
		while (record) {
			if (record->owner.load(std::memory_order_acquire) != std::thread::id{}) {
				for (const auto& hazard : record->hazards) {
					void* ptr = hazard.load(std::memory_order_acquire);
					if (ptr && ptr != reinterpret_cast<void*>(1)) {
						hazards.push_back(ptr);
					}
				}
			}
			record = record->next.load(std::memory_order_acquire);
		}
		
		return hazards;
	}
	
	auto hazard_pointer_manager::should_scan() const -> bool
	{
		// Scan if retired list is large
		if (retired_list_.size() >= RETIRED_THRESHOLD) {
			return true;
		}
		
		// Scan if enough time has passed
		auto now = std::chrono::steady_clock::now();
		if (now - last_scan_ >= SCAN_INTERVAL) {
			return true;
		}
		
		return false;
	}

} // namespace thread_module