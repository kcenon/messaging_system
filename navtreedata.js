/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Messaging System", "index.html", [
    [ "Application Layer - Messaging System", "index.html", "index" ],
    [ "Messaging System Samples", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html", [
      [ "Sample Applications", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md37", [
        [ "1. <strong>basic_usage_example.cpp</strong>", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md38", null ],
        [ "2. <strong>production_ready_example.cpp</strong> ⭐ NEW", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md39", null ],
        [ "3. <strong>chat_server.cpp</strong> (Enhanced)", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md40", null ],
        [ "4. <strong>distributed_worker.cpp</strong>", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md41", null ],
        [ "5. <strong>iot_monitoring.cpp</strong>", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md42", null ],
        [ "6. <strong>microservices_orchestrator.cpp</strong>", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md43", null ],
        [ "7. <strong>event_pipeline.cpp</strong>", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md44", null ],
        [ "8. <strong>message_bus_benchmark.cpp</strong>", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md45", null ]
      ] ],
      [ "Building the Samples", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md46", null ],
      [ "Configuration", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md47", [
        [ "Key Configuration Options:", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md48", null ]
      ] ],
      [ "Production Features", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md49", null ],
      [ "Best Practices Demonstrated", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md50", null ],
      [ "Logging", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md51", null ],
      [ "Monitoring", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md52", null ],
      [ "Running in Production", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md53", null ],
      [ "Troubleshooting", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md54", null ],
      [ "Contributing", "dd/dbf/md_application__layer_2samples_2SAMPLES__README.html#autotoc_md55", null ]
    ] ],
    [ "Threading Ecosystem Architecture", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html", [
      [ "🏗️ Ecosystem Overview", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md120", null ],
      [ "📋 Project Roles & Responsibilities", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md121", [
        [ "1. thread_system (Foundation)", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md122", [
          [ "Responsibilities:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md123", null ],
          [ "Key Components:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md124", null ],
          [ "Dependencies:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md125", null ]
        ] ],
        [ "2. logger_system (Logging)", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md127", [
          [ "Responsibilities:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md128", null ],
          [ "Key Components:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md129", null ],
          [ "Dependencies:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md130", null ]
        ] ],
        [ "3. monitoring_system (Metrics)", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md132", [
          [ "Responsibilities:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md133", null ],
          [ "Key Components:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md134", null ],
          [ "Dependencies:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md135", null ]
        ] ],
        [ "4. integrated_thread_system (Integration Hub)", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md137", [
          [ "Responsibilities:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md138", null ],
          [ "Key Components:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md139", null ],
          [ "Dependencies:", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md140", null ]
        ] ]
      ] ],
      [ "🔄 Dependency Flow & Interface Contracts", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md141", [
        [ "Interface Hierarchy", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md142", null ],
        [ "Dependency Graph", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md143", null ],
        [ "Build Order Requirements", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md144", null ]
      ] ],
      [ "🔧 Integration Patterns", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md145", [
        [ "1. Interface-Based Integration", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md146", null ],
        [ "2. Dependency Injection Pattern", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md147", null ],
        [ "3. Configuration Management", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md148", null ]
      ] ],
      [ "📊 Performance Characteristics", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md149", [
        [ "Design Principles", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md150", null ],
        [ "Performance Metrics", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md151", null ]
      ] ],
      [ "🔄 Evolution: Monolithic → Modular", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md152", [
        [ "Before: Monolithic Architecture", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md153", null ],
        [ "After: Modular Architecture", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md154", null ],
        [ "Migration Benefits", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md155", null ]
      ] ],
      [ "🚀 Getting Started", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md156", [
        [ "1. Development Environment Setup", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md157", null ],
        [ "2. Build Order (Local Development)", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md158", null ],
        [ "3. Verification", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md159", null ]
      ] ],
      [ "📚 Documentation Structure", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md160", [
        [ "thread_system", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md161", null ],
        [ "logger_system", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md162", null ],
        [ "monitoring_system", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md163", null ],
        [ "integrated_thread_system", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md164", null ]
      ] ],
      [ "🔮 Future Roadmap", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md165", [
        [ "Phase 1: Stabilization (Current)", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md166", null ],
        [ "Phase 2: Enhancement", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md167", null ],
        [ "Phase 3: Ecosystem Expansion", "d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md168", null ]
      ] ]
    ] ],
    [ "Changelog", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html", [
      [ "<a href=\"https://github.com/kcenon/logger_system/compare/v1.0.0...HEAD\"", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md171", [
        [ "Added", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md172", null ],
        [ "Changed", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md173", null ],
        [ "Deprecated", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md174", null ],
        [ "Removed", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md175", null ],
        [ "Fixed", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md176", null ],
        [ "Security", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md177", null ]
      ] ],
      [ "<a href=\"https://github.com/kcenon/logger_system/releases/tag/v1.0.0\" >1.0.0</a> - 2025-01-12", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md178", [
        [ "Added", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md179", null ],
        [ "Known Issues", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md180", null ]
      ] ],
      [ "Future Releases", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md181", [
        [ "[1.1.0] - Planned", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md182", null ],
        [ "[1.2.0] - Planned", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md183", null ],
        [ "[2.0.0] - Future", "d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md184", null ]
      ] ]
    ] ],
    [ "Contributing to Logger System", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html", [
      [ "Table of Contents", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md187", null ],
      [ "Code of Conduct", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md188", null ],
      [ "Getting Started", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md189", null ],
      [ "Development Process", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md190", [
        [ "1. Before You Start", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md191", null ],
        [ "2. Making Changes", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md192", null ],
        [ "3. Commit Message Format", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md193", null ]
      ] ],
      [ "Code Style", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md194", [
        [ "C++ Guidelines", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md195", null ],
        [ "Code Formatting", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md196", null ],
        [ "Naming Conventions", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md197", null ]
      ] ],
      [ "Testing", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md198", [
        [ "Unit Tests", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md199", null ],
        [ "Running Tests", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md200", null ],
        [ "Benchmarks", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md201", null ]
      ] ],
      [ "Documentation", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md202", [
        [ "Code Documentation", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md203", null ],
        [ "Documentation Updates", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md204", null ]
      ] ],
      [ "Submitting Changes", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md205", [
        [ "Pull Request Process", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md206", null ],
        [ "Review Process", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md207", null ]
      ] ],
      [ "Questions?", "d3/da5/md_libraries_2logger__system_2CONTRIBUTING.html#autotoc_md208", null ]
    ] ],
    [ "Logger System API Reference", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html", [
      [ "Table of Contents", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md210", null ],
      [ "Core Classes", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md211", [
        [ "logger_module::logger", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md212", null ]
      ] ],
      [ "Enumerations", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md213", [
        [ "thread_module::log_level", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md214", null ]
      ] ],
      [ "Logger Class", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md215", [
        [ "Constructor", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md216", null ],
        [ "Logging Methods", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md217", [
          [ "log (simple)", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md218", null ],
          [ "log (with source location)", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md219", null ]
        ] ],
        [ "Configuration Methods", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md220", [
          [ "set_min_level", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md221", null ],
          [ "get_min_level", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md222", null ],
          [ "is_enabled", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md223", null ]
        ] ],
        [ "Writer Management", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md224", [
          [ "add_writer", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md225", null ],
          [ "clear_writers", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md226", null ]
        ] ],
        [ "Lifecycle Methods", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md227", [
          [ "start", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md228", null ],
          [ "stop", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md229", null ],
          [ "is_running", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md230", null ],
          [ "flush", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md231", null ]
        ] ]
      ] ],
      [ "Writer Classes", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md232", [
        [ "base_writer", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md233", null ],
        [ "console_writer", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md234", null ]
      ] ],
      [ "Log Collector", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md235", [
        [ "Features:", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md236", null ]
      ] ],
      [ "Macros and Helpers", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md237", null ],
      [ "Thread Safety", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md238", null ],
      [ "Example Usage", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md239", [
        [ "Complete Example", "da/dc7/md_libraries_2logger__system_2docs_2api-reference.html#autotoc_md240", null ]
      ] ]
    ] ],
    [ "Logger System Architecture", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html", [
      [ "Overview", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md242", null ],
      [ "Architecture Diagram", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md243", null ],
      [ "Core Components", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md244", [
        [ "1. Logger Interface", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md245", null ],
        [ "2. Logger Implementation", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md246", [
          [ "Key Features:", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md247", null ]
        ] ],
        [ "3. Log Collector (Async Mode)", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md248", [
          [ "Design Decisions:", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md249", null ]
        ] ],
        [ "4. Writers", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md250", [
          [ "Base Writer", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md251", null ],
          [ "Console Writer", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md252", null ],
          [ "Extensibility", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md253", null ]
        ] ]
      ] ],
      [ "Threading Model", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md254", [
        [ "Synchronous Mode", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md255", null ],
        [ "Asynchronous Mode", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md256", null ],
        [ "Thread Safety Guarantees", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md257", null ]
      ] ],
      [ "Memory Management", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md258", [
        [ "Buffer Management", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md259", null ],
        [ "Object Lifetime", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md260", null ],
        [ "Performance Considerations", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md261", null ]
      ] ],
      [ "Integration Points", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md262", [
        [ "Service Container Integration", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md263", null ],
        [ "Direct Integration", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md264", null ]
      ] ],
      [ "Performance Characteristics", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md265", [
        [ "Synchronous Mode", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md266", null ],
        [ "Asynchronous Mode", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md267", null ]
      ] ],
      [ "Future Enhancements", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md268", [
        [ "Planned Features", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md269", null ],
        [ "Extension Points", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md270", null ]
      ] ],
      [ "Design Patterns Used", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md271", null ],
      [ "Best Practices", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md272", [
        [ "For Library Users", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md273", null ],
        [ "For Contributors", "d5/d03/md_libraries_2logger__system_2docs_2architecture.html#autotoc_md274", null ]
      ] ]
    ] ],
    [ "Creating Custom Writers", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html", [
      [ "Overview", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md276", null ],
      [ "Base Writer Interface", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md277", null ],
      [ "Simple Examples", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md278", [
        [ "1. File Writer", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md279", null ],
        [ "2. Rotating File Writer", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md280", null ],
        [ "3. Network Writer", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md281", null ],
        [ "4. Database Writer", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md282", null ]
      ] ],
      [ "Advanced Patterns", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md283", [
        [ "1. Filtering Writer", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md284", null ],
        [ "2. Async Writer Wrapper", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md285", null ]
      ] ],
      [ "Best Practices", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md286", null ],
      [ "Testing Custom Writers", "dd/dc8/md_libraries_2logger__system_2docs_2custom-writers.html#autotoc_md287", null ]
    ] ],
    [ "Getting Started with Logger System", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html", [
      [ "Table of Contents", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md289", null ],
      [ "Requirements", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md290", null ],
      [ "Installation", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md291", [
        [ "Using CMake FetchContent (Recommended)", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md292", null ],
        [ "Building from Source", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md293", null ],
        [ "Using as Installed Package", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md294", null ]
      ] ],
      [ "Basic Usage", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md295", [
        [ "Simple Console Logging", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md296", null ],
        [ "Logging with Source Location", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md297", null ],
        [ "Log Levels", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md298", null ],
        [ "Filtering by Level", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md299", null ]
      ] ],
      [ "Configuration", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md300", [
        [ "Synchronous vs Asynchronous Logging", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md301", null ],
        [ "Multiple Writers", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md302", null ],
        [ "Console Writer Options", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md303", null ]
      ] ],
      [ "Integration with Thread System", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md304", [
        [ "Using Service Container", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md305", null ],
        [ "Direct Integration", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md306", null ]
      ] ],
      [ "Next Steps", "d7/de1/md_libraries_2logger__system_2docs_2getting-started.html#autotoc_md307", null ]
    ] ],
    [ "Logger System Performance Guide", "de/d4f/md_libraries_2logger__system_2docs_2performance.html", [
      [ "Overview", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md309", null ],
      [ "Performance Characteristics", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md310", [
        [ "Synchronous Mode", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md311", null ],
        [ "Asynchronous Mode", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md312", null ]
      ] ],
      [ "Benchmarks", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md313", [
        [ "Test Environment", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md314", null ],
        [ "Single Thread Performance", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md315", null ],
        [ "Multi-threaded Performance", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md316", null ],
        [ "Memory Usage", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md317", null ]
      ] ],
      [ "Optimization Strategies", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md318", [
        [ "1. Choose the Right Mode", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md319", null ],
        [ "2. Buffer Size Tuning", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md320", null ],
        [ "3. Level Filtering", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md321", null ],
        [ "4. Message Construction", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md322", null ],
        [ "5. Writer Optimization", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md323", [
          [ "Console Writer", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md324", null ],
          [ "Custom High-Performance Writer", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md325", null ]
        ] ],
        [ "6. Batch Processing", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md326", null ]
      ] ],
      [ "Performance Anti-patterns", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md327", [
        [ "1. Synchronous I/O in Hot Paths", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md328", null ],
        [ "2. Excessive String Formatting", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md329", null ],
        [ "3. Logging in Tight Loops", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md330", null ]
      ] ],
      [ "Profiling and Monitoring", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md331", [
        [ "Built-in Metrics", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md332", null ],
        [ "External Profiling", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md333", null ]
      ] ],
      [ "Best Practices Summary", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md334", null ],
      [ "Platform-Specific Optimizations", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md335", [
        [ "Linux", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md336", null ],
        [ "Windows", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md337", null ],
        [ "macOS", "de/d4f/md_libraries_2logger__system_2docs_2performance.html#autotoc_md338", null ]
      ] ]
    ] ],
    [ "Threading Ecosystem Architecture", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html", [
      [ "🏗️ Ecosystem Overview", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md374", null ],
      [ "📋 Project Roles & Responsibilities", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md375", [
        [ "1. thread_system (Foundation)", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md376", [
          [ "Responsibilities:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md377", null ],
          [ "Key Components:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md378", null ],
          [ "Dependencies:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md379", null ]
        ] ],
        [ "2. logger_system (Logging)", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md381", [
          [ "Responsibilities:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md382", null ],
          [ "Key Components:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md383", null ],
          [ "Dependencies:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md384", null ]
        ] ],
        [ "3. monitoring_system (Metrics)", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md386", [
          [ "Responsibilities:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md387", null ],
          [ "Key Components:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md388", null ],
          [ "Dependencies:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md389", null ]
        ] ],
        [ "4. integrated_thread_system (Integration Hub)", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md391", [
          [ "Responsibilities:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md392", null ],
          [ "Key Components:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md393", null ],
          [ "Dependencies:", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md394", null ]
        ] ]
      ] ],
      [ "🔄 Dependency Flow & Interface Contracts", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md395", [
        [ "Interface Hierarchy", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md396", null ],
        [ "Dependency Graph", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md397", null ],
        [ "Build Order Requirements", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md398", null ]
      ] ],
      [ "🔧 Integration Patterns", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md399", [
        [ "1. Interface-Based Integration", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md400", null ],
        [ "2. Dependency Injection Pattern", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md401", null ],
        [ "3. Configuration Management", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md402", null ]
      ] ],
      [ "📊 Performance Characteristics", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md403", [
        [ "Design Principles", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md404", null ],
        [ "Performance Metrics", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md405", null ]
      ] ],
      [ "🔄 Evolution: Monolithic → Modular", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md406", [
        [ "Before: Monolithic Architecture", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md407", null ],
        [ "After: Modular Architecture", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md408", null ],
        [ "Migration Benefits", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md409", null ]
      ] ],
      [ "🚀 Getting Started", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md410", [
        [ "1. Development Environment Setup", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md411", null ],
        [ "2. Build Order (Local Development)", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md412", null ],
        [ "3. Verification", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md413", null ]
      ] ],
      [ "📚 Documentation Structure", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md414", [
        [ "thread_system", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md415", null ],
        [ "logger_system", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md416", null ],
        [ "monitoring_system", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md417", null ],
        [ "integrated_thread_system", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md418", null ]
      ] ],
      [ "🔮 Future Roadmap", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md419", [
        [ "Phase 1: Stabilization (Current)", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md420", null ],
        [ "Phase 2: Enhancement", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md421", null ],
        [ "Phase 3: Ecosystem Expansion", "dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md422", null ]
      ] ]
    ] ],
    [ "Changelog", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html", [
      [ "<a href=\"https://github.com/kcenon/monitoring_system/compare/v1.0.0...HEAD\"", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md425", [
        [ "Added", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md426", null ],
        [ "Changed", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md427", null ],
        [ "Deprecated", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md428", null ],
        [ "Removed", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md429", null ],
        [ "Fixed", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md430", null ],
        [ "Security", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md431", null ]
      ] ],
      [ "<a href=\"https://github.com/kcenon/monitoring_system/releases/tag/v1.0.0\" >1.0.0</a> - 2025-01-12", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md432", [
        [ "Added", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md433", null ],
        [ "Known Issues", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md434", null ]
      ] ],
      [ "Future Releases", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md435", [
        [ "[1.1.0] - Planned", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md436", null ],
        [ "[1.2.0] - Planned", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md437", null ],
        [ "[2.0.0] - Future", "de/da8/md_libraries_2monitoring__system_2CHANGELOG.html#autotoc_md438", null ]
      ] ]
    ] ],
    [ "Contributing to Monitoring System", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html", [
      [ "Table of Contents", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md441", null ],
      [ "Code of Conduct", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md442", null ],
      [ "Getting Started", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md443", null ],
      [ "Development Process", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md444", [
        [ "1. Before You Start", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md445", null ],
        [ "2. Making Changes", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md446", null ],
        [ "3. Commit Message Format", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md447", null ]
      ] ],
      [ "Code Style", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md448", [
        [ "C++ Guidelines", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md449", null ],
        [ "Code Formatting", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md450", null ],
        [ "Naming Conventions", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md451", null ]
      ] ],
      [ "Testing", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md452", [
        [ "Unit Tests", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md453", null ],
        [ "Performance Tests", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md454", null ],
        [ "Integration Tests", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md455", null ]
      ] ],
      [ "Documentation", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md456", [
        [ "Code Documentation", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md457", null ],
        [ "Documentation Updates", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md458", null ]
      ] ],
      [ "Submitting Changes", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md459", [
        [ "Pull Request Process", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md460", null ],
        [ "Code Review Process", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md461", null ]
      ] ],
      [ "Performance Considerations", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md462", null ],
      [ "Questions?", "d5/dab/md_libraries_2monitoring__system_2CONTRIBUTING.html#autotoc_md463", null ]
    ] ],
    [ "Monitoring System API Reference", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html", [
      [ "Table of Contents", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md465", null ],
      [ "Core Classes", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md466", [
        [ "monitoring_module::monitoring", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md467", null ]
      ] ],
      [ "Data Structures", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md468", [
        [ "monitoring_interface::system_metrics", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md469", null ],
        [ "monitoring_interface::thread_pool_metrics", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md470", null ],
        [ "monitoring_interface::worker_metrics", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md471", null ],
        [ "monitoring_interface::metrics_snapshot", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md472", null ]
      ] ],
      [ "Monitoring Class", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md473", [
        [ "Constructor", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md474", null ],
        [ "Metric Update Methods", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md475", [
          [ "update_system_metrics", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md476", null ],
          [ "update_thread_pool_metrics", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md477", null ],
          [ "update_worker_metrics", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md478", null ]
        ] ],
        [ "Data Retrieval Methods", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md479", [
          [ "get_current_snapshot", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md480", null ],
          [ "get_recent_snapshots", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md481", null ]
        ] ],
        [ "Lifecycle Methods", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md482", [
          [ "start", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md483", null ],
          [ "stop", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md484", null ],
          [ "is_active", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md485", null ]
        ] ],
        [ "Configuration Methods", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md486", [
          [ "set_collection_interval", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md487", null ],
          [ "get_collection_interval", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md488", null ]
        ] ],
        [ "Collector Management", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md489", [
          [ "add_collector", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md490", null ],
          [ "clear_collectors", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md491", null ]
        ] ],
        [ "Utility Methods", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md492", [
          [ "collect_now", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md493", null ],
          [ "clear_history", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md494", null ],
          [ "get_stats", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md495", null ]
        ] ]
      ] ],
      [ "Collector Classes", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md496", [
        [ "metrics_collector", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md497", null ],
        [ "Example Custom Collector", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md498", null ]
      ] ],
      [ "Ring Buffer", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md499", [
        [ "Template Class", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md500", null ],
        [ "Usage Example", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md501", null ]
      ] ],
      [ "Integration Types", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md502", [
        [ "Service Container Integration", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md503", null ],
        [ "Direct Usage", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md504", null ]
      ] ],
      [ "Thread Safety", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md505", null ],
      [ "Complete Example", "d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md506", null ]
    ] ],
    [ "Monitoring System Architecture", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html", [
      [ "Overview", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md508", null ],
      [ "Architecture Diagram", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md509", null ],
      [ "Core Components", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md510", [
        [ "1. Monitoring Interface", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md511", null ],
        [ "2. Monitoring Implementation", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md512", [
          [ "Key Features:", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md513", null ]
        ] ],
        [ "3. Metrics Types", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md514", [
          [ "System Metrics", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md515", null ],
          [ "Thread Pool Metrics", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md516", null ],
          [ "Worker Metrics", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md517", null ]
        ] ],
        [ "4. Ring Buffer Storage", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md518", [
          [ "Design Decisions:", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md519", null ]
        ] ],
        [ "5. Collector System", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md520", [
          [ "Base Collector", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md521", null ],
          [ "Collector Chain", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md522", null ]
        ] ]
      ] ],
      [ "Threading Model", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md523", [
        [ "Update Operations", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md524", null ],
        [ "Collection Thread", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md525", null ],
        [ "Thread Safety Guarantees", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md526", null ]
      ] ],
      [ "Memory Management", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md527", [
        [ "Storage Strategy", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md528", null ],
        [ "Object Lifetime", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md529", null ],
        [ "Performance Considerations", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md530", null ]
      ] ],
      [ "Integration Points", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md531", [
        [ "Service Container Integration", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md532", null ],
        [ "Direct Integration", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md533", null ],
        [ "Metric Sources", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md534", null ]
      ] ],
      [ "Performance Characteristics", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md535", [
        [ "Update Operations", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md536", null ],
        [ "Collection Operations", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md537", null ],
        [ "Storage Costs", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md538", null ]
      ] ],
      [ "Design Patterns Used", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md539", null ],
      [ "Extensibility", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md540", [
        [ "Custom Collectors", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md541", null ],
        [ "Custom Metrics", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md542", null ],
        [ "Storage Backends", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md543", null ]
      ] ],
      [ "Best Practices", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md544", [
        [ "For Library Users", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md545", null ],
        [ "For Contributors", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md546", null ]
      ] ],
      [ "Future Enhancements", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md547", [
        [ "Planned Features", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md548", null ],
        [ "Extension Points", "d3/dd8/md_libraries_2monitoring__system_2docs_2architecture.html#autotoc_md549", null ]
      ] ]
    ] ],
    [ "Creating Custom Collectors", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html", [
      [ "Overview", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md551", null ],
      [ "Base Collector Interface", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md552", null ],
      [ "Simple Examples", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md553", [
        [ "1. CPU Usage Collector (Linux)", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md554", null ],
        [ "2. Memory Usage Collector", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md555", null ],
        [ "3. Process-Specific Collector", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md556", null ]
      ] ],
      [ "Advanced Collectors", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md557", [
        [ "1. Network Statistics Collector", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md558", null ],
        [ "2. Disk I/O Collector", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md559", null ],
        [ "3. Application-Specific Collector", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md560", null ]
      ] ],
      [ "Best Practices", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md561", [
        [ "1. Error Handling", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md562", null ],
        [ "2. Caching Expensive Operations", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md563", null ],
        [ "3. Thread-Safe Collectors", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md564", null ]
      ] ],
      [ "Testing Custom Collectors", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md565", null ],
      [ "Integration Example", "df/d9a/md_libraries_2monitoring__system_2docs_2custom-collectors.html#autotoc_md566", null ]
    ] ],
    [ "Getting Started with Monitoring System", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html", [
      [ "Table of Contents", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md568", null ],
      [ "Requirements", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md569", null ],
      [ "Installation", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md570", [
        [ "Using CMake FetchContent (Recommended)", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md571", null ],
        [ "Building from Source", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md572", null ],
        [ "Using as Installed Package", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md573", null ]
      ] ],
      [ "Basic Usage", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md574", [
        [ "Simple Monitoring", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md575", null ],
        [ "Continuous Monitoring", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md576", null ]
      ] ],
      [ "Understanding Metrics", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md577", [
        [ "System Metrics", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md578", null ],
        [ "Thread Pool Metrics", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md579", null ],
        [ "Worker Metrics", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md580", null ]
      ] ],
      [ "Configuration", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md581", [
        [ "Construction Options", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md582", null ],
        [ "Runtime Configuration", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md583", null ],
        [ "Custom Collectors", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md584", null ]
      ] ],
      [ "Integration with Thread System", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md585", [
        [ "Using Service Container", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md586", null ],
        [ "Direct Integration", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md587", null ],
        [ "Monitoring Multiple Components", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md588", null ]
      ] ],
      [ "Next Steps", "d2/d17/md_libraries_2monitoring__system_2docs_2getting-started.html#autotoc_md589", null ]
    ] ],
    [ "Monitoring System Performance Guide", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html", [
      [ "Overview", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md591", null ],
      [ "Performance Characteristics", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md592", [
        [ "Update Operations", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md593", null ],
        [ "Collection Operations", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md594", null ]
      ] ],
      [ "Benchmarks", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md595", [
        [ "Test Environment", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md596", null ],
        [ "Update Performance", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md597", null ],
        [ "Collection Performance", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md598", null ],
        [ "Memory Usage", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md599", null ]
      ] ],
      [ "Overhead Analysis", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md600", [
        [ "CPU Overhead", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md601", null ],
        [ "Memory Overhead", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md602", null ]
      ] ],
      [ "Optimization Strategies", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md603", [
        [ "1. Tune Collection Frequency", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md604", null ],
        [ "2. Optimize History Size", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md605", null ],
        [ "3. Batch Updates", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md606", null ],
        [ "4. Conditional Updates", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md607", null ],
        [ "5. Lightweight Collectors", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md608", null ]
      ] ],
      [ "Performance Anti-patterns", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md609", [
        [ "1. High-Frequency String Operations", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md610", null ],
        [ "2. Synchronous I/O in Collectors", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md611", null ],
        [ "3. Unbounded History", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md612", null ]
      ] ],
      [ "Profiling and Monitoring", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md613", [
        [ "Built-in Metrics", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md614", null ],
        [ "External Profiling", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md615", null ]
      ] ],
      [ "Best Practices Summary", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md616", null ],
      [ "Platform-Specific Optimizations", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md617", [
        [ "Linux", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md618", null ],
        [ "Windows", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md619", null ],
        [ "macOS", "da/dbb/md_libraries_2monitoring__system_2docs_2performance.html#autotoc_md620", null ]
      ] ]
    ] ],
    [ "Threading Ecosystem Architecture", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html", [
      [ "🏗️ Ecosystem Overview", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md708", null ],
      [ "📋 Project Roles & Responsibilities", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md709", [
        [ "1. thread_system (Foundation)", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md710", [
          [ "Responsibilities:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md711", null ],
          [ "Key Components:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md712", null ],
          [ "Dependencies:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md713", null ]
        ] ],
        [ "2. logger_system (Logging)", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md715", [
          [ "Responsibilities:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md716", null ],
          [ "Key Components:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md717", null ],
          [ "Dependencies:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md718", null ]
        ] ],
        [ "3. monitoring_system (Metrics)", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md720", [
          [ "Responsibilities:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md721", null ],
          [ "Key Components:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md722", null ],
          [ "Dependencies:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md723", null ]
        ] ],
        [ "4. integrated_thread_system (Integration Hub)", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md725", [
          [ "Responsibilities:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md726", null ],
          [ "Key Components:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md727", null ],
          [ "Dependencies:", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md728", null ]
        ] ]
      ] ],
      [ "🔄 Dependency Flow & Interface Contracts", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md729", [
        [ "Interface Hierarchy", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md730", null ],
        [ "Dependency Graph", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md731", null ],
        [ "Build Order Requirements", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md732", null ]
      ] ],
      [ "🔧 Integration Patterns", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md733", [
        [ "1. Interface-Based Integration", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md734", null ],
        [ "2. Dependency Injection Pattern", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md735", null ],
        [ "3. Configuration Management", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md736", null ]
      ] ],
      [ "📊 Performance Characteristics", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md737", [
        [ "Design Principles", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md738", null ],
        [ "Performance Metrics", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md739", null ]
      ] ],
      [ "🔄 Evolution: Monolithic → Modular", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md740", [
        [ "Before: Monolithic Architecture", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md741", null ],
        [ "After: Modular Architecture", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md742", null ],
        [ "Migration Benefits", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md743", null ]
      ] ],
      [ "🚀 Getting Started", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md744", [
        [ "1. Development Environment Setup", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md745", null ],
        [ "2. Build Order (Local Development)", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md746", null ],
        [ "3. Verification", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md747", null ]
      ] ],
      [ "📚 Documentation Structure", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md748", [
        [ "thread_system", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md749", null ],
        [ "logger_system", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md750", null ],
        [ "monitoring_system", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md751", null ],
        [ "integrated_thread_system", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md752", null ]
      ] ],
      [ "🔮 Future Roadmap", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md753", [
        [ "Phase 1: Stabilization (Current)", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md754", null ],
        [ "Phase 2: Enhancement", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md755", null ],
        [ "Phase 3: Ecosystem Expansion", "df/d5b/md_libraries_2thread__system_2ARCHITECTURE.html#autotoc_md756", null ]
      ] ]
    ] ],
    [ "Changelog", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html", [
      [ "[Unreleased] - 2025-07-25", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md781", [
        [ "Changed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md782", null ],
        [ "Removed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md783", null ],
        [ "Added", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md784", null ],
        [ "Documentation", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md785", null ]
      ] ],
      [ "[2.0.0] - 2025-07-22", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md786", [
        [ "Added", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md787", null ],
        [ "Changed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md788", null ]
      ] ],
      [ "[Previous] - 2025-07-09", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md789", [
        [ "Changed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md790", null ],
        [ "Added", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md791", null ],
        [ "Documentation", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md792", null ]
      ] ],
      [ "[Previous Release] - 2025-06-30", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md793", [
        [ "Fixed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md794", null ],
        [ "Added", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md795", null ],
        [ "Changed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md796", null ],
        [ "Removed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md797", null ]
      ] ],
      [ "[Previous Release] - 2025-06-29", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md798", [
        [ "Added", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md799", null ],
        [ "Changed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md800", null ],
        [ "Fixed", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md801", null ]
      ] ],
      [ "[1.0.0] - 2024-01-15", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md802", [
        [ "Added", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md803", null ],
        [ "Core Components", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md804", null ],
        [ "Supported Platforms", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md805", null ],
        [ "Performance Characteristics", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md806", [
          [ "Latest Performance Metrics (Apple M1, 8-core @ 3.2GHz, 16GB, macOS Sonoma)", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md807", null ]
        ] ]
      ] ],
      [ "Contributing", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md809", null ],
      [ "License", "d7/df4/md_libraries_2thread__system_2CHANGELOG.html#autotoc_md810", null ]
    ] ],
    [ "Thread System API Reference", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html", [
      [ "Table of Contents", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md812", null ],
      [ "Overview", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md813", [
        [ "Core Components", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md814", null ],
        [ "Thread Pool Components", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md815", null ],
        [ "Typed Thread Pool Components", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md816", null ],
        [ "Advanced Features", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md817", null ]
      ] ],
      [ "Core Module (thread_module)", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md818", [
        [ "thread_base Class", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md819", null ],
        [ "job Class", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md820", null ],
        [ "job_queue Class", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md821", null ],
        [ "result<T> Template", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md822", null ]
      ] ],
      [ "Thread Pool Module (thread_pool_module)", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md823", [
        [ "thread_pool Class", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md824", null ],
        [ "thread_worker Class", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md825", null ]
      ] ],
      [ "Typed Thread Pool Module (typed_thread_pool_module)", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md826", [
        [ "typed_thread_pool_t Template", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md827", null ],
        [ "typed_thread_worker_t Template", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md828", null ],
        [ "job_types Enumeration", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md829", null ]
      ] ],
      [ "External Modules", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md830", [
        [ "Logger Module", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md831", null ],
        [ "Monitoring Module", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md832", null ],
        [ "Integration with Thread System", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md833", null ]
      ] ],
      [ "Utilities Module (utility_module)", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md834", [
        [ "formatter_macros", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md835", null ],
        [ "convert_string", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md836", null ]
      ] ],
      [ "Quick Reference", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md837", [
        [ "Creating a Basic Thread Pool", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md838", null ],
        [ "Using Typed Thread Pool", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md839", null ],
        [ "Error Handling Example", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md840", null ],
        [ "Batch Processing Example", "de/ded/md_libraries_2thread__system_2docs_2api-reference.html#autotoc_md841", null ]
      ] ]
    ] ],
    [ "Thread System Architecture", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html", [
      [ "Table of Contents", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md843", null ],
      [ "Overview", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md844", [
        [ "Architecture Principles", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md845", null ]
      ] ],
      [ "Core Architecture", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md846", [
        [ "System Layers", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md847", null ],
        [ "Component Hierarchy", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md848", null ]
      ] ],
      [ "Component Design", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md849", [
        [ "Modular Architecture Benefits", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md850", null ],
        [ "Thread Base Module", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md851", null ],
        [ "Thread Pool Module", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md852", null ],
        [ "Typed Thread Pool Module", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md853", null ],
        [ "Interface-Based External Module Integration", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md854", null ],
        [ "Adaptive Queue System", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md855", null ]
      ] ],
      [ "Implementation Details", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md856", [
        [ "Lock-Free MPMC Queue", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md857", null ],
        [ "Hazard Pointer Memory Management", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md858", null ],
        [ "Adaptive Strategy Selection", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md859", null ]
      ] ],
      [ "Performance Characteristics", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md860", [
        [ "Measured Performance (Apple M1, 8 cores)", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md861", null ],
        [ "Adaptive Queue Performance", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md862", null ]
      ] ],
      [ "Design Patterns", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md863", [
        [ "Template Method Pattern", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md864", null ],
        [ "Command Pattern", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md865", null ],
        [ "Strategy Pattern", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md866", null ]
      ] ],
      [ "Memory Management", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md867", [
        [ "Smart Pointer Usage", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md868", null ],
        [ "RAII Principles", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md869", null ],
        [ "Lock-Free Memory Safety", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md870", null ],
        [ "Memory Optimization Strategies", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md871", [
          [ "Lazy Initialization", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md872", null ],
          [ "Optimized Node Pool", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md873", null ],
          [ "Memory Usage Profile", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md874", null ]
        ] ]
      ] ],
      [ "Platform Support", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md875", [
        [ "Conditional Compilation", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md876", null ],
        [ "Platform-Specific Features", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md877", null ],
        [ "Build Configuration", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md878", null ]
      ] ],
      [ "Modularization Impact", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md879", [
        [ "Before Modularization", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md880", null ],
        [ "After Modularization", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md881", null ],
        [ "Key Improvements", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md882", null ]
      ] ],
      [ "Future Roadmap", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md883", [
        [ "Short Term (Completed)", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md884", null ],
        [ "Medium Term", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md885", null ],
        [ "Long Term", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md886", null ]
      ] ],
      [ "Best Practices", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md887", [
        [ "When to Use Each Component", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md888", null ],
        [ "Module Integration Guidelines", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md889", null ],
        [ "Configuration Guidelines", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md890", null ],
        [ "Performance Tuning", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md891", null ]
      ] ],
      [ "Conclusion", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md892", [
        [ "External Module Projects", "d6/d31/md_libraries_2thread__system_2docs_2architecture.html#autotoc_md893", null ]
      ] ]
    ] ],
    [ "Contributing to Thread System", "da/dae/md_libraries_2thread__system_2docs_2contributing.html", [
      [ "Getting Started", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md895", [
        [ "Prerequisites", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md896", null ],
        [ "Development Setup", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md897", null ]
      ] ],
      [ "Contribution Workflow", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md898", [
        [ "1. Create a Feature Branch", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md899", null ],
        [ "2. Make Changes", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md900", null ],
        [ "3. Commit Changes", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md901", null ],
        [ "4. Submit Pull Request", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md902", null ]
      ] ],
      [ "Coding Standards", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md903", [
        [ "C++ Style Guidelines", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md904", null ],
        [ "Error Handling", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md905", null ]
      ] ],
      [ "Testing Guidelines", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md906", [
        [ "Unit Tests", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md907", null ],
        [ "Performance Tests", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md908", null ]
      ] ],
      [ "Documentation Guidelines", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md909", [
        [ "Code Documentation", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md910", null ]
      ] ],
      [ "Review Process", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md911", [
        [ "Pull Request Requirements", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md912", null ],
        [ "Review Criteria", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md913", null ]
      ] ],
      [ "Getting Help", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md914", [
        [ "Support Channels", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md915", null ],
        [ "Response Times", "da/dae/md_libraries_2thread__system_2docs_2contributing.html#autotoc_md916", null ]
      ] ]
    ] ],
    [ "Thread System Examples", "de/deb/md_libraries_2thread__system_2docs_2examples.html", [
      [ "Quick Start Examples", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md919", [
        [ "Hello World with Thread Pool", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md920", null ],
        [ "Parallel Computation", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md921", null ]
      ] ],
      [ "Basic Examples", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md922", [
        [ "Simple Job Submission", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md923", null ],
        [ "Error Handling", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md924", null ],
        [ "File Processing", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md925", null ]
      ] ],
      [ "Type-Based Thread Pool Examples", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md926", [
        [ "Basic Type Scheduling", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md927", null ],
        [ "Custom Type Types", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md928", null ]
      ] ],
      [ "Advanced Thread Pool Examples", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md929", [
        [ "Custom Job Priority Queue", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md930", null ],
        [ "Performance Measurement", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md931", null ]
      ] ],
      [ "Real-World Scenarios", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md932", [
        [ "Web Server Request Handler", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md933", null ],
        [ "Data Processing Pipeline", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md934", null ]
      ] ],
      [ "Best Practices", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md935", [
        [ "Resource Management", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md936", null ],
        [ "Exception Safety", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md937", null ]
      ] ],
      [ "Examples with Optional Modules", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md938", [
        [ "Integration with Logger Module", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md939", null ],
        [ "Custom Error Handler with Optional Monitoring", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md940", null ],
        [ "Building with Optional Modules", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md941", null ],
        [ "Compilation Instructions", "de/deb/md_libraries_2thread__system_2docs_2examples.html#autotoc_md942", null ]
      ] ]
    ] ],
    [ "Frequently Asked Questions (FAQ)", "d8/d47/md_libraries_2thread__system_2docs_2faq.html", [
      [ "General Questions", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md944", [
        [ "Q: What is Thread System?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md945", null ],
        [ "Q: What C++ standard does Thread System require?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md946", null ],
        [ "Q: Does Thread System work on all major platforms?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md947", null ],
        [ "Q: Does Thread System require external dependencies?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md948", null ],
        [ "Q: Is Thread System thread-safe?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md949", null ]
      ] ],
      [ "Thread Base Questions", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md950", [
        [ "Q: What is the difference between <tt>thread_base</tt> and <tt>std::thread</tt>?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md951", null ],
        [ "Q: How do I create a custom worker thread?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md952", null ],
        [ "Q: How do I make a worker thread wake up periodically?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md953", null ],
        [ "Q: How do I handle errors in worker thread methods?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md954", null ]
      ] ],
      [ "Thread Pool Questions", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md955", [
        [ "Q: How many worker threads should I create in my thread pool?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md956", null ],
        [ "Q: Can I reuse a thread pool for multiple tasks?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md957", null ],
        [ "Q: How do I wait for all jobs in a thread pool to complete?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md958", null ],
        [ "Q: What happens if a job throws an exception?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md959", null ],
        [ "Q: How do I process the results of jobs submitted to a thread pool?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md960", null ]
      ] ],
      [ "Type Thread Pool Questions", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md961", [
        [ "Q: What is the difference between <tt>thread_pool</tt> and <tt>typed_thread_pool</tt>?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md962", null ],
        [ "Q: How many type levels should I use?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md963", null ],
        [ "Q: Can I create custom type types?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md964", null ],
        [ "Q: How do I assign workers to specific type levels?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md965", null ],
        [ "Q: What happens if there are no workers for a specific type level?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md966", null ]
      ] ],
      [ "Logging Questions", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md967", [
        [ "Q: Is logging thread-safe?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md968", null ],
        [ "Q: How do I configure where logs are written?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md969", null ],
        [ "Q: How do I customize log formatting?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md970", null ],
        [ "Q: Does logging affect application performance?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md971", null ],
        [ "Q: How do I handle log rotation?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md972", null ]
      ] ],
      [ "Performance Questions", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md973", [
        [ "Q: How do I measure the performance of Thread System?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md974", null ],
        [ "Q: What is the overhead of using Thread System compared to raw threads?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md975", null ],
        [ "Q: How does Thread System handle thread contention?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md976", null ],
        [ "Q: Is there a limit to how many jobs I can submit to a thread pool?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md977", null ]
      ] ],
      [ "Troubleshooting", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md978", null ],
      [ "Build and Integration", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md979", [
        [ "Q: How do I include Thread System in my project?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md980", null ],
        [ "Q: How do I build Thread System from source?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md981", null ],
        [ "Q: How do I run the Thread System unit tests?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md982", null ],
        [ "Q: What version of CMake is required to build Thread System?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md983", null ],
        [ "Q: Can I use Thread System in a non-CMake project?", "d8/d47/md_libraries_2thread__system_2docs_2faq.html#autotoc_md984", null ]
      ] ]
    ] ],
    [ "Getting Started with Thread System", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html", [
      [ "System Requirements", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md986", [
        [ "Supported Platforms", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md987", null ]
      ] ],
      [ "Quick Installation", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md988", [
        [ "One-Command Setup", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md989", null ],
        [ "Optional External Modules", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md990", null ]
      ] ],
      [ "Platform-Specific Instructions", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md991", [
        [ "Linux (Ubuntu/Debian)", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md992", null ],
        [ "Windows (Visual Studio)", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md993", null ],
        [ "macOS", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md994", null ]
      ] ],
      [ "Your First Thread System Program", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md995", [
        [ "Compile and Run", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md996", null ]
      ] ],
      [ "High-Performance Adaptive Example 🆕", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md997", null ],
      [ "Core Concepts", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md998", [
        [ "1. Thread Pool", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md999", null ],
        [ "2. Job Submission", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1000", null ],
        [ "3. Type Scheduling", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1001", [
          [ "High-Performance Adaptive Type Scheduling 🆕", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1002", null ]
        ] ],
        [ "4. Optional Module Integration", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1003", [
          [ "Logger Module (External)", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1004", null ],
          [ "Monitoring Module (External)", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1005", null ]
        ] ]
      ] ],
      [ "Common Use Cases", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1006", [
        [ "Parallel Data Processing", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1007", null ],
        [ "Asynchronous I/O Operations", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1008", null ]
      ] ],
      [ "Build Options", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1009", [
        [ "CMake Configuration", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1010", null ],
        [ "Build Script Options", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1011", null ]
      ] ],
      [ "Verification", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1012", [
        [ "Test Installation", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1013", null ],
        [ "Run Tests (Linux only)", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1014", null ]
      ] ],
      [ "Troubleshooting", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1015", [
        [ "Common Issues", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1016", [
          [ "CMake can't find vcpkg toolchain", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1017", null ],
          [ "Compiler not found", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1018", null ],
          [ "Missing dependencies", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1019", null ]
        ] ]
      ] ],
      [ "Integration Options", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1020", [
        [ "Using as Submodule", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1021", null ],
        [ "Using with CMake FetchContent", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1022", null ]
      ] ],
      [ "Best Practices", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1023", [
        [ "Performance Optimization", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1024", null ],
        [ "Resource Management", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1025", null ],
        [ "Error Handling and Monitoring", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1026", null ],
        [ "Adaptive Queue Tips", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1027", null ]
      ] ],
      [ "What's Next?", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1028", null ],
      [ "Need Help?", "d8/d53/md_libraries_2thread__system_2docs_2getting-started.html#autotoc_md1029", null ]
    ] ],
    [ "Migration Guide: Transitioning to Thread System", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html", [
      [ "Why Migrate to Thread System?", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1032", null ],
      [ "Migration Process Overview", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1033", null ],
      [ "Step 1: Analyze Existing Code", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1034", [
        [ "Common Threading Patterns", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1035", [
          [ "Raw Thread Usage", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1036", null ],
          [ "Thread Pool Pattern", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1037", null ],
          [ "Asynchronous Processing", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1038", null ],
          [ "Producer-Consumer Pattern", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1039", null ]
        ] ]
      ] ],
      [ "Step 2: Select Thread System Components", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1040", null ],
      [ "Step 3: Basic Refactoring", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1041", [
        [ "Raw Thread to thread_base", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1042", null ],
        [ "Thread Pool to thread_pool", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1043", null ],
        [ "Async Processing to Thread System", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1044", null ],
        [ "Producer-Consumer to Thread System", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1045", null ]
      ] ],
      [ "Step 4: Advanced Refactoring", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1046", [
        [ "Adding Type Support", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1047", null ],
        [ "Integrating Logging", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1048", null ],
        [ "Implementing Periodic Tasks", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1049", null ],
        [ "Error Handling Improvements", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1050", null ]
      ] ],
      [ "Step 5: Testing and Validation", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1051", [
        [ "Unit Tests for Thread System Components", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1052", null ],
        [ "Performance Validation", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1053", null ]
      ] ],
      [ "Common Migration Challenges and Solutions", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1054", [
        [ "Challenge 1: Adapting to Result-based Error Handling", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1055", null ],
        [ "Challenge 2: Managing Thread Lifecycle", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1056", null ],
        [ "Challenge 3: Adapting to Job-based Design", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1057", null ],
        [ "Challenge 4: Integrating with Existing Libraries", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1058", null ]
      ] ],
      [ "Migrating Specific Threading Models", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1059", [
        [ "Single Background Thread", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1060", null ],
        [ "Worker Thread Pool", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1061", null ],
        [ "Event Loop", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1062", null ]
      ] ],
      [ "Best Practices for Successful Migration", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1063", null ],
      [ "Migration Checklist", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1064", null ],
      [ "Conclusion", "d9/dae/md_libraries_2thread__system_2docs_2migration__guide.html#autotoc_md1065", null ]
    ] ],
    [ "Thread System: Patterns, Best Practices, and Troubleshooting Guide", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html", [
      [ "Table of Contents", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1067", null ],
      [ "Best Practices", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1068", [
        [ "1. Thread Base Usage", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1069", [
          [ "✅ DO:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1070", null ],
          [ "❌ DON'T:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1071", null ]
        ] ],
        [ "2. Thread Pool Usage", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1072", [
          [ "✅ DO:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1073", null ],
          [ "❌ DON'T:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1074", null ]
        ] ],
        [ "3. Type Thread Pool Usage", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1075", [
          [ "✅ DO:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1076", null ],
          [ "❌ DON'T:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1077", null ]
        ] ],
        [ "4. Error Handling", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1078", [
          [ "✅ DO:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1079", null ],
          [ "❌ DON'T:", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1080", null ]
        ] ]
      ] ],
      [ "Common Patterns", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1081", [
        [ "1. Worker Thread Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1082", null ],
        [ "2. Thread Pool Task Processing Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1083", null ],
        [ "3. Type-Based Job Execution Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1084", null ],
        [ "4. Error Handler Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1085", null ],
        [ "5. Producer-Consumer Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1086", null ],
        [ "6. Task Partitioning Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1087", null ]
      ] ],
      [ "Antipatterns to Avoid", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1088", [
        [ "1. The Thread Explosion Antipattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1089", null ],
        [ "2. The Busy Waiting Antipattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1090", null ],
        [ "3. The Type Abuse Antipattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1091", null ],
        [ "4. The Blocking Thread Pool Antipattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1092", null ],
        [ "5. The Performance Monitoring Antipattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1093", null ]
      ] ],
      [ "Troubleshooting Common Issues", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1094", [
        [ "1. Race Conditions", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1095", [
          [ "Symptoms", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1096", null ],
          [ "Solution Approaches", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1097", null ]
        ] ],
        [ "2. Deadlocks", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1098", [
          [ "Symptoms", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1099", null ],
          [ "Solution Approaches", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1100", null ]
        ] ],
        [ "3. Type Inversion", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1101", [
          [ "Symptoms", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1102", null ],
          [ "Solution Approaches", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1103", null ]
        ] ],
        [ "4. Thread Starvation", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1104", [
          [ "Symptoms", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1105", null ],
          [ "Solution Approaches", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1106", null ]
        ] ],
        [ "5. False Sharing", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1107", [
          [ "Symptoms", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1108", null ],
          [ "Solution Approaches", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1109", null ]
        ] ],
        [ "6. Memory Visibility Issues", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1110", [
          [ "Symptoms", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1111", null ],
          [ "Solution Approaches", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1112", null ]
        ] ]
      ] ],
      [ "Advanced Concurrency Patterns", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1113", [
        [ "1. Event-Based Communication", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1114", null ],
        [ "2. Work Stealing Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1115", null ],
        [ "3. Read-Write Lock Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1116", null ]
      ] ],
      [ "Debugging Concurrent Code", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1117", [
        [ "Using Diagnostics Effectively", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1118", null ],
        [ "Using Thread Sanitizers", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1119", null ],
        [ "Common Thread System Debugging Steps", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1120", null ]
      ] ],
      [ "Performance Optimization", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1121", [
        [ "Thread Pool Sizing Guidelines", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1122", null ],
        [ "Batch Job Submission", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1123", null ],
        [ "Wake Interval Optimization", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1124", null ]
      ] ],
      [ "Integrating External Modules", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1125", [
        [ "Logger Integration Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1126", null ],
        [ "Monitoring Integration Pattern", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1127", null ],
        [ "Best Practices for Modular Architecture", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1128", null ]
      ] ],
      [ "Conclusion", "d5/de2/md_libraries_2thread__system_2docs_2patterns.html#autotoc_md1129", null ]
    ] ],
    [ "Thread System Performance Guide", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html", [
      [ "Table of Contents", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1131", null ],
      [ "Performance Overview", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1132", [
        [ "Key Performance Highlights (Current Architecture)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1133", null ]
      ] ],
      [ "Benchmark Environment", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1134", [
        [ "Test Hardware", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1135", null ],
        [ "Compiler Configuration", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1136", null ],
        [ "Thread System Version", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1137", null ]
      ] ],
      [ "Core Performance Metrics", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1138", [
        [ "Component Overhead", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1139", null ],
        [ "Thread Pool Creation Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1140", null ]
      ] ],
      [ "Data Race Fix Impact", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1141", [
        [ "Overview", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1142", null ],
        [ "Performance Impact Analysis", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1143", null ],
        [ "Before vs After Comparison", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1144", null ],
        [ "Real-World Impact", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1145", null ]
      ] ],
      [ "Detailed Benchmark Results", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1146", [
        [ "Job Submission Latency", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1147", [
          [ "Standard Thread Pool (Mutex-based)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1148", null ],
          [ "Adaptive Queue (Lock-free Mode)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1149", null ]
        ] ],
        [ "Throughput by Job Complexity", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1150", [
          [ "Standard Thread Pool Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1151", null ],
          [ "Adaptive Job Queue Performance (Lock-free Mode)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1152", null ],
          [ "Type Thread Pool Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1153", null ],
          [ "Real-World Measurements (Lock-Free Implementation)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1154", null ],
          [ "Type Thread Pool with Adaptive Queues", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1155", [
            [ "typed_thread_pool (Current Implementation)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1156", null ],
            [ "Adaptive Queue Strategy", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1157", null ]
          ] ]
        ] ]
      ] ],
      [ "Adaptive Job Queue Benchmarks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1158", [
        [ "Overview", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1159", null ],
        [ "Thread Pool Level Benchmarks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1160", [
          [ "Simple Job Processing", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1161", null ],
          [ "Medium Workload Processing", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1162", null ],
          [ "Priority Scheduling Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1163", null ],
          [ "High Contention Scenarios", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1164", null ]
        ] ],
        [ "Queue Level Benchmarks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1165", [
          [ "Basic Queue Operations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1166", null ],
          [ "Batch Operations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1167", null ],
          [ "Contention Stress Tests", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1168", null ],
          [ "Job Type Routing Features", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1169", null ],
          [ "Memory Usage Comparison", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1170", null ]
        ] ],
        [ "Benchmark Environment Details", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1171", null ],
        [ "Available Benchmarks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1172", [
          [ "Thread Pool Benchmarks (<tt>benchmarks/thread_pool_benchmarks/</tt>)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1173", null ],
          [ "Queue Benchmarks (<tt>benchmarks/thread_base_benchmarks/</tt>)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1174", null ],
          [ "Other Benchmarks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1175", null ],
          [ "Running Benchmarks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1176", null ]
        ] ],
        [ "Key Performance Insights", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1177", null ]
      ] ],
      [ "Scalability Analysis", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1178", [
        [ "Worker Thread Scaling Efficiency", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1179", null ],
        [ "Workload-Specific Scaling", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1180", [
          [ "CPU-Bound Tasks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1181", null ],
          [ "I/O-Bound Tasks", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1182", null ],
          [ "Mixed Workloads", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1183", null ]
        ] ]
      ] ],
      [ "Memory Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1184", [
        [ "Memory Usage by Configuration", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1185", null ],
        [ "Memory Optimization Impact (v2.0)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1186", null ],
        [ "Startup Memory Profile", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1187", null ],
        [ "Memory Allocation Impact on Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1188", null ],
        [ "Potential Memory Pool Optimization", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1189", null ]
      ] ],
      [ "Adaptive MPMC Queue Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1190", [
        [ "Overview", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1191", null ],
        [ "Performance Comparison", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1192", null ],
        [ "Scalability Analysis", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1193", null ],
        [ "Implementation Details", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1194", null ]
      ] ],
      [ "Adaptive Logger Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1195", [
        [ "Overview", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1196", null ],
        [ "Single-Threaded Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1197", null ],
        [ "Multi-Threaded Scalability", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1198", null ],
        [ "Formatted Logging Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1199", null ],
        [ "Burst Logging Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1200", null ],
        [ "Mixed Log Types Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1201", null ],
        [ "Key Findings", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1202", null ],
        [ "Recommendations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1203", null ],
        [ "Implementation Details", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1204", null ],
        [ "Current Status", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1205", null ],
        [ "Recent Benchmark Results (2025-07-25)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1206", [
          [ "Data Race Fix Verification", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1207", null ]
        ] ],
        [ "Usage Recommendations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1208", null ],
        [ "Performance Tuning Tips", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1209", null ]
      ] ],
      [ "Logger Performance (Now Separate Project)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1210", [
        [ "Logger Comparison with Industry Standards", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1211", null ],
        [ "Overview", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1212", null ],
        [ "Single-Threaded Performance Comparison", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1213", null ],
        [ "Multi-Threaded Scalability", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1214", null ],
        [ "Latency Characteristics", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1215", null ],
        [ "Key Findings", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1216", null ],
        [ "Comparison with spdlog", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1217", [
          [ "Single-Threaded Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1218", null ],
          [ "Multi-Threaded Performance (4 Threads)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1219", null ],
          [ "High Contention (8 Threads)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1220", null ],
          [ "Key Findings", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1221", null ]
        ] ],
        [ "Recommendations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1222", null ]
      ] ],
      [ "Comparison with Other Libraries", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1223", [
        [ "Throughput Comparison (Real-world measurements)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1224", null ],
        [ "Feature Comparison", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1225", null ],
        [ "Latency Comparison (μs)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1226", null ]
      ] ],
      [ "Optimization Strategies", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1227", [
        [ "1. Optimal Thread Count Selection", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1228", null ],
        [ "2. Job Batching for Performance", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1229", null ],
        [ "3. Job Granularity Optimization", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1230", null ],
        [ "4. Type Pool Configuration", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1231", null ],
        [ "4b. Adaptive Queue Configuration", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1232", null ],
        [ "5. Memory Optimization", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1233", [
          [ "Cache-Line Alignment", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1234", null ],
          [ "Memory Pool Implementation (Suggested Optimization)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1235", null ]
        ] ]
      ] ],
      [ "Platform-Specific Optimizations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1236", [
        [ "macOS/ARM64 Optimizations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1237", null ],
        [ "Linux Optimizations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1238", null ],
        [ "Windows Optimizations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1239", null ]
      ] ],
      [ "Best Practices", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1240", [
        [ "Performance Tuning Checklist", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1241", [
          [ "Measurement and Analysis", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1242", null ],
          [ "Thread Pool Configuration", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1243", null ],
          [ "Job Design", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1244", null ],
          [ "Memory Considerations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1245", null ],
          [ "Advanced Techniques", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1246", null ]
        ] ],
        [ "Real-World Performance Guidelines", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1247", [
          [ "Web Server Applications", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1248", null ],
          [ "Data Processing Pipelines", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1249", null ],
          [ "Real-Time Systems", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1250", null ],
          [ "Scientific Computing", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1251", null ]
        ] ],
        [ "Monitoring and Diagnostics", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1252", [
          [ "Key Performance Indicators", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1253", null ],
          [ "Diagnostic Tools", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1254", null ]
        ] ]
      ] ],
      [ "Future Performance Improvements", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1255", [
        [ "Planned Optimizations", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1256", null ]
      ] ],
      [ "Performance Recommendations Summary (2025)", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1257", [
        [ "Quick Configuration Guide", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1258", [
          [ "1. <strong>For General Applications</strong>", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1259", null ],
          [ "2. <strong>For Priority-Sensitive Applications</strong>", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1260", null ],
          [ "3. <strong>For High-Concurrency Scenarios</strong>", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1261", null ]
        ] ],
        [ "Performance Tuning Quick Reference", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1262", null ],
        [ "Common Pitfalls to Avoid", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1263", null ]
      ] ],
      [ "Conclusion", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1264", [
        [ "Key Success Factors", "d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1265", null ]
      ] ]
    ] ],
    [ "Thread System Migration Guide", "d9/dea/md_libraries_2thread__system_2MIGRATION.html", [
      [ "Overview", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1277", null ],
      [ "Migration Status", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1278", [
        [ "Phase 1: Interface Extraction and Cleanup ✅ COMPLETE", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1279", null ],
        [ "Phase 2: Create New Repository Structure ✅ COMPLETE", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1280", null ],
        [ "Phase 3: Component Migration ✅ COMPLETE", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1281", null ],
        [ "Phase 4: Integration Testing ✅ COMPLETE", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1282", null ],
        [ "Phase 5: Gradual Deployment 🔄 PENDING", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1283", null ]
      ] ],
      [ "Breaking Changes", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1284", [
        [ "API Changes", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1285", null ],
        [ "Build System Changes", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1286", null ]
      ] ],
      [ "Migration Instructions for Users", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1287", [
        [ "Current Users (Phase 1)", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1288", null ],
        [ "Future Migration (Phase 2-5)", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1289", null ]
      ] ],
      [ "Timeline", "d9/dea/md_libraries_2thread__system_2MIGRATION.html#autotoc_md1290", null ]
    ] ],
    [ "Test Coverage Improvement Report - Thread System", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html", [
      [ "Executive Summary", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1533", null ],
      [ "Phases Completed", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1534", [
        [ "Phase 1: Monitoring Module Tests ✅", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1535", null ],
        [ "Phase 2: Error Handling Tests ✅", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1536", null ],
        [ "Phase 3: Concurrency Boundary Tests ✅", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1537", null ],
        [ "Phase 4: Platform-Specific Tests ✅", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1538", null ],
        [ "Code Coverage Tools Integration ✅", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1539", null ]
      ] ],
      [ "Critical Issues Found and Fixed", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1540", null ],
      [ "Test Coverage Improvements", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1541", [
        [ "Before Implementation", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1542", null ],
        [ "After Implementation", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1543", null ]
      ] ],
      [ "How to Generate Coverage Report", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1544", null ],
      [ "Recommendations for Maintaining High Coverage", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1545", null ],
      [ "Next Steps", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1546", null ],
      [ "Conclusion", "dd/da6/md_libraries_2thread__system_2TEST__COVERAGE__IMPROVEMENT__REPORT.html#autotoc_md1547", null ]
    ] ],
    [ "API Reference", "db/d91/md_docs_2API__REFERENCE.html", [
      [ "Table of Contents", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1549", null ],
      [ "Container System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1551", [
        [ "Namespace: <tt>container_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1552", null ],
        [ "Class: <tt>value_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1553", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1554", null ],
          [ "Core Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1555", [
            [ "Setting Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1556", null ],
            [ "Adding Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1557", null ],
            [ "Retrieving Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1558", null ],
            [ "Serialization", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1559", null ]
          ] ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1560", null ]
        ] ],
        [ "Class: <tt>variant</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1561", [
          [ "Supported Types", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1562", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1563", null ]
        ] ]
      ] ],
      [ "Network System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1565", [
        [ "Namespace: <tt>network_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1566", null ],
        [ "Class: <tt>messaging_server</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1567", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1568", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1569", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1570", null ]
        ] ],
        [ "Class: <tt>messaging_client</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1571", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1572", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1573", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1574", null ]
        ] ]
      ] ],
      [ "Database System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1576", [
        [ "Namespace: <tt>database</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1577", null ],
        [ "Class: <tt>database_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1578", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1579", null ],
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1580", null ],
          [ "Connection Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1581", null ],
          [ "Query Execution", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1582", null ],
          [ "Transaction Support", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1583", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1584", null ]
        ] ]
      ] ],
      [ "Message Bus API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1586", [
        [ "Namespace: <tt>kcenon::messaging::core</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1587", null ],
        [ "Class: <tt>message_bus</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1588", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1589", null ],
          [ "Configuration Structure", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1590", null ],
          [ "Lifecycle Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1591", null ],
          [ "Publishing", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1592", null ],
          [ "Subscription", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1593", null ],
          [ "Request-Response", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1594", null ],
          [ "Monitoring", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1595", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1596", null ]
        ] ]
      ] ],
      [ "Service Container API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1598", [
        [ "Namespace: <tt>kcenon::messaging::services</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1599", null ],
        [ "Class: <tt>service_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1600", [
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1601", null ],
          [ "Service Lifetimes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1602", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1603", null ]
        ] ]
      ] ],
      [ "Thread System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1605", [
        [ "Namespace: <tt>thread_system</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1606", null ],
        [ "Class: <tt>thread_pool</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1607", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1608", null ],
          [ "Job Submission", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1609", null ],
          [ "Pool Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1610", null ],
          [ "Priority Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1611", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1612", null ]
        ] ],
        [ "Class: <tt>lock_free_queue</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1613", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1614", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1615", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1616", null ]
        ] ]
      ] ],
      [ "Logger System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1618", [
        [ "Namespace: <tt>logger</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1619", null ],
        [ "Class: <tt>logger_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1620", [
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1621", null ],
          [ "Log Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1622", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1623", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1624", null ]
        ] ]
      ] ],
      [ "Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1626", [
        [ "System Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1627", null ],
        [ "Error Handling", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1628", null ],
        [ "Error Recovery Strategies", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1629", null ]
      ] ],
      [ "Configuration Reference", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1630", [
        [ "System Configuration File Format", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1631", null ],
        [ "Environment Variables", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1632", null ]
      ] ]
    ] ],
    [ "Architecture", "d2/d64/md_docs_2architecture.html", [
      [ "Overview", "d2/d64/md_docs_2architecture.html#autotoc_md1634", null ],
      [ "Related Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md1635", [
        [ "Component-Specific Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1636", null ],
        [ "Module Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md1637", null ],
        [ "Integration Guides", "d2/d64/md_docs_2architecture.html#autotoc_md1638", null ]
      ] ],
      [ "Architectural Principles", "d2/d64/md_docs_2architecture.html#autotoc_md1639", null ],
      [ "System Layers", "d2/d64/md_docs_2architecture.html#autotoc_md1640", null ],
      [ "Component Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1641", [
        [ "1. Container System", "d2/d64/md_docs_2architecture.html#autotoc_md1642", null ],
        [ "2. Network System", "d2/d64/md_docs_2architecture.html#autotoc_md1643", null ],
        [ "3. Database System", "d2/d64/md_docs_2architecture.html#autotoc_md1644", null ],
        [ "4. Thread System", "d2/d64/md_docs_2architecture.html#autotoc_md1645", null ]
      ] ],
      [ "Data Flow Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1646", [
        [ "Message Processing Pipeline", "d2/d64/md_docs_2architecture.html#autotoc_md1647", null ],
        [ "Request-Response Flow", "d2/d64/md_docs_2architecture.html#autotoc_md1648", null ],
        [ "Distributed Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1649", null ]
      ] ],
      [ "Scalability Patterns", "d2/d64/md_docs_2architecture.html#autotoc_md1650", [
        [ "Horizontal Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md1651", null ],
        [ "Vertical Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md1652", null ]
      ] ],
      [ "Integration Points", "d2/d64/md_docs_2architecture.html#autotoc_md1653", [
        [ "External Systems", "d2/d64/md_docs_2architecture.html#autotoc_md1654", null ],
        [ "Internal Communication", "d2/d64/md_docs_2architecture.html#autotoc_md1655", null ]
      ] ],
      [ "Performance Optimization", "d2/d64/md_docs_2architecture.html#autotoc_md1656", [
        [ "Memory Management", "d2/d64/md_docs_2architecture.html#autotoc_md1657", null ],
        [ "Concurrency", "d2/d64/md_docs_2architecture.html#autotoc_md1658", null ],
        [ "Network", "d2/d64/md_docs_2architecture.html#autotoc_md1659", null ]
      ] ],
      [ "Fault Tolerance", "d2/d64/md_docs_2architecture.html#autotoc_md1660", [
        [ "Error Recovery", "d2/d64/md_docs_2architecture.html#autotoc_md1661", null ],
        [ "High Availability", "d2/d64/md_docs_2architecture.html#autotoc_md1662", null ]
      ] ],
      [ "Security Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md1663", [
        [ "Transport Security", "d2/d64/md_docs_2architecture.html#autotoc_md1664", null ],
        [ "Application Security", "d2/d64/md_docs_2architecture.html#autotoc_md1665", null ]
      ] ],
      [ "Monitoring and Observability", "d2/d64/md_docs_2architecture.html#autotoc_md1666", [
        [ "Metrics Collection", "d2/d64/md_docs_2architecture.html#autotoc_md1667", null ],
        [ "Distributed Tracing", "d2/d64/md_docs_2architecture.html#autotoc_md1668", null ],
        [ "Logging", "d2/d64/md_docs_2architecture.html#autotoc_md1669", null ]
      ] ],
      [ "Configuration Management", "d2/d64/md_docs_2architecture.html#autotoc_md1670", [
        [ "Static Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md1671", null ],
        [ "Dynamic Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md1672", null ]
      ] ],
      [ "Deployment Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1673", [
        [ "Container Deployment", "d2/d64/md_docs_2architecture.html#autotoc_md1674", null ],
        [ "Kubernetes Integration", "d2/d64/md_docs_2architecture.html#autotoc_md1675", null ]
      ] ],
      [ "Future Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md1676", [
        [ "Planned Enhancements", "d2/d64/md_docs_2architecture.html#autotoc_md1677", null ],
        [ "Technology Evaluation", "d2/d64/md_docs_2architecture.html#autotoc_md1678", null ]
      ] ]
    ] ],
    [ "Build Troubleshooting Guide", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html", [
      [ "Common Build Issues and Solutions", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1680", [
        [ "Issue 1: Target Name Conflicts", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1681", null ],
        [ "Issue 2: GTest Not Found in External Systems", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1683", null ],
        [ "Issue 3: grep -P Not Supported (macOS)", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1685", null ],
        [ "Issue 4: yaml-cpp Not Found", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1687", null ]
      ] ],
      [ "Recommended Build Workflow", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1689", [
        [ "For Development (FetchContent Mode)", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1690", null ],
        [ "For Production (find_package Mode)", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1691", null ]
      ] ],
      [ "Alternative: Minimal Build (No External Systems)", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1693", null ],
      [ "Verifying Successful Build", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1695", null ],
      [ "Getting Help", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1697", null ],
      [ "Known Limitations", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1699", null ],
      [ "Platform-Specific Notes", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1701", [
        [ "macOS", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1702", null ],
        [ "Linux (Ubuntu/Debian)", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1703", null ],
        [ "Linux (Fedora/RHEL)", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1704", null ]
      ] ],
      [ "Quick Fix Summary", "d7/d68/md_docs_2BUILD__TROUBLESHOOTING.html#autotoc_md1706", null ]
    ] ],
    [ "Deployment Guide", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html", [
      [ "System Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1709", [
        [ "Hardware Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1710", [
          [ "Minimum Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1711", null ],
          [ "Recommended Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1712", null ],
          [ "Production Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1713", null ]
        ] ],
        [ "Operating System Support", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1714", null ],
        [ "Software Dependencies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1715", [
          [ "Build Dependencies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1716", null ],
          [ "Runtime Dependencies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1717", null ]
        ] ]
      ] ],
      [ "Installation", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1718", [
        [ "From Source", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1719", [
          [ "1. Clone Repository", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1720", null ],
          [ "2. Configure Build", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1721", null ],
          [ "3. Build and Install", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1722", null ]
        ] ],
        [ "Using Package Managers", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1723", [
          [ "APT Repository (Ubuntu/Debian)", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1724", null ],
          [ "YUM Repository (RHEL/CentOS)", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1725", null ]
        ] ],
        [ "Docker Installation", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1726", [
          [ "Pull Official Image", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1727", null ],
          [ "Build Custom Image", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1728", null ]
        ] ]
      ] ],
      [ "Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1729", [
        [ "Configuration File Structure", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1730", null ],
        [ "Environment Variables", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1731", null ],
        [ "Secrets Management", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1732", [
          [ "Using HashiCorp Vault", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1733", null ],
          [ "Using Kubernetes Secrets", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1734", null ]
        ] ]
      ] ],
      [ "Deployment Scenarios", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1735", [
        [ "Single Server Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1736", null ],
        [ "High Availability Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1737", [
          [ "Architecture", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1738", null ],
          [ "HAProxy Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1739", null ]
        ] ],
        [ "Kubernetes Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1740", [
          [ "Namespace and ConfigMap", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1741", null ],
          [ "Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1742", null ],
          [ "Service and Ingress", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1743", null ],
          [ "Horizontal Pod Autoscaler", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1744", null ]
        ] ],
        [ "Docker Compose Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1745", null ]
      ] ],
      [ "Performance Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1746", [
        [ "System Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1747", [
          [ "Linux Kernel Parameters", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1748", null ],
          [ "ulimit Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1749", null ]
        ] ],
        [ "Application Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1750", [
          [ "Thread Pool Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1751", null ],
          [ "Memory Management", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1752", null ],
          [ "Network Optimization", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1753", null ]
        ] ],
        [ "Database Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1754", [
          [ "PostgreSQL Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1755", null ],
          [ "Connection Pool Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1756", null ]
        ] ]
      ] ],
      [ "Monitoring Setup", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1757", [
        [ "Prometheus Integration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1758", [
          [ "Prometheus Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1759", null ],
          [ "Metrics Exposed", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1760", null ]
        ] ],
        [ "Grafana Dashboard", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1761", null ],
        [ "Logging Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1762", [
          [ "Structured Logging", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1763", null ],
          [ "Log Aggregation (ELK Stack)", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1764", null ]
        ] ]
      ] ],
      [ "Scaling Strategies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1765", [
        [ "Horizontal Scaling", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1766", [
          [ "Load Balancer Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1767", null ],
          [ "Auto-scaling Rules", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1768", null ]
        ] ],
        [ "Vertical Scaling", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1769", [
          [ "Resource Allocation", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1770", null ]
        ] ],
        [ "Database Scaling", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1771", [
          [ "Read Replicas", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1772", null ],
          [ "Sharding Strategy", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1773", null ]
        ] ]
      ] ],
      [ "Backup and Recovery", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1774", [
        [ "Backup Strategy", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1775", [
          [ "Application State Backup", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1776", null ],
          [ "Database Backup", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1777", null ]
        ] ],
        [ "Recovery Procedures", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1778", [
          [ "Service Recovery", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1779", null ],
          [ "Disaster Recovery Plan", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1780", null ]
        ] ]
      ] ],
      [ "Security Hardening", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1781", [
        [ "Network Security", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1782", [
          [ "Firewall Rules", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1783", null ],
          [ "SSL/TLS Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1784", null ]
        ] ],
        [ "Application Security", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1785", [
          [ "Authentication", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1786", null ],
          [ "Rate Limiting", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1787", null ]
        ] ],
        [ "Compliance", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1788", [
          [ "Audit Logging", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1789", null ],
          [ "Data Encryption", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1790", null ]
        ] ]
      ] ],
      [ "Troubleshooting Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1791", [
        [ "Common Issues", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1792", [
          [ "Service Won't Start", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1793", null ],
          [ "High Memory Usage", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1794", null ],
          [ "Performance Issues", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1795", null ]
        ] ],
        [ "Health Checks", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1796", [
          [ "Application Health", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1797", null ],
          [ "System Health", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1798", null ]
        ] ]
      ] ]
    ] ],
    [ "Design Patterns and Architectural Decisions", "d5/dca/md_docs_2DESIGN__PATTERNS.html", [
      [ "Overview", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1800", null ],
      [ "Creational Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1801", [
        [ "1. Singleton Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1802", null ],
        [ "2. Factory Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1803", null ],
        [ "3. Builder Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1804", null ],
        [ "4. Object Pool Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1805", null ]
      ] ],
      [ "Structural Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1806", [
        [ "5. Adapter Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1807", null ],
        [ "6. Decorator Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1808", null ],
        [ "7. Proxy Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1809", null ],
        [ "8. Composite Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1810", null ]
      ] ],
      [ "Behavioral Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1811", [
        [ "9. Observer Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1812", null ],
        [ "10. Strategy Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1813", null ],
        [ "11. Chain of Responsibility Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1814", null ],
        [ "12. Command Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1815", null ],
        [ "13. Template Method Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1816", null ]
      ] ],
      [ "Concurrency Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1817", [
        [ "14. Producer-Consumer Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1818", null ],
        [ "15. Thread Pool Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1819", null ]
      ] ],
      [ "Architectural Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1820", [
        [ "16. Microkernel Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1821", null ],
        [ "17. Event-Driven Architecture", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1822", null ]
      ] ],
      [ "Summary", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1823", null ]
    ] ],
    [ "Developer Guide", "db/dac/md_docs_2DEVELOPER__GUIDE.html", [
      [ "Quick Start", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1825", [
        [ "Prerequisites", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1826", null ],
        [ "1. Clone and Setup", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1827", null ],
        [ "2. Build the Project", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1828", null ],
        [ "3. Your First Application", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1829", null ]
      ] ],
      [ "Development Setup", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1830", [
        [ "IDE Configuration", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1831", [
          [ "Visual Studio Code", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1832", null ],
          [ "CLion", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1833", null ],
          [ "Visual Studio", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1834", null ]
        ] ],
        [ "Development Dependencies", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1835", null ]
      ] ],
      [ "Project Structure", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1836", [
        [ "Directory Layout", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1837", null ],
        [ "Module Organization", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1838", null ]
      ] ],
      [ "Coding Standards", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1839", [
        [ "C++ Style Guide", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1840", [
          [ "Naming Conventions", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1841", null ],
          [ "File Organization", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1842", null ],
          [ "Best Practices", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1843", null ]
        ] ],
        [ "Documentation Standards", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1844", [
          [ "Code Documentation", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1845", null ],
          [ "Comment Guidelines", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1846", null ]
        ] ]
      ] ],
      [ "Testing Guidelines", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1847", [
        [ "Unit Testing", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1848", [
          [ "Test Structure", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1849", null ],
          [ "Test Coverage", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1850", null ]
        ] ],
        [ "Integration Testing", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1851", null ],
        [ "Performance Testing", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1852", null ]
      ] ],
      [ "Debugging", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1853", [
        [ "Using GDB", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1854", null ],
        [ "Using Valgrind", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1855", null ],
        [ "Using AddressSanitizer", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1856", null ],
        [ "Using ThreadSanitizer", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1857", null ]
      ] ],
      [ "Performance Profiling", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1858", [
        [ "Using perf", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1859", null ],
        [ "Using Instruments (macOS)", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1860", null ],
        [ "Using Intel VTune", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1861", null ]
      ] ],
      [ "Contributing Guidelines", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1862", [
        [ "Workflow", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1863", null ],
        [ "Commit Message Format", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1864", null ],
        [ "Code Review Checklist", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1865", null ]
      ] ],
      [ "Build System", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1866", [
        [ "CMake Configuration", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1867", null ],
        [ "Creating New Modules", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1868", null ]
      ] ],
      [ "Deployment", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1869", [
        [ "Docker Deployment", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1870", null ],
        [ "Kubernetes Deployment", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1871", null ]
      ] ],
      [ "Security Best Practices", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1872", [
        [ "Input Validation", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1873", null ],
        [ "Secure Communication", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1874", null ],
        [ "Rate Limiting", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1875", null ]
      ] ],
      [ "Troubleshooting Common Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1876", [
        [ "Build Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1877", null ],
        [ "Runtime Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1878", null ],
        [ "Performance Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1879", null ]
      ] ]
    ] ],
    [ "Getting Started", "d2/d41/md_docs_2GETTING__STARTED.html", [
      [ "Table of Contents", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1881", null ],
      [ "Prerequisites", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1882", [
        [ "System Requirements", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1883", null ],
        [ "Development Dependencies", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1884", null ],
        [ "Runtime Dependencies", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1885", null ]
      ] ],
      [ "Installation", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1886", [
        [ "1. Clone the Repository", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1887", null ],
        [ "2. Platform-Specific Setup", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1888", [
          [ "Linux (Ubuntu/Debian)", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1889", null ],
          [ "macOS", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1890", null ],
          [ "Windows", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1891", null ]
        ] ],
        [ "3. Build the System", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1892", [
          [ "Quick Build (Recommended)", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1893", null ],
          [ "Custom Build Options", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1894", null ]
        ] ],
        [ "4. Verify Installation", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1895", null ]
      ] ],
      [ "Quick Start", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1896", [
        [ "1. Basic Message Bus Usage", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1897", null ],
        [ "2. Container-Based Data Handling", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1898", null ],
        [ "3. Network Client/Server", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1899", null ]
      ] ],
      [ "Basic Usage", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1900", [
        [ "Project Structure", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1901", null ],
        [ "Environment Configuration", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1902", null ],
        [ "Basic Configuration File", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1903", null ]
      ] ],
      [ "First Application", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1904", [
        [ "1. Chat Server (chat_server.cpp)", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1905", null ],
        [ "2. Build and Run", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1906", null ],
        [ "3. Test with Sample Client", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1907", null ]
      ] ],
      [ "Next Steps", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1908", [
        [ "1. Explore Sample Applications", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1909", null ],
        [ "2. Advanced Features", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1910", null ],
        [ "3. Development", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1911", null ],
        [ "4. Community", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1912", null ]
      ] ]
    ] ],
    [ "Performance Guide", "df/d94/md_docs_2performance.html", [
      [ "Table of Contents", "df/d94/md_docs_2performance.html#autotoc_md1915", null ],
      [ "Performance Overview", "df/d94/md_docs_2performance.html#autotoc_md1916", [
        [ "Design Goals", "df/d94/md_docs_2performance.html#autotoc_md1917", null ],
        [ "Key Performance Features", "df/d94/md_docs_2performance.html#autotoc_md1918", null ]
      ] ],
      [ "Benchmark Results", "df/d94/md_docs_2performance.html#autotoc_md1919", [
        [ "Test Environment", "df/d94/md_docs_2performance.html#autotoc_md1920", null ],
        [ "Overall System Performance", "df/d94/md_docs_2performance.html#autotoc_md1921", null ],
        [ "Latency Measurements", "df/d94/md_docs_2performance.html#autotoc_md1922", null ],
        [ "Memory Performance", "df/d94/md_docs_2performance.html#autotoc_md1923", null ]
      ] ],
      [ "Component Performance", "df/d94/md_docs_2performance.html#autotoc_md1924", [
        [ "Thread System Performance", "df/d94/md_docs_2performance.html#autotoc_md1925", [
          [ "Lock-free vs Mutex Comparison", "df/d94/md_docs_2performance.html#autotoc_md1926", null ],
          [ "Scaling Characteristics", "df/d94/md_docs_2performance.html#autotoc_md1927", null ]
        ] ],
        [ "Container System Performance", "df/d94/md_docs_2performance.html#autotoc_md1928", [
          [ "Serialization Performance", "df/d94/md_docs_2performance.html#autotoc_md1929", null ],
          [ "SIMD Optimization Impact", "df/d94/md_docs_2performance.html#autotoc_md1930", null ]
        ] ],
        [ "Network System Performance", "df/d94/md_docs_2performance.html#autotoc_md1931", [
          [ "Connection Scaling", "df/d94/md_docs_2performance.html#autotoc_md1932", null ],
          [ "Protocol Overhead", "df/d94/md_docs_2performance.html#autotoc_md1933", null ]
        ] ],
        [ "Database System Performance", "df/d94/md_docs_2performance.html#autotoc_md1934", [
          [ "Query Performance", "df/d94/md_docs_2performance.html#autotoc_md1935", null ],
          [ "Connection Pool Impact", "df/d94/md_docs_2performance.html#autotoc_md1936", null ]
        ] ]
      ] ],
      [ "Optimization Techniques", "df/d94/md_docs_2performance.html#autotoc_md1937", [
        [ "1. Memory Optimization", "df/d94/md_docs_2performance.html#autotoc_md1938", [
          [ "Object Pooling", "df/d94/md_docs_2performance.html#autotoc_md1939", null ],
          [ "Custom Allocators", "df/d94/md_docs_2performance.html#autotoc_md1940", null ]
        ] ],
        [ "2. CPU Optimization", "df/d94/md_docs_2performance.html#autotoc_md1941", [
          [ "SIMD Utilization", "df/d94/md_docs_2performance.html#autotoc_md1942", null ],
          [ "Cache Optimization", "df/d94/md_docs_2performance.html#autotoc_md1943", null ]
        ] ],
        [ "3. Network Optimization", "df/d94/md_docs_2performance.html#autotoc_md1944", [
          [ "Batching and Pipelining", "df/d94/md_docs_2performance.html#autotoc_md1945", null ]
        ] ]
      ] ],
      [ "Performance Monitoring", "df/d94/md_docs_2performance.html#autotoc_md1946", [
        [ "1. Built-in Metrics", "df/d94/md_docs_2performance.html#autotoc_md1947", null ],
        [ "2. Performance Profiling", "df/d94/md_docs_2performance.html#autotoc_md1948", null ]
      ] ],
      [ "Tuning Guidelines", "df/d94/md_docs_2performance.html#autotoc_md1949", [
        [ "1. Thread Configuration", "df/d94/md_docs_2performance.html#autotoc_md1950", null ],
        [ "2. Memory Configuration", "df/d94/md_docs_2performance.html#autotoc_md1951", null ],
        [ "3. Network Configuration", "df/d94/md_docs_2performance.html#autotoc_md1952", null ]
      ] ],
      [ "Troubleshooting Performance Issues", "df/d94/md_docs_2performance.html#autotoc_md1953", [
        [ "1. Common Performance Problems", "df/d94/md_docs_2performance.html#autotoc_md1954", [
          [ "High CPU Usage", "df/d94/md_docs_2performance.html#autotoc_md1955", null ],
          [ "Memory Leaks", "df/d94/md_docs_2performance.html#autotoc_md1956", null ],
          [ "Network Bottlenecks", "df/d94/md_docs_2performance.html#autotoc_md1957", null ]
        ] ],
        [ "2. Performance Monitoring Dashboard", "df/d94/md_docs_2performance.html#autotoc_md1958", null ]
      ] ]
    ] ],
    [ "Build Configuration Design Document", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html", [
      [ "Overview", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1961", null ],
      [ "Dependency Integration Strategy", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1962", [
        [ "Option 1: FetchContent (Development Environment)", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1963", null ],
        [ "Option 2: find_package (Production Environment)", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1964", null ]
      ] ],
      [ "Unified CMake Configuration Options", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1965", [
        [ "Feature Flags", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1966", null ],
        [ "Build Profiles", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1967", null ],
        [ "Compiler Options", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1968", null ]
      ] ],
      [ "Propagating Options to External Systems", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1969", [
        [ "Strategy", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1970", null ]
      ] ],
      [ "CMake Presets", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1971", [
        [ "CMakePresets.json", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1972", null ]
      ] ],
      [ "CI Profile Configurations", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1973", [
        [ "GitHub Actions Matrix", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1974", null ]
      ] ],
      [ "Documentation of Option Impact", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1975", [
        [ "MESSAGING_USE_LOCKFREE", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1976", null ],
        [ "MESSAGING_ENABLE_MONITORING", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1977", null ],
        [ "MESSAGING_ENABLE_LOGGING", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1978", null ],
        [ "MESSAGING_ENABLE_TLS", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1979", null ]
      ] ],
      [ "Installation Guide", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1980", [
        [ "System-Wide Installation (find_package mode)", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1981", null ],
        [ "FetchContent Mode (Development)", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1982", null ]
      ] ],
      [ "Completion Checklist", "d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1983", null ]
    ] ],
    [ "System Interface Mapping Document", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html", [
      [ "Overview", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1985", null ],
      [ "Common System Interfaces", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1986", [
        [ "Error Handling", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1987", null ],
        [ "Event System", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1988", null ],
        [ "Execution Contracts", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1989", null ],
        [ "Logging Contracts", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1990", null ],
        [ "Monitoring Contracts", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1991", null ],
        [ "Database Contracts", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1992", null ]
      ] ],
      [ "Error Code Ranges", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1993", [
        [ "System Error Code Allocation", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1994", null ]
      ] ],
      [ "Legacy Dependencies to Remove", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1995", [
        [ "Internal Implementations", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1996", null ],
        [ "External Dependencies", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1997", null ]
      ] ],
      [ "Exception Mapper Utility", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1998", [
        [ "Usage Pattern", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md1999", null ]
      ] ],
      [ "Migration Checklist", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md2000", [
        [ "Phase 0 Completion Criteria", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md2001", null ],
        [ "Next Steps", "df/dad/md_docs_2phase0_2INTERFACE__MAPPING.html#autotoc_md2002", null ]
      ] ]
    ] ],
    [ "Legacy Code Removal Plan", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html", [
      [ "Overview", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2004", null ],
      [ "Legacy Code Inventory", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2005", [
        [ "1. Internal Container System", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2006", null ],
        [ "2. Internal Network System", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2007", null ],
        [ "3. Internal Thread System", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2008", null ],
        [ "4. Exception-Based Error Handling", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2009", null ],
        [ "5. Direct Console Output", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2010", null ]
      ] ],
      [ "Responsibility Matrix", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2011", null ],
      [ "Detailed Replacement Mapping", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2012", [
        [ "Error Handling Transformation", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2013", [
          [ "Before (Exception-based)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2014", null ],
          [ "After (Result-based)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2015", null ]
        ] ],
        [ "Async Execution Transformation", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2016", [
          [ "Before (Internal thread pool)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2017", null ],
          [ "After (IExecutor-based)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2018", null ]
        ] ],
        [ "Container API Transformation", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2019", [
          [ "Before (Internal container)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2020", null ],
          [ "After (value_container)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2021", null ]
        ] ],
        [ "Logging Transformation", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2022", [
          [ "Before (Direct output)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2023", null ],
          [ "After (ILogger)", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2024", null ]
        ] ]
      ] ],
      [ "Archive Strategy", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2025", [
        [ "Archive Directory Structure", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2026", null ],
        [ "Archive Script", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2027", null ],
        [ "CMakeLists.txt Cleanup", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2028", null ]
      ] ],
      [ "Build Guard Implementation", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2029", [
        [ "Legacy Guard Header", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2030", null ],
        [ "CMake Configuration for Guards", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2031", null ]
      ] ],
      [ "Removal Checklist", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2032", [
        [ "Phase 1.4 Execution Checklist", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2033", null ],
        [ "Post-Removal Verification", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2034", null ]
      ] ],
      [ "Success Criteria", "dd/d64/md_docs_2phase0_2LEGACY__REMOVAL__PLAN.html#autotoc_md2035", null ]
    ] ],
    [ "Migration Strategy and Risk Assessment", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html", [
      [ "Overview", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2037", null ],
      [ "Breaking Changes", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2038", [
        [ "1. Error Handling Paradigm Shift", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2039", null ],
        [ "2. External System Dependencies", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2040", null ],
        [ "3. Container API Changes", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2041", null ]
      ] ],
      [ "Compatibility Layer Design", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2042", [
        [ "Legacy API Wrapper", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2043", null ],
        [ "Feature Flag Configuration", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2044", null ]
      ] ],
      [ "Gradual Migration Scenario", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2045", [
        [ "Release Timeline", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2046", [
          [ "v1.9 (Current)", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2047", null ],
          [ "v1.10 (Transition Release)", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2048", null ],
          [ "v1.11 (Final Transition)", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2049", null ],
          [ "v2.0 (New Architecture)", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2050", null ]
        ] ],
        [ "Migration Schedule", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2051", null ]
      ] ],
      [ "Rollback Strategy", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2052", [
        [ "Blue-Green Deployment", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2053", [
          [ "Preparation", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2054", null ],
          [ "Cutover", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2055", null ],
          [ "Rollback (If Issues Detected)", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2056", null ]
        ] ],
        [ "Database Migration Rollback", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2057", [
          [ "Forward Migration Script", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2058", null ],
          [ "Rollback Script", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2059", null ],
          [ "Execution", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2060", null ]
        ] ],
        [ "Configuration Rollback", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2061", [
          [ "Configuration Versioning", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2062", null ],
          [ "Ansible Rollback Playbook", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2063", null ]
        ] ]
      ] ],
      [ "Risk Register", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2064", [
        [ "High Risk Items", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2065", [
          [ "Risk 1: Performance Regression", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2066", null ],
          [ "Risk 2: Data Loss During Migration", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2067", null ],
          [ "Risk 3: Client Breaking Changes", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2068", null ]
        ] ],
        [ "Medium Risk Items", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2069", [
          [ "Risk 4: Dependency Version Conflicts", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2070", null ],
          [ "Risk 5: Build System Issues", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2071", null ]
        ] ]
      ] ],
      [ "Risk Mitigation Checklist", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2072", [
        [ "Pre-Migration", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2073", null ],
        [ "During Migration", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2074", null ],
        [ "Post-Migration", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2075", null ]
      ] ],
      [ "Communication Plan", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2076", [
        [ "Stakeholders", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2077", null ],
        [ "Notification Timeline", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2078", null ]
      ] ],
      [ "Success Criteria", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2079", [
        [ "Technical Criteria", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2080", null ],
        [ "Business Criteria", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2081", null ]
      ] ],
      [ "Completion Checklist", "d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2082", null ]
    ] ],
    [ "Phase 1: Build System and Dependency Refactoring - Design Document", "d5/d7c/md_docs_2phase1_2DESIGN.html", [
      [ "Overview", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2084", null ],
      [ "Architecture Diagram", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2085", null ],
      [ "Task 1.1: External Module Integration", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2086", [
        [ "Updated CMakeLists.txt Structure", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2087", null ],
        [ "Dependency Validation Script", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2088", null ]
      ] ],
      [ "Task 1.2: Compiler Options and Feature Flags", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2089", [
        [ "Flag Propagation Strategy", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2090", null ],
        [ "CMakePresets.json (Already created in Phase 0)", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2091", null ]
      ] ],
      [ "Task 1.3: Configuration Validation", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2092", [
        [ "Validation Checklist", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2093", null ],
        [ "Post-build Verification", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2094", null ]
      ] ],
      [ "Task 1.4: Legacy Code Removal", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2095", [
        [ "Execution Steps", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2096", null ]
      ] ],
      [ "Success Criteria", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2097", [
        [ "Build Success", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2098", null ],
        [ "Functionality", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2099", null ],
        [ "Performance", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2100", null ],
        [ "Documentation", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2101", null ]
      ] ],
      [ "Implementation Timeline", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2102", null ],
      [ "Rollback Plan", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2103", null ],
      [ "Next Phase", "d5/d7c/md_docs_2phase1_2DESIGN.html#autotoc_md2104", null ]
    ] ],
    [ "Phase 2: Messaging Core Redesign - Design Document", "d6/df5/md_docs_2phase2_2DESIGN.html", [
      [ "Overview", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2106", null ],
      [ "Core Components", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2107", [
        [ "1. Message Container DSL", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2108", null ],
        [ "2. Error Code Definitions", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2109", null ],
        [ "3. Message Bus with DI", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2110", null ],
        [ "4. Topic Router", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2111", null ]
      ] ],
      [ "Result<T> Migration Patterns", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2112", [
        [ "Pattern 1: Simple Function Conversion", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2113", null ],
        [ "Pattern 2: Error Propagation with RETURN_IF_ERROR", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2114", null ],
        [ "Pattern 3: Monadic Chaining", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2115", null ]
      ] ],
      [ "Performance Targets", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2116", [
        [ "Message Container", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2117", null ],
        [ "Message Bus", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2118", null ],
        [ "Topic Router", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2119", null ]
      ] ],
      [ "Implementation Plan", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2120", [
        [ "Week 1: Message Container", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2121", null ],
        [ "Week 2: Result<T> Migration", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2122", null ],
        [ "Week 3: DI Architecture", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2123", null ],
        [ "Week 4: Topic Router", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2124", null ]
      ] ],
      [ "Testing Strategy", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2125", [
        [ "Unit Tests", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2126", null ],
        [ "Integration Tests", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2127", null ],
        [ "Benchmarks", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2128", null ]
      ] ],
      [ "Success Criteria", "d6/df5/md_docs_2phase2_2DESIGN.html#autotoc_md2129", null ]
    ] ],
    [ "Phase 3: Infrastructure Integration - Design Document", "d7/de1/md_docs_2phase3_2DESIGN.html", [
      [ "Overview", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2131", null ],
      [ "Architecture", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2132", null ],
      [ "Component Designs", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2133", [
        [ "1. Network Bridge (Task 3.1)", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2134", null ],
        [ "2. Persistent Message Queue (Task 3.2)", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2135", null ],
        [ "3. Tracing and Monitoring (Task 3.3)", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2136", null ],
        [ "4. Configuration System (Task 3.4)", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2137", null ]
      ] ],
      [ "Integration Points", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2138", [
        [ "Startup Sequence", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2139", null ]
      ] ],
      [ "Performance Targets", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2140", null ],
      [ "Success Criteria", "d7/de1/md_docs_2phase3_2DESIGN.html#autotoc_md2141", null ]
    ] ],
    [ "Phase 4: Validation and Deployment - Design Document", "db/d5d/md_docs_2phase4_2DESIGN.html", [
      [ "Overview", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2143", null ],
      [ "Architecture", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2144", null ],
      [ "Component Designs", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2145", [
        [ "1. Unit Test Framework (Task 4.1)", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2146", null ],
        [ "2. Integration Tests (Task 4.1)", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2147", null ],
        [ "3. Benchmarks (Task 4.2)", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2148", null ],
        [ "4. Security Review (Task 4.4)", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2149", null ]
      ] ],
      [ "Address Sanitizer", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2150", null ],
      [ "Thread Sanitizer", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2151", null ],
      [ "Undefined Behavior Sanitizer", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2152", null ],
      [ "clang-tidy", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2153", null ],
      [ "cppcheck", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2154", [
        [ "Testing Strategy", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2157", [
          [ "5. Performance Validation (Task 4.2)", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2155", null ],
          [ "6. Deployment Artifacts (Task 4.5)", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2156", null ],
          [ "Validation Phases", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2158", null ]
        ] ],
        [ "Performance Targets", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2159", null ],
        [ "Success Criteria", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2160", null ],
        [ "Implementation Timeline", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2161", null ],
        [ "Rollback Plan", "db/d5d/md_docs_2phase4_2DESIGN.html#autotoc_md2162", null ]
      ] ]
    ] ],
    [ "Phase 4: Test Suite Summary", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html", [
      [ "Overview", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2164", null ],
      [ "Test Structure", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2165", null ],
      [ "Unit Tests", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2166", [
        [ "1. MessagingContainer Tests (test_messaging_container.cpp)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2167", null ],
        [ "2. TopicRouter Tests (test_topic_router.cpp)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2169", null ],
        [ "3. MessageBus Tests (test_message_bus.cpp)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2171", null ],
        [ "4. TraceContext Tests (test_trace_context.cpp)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2173", null ],
        [ "5. ConfigLoader Tests (test_config_loader.cpp)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2175", null ]
      ] ],
      [ "Integration Tests", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2177", [
        [ "End-to-End Integration (test_end_to_end.cpp)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2178", [
          [ "1. Complete Pub/Sub Flow (<tt>test_complete_pubsub_flow</tt>)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2179", null ],
          [ "2. Complex Routing (<tt>test_complex_routing_scenario</tt>)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2180", null ],
          [ "3. Multi-Subscriber Coordination (<tt>test_multi_subscriber_coordination</tt>)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2181", null ],
          [ "4. High Throughput (<tt>test_high_throughput_scenario</tt>)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2182", null ],
          [ "5. Subscription Lifecycle (<tt>test_subscribe_unsubscribe_lifecycle</tt>)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2183", null ],
          [ "6. Config-Driven Initialization (<tt>test_config_driven_initialization</tt>)", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2184", null ]
        ] ]
      ] ],
      [ "Test Execution", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2186", [
        [ "Build with Tests", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2187", null ],
        [ "Run Tests", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2188", null ],
        [ "Test Summary", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2189", null ]
      ] ],
      [ "Test Coverage", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2191", [
        [ "Components Tested", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2192", null ],
        [ "Feature Coverage", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2193", null ]
      ] ],
      [ "Performance Benchmarks", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2195", null ],
      [ "Known Limitations", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2197", null ],
      [ "Test Dependencies", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2199", [
        [ "Required", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2200", null ],
        [ "Optional", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2201", null ]
      ] ],
      [ "Future Enhancements", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2203", null ],
      [ "Conclusion", "df/d5e/md_docs_2phase4_2TEST__SUMMARY.html#autotoc_md2205", null ]
    ] ],
    [ "Messaging System v2.0 - Project Completion Summary", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html", [
      [ "Overview", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2207", null ],
      [ "Project Timeline", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2209", [
        [ "Phase 0: Foundation and Preparation ✅", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2210", null ],
        [ "Phase 1: Build System and External Integration ✅", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2212", null ],
        [ "Phase 2: Messaging Core Implementation ✅", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2214", null ],
        [ "Phase 3: Infrastructure Integration ✅", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2216", null ],
        [ "Phase 4: Comprehensive Test Suite ✅", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2218", null ],
        [ "Build System Improvements ✅", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2220", null ]
      ] ],
      [ "Technical Architecture", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2222", [
        [ "Core Components", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2223", null ],
        [ "External System Integration", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2224", null ],
        [ "Key Design Patterns", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2225", null ]
      ] ],
      [ "Performance Characteristics", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2227", [
        [ "Benchmarks (from integration tests)", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2228", null ],
        [ "Resource Usage", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2229", null ]
      ] ],
      [ "Documentation Deliverables", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2231", [
        [ "Phase-Specific Docs", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2232", null ],
        [ "Additional Documentation", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2233", null ],
        [ "Code Documentation", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2234", null ]
      ] ],
      [ "Commit History", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2236", null ],
      [ "Known Limitations and Future Work", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2238", [
        [ "Current Limitations", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2239", null ],
        [ "Future Enhancements", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2240", null ]
      ] ],
      [ "Success Metrics", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2242", [
        [ "Quantitative", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2243", null ],
        [ "Qualitative", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2244", null ]
      ] ],
      [ "Lessons Learned", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2246", null ],
      [ "Conclusion", "da/da3/md_docs_2PROJECT__COMPLETION__SUMMARY.html#autotoc_md2248", null ]
    ] ],
    [ "Messaging System 재구성 실행 계획", "d7/d01/md_docs_2REBUILD__PLAN.html", [
      [ "개요 및 우선순위", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2266", null ],
      [ "Phase 0. 준비 및 기반 정비", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2267", [
        [ "**Task 0.0 – common_system 즉시 통합 (최우선)**", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2268", null ],
        [ "<strong>Task 0.1 – 시스템 인터페이스 맵핑</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2269", null ],
        [ "<strong>Task 0.2 – 빌드 환경 감사</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2270", null ],
        [ "<strong>Task 0.3 – 위험 및 마이그레이션 전략 확정</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2271", null ],
        [ "<strong>Task 0.4 – 레거시 구현 차단 계획 수립</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2272", null ]
      ] ],
      [ "Phase 1. 빌드 및 의존성 리팩터링", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2273", [
        [ "<strong>Task 1.1 – 외부 모듈 통합 재구성</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2274", null ],
        [ "<strong>Task 1.2 – 컴파일 옵션 및 피처 플래그 통합</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2275", null ],
        [ "<strong>Task 1.3 – 구성 검증 자동화</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2276", null ],
        [ "<strong>Task 1.4 – 레거시 코드 비활성화 및 제거</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2277", null ]
      ] ],
      [ "Phase 2. 메시징 코어 재설계", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2278", [
        [ "<strong>Task 2.1 – 메시지 컨테이너 DSL 정립</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2279", null ],
        [ "<strong>Task 2.2 – Result 기반 플로우 제로화</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2280", null ],
        [ "<strong>Task 2.3 – IExecutor 주입식 아키텍처 구현</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2281", null ],
        [ "<strong>Task 2.4 – 라우팅/토픽 엔진 정비</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2282", null ]
      ] ],
      [ "Phase 3. 인프라 통합 계층 구축", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2283", [
        [ "<strong>Task 3.1 – 네트워크 경계 계층 구현</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2284", null ],
        [ "<strong>Task 3.2 – 영속성 및 재처리 경로 구성</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2285", null ],
        [ "<strong>Task 3.3 – 로깅/모니터링 데이터 흐름 통합</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2286", null ],
        [ "<strong>Task 3.4 – 구성 및 정책 레이어</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2287", null ]
      ] ],
      [ "Phase 4. 검증·배포 준비", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2288", [
        [ "<strong>Task 4.1 – 교차 모듈 테스트 수립</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2289", null ],
        [ "**Task 4.2 – 마이그레이션 가이드 및 API 문서화**", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2290", null ],
        [ "<strong>Task 4.3 – 성능 및 회귀 검증</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2291", null ],
        [ "<strong>Task 4.4 – 보안 검토 및 강화</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2292", null ],
        [ "<strong>Task 4.5 – 출시 체크리스트/런북 작성</strong>", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2293", null ]
      ] ],
      [ "전체 일정 요약", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2295", null ],
      [ "병렬 작업 최적화", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2296", null ],
      [ "성공 기준 (최종)", "d7/d01/md_docs_2REBUILD__PLAN.html#autotoc_md2297", null ]
    ] ],
    [ "System Architecture", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html", [
      [ "Overview", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2300", null ],
      [ "Architectural Principles", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2301", null ],
      [ "System Layers", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2302", null ],
      [ "Component Architecture", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2303", [
        [ "1. Container System", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2304", null ],
        [ "2. Network System", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2305", null ],
        [ "3. Database System", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2306", null ],
        [ "4. Thread System", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2307", null ]
      ] ],
      [ "Data Flow Architecture", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2308", [
        [ "Message Processing Pipeline", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2309", null ],
        [ "Request-Response Flow", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2310", null ],
        [ "Distributed Architecture", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2311", null ]
      ] ],
      [ "Scalability Patterns", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2312", [
        [ "Horizontal Scaling", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2313", null ],
        [ "Vertical Scaling", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2314", null ]
      ] ],
      [ "Integration Points", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2315", [
        [ "External Systems", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2316", null ],
        [ "Internal Communication", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2317", null ]
      ] ],
      [ "Performance Optimization", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2318", [
        [ "Memory Management", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2319", null ],
        [ "Concurrency", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2320", null ],
        [ "Network", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2321", null ]
      ] ],
      [ "Fault Tolerance", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2322", [
        [ "Error Recovery", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2323", null ],
        [ "High Availability", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2324", null ]
      ] ],
      [ "Security Considerations", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2325", [
        [ "Transport Security", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2326", null ],
        [ "Application Security", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2327", null ]
      ] ],
      [ "Monitoring and Observability", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2328", [
        [ "Metrics Collection", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2329", null ],
        [ "Distributed Tracing", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2330", null ],
        [ "Logging", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2331", null ]
      ] ],
      [ "Configuration Management", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2332", [
        [ "Static Configuration", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2333", null ],
        [ "Dynamic Configuration", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2334", null ]
      ] ],
      [ "Deployment Architecture", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2335", [
        [ "Container Deployment", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2336", null ],
        [ "Kubernetes Integration", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2337", null ]
      ] ],
      [ "Future Considerations", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2338", [
        [ "Planned Enhancements", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2339", null ],
        [ "Technology Evaluation", "da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2340", null ]
      ] ]
    ] ],
    [ "Troubleshooting Guide", "d4/d2f/md_docs_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2342", null ],
      [ "FAQ", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2343", [
        [ "General Questions", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2344", null ],
        [ "Performance Questions", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2345", null ],
        [ "Configuration Questions", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2346", null ]
      ] ],
      [ "Debug Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2347", [
        [ "Enable Debug Logging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2348", [
          [ "Runtime Configuration", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2349", null ],
          [ "Environment Variables", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2350", null ],
          [ "Programmatic", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2351", null ]
        ] ],
        [ "Using GDB for Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2352", [
          [ "Attach to Running Process", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2353", null ],
          [ "Debug Core Dumps", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2354", null ]
        ] ],
        [ "Memory Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2355", [
          [ "Using Valgrind", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2356", null ],
          [ "Using AddressSanitizer", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2357", null ],
          [ "Using HeapTrack", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2358", null ]
        ] ],
        [ "Network Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2359", [
          [ "TCP Dump", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2360", null ],
          [ "Network Statistics", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2361", null ],
          [ "Test Connectivity", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2362", null ]
        ] ],
        [ "Thread Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2363", [
          [ "Thread Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2364", null ],
          [ "Detect Deadlocks", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2365", null ],
          [ "ThreadSanitizer", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2366", null ]
        ] ],
        [ "Tracing and Profiling", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2367", [
          [ "System Tracing with strace", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2368", null ],
          [ "Performance Profiling with perf", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2369", null ],
          [ "Application Tracing", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2370", null ]
        ] ]
      ] ],
      [ "Performance Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2371", [
        [ "CPU Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2372", [
          [ "Identify CPU Bottlenecks", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2373", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2374", null ]
        ] ],
        [ "Memory Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2375", [
          [ "Memory Usage Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2376", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2377", null ]
        ] ],
        [ "I/O Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2378", [
          [ "Disk I/O Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2379", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2380", null ]
        ] ],
        [ "Network Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2381", [
          [ "Network Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2382", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2383", null ]
        ] ]
      ] ],
      [ "Known Issues", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2384", [
        [ "Issue 1: High CPU Usage with Small Messages", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2385", null ],
        [ "Issue 2: Memory Leak in Long-Running Connections", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2386", null ],
        [ "Issue 3: Database Connection Pool Exhaustion", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2387", null ],
        [ "Issue 4: Deadlock in Message Processing", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2388", null ],
        [ "Issue 5: Performance Degradation with Many Topics", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2389", null ]
      ] ],
      [ "Error Messages", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2390", [
        [ "Connection Errors", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2391", [
          [ "\"Connection refused\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2392", null ],
          [ "\"Connection timeout\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2393", null ]
        ] ],
        [ "Database Errors", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2394", [
          [ "\"Database connection failed\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2395", null ],
          [ "\"Deadlock detected\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2396", null ]
        ] ],
        [ "Memory Errors", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2397", [
          [ "\"Out of memory\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2398", null ],
          [ "\"Segmentation fault\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2399", null ]
        ] ]
      ] ],
      [ "Common Problems", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2400", [
        [ "Problem: Service Won't Start", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2401", null ],
        [ "Problem: Slow Message Processing", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2402", null ],
        [ "Problem: Connection Drops", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2403", null ],
        [ "Problem: High Memory Usage", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2404", null ],
        [ "Problem: Database Bottleneck", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2405", null ]
      ] ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", "namespacemembers_dup" ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Concepts", "concepts.html", "concepts" ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"d0/d67/classtyped__thread__pool__module_1_1typed__job__interface.html#aed690feb206238bedfbadc566ca7eba6",
"d0/da1/md_libraries_2monitoring__system_2docs_2api-reference.html#autotoc_md479",
"d1/d1a/namespacecontainer__module.html#ac5fba890843eac300daef1fd61c5ff01a50c55af2afeb25bf758254cc1dbd20ec",
"d1/d57/md_docs_2phase0_2BUILD__CONFIGURATION.html#autotoc_md1977",
"d1/dcb/classthread__module_1_1logger__interface.html#ab7259d5cefc6b03319317b4921db7d52",
"d1/df7/logger__memory__benchmark_8cpp.html#a6c423e2e2b93292e0e726ec9439a62d6",
"d2/d8e/classlogger__module_1_1log__collector_1_1impl.html#a71911e4c16cb51090d70c35591209681",
"d2/d9e/classtyped__thread__pool__module_1_1adaptive__typed__job__queue__t.html#a4940f8b314143da9418c74bcaecdd2b8afe210246000a180fd0d5f617009a7745",
"d2/dca/namespacethread__module_1_1jobs.html#ab7e88d20a4feef6010627676c6cc6a93a75101dcdfc88455bcafc9e53e0b06689",
"d3/d02/monitoring__crash__safety_8h.html#a8232a34f1f1448fb35bad26059c2d6ec",
"d3/d35/classtyped__thread__pool__module_1_1typed__lockfree__job__queue__t.html#ad095b5aac513c3dbf1dcd8652582beb4",
"d3/d70/md_docs_2phase0_2MIGRATION__STRATEGY.html#autotoc_md2065",
"d3/dce/classTestMonitoring.html#add013626938fe445956b7ee80941de07",
"d4/d25/modular__structure_2core_2include_2thread__system__core_2thread__pool_2workers_2worker__policy_8h.html#a91dfb70040110beaa487b8abbdc8d9fdac76a5e84e4bdee527e274ea30c680d79",
"d4/d75/classkcenon_1_1messaging_1_1routing_1_1message__relay.html#a7e34a21d91c7b8da28890e57207f8d45",
"d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1183",
"d4/dd6/typed__thread__pool__sample_8cpp.html#a69de338eb61ed8f66615f17866c16da9",
"d5/d22/classcontainer__module_1_1string__value.html#a7b65943be3bd8f514551b40dadd74e9d",
"d5/d7d/classmonitoring__module_1_1tiered__storage.html#ae078be0774859787f2a1fe804cb31c91",
"d5/dd2/structmonitoring__module_1_1monitoring_1_1monitoring__stats.html#ae4fa5a2eb2b6f94e068a6d4bdba6d428",
"d5/df9/classcontainer__module_1_1value.html#abf0473232acd0fc08e95a084d3e81ed6",
"d6/d3b/structmonitoring__interface_1_1thread__pool__metrics.html#a303c441cc66a7ebbf1562853230ba9ae",
"d6/d8c/classnull__writer.html#a941cd2576c07e65aafe89a85f50806ea",
"d6/dc5/md_libraries_2logger__system_2CHANGELOG.html#autotoc_md182",
"d7/d1b/classkcenon_1_1messaging_1_1cluster_1_1distributed__broker__builder.html#a2424402667cbae6901c78005daa133e1",
"d7/d5c/classlogger__module_1_1router__builder.html#a3c2aa22f75d48eec7f7b70aa4287fb00",
"d7/dc0/classmonitoring__module_1_1ring__buffer.html#a5c28c675624ef7b4bed32f47a24cd405",
"d8/d26/convert__string__test_8cpp.html#a37ed6a912dd972f0f5b478d42d2237cc",
"d8/d4f/classtyped__thread__pool__module_1_1typed__job__t.html#a8122d8a0b487f3f64d4d98a6ed6a095f",
"d8/d4f/classutility__module_1_1span.html#af11daa6bd31b86d8e08d6434965fd148",
"d8/d98/namespacethread__pool__module.html#a91dfb70040110beaa487b8abbdc8d9fd",
"d8/db7/classanalysis__visualizer.html#a49aef1ed9c435d70e0bb6ae92f774175",
"d9/d2e/classiot__monitoring__system.html#aee32674506cc64f21d5ae7c4b54ffb85",
"d9/d67/classthread__module_1_1adaptive__job__queue.html#a99ade5899f88831d716301436307624ea8640c650d24d411eed108592294d9750",
"d9/d9b/md_libraries_2logger__system_2ARCHITECTURE.html#autotoc_md134",
"d9/dad/classcontainer__module_1_1value__container.html#ae4aa9e95aadd89c1f2bb0d82a002b20a",
"da/d2e/classlogger__module_1_1logger.html#a59ca1ba7748200834f3e329c1509a887",
"da/d69/structkcenon_1_1messaging_1_1security_1_1authentication__manager_1_1auth__statistics.html",
"da/db4/md_docs_2SYSTEM__ARCHITECTURE.html#autotoc_md2300",
"da/df8/classthread__module_1_1sync_1_1condition__variable__wrapper.html#ac0beba8e3bed8e4bfe73c4cdf5f86db0",
"db/d54/classlogger__module_1_1log__sanitizer.html#af8ce7353abe1280767d4980f90edb0c8",
"db/dac/md_docs_2DEVELOPER__GUIDE.html",
"db/de8/classtyped__thread__pool__module_1_1typed__job__queue__t.html#a42501ddf68f746e78064242d0884f267",
"dc/d06/modular__structure_2core_2src_2thread__base_2sync_2sync__primitives_8h_source.html",
"dc/d4e/md_libraries_2monitoring__system_2ARCHITECTURE.html#autotoc_md420",
"dc/d7f/classthread__module_1_1callback__job.html#adac1dc1c40f32c01fc2db47508fc5ef6",
"dc/dd1/classtyped__thread__pool__module_1_1typed__thread__pool__t.html#af7e2734721f98ba405a74466361841da",
"dd/d11/classthread__module_1_1test_1_1ErrorHandlingTest.html#aced9e6617a3c36ce6f1fe7180525699f",
"dd/d8d/external__system__adapter_8h.html#a9709b17de9726ec32bb398f5868ce728",
"dd/de7/structfmt_1_1formatter_3_01typed__thread__pool__module_1_1job__types_00_01wchar__t_01_4.html#ac6ed7b3b17851d44c12b5e4a5468efe7",
"de/d42/monitoring__system_2samples_2crash__protection_2main_8cpp.html#a28dae233a4d9179b2d2d6e346ca22a0f",
"de/d48/namespacethread__module.html#a5a2719d6d43f27e1e8e913fbfa5c9288",
"de/d56/distributed__logging__demo_8cpp.html#ae66f6b31b5ad750f1fe042a706a4e3d4",
"de/dc4/classcontainer__module_1_1numeric__value.html#aa39d372c4fb170e8428b54460270869e",
"df/d1b/structkcenon_1_1messaging_1_1services_1_1network_1_1network__config.html",
"df/d38/classthread__pool__module_1_1pool__future.html#ade5ca83f691d58df6ad70e2e900f3c03",
"df/d6d/classthread__module_1_1job__queue.html#a1395d9077f9fe6a16990a1965fb678b3",
"df/dac/modular__structure_2core_2include_2thread__system__core_2thread__base_2lockfree_2lockfree__config_8h.html#a07cdd7466a09a6c46b1f7a1121ab9037a86a4fa105ff51b8c3be84734797d8144",
"dir_a130bef755e89bb421c009f1cf85cd40.html",
"namespacemembers.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';