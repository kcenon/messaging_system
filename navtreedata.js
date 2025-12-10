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
    [ "Architecture Decision Records (ADR)", "index.html", "index" ],
    [ "ADR-001: Logging Architecture - ILogger Interface vs logger_system", "de/dd2/md_docs_2adr_2001-logging-architecture.html", [
      [ "Status", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md1", null ],
      [ "Context", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md2", [
        [ "Background", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md3", null ],
        [ "Problem Statement", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md4", null ]
      ] ],
      [ "Decision", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md5", [
        [ "Architecture Overview", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md6", null ],
        [ "Implementation Details", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md7", [
          [ "1. ILogger Interface (<tt>common_system</tt>)", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md8", null ],
          [ "2. GlobalLoggerRegistry (<tt>common_system</tt>)", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md9", null ],
          [ "3. Convenience Functions (<tt>common_system</tt>)", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md10", null ],
          [ "4. CMake Dependency Configuration", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md11", null ]
        ] ]
      ] ],
      [ "Consequences", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md12", [
        [ "Positive", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md13", null ],
        [ "Negative", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md14", null ],
        [ "Neutral", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md15", null ]
      ] ],
      [ "Alternatives Considered", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md16", [
        [ "Alternative A: Direct logger_system Dependency", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md17", null ],
        [ "Alternative B: Compile-time Logger Selection via Templates", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md18", null ],
        [ "Alternative C: Macro-based Logging", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md19", null ]
      ] ],
      [ "Compliance", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md20", [
        [ "Current Implementation Status", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md21", null ],
        [ "Usage Examples in messaging_system", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md22", null ]
      ] ],
      [ "Related Decisions", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md23", null ],
      [ "References", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md24", null ],
      [ "Revision History", "de/dd2/md_docs_2adr_2001-logging-architecture.html#autotoc_md25", null ]
    ] ],
    [ "ADR-001: 로깅 아키텍처 - ILogger 인터페이스 vs logger_system", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html", [
      [ "상태", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md27", null ],
      [ "배경", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md28", [
        [ "배경 설명", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md29", null ],
        [ "문제 정의", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md30", null ]
      ] ],
      [ "결정", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md31", [
        [ "아키텍처 개요", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md32", null ],
        [ "구현 세부사항", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md33", [
          [ "1. ILogger 인터페이스 (<tt>common_system</tt>)", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md34", null ],
          [ "2. GlobalLoggerRegistry (<tt>common_system</tt>)", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md35", null ],
          [ "3. 편의 함수 (<tt>common_system</tt>)", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md36", null ],
          [ "4. CMake 의존성 구성", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md37", null ]
        ] ]
      ] ],
      [ "결과", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md38", [
        [ "긍정적", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md39", null ],
        [ "부정적", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md40", null ],
        [ "중립적", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md41", null ]
      ] ],
      [ "고려된 대안", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md42", [
        [ "대안 A: logger_system 직접 의존성", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md43", null ],
        [ "대안 B: 템플릿을 통한 컴파일 타임 로거 선택", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md44", null ],
        [ "대안 C: 매크로 기반 로깅", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md45", null ]
      ] ],
      [ "준수 사항", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md46", [
        [ "현재 구현 상태", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md47", null ],
        [ "messaging_system에서의 사용 예시", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md48", null ]
      ] ],
      [ "관련 결정", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md49", null ],
      [ "참조", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md50", null ],
      [ "수정 이력", "dd/da0/md_docs_2adr_2001-logging-architecture__KO.html#autotoc_md51", null ]
    ] ],
    [ "Design Patterns and Architectural Decisions", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html", [
      [ "Overview", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md60", null ],
      [ "Creational Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md61", [
        [ "1. Singleton Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md62", null ],
        [ "2. Factory Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md63", null ],
        [ "3. Builder Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md64", null ],
        [ "4. Object Pool Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md65", null ]
      ] ],
      [ "Structural Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md66", [
        [ "5. Adapter Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md67", null ],
        [ "6. Decorator Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md68", null ],
        [ "7. Proxy Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md69", null ],
        [ "8. Composite Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md70", null ]
      ] ],
      [ "Behavioral Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md71", [
        [ "9. Observer Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md72", null ],
        [ "10. Strategy Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md73", null ],
        [ "11. Chain of Responsibility Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md74", null ],
        [ "12. Command Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md75", null ],
        [ "13. Template Method Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md76", null ]
      ] ],
      [ "Concurrency Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md77", [
        [ "14. Producer-Consumer Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md78", null ],
        [ "15. Thread Pool Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md79", null ]
      ] ],
      [ "Architectural Patterns", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md80", [
        [ "16. Microkernel Pattern", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md81", null ],
        [ "17. Event-Driven Architecture", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md82", null ]
      ] ],
      [ "Summary", "d6/d58/md_docs_2advanced_2DESIGN__PATTERNS.html#autotoc_md83", null ]
    ] ],
    [ "Messaging Patterns API Reference", "df/dde/md_docs_2advanced_2PATTERNS__API.html", [
      [ "Table of Contents", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md85", null ],
      [ "Pub/Sub Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md87", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md88", null ],
        [ "Class: <tt>publisher</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md89", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md90", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md91", [
            [ "<tt>publish</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md92", null ],
            [ "<tt>get_default_topic</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md93", null ],
            [ "<tt>set_default_topic</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md94", null ],
            [ "<tt>is_ready</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md95", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md96", null ]
        ] ],
        [ "Class: <tt>subscriber</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md97", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md98", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md99", [
            [ "<tt>subscribe</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md100", null ],
            [ "<tt>unsubscribe_all</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md101", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md102", null ]
        ] ]
      ] ],
      [ "Request-Reply Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md104", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md105", null ],
        [ "Class: <tt>request_reply_handler</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md106", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md107", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md108", [
            [ "<tt>request</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md109", null ],
            [ "<tt>register_handler</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md110", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md111", null ]
        ] ],
        [ "Class: <tt>request_client</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md112", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md113", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md114", [
            [ "<tt>request</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md115", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md116", null ]
        ] ],
        [ "Class: <tt>request_server</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md117", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md118", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md119", [
            [ "<tt>register_handler</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md120", null ],
            [ "<tt>start</tt> / <tt>stop</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md121", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md122", null ]
        ] ]
      ] ],
      [ "Event Streaming Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md124", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md125", null ],
        [ "Struct: <tt>event_stream_config</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md126", null ],
        [ "Class: <tt>event_stream</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md127", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md128", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md129", [
            [ "<tt>publish_event</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md130", null ],
            [ "<tt>subscribe</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md131", null ],
            [ "<tt>unsubscribe</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md132", null ],
            [ "<tt>replay</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md133", null ],
            [ "<tt>get_events</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md134", null ],
            [ "<tt>event_count</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md135", null ],
            [ "<tt>clear_buffer</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md136", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md137", null ]
        ] ],
        [ "Class: <tt>event_batch_processor</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md138", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md139", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md140", [
            [ "<tt>start</tt> / <tt>stop</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md141", null ],
            [ "<tt>flush</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md142", null ],
            [ "<tt>is_running</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md143", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md144", null ]
        ] ]
      ] ],
      [ "Message Pipeline Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md146", [
        [ "Namespace: <tt>kcenon::messaging::patterns</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md147", null ],
        [ "Type Aliases", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md148", null ],
        [ "Class: <tt>message_pipeline</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md149", [
          [ "Struct: <tt>pipeline_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md150", null ],
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md151", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md152", [
            [ "<tt>add_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md153", null ],
            [ "<tt>remove_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md154", null ],
            [ "<tt>start</tt> / <tt>stop</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md155", null ],
            [ "<tt>process</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md156", null ],
            [ "<tt>stage_count</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md157", null ],
            [ "<tt>get_stage_names</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md158", null ],
            [ "<tt>get_statistics</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md159", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md160", null ]
        ] ],
        [ "Class: <tt>pipeline_builder</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md161", [
          [ "Constructor", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md162", null ],
          [ "Methods", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md163", [
            [ "<tt>from</tt> / <tt>to</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md164", null ],
            [ "<tt>add_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md165", null ],
            [ "<tt>add_filter</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md166", null ],
            [ "<tt>add_transformer</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md167", null ],
            [ "<tt>build</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md168", null ]
          ] ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md169", null ]
        ] ],
        [ "Namespace: <tt>pipeline_stages</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md170", [
          [ "<tt>create_logging_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md171", null ],
          [ "<tt>create_validation_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md172", null ],
          [ "<tt>create_enrichment_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md173", null ],
          [ "<tt>create_retry_stage</tt>", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md174", null ],
          [ "Usage Example", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md175", null ]
        ] ]
      ] ],
      [ "Best Practices", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md177", [
        [ "Pub/Sub Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md178", null ],
        [ "Request-Reply Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md179", null ],
        [ "Event Streaming Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md180", null ],
        [ "Message Pipeline Pattern", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md181", null ],
        [ "General Guidelines", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md182", null ]
      ] ],
      [ "Error Handling", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md184", null ],
      [ "Thread Safety", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md186", null ],
      [ "Performance Considerations", "df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md188", null ]
    ] ],
    [ "System Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html", [
      [ "Overview", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md190", null ],
      [ "Architectural Principles", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md191", null ],
      [ "System Layers", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md192", null ],
      [ "Component Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md193", [
        [ "1. Container System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md194", null ],
        [ "2. Network System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md195", null ],
        [ "3. Database System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md196", null ],
        [ "4. Thread System", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md197", null ]
      ] ],
      [ "Data Flow Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md198", [
        [ "Message Processing Pipeline", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md199", null ],
        [ "Request-Response Flow", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md200", null ],
        [ "Distributed Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md201", null ]
      ] ],
      [ "Scalability Patterns", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md202", [
        [ "Horizontal Scaling", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md203", null ],
        [ "Vertical Scaling", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md204", null ]
      ] ],
      [ "Integration Points", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md205", [
        [ "External Systems", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md206", null ],
        [ "Internal Communication", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md207", null ]
      ] ],
      [ "Performance Optimization", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md208", [
        [ "Memory Management", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md209", null ],
        [ "Concurrency", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md210", null ],
        [ "Network", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md211", null ]
      ] ],
      [ "Fault Tolerance", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md212", [
        [ "Error Recovery", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md213", null ],
        [ "High Availability", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md214", null ]
      ] ],
      [ "Security Considerations", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md215", [
        [ "Transport Security", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md216", null ],
        [ "Application Security", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md217", null ]
      ] ],
      [ "Monitoring and Observability", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md218", [
        [ "Metrics Collection", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md219", null ],
        [ "Distributed Tracing", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md220", null ],
        [ "Logging", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md221", null ]
      ] ],
      [ "Configuration Management", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md222", [
        [ "Static Configuration", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md223", null ],
        [ "Dynamic Configuration", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md224", null ]
      ] ],
      [ "Deployment Architecture", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md225", [
        [ "Container Deployment", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md226", null ],
        [ "Kubernetes Integration", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md227", null ]
      ] ],
      [ "Future Considerations", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md228", [
        [ "Planned Enhancements", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md229", null ],
        [ "Technology Evaluation", "df/dbc/md_docs_2advanced_2SYSTEM__ARCHITECTURE.html#autotoc_md230", null ]
      ] ]
    ] ],
    [ "Windows CI Performance Optimization", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html", [
      [ "Executive Summary", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md232", null ],
      [ "Problem Analysis", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md233", [
        [ "Initial State (PR #31 - Before Fix)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md234", null ],
        [ "Target State (PR #30 - Success)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md235", null ]
      ] ],
      [ "Implemented Solutions", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md236", [
        [ "1. Test-Level Timeouts ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md237", null ],
        [ "2. CTest Parallel Execution ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md238", null ],
        [ "3. Build Parallelization ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md239", null ]
      ] ],
      [ "Performance Comparison", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md240", null ],
      [ "Additional Recommendations", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md241", [
        [ "Short-Term (Quick Wins)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md242", [
          [ "1. Use Release Build for Tests", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md243", null ],
          [ "2. Increase vcpkg Cache Efficiency", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md244", null ],
          [ "3. Reduce FetchContent Downloads", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md245", null ]
        ] ],
        [ "Medium-Term (Architectural)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md246", [
          [ "1. Split Test Execution", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md247", null ],
          [ "2. Implement Test Sharding", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md248", null ],
          [ "3. Conditional Integration Tests", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md249", null ]
        ] ],
        [ "Long-Term (Infrastructure)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md250", [
          [ "1. Use Self-Hosted Runners", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md251", null ],
          [ "2. Implement Build Caching Service", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md252", null ],
          [ "3. Prebuilt Dependency Binaries", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md253", null ]
        ] ]
      ] ],
      [ "Implementation Priority", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md254", [
        [ "Priority 1 (Implemented) ✅", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md255", null ],
        [ "Priority 2 (Recommended - Next Sprint)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md256", null ],
        [ "Priority 3 (Future Consideration)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md257", null ],
        [ "Priority 4 (Long-Term Planning)", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md258", null ]
      ] ],
      [ "Monitoring", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md259", [
        [ "Key Metrics to Track", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md260", null ],
        [ "Alerting Thresholds", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md261", null ]
      ] ],
      [ "Conclusion", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md262", null ],
      [ "References", "d1/dc9/md_docs_2advanced_2WINDOWS__OPTIMIZATION.html#autotoc_md263", null ]
    ] ],
    [ "API Reference", "db/d91/md_docs_2API__REFERENCE.html", [
      [ "Table of Contents", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md265", null ],
      [ "Container System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md267", [
        [ "Namespace: <tt>container_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md268", null ],
        [ "Class: <tt>value_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md269", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md270", null ],
          [ "Core Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md271", [
            [ "Setting Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md272", null ],
            [ "Adding Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md273", null ],
            [ "Retrieving Values", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md274", null ],
            [ "Serialization", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md275", null ]
          ] ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md276", null ]
        ] ],
        [ "Class: <tt>variant</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md277", [
          [ "Supported Types", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md278", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md279", null ]
        ] ]
      ] ],
      [ "Network System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md281", [
        [ "Namespace: <tt>network_module</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md282", null ],
        [ "Class: <tt>messaging_server</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md283", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md284", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md285", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md286", null ]
        ] ],
        [ "Class: <tt>messaging_client</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md287", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md288", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md289", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md290", null ]
        ] ]
      ] ],
      [ "Database System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md292", [
        [ "Namespace: <tt>database</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md293", null ],
        [ "Class: <tt>database_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md294", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md295", null ],
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md296", null ],
          [ "Connection Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md297", null ],
          [ "Query Execution", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md298", null ],
          [ "Transaction Support", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md299", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md300", null ]
        ] ]
      ] ],
      [ "Message Bus API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md302", [
        [ "Namespace: <tt>kcenon::messaging::core</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md303", null ],
        [ "Class: <tt>message_bus</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md304", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md305", null ],
          [ "Configuration Structure", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md306", null ],
          [ "Lifecycle Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md307", null ],
          [ "Publishing", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md308", null ],
          [ "Subscription", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md309", null ],
          [ "Request-Response", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md310", null ],
          [ "Monitoring", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md311", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md312", null ]
        ] ]
      ] ],
      [ "Service Container API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md314", [
        [ "Namespace: <tt>kcenon::messaging::services</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md315", null ],
        [ "Class: <tt>service_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md316", [
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md317", null ],
          [ "Service Lifetimes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md318", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md319", null ]
        ] ]
      ] ],
      [ "Thread System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md321", [
        [ "Namespace: <tt>thread_system</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md322", null ],
        [ "Class: <tt>thread_pool</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md323", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md324", null ],
          [ "Job Submission", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md325", null ],
          [ "Pool Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md326", null ],
          [ "Priority Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md327", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md328", null ]
        ] ],
        [ "Class: <tt>lock_free_queue</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md329", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md330", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md331", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md332", null ]
        ] ]
      ] ],
      [ "Logger System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md334", [
        [ "Namespace: <tt>logger</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md335", null ],
        [ "Class: <tt>logger_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md336", [
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md337", null ],
          [ "Log Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md338", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md339", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md340", null ]
        ] ]
      ] ],
      [ "Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md342", [
        [ "System Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md343", null ],
        [ "Error Handling", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md344", null ],
        [ "Error Recovery Strategies", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md345", null ]
      ] ],
      [ "Configuration Reference", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md346", [
        [ "System Configuration File Format", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md347", null ],
        [ "Environment Variables", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md348", null ]
      ] ]
    ] ],
    [ "Messaging System API 레퍼런스", "d4/d54/md_docs_2API__REFERENCE__KO.html", [
      [ "개요", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md351", null ],
      [ "목차", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md353", null ],
      [ "핵심 클래스", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md355", [
        [ "message_bus", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md356", null ]
      ] ],
      [ "메시지 버스", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md358", [
        [ "사용 예제", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md359", null ]
      ] ],
      [ "토픽 라우터", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md361", [
        [ "와일드카드 패턴", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md362", null ]
      ] ],
      [ "메시징 패턴", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md364", [
        [ "Pub/Sub", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md365", null ],
        [ "Request/Reply", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md366", null ]
      ] ],
      [ "직렬화", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md368", [
        [ "메시지 구조", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md369", null ]
      ] ],
      [ "관련 문서", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md371", null ]
    ] ],
    [ "Architecture", "d2/d64/md_docs_2architecture.html", [
      [ "Overview", "d2/d64/md_docs_2architecture.html#autotoc_md373", null ],
      [ "Related Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md374", [
        [ "Component-Specific Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md375", null ],
        [ "Module Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md376", null ],
        [ "Integration Guides", "d2/d64/md_docs_2architecture.html#autotoc_md377", null ]
      ] ],
      [ "Architectural Principles", "d2/d64/md_docs_2architecture.html#autotoc_md378", null ],
      [ "System Layers", "d2/d64/md_docs_2architecture.html#autotoc_md379", null ],
      [ "Component Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md380", [
        [ "1. Container System", "d2/d64/md_docs_2architecture.html#autotoc_md381", null ],
        [ "2. Network System", "d2/d64/md_docs_2architecture.html#autotoc_md382", null ],
        [ "3. Database System", "d2/d64/md_docs_2architecture.html#autotoc_md383", null ],
        [ "4. Thread System", "d2/d64/md_docs_2architecture.html#autotoc_md384", null ]
      ] ],
      [ "Data Flow Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md385", [
        [ "Message Processing Pipeline", "d2/d64/md_docs_2architecture.html#autotoc_md386", null ],
        [ "Request-Response Flow", "d2/d64/md_docs_2architecture.html#autotoc_md387", null ],
        [ "Distributed Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md388", null ]
      ] ],
      [ "Scalability Patterns", "d2/d64/md_docs_2architecture.html#autotoc_md389", [
        [ "Horizontal Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md390", null ],
        [ "Vertical Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md391", null ]
      ] ],
      [ "Integration Points", "d2/d64/md_docs_2architecture.html#autotoc_md392", [
        [ "External Systems", "d2/d64/md_docs_2architecture.html#autotoc_md393", null ],
        [ "Internal Communication", "d2/d64/md_docs_2architecture.html#autotoc_md394", null ]
      ] ],
      [ "Performance Optimization", "d2/d64/md_docs_2architecture.html#autotoc_md395", [
        [ "Memory Management", "d2/d64/md_docs_2architecture.html#autotoc_md396", null ],
        [ "Concurrency", "d2/d64/md_docs_2architecture.html#autotoc_md397", null ],
        [ "Network", "d2/d64/md_docs_2architecture.html#autotoc_md398", null ]
      ] ],
      [ "Fault Tolerance", "d2/d64/md_docs_2architecture.html#autotoc_md399", [
        [ "Error Recovery", "d2/d64/md_docs_2architecture.html#autotoc_md400", null ],
        [ "High Availability", "d2/d64/md_docs_2architecture.html#autotoc_md401", null ]
      ] ],
      [ "Security Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md402", [
        [ "Transport Security", "d2/d64/md_docs_2architecture.html#autotoc_md403", null ],
        [ "Application Security", "d2/d64/md_docs_2architecture.html#autotoc_md404", null ]
      ] ],
      [ "Monitoring and Observability", "d2/d64/md_docs_2architecture.html#autotoc_md405", [
        [ "Metrics Collection", "d2/d64/md_docs_2architecture.html#autotoc_md406", null ],
        [ "Distributed Tracing", "d2/d64/md_docs_2architecture.html#autotoc_md407", null ],
        [ "Logging", "d2/d64/md_docs_2architecture.html#autotoc_md408", null ]
      ] ],
      [ "Configuration Management", "d2/d64/md_docs_2architecture.html#autotoc_md409", [
        [ "Static Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md410", null ],
        [ "Dynamic Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md411", null ]
      ] ],
      [ "Deployment Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md412", [
        [ "Container Deployment", "d2/d64/md_docs_2architecture.html#autotoc_md413", null ],
        [ "Kubernetes Integration", "d2/d64/md_docs_2architecture.html#autotoc_md414", null ]
      ] ],
      [ "Future Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md415", [
        [ "Planned Enhancements", "d2/d64/md_docs_2architecture.html#autotoc_md416", null ],
        [ "Technology Evaluation", "d2/d64/md_docs_2architecture.html#autotoc_md417", null ]
      ] ]
    ] ],
    [ "Messaging System 아키텍처", "de/de3/md_docs_2ARCHITECTURE__KO.html", [
      [ "개요", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md420", null ],
      [ "목차", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md422", null ],
      [ "시스템 개요", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md424", null ],
      [ "컴포넌트 아키텍처", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md426", [
        [ "핵심 컴포넌트", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md427", null ],
        [ "메시징 패턴", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md428", null ]
      ] ],
      [ "의존성 구조", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md430", [
        [ "버전 매트릭스", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md431", null ]
      ] ],
      [ "데이터 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md433", [
        [ "메시지 발행 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md434", null ],
        [ "Request/Reply 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md435", null ]
      ] ],
      [ "설계 원칙", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md437", [
        [ "1. 느슨한 결합", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md438", null ],
        [ "2. 비동기 우선", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md439", null ],
        [ "3. 스레드 안전", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md440", null ],
        [ "4. 확장성", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md441", null ]
      ] ],
      [ "관련 문서", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md443", null ]
    ] ],
    [ "Messaging System Performance Benchmarks", "de/d93/md_docs_2BENCHMARKS.html", [
      [ "Executive Summary", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md446", null ],
      [ "Table of Contents", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md448", null ],
      [ "Core Performance Metrics", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md450", [
        [ "Reference Performance Metrics", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md451", null ]
      ] ],
      [ "Message Creation Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md453", [
        [ "Message Builder Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md454", null ]
      ] ],
      [ "Queue Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md456", [
        [ "Priority Queue Operations", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md457", null ],
        [ "Queue Contention Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md458", null ]
      ] ],
      [ "Topic Router Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md460", [
        [ "Pattern Matching Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md461", null ],
        [ "Subscription Count Impact", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md462", null ]
      ] ],
      [ "Pub/Sub Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md464", [
        [ "End-to-End Throughput", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md465", null ],
        [ "Backend Impact", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md466", null ]
      ] ],
      [ "Request/Reply Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md468", [
        [ "Synchronous RPC Latency", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md469", null ],
        [ "Concurrent Request Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md470", null ]
      ] ],
      [ "Task Module Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md472", [
        [ "Task Queue Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md473", null ],
        [ "Worker Throughput", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md474", null ],
        [ "Worker Scalability", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md475", null ],
        [ "Result Backend Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md476", null ],
        [ "Scheduler Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md477", null ],
        [ "Performance Targets Summary", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md478", null ]
      ] ],
      [ "Memory Usage", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md480", [
        [ "Per-Component Memory Overhead", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md481", null ],
        [ "Memory Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md482", null ]
      ] ],
      [ "Latency Analysis", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md484", [
        [ "Latency Distribution (Pub/Sub)", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md485", null ],
        [ "Latency Breakdown", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md486", null ]
      ] ],
      [ "Scalability", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md488", [
        [ "Thread Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md489", null ],
        [ "Publisher/Subscriber Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md490", null ]
      ] ],
      [ "Optimization Insights", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md492", [
        [ "Performance Tuning Recommendations", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md493", null ],
        [ "Common Bottlenecks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md494", null ]
      ] ],
      [ "Running Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md496", [
        [ "Prerequisites", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md497", null ],
        [ "Available Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md498", null ],
        [ "Benchmark Output Format", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md499", null ]
      ] ],
      [ "Continuous Performance Monitoring", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md501", null ],
      [ "Comparison with Other Systems", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md503", [
        [ "Feature Comparison", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md504", null ]
      ] ],
      [ "Conclusion", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md506", null ]
    ] ],
    [ "Messaging System 성능 벤치마크", "d5/ddb/md_docs_2BENCHMARKS__KO.html", [
      [ "요약", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md510", null ],
      [ "목차", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md512", null ],
      [ "핵심 성능 메트릭", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md514", [
        [ "요약 테이블", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md515", null ]
      ] ],
      [ "메시지 버스 벤치마크", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md517", [
        [ "처리량", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md518", null ],
        [ "토픽 라우팅 성능", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md519", null ]
      ] ],
      [ "패턴별 성능", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md521", [
        [ "Pub/Sub", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md522", null ],
        [ "Request/Reply", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md523", null ]
      ] ],
      [ "확장성", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md525", [
        [ "수평 확장", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md526", null ]
      ] ],
      [ "벤치마크 방법론", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md528", null ],
      [ "관련 문서", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md530", null ]
    ] ],
    [ "Messaging System Changelog", "da/dee/md_docs_2CHANGELOG.html", [
      [ "[Unreleased]", "da/dee/md_docs_2CHANGELOG.html#autotoc_md532", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md533", null ],
        [ "Fixed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md534", null ],
        [ "Changed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md535", null ],
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md536", null ]
      ] ],
      [ "[1.0.0] - 2025-11-17", "da/dee/md_docs_2CHANGELOG.html#autotoc_md538", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md539", null ],
        [ "Dependencies", "da/dee/md_docs_2CHANGELOG.html#autotoc_md540", null ]
      ] ],
      [ "[0.9.0] - 2025-10-20", "da/dee/md_docs_2CHANGELOG.html#autotoc_md542", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md543", null ],
        [ "Changed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md544", null ]
      ] ]
    ] ],
    [ "Messaging System 변경 이력", "db/d9f/md_docs_2CHANGELOG__KO.html", [
      [ "[Unreleased]", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md547", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md548", null ],
        [ "수정됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md549", null ],
        [ "변경됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md550", null ],
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md551", null ]
      ] ],
      [ "[1.0.0] - 2025-11-17", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md553", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md554", null ],
        [ "의존성", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md555", null ]
      ] ],
      [ "[0.9.0] - 2025-10-20", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md557", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md558", null ],
        [ "변경됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md559", null ]
      ] ]
    ] ],
    [ "Contributing to Messaging System", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html", [
      [ "Table of Contents", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md562", null ],
      [ "Code of Conduct", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md564", null ],
      [ "Getting Started", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md566", [
        [ "Prerequisites", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md567", null ],
        [ "Building for Development", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md568", null ]
      ] ],
      [ "Development Workflow", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md570", null ],
      [ "Coding Standards", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md572", null ],
      [ "Testing Requirements", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md574", null ],
      [ "Pull Request Process", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md576", [
        [ "PR Title Format", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md577", null ]
      ] ],
      [ "Questions?", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md579", null ]
    ] ],
    [ "Messaging System Features", "da/db6/md_docs_2FEATURES.html", [
      [ "Overview", "da/db6/md_docs_2FEATURES.html#autotoc_md582", null ],
      [ "Table of Contents", "da/db6/md_docs_2FEATURES.html#autotoc_md584", null ],
      [ "Core Messaging", "da/db6/md_docs_2FEATURES.html#autotoc_md586", [
        [ "Message Bus", "da/db6/md_docs_2FEATURES.html#autotoc_md587", null ],
        [ "Message Broker", "da/db6/md_docs_2FEATURES.html#autotoc_md588", null ]
      ] ],
      [ "Messaging Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md590", [
        [ "Pub/Sub Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md591", null ],
        [ "Request/Reply Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md592", null ],
        [ "Event Streaming", "da/db6/md_docs_2FEATURES.html#autotoc_md593", null ],
        [ "Message Pipeline", "da/db6/md_docs_2FEATURES.html#autotoc_md594", null ]
      ] ],
      [ "Backend Support", "da/db6/md_docs_2FEATURES.html#autotoc_md596", [
        [ "Backend Interface", "da/db6/md_docs_2FEATURES.html#autotoc_md597", null ],
        [ "Standalone Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md598", null ],
        [ "Integration Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md599", null ],
        [ "Auto-Detection", "da/db6/md_docs_2FEATURES.html#autotoc_md600", null ]
      ] ],
      [ "Message Types", "da/db6/md_docs_2FEATURES.html#autotoc_md602", [
        [ "Message Structure", "da/db6/md_docs_2FEATURES.html#autotoc_md603", null ],
        [ "Message Builder", "da/db6/md_docs_2FEATURES.html#autotoc_md604", null ],
        [ "Message Serialization", "da/db6/md_docs_2FEATURES.html#autotoc_md605", null ]
      ] ],
      [ "Topic Routing", "da/db6/md_docs_2FEATURES.html#autotoc_md607", [
        [ "Wildcard Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md608", null ],
        [ "Subscription Management", "da/db6/md_docs_2FEATURES.html#autotoc_md609", null ]
      ] ],
      [ "Message Queue", "da/db6/md_docs_2FEATURES.html#autotoc_md611", [
        [ "Queue Types", "da/db6/md_docs_2FEATURES.html#autotoc_md612", null ],
        [ "Queue Configuration", "da/db6/md_docs_2FEATURES.html#autotoc_md613", null ]
      ] ],
      [ "Dependency Injection", "da/db6/md_docs_2FEATURES.html#autotoc_md615", [
        [ "DI Container", "da/db6/md_docs_2FEATURES.html#autotoc_md616", null ]
      ] ],
      [ "Error Handling", "da/db6/md_docs_2FEATURES.html#autotoc_md618", [
        [ "Error Codes", "da/db6/md_docs_2FEATURES.html#autotoc_md619", null ],
        [ "Result<T> Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md620", null ]
      ] ],
      [ "Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md622", [
        [ "Thread System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md623", null ],
        [ "Logger System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md624", null ],
        [ "Monitoring System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md625", null ],
        [ "Container System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md626", null ]
      ] ],
      [ "Production Features", "da/db6/md_docs_2FEATURES.html#autotoc_md628", [
        [ "Reliability", "da/db6/md_docs_2FEATURES.html#autotoc_md629", null ],
        [ "Observability", "da/db6/md_docs_2FEATURES.html#autotoc_md630", null ],
        [ "Testing Support", "da/db6/md_docs_2FEATURES.html#autotoc_md631", null ]
      ] ],
      [ "Feature Matrix", "da/db6/md_docs_2FEATURES.html#autotoc_md633", null ],
      [ "Getting Started", "da/db6/md_docs_2FEATURES.html#autotoc_md635", null ]
    ] ],
    [ "Messaging System 기능", "d4/d0c/md_docs_2FEATURES__KO.html", [
      [ "개요", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md639", null ],
      [ "목차", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md641", null ],
      [ "핵심 메시징", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md643", [
        [ "메시지 버스", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md644", null ],
        [ "토픽 라우터", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md645", null ],
        [ "메시지 큐", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md646", null ],
        [ "메시지 직렬화", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md647", null ],
        [ "트레이스 컨텍스트", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md648", null ]
      ] ],
      [ "고급 패턴", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md650", [
        [ "Pub/Sub", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md651", null ],
        [ "Request/Reply", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md652", null ],
        [ "Event Streaming", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md653", null ],
        [ "Message Pipeline", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md654", null ]
      ] ],
      [ "백엔드 지원", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md656", [
        [ "독립 실행", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md657", null ],
        [ "스레드 풀 통합", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md658", null ],
        [ "런타임 백엔드 선택", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md659", null ]
      ] ],
      [ "프로덕션 기능", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md661", [
        [ "스레드 안전", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md662", null ],
        [ "타입 안전", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md663", null ],
        [ "테스트", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md664", null ]
      ] ],
      [ "관련 문서", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md666", null ]
    ] ],
    [ "Build Troubleshooting Guide", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html", [
      [ "Common Build Issues and Solutions", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md668", [
        [ "Issue 1: Target Name Conflicts", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md669", null ],
        [ "Issue 2: GTest Not Found in External Systems", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md671", null ],
        [ "Issue 3: grep -P Not Supported (macOS)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md673", null ],
        [ "Issue 4: yaml-cpp Not Found", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md675", null ]
      ] ],
      [ "Recommended Build Workflow", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md677", [
        [ "For Development (FetchContent Mode)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md678", null ],
        [ "For Production (find_package Mode)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md679", null ]
      ] ],
      [ "Alternative: Minimal Build (No External Systems)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md681", null ],
      [ "Verifying Successful Build", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md683", null ],
      [ "Getting Help", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md685", null ],
      [ "Known Limitations", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md687", null ],
      [ "Platform-Specific Notes", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md689", [
        [ "macOS", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md690", null ],
        [ "Linux (Ubuntu/Debian)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md691", null ],
        [ "Linux (Fedora/RHEL)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md692", null ]
      ] ],
      [ "Quick Fix Summary", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md694", null ]
    ] ],
    [ "Deployment Guide", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html", [
      [ "System Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md697", [
        [ "Hardware Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md698", [
          [ "Minimum Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md699", null ],
          [ "Recommended Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md700", null ],
          [ "Production Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md701", null ]
        ] ],
        [ "Operating System Support", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md702", null ],
        [ "Software Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md703", [
          [ "Build Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md704", null ],
          [ "Runtime Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md705", null ]
        ] ]
      ] ],
      [ "Installation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md706", [
        [ "From Source", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md707", [
          [ "1. Clone Repository", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md708", null ],
          [ "2. Configure Build", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md709", null ],
          [ "3. Build and Install", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md710", null ]
        ] ],
        [ "Using Package Managers", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md711", [
          [ "APT Repository (Ubuntu/Debian)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md712", null ],
          [ "YUM Repository (RHEL/CentOS)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md713", null ]
        ] ],
        [ "Docker Installation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md714", [
          [ "Pull Official Image", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md715", null ],
          [ "Build Custom Image", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md716", null ]
        ] ]
      ] ],
      [ "Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md717", [
        [ "Configuration File Structure", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md718", null ],
        [ "Environment Variables", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md719", null ],
        [ "Secrets Management", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md720", [
          [ "Using HashiCorp Vault", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md721", null ],
          [ "Using Kubernetes Secrets", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md722", null ]
        ] ]
      ] ],
      [ "Deployment Scenarios", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md723", [
        [ "Single Server Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md724", null ],
        [ "High Availability Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md725", [
          [ "Architecture", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md726", null ],
          [ "HAProxy Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md727", null ]
        ] ],
        [ "Kubernetes Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md728", [
          [ "Namespace and ConfigMap", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md729", null ],
          [ "Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md730", null ],
          [ "Service and Ingress", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md731", null ],
          [ "Horizontal Pod Autoscaler", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md732", null ]
        ] ],
        [ "Docker Compose Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md733", null ]
      ] ],
      [ "Performance Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md734", [
        [ "System Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md735", [
          [ "Linux Kernel Parameters", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md736", null ],
          [ "ulimit Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md737", null ]
        ] ],
        [ "Application Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md738", [
          [ "Thread Pool Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md739", null ],
          [ "Memory Management", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md740", null ],
          [ "Network Optimization", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md741", null ]
        ] ],
        [ "Database Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md742", [
          [ "PostgreSQL Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md743", null ],
          [ "Connection Pool Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md744", null ]
        ] ]
      ] ],
      [ "Monitoring Setup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md745", [
        [ "Prometheus Integration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md746", [
          [ "Prometheus Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md747", null ],
          [ "Metrics Exposed", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md748", null ]
        ] ],
        [ "Grafana Dashboard", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md749", null ],
        [ "Logging Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md750", [
          [ "Structured Logging", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md751", null ],
          [ "Log Aggregation (ELK Stack)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md752", null ]
        ] ]
      ] ],
      [ "Scaling Strategies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md753", [
        [ "Horizontal Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md754", [
          [ "Load Balancer Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md755", null ],
          [ "Auto-scaling Rules", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md756", null ]
        ] ],
        [ "Vertical Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md757", [
          [ "Resource Allocation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md758", null ]
        ] ],
        [ "Database Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md759", [
          [ "Read Replicas", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md760", null ],
          [ "Sharding Strategy", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md761", null ]
        ] ]
      ] ],
      [ "Backup and Recovery", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md762", [
        [ "Backup Strategy", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md763", [
          [ "Application State Backup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md764", null ],
          [ "Database Backup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md765", null ]
        ] ],
        [ "Recovery Procedures", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md766", [
          [ "Service Recovery", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md767", null ],
          [ "Disaster Recovery Plan", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md768", null ]
        ] ]
      ] ],
      [ "Security Hardening", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md769", [
        [ "Network Security", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md770", [
          [ "Firewall Rules", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md771", null ],
          [ "SSL/TLS Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md772", null ]
        ] ],
        [ "Application Security", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md773", [
          [ "Authentication", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md774", null ],
          [ "Rate Limiting", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md775", null ]
        ] ],
        [ "Compliance", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md776", [
          [ "Audit Logging", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md777", null ],
          [ "Data Encryption", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md778", null ]
        ] ]
      ] ],
      [ "Troubleshooting Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md779", [
        [ "Common Issues", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md780", [
          [ "Service Won't Start", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md781", null ],
          [ "High Memory Usage", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md782", null ],
          [ "Performance Issues", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md783", null ]
        ] ],
        [ "Health Checks", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md784", [
          [ "Application Health", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md785", null ],
          [ "System Health", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md786", null ]
        ] ]
      ] ]
    ] ],
    [ "Developer Guide", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html", [
      [ "Quick Start", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md788", [
        [ "Prerequisites", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md789", null ],
        [ "1. Clone and Setup", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md790", null ],
        [ "2. Build the Project", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md791", null ],
        [ "3. Your First Application", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md792", null ]
      ] ],
      [ "Development Setup", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md793", [
        [ "IDE Configuration", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md794", [
          [ "Visual Studio Code", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md795", null ],
          [ "CLion", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md796", null ],
          [ "Visual Studio", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md797", null ]
        ] ],
        [ "Development Dependencies", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md798", null ]
      ] ],
      [ "Project Structure", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md799", [
        [ "Directory Layout", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md800", null ],
        [ "Module Organization", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md801", null ]
      ] ],
      [ "Coding Standards", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md802", [
        [ "C++ Style Guide", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md803", [
          [ "Naming Conventions", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md804", null ],
          [ "File Organization", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md805", null ],
          [ "Best Practices", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md806", null ]
        ] ],
        [ "Documentation Standards", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md807", [
          [ "Code Documentation", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md808", null ],
          [ "Comment Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md809", null ]
        ] ]
      ] ],
      [ "Testing Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md810", [
        [ "Unit Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md811", [
          [ "Test Structure", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md812", null ],
          [ "Test Coverage", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md813", null ]
        ] ],
        [ "Integration Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md814", null ],
        [ "Performance Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md815", null ]
      ] ],
      [ "Debugging", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md816", [
        [ "Using GDB", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md817", null ],
        [ "Using Valgrind", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md818", null ],
        [ "Using AddressSanitizer", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md819", null ],
        [ "Using ThreadSanitizer", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md820", null ]
      ] ],
      [ "Performance Profiling", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md821", [
        [ "Using perf", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md822", null ],
        [ "Using Instruments (macOS)", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md823", null ],
        [ "Using Intel VTune", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md824", null ]
      ] ],
      [ "Contributing Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md825", [
        [ "Workflow", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md826", null ],
        [ "Commit Message Format", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md827", null ],
        [ "Code Review Checklist", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md828", null ]
      ] ],
      [ "Build System", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md829", [
        [ "CMake Configuration", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md830", null ],
        [ "Creating New Modules", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md831", null ]
      ] ],
      [ "Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md832", [
        [ "Docker Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md833", null ],
        [ "Kubernetes Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md834", null ]
      ] ],
      [ "Security Best Practices", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md835", [
        [ "Input Validation", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md836", null ],
        [ "Secure Communication", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md837", null ],
        [ "Rate Limiting", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md838", null ]
      ] ],
      [ "Troubleshooting Common Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md839", [
        [ "Build Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md840", null ],
        [ "Runtime Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md841", null ],
        [ "Performance Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md842", null ]
      ] ]
    ] ],
    [ "Frequently Asked Questions", "d4/da1/md_docs_2guides_2FAQ.html", [
      [ "General Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md845", [
        [ "Q: What is messaging_system?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md846", null ],
        [ "Q: What C++ standard is required?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md847", null ],
        [ "Q: Which platforms are supported?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md848", null ]
      ] ],
      [ "Installation & Setup", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md850", [
        [ "Q: How do I install the library?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md851", null ],
        [ "Q: What dependencies are required?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md852", null ]
      ] ],
      [ "Usage Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md854", [
        [ "Q: How do I create a message bus?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md855", null ],
        [ "Q: How do I subscribe to topics?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md856", null ],
        [ "Q: How do I use wildcards?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md857", null ]
      ] ],
      [ "Performance Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md859", [
        [ "Q: What throughput can I expect?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md860", null ],
        [ "Q: What is the latency?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md861", null ]
      ] ],
      [ "More Questions?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md863", null ]
    ] ],
    [ "Migration Guide", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html", [
      [ "Table of Contents", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md865", null ],
      [ "Overview", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md867", [
        [ "Migration Timeline", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md868", null ],
        [ "Compatibility", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md869", null ]
      ] ],
      [ "Breaking Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md871", [
        [ "1. Namespace Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md872", null ],
        [ "2. Error Code Range", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md873", null ],
        [ "3. Message Structure", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md874", null ],
        [ "4. Message Bus Interface", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md875", null ],
        [ "5. Topic Router", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md876", null ]
      ] ],
      [ "Step-by-Step Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md878", [
        [ "Step 1: Update Dependencies", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md879", [
          [ "Update CMakeLists.txt", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md880", null ],
          [ "Update Include Paths", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md881", null ]
        ] ],
        [ "Step 2: Update Includes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md882", null ],
        [ "Step 3: Migrate Message Creation", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md883", null ],
        [ "Step 4: Migrate Message Bus Usage", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md884", null ],
        [ "Step 5: Migrate to Patterns (Optional but Recommended)", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md885", [
          [ "Pub/Sub Pattern", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md886", null ],
          [ "Request-Reply Pattern", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md887", null ]
        ] ],
        [ "Step 6: Update Error Handling", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md888", null ],
        [ "Step 7: Integration with Other Systems", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md889", null ]
      ] ],
      [ "API Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md891", [
        [ "Message API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md892", null ],
        [ "Message Bus API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md893", null ],
        [ "Topic Router API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md894", null ]
      ] ],
      [ "Code Examples", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md896", [
        [ "Example 1: Simple Pub/Sub Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md897", null ],
        [ "Example 2: Request-Reply Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md898", null ],
        [ "Example 3: Event Streaming", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md899", null ],
        [ "Example 4: Message Pipeline", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md900", null ]
      ] ],
      [ "Troubleshooting", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md902", [
        [ "Issue: Compilation errors with old includes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md903", null ],
        [ "Issue: Error code conflicts", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md904", null ],
        [ "Issue: Message bus doesn't start", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md905", null ],
        [ "Issue: Messages not being delivered", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md906", null ],
        [ "Issue: Build performance degradation", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md907", null ]
      ] ],
      [ "FAQ", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md909", [
        [ "Q: Can I use v1.x and v2.0 together?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md910", null ],
        [ "Q: Do I need to migrate all code at once?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md911", null ],
        [ "Q: What about performance?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md912", null ],
        [ "Q: Are there any runtime dependencies?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md913", null ],
        [ "Q: How do I enable lock-free queues?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md914", null ],
        [ "Q: Can I still use simple publish/subscribe?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md915", null ],
        [ "Q: How do I handle migration testing?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md916", null ]
      ] ],
      [ "Additional Resources", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md918", null ],
      [ "Support", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md920", null ]
    ] ],
    [ "Getting Started", "d6/d63/md_docs_2guides_2QUICK__START.html", [
      [ "Table of Contents", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md923", null ],
      [ "Prerequisites", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md924", [
        [ "System Requirements", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md925", null ],
        [ "Development Dependencies", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md926", null ],
        [ "Runtime Dependencies", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md927", null ]
      ] ],
      [ "Installation", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md928", [
        [ "1. Clone the Repository", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md929", null ],
        [ "2. Platform-Specific Setup", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md930", [
          [ "Linux (Ubuntu/Debian)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md931", null ],
          [ "macOS", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md932", null ],
          [ "Windows", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md933", null ]
        ] ],
        [ "3. Build the System", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md934", [
          [ "Quick Build (Recommended)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md935", null ],
          [ "Custom Build Options", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md936", null ]
        ] ],
        [ "4. Verify Installation", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md937", null ]
      ] ],
      [ "Quick Start", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md938", [
        [ "1. Basic Message Bus Usage", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md939", null ],
        [ "2. Container-Based Data Handling", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md940", null ],
        [ "3. Network Client/Server", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md941", null ]
      ] ],
      [ "Basic Usage", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md942", [
        [ "Project Structure", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md943", null ],
        [ "Environment Configuration", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md944", null ],
        [ "Basic Configuration File", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md945", null ]
      ] ],
      [ "First Application", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md946", [
        [ "1. Chat Server (chat_server.cpp)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md947", null ],
        [ "2. Build and Run", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md948", null ],
        [ "3. Test with Sample Client", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md949", null ]
      ] ],
      [ "Next Steps", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md950", [
        [ "1. Explore Sample Applications", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md951", null ],
        [ "2. Advanced Features", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md952", null ],
        [ "3. Development", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md953", null ],
        [ "4. Community", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md954", null ]
      ] ]
    ] ],
    [ "Troubleshooting Guide", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md957", null ],
      [ "FAQ", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md958", [
        [ "General Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md959", null ],
        [ "Performance Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md960", null ],
        [ "Configuration Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md961", null ]
      ] ],
      [ "Debug Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md962", [
        [ "Enable Debug Logging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md963", [
          [ "Runtime Configuration", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md964", null ],
          [ "Environment Variables", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md965", null ],
          [ "Programmatic", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md966", null ]
        ] ],
        [ "Using GDB for Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md967", [
          [ "Attach to Running Process", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md968", null ],
          [ "Debug Core Dumps", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md969", null ]
        ] ],
        [ "Memory Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md970", [
          [ "Using Valgrind", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md971", null ],
          [ "Using AddressSanitizer", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md972", null ],
          [ "Using HeapTrack", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md973", null ]
        ] ],
        [ "Network Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md974", [
          [ "TCP Dump", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md975", null ],
          [ "Network Statistics", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md976", null ],
          [ "Test Connectivity", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md977", null ]
        ] ],
        [ "Thread Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md978", [
          [ "Thread Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md979", null ],
          [ "Detect Deadlocks", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md980", null ],
          [ "ThreadSanitizer", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md981", null ]
        ] ],
        [ "Tracing and Profiling", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md982", [
          [ "System Tracing with strace", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md983", null ],
          [ "Performance Profiling with perf", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md984", null ],
          [ "Application Tracing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md985", null ]
        ] ]
      ] ],
      [ "Performance Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md986", [
        [ "CPU Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md987", [
          [ "Identify CPU Bottlenecks", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md988", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md989", null ]
        ] ],
        [ "Memory Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md990", [
          [ "Memory Usage Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md991", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md992", null ]
        ] ],
        [ "I/O Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md993", [
          [ "Disk I/O Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md994", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md995", null ]
        ] ],
        [ "Network Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md996", [
          [ "Network Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md997", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md998", null ]
        ] ]
      ] ],
      [ "Known Issues", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md999", [
        [ "Issue 1: High CPU Usage with Small Messages", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1000", null ],
        [ "Issue 2: Memory Leak in Long-Running Connections", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1001", null ],
        [ "Issue 3: Database Connection Pool Exhaustion", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1002", null ],
        [ "Issue 4: Deadlock in Message Processing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1003", null ],
        [ "Issue 5: Performance Degradation with Many Topics", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1004", null ]
      ] ],
      [ "Error Messages", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1005", [
        [ "Connection Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1006", [
          [ "\"Connection refused\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1007", null ],
          [ "\"Connection timeout\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1008", null ]
        ] ],
        [ "Database Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1009", [
          [ "\"Database connection failed\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1010", null ],
          [ "\"Deadlock detected\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1011", null ]
        ] ],
        [ "Memory Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1012", [
          [ "\"Out of memory\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1013", null ],
          [ "\"Segmentation fault\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1014", null ]
        ] ]
      ] ],
      [ "Common Problems", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1015", [
        [ "Problem: Service Won't Start", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1016", null ],
        [ "Problem: Slow Message Processing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1017", null ],
        [ "Problem: Connection Drops", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1018", null ],
        [ "Problem: High Memory Usage", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1019", null ],
        [ "Problem: Database Bottleneck", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1020", null ]
      ] ]
    ] ],
    [ "Performance Baseline", "d9/df1/md_docs_2performance_2BASELINE.html", [
      [ "Overview", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1044", null ],
      [ "Baseline Metrics", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1046", [
        [ "Message Bus Performance", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1047", null ],
        [ "Pattern Performance", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1048", null ]
      ] ],
      [ "Test Environment", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1050", [
        [ "Reference Platform", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1051", null ]
      ] ],
      [ "Benchmark Commands", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1053", null ],
      [ "Regression Detection", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1055", null ],
      [ "Historical Data", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1057", null ]
    ] ],
    [ "Performance Guide", "d9/dd2/md_docs_2performance_2PERFORMANCE.html", [
      [ "Table of Contents", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1059", null ],
      [ "Performance Overview", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1060", [
        [ "Design Goals", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1061", null ],
        [ "Key Performance Features", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1062", null ]
      ] ],
      [ "Benchmark Results", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1063", [
        [ "Test Environment", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1064", null ],
        [ "Overall System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1065", null ],
        [ "Latency Measurements", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1066", null ],
        [ "Memory Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1067", null ]
      ] ],
      [ "Component Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1068", [
        [ "Thread System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1069", [
          [ "Lock-free vs Mutex Comparison", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1070", null ],
          [ "Scaling Characteristics", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1071", null ]
        ] ],
        [ "Container System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1072", [
          [ "Serialization Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1073", null ],
          [ "SIMD Optimization Impact", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1074", null ]
        ] ],
        [ "Network System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1075", [
          [ "Connection Scaling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1076", null ],
          [ "Protocol Overhead", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1077", null ]
        ] ],
        [ "Database System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1078", [
          [ "Query Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1079", null ],
          [ "Connection Pool Impact", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1080", null ]
        ] ]
      ] ],
      [ "Optimization Techniques", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1081", [
        [ "1. Memory Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1082", [
          [ "Object Pooling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1083", null ],
          [ "Custom Allocators", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1084", null ]
        ] ],
        [ "2. CPU Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1085", [
          [ "SIMD Utilization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1086", null ],
          [ "Cache Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1087", null ]
        ] ],
        [ "3. Network Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1088", [
          [ "Batching and Pipelining", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1089", null ]
        ] ]
      ] ],
      [ "Performance Monitoring", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1090", [
        [ "1. Built-in Metrics", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1091", null ],
        [ "2. Performance Profiling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1092", null ]
      ] ],
      [ "Tuning Guidelines", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1093", [
        [ "1. Thread Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1094", null ],
        [ "2. Memory Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1095", null ],
        [ "3. Network Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1096", null ]
      ] ],
      [ "Troubleshooting Performance Issues", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1097", [
        [ "1. Common Performance Problems", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1098", [
          [ "High CPU Usage", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1099", null ],
          [ "Memory Leaks", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1100", null ],
          [ "Network Bottlenecks", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1101", null ]
        ] ],
        [ "2. Performance Monitoring Dashboard", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1102", null ]
      ] ]
    ] ],
    [ "Messaging System Production Quality", "dd/def/md_docs_2PRODUCTION__QUALITY.html", [
      [ "Overview", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1106", null ],
      [ "Quality Metrics", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1108", [
        [ "Test Coverage", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1109", null ],
        [ "CI/CD Pipeline", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1110", null ]
      ] ],
      [ "Code Quality", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1112", [
        [ "Static Analysis", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1113", null ],
        [ "Memory Safety", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1114", null ],
        [ "Code Reviews", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1115", null ]
      ] ],
      [ "Testing Strategy", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1117", [
        [ "Unit Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1118", null ],
        [ "Integration Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1119", null ],
        [ "Performance Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1120", null ]
      ] ],
      [ "Reliability Features", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1122", [
        [ "Error Handling", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1123", null ],
        [ "Fault Tolerance", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1124", null ],
        [ "Monitoring", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1125", null ]
      ] ],
      [ "Production Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1127", [
        [ "Configuration", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1128", null ],
        [ "Resource Requirements", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1129", null ],
        [ "Scalability", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1130", null ]
      ] ],
      [ "Security", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1132", [
        [ "Input Validation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1133", null ],
        [ "Thread Safety", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1134", null ],
        [ "Resource Management", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1135", null ]
      ] ],
      [ "Documentation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1137", [
        [ "API Documentation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1138", null ],
        [ "Guides", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1139", null ]
      ] ],
      [ "Industry Standards", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1141", [
        [ "Compliance", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1142", null ],
        [ "Best Practices", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1143", null ]
      ] ],
      [ "Known Limitations", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1145", [
        [ "Current Limitations", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1146", null ],
        [ "Planned Improvements", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1147", null ]
      ] ],
      [ "Production Checklist", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1149", [
        [ "Pre-Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1150", null ],
        [ "Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1151", null ],
        [ "Post-Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1152", null ]
      ] ],
      [ "Support", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1154", [
        [ "Getting Help", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1155", null ],
        [ "Reporting Bugs", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1156", null ]
      ] ],
      [ "Conclusion", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1158", null ]
    ] ],
    [ "Messaging System 프로덕션 품질", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html", [
      [ "요약", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1162", null ],
      [ "목차", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1164", null ],
      [ "CI/CD 인프라", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1166", [
        [ "빌드 매트릭스", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1167", null ]
      ] ],
      [ "스레드 안전성 검증", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1169", [
        [ "ThreadSanitizer 결과", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1170", null ]
      ] ],
      [ "테스트 커버리지", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1172", [
        [ "컴포넌트별 커버리지", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1173", null ]
      ] ],
      [ "플랫폼 지원", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1175", [
        [ "지원 플랫폼", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1176", null ],
        [ "컴파일러 지원", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1177", null ]
      ] ],
      [ "관련 문서", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1179", null ]
    ] ],
    [ "Messaging System Project Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html", [
      [ "Overview", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1182", null ],
      [ "Directory Layout", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1184", null ],
      [ "Component Organization", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1186", [
        [ "Core Components (<tt>include/kcenon/messaging/core/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1187", null ],
        [ "Interfaces (<tt>include/kcenon/messaging/interfaces/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1188", null ],
        [ "Backends (<tt>include/kcenon/messaging/backends/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1189", null ],
        [ "Patterns (<tt>include/kcenon/messaging/patterns/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1190", null ]
      ] ],
      [ "Build System", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1192", [
        [ "CMake Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1193", null ],
        [ "Build Targets", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1194", null ]
      ] ],
      [ "Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1196", [
        [ "Required Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1197", null ],
        [ "Optional Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1198", null ]
      ] ],
      [ "Include Patterns", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1200", [
        [ "Application Code", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1201", null ],
        [ "Internal Implementation", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1202", null ]
      ] ],
      [ "Naming Conventions", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1204", [
        [ "Files", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1205", null ],
        [ "Classes", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1206", null ],
        [ "Namespaces", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1207", null ]
      ] ],
      [ "Code Organization Principles", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1209", [
        [ "Separation of Concerns", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1210", null ],
        [ "Modularity", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1211", null ],
        [ "Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1212", null ]
      ] ],
      [ "Future Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1214", [
        [ "Planned Additions", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1215", null ]
      ] ]
    ] ],
    [ "Messaging System 프로젝트 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html", [
      [ "개요", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1219", null ],
      [ "디렉토리 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1221", null ],
      [ "주요 디렉토리 설명", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1223", [
        [ "include/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1224", null ],
        [ "src/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1225", null ],
        [ "docs/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1226", null ],
        [ "tests/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1227", null ],
        [ "examples/", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1228", null ]
      ] ],
      [ "관련 문서", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1230", null ]
    ] ],
    [ "Messaging System 문서", "d8/d0a/md_docs_2README__KO.html", [
      [ "문서 인덱스", "d8/d0a/md_docs_2README__KO.html#autotoc_md1252", null ],
      [ "시작하기", "d8/d0a/md_docs_2README__KO.html#autotoc_md1254", null ],
      [ "핵심 문서", "d8/d0a/md_docs_2README__KO.html#autotoc_md1256", null ],
      [ "품질 및 운영", "d8/d0a/md_docs_2README__KO.html#autotoc_md1258", null ],
      [ "Task 큐 시스템", "d8/d0a/md_docs_2README__KO.html#autotoc_md1260", null ],
      [ "아키텍처 결정 기록 (ADR)", "d8/d0a/md_docs_2README__KO.html#autotoc_md1262", null ],
      [ "디렉토리 구조", "d8/d0a/md_docs_2README__KO.html#autotoc_md1264", null ],
      [ "관련 링크", "d8/d0a/md_docs_2README__KO.html#autotoc_md1266", null ]
    ] ],
    [ "Task Module API Reference", "df/d18/md_docs_2task_2API__REFERENCE.html", [
      [ "Table of Contents", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1268", null ],
      [ "task", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1270", [
        [ "Enumerations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1271", [
          [ "task_state", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1272", null ]
        ] ],
        [ "Constructors", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1273", null ],
        [ "Identification Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1274", null ],
        [ "State Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1275", null ],
        [ "Configuration Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1276", null ],
        [ "Execution Tracking", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1277", null ],
        [ "Progress Tracking", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1278", null ],
        [ "Result/Error Handling", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1279", null ],
        [ "Retry Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1280", null ],
        [ "Serialization", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1281", null ]
      ] ],
      [ "task_config", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1283", [
        [ "Fields", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1284", null ]
      ] ],
      [ "task_builder", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1286", [
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1287", null ],
        [ "Builder Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1288", null ],
        [ "Build Method", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1289", null ],
        [ "Example", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1290", null ]
      ] ],
      [ "task_handler_interface", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1292", [
        [ "Pure Virtual Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1293", null ],
        [ "Virtual Lifecycle Hooks", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1294", null ],
        [ "Example", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1295", null ],
        [ "Helper Functions", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1296", null ]
      ] ],
      [ "task_context", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1298", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1299", [
          [ "progress_info", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1300", null ],
          [ "task_log_entry", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1301", null ]
        ] ],
        [ "Progress Tracking", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1302", null ],
        [ "Checkpoint Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1303", null ],
        [ "Subtask Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1304", null ],
        [ "Cancellation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1305", null ],
        [ "Logging", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1306", null ]
      ] ],
      [ "task_queue", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1308", [
        [ "Configuration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1309", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1310", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1311", null ],
        [ "Enqueue Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1312", null ],
        [ "Dequeue Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1313", null ],
        [ "Cancellation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1314", null ],
        [ "Query Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1315", null ]
      ] ],
      [ "worker_pool", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1317", [
        [ "Configuration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1318", null ],
        [ "Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1319", null ],
        [ "Handler Registration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1320", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1321", null ],
        [ "Status", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1322", null ],
        [ "Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1323", null ]
      ] ],
      [ "task_client", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1325", [
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1326", null ],
        [ "Immediate Execution", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1327", null ],
        [ "Delayed Execution", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1328", null ],
        [ "Batch Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1329", null ],
        [ "Workflow Patterns", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1330", null ],
        [ "Result/Cancellation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1331", null ],
        [ "Status", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1332", null ]
      ] ],
      [ "async_result", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1334", [
        [ "Status Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1335", null ],
        [ "Progress Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1336", null ],
        [ "Blocking Result Retrieval", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1337", null ],
        [ "Callback-Based", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1338", null ],
        [ "Task Control", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1339", null ]
      ] ],
      [ "result_backend_interface", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1341", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1342", null ],
        [ "Pure Virtual Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1343", null ],
        [ "Optional Virtual Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1344", null ]
      ] ],
      [ "memory_result_backend", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1346", [
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1347", null ],
        [ "Additional Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1348", null ]
      ] ],
      [ "task_scheduler", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1350", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1351", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1352", null ],
        [ "Schedule Registration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1353", null ],
        [ "Schedule Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1354", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1355", null ],
        [ "Query", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1356", null ],
        [ "Event Callbacks", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1357", null ]
      ] ],
      [ "cron_parser", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1359", [
        [ "Cron Expression Format", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1360", null ],
        [ "Supported Syntax", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1361", null ],
        [ "Structure", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1362", null ],
        [ "Static Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1363", null ],
        [ "Examples", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1364", null ]
      ] ],
      [ "task_monitor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1366", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1367", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1368", null ],
        [ "Queue Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1369", null ],
        [ "Worker Status", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1370", null ],
        [ "Task Queries", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1371", null ],
        [ "Task Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1372", null ],
        [ "Event Subscriptions", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1373", null ]
      ] ],
      [ "task_system", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1375", [
        [ "Configuration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1376", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1377", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1378", null ],
        [ "Component Access", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1379", null ],
        [ "Handler Registration (Convenience)", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1380", null ],
        [ "Task Submission (Convenience)", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1381", null ],
        [ "Scheduling (Convenience)", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1382", null ],
        [ "Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1383", null ]
      ] ],
      [ "Related Documentation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1385", null ]
    ] ],
    [ "Task 모듈 API 레퍼런스", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html", [
      [ "목차", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1387", null ],
      [ "task", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1389", [
        [ "열거형", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1390", [
          [ "task_state", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1391", null ]
        ] ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1392", null ],
        [ "식별 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1393", null ],
        [ "상태 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1394", null ],
        [ "설정 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1395", null ],
        [ "실행 추적", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1396", null ],
        [ "진행 상황 추적", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1397", null ],
        [ "결과/에러 처리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1398", null ],
        [ "재시도 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1399", null ],
        [ "직렬화", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1400", null ]
      ] ],
      [ "task_config", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1402", [
        [ "필드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1403", null ]
      ] ],
      [ "task_builder", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1405", [
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1406", null ],
        [ "빌더 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1407", null ],
        [ "빌드 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1408", null ],
        [ "예제", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1409", null ]
      ] ],
      [ "task_handler_interface", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1411", [
        [ "순수 가상 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1412", null ],
        [ "가상 라이프사이클 훅", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1413", null ],
        [ "예제", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1414", null ],
        [ "헬퍼 함수", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1415", null ]
      ] ],
      [ "task_context", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1417", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1418", [
          [ "progress_info", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1419", null ],
          [ "task_log_entry", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1420", null ]
        ] ],
        [ "진행 상황 추적", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1421", null ],
        [ "체크포인트 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1422", null ],
        [ "하위 작업 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1423", null ],
        [ "취소", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1424", null ],
        [ "로깅", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1425", null ]
      ] ],
      [ "task_queue", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1427", [
        [ "설정", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1428", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1429", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1430", null ],
        [ "인큐 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1431", null ],
        [ "디큐 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1432", null ],
        [ "취소", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1433", null ],
        [ "쿼리 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1434", null ]
      ] ],
      [ "worker_pool", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1436", [
        [ "설정", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1437", null ],
        [ "통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1438", null ],
        [ "핸들러 등록", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1439", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1440", null ],
        [ "상태", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1441", null ],
        [ "통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1442", null ]
      ] ],
      [ "task_client", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1444", [
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1445", null ],
        [ "즉시 실행", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1446", null ],
        [ "지연 실행", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1447", null ],
        [ "배치 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1448", null ],
        [ "워크플로우 패턴", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1449", null ],
        [ "결과/취소", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1450", null ],
        [ "상태", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1451", null ]
      ] ],
      [ "async_result", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1453", [
        [ "상태 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1454", null ],
        [ "진행 상황 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1455", null ],
        [ "블로킹 결과 조회", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1456", null ],
        [ "콜백 기반", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1457", null ],
        [ "작업 제어", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1458", null ]
      ] ],
      [ "result_backend_interface", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1460", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1461", null ],
        [ "순수 가상 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1462", null ],
        [ "선택적 가상 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1463", null ]
      ] ],
      [ "memory_result_backend", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1465", [
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1466", null ],
        [ "추가 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1467", null ]
      ] ],
      [ "task_scheduler", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1469", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1470", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1471", null ],
        [ "스케줄 등록", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1472", null ],
        [ "스케줄 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1473", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1474", null ],
        [ "쿼리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1475", null ],
        [ "이벤트 콜백", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1476", null ]
      ] ],
      [ "cron_parser", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1478", [
        [ "Cron 표현식 형식", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1479", null ],
        [ "지원 구문", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1480", null ],
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1481", null ],
        [ "정적 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1482", null ],
        [ "예제", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1483", null ]
      ] ],
      [ "task_monitor", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1485", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1486", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1487", null ],
        [ "큐 통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1488", null ],
        [ "워커 상태", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1489", null ],
        [ "작업 쿼리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1490", null ],
        [ "작업 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1491", null ],
        [ "이벤트 구독", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1492", null ]
      ] ],
      [ "task_system", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1494", [
        [ "설정", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1495", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1496", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1497", null ],
        [ "컴포넌트 접근", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1498", null ],
        [ "핸들러 등록 (편의 메서드)", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1499", null ],
        [ "작업 제출 (편의 메서드)", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1500", null ],
        [ "스케줄링 (편의 메서드)", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1501", null ],
        [ "통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1502", null ]
      ] ],
      [ "관련 문서", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1504", null ]
    ] ],
    [ "Task Module Architecture", "d1/d5e/md_docs_2task_2ARCHITECTURE.html", [
      [ "Overview", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1506", null ],
      [ "System Architecture", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1507", null ],
      [ "Core Components", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1508", [
        [ "1. task_system (Facade)", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1509", null ],
        [ "2. task", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1510", null ],
        [ "3. task_queue", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1511", null ],
        [ "4. worker_pool", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1512", null ],
        [ "5. task_handler", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1513", null ],
        [ "6. task_context", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1514", null ],
        [ "7. result_backend", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1515", null ],
        [ "8. async_result", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1516", null ],
        [ "9. task_scheduler", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1517", null ],
        [ "10. task_monitor", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1518", null ]
      ] ],
      [ "Data Flow", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1519", [
        [ "Task Submission Flow", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1520", null ],
        [ "Retry Flow", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1521", null ]
      ] ],
      [ "Thread Safety", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1522", [
        [ "thread_system Integration", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1523", null ]
      ] ],
      [ "Extension Points", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1524", [
        [ "Custom Result Backend", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1525", null ],
        [ "Custom Task Handler", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1526", null ]
      ] ],
      [ "Design Decisions", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1527", [
        [ "1. Message-Based Tasks", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1528", null ],
        [ "2. Result Type Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1529", null ],
        [ "3. Builder Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1530", null ],
        [ "4. Facade Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1531", null ],
        [ "5. Strategy Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1532", null ]
      ] ],
      [ "Testing", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1533", [
        [ "Running Tests", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1534", null ]
      ] ],
      [ "Related Documentation", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1535", null ]
    ] ],
    [ "Task 모듈 아키텍처", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html", [
      [ "개요", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1537", null ],
      [ "시스템 아키텍처", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1538", null ],
      [ "핵심 컴포넌트", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1539", [
        [ "1. task_system (파사드)", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1540", null ],
        [ "2. task", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1541", null ],
        [ "3. task_queue", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1542", null ],
        [ "4. worker_pool", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1543", null ],
        [ "5. task_handler", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1544", null ],
        [ "6. task_context", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1545", null ],
        [ "7. result_backend", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1546", null ],
        [ "8. async_result", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1547", null ],
        [ "9. task_scheduler", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1548", null ],
        [ "10. task_monitor", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1549", null ]
      ] ],
      [ "데이터 흐름", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1550", [
        [ "작업 제출 흐름", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1551", null ],
        [ "재시도 흐름", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1552", null ]
      ] ],
      [ "스레드 안전성", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1553", [
        [ "thread_system 통합", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1554", null ]
      ] ],
      [ "확장 포인트", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1555", [
        [ "커스텀 Result Backend", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1556", null ],
        [ "커스텀 Task Handler", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1557", null ]
      ] ],
      [ "설계 결정", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1558", [
        [ "1. 메시지 기반 작업", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1559", null ],
        [ "2. Result 타입 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1560", null ],
        [ "3. 빌더 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1561", null ],
        [ "4. 파사드 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1562", null ],
        [ "5. 전략 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1563", null ]
      ] ],
      [ "테스트", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1564", [
        [ "테스트 실행", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1565", null ]
      ] ],
      [ "관련 문서", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1566", null ]
    ] ],
    [ "Task Module Configuration Guide", "dc/d1e/md_docs_2task_2CONFIGURATION.html", [
      [ "Table of Contents", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1568", null ],
      [ "Task System Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1570", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1571", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1572", null ],
        [ "Example", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1573", null ]
      ] ],
      [ "Task Queue Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1575", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1576", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1577", null ],
        [ "Persistence Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1578", null ],
        [ "Delayed Queue Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1579", null ]
      ] ],
      [ "Worker Pool Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1581", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1582", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1583", null ],
        [ "Concurrency Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1584", null ],
        [ "Queue Priority", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1585", null ],
        [ "Prefetch Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1586", null ]
      ] ],
      [ "Task Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1588", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1589", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1590", null ],
        [ "Using Task Builder", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1591", null ],
        [ "Priority Levels", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1592", null ]
      ] ],
      [ "Environment-Specific Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1594", [
        [ "Development Environment", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1595", null ],
        [ "Staging Environment", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1596", null ],
        [ "Production Environment", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1597", null ]
      ] ],
      [ "Performance Tuning", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1599", [
        [ "High Throughput Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1600", null ],
        [ "Low Latency Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1601", null ],
        [ "Resource-Constrained Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1602", null ],
        [ "Long-Running Tasks Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1603", null ],
        [ "Memory Optimization", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1604", null ]
      ] ],
      [ "Configuration Recommendations", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1606", [
        [ "By Workload Type", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1607", null ],
        [ "By Scale", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1608", null ]
      ] ],
      [ "Related Documentation", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1610", null ]
    ] ],
    [ "Task 모듈 설정 가이드", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html", [
      [ "목차", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1612", null ],
      [ "Task 시스템 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1614", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1615", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1616", null ],
        [ "예제", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1617", null ]
      ] ],
      [ "Task 큐 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1619", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1620", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1621", null ],
        [ "영속성 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1622", null ],
        [ "지연 큐 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1623", null ]
      ] ],
      [ "워커 풀 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1625", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1626", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1627", null ],
        [ "동시성 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1628", null ],
        [ "큐 우선순위", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1629", null ],
        [ "프리페치 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1630", null ]
      ] ],
      [ "Task 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1632", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1633", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1634", null ],
        [ "Task 빌더 사용", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1635", null ],
        [ "우선순위 수준", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1636", null ]
      ] ],
      [ "환경별 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1638", [
        [ "개발 환경", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1639", null ],
        [ "스테이징 환경", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1640", null ],
        [ "프로덕션 환경", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1641", null ]
      ] ],
      [ "성능 튜닝", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1643", [
        [ "고처리량 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1644", null ],
        [ "저지연 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1645", null ],
        [ "리소스 제한 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1646", null ],
        [ "장기 실행 작업 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1647", null ],
        [ "메모리 최적화", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1648", null ]
      ] ],
      [ "설정 권장 사항", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1650", [
        [ "워크로드 유형별", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1651", null ],
        [ "규모별", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1652", null ]
      ] ],
      [ "관련 문서", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1654", null ]
    ] ],
    [ "Task Module Migration Guide", "d2/d8d/md_docs_2task_2MIGRATION.html", [
      [ "Table of Contents", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1656", null ],
      [ "Version Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1658", [
        [ "Migrating to 1.0", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1659", [
          [ "API Changes", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1660", null ],
          [ "Configuration Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1661", null ]
        ] ]
      ] ],
      [ "Migration from Custom Solutions", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1663", [
        [ "From Thread Pool Implementation", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1664", [
          [ "Before (Custom Thread Pool)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1665", null ],
          [ "After (Task Module)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1666", null ],
          [ "Benefits of Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1667", null ]
        ] ],
        [ "From Message Queue Systems", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1668", [
          [ "Mapping Concepts", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1669", null ],
          [ "Migration Example", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1670", null ]
        ] ],
        [ "From Cron Jobs", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1671", [
          [ "Before (System Cron)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1672", null ],
          [ "After (Task Scheduler)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1673", null ],
          [ "Benefits", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1674", null ]
        ] ]
      ] ],
      [ "Migration Checklist", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1676", [
        [ "Pre-Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1677", null ],
        [ "During Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1678", null ],
        [ "Post-Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1679", null ],
        [ "Testing Checklist", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1680", null ]
      ] ],
      [ "Common Migration Issues", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1682", [
        [ "Handler Throws Instead of Returning Error", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1683", null ],
        [ "Missing Queue Configuration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1684", null ],
        [ "Timeout Too Short", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1685", null ]
      ] ],
      [ "Related Documentation", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1687", null ]
    ] ],
    [ "Task 모듈 마이그레이션 가이드", "d7/d10/md_docs_2task_2MIGRATION__KO.html", [
      [ "목차", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1689", null ],
      [ "버전 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1691", [
        [ "1.0으로 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1692", [
          [ "API 변경 사항", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1693", null ],
          [ "설정 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1694", null ]
        ] ]
      ] ],
      [ "커스텀 솔루션에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1696", [
        [ "스레드 풀 구현에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1697", [
          [ "이전 (커스텀 스레드 풀)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1698", null ],
          [ "이후 (Task 모듈)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1699", null ],
          [ "마이그레이션의 이점", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1700", null ]
        ] ],
        [ "메시지 큐 시스템에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1701", [
          [ "개념 매핑", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1702", null ],
          [ "마이그레이션 예제", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1703", null ]
        ] ],
        [ "Cron 작업에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1704", [
          [ "이전 (시스템 Cron)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1705", null ],
          [ "이후 (Task 스케줄러)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1706", null ],
          [ "이점", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1707", null ]
        ] ]
      ] ],
      [ "마이그레이션 체크리스트", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1709", [
        [ "마이그레이션 전", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1710", null ],
        [ "마이그레이션 중", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1711", null ],
        [ "마이그레이션 후", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1712", null ],
        [ "테스트 체크리스트", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1713", null ]
      ] ],
      [ "일반적인 마이그레이션 문제", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1715", [
        [ "핸들러가 에러 반환 대신 예외를 던짐", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1716", null ],
        [ "누락된 큐 설정", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1717", null ],
        [ "타임아웃이 너무 짧음", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1718", null ]
      ] ],
      [ "관련 문서", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1720", null ]
    ] ],
    [ "Task Module Workflow Patterns", "d5/dd2/md_docs_2task_2PATTERNS.html", [
      [ "Table of Contents", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1722", null ],
      [ "Chain Pattern", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1724", [
        [ "Concept", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1725", null ],
        [ "Usage", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1726", null ],
        [ "Handler Implementation", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1727", null ],
        [ "Error Handling", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1728", null ],
        [ "Use Cases", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1729", null ]
      ] ],
      [ "Chord Pattern", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1731", [
        [ "Concept", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1732", null ],
        [ "Usage", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1733", null ],
        [ "Callback Handler", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1734", null ],
        [ "Error Handling", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1735", null ],
        [ "Use Cases", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1736", null ]
      ] ],
      [ "Retry Strategies", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1738", [
        [ "Default Configuration", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1739", null ],
        [ "Retry Delay Calculation", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1740", null ],
        [ "Using Task Builder", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1741", null ],
        [ "Custom Retry Logic", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1742", null ],
        [ "Retry vs. No Retry Errors", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1743", null ]
      ] ],
      [ "Priority Queues", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1745", [
        [ "Priority Levels", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1746", null ],
        [ "Setting Priority", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1747", null ],
        [ "Priority Queue Setup", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1748", null ],
        [ "Submitting to Priority Queues", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1749", null ],
        [ "Best Practices", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1750", null ]
      ] ],
      [ "Scheduled Tasks", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1752", [
        [ "Periodic Execution", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1753", null ],
        [ "Cron-Based Scheduling", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1754", null ],
        [ "Cron Expression Format", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1755", null ],
        [ "Managing Schedules", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1756", null ],
        [ "Delayed Task Execution", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1757", null ]
      ] ],
      [ "Progress Tracking", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1759", [
        [ "Reporting Progress", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1760", null ],
        [ "Monitoring Progress", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1761", null ],
        [ "Progress History", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1762", null ]
      ] ],
      [ "Checkpoint and Recovery", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1764", [
        [ "Saving Checkpoints", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1765", null ],
        [ "Checkpoint Data", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1766", null ]
      ] ],
      [ "Subtask Spawning", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1768", [
        [ "Spawning Subtasks", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1769", null ],
        [ "Tracking Subtasks", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1770", null ],
        [ "Use Cases", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1771", null ]
      ] ],
      [ "Combining Patterns", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1773", [
        [ "Chain with Retry", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1774", null ],
        [ "Scheduled Chord", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1775", null ]
      ] ],
      [ "Related Documentation", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1777", null ]
    ] ],
    [ "Task 모듈 워크플로우 패턴", "dd/d35/md_docs_2task_2PATTERNS__KO.html", [
      [ "목차", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1779", null ],
      [ "Chain 패턴", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1781", [
        [ "개념", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1782", null ],
        [ "사용법", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1783", null ],
        [ "핸들러 구현", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1784", null ],
        [ "에러 처리", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1785", null ],
        [ "사용 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1786", null ]
      ] ],
      [ "Chord 패턴", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1788", [
        [ "개념", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1789", null ],
        [ "사용법", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1790", null ],
        [ "콜백 핸들러", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1791", null ],
        [ "에러 처리", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1792", null ],
        [ "사용 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1793", null ]
      ] ],
      [ "재시도 전략", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1795", [
        [ "기본 설정", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1796", null ],
        [ "재시도 지연 계산", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1797", null ],
        [ "Task 빌더 사용", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1798", null ],
        [ "커스텀 재시도 로직", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1799", null ],
        [ "재시도 vs 재시도하지 않을 에러", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1800", null ]
      ] ],
      [ "우선순위 큐", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1802", [
        [ "우선순위 수준", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1803", null ],
        [ "우선순위 설정", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1804", null ],
        [ "우선순위 큐 설정", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1805", null ],
        [ "우선순위 큐에 제출", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1806", null ],
        [ "모범 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1807", null ]
      ] ],
      [ "예약 작업", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1809", [
        [ "주기적 실행", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1810", null ],
        [ "Cron 기반 스케줄링", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1811", null ],
        [ "Cron 표현식 형식", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1812", null ],
        [ "스케줄 관리", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1813", null ],
        [ "지연 작업 실행", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1814", null ]
      ] ],
      [ "진행 상황 추적", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1816", [
        [ "진행 상황 보고", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1817", null ],
        [ "진행 상황 모니터링", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1818", null ],
        [ "진행 기록", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1819", null ]
      ] ],
      [ "체크포인트와 복구", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1821", [
        [ "체크포인트 저장", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1822", null ],
        [ "체크포인트 데이터", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1823", null ]
      ] ],
      [ "하위 작업 생성", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1825", [
        [ "하위 작업 생성", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1826", null ],
        [ "하위 작업 추적", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1827", null ],
        [ "사용 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1828", null ]
      ] ],
      [ "패턴 조합", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1830", [
        [ "재시도가 있는 Chain", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1831", null ],
        [ "예약된 Chord", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1832", null ]
      ] ],
      [ "관련 문서", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1834", null ]
    ] ],
    [ "Task Module Quick Start Guide", "d9/dff/md_docs_2task_2QUICK__START.html", [
      [ "Prerequisites", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1836", null ],
      [ "Installation", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1837", null ],
      [ "Basic Usage", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1838", [
        [ "1. Include Headers", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1839", null ],
        [ "2. Create and Configure the Task System", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1840", null ],
        [ "3. Register a Task Handler", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1841", null ],
        [ "4. Start the System", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1842", null ],
        [ "5. Submit a Task", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1843", null ],
        [ "6. Stop the System", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1844", null ]
      ] ],
      [ "Complete Example", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1845", null ],
      [ "Using the Task Builder", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1846", null ],
      [ "Scheduling Tasks", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1847", [
        [ "Periodic Execution", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1848", null ],
        [ "Cron-based Execution", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1849", null ]
      ] ],
      [ "Monitoring Progress", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1850", null ],
      [ "Error Handling", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1851", null ],
      [ "Next Steps", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1852", null ]
    ] ],
    [ "Task 모듈 빠른 시작 가이드", "da/dee/md_docs_2task_2QUICK__START__KO.html", [
      [ "사전 요구사항", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1854", null ],
      [ "설치", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1855", null ],
      [ "기본 사용법", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1856", [
        [ "1. 헤더 포함", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1857", null ],
        [ "2. Task 시스템 생성 및 설정", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1858", null ],
        [ "3. Task 핸들러 등록", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1859", null ],
        [ "4. 시스템 시작", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1860", null ],
        [ "5. Task 제출", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1861", null ],
        [ "6. 시스템 중지", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1862", null ]
      ] ],
      [ "전체 예제", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1863", null ],
      [ "Task 빌더 사용하기", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1864", null ],
      [ "Task 스케줄링", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1865", [
        [ "주기적 실행", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1866", null ],
        [ "Cron 기반 실행", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1867", null ]
      ] ],
      [ "진행 상황 모니터링", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1868", null ],
      [ "에러 처리", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1869", null ],
      [ "다음 단계", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md1870", null ]
    ] ],
    [ "Task Module Troubleshooting Guide", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1872", null ],
      [ "Common Issues", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1874", [
        [ "Tasks Not Being Processed", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1875", null ],
        [ "Tasks Failing Immediately", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1876", null ],
        [ "Tasks Timing Out", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1877", null ],
        [ "Retry Not Working", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1878", null ],
        [ "Memory Usage Growing", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1879", null ]
      ] ],
      [ "Debugging Methods", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1881", [
        [ "Enable Logging", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1882", null ],
        [ "Monitor Events", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1883", null ],
        [ "Check Queue Status", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1884", null ],
        [ "Check Worker Status", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1885", null ],
        [ "View Task Logs", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1886", null ]
      ] ],
      [ "Performance Issues", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1888", [
        [ "High CPU Usage", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1889", null ],
        [ "High Memory Usage", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1890", null ],
        [ "Low Throughput", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1891", null ],
        [ "High Latency", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1892", null ]
      ] ],
      [ "FAQ", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1894", [
        [ "Q: How do I cancel a running task?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1895", null ],
        [ "Q: How do I get task progress from outside the handler?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1896", null ],
        [ "Q: How do I handle task dependencies?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1897", null ],
        [ "Q: How do I process tasks in order?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1898", null ],
        [ "Q: How do I prioritize certain tasks?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1899", null ],
        [ "Q: How do I limit retry attempts?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1900", null ],
        [ "Q: How do I handle poison messages (tasks that always fail)?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1901", null ],
        [ "Q: How do I gracefully shut down the system?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1902", null ]
      ] ],
      [ "Related Documentation", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md1904", null ]
    ] ],
    [ "Task 모듈 문제 해결 가이드", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html", [
      [ "목차", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1906", null ],
      [ "일반적인 문제", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1908", [
        [ "작업이 처리되지 않음", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1909", null ],
        [ "작업이 즉시 실패함", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1910", null ],
        [ "작업 타임아웃", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1911", null ],
        [ "재시도가 작동하지 않음", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1912", null ],
        [ "메모리 사용량 증가", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1913", null ]
      ] ],
      [ "디버깅 방법", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1915", [
        [ "로깅 활성화", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1916", null ],
        [ "이벤트 모니터링", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1917", null ],
        [ "큐 상태 확인", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1918", null ],
        [ "워커 상태 확인", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1919", null ],
        [ "작업 로그 보기", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1920", null ]
      ] ],
      [ "성능 문제", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1922", [
        [ "높은 CPU 사용량", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1923", null ],
        [ "높은 메모리 사용량", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1924", null ],
        [ "낮은 처리량", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1925", null ],
        [ "높은 지연 시간", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1926", null ]
      ] ],
      [ "FAQ", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1928", [
        [ "Q: 실행 중인 작업을 어떻게 취소하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1929", null ],
        [ "Q: 핸들러 외부에서 작업 진행 상황을 어떻게 얻나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1930", null ],
        [ "Q: 작업 종속성을 어떻게 처리하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1931", null ],
        [ "Q: 작업을 순서대로 처리하려면 어떻게 하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1932", null ],
        [ "Q: 특정 작업의 우선순위를 어떻게 높이나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1933", null ],
        [ "Q: 재시도 횟수를 어떻게 제한하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1934", null ],
        [ "Q: 독약 메시지(항상 실패하는 작업)를 어떻게 처리하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1935", null ],
        [ "Q: 시스템을 우아하게 종료하려면 어떻게 하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1936", null ]
      ] ],
      [ "관련 문서", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md1938", null ]
    ] ],
    [ "Distributed Task Queue System - Improvement Plan", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html", [
      [ "Executive Summary", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1940", null ],
      [ "Phase 1: Task Core (기반 확장)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1942", [
        [ "1.1 Task 정의 확장", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1943", null ],
        [ "1.2 Task Builder", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1944", null ]
      ] ],
      [ "Phase 2: Worker System", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1946", [
        [ "2.1 Task Handler Interface", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1947", null ],
        [ "2.2 Worker Pool", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1948", null ]
      ] ],
      [ "Phase 3: Task Queue (확장)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1950", [
        [ "3.1 Priority Task Queue", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1951", null ]
      ] ],
      [ "Phase 4: Result Backend", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1953", [
        [ "4.1 Result Backend Interface", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1954", null ],
        [ "4.2 In-Memory Result Backend", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1955", null ],
        [ "4.3 Redis Result Backend (선택적)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1956", null ]
      ] ],
      [ "Phase 5: Task Client (Producer)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1958", [
        [ "5.1 Task Client", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1959", null ]
      ] ],
      [ "Phase 6: Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1961", [
        [ "6.1 Periodic Task Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1962", null ]
      ] ],
      [ "Phase 7: Monitoring & Management", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1964", [
        [ "7.1 Task Monitor", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1965", null ]
      ] ],
      [ "Phase 8: Integration", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1967", [
        [ "8.1 통합 서비스 컨테이너", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1968", null ]
      ] ],
      [ "Implementation Roadmap", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1970", [
        [ "Sprint 1: Core Task Infrastructure ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1971", null ],
        [ "Sprint 2: Queue & Result Backend ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1972", null ],
        [ "Sprint 3: Worker System ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1973", null ],
        [ "Sprint 4: Client & Async Result ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1974", null ],
        [ "Sprint 5: Scheduler ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1975", null ],
        [ "Sprint 6: Monitoring & System Integration ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1976", null ],
        [ "Sprint 7: Testing & Documentation ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1977", null ]
      ] ],
      [ "File Structure", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1979", null ],
      [ "Usage Example", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1981", null ],
      [ "Reusing Existing Components", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1983", null ],
      [ "Key Design Decisions", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1985", [
        [ "1. Task는 Message를 상속", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1986", null ],
        [ "2. Result Backend 분리", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1987", null ],
        [ "3. 핸들러 등록 방식", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1988", null ],
        [ "4. 비동기 우선 설계", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1989", null ]
      ] ],
      [ "Version", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md1991", null ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"d1/d5e/md_docs_2task_2ARCHITECTURE.html",
"d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md365",
"d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md980",
"d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md1839",
"dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md886",
"dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1117",
"df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1347"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';