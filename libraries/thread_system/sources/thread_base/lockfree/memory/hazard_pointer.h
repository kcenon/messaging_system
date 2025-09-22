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
#include <thread>
#include <vector>
#include <array>
#include <functional>
#include <chrono>
#include <memory>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4324) // structure was padded due to alignment specifier
#endif

namespace thread_module
{
	/**
	 * @class hazard_pointer_manager
	 * @brief Thread-safe memory reclamation using hazard pointers
	 * 
	 * This class implements the hazard pointer technique for safe memory
	 * reclamation in lock-free data structures. It prevents the ABA problem
	 * and ensures that memory is not freed while other threads might still
	 * be accessing it.
	 */
	class hazard_pointer_manager
	{
	public:
		/**
		 * @brief Constructor
		 * @param max_threads Maximum number of threads that will use hazard pointers
		 * @param pointers_per_thread Number of hazard pointers per thread
		 */
		explicit hazard_pointer_manager(size_t max_threads = 128, size_t pointers_per_thread = 4);
		
		/**
		 * @brief Destructor - ensures all retired objects are reclaimed
		 */
		~hazard_pointer_manager();
		
		// Delete copy operations
		hazard_pointer_manager(const hazard_pointer_manager&) = delete;
		hazard_pointer_manager& operator=(const hazard_pointer_manager&) = delete;
		
		// Delete move operations
		hazard_pointer_manager(hazard_pointer_manager&&) = delete;
		hazard_pointer_manager& operator=(hazard_pointer_manager&&) = delete;
		
		/**
		 * @class hazard_pointer
		 * @brief RAII wrapper for a hazard pointer slot
		 */
		class hazard_pointer
		{
		public:
			hazard_pointer();
			~hazard_pointer();
			
			// Delete copy operations
			hazard_pointer(const hazard_pointer&) = delete;
			hazard_pointer& operator=(const hazard_pointer&) = delete;
			
			// Allow move operations
			hazard_pointer(hazard_pointer&& other) noexcept;
			hazard_pointer& operator=(hazard_pointer&& other) noexcept;
			
			/**
			 * @brief Protect a pointer from being reclaimed
			 * @tparam T Type of the object being protected
			 * @param atomic_ptr Atomic pointer to protect
			 * @return Protected pointer value
			 */
			template<typename T>
			[[nodiscard]] auto protect(const std::atomic<T*>& atomic_ptr) -> T*
			{
				T* ptr;
				do {
					ptr = atomic_ptr.load(std::memory_order_acquire);
					hp_slot_->store(ptr, std::memory_order_release);
					// Double-check to ensure the pointer hasn't changed
				} while (ptr != atomic_ptr.load(std::memory_order_acquire));
				
				return ptr;
			}
			
			/**
			 * @brief Clear the hazard pointer
			 */
			auto clear() -> void
			{
				if (hp_slot_) {
					hp_slot_->store(nullptr, std::memory_order_release);
				}
			}
			
			/**
			 * @brief Check if the hazard pointer is valid
			 * @return true if valid, false otherwise
			 */
			[[nodiscard]] auto is_valid() const -> bool { return hp_slot_ != nullptr; }
			
		private:
			friend class hazard_pointer_manager;
			std::atomic<void*>* hp_slot_{nullptr};
			
			explicit hazard_pointer(std::atomic<void*>* slot) : hp_slot_(slot) {}
		};
		
		/**
		 * @brief Acquire a hazard pointer
		 * @return RAII hazard pointer object
		 */
		[[nodiscard]] auto acquire() -> hazard_pointer;
		
		/**
		 * @brief Retire a pointer for later reclamation
		 * @tparam T Type of the object to retire
		 * @param ptr Pointer to retire
		 * @param deleter Custom deleter (defaults to delete)
		 */
		template<typename T>
		auto retire(T* ptr, std::function<void(T*)> deleter = std::default_delete<T>{}) -> void
		{
			if (!ptr) return;
			
			retire_impl(ptr, [deleter = std::move(deleter)](void* p) {
				deleter(static_cast<T*>(p));
			});
		}
		
		/**
		 * @brief Force a scan and reclaim cycle
		 */
		auto scan_and_reclaim() -> void;
		
		/**
		 * @brief Get statistics about the hazard pointer system
		 */
		struct statistics
		{
			size_t active_hazard_pointers;
			size_t retired_list_size;
			size_t total_reclaimed;
			size_t total_retired;
		};
		
		[[nodiscard]] auto get_statistics() const -> statistics;
		
	private:
		/**
		 * @struct HazardRecord
		 * @brief Per-thread hazard pointer record
		 */
		struct alignas(64) HazardRecord
		{
			std::atomic<std::thread::id> owner{};
			std::vector<std::atomic<void*>> hazards;
			std::atomic<HazardRecord*> next{nullptr};
			
			explicit HazardRecord(size_t num_pointers) : hazards(num_pointers) {}
		};
		
		/**
		 * @struct RetiredNode
		 * @brief Node in the retired list
		 */
		struct RetiredNode
		{
			void* ptr;
			std::function<void(void*)> deleter;
			std::chrono::steady_clock::time_point retire_time;
			
			RetiredNode(void* p, std::function<void(void*)> d)
				: ptr(p), deleter(std::move(d)), retire_time(std::chrono::steady_clock::now()) {}
		};
		
		// Configuration
		[[maybe_unused]] const size_t max_threads_;
		[[maybe_unused]] const size_t pointers_per_thread_;
		static constexpr size_t RETIRED_THRESHOLD = 64;
		static constexpr auto SCAN_INTERVAL = std::chrono::milliseconds(100);
		
		// Hazard pointer records
		alignas(64) std::atomic<HazardRecord*> head_record_{nullptr};
		
		// Thread-local data
		static thread_local HazardRecord* local_record_;
		static thread_local std::vector<RetiredNode> retired_list_;
		static thread_local std::chrono::steady_clock::time_point last_scan_;
		
		// Statistics
		mutable std::atomic<size_t> total_retired_{0};
		mutable std::atomic<size_t> total_reclaimed_{0};
		
		// Internal methods
		auto acquire_record() -> HazardRecord*;
		auto release_record(HazardRecord* record) -> void;
		auto acquire_slot() -> std::atomic<void*>*;
		auto release_slot(std::atomic<void*>* slot) -> void;
		auto retire_impl(void* ptr, std::function<void(void*)> deleter) -> void;
		auto collect_hazard_pointers() const -> std::vector<void*>;
		auto should_scan() const -> bool;
	};
	
	// Thread-local static member declarations
	// These are defined in hazard_pointer.cpp to avoid multiple definition errors in MinGW
	#ifndef __MINGW32__
	inline thread_local hazard_pointer_manager::HazardRecord* hazard_pointer_manager::local_record_ = nullptr;
	inline thread_local std::vector<hazard_pointer_manager::RetiredNode> hazard_pointer_manager::retired_list_;
	inline thread_local std::chrono::steady_clock::time_point hazard_pointer_manager::last_scan_;
	#endif

} // namespace thread_module

#ifdef _MSC_VER
#pragma warning(pop)
#endif