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
    [ "Integration Guide", "index.html", "index" ],
    [ "Design Patterns and Architectural Decisions", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html", [
      [ "Overview", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md1", null ],
      [ "Creational Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md2", [
        [ "1. Singleton Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md3", null ],
        [ "2. Factory Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md4", null ],
        [ "3. Builder Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md5", null ],
        [ "4. Object Pool Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md6", null ]
      ] ],
      [ "Structural Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md7", [
        [ "5. Adapter Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md8", null ],
        [ "6. Decorator Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md9", null ],
        [ "7. Proxy Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md10", null ],
        [ "8. Composite Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md11", null ]
      ] ],
      [ "Behavioral Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md12", [
        [ "9. Observer Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md13", null ],
        [ "10. Strategy Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md14", null ],
        [ "11. Chain of Responsibility Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md15", null ],
        [ "12. Command Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md16", null ],
        [ "13. Template Method Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md17", null ]
      ] ],
      [ "Concurrency Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md18", [
        [ "14. Producer-Consumer Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md19", null ],
        [ "15. Thread Pool Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md20", null ]
      ] ],
      [ "Architectural Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md21", [
        [ "16. Microkernel Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md22", null ],
        [ "17. Event-Driven Architecture", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md23", null ]
      ] ],
      [ "Summary", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md24", null ]
    ] ],
    [ "Messaging Patterns API Reference", "df/dde/md_docs_2advanced_2PATTERNS__API.html", [
      [ "Table of Contents", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md26", null ],
      [ "Pub/Sub Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md28", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md29", null ],
        [ "Class: <tt>publisher</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md30", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md31", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md32", [
            [ "<tt>publish</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md33", null ],
            [ "<tt>get_default_topic</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md34", null ],
            [ "<tt>set_default_topic</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md35", null ],
            [ "<tt>is_ready</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md36", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md37", null ]
        ] ],
        [ "Class: <tt>subscriber</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md38", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md39", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md40", [
            [ "<tt>subscribe</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md41", null ],
            [ "<tt>unsubscribe_all</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md42", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md43", null ]
        ] ]
      ] ],
      [ "Request-Reply Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md45", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md46", null ],
        [ "Class: <tt>request_reply_handler</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md47", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md48", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md49", [
            [ "<tt>request</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md50", null ],
            [ "<tt>register_handler</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md51", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md52", null ]
        ] ],
        [ "Class: <tt>request_client</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md53", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md54", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md55", [
            [ "<tt>request</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md56", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md57", null ]
        ] ],
        [ "Class: <tt>request_server</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md58", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md59", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md60", [
            [ "<tt>register_handler</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md61", null ],
            [ "<tt>start</tt> / <tt>stop</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md62", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md63", null ]
        ] ]
      ] ],
      [ "Event Streaming Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md65", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md66", null ],
        [ "Struct: <tt>event_stream_config</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md67", null ],
        [ "Class: <tt>event_stream</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md68", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md69", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md70", [
            [ "<tt>publish_event</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md71", null ],
            [ "<tt>subscribe</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md72", null ],
            [ "<tt>unsubscribe</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md73", null ],
            [ "<tt>replay</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md74", null ],
            [ "<tt>get_events</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md75", null ],
            [ "<tt>event_count</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md76", null ],
            [ "<tt>clear_buffer</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md77", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md78", null ]
        ] ],
        [ "Class: <tt>event_batch_processor</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md79", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md80", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md81", [
            [ "<tt>start</tt> / <tt>stop</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md82", null ],
            [ "<tt>flush</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md83", null ],
            [ "<tt>is_running</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md84", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md85", null ]
        ] ]
      ] ],
      [ "Message Pipeline Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md87", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md88", null ],
        [ "Type Aliases", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md89", null ],
        [ "Class: <tt>message_pipeline</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md90", [
          [ "Struct: <tt>pipeline_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md91", null ],
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md92", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md93", [
            [ "<tt>add_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md94", null ],
            [ "<tt>remove_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md95", null ],
            [ "<tt>start</tt> / <tt>stop</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md96", null ],
            [ "<tt>process</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md97", null ],
            [ "<tt>stage_count</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md98", null ],
            [ "<tt>get_stage_names</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md99", null ],
            [ "<tt>get_statistics</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md100", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md101", null ]
        ] ],
        [ "Class: <tt>pipeline_builder</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md102", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md103", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md104", [
            [ "<tt>from</tt> / <tt>to</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md105", null ],
            [ "<tt>add_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md106", null ],
            [ "<tt>add_filter</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md107", null ],
            [ "<tt>add_transformer</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md108", null ],
            [ "<tt>build</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md109", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md110", null ]
        ] ],
        [ "Namespace: <tt>pipeline_stages</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md111", [
          [ "<tt>create_logging_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md112", null ],
          [ "<tt>create_validation_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md113", null ],
          [ "<tt>create_enrichment_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md114", null ],
          [ "<tt>create_retry_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md115", null ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md116", null ]
        ] ]
      ] ],
      [ "Best Practices", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md118", [
        [ "Pub/Sub Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md119", null ],
        [ "Request-Reply Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md120", null ],
        [ "Event Streaming Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md121", null ],
        [ "Message Pipeline Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md122", null ],
        [ "General Guidelines", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md123", null ]
      ] ],
      [ "Error Handling", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md125", null ],
      [ "Thread Safety", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md127", null ],
      [ "Performance Considerations", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md129", null ]
    ] ],
    [ "System Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html", [
      [ "Overview", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md131", null ],
      [ "Architectural Principles", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md132", null ],
      [ "System Layers", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md133", null ],
      [ "Component Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md134", [
        [ "1. Container System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md135", null ],
        [ "2. Network System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md136", null ],
        [ "3. Database System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md137", null ],
        [ "4. Thread System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md138", null ]
      ] ],
      [ "Data Flow Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md139", [
        [ "Message Processing Pipeline", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md140", null ],
        [ "Request-Response Flow", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md141", null ],
        [ "Distributed Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md142", null ]
      ] ],
      [ "Scalability Patterns", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md143", [
        [ "Horizontal Scaling", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md144", null ],
        [ "Vertical Scaling", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md145", null ]
      ] ],
      [ "Integration Points", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md146", [
        [ "External Systems", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md147", null ],
        [ "Internal Communication", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md148", null ]
      ] ],
      [ "Performance Optimization", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md149", [
        [ "Memory Management", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md150", null ],
        [ "Concurrency", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md151", null ],
        [ "Network", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md152", null ]
      ] ],
      [ "Fault Tolerance", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md153", [
        [ "Error Recovery", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md154", null ],
        [ "High Availability", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md155", null ]
      ] ],
      [ "Security Considerations", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md156", [
        [ "Transport Security", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md157", null ],
        [ "Application Security", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md158", null ]
      ] ],
      [ "Monitoring and Observability", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md159", [
        [ "Metrics Collection", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md160", null ],
        [ "Distributed Tracing", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md161", null ],
        [ "Logging", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md162", null ]
      ] ],
      [ "Configuration Management", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md163", [
        [ "Static Configuration", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md164", null ],
        [ "Dynamic Configuration", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md165", null ]
      ] ],
      [ "Deployment Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md166", [
        [ "Container Deployment", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md167", null ],
        [ "Kubernetes Integration", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md168", null ]
      ] ],
      [ "Future Considerations", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md169", [
        [ "Planned Enhancements", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md170", null ],
        [ "Technology Evaluation", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md171", null ]
      ] ]
    ] ],
    [ "Windows CI Performance Optimization", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html", [
      [ "Executive Summary", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md173", null ],
      [ "Problem Analysis", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md174", [
        [ "Initial State (PR #31 - Before Fix)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md175", null ],
        [ "Target State (PR #30 - Success)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md176", null ]
      ] ],
      [ "Implemented Solutions", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md177", [
        [ "1. Test-Level Timeouts ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md178", null ],
        [ "2. CTest Parallel Execution ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md179", null ],
        [ "3. Build Parallelization ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md180", null ]
      ] ],
      [ "Performance Comparison", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md181", null ],
      [ "Additional Recommendations", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md182", [
        [ "Short-Term (Quick Wins)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md183", [
          [ "1. Use Release Build for Tests", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md184", null ],
          [ "2. Increase vcpkg Cache Efficiency", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md185", null ],
          [ "3. Reduce FetchContent Downloads", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md186", null ]
        ] ],
        [ "Medium-Term (Architectural)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md187", [
          [ "1. Split Test Execution", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md188", null ],
          [ "2. Implement Test Sharding", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md189", null ],
          [ "3. Conditional Integration Tests", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md190", null ]
        ] ],
        [ "Long-Term (Infrastructure)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md191", [
          [ "1. Use Self-Hosted Runners", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md192", null ],
          [ "2. Implement Build Caching Service", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md193", null ],
          [ "3. Prebuilt Dependency Binaries", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md194", null ]
        ] ]
      ] ],
      [ "Implementation Priority", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md195", [
        [ "Priority 1 (Implemented) ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md196", null ],
        [ "Priority 2 (Recommended - Next Sprint)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md197", null ],
        [ "Priority 3 (Future Consideration)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md198", null ],
        [ "Priority 4 (Long-Term Planning)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md199", null ]
      ] ],
      [ "Monitoring", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md200", [
        [ "Key Metrics to Track", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md201", null ],
        [ "Alerting Thresholds", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md202", null ]
      ] ],
      [ "Conclusion", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md203", null ],
      [ "References", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md204", null ]
    ] ],
    [ "API Reference", "db/d91/md_docs_2API__REFERENCE.html", [
      [ "Table of Contents", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md206", null ],
      [ "Container System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md208", [
        [ "Namespace: <tt>container_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md209", null ],
        [ "Class: <tt>value_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md210", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md211", null ],
          [ "Core Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md212", [
            [ "Setting Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md213", null ],
            [ "Adding Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md214", null ],
            [ "Retrieving Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md215", null ],
            [ "Serialization", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md216", null ]
          ] ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md217", null ]
        ] ],
        [ "Class: <tt>variant</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md218", [
          [ "Supported Types", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md219", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md220", null ]
        ] ]
      ] ],
      [ "Network System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md222", [
        [ "Namespace: <tt>network_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md223", null ],
        [ "Class: <tt>messaging_server</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md224", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md225", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md226", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md227", null ]
        ] ],
        [ "Class: <tt>messaging_client</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md228", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md229", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md230", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md231", null ]
        ] ]
      ] ],
      [ "Database System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md233", [
        [ "Namespace: <tt>database</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md234", null ],
        [ "Class: <tt>database_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md235", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md236", null ],
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md237", null ],
          [ "Connection Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md238", null ],
          [ "Query Execution", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md239", null ],
          [ "Transaction Support", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md240", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md241", null ]
        ] ]
      ] ],
      [ "Message Bus API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md243", [
        [ "Namespace: <tt>kcenon::messaging::core</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md244", null ],
        [ "Class: <tt>message_bus</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md245", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md246", null ],
          [ "Configuration Structure", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md247", null ],
          [ "Lifecycle Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md248", null ],
          [ "Publishing", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md249", null ],
          [ "Subscription", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md250", null ],
          [ "Request-Response", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md251", null ],
          [ "Monitoring", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md252", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md253", null ]
        ] ]
      ] ],
      [ "Service Container API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md255", [
        [ "Namespace: <tt>kcenon::messaging::services</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md256", null ],
        [ "Class: <tt>service_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md257", [
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md258", null ],
          [ "Service Lifetimes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md259", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md260", null ]
        ] ]
      ] ],
      [ "Thread System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md262", [
        [ "Namespace: <tt>thread_system</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md263", null ],
        [ "Class: <tt>thread_pool</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md264", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md265", null ],
          [ "Job Submission", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md266", null ],
          [ "Pool Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md267", null ],
          [ "Priority Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md268", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md269", null ]
        ] ],
        [ "Class: <tt>lock_free_queue</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md270", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md271", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md272", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md273", null ]
        ] ]
      ] ],
      [ "Logger System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md275", [
        [ "Namespace: <tt>logger</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md276", null ],
        [ "Class: <tt>logger_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md277", [
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md278", null ],
          [ "Log Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md279", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md280", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md281", null ]
        ] ]
      ] ],
      [ "Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md283", [
        [ "System Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md284", null ],
        [ "Error Handling", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md285", null ],
        [ "Error Recovery Strategies", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md286", null ]
      ] ],
      [ "Configuration Reference", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md287", [
        [ "System Configuration File Format", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md288", null ],
        [ "Environment Variables", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md289", null ]
      ] ]
    ] ],
    [ "Messaging System API 레퍼런스", "d4/d54/md_docs_2API__REFERENCE__KO.html", [
      [ "개요", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md292", null ],
      [ "목차", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md294", null ],
      [ "핵심 클래스", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md296", [
        [ "message_bus", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md297", null ]
      ] ],
      [ "메시지 버스", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md299", [
        [ "사용 예제", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md300", null ]
      ] ],
      [ "토픽 라우터", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md302", [
        [ "와일드카드 패턴", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md303", null ]
      ] ],
      [ "메시징 패턴", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md305", [
        [ "Pub/Sub", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md306", null ],
        [ "Request/Reply", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md307", null ]
      ] ],
      [ "직렬화", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md309", [
        [ "메시지 구조", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md310", null ]
      ] ],
      [ "관련 문서", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md312", null ]
    ] ],
    [ "Architecture", "d2/d64/md_docs_2architecture.html", [
      [ "Overview", "d2/d64/md_docs_2architecture.html#autotoc_md314", null ],
      [ "Related Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md315", [
        [ "Component-Specific Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md316", null ],
        [ "Module Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md317", null ],
        [ "Integration Guides", "d2/d64/md_docs_2architecture.html#autotoc_md318", null ]
      ] ],
      [ "Architectural Principles", "d2/d64/md_docs_2architecture.html#autotoc_md319", null ],
      [ "System Layers", "d2/d64/md_docs_2architecture.html#autotoc_md320", null ],
      [ "Component Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md321", [
        [ "1. Container System", "d2/d64/md_docs_2architecture.html#autotoc_md322", null ],
        [ "2. Network System", "d2/d64/md_docs_2architecture.html#autotoc_md323", null ],
        [ "3. Database System", "d2/d64/md_docs_2architecture.html#autotoc_md324", null ],
        [ "4. Thread System", "d2/d64/md_docs_2architecture.html#autotoc_md325", null ]
      ] ],
      [ "Data Flow Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md326", [
        [ "Message Processing Pipeline", "d2/d64/md_docs_2architecture.html#autotoc_md327", null ],
        [ "Request-Response Flow", "d2/d64/md_docs_2architecture.html#autotoc_md328", null ],
        [ "Distributed Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md329", null ]
      ] ],
      [ "Scalability Patterns", "d2/d64/md_docs_2architecture.html#autotoc_md330", [
        [ "Horizontal Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md331", null ],
        [ "Vertical Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md332", null ]
      ] ],
      [ "Integration Points", "d2/d64/md_docs_2architecture.html#autotoc_md333", [
        [ "External Systems", "d2/d64/md_docs_2architecture.html#autotoc_md334", null ],
        [ "Internal Communication", "d2/d64/md_docs_2architecture.html#autotoc_md335", null ]
      ] ],
      [ "Performance Optimization", "d2/d64/md_docs_2architecture.html#autotoc_md336", [
        [ "Memory Management", "d2/d64/md_docs_2architecture.html#autotoc_md337", null ],
        [ "Concurrency", "d2/d64/md_docs_2architecture.html#autotoc_md338", null ],
        [ "Network", "d2/d64/md_docs_2architecture.html#autotoc_md339", null ]
      ] ],
      [ "Fault Tolerance", "d2/d64/md_docs_2architecture.html#autotoc_md340", [
        [ "Error Recovery", "d2/d64/md_docs_2architecture.html#autotoc_md341", null ],
        [ "High Availability", "d2/d64/md_docs_2architecture.html#autotoc_md342", null ]
      ] ],
      [ "Security Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md343", [
        [ "Transport Security", "d2/d64/md_docs_2architecture.html#autotoc_md344", null ],
        [ "Application Security", "d2/d64/md_docs_2architecture.html#autotoc_md345", null ]
      ] ],
      [ "Monitoring and Observability", "d2/d64/md_docs_2architecture.html#autotoc_md346", [
        [ "Metrics Collection", "d2/d64/md_docs_2architecture.html#autotoc_md347", null ],
        [ "Distributed Tracing", "d2/d64/md_docs_2architecture.html#autotoc_md348", null ],
        [ "Logging", "d2/d64/md_docs_2architecture.html#autotoc_md349", null ]
      ] ],
      [ "Configuration Management", "d2/d64/md_docs_2architecture.html#autotoc_md350", [
        [ "Static Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md351", null ],
        [ "Dynamic Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md352", null ]
      ] ],
      [ "Deployment Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md353", [
        [ "Container Deployment", "d2/d64/md_docs_2architecture.html#autotoc_md354", null ],
        [ "Kubernetes Integration", "d2/d64/md_docs_2architecture.html#autotoc_md355", null ]
      ] ],
      [ "Future Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md356", [
        [ "Planned Enhancements", "d2/d64/md_docs_2architecture.html#autotoc_md357", null ],
        [ "Technology Evaluation", "d2/d64/md_docs_2architecture.html#autotoc_md358", null ]
      ] ]
    ] ],
    [ "Messaging System 아키텍처", "de/de3/md_docs_2ARCHITECTURE__KO.html", [
      [ "개요", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md361", null ],
      [ "목차", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md363", null ],
      [ "시스템 개요", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md365", null ],
      [ "컴포넌트 아키텍처", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md367", [
        [ "핵심 컴포넌트", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md368", null ],
        [ "메시징 패턴", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md369", null ]
      ] ],
      [ "의존성 구조", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md371", [
        [ "버전 매트릭스", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md372", null ]
      ] ],
      [ "데이터 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md374", [
        [ "메시지 발행 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md375", null ],
        [ "Request/Reply 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md376", null ]
      ] ],
      [ "설계 원칙", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md378", [
        [ "1. 느슨한 결합", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md379", null ],
        [ "2. 비동기 우선", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md380", null ],
        [ "3. 스레드 안전", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md381", null ],
        [ "4. 확장성", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md382", null ]
      ] ],
      [ "관련 문서", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md384", null ]
    ] ],
    [ "Messaging System Performance Benchmarks", "de/d93/md_docs_2BENCHMARKS.html", [
      [ "Executive Summary", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md387", null ],
      [ "Table of Contents", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md389", null ],
      [ "Core Performance Metrics", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md391", [
        [ "Reference Performance Metrics", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md392", null ]
      ] ],
      [ "Message Creation Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md394", [
        [ "Message Builder Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md395", null ]
      ] ],
      [ "Queue Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md397", [
        [ "Priority Queue Operations", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md398", null ],
        [ "Queue Contention Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md399", null ]
      ] ],
      [ "Topic Router Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md401", [
        [ "Pattern Matching Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md402", null ],
        [ "Subscription Count Impact", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md403", null ]
      ] ],
      [ "Pub/Sub Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md405", [
        [ "End-to-End Throughput", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md406", null ],
        [ "Backend Impact", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md407", null ]
      ] ],
      [ "Request/Reply Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md409", [
        [ "Synchronous RPC Latency", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md410", null ],
        [ "Concurrent Request Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md411", null ]
      ] ],
      [ "Memory Usage", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md413", [
        [ "Per-Component Memory Overhead", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md414", null ],
        [ "Memory Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md415", null ]
      ] ],
      [ "Latency Analysis", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md417", [
        [ "Latency Distribution (Pub/Sub)", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md418", null ],
        [ "Latency Breakdown", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md419", null ]
      ] ],
      [ "Scalability", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md421", [
        [ "Thread Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md422", null ],
        [ "Publisher/Subscriber Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md423", null ]
      ] ],
      [ "Optimization Insights", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md425", [
        [ "Performance Tuning Recommendations", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md426", null ],
        [ "Common Bottlenecks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md427", null ]
      ] ],
      [ "Running Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md429", [
        [ "Prerequisites", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md430", null ],
        [ "Available Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md431", null ],
        [ "Benchmark Output Format", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md432", null ]
      ] ],
      [ "Continuous Performance Monitoring", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md434", null ],
      [ "Comparison with Other Systems", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md436", [
        [ "Feature Comparison", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md437", null ]
      ] ],
      [ "Conclusion", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md439", null ]
    ] ],
    [ "Messaging System 성능 벤치마크", "d5/ddb/md_docs_2BENCHMARKS__KO.html", [
      [ "요약", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md443", null ],
      [ "목차", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md445", null ],
      [ "핵심 성능 메트릭", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md447", [
        [ "요약 테이블", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md448", null ]
      ] ],
      [ "메시지 버스 벤치마크", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md450", [
        [ "처리량", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md451", null ],
        [ "토픽 라우팅 성능", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md452", null ]
      ] ],
      [ "패턴별 성능", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md454", [
        [ "Pub/Sub", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md455", null ],
        [ "Request/Reply", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md456", null ]
      ] ],
      [ "확장성", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md458", [
        [ "수평 확장", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md459", null ]
      ] ],
      [ "벤치마크 방법론", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md461", null ],
      [ "관련 문서", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md463", null ]
    ] ],
    [ "Messaging System Changelog", "da/dee/md_docs_2CHANGELOG.html", [
      [ "[Unreleased]", "da/dee/md_docs_2CHANGELOG.html#autotoc_md465", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md466", null ],
        [ "Fixed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md467", null ],
        [ "Changed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md468", null ],
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md469", null ]
      ] ],
      [ "[1.0.0] - 2025-11-17", "da/dee/md_docs_2CHANGELOG.html#autotoc_md471", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md472", null ],
        [ "Dependencies", "da/dee/md_docs_2CHANGELOG.html#autotoc_md473", null ]
      ] ],
      [ "[0.9.0] - 2025-10-20", "da/dee/md_docs_2CHANGELOG.html#autotoc_md475", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md476", null ],
        [ "Changed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md477", null ]
      ] ]
    ] ],
    [ "Messaging System 변경 이력", "db/d9f/md_docs_2CHANGELOG__KO.html", [
      [ "[Unreleased]", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md480", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md481", null ],
        [ "수정됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md482", null ],
        [ "변경됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md483", null ],
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md484", null ]
      ] ],
      [ "[1.0.0] - 2025-11-17", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md486", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md487", null ],
        [ "의존성", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md488", null ]
      ] ],
      [ "[0.9.0] - 2025-10-20", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md490", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md491", null ],
        [ "변경됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md492", null ]
      ] ]
    ] ],
    [ "Contributing to Messaging System", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html", [
      [ "Table of Contents", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md495", null ],
      [ "Code of Conduct", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md497", null ],
      [ "Getting Started", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md499", [
        [ "Prerequisites", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md500", null ],
        [ "Building for Development", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md501", null ]
      ] ],
      [ "Development Workflow", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md503", null ],
      [ "Coding Standards", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md505", null ],
      [ "Testing Requirements", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md507", null ],
      [ "Pull Request Process", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md509", [
        [ "PR Title Format", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md510", null ]
      ] ],
      [ "Questions?", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md512", null ]
    ] ],
    [ "Messaging System Features", "da/db6/md_docs_2FEATURES.html", [
      [ "Overview", "da/db6/md_docs_2FEATURES.html#autotoc_md515", null ],
      [ "Table of Contents", "da/db6/md_docs_2FEATURES.html#autotoc_md517", null ],
      [ "Core Messaging", "da/db6/md_docs_2FEATURES.html#autotoc_md519", [
        [ "Message Bus", "da/db6/md_docs_2FEATURES.html#autotoc_md520", null ],
        [ "Message Broker", "da/db6/md_docs_2FEATURES.html#autotoc_md521", null ]
      ] ],
      [ "Messaging Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md523", [
        [ "Pub/Sub Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md524", null ],
        [ "Request/Reply Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md525", null ],
        [ "Event Streaming", "da/db6/md_docs_2FEATURES.html#autotoc_md526", null ],
        [ "Message Pipeline", "da/db6/md_docs_2FEATURES.html#autotoc_md527", null ]
      ] ],
      [ "Backend Support", "da/db6/md_docs_2FEATURES.html#autotoc_md529", [
        [ "Backend Interface", "da/db6/md_docs_2FEATURES.html#autotoc_md530", null ],
        [ "Standalone Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md531", null ],
        [ "Integration Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md532", null ],
        [ "Auto-Detection", "da/db6/md_docs_2FEATURES.html#autotoc_md533", null ]
      ] ],
      [ "Message Types", "da/db6/md_docs_2FEATURES.html#autotoc_md535", [
        [ "Message Structure", "da/db6/md_docs_2FEATURES.html#autotoc_md536", null ],
        [ "Message Builder", "da/db6/md_docs_2FEATURES.html#autotoc_md537", null ],
        [ "Message Serialization", "da/db6/md_docs_2FEATURES.html#autotoc_md538", null ]
      ] ],
      [ "Topic Routing", "da/db6/md_docs_2FEATURES.html#autotoc_md540", [
        [ "Wildcard Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md541", null ],
        [ "Subscription Management", "da/db6/md_docs_2FEATURES.html#autotoc_md542", null ]
      ] ],
      [ "Message Queue", "da/db6/md_docs_2FEATURES.html#autotoc_md544", [
        [ "Queue Types", "da/db6/md_docs_2FEATURES.html#autotoc_md545", null ],
        [ "Queue Configuration", "da/db6/md_docs_2FEATURES.html#autotoc_md546", null ]
      ] ],
      [ "Dependency Injection", "da/db6/md_docs_2FEATURES.html#autotoc_md548", [
        [ "DI Container", "da/db6/md_docs_2FEATURES.html#autotoc_md549", null ]
      ] ],
      [ "Error Handling", "da/db6/md_docs_2FEATURES.html#autotoc_md551", [
        [ "Error Codes", "da/db6/md_docs_2FEATURES.html#autotoc_md552", null ],
        [ "Result<T> Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md553", null ]
      ] ],
      [ "Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md555", [
        [ "Thread System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md556", null ],
        [ "Logger System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md557", null ],
        [ "Monitoring System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md558", null ],
        [ "Container System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md559", null ]
      ] ],
      [ "Production Features", "da/db6/md_docs_2FEATURES.html#autotoc_md561", [
        [ "Reliability", "da/db6/md_docs_2FEATURES.html#autotoc_md562", null ],
        [ "Observability", "da/db6/md_docs_2FEATURES.html#autotoc_md563", null ],
        [ "Testing Support", "da/db6/md_docs_2FEATURES.html#autotoc_md564", null ]
      ] ],
      [ "Feature Matrix", "da/db6/md_docs_2FEATURES.html#autotoc_md566", null ],
      [ "Getting Started", "da/db6/md_docs_2FEATURES.html#autotoc_md568", null ]
    ] ],
    [ "Messaging System 기능", "d4/d0c/md_docs_2FEATURES__KO.html", [
      [ "개요", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md572", null ],
      [ "목차", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md574", null ],
      [ "핵심 메시징", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md576", [
        [ "메시지 버스", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md577", null ],
        [ "토픽 라우터", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md578", null ],
        [ "메시지 큐", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md579", null ],
        [ "메시지 직렬화", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md580", null ],
        [ "트레이스 컨텍스트", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md581", null ]
      ] ],
      [ "고급 패턴", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md583", [
        [ "Pub/Sub", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md584", null ],
        [ "Request/Reply", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md585", null ],
        [ "Event Streaming", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md586", null ],
        [ "Message Pipeline", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md587", null ]
      ] ],
      [ "백엔드 지원", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md589", [
        [ "독립 실행", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md590", null ],
        [ "스레드 풀 통합", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md591", null ],
        [ "런타임 백엔드 선택", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md592", null ]
      ] ],
      [ "프로덕션 기능", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md594", [
        [ "스레드 안전", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md595", null ],
        [ "타입 안전", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md596", null ],
        [ "테스트", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md597", null ]
      ] ],
      [ "관련 문서", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md599", null ]
    ] ],
    [ "Build Troubleshooting Guide", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html", [
      [ "Common Build Issues and Solutions", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md601", [
        [ "Issue 1: Target Name Conflicts", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md602", null ],
        [ "Issue 2: GTest Not Found in External Systems", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md604", null ],
        [ "Issue 3: grep -P Not Supported (macOS)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md606", null ],
        [ "Issue 4: yaml-cpp Not Found", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md608", null ]
      ] ],
      [ "Recommended Build Workflow", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md610", [
        [ "For Development (FetchContent Mode)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md611", null ],
        [ "For Production (find_package Mode)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md612", null ]
      ] ],
      [ "Alternative: Minimal Build (No External Systems)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md614", null ],
      [ "Verifying Successful Build", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md616", null ],
      [ "Getting Help", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md618", null ],
      [ "Known Limitations", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md620", null ],
      [ "Platform-Specific Notes", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md622", [
        [ "macOS", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md623", null ],
        [ "Linux (Ubuntu/Debian)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md624", null ],
        [ "Linux (Fedora/RHEL)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md625", null ]
      ] ],
      [ "Quick Fix Summary", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md627", null ]
    ] ],
    [ "Deployment Guide", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html", [
      [ "System Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md630", [
        [ "Hardware Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md631", [
          [ "Minimum Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md632", null ],
          [ "Recommended Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md633", null ],
          [ "Production Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md634", null ]
        ] ],
        [ "Operating System Support", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md635", null ],
        [ "Software Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md636", [
          [ "Build Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md637", null ],
          [ "Runtime Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md638", null ]
        ] ]
      ] ],
      [ "Installation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md639", [
        [ "From Source", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md640", [
          [ "1. Clone Repository", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md641", null ],
          [ "2. Configure Build", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md642", null ],
          [ "3. Build and Install", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md643", null ]
        ] ],
        [ "Using Package Managers", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md644", [
          [ "APT Repository (Ubuntu/Debian)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md645", null ],
          [ "YUM Repository (RHEL/CentOS)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md646", null ]
        ] ],
        [ "Docker Installation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md647", [
          [ "Pull Official Image", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md648", null ],
          [ "Build Custom Image", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md649", null ]
        ] ]
      ] ],
      [ "Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md650", [
        [ "Configuration File Structure", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md651", null ],
        [ "Environment Variables", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md652", null ],
        [ "Secrets Management", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md653", [
          [ "Using HashiCorp Vault", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md654", null ],
          [ "Using Kubernetes Secrets", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md655", null ]
        ] ]
      ] ],
      [ "Deployment Scenarios", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md656", [
        [ "Single Server Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md657", null ],
        [ "High Availability Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md658", [
          [ "Architecture", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md659", null ],
          [ "HAProxy Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md660", null ]
        ] ],
        [ "Kubernetes Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md661", [
          [ "Namespace and ConfigMap", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md662", null ],
          [ "Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md663", null ],
          [ "Service and Ingress", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md664", null ],
          [ "Horizontal Pod Autoscaler", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md665", null ]
        ] ],
        [ "Docker Compose Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md666", null ]
      ] ],
      [ "Performance Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md667", [
        [ "System Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md668", [
          [ "Linux Kernel Parameters", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md669", null ],
          [ "ulimit Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md670", null ]
        ] ],
        [ "Application Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md671", [
          [ "Thread Pool Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md672", null ],
          [ "Memory Management", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md673", null ],
          [ "Network Optimization", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md674", null ]
        ] ],
        [ "Database Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md675", [
          [ "PostgreSQL Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md676", null ],
          [ "Connection Pool Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md677", null ]
        ] ]
      ] ],
      [ "Monitoring Setup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md678", [
        [ "Prometheus Integration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md679", [
          [ "Prometheus Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md680", null ],
          [ "Metrics Exposed", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md681", null ]
        ] ],
        [ "Grafana Dashboard", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md682", null ],
        [ "Logging Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md683", [
          [ "Structured Logging", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md684", null ],
          [ "Log Aggregation (ELK Stack)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md685", null ]
        ] ]
      ] ],
      [ "Scaling Strategies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md686", [
        [ "Horizontal Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md687", [
          [ "Load Balancer Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md688", null ],
          [ "Auto-scaling Rules", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md689", null ]
        ] ],
        [ "Vertical Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md690", [
          [ "Resource Allocation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md691", null ]
        ] ],
        [ "Database Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md692", [
          [ "Read Replicas", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md693", null ],
          [ "Sharding Strategy", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md694", null ]
        ] ]
      ] ],
      [ "Backup and Recovery", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md695", [
        [ "Backup Strategy", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md696", [
          [ "Application State Backup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md697", null ],
          [ "Database Backup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md698", null ]
        ] ],
        [ "Recovery Procedures", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md699", [
          [ "Service Recovery", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md700", null ],
          [ "Disaster Recovery Plan", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md701", null ]
        ] ]
      ] ],
      [ "Security Hardening", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md702", [
        [ "Network Security", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md703", [
          [ "Firewall Rules", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md704", null ],
          [ "SSL/TLS Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md705", null ]
        ] ],
        [ "Application Security", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md706", [
          [ "Authentication", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md707", null ],
          [ "Rate Limiting", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md708", null ]
        ] ],
        [ "Compliance", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md709", [
          [ "Audit Logging", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md710", null ],
          [ "Data Encryption", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md711", null ]
        ] ]
      ] ],
      [ "Troubleshooting Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md712", [
        [ "Common Issues", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md713", [
          [ "Service Won't Start", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md714", null ],
          [ "High Memory Usage", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md715", null ],
          [ "Performance Issues", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md716", null ]
        ] ],
        [ "Health Checks", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md717", [
          [ "Application Health", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md718", null ],
          [ "System Health", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md719", null ]
        ] ]
      ] ]
    ] ],
    [ "Developer Guide", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html", [
      [ "Quick Start", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md721", [
        [ "Prerequisites", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md722", null ],
        [ "1. Clone and Setup", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md723", null ],
        [ "2. Build the Project", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md724", null ],
        [ "3. Your First Application", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md725", null ]
      ] ],
      [ "Development Setup", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md726", [
        [ "IDE Configuration", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md727", [
          [ "Visual Studio Code", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md728", null ],
          [ "CLion", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md729", null ],
          [ "Visual Studio", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md730", null ]
        ] ],
        [ "Development Dependencies", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md731", null ]
      ] ],
      [ "Project Structure", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md732", [
        [ "Directory Layout", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md733", null ],
        [ "Module Organization", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md734", null ]
      ] ],
      [ "Coding Standards", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md735", [
        [ "C++ Style Guide", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md736", [
          [ "Naming Conventions", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md737", null ],
          [ "File Organization", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md738", null ],
          [ "Best Practices", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md739", null ]
        ] ],
        [ "Documentation Standards", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md740", [
          [ "Code Documentation", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md741", null ],
          [ "Comment Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md742", null ]
        ] ]
      ] ],
      [ "Testing Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md743", [
        [ "Unit Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md744", [
          [ "Test Structure", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md745", null ],
          [ "Test Coverage", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md746", null ]
        ] ],
        [ "Integration Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md747", null ],
        [ "Performance Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md748", null ]
      ] ],
      [ "Debugging", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md749", [
        [ "Using GDB", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md750", null ],
        [ "Using Valgrind", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md751", null ],
        [ "Using AddressSanitizer", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md752", null ],
        [ "Using ThreadSanitizer", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md753", null ]
      ] ],
      [ "Performance Profiling", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md754", [
        [ "Using perf", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md755", null ],
        [ "Using Instruments (macOS)", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md756", null ],
        [ "Using Intel VTune", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md757", null ]
      ] ],
      [ "Contributing Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md758", [
        [ "Workflow", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md759", null ],
        [ "Commit Message Format", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md760", null ],
        [ "Code Review Checklist", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md761", null ]
      ] ],
      [ "Build System", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md762", [
        [ "CMake Configuration", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md763", null ],
        [ "Creating New Modules", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md764", null ]
      ] ],
      [ "Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md765", [
        [ "Docker Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md766", null ],
        [ "Kubernetes Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md767", null ]
      ] ],
      [ "Security Best Practices", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md768", [
        [ "Input Validation", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md769", null ],
        [ "Secure Communication", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md770", null ],
        [ "Rate Limiting", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md771", null ]
      ] ],
      [ "Troubleshooting Common Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md772", [
        [ "Build Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md773", null ],
        [ "Runtime Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md774", null ],
        [ "Performance Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md775", null ]
      ] ]
    ] ],
    [ "Frequently Asked Questions", "d4/da1/md_docs_2guides_2FAQ.html", [
      [ "General Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md778", [
        [ "Q: What is messaging_system?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md779", null ],
        [ "Q: What C++ standard is required?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md780", null ],
        [ "Q: Which platforms are supported?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md781", null ]
      ] ],
      [ "Installation & Setup", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md783", [
        [ "Q: How do I install the library?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md784", null ],
        [ "Q: What dependencies are required?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md785", null ]
      ] ],
      [ "Usage Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md787", [
        [ "Q: How do I create a message bus?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md788", null ],
        [ "Q: How do I subscribe to topics?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md789", null ],
        [ "Q: How do I use wildcards?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md790", null ]
      ] ],
      [ "Performance Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md792", [
        [ "Q: What throughput can I expect?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md793", null ],
        [ "Q: What is the latency?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md794", null ]
      ] ],
      [ "More Questions?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md796", null ]
    ] ],
    [ "Migration Guide", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html", [
      [ "Table of Contents", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md798", null ],
      [ "Overview", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md800", [
        [ "Migration Timeline", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md801", null ],
        [ "Compatibility", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md802", null ]
      ] ],
      [ "Breaking Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md804", [
        [ "1. Namespace Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md805", null ],
        [ "2. Error Code Range", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md806", null ],
        [ "3. Message Structure", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md807", null ],
        [ "4. Message Bus Interface", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md808", null ],
        [ "5. Topic Router", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md809", null ]
      ] ],
      [ "Step-by-Step Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md811", [
        [ "Step 1: Update Dependencies", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md812", [
          [ "Update CMakeLists.txt", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md813", null ],
          [ "Update Include Paths", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md814", null ]
        ] ],
        [ "Step 2: Update Includes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md815", null ],
        [ "Step 3: Migrate Message Creation", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md816", null ],
        [ "Step 4: Migrate Message Bus Usage", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md817", null ],
        [ "Step 5: Migrate to Patterns (Optional but Recommended)", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md818", [
          [ "Pub/Sub Pattern", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md819", null ],
          [ "Request-Reply Pattern", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md820", null ]
        ] ],
        [ "Step 6: Update Error Handling", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md821", null ],
        [ "Step 7: Integration with Other Systems", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md822", null ]
      ] ],
      [ "API Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md824", [
        [ "Message API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md825", null ],
        [ "Message Bus API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md826", null ],
        [ "Topic Router API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md827", null ]
      ] ],
      [ "Code Examples", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md829", [
        [ "Example 1: Simple Pub/Sub Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md830", null ],
        [ "Example 2: Request-Reply Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md831", null ],
        [ "Example 3: Event Streaming", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md832", null ],
        [ "Example 4: Message Pipeline", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md833", null ]
      ] ],
      [ "Troubleshooting", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md835", [
        [ "Issue: Compilation errors with old includes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md836", null ],
        [ "Issue: Error code conflicts", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md837", null ],
        [ "Issue: Message bus doesn't start", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md838", null ],
        [ "Issue: Messages not being delivered", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md839", null ],
        [ "Issue: Build performance degradation", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md840", null ]
      ] ],
      [ "FAQ", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md842", [
        [ "Q: Can I use v1.x and v2.0 together?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md843", null ],
        [ "Q: Do I need to migrate all code at once?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md844", null ],
        [ "Q: What about performance?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md845", null ],
        [ "Q: Are there any runtime dependencies?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md846", null ],
        [ "Q: How do I enable lock-free queues?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md847", null ],
        [ "Q: Can I still use simple publish/subscribe?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md848", null ],
        [ "Q: How do I handle migration testing?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md849", null ]
      ] ],
      [ "Additional Resources", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md851", null ],
      [ "Support", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md853", null ]
    ] ],
    [ "Getting Started", "d6/d63/md_docs_2guides_2QUICK__START.html", [
      [ "Table of Contents", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md856", null ],
      [ "Prerequisites", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md857", [
        [ "System Requirements", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md858", null ],
        [ "Development Dependencies", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md859", null ],
        [ "Runtime Dependencies", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md860", null ]
      ] ],
      [ "Installation", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md861", [
        [ "1. Clone the Repository", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md862", null ],
        [ "2. Platform-Specific Setup", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md863", [
          [ "Linux (Ubuntu/Debian)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md864", null ],
          [ "macOS", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md865", null ],
          [ "Windows", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md866", null ]
        ] ],
        [ "3. Build the System", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md867", [
          [ "Quick Build (Recommended)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md868", null ],
          [ "Custom Build Options", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md869", null ]
        ] ],
        [ "4. Verify Installation", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md870", null ]
      ] ],
      [ "Quick Start", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md871", [
        [ "1. Basic Message Bus Usage", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md872", null ],
        [ "2. Container-Based Data Handling", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md873", null ],
        [ "3. Network Client/Server", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md874", null ]
      ] ],
      [ "Basic Usage", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md875", [
        [ "Project Structure", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md876", null ],
        [ "Environment Configuration", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md877", null ],
        [ "Basic Configuration File", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md878", null ]
      ] ],
      [ "First Application", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md879", [
        [ "1. Chat Server (chat_server.cpp)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md880", null ],
        [ "2. Build and Run", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md881", null ],
        [ "3. Test with Sample Client", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md882", null ]
      ] ],
      [ "Next Steps", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md883", [
        [ "1. Explore Sample Applications", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md884", null ],
        [ "2. Advanced Features", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md885", null ],
        [ "3. Development", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md886", null ],
        [ "4. Community", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md887", null ]
      ] ]
    ] ],
    [ "Troubleshooting Guide", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md890", null ],
      [ "FAQ", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md891", [
        [ "General Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md892", null ],
        [ "Performance Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md893", null ],
        [ "Configuration Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md894", null ]
      ] ],
      [ "Debug Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md895", [
        [ "Enable Debug Logging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md896", [
          [ "Runtime Configuration", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md897", null ],
          [ "Environment Variables", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md898", null ],
          [ "Programmatic", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md899", null ]
        ] ],
        [ "Using GDB for Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md900", [
          [ "Attach to Running Process", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md901", null ],
          [ "Debug Core Dumps", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md902", null ]
        ] ],
        [ "Memory Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md903", [
          [ "Using Valgrind", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md904", null ],
          [ "Using AddressSanitizer", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md905", null ],
          [ "Using HeapTrack", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md906", null ]
        ] ],
        [ "Network Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md907", [
          [ "TCP Dump", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md908", null ],
          [ "Network Statistics", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md909", null ],
          [ "Test Connectivity", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md910", null ]
        ] ],
        [ "Thread Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md911", [
          [ "Thread Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md912", null ],
          [ "Detect Deadlocks", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md913", null ],
          [ "ThreadSanitizer", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md914", null ]
        ] ],
        [ "Tracing and Profiling", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md915", [
          [ "System Tracing with strace", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md916", null ],
          [ "Performance Profiling with perf", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md917", null ],
          [ "Application Tracing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md918", null ]
        ] ]
      ] ],
      [ "Performance Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md919", [
        [ "CPU Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md920", [
          [ "Identify CPU Bottlenecks", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md921", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md922", null ]
        ] ],
        [ "Memory Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md923", [
          [ "Memory Usage Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md924", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md925", null ]
        ] ],
        [ "I/O Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md926", [
          [ "Disk I/O Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md927", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md928", null ]
        ] ],
        [ "Network Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md929", [
          [ "Network Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md930", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md931", null ]
        ] ]
      ] ],
      [ "Known Issues", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md932", [
        [ "Issue 1: High CPU Usage with Small Messages", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md933", null ],
        [ "Issue 2: Memory Leak in Long-Running Connections", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md934", null ],
        [ "Issue 3: Database Connection Pool Exhaustion", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md935", null ],
        [ "Issue 4: Deadlock in Message Processing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md936", null ],
        [ "Issue 5: Performance Degradation with Many Topics", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md937", null ]
      ] ],
      [ "Error Messages", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md938", [
        [ "Connection Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md939", [
          [ "\"Connection refused\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md940", null ],
          [ "\"Connection timeout\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md941", null ]
        ] ],
        [ "Database Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md942", [
          [ "\"Database connection failed\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md943", null ],
          [ "\"Deadlock detected\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md944", null ]
        ] ],
        [ "Memory Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md945", [
          [ "\"Out of memory\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md946", null ],
          [ "\"Segmentation fault\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md947", null ]
        ] ]
      ] ],
      [ "Common Problems", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md948", [
        [ "Problem: Service Won't Start", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md949", null ],
        [ "Problem: Slow Message Processing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md950", null ],
        [ "Problem: Connection Drops", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md951", null ],
        [ "Problem: High Memory Usage", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md952", null ],
        [ "Problem: Database Bottleneck", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md953", null ]
      ] ]
    ] ],
    [ "Performance Baseline", "d9/df1/md_docs_2performance_2BASELINE.html", [
      [ "Overview", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md971", null ],
      [ "Baseline Metrics", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md973", [
        [ "Message Bus Performance", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md974", null ],
        [ "Pattern Performance", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md975", null ]
      ] ],
      [ "Test Environment", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md977", [
        [ "Reference Platform", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md978", null ]
      ] ],
      [ "Benchmark Commands", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md980", null ],
      [ "Regression Detection", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md982", null ],
      [ "Historical Data", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md984", null ]
    ] ],
    [ "Performance Guide", "d9/dd2/md_docs_2performance_2PERFORMANCE.html", [
      [ "Table of Contents", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md986", null ],
      [ "Performance Overview", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md987", [
        [ "Design Goals", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md988", null ],
        [ "Key Performance Features", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md989", null ]
      ] ],
      [ "Benchmark Results", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md990", [
        [ "Test Environment", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md991", null ],
        [ "Overall System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md992", null ],
        [ "Latency Measurements", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md993", null ],
        [ "Memory Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md994", null ]
      ] ],
      [ "Component Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md995", [
        [ "Thread System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md996", [
          [ "Lock-free vs Mutex Comparison", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md997", null ],
          [ "Scaling Characteristics", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md998", null ]
        ] ],
        [ "Container System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md999", [
          [ "Serialization Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1000", null ],
          [ "SIMD Optimization Impact", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1001", null ]
        ] ],
        [ "Network System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1002", [
          [ "Connection Scaling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1003", null ],
          [ "Protocol Overhead", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1004", null ]
        ] ],
        [ "Database System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1005", [
          [ "Query Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1006", null ],
          [ "Connection Pool Impact", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1007", null ]
        ] ]
      ] ],
      [ "Optimization Techniques", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1008", [
        [ "1. Memory Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1009", [
          [ "Object Pooling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1010", null ],
          [ "Custom Allocators", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1011", null ]
        ] ],
        [ "2. CPU Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1012", [
          [ "SIMD Utilization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1013", null ],
          [ "Cache Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1014", null ]
        ] ],
        [ "3. Network Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1015", [
          [ "Batching and Pipelining", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1016", null ]
        ] ]
      ] ],
      [ "Performance Monitoring", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1017", [
        [ "1. Built-in Metrics", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1018", null ],
        [ "2. Performance Profiling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1019", null ]
      ] ],
      [ "Tuning Guidelines", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1020", [
        [ "1. Thread Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1021", null ],
        [ "2. Memory Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1022", null ],
        [ "3. Network Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1023", null ]
      ] ],
      [ "Troubleshooting Performance Issues", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1024", [
        [ "1. Common Performance Problems", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1025", [
          [ "High CPU Usage", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1026", null ],
          [ "Memory Leaks", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1027", null ],
          [ "Network Bottlenecks", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1028", null ]
        ] ],
        [ "2. Performance Monitoring Dashboard", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1029", null ]
      ] ]
    ] ],
    [ "Messaging System Production Quality", "dd/def/md_docs_2PRODUCTION__QUALITY.html", [
      [ "Overview", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1033", null ],
      [ "Quality Metrics", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1035", [
        [ "Test Coverage", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1036", null ],
        [ "CI/CD Pipeline", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1037", null ]
      ] ],
      [ "Code Quality", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1039", [
        [ "Static Analysis", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1040", null ],
        [ "Memory Safety", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1041", null ],
        [ "Code Reviews", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1042", null ]
      ] ],
      [ "Testing Strategy", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1044", [
        [ "Unit Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1045", null ],
        [ "Integration Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1046", null ],
        [ "Performance Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1047", null ]
      ] ],
      [ "Reliability Features", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1049", [
        [ "Error Handling", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1050", null ],
        [ "Fault Tolerance", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1051", null ],
        [ "Monitoring", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1052", null ]
      ] ],
      [ "Production Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1054", [
        [ "Configuration", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1055", null ],
        [ "Resource Requirements", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1056", null ],
        [ "Scalability", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1057", null ]
      ] ],
      [ "Security", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1059", [
        [ "Input Validation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1060", null ],
        [ "Thread Safety", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1061", null ],
        [ "Resource Management", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1062", null ]
      ] ],
      [ "Documentation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1064", [
        [ "API Documentation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1065", null ],
        [ "Guides", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1066", null ]
      ] ],
      [ "Industry Standards", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1068", [
        [ "Compliance", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1069", null ],
        [ "Best Practices", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1070", null ]
      ] ],
      [ "Known Limitations", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1072", [
        [ "Current Limitations", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1073", null ],
        [ "Planned Improvements", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1074", null ]
      ] ],
      [ "Production Checklist", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1076", [
        [ "Pre-Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1077", null ],
        [ "Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1078", null ],
        [ "Post-Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1079", null ]
      ] ],
      [ "Support", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1081", [
        [ "Getting Help", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1082", null ],
        [ "Reporting Bugs", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1083", null ]
      ] ],
      [ "Conclusion", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1085", null ]
    ] ],
    [ "Messaging System 프로덕션 품질", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html", [
      [ "요약", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1089", null ],
      [ "목차", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1091", null ],
      [ "CI/CD 인프라", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1093", [
        [ "빌드 매트릭스", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1094", null ]
      ] ],
      [ "스레드 안전성 검증", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1096", [
        [ "ThreadSanitizer 결과", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1097", null ]
      ] ],
      [ "테스트 커버리지", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1099", [
        [ "컴포넌트별 커버리지", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1100", null ]
      ] ],
      [ "플랫폼 지원", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1102", [
        [ "지원 플랫폼", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1103", null ],
        [ "컴파일러 지원", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1104", null ]
      ] ],
      [ "관련 문서", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1106", null ]
    ] ],
    [ "Messaging System Project Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html", [
      [ "Overview", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1109", null ],
      [ "Directory Layout", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1111", null ],
      [ "Component Organization", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1113", [
        [ "Core Components (<tt>include/kcenon/messaging/core/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1114", null ],
        [ "Interfaces (<tt>include/kcenon/messaging/interfaces/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1115", null ],
        [ "Backends (<tt>include/kcenon/messaging/backends/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1116", null ],
        [ "Patterns (<tt>include/kcenon/messaging/patterns/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1117", null ]
      ] ],
      [ "Build System", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1119", [
        [ "CMake Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1120", null ],
        [ "Build Targets", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1121", null ]
      ] ],
      [ "Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1123", [
        [ "Required Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1124", null ],
        [ "Optional Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1125", null ]
      ] ],
      [ "Include Patterns", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1127", [
        [ "Application Code", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1128", null ],
        [ "Internal Implementation", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1129", null ]
      ] ],
      [ "Naming Conventions", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1131", [
        [ "Files", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1132", null ],
        [ "Classes", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1133", null ],
        [ "Namespaces", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1134", null ]
      ] ],
      [ "Code Organization Principles", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1136", [
        [ "Separation of Concerns", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1137", null ],
        [ "Modularity", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1138", null ],
        [ "Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1139", null ]
      ] ],
      [ "Future Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1141", [
        [ "Planned Additions", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1142", null ]
      ] ]
    ] ],
    [ "Messaging System 프로젝트 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html", [
      [ "개요", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1146", null ],
      [ "디렉토리 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1148", null ],
      [ "주요 디렉토리 설명", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1150", [
        [ "include/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1151", null ],
        [ "src/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1152", null ],
        [ "docs/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1153", null ],
        [ "tests/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1154", null ],
        [ "examples/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1155", null ]
      ] ],
      [ "관련 문서", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1157", null ]
    ] ],
    [ "Messaging System 문서", "d8/d0a/md_docs_2README__KO.html", [
      [ "문서 인덱스", "d8/d0a/md_docs_2README__KO.html#autotoc_md1177", null ],
      [ "시작하기", "d8/d0a/md_docs_2README__KO.html#autotoc_md1179", null ],
      [ "핵심 문서", "d8/d0a/md_docs_2README__KO.html#autotoc_md1181", null ],
      [ "품질 및 운영", "d8/d0a/md_docs_2README__KO.html#autotoc_md1183", null ],
      [ "디렉토리 구조", "d8/d0a/md_docs_2README__KO.html#autotoc_md1185", null ],
      [ "관련 링크", "d8/d0a/md_docs_2README__KO.html#autotoc_md1187", null ]
    ] ],
    [ "Distributed Task Queue System - Improvement Plan", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html", [
      [ "Executive Summary", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1189", null ],
      [ "Phase 1: Task Core (기반 확장)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1191", [
        [ "1.1 Task 정의 확장", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1192", null ],
        [ "1.2 Task Builder", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1193", null ]
      ] ],
      [ "Phase 2: Worker System", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1195", [
        [ "2.1 Task Handler Interface", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1196", null ],
        [ "2.2 Worker Pool", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1197", null ]
      ] ],
      [ "Phase 3: Task Queue (확장)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1199", [
        [ "3.1 Priority Task Queue", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1200", null ]
      ] ],
      [ "Phase 4: Result Backend", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1202", [
        [ "4.1 Result Backend Interface", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1203", null ],
        [ "4.2 In-Memory Result Backend", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1204", null ],
        [ "4.3 Redis Result Backend (선택적)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1205", null ]
      ] ],
      [ "Phase 5: Task Client (Producer)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1207", [
        [ "5.1 Task Client", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1208", null ]
      ] ],
      [ "Phase 6: Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1210", [
        [ "6.1 Periodic Task Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1211", null ]
      ] ],
      [ "Phase 7: Monitoring & Management", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1213", [
        [ "7.1 Task Monitor", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1214", null ]
      ] ],
      [ "Phase 8: Integration", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1216", [
        [ "8.1 통합 서비스 컨테이너", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1217", null ]
      ] ],
      [ "Implementation Roadmap", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1219", [
        [ "Sprint 1: Core Task Infrastructure ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1220", null ],
        [ "Sprint 2: Queue & Result Backend ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1221", null ],
        [ "Sprint 3: Worker System ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1222", null ],
        [ "Sprint 4: Client & Async Result ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1223", null ],
        [ "Sprint 5: Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1224", null ],
        [ "Sprint 6: Monitoring & System Integration", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1225", null ],
        [ "Sprint 7: Testing & Documentation", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1226", null ]
      ] ],
      [ "File Structure", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1228", null ],
      [ "Usage Example", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1230", null ],
      [ "Reusing Existing Components", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1232", null ],
      [ "Key Design Decisions", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1234", [
        [ "1. Task는 Message를 상속", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1235", null ],
        [ "2. Result Backend 분리", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1236", null ],
        [ "3. 핸들러 등록 방식", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1237", null ],
        [ "4. 비동기 우선 설계", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1238", null ]
      ] ],
      [ "Version", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1240", null ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html",
"d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md622",
"d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1011",
"dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md807",
"df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md71"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';