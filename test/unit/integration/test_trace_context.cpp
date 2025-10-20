// Unit tests for TraceContext propagation

#include "messaging_system/integration/trace_context.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <vector>

using namespace messaging;

void test_generate_trace_id() {
    std::cout << "Test: Generate trace ID..." << std::endl;

    auto trace_id1 = TraceContext::generate_trace_id();
    auto trace_id2 = TraceContext::generate_trace_id();

    assert(!trace_id1.empty() && "Trace ID should not be empty");
    assert(!trace_id2.empty() && "Trace ID should not be empty");
    assert(trace_id1 != trace_id2 && "Trace IDs should be unique");

    // Check format (should contain hyphen)
    assert(trace_id1.find('-') != std::string::npos && "Trace ID should contain hyphen");

    std::cout << "  Generated trace IDs: " << trace_id1 << ", " << trace_id2 << std::endl;
    std::cout << "  ✓ Passed" << std::endl;
}

void test_set_get_trace_id() {
    std::cout << "Test: Set and get trace ID..." << std::endl;

    const std::string test_trace_id = "test-trace-12345";

    TraceContext::set_trace_id(test_trace_id);
    auto retrieved = TraceContext::get_trace_id();

    assert(retrieved == test_trace_id && "Should retrieve set trace ID");

    TraceContext::clear();
    auto after_clear = TraceContext::get_trace_id();
    assert(after_clear.empty() && "Trace ID should be empty after clear");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_thread_local_isolation() {
    std::cout << "Test: Thread-local isolation..." << std::endl;

    const std::string main_trace_id = "main-thread-trace";
    TraceContext::set_trace_id(main_trace_id);

    std::string thread_trace_id;
    std::thread worker([&]() {
        // Thread should not see main thread's trace ID
        auto initial = TraceContext::get_trace_id();
        assert(initial.empty() && "New thread should have empty trace ID");

        // Set thread-specific trace ID
        const std::string worker_trace = "worker-thread-trace";
        TraceContext::set_trace_id(worker_trace);

        thread_trace_id = TraceContext::get_trace_id();
    });

    worker.join();

    // Main thread should still have its own trace ID
    auto main_after = TraceContext::get_trace_id();
    assert(main_after == main_trace_id && "Main thread trace ID should be unchanged");
    assert(thread_trace_id == "worker-thread-trace" && "Worker thread had correct trace ID");

    TraceContext::clear();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_scoped_trace_basic() {
    std::cout << "Test: ScopedTrace basic functionality..." << std::endl;

    TraceContext::clear();

    const std::string trace_id = "scoped-trace-test";

    {
        ScopedTrace scope(trace_id);
        auto current = TraceContext::get_trace_id();
        assert(current == trace_id && "Should set trace ID in scope");
    }

    // After scope, trace ID should be restored (to empty)
    auto after_scope = TraceContext::get_trace_id();
    assert(after_scope.empty() && "Trace ID should be restored after scope");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_scoped_trace_nesting() {
    std::cout << "Test: ScopedTrace nesting..." << std::endl;

    TraceContext::clear();

    const std::string outer_trace = "outer-trace";
    const std::string inner_trace = "inner-trace";

    {
        ScopedTrace outer(outer_trace);
        assert(TraceContext::get_trace_id() == outer_trace && "Outer scope should set trace ID");

        {
            ScopedTrace inner(inner_trace);
            assert(TraceContext::get_trace_id() == inner_trace && "Inner scope should override trace ID");
        }

        // After inner scope, should restore outer trace ID
        assert(TraceContext::get_trace_id() == outer_trace && "Should restore outer trace ID");
    }

    // After all scopes, should be empty
    assert(TraceContext::get_trace_id().empty() && "Should be empty after all scopes");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_scoped_trace_with_existing_id() {
    std::cout << "Test: ScopedTrace with existing trace ID..." << std::endl;

    const std::string initial_trace = "initial-trace";
    const std::string new_trace = "new-trace";

    TraceContext::set_trace_id(initial_trace);

    {
        ScopedTrace scope(new_trace);
        assert(TraceContext::get_trace_id() == new_trace && "Should override with new trace ID");
    }

    // Should restore initial trace ID
    assert(TraceContext::get_trace_id() == initial_trace && "Should restore initial trace ID");

    TraceContext::clear();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_multiple_threads_with_scoped_trace() {
    std::cout << "Test: Multiple threads with ScopedTrace..." << std::endl;

    const int num_threads = 4;
    std::vector<std::thread> threads;
    std::vector<std::string> captured_trace_ids(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            std::string trace_id = "thread-" + std::to_string(i) + "-trace";

            ScopedTrace scope(trace_id);
            captured_trace_ids[i] = TraceContext::get_trace_id();

            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            // Verify trace ID is still correct
            assert(TraceContext::get_trace_id() == trace_id &&
                   "Trace ID should remain consistent within thread");
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Verify each thread had its own trace ID
    for (int i = 0; i < num_threads; ++i) {
        std::string expected = "thread-" + std::to_string(i) + "-trace";
        assert(captured_trace_ids[i] == expected &&
               "Each thread should have had its own trace ID");
    }

    std::cout << "  ✓ Passed" << std::endl;
}

void test_trace_id_propagation_pattern() {
    std::cout << "Test: Trace ID propagation pattern (simulated async)..." << std::endl;

    // Simulate pattern: main thread creates trace, worker thread continues it
    const std::string original_trace = TraceContext::generate_trace_id();

    // Main thread sets trace
    TraceContext::set_trace_id(original_trace);

    // Capture trace ID for async operation
    std::string captured_trace = TraceContext::get_trace_id();

    std::string worker_saw_trace;
    std::thread worker([&]() {
        // Worker restores the captured trace ID
        ScopedTrace scope(captured_trace);
        worker_saw_trace = TraceContext::get_trace_id();
    });

    worker.join();

    // Worker should have seen the same trace ID
    assert(worker_saw_trace == original_trace &&
           "Worker should have restored the original trace ID");

    TraceContext::clear();
    std::cout << "  ✓ Passed" << std::endl;
}

void test_scoped_trace_move_semantics() {
    std::cout << "Test: ScopedTrace move semantics..." << std::endl;

    TraceContext::clear();

    const std::string trace_id = "move-test-trace";

    // Create and move ScopedTrace
    auto create_scope = [&]() -> ScopedTrace {
        return ScopedTrace(trace_id);
    };

    {
        ScopedTrace scope = create_scope();
        assert(TraceContext::get_trace_id() == trace_id &&
               "Moved ScopedTrace should work correctly");
    }

    assert(TraceContext::get_trace_id().empty() &&
           "Trace ID should be cleared after moved scope");

    std::cout << "  ✓ Passed" << std::endl;
}

void test_empty_trace_id_handling() {
    std::cout << "Test: Empty trace ID handling..." << std::endl;

    TraceContext::clear();

    // Setting empty trace ID
    TraceContext::set_trace_id("");
    assert(TraceContext::get_trace_id().empty() && "Empty trace ID should be handled");

    // ScopedTrace with empty trace ID
    {
        ScopedTrace scope("");
        assert(TraceContext::get_trace_id().empty() && "ScopedTrace with empty ID should work");
    }

    assert(TraceContext::get_trace_id().empty() && "Should remain empty after scope");

    std::cout << "  ✓ Passed" << std::endl;
}

int main() {
    std::cout << "=== TraceContext Unit Tests ===" << std::endl;
    std::cout << std::endl;

    try {
        test_generate_trace_id();
        test_set_get_trace_id();
        test_thread_local_isolation();
        test_scoped_trace_basic();
        test_scoped_trace_nesting();
        test_scoped_trace_with_existing_id();
        test_multiple_threads_with_scoped_trace();
        test_trace_id_propagation_pattern();
        test_scoped_trace_move_semantics();
        test_empty_trace_id_handling();

        std::cout << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
