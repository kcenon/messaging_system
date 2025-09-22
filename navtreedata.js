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
      [ "Component Documentation Links", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1550", null ],
      [ "Container System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1552", [
        [ "Namespace: <tt>container_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1553", null ],
        [ "Class: <tt>value_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1554", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1555", null ],
          [ "Core Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1556", [
            [ "Setting Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1557", null ],
            [ "Adding Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1558", null ],
            [ "Retrieving Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1559", null ],
            [ "Serialization", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1560", null ]
          ] ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1561", null ]
        ] ],
        [ "Class: <tt>variant</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1562", [
          [ "Supported Types", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1563", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1564", null ]
        ] ]
      ] ],
      [ "Network System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1566", [
        [ "Namespace: <tt>network_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1567", null ],
        [ "Class: <tt>messaging_server</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1568", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1569", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1570", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1571", null ]
        ] ],
        [ "Class: <tt>messaging_client</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1572", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1573", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1574", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1575", null ]
        ] ]
      ] ],
      [ "Database System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1577", [
        [ "Namespace: <tt>database</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1578", null ],
        [ "Class: <tt>database_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1579", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1580", null ],
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1581", null ],
          [ "Connection Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1582", null ],
          [ "Query Execution", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1583", null ],
          [ "Transaction Support", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1584", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1585", null ]
        ] ]
      ] ],
      [ "Message Bus API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1587", [
        [ "Namespace: <tt>kcenon::messaging::core</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1588", null ],
        [ "Class: <tt>message_bus</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1589", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1590", null ],
          [ "Configuration Structure", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1591", null ],
          [ "Lifecycle Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1592", null ],
          [ "Publishing", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1593", null ],
          [ "Subscription", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1594", null ],
          [ "Request-Response", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1595", null ],
          [ "Monitoring", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1596", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1597", null ]
        ] ]
      ] ],
      [ "Service Container API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1599", [
        [ "Namespace: <tt>kcenon::messaging::services</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1600", null ],
        [ "Class: <tt>service_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1601", [
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1602", null ],
          [ "Service Lifetimes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1603", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1604", null ]
        ] ]
      ] ],
      [ "Thread System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1606", [
        [ "Namespace: <tt>thread_system</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1607", null ],
        [ "Class: <tt>thread_pool</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1608", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1609", null ],
          [ "Job Submission", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1610", null ],
          [ "Pool Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1611", null ],
          [ "Priority Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1612", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1613", null ]
        ] ],
        [ "Class: <tt>lock_free_queue</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1614", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1615", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1616", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1617", null ]
        ] ]
      ] ],
      [ "Logger System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1619", [
        [ "Namespace: <tt>logger</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1620", null ],
        [ "Class: <tt>logger_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1621", [
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1622", null ],
          [ "Log Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1623", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1624", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1625", null ]
        ] ]
      ] ],
      [ "Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1627", [
        [ "System Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1628", null ],
        [ "Error Handling", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1629", null ],
        [ "Error Recovery Strategies", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1630", null ]
      ] ],
      [ "Configuration Reference", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1631", [
        [ "System Configuration File Format", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1632", null ],
        [ "Environment Variables", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md1633", null ]
      ] ]
    ] ],
    [ "Architecture", "d2/d64/md_docs_2architecture.html", [
      [ "Overview", "d2/d64/md_docs_2architecture.html#autotoc_md1635", null ],
      [ "Related Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md1636", [
        [ "Component-Specific Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1637", null ],
        [ "Module Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md1638", null ],
        [ "Integration Guides", "d2/d64/md_docs_2architecture.html#autotoc_md1639", null ]
      ] ],
      [ "Architectural Principles", "d2/d64/md_docs_2architecture.html#autotoc_md1640", null ],
      [ "System Layers", "d2/d64/md_docs_2architecture.html#autotoc_md1641", null ],
      [ "Component Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1642", [
        [ "1. Container System", "d2/d64/md_docs_2architecture.html#autotoc_md1643", null ],
        [ "2. Network System", "d2/d64/md_docs_2architecture.html#autotoc_md1644", null ],
        [ "3. Database System", "d2/d64/md_docs_2architecture.html#autotoc_md1645", null ],
        [ "4. Thread System", "d2/d64/md_docs_2architecture.html#autotoc_md1646", null ]
      ] ],
      [ "Data Flow Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1647", [
        [ "Message Processing Pipeline", "d2/d64/md_docs_2architecture.html#autotoc_md1648", null ],
        [ "Request-Response Flow", "d2/d64/md_docs_2architecture.html#autotoc_md1649", null ],
        [ "Distributed Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1650", null ]
      ] ],
      [ "Scalability Patterns", "d2/d64/md_docs_2architecture.html#autotoc_md1651", [
        [ "Horizontal Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md1652", null ],
        [ "Vertical Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md1653", null ]
      ] ],
      [ "Integration Points", "d2/d64/md_docs_2architecture.html#autotoc_md1654", [
        [ "External Systems", "d2/d64/md_docs_2architecture.html#autotoc_md1655", null ],
        [ "Internal Communication", "d2/d64/md_docs_2architecture.html#autotoc_md1656", null ]
      ] ],
      [ "Performance Optimization", "d2/d64/md_docs_2architecture.html#autotoc_md1657", [
        [ "Memory Management", "d2/d64/md_docs_2architecture.html#autotoc_md1658", null ],
        [ "Concurrency", "d2/d64/md_docs_2architecture.html#autotoc_md1659", null ],
        [ "Network", "d2/d64/md_docs_2architecture.html#autotoc_md1660", null ]
      ] ],
      [ "Fault Tolerance", "d2/d64/md_docs_2architecture.html#autotoc_md1661", [
        [ "Error Recovery", "d2/d64/md_docs_2architecture.html#autotoc_md1662", null ],
        [ "High Availability", "d2/d64/md_docs_2architecture.html#autotoc_md1663", null ]
      ] ],
      [ "Security Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md1664", [
        [ "Transport Security", "d2/d64/md_docs_2architecture.html#autotoc_md1665", null ],
        [ "Application Security", "d2/d64/md_docs_2architecture.html#autotoc_md1666", null ]
      ] ],
      [ "Monitoring and Observability", "d2/d64/md_docs_2architecture.html#autotoc_md1667", [
        [ "Metrics Collection", "d2/d64/md_docs_2architecture.html#autotoc_md1668", null ],
        [ "Distributed Tracing", "d2/d64/md_docs_2architecture.html#autotoc_md1669", null ],
        [ "Logging", "d2/d64/md_docs_2architecture.html#autotoc_md1670", null ]
      ] ],
      [ "Configuration Management", "d2/d64/md_docs_2architecture.html#autotoc_md1671", [
        [ "Static Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md1672", null ],
        [ "Dynamic Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md1673", null ]
      ] ],
      [ "Deployment Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md1674", [
        [ "Container Deployment", "d2/d64/md_docs_2architecture.html#autotoc_md1675", null ],
        [ "Kubernetes Integration", "d2/d64/md_docs_2architecture.html#autotoc_md1676", null ]
      ] ],
      [ "Future Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md1677", [
        [ "Planned Enhancements", "d2/d64/md_docs_2architecture.html#autotoc_md1678", null ],
        [ "Technology Evaluation", "d2/d64/md_docs_2architecture.html#autotoc_md1679", null ]
      ] ]
    ] ],
    [ "Deployment Guide", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html", [
      [ "System Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1681", [
        [ "Hardware Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1682", [
          [ "Minimum Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1683", null ],
          [ "Recommended Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1684", null ],
          [ "Production Requirements", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1685", null ]
        ] ],
        [ "Operating System Support", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1686", null ],
        [ "Software Dependencies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1687", [
          [ "Build Dependencies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1688", null ],
          [ "Runtime Dependencies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1689", null ]
        ] ]
      ] ],
      [ "Installation", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1690", [
        [ "From Source", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1691", [
          [ "1. Clone Repository", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1692", null ],
          [ "2. Configure Build", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1693", null ],
          [ "3. Build and Install", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1694", null ]
        ] ],
        [ "Using Package Managers", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1695", [
          [ "APT Repository (Ubuntu/Debian)", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1696", null ],
          [ "YUM Repository (RHEL/CentOS)", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1697", null ]
        ] ],
        [ "Docker Installation", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1698", [
          [ "Pull Official Image", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1699", null ],
          [ "Build Custom Image", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1700", null ]
        ] ]
      ] ],
      [ "Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1701", [
        [ "Configuration File Structure", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1702", null ],
        [ "Environment Variables", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1703", null ],
        [ "Secrets Management", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1704", [
          [ "Using HashiCorp Vault", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1705", null ],
          [ "Using Kubernetes Secrets", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1706", null ]
        ] ]
      ] ],
      [ "Deployment Scenarios", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1707", [
        [ "Single Server Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1708", null ],
        [ "High Availability Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1709", [
          [ "Architecture", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1710", null ],
          [ "HAProxy Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1711", null ]
        ] ],
        [ "Kubernetes Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1712", [
          [ "Namespace and ConfigMap", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1713", null ],
          [ "Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1714", null ],
          [ "Service and Ingress", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1715", null ],
          [ "Horizontal Pod Autoscaler", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1716", null ]
        ] ],
        [ "Docker Compose Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1717", null ]
      ] ],
      [ "Performance Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1718", [
        [ "System Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1719", [
          [ "Linux Kernel Parameters", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1720", null ],
          [ "ulimit Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1721", null ]
        ] ],
        [ "Application Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1722", [
          [ "Thread Pool Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1723", null ],
          [ "Memory Management", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1724", null ],
          [ "Network Optimization", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1725", null ]
        ] ],
        [ "Database Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1726", [
          [ "PostgreSQL Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1727", null ],
          [ "Connection Pool Tuning", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1728", null ]
        ] ]
      ] ],
      [ "Monitoring Setup", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1729", [
        [ "Prometheus Integration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1730", [
          [ "Prometheus Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1731", null ],
          [ "Metrics Exposed", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1732", null ]
        ] ],
        [ "Grafana Dashboard", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1733", null ],
        [ "Logging Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1734", [
          [ "Structured Logging", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1735", null ],
          [ "Log Aggregation (ELK Stack)", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1736", null ]
        ] ]
      ] ],
      [ "Scaling Strategies", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1737", [
        [ "Horizontal Scaling", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1738", [
          [ "Load Balancer Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1739", null ],
          [ "Auto-scaling Rules", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1740", null ]
        ] ],
        [ "Vertical Scaling", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1741", [
          [ "Resource Allocation", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1742", null ]
        ] ],
        [ "Database Scaling", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1743", [
          [ "Read Replicas", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1744", null ],
          [ "Sharding Strategy", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1745", null ]
        ] ]
      ] ],
      [ "Backup and Recovery", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1746", [
        [ "Backup Strategy", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1747", [
          [ "Application State Backup", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1748", null ],
          [ "Database Backup", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1749", null ]
        ] ],
        [ "Recovery Procedures", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1750", [
          [ "Service Recovery", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1751", null ],
          [ "Disaster Recovery Plan", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1752", null ]
        ] ]
      ] ],
      [ "Security Hardening", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1753", [
        [ "Network Security", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1754", [
          [ "Firewall Rules", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1755", null ],
          [ "SSL/TLS Configuration", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1756", null ]
        ] ],
        [ "Application Security", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1757", [
          [ "Authentication", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1758", null ],
          [ "Rate Limiting", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1759", null ]
        ] ],
        [ "Compliance", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1760", [
          [ "Audit Logging", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1761", null ],
          [ "Data Encryption", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1762", null ]
        ] ]
      ] ],
      [ "Troubleshooting Deployment", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1763", [
        [ "Common Issues", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1764", [
          [ "Service Won't Start", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1765", null ],
          [ "High Memory Usage", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1766", null ],
          [ "Performance Issues", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1767", null ]
        ] ],
        [ "Health Checks", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1768", [
          [ "Application Health", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1769", null ],
          [ "System Health", "df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1770", null ]
        ] ]
      ] ]
    ] ],
    [ "Design Patterns and Architectural Decisions", "d5/dca/md_docs_2DESIGN__PATTERNS.html", [
      [ "Overview", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1772", null ],
      [ "Creational Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1773", [
        [ "1. Singleton Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1774", null ],
        [ "2. Factory Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1775", null ],
        [ "3. Builder Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1776", null ],
        [ "4. Object Pool Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1777", null ]
      ] ],
      [ "Structural Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1778", [
        [ "5. Adapter Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1779", null ],
        [ "6. Decorator Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1780", null ],
        [ "7. Proxy Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1781", null ],
        [ "8. Composite Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1782", null ]
      ] ],
      [ "Behavioral Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1783", [
        [ "9. Observer Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1784", null ],
        [ "10. Strategy Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1785", null ],
        [ "11. Chain of Responsibility Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1786", null ],
        [ "12. Command Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1787", null ],
        [ "13. Template Method Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1788", null ]
      ] ],
      [ "Concurrency Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1789", [
        [ "14. Producer-Consumer Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1790", null ],
        [ "15. Thread Pool Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1791", null ]
      ] ],
      [ "Architectural Patterns", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1792", [
        [ "16. Microkernel Pattern", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1793", null ],
        [ "17. Event-Driven Architecture", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1794", null ]
      ] ],
      [ "Summary", "d5/dca/md_docs_2DESIGN__PATTERNS.html#autotoc_md1795", null ]
    ] ],
    [ "Developer Guide", "db/dac/md_docs_2DEVELOPER__GUIDE.html", [
      [ "Quick Start", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1797", [
        [ "Prerequisites", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1798", null ],
        [ "1. Clone and Setup", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1799", null ],
        [ "2. Build the Project", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1800", null ],
        [ "3. Your First Application", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1801", null ]
      ] ],
      [ "Development Setup", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1802", [
        [ "IDE Configuration", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1803", [
          [ "Visual Studio Code", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1804", null ],
          [ "CLion", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1805", null ],
          [ "Visual Studio", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1806", null ]
        ] ],
        [ "Development Dependencies", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1807", null ]
      ] ],
      [ "Project Structure", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1808", [
        [ "Directory Layout", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1809", null ],
        [ "Module Organization", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1810", null ]
      ] ],
      [ "Coding Standards", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1811", [
        [ "C++ Style Guide", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1812", [
          [ "Naming Conventions", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1813", null ],
          [ "File Organization", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1814", null ],
          [ "Best Practices", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1815", null ]
        ] ],
        [ "Documentation Standards", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1816", [
          [ "Code Documentation", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1817", null ],
          [ "Comment Guidelines", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1818", null ]
        ] ]
      ] ],
      [ "Testing Guidelines", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1819", [
        [ "Unit Testing", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1820", [
          [ "Test Structure", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1821", null ],
          [ "Test Coverage", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1822", null ]
        ] ],
        [ "Integration Testing", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1823", null ],
        [ "Performance Testing", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1824", null ]
      ] ],
      [ "Debugging", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1825", [
        [ "Using GDB", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1826", null ],
        [ "Using Valgrind", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1827", null ],
        [ "Using AddressSanitizer", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1828", null ],
        [ "Using ThreadSanitizer", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1829", null ]
      ] ],
      [ "Performance Profiling", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1830", [
        [ "Using perf", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1831", null ],
        [ "Using Instruments (macOS)", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1832", null ],
        [ "Using Intel VTune", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1833", null ]
      ] ],
      [ "Contributing Guidelines", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1834", [
        [ "Workflow", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1835", null ],
        [ "Commit Message Format", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1836", null ],
        [ "Code Review Checklist", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1837", null ]
      ] ],
      [ "Build System", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1838", [
        [ "CMake Configuration", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1839", null ],
        [ "Creating New Modules", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1840", null ]
      ] ],
      [ "Deployment", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1841", [
        [ "Docker Deployment", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1842", null ],
        [ "Kubernetes Deployment", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1843", null ]
      ] ],
      [ "Security Best Practices", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1844", [
        [ "Input Validation", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1845", null ],
        [ "Secure Communication", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1846", null ],
        [ "Rate Limiting", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1847", null ]
      ] ],
      [ "Troubleshooting Common Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1848", [
        [ "Build Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1849", null ],
        [ "Runtime Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1850", null ],
        [ "Performance Issues", "db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1851", null ]
      ] ]
    ] ],
    [ "Getting Started", "d2/d41/md_docs_2GETTING__STARTED.html", [
      [ "Table of Contents", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1853", null ],
      [ "Prerequisites", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1854", [
        [ "System Requirements", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1855", null ],
        [ "Development Dependencies", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1856", null ],
        [ "Runtime Dependencies", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1857", null ]
      ] ],
      [ "Installation", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1858", [
        [ "1. Clone the Repository", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1859", null ],
        [ "2. Platform-Specific Setup", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1860", [
          [ "Linux (Ubuntu/Debian)", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1861", null ],
          [ "macOS", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1862", null ],
          [ "Windows", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1863", null ]
        ] ],
        [ "3. Build the System", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1864", [
          [ "Quick Build (Recommended)", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1865", null ],
          [ "Custom Build Options", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1866", null ]
        ] ],
        [ "4. Verify Installation", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1867", null ]
      ] ],
      [ "Quick Start", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1868", [
        [ "1. Basic Message Bus Usage", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1869", null ],
        [ "2. Container-Based Data Handling", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1870", null ],
        [ "3. Network Client/Server", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1871", null ]
      ] ],
      [ "Basic Usage", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1872", [
        [ "Project Structure", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1873", null ],
        [ "Environment Configuration", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1874", null ],
        [ "Basic Configuration File", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1875", null ]
      ] ],
      [ "First Application", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1876", [
        [ "1. Chat Server (chat_server.cpp)", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1877", null ],
        [ "2. Build and Run", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1878", null ],
        [ "3. Test with Sample Client", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1879", null ]
      ] ],
      [ "Next Steps", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1880", [
        [ "1. Explore Sample Applications", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1881", null ],
        [ "2. Advanced Features", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1882", null ],
        [ "3. Development", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1883", null ],
        [ "4. Community", "d2/d41/md_docs_2GETTING__STARTED.html#autotoc_md1884", null ]
      ] ]
    ] ],
    [ "Performance Guide", "df/d94/md_docs_2performance.html", [
      [ "Table of Contents", "df/d94/md_docs_2performance.html#autotoc_md1887", null ],
      [ "Performance Overview", "df/d94/md_docs_2performance.html#autotoc_md1888", [
        [ "Design Goals", "df/d94/md_docs_2performance.html#autotoc_md1889", null ],
        [ "Key Performance Features", "df/d94/md_docs_2performance.html#autotoc_md1890", null ]
      ] ],
      [ "Benchmark Results", "df/d94/md_docs_2performance.html#autotoc_md1891", [
        [ "Test Environment", "df/d94/md_docs_2performance.html#autotoc_md1892", null ],
        [ "Overall System Performance", "df/d94/md_docs_2performance.html#autotoc_md1893", null ],
        [ "Latency Measurements", "df/d94/md_docs_2performance.html#autotoc_md1894", null ],
        [ "Memory Performance", "df/d94/md_docs_2performance.html#autotoc_md1895", null ]
      ] ],
      [ "Component Performance", "df/d94/md_docs_2performance.html#autotoc_md1896", [
        [ "Thread System Performance", "df/d94/md_docs_2performance.html#autotoc_md1897", [
          [ "Lock-free vs Mutex Comparison", "df/d94/md_docs_2performance.html#autotoc_md1898", null ],
          [ "Scaling Characteristics", "df/d94/md_docs_2performance.html#autotoc_md1899", null ]
        ] ],
        [ "Container System Performance", "df/d94/md_docs_2performance.html#autotoc_md1900", [
          [ "Serialization Performance", "df/d94/md_docs_2performance.html#autotoc_md1901", null ],
          [ "SIMD Optimization Impact", "df/d94/md_docs_2performance.html#autotoc_md1902", null ]
        ] ],
        [ "Network System Performance", "df/d94/md_docs_2performance.html#autotoc_md1903", [
          [ "Connection Scaling", "df/d94/md_docs_2performance.html#autotoc_md1904", null ],
          [ "Protocol Overhead", "df/d94/md_docs_2performance.html#autotoc_md1905", null ]
        ] ],
        [ "Database System Performance", "df/d94/md_docs_2performance.html#autotoc_md1906", [
          [ "Query Performance", "df/d94/md_docs_2performance.html#autotoc_md1907", null ],
          [ "Connection Pool Impact", "df/d94/md_docs_2performance.html#autotoc_md1908", null ]
        ] ]
      ] ],
      [ "Optimization Techniques", "df/d94/md_docs_2performance.html#autotoc_md1909", [
        [ "1. Memory Optimization", "df/d94/md_docs_2performance.html#autotoc_md1910", [
          [ "Object Pooling", "df/d94/md_docs_2performance.html#autotoc_md1911", null ],
          [ "Custom Allocators", "df/d94/md_docs_2performance.html#autotoc_md1912", null ]
        ] ],
        [ "2. CPU Optimization", "df/d94/md_docs_2performance.html#autotoc_md1913", [
          [ "SIMD Utilization", "df/d94/md_docs_2performance.html#autotoc_md1914", null ],
          [ "Cache Optimization", "df/d94/md_docs_2performance.html#autotoc_md1915", null ]
        ] ],
        [ "3. Network Optimization", "df/d94/md_docs_2performance.html#autotoc_md1916", [
          [ "Batching and Pipelining", "df/d94/md_docs_2performance.html#autotoc_md1917", null ]
        ] ]
      ] ],
      [ "Performance Monitoring", "df/d94/md_docs_2performance.html#autotoc_md1918", [
        [ "1. Built-in Metrics", "df/d94/md_docs_2performance.html#autotoc_md1919", null ],
        [ "2. Performance Profiling", "df/d94/md_docs_2performance.html#autotoc_md1920", null ]
      ] ],
      [ "Tuning Guidelines", "df/d94/md_docs_2performance.html#autotoc_md1921", [
        [ "1. Thread Configuration", "df/d94/md_docs_2performance.html#autotoc_md1922", null ],
        [ "2. Memory Configuration", "df/d94/md_docs_2performance.html#autotoc_md1923", null ],
        [ "3. Network Configuration", "df/d94/md_docs_2performance.html#autotoc_md1924", null ]
      ] ],
      [ "Troubleshooting Performance Issues", "df/d94/md_docs_2performance.html#autotoc_md1925", [
        [ "1. Common Performance Problems", "df/d94/md_docs_2performance.html#autotoc_md1926", [
          [ "High CPU Usage", "df/d94/md_docs_2performance.html#autotoc_md1927", null ],
          [ "Memory Leaks", "df/d94/md_docs_2performance.html#autotoc_md1928", null ],
          [ "Network Bottlenecks", "df/d94/md_docs_2performance.html#autotoc_md1929", null ]
        ] ],
        [ "2. Performance Monitoring Dashboard", "df/d94/md_docs_2performance.html#autotoc_md1930", null ]
      ] ]
    ] ],
    [ "Troubleshooting Guide", "d4/d2f/md_docs_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1947", null ],
      [ "FAQ", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1948", [
        [ "General Questions", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1949", null ],
        [ "Performance Questions", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1950", null ],
        [ "Configuration Questions", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1951", null ]
      ] ],
      [ "Debug Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1952", [
        [ "Enable Debug Logging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1953", [
          [ "Runtime Configuration", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1954", null ],
          [ "Environment Variables", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1955", null ],
          [ "Programmatic", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1956", null ]
        ] ],
        [ "Using GDB for Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1957", [
          [ "Attach to Running Process", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1958", null ],
          [ "Debug Core Dumps", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1959", null ]
        ] ],
        [ "Memory Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1960", [
          [ "Using Valgrind", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1961", null ],
          [ "Using AddressSanitizer", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1962", null ],
          [ "Using HeapTrack", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1963", null ]
        ] ],
        [ "Network Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1964", [
          [ "TCP Dump", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1965", null ],
          [ "Network Statistics", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1966", null ],
          [ "Test Connectivity", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1967", null ]
        ] ],
        [ "Thread Debugging", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1968", [
          [ "Thread Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1969", null ],
          [ "Detect Deadlocks", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1970", null ],
          [ "ThreadSanitizer", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1971", null ]
        ] ],
        [ "Tracing and Profiling", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1972", [
          [ "System Tracing with strace", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1973", null ],
          [ "Performance Profiling with perf", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1974", null ],
          [ "Application Tracing", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1975", null ]
        ] ]
      ] ],
      [ "Performance Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1976", [
        [ "CPU Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1977", [
          [ "Identify CPU Bottlenecks", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1978", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1979", null ]
        ] ],
        [ "Memory Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1980", [
          [ "Memory Usage Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1981", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1982", null ]
        ] ],
        [ "I/O Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1983", [
          [ "Disk I/O Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1984", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1985", null ]
        ] ],
        [ "Network Optimization", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1986", [
          [ "Network Analysis", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1987", null ],
          [ "Optimization Techniques", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1988", null ]
        ] ]
      ] ],
      [ "Known Issues", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1989", [
        [ "Issue 1: High CPU Usage with Small Messages", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1990", null ],
        [ "Issue 2: Memory Leak in Long-Running Connections", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1991", null ],
        [ "Issue 3: Database Connection Pool Exhaustion", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1992", null ],
        [ "Issue 4: Deadlock in Message Processing", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1993", null ],
        [ "Issue 5: Performance Degradation with Many Topics", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1994", null ]
      ] ],
      [ "Error Messages", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1995", [
        [ "Connection Errors", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1996", [
          [ "\"Connection refused\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1997", null ],
          [ "\"Connection timeout\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1998", null ]
        ] ],
        [ "Database Errors", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1999", [
          [ "\"Database connection failed\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2000", null ],
          [ "\"Deadlock detected\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2001", null ]
        ] ],
        [ "Memory Errors", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2002", [
          [ "\"Out of memory\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2003", null ],
          [ "\"Segmentation fault\"", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2004", null ]
        ] ]
      ] ],
      [ "Common Problems", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2005", [
        [ "Problem: Service Won't Start", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2006", null ],
        [ "Problem: Slow Message Processing", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2007", null ],
        [ "Problem: Connection Drops", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2008", null ],
        [ "Problem: High Memory Usage", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2009", null ],
        [ "Problem: Database Bottleneck", "d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md2010", null ]
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
"d1/d69/quick__mpmc__test__google_8cpp.html#ae408050e4b1b4e2c95fce9f897d9878b",
"d1/dd3/structkcenon_1_1messaging_1_1management_1_1management__command__processor_1_1command__response.html#a02949e7eb20596181c871ed1407a67e3",
"d2/d11/test__std__concepts_8cpp.html#ae66f6b31b5ad750f1fe042a706a4e3d4",
"d2/d93/classthread__module_1_1result.html#a138e1fd19968c104ca4cb63302f4ad1c",
"d2/d9e/classtyped__thread__pool__module_1_1adaptive__typed__job__queue__t.html#a58dbac39f0cc096f51ce637d8305833b",
"d2/dca/namespacethread__module_1_1jobs.html#aee9a7b3c53d29c84ac5dbb96077f2e54a53cced8d281a1a0ace3cb6594daaa4f7",
"d3/d07/classkcenon_1_1messaging_1_1persistence_1_1file__message__storage.html#a6a78a8bc40fb1e17575e18d8b58fcfda",
"d3/d35/classtyped__thread__pool__module_1_1typed__lockfree__job__queue__t.html#adc1cd8fdbd2fd1b527c3a444a3f85540",
"d3/d91/structkcenon_1_1messaging_1_1config_1_1messaging__config.html#a55653f7b1fc29e6988a12cd49927945e",
"d3/dea/classtyped__thread__pool__module_1_1callback__typed__job__t.html#a03e4d2b239b48e210e67fc322cccbfd0",
"d4/d2f/md_docs_2TROUBLESHOOTING.html#autotoc_md1973",
"d4/d7d/sources_2thread__base_2sync_2error__handling_8h.html#a215ffba45222b5527566469d33e4447caa09332d99869fd7d67c72c1e9187f8e1",
"d4/d9e/md_libraries_2thread__system_2docs_2performance.html#autotoc_md1254",
"d4/df7/structthread__module_1_1lockfree__job__queue_1_1atomic__statistics.html#a1af1969730e0c853c4b67970167f8f6d",
"d5/d31/classthread__module_1_1hazard__pointer__manager_1_1hazard__pointer.html#a686737f3638769a82bb5c9f13fd577aa",
"d5/da5/trend__analyzer_8h_source.html",
"d5/de9/thread__pool__sample_8cpp.html#aac3d4a1520fb7a63f35125d262273453",
"d6/d06/classSystemIntegratorPerformanceTest.html#a337ed6096606fc2c4668deff410ae967",
"d6/d5e/classmonitoring__module_1_1monitoring_1_1impl.html#a77193f52be6988c8fae7c0e717df6219",
"d6/d9d/structkcenon_1_1messaging_1_1cluster_1_1cluster__node.html#a2438aa8d72132608dbaf2ed5629f260d",
"d6/dff/log__collector__test_8cpp.html#ae4aad73419c7267bc24e088d44895e25",
"d7/d36/sources_2typed__thread__pool_2core_2job__types_8h.html#a2c8ae69464a38fdf60ba05576e0c85ad",
"d7/d9b/classkcenon_1_1messaging_1_1core_1_1message__queue.html#a7d8f5f50c3b88aa204d3640863485588",
"d7/dfd/classmonitoring__interface_1_1scoped__timer.html#a0feb5f6c1cee5e9c7ca9aef288c9e8a0",
"d8/d4a/classthread__pool__module_1_1pool__promise_3_01void_01_4.html#af8f6e5ee5419e6ac2737e6d6eebce91e",
"d8/d4f/classutility__module_1_1span.html#aaf064a0e467811161c0480be03b3fa40",
"d8/d7c/modular__structure_2core_2src_2typed__thread__pool_2detail_2forward__declarations_8h.html",
"d8/dad/classthread__module_1_1node__pool.html#a5eb347b7451fc73b86deb90ec1a2b804",
"d9/d22/ring__buffer__test_8cpp.html#a00f7cf7e0a118da418e38a390949587d",
"d9/d62/structmonitoring__module_1_1performance__optimizer_1_1optimization__stats.html#a27d9e6d24c34b704d1bba8e28c80b477",
"d9/d93/sources_2thread__base_2sync_2sync__primitives_8h_source.html",
"d9/dad/classcontainer__module_1_1value__container.html#a611e8c57bc25142f572a7f82fd1829fa",
"da/d1c/classcontainer__module_1_1container__value.html#a53985d014c337b8534232903f2408195",
"da/d62/classthread__module_1_1sync_1_1atomic__flag__wrapper.html#a044a9a59ad66c5336f503c5c41f35598",
"da/da0/sources_2typed__thread__pool_2core_2typed__job__interface_8h_source.html",
"da/df8/classthread__module_1_1sync_1_1condition__variable__wrapper.html#ac0beba8e3bed8e4bfe73c4cdf5f86db0",
"db/d54/classlogger__module_1_1log__sanitizer.html#af8ce7353abe1280767d4980f90edb0c8",
"db/dac/md_docs_2DEVELOPER__GUIDE.html#autotoc_md1817",
"db/de8/classtyped__thread__pool__module_1_1typed__job__queue__t.html#a7cf695694db1a864899f15df036e8531",
"dc/d0a/simple__mpmc__test_8cpp.html#a8341fc575ac948f3300c80e4f48744e4",
"dc/d6a/classmonitoring__interface_1_1null__monitoring.html",
"dc/d9f/structlogger__module_1_1performance__metrics.html#a09f00ae5574d9bc214f565a516637e89",
"dc/de7/classkcenon_1_1messaging_1_1services_1_1discovery_1_1discovery__service.html#aaf128101972eb8b7f6ef744af5b255c7",
"dd/d2d/classkcenon_1_1messaging_1_1management_1_1system__management__commands.html#a6aeeea50751cfa4c35d21d88b787ea1c",
"dd/da2/structlogger__module_1_1log__collector_1_1performance__stats.html#a68ff507b69c7739c7958bc8145f09d1b",
"de/d07/classkcenon_1_1messaging_1_1routing_1_1advanced__router.html#ad673fe9c84b69b96c73aa9dca4601d9d",
"de/d48/namespacethread__module.html#a215ffba45222b5527566469d33e4447ca260ca9dd8a4577fc00b7bd5810298076",
"de/d48/namespacethread__module.html#a5a2719d6d43f27e1e8e913fbfa5c9288acb5e100e5a9a3e7f6d1fd97512215282",
"de/d5e/classthread__module_1_1result_3_01void_01_4.html#ad3b81812f8c8b75320b4daabc08990a4",
"de/dca/structmonitoring__module_1_1critical__metrics__snapshot.html",
"df/d23/md_docs_2DEPLOYMENT__GUIDE.html#autotoc_md1704",
"df/d50/classthread__module_1_1detail_1_1thread__impl.html#a12d6f8442378baf8c1a7be7378293528",
"df/d77/modular__structure_2core_2src_2typed__thread__pool_2scheduling_2adaptive__typed__job__queue_8cpp.html",
"df/df7/classcontainer__module_1_1bytes__value.html#adc1051678ad6a8baeac55c6a552ebfe9",
"functions_func_q.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';