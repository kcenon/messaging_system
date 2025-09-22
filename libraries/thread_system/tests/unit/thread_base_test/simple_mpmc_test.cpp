/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, 🍀☀🌕🌥 🌊
All rights reserved.

This is a simplified test to isolate the segmentation fault issue.

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

#include "gtest/gtest.h"
#include "lockfree_job_queue.h"
#include "callback_job.h"
#include <thread>

using namespace kcenon::thread;

// Test 1: Just create and destroy queue
TEST(SimpleMPMCTest, CreateDestroy)
{
	lockfree_job_queue queue;
}

// Test 2: Create, enqueue one item, destroy
TEST(SimpleMPMCTest, SingleEnqueue)
{
	lockfree_job_queue queue;
	
	auto job = std::make_unique<callback_job>([]() -> result_void {
		return result_void();
	});
	
	auto result = queue.enqueue(std::move(job));
	EXPECT_TRUE(result);
}

// Test 3: Create, enqueue and dequeue one item, destroy
TEST(SimpleMPMCTest, SingleEnqueueDequeue)
{
	lockfree_job_queue queue;
	
	auto job = std::make_unique<callback_job>([]() -> result_void {
		return result_void();
	});
	
	auto enqueue_result = queue.enqueue(std::move(job));
	EXPECT_TRUE(enqueue_result);
	
	auto dequeue_result = queue.dequeue();
	EXPECT_TRUE(dequeue_result.has_value());
}

// Test 4: Multiple queues in sequence
TEST(SimpleMPMCTest, MultipleQueues)
{
	for (int i = 0; i < 3; ++i) {
		lockfree_job_queue queue;
		
		auto job = std::make_unique<callback_job>([]() -> result_void {
			return result_void();
		});
		
		auto enqueue_result = queue.enqueue(std::move(job));
		EXPECT_TRUE(enqueue_result);
		
		auto dequeue_result = queue.dequeue();
		EXPECT_TRUE(dequeue_result.has_value());
	}
}

// Test 5: Thread with queue access
TEST(SimpleMPMCTest, ThreadAccess)
{
	lockfree_job_queue queue;
	
	std::thread t([&queue]() {
		auto job = std::make_unique<callback_job>([]() -> result_void {
			return result_void();
		});
		
		auto result = queue.enqueue(std::move(job));
		(void)result;
	});
	
	t.join();
}
