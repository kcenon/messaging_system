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

#include "thread_base/lockfree/memory/hazard_pointer.h"
#include "logger/core/logger.h"
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <atomic>

using namespace kcenon::thread;

struct TestNode {
    std::atomic<int> data{0};
    std::atomic<TestNode*> next{nullptr};
    
    explicit TestNode(int value) : data(value) {}
};

class LockFreeStack {
private:
    std::atomic<TestNode*> head_{nullptr};
    hazard_pointer_manager& hp_manager_;
    
public:
    explicit LockFreeStack(hazard_pointer_manager& hp_mgr) : hp_manager_(hp_mgr) {}
    
    ~LockFreeStack() {
        while (auto* node = head_.load()) {
            head_.store(node->next.load());
            delete node;
        }
    }
    
    void push(int value) {
        auto* new_node = new TestNode(value);
        auto* old_head = head_.load();
        
        do {
            new_node->next.store(old_head);
        } while (!head_.compare_exchange_weak(old_head, new_node));
    }
    
    bool pop(int& result) {
        auto hp = hp_manager_.acquire();
        
        while (true) {
            auto* head = hp.protect(head_);
            if (!head) {
                return false; // Stack is empty
            }
            
            auto* next = head->next.load();
            
            // Try to update head
            if (head_.compare_exchange_weak(head, next)) {
                result = head->data.load();
                hp_manager_.retire(head);
                return true;
            }
        }
    }
};

void demonstrate_basic_usage() {
    log_module::write_information("\n=== Basic Hazard Pointer Usage Demo ===");
    
    hazard_pointer_manager hp_manager(4, 2); // 4 threads, 2 pointers per thread
    
    // Show initial statistics
    auto stats = hp_manager.get_statistics();
    log_module::write_information("Initial statistics:");
    log_module::write_information("  Active hazard pointers: {}", stats.active_hazard_pointers);
    log_module::write_information("  Retired list size: {}", stats.retired_list_size);
    log_module::write_information("  Total retired: {}", stats.total_retired);
    log_module::write_information("  Total reclaimed: {}", stats.total_reclaimed);
    
    // Create a simple atomic pointer
    std::atomic<TestNode*> test_ptr{new TestNode(42)};
    
    {
        // Acquire hazard pointer and protect the object
        auto hp = hp_manager.acquire();
        auto* protected_ptr = hp.protect(test_ptr);
        
        log_module::write_information("Protected pointer value: {}", protected_ptr->data.load());
        
        // The hazard pointer automatically clears when going out of scope
    }
    
    // Retire the object
    auto* node_to_retire = test_ptr.exchange(nullptr);
    hp_manager.retire(node_to_retire);
    
    // Force reclamation
    hp_manager.scan_and_reclaim();
    
    // Show final statistics
    stats = hp_manager.get_statistics();
    log_module::write_information("Final statistics:");
    log_module::write_information("  Active hazard pointers: {}", stats.active_hazard_pointers);
    log_module::write_information("  Retired list size: {}", stats.retired_list_size);
    log_module::write_information("  Total retired: {}", stats.total_retired);
    log_module::write_information("  Total reclaimed: {}", stats.total_reclaimed);
}

void demonstrate_concurrent_access() {
    log_module::write_information("\n=== Concurrent Access Demo ===");
    
    constexpr int NUM_THREADS = 4;
    constexpr int OPERATIONS_PER_THREAD = 1000;
    
    hazard_pointer_manager hp_manager(NUM_THREADS, 2);
    LockFreeStack stack(hp_manager);
    
    // Fill the stack initially
    for (int i = 0; i < 100; ++i) {
        stack.push(i);
    }
    
    std::atomic<int> push_count{0};
    std::atomic<int> pop_count{0};
    std::atomic<int> failed_pops{0};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);
    
    // Create worker threads
    for (int thread_id = 0; thread_id < NUM_THREADS; ++thread_id) {
        threads.emplace_back([&, thread_id]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 1);
            
            for (int op = 0; op < OPERATIONS_PER_THREAD; ++op) {
                if (dis(gen) == 0) {
                    // Push operation
                    int value = thread_id * OPERATIONS_PER_THREAD + op;
                    stack.push(value);
                    push_count.fetch_add(1);
                } else {
                    // Pop operation
                    int result;
                    if (stack.pop(result)) {
                        pop_count.fetch_add(1);
                    } else {
                        failed_pops.fetch_add(1);
                    }
                }
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    log_module::write_information("Concurrent operations completed in {} ms", duration.count());
    log_module::write_information("Push operations: {}", push_count.load());
    log_module::write_information("Successful pop operations: {}", pop_count.load());
    log_module::write_information("Failed pop operations: {}", failed_pops.load());
    
    // Final statistics
    auto stats = hp_manager.get_statistics();
    log_module::write_information("Final hazard pointer statistics:");
    log_module::write_information("  Active hazard pointers: {}", stats.active_hazard_pointers);
    log_module::write_information("  Retired list size: {}", stats.retired_list_size);
    log_module::write_information("  Total retired: {}", stats.total_retired);
    log_module::write_information("  Total reclaimed: {}", stats.total_reclaimed);
}

void demonstrate_memory_safety() {
    log_module::write_information("\n=== Memory Safety Demo ===");
    
    hazard_pointer_manager hp_manager(2, 1);
    std::atomic<TestNode*> shared_ptr{new TestNode(123)};
    
    std::atomic<bool> reader_done{false};
    std::atomic<bool> writer_done{false};
    
    // Reader thread - tries to access the shared object
    std::thread reader([&]() {
        auto hp = hp_manager.acquire();
        
        for (int i = 0; i < 100; ++i) {
            auto* protected_ptr = hp.protect(shared_ptr);
            if (protected_ptr) {
                // Safe to access the object
                volatile int value = protected_ptr->data.load();
                (void)value; // Prevent optimization
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
            hp.clear();
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        reader_done = true;
    });
    
    // Writer thread - tries to replace and retire the shared object
    std::thread writer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Let reader start
        
        for (int i = 0; i < 10; ++i) {
            auto* new_node = new TestNode(456 + i);
            auto* old_node = shared_ptr.exchange(new_node);
            
            if (old_node) {
                hp_manager.retire(old_node);
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        writer_done = true;
    });
    
    reader.join();
    writer.join();
    
    // Clean up remaining object
    auto* final_node = shared_ptr.exchange(nullptr);
    if (final_node) {
        hp_manager.retire(final_node);
    }
    
    hp_manager.scan_and_reclaim();
    
    auto stats = hp_manager.get_statistics();
    log_module::write_information("Memory safety test completed safely!");
    log_module::write_information("Final statistics:");
    log_module::write_information("  Total retired: {}", stats.total_retired);
    log_module::write_information("  Total reclaimed: {}", stats.total_reclaimed);
}

int main() {
    // Initialize logger
    log_module::start();
    log_module::console_target(log_module::log_types::Information);
    
    log_module::write_information("Hazard Pointer Manager Sample");
    log_module::write_information("=============================");
    
    try {
        demonstrate_basic_usage();
        demonstrate_concurrent_access();
        demonstrate_memory_safety();
        
        log_module::write_information("\n=== All demos completed successfully! ===");
        
    } catch (const std::exception& e) {
        log_module::write_error("Error: {}", e.what());
        log_module::stop();
        return 1;
    }
    
    // Cleanup logger
    log_module::stop();
    return 0;
}