# Task module examples
cmake_minimum_required(VERSION 3.16)

# Helper function to create example executables
function(add_task_example name)
    add_executable(example_task_${name} ${name}/main.cpp)
    target_link_libraries(example_task_${name} PRIVATE messaging_system_core)
    target_compile_features(example_task_${name} PRIVATE cxx_std_20)
endfunction()

# Create all task examples
add_task_example(simple_worker)
add_task_example(priority_tasks)
add_task_example(scheduled_tasks)
add_task_example(chain_workflow)
add_task_example(chord_aggregation)
add_task_example(progress_tracking)
add_task_example(monitoring_dashboard)
