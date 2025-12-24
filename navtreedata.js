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
        [ "Namespace: <tt>kcenon::messaging</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md303", null ],
        [ "Enum: <tt>transport_mode</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md304", null ],
        [ "Class: <tt>message_bus</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md305", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md306", null ],
          [ "Configuration Structure", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md307", null ],
          [ "Lifecycle Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md308", null ],
          [ "Publishing", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md309", null ],
          [ "Subscription", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md310", null ],
          [ "Request-Response", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md311", null ],
          [ "Monitoring", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md312", null ],
          [ "Transport Accessors", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md313", null ],
          [ "Usage Example (Local Mode)", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md314", null ],
          [ "Usage Example (Hybrid Mode with Transport)", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md315", null ]
        ] ]
      ] ],
      [ "Message Broker API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md317", [
        [ "Namespace: <tt>kcenon::messaging</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md318", null ],
        [ "Overview", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md319", null ],
        [ "Struct: <tt>broker_config</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md320", null ],
        [ "Struct: <tt>broker_statistics</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md321", null ],
        [ "Struct: <tt>route_info</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md322", null ],
        [ "Class: <tt>message_broker</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md323", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md324", null ],
          [ "Lifecycle Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md325", null ],
          [ "Route Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md326", null ],
          [ "Message Routing", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md327", null ],
          [ "Statistics", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md328", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md329", null ],
          [ "Topic Pattern Wildcards", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md330", null ]
        ] ]
      ] ],
      [ "Service Container API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md332", [
        [ "Namespace: <tt>kcenon::messaging::services</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md333", null ],
        [ "Class: <tt>service_container</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md334", [
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md335", null ],
          [ "Service Lifetimes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md336", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md337", null ]
        ] ]
      ] ],
      [ "Thread System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md339", [
        [ "Namespace: <tt>thread_system</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md340", null ],
        [ "Class: <tt>thread_pool</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md341", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md342", null ],
          [ "Job Submission", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md343", null ],
          [ "Pool Management", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md344", null ],
          [ "Priority Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md345", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md346", null ]
        ] ],
        [ "Class: <tt>lock_free_queue</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md347", [
          [ "Constructor", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md348", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md349", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md350", null ]
        ] ]
      ] ],
      [ "Logger System API", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md352", [
        [ "Namespace: <tt>logger</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md353", null ],
        [ "Class: <tt>logger_manager</tt>", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md354", [
          [ "Configuration", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md355", null ],
          [ "Log Levels", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md356", null ],
          [ "Methods", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md357", null ],
          [ "Usage Example", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md358", null ]
        ] ]
      ] ],
      [ "Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md360", [
        [ "System Error Codes", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md361", null ],
        [ "Error Handling", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md362", null ],
        [ "Error Recovery Strategies", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md363", null ]
      ] ],
      [ "Configuration Reference", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md364", [
        [ "System Configuration File Format", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md365", null ],
        [ "Environment Variables", "db/d91/md_docs_2API__REFERENCE.html#autotoc_md366", null ]
      ] ]
    ] ],
    [ "Messaging System API 레퍼런스", "d4/d54/md_docs_2API__REFERENCE__KO.html", [
      [ "개요", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md369", null ],
      [ "목차", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md371", null ],
      [ "핵심 클래스", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md373", [
        [ "message_bus", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md374", null ]
      ] ],
      [ "메시지 버스", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md376", [
        [ "사용 예제", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md377", null ]
      ] ],
      [ "토픽 라우터", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md379", [
        [ "와일드카드 패턴", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md380", null ]
      ] ],
      [ "메시징 패턴", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md382", [
        [ "Pub/Sub", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md383", null ],
        [ "Request/Reply", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md384", null ]
      ] ],
      [ "직렬화", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md386", [
        [ "메시지 구조", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md387", null ]
      ] ],
      [ "관련 문서", "d4/d54/md_docs_2API__REFERENCE__KO.html#autotoc_md389", null ]
    ] ],
    [ "Architecture", "d2/d64/md_docs_2architecture.html", [
      [ "Overview", "d2/d64/md_docs_2architecture.html#autotoc_md391", null ],
      [ "Related Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md392", [
        [ "Component-Specific Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md393", null ],
        [ "Module Documentation", "d2/d64/md_docs_2architecture.html#autotoc_md394", null ],
        [ "Integration Guides", "d2/d64/md_docs_2architecture.html#autotoc_md395", null ]
      ] ],
      [ "Architectural Principles", "d2/d64/md_docs_2architecture.html#autotoc_md396", null ],
      [ "System Layers", "d2/d64/md_docs_2architecture.html#autotoc_md397", null ],
      [ "Component Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md398", [
        [ "1. Container System", "d2/d64/md_docs_2architecture.html#autotoc_md399", null ],
        [ "2. Network System", "d2/d64/md_docs_2architecture.html#autotoc_md400", null ],
        [ "3. Database System", "d2/d64/md_docs_2architecture.html#autotoc_md401", null ],
        [ "4. Thread System", "d2/d64/md_docs_2architecture.html#autotoc_md402", null ]
      ] ],
      [ "Data Flow Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md403", [
        [ "Message Processing Pipeline", "d2/d64/md_docs_2architecture.html#autotoc_md404", null ],
        [ "Request-Response Flow", "d2/d64/md_docs_2architecture.html#autotoc_md405", null ],
        [ "Distributed Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md406", null ]
      ] ],
      [ "Scalability Patterns", "d2/d64/md_docs_2architecture.html#autotoc_md407", [
        [ "Horizontal Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md408", null ],
        [ "Vertical Scaling", "d2/d64/md_docs_2architecture.html#autotoc_md409", null ]
      ] ],
      [ "Integration Points", "d2/d64/md_docs_2architecture.html#autotoc_md410", [
        [ "External Systems", "d2/d64/md_docs_2architecture.html#autotoc_md411", null ],
        [ "Internal Communication", "d2/d64/md_docs_2architecture.html#autotoc_md412", null ]
      ] ],
      [ "Performance Optimization", "d2/d64/md_docs_2architecture.html#autotoc_md413", [
        [ "Memory Management", "d2/d64/md_docs_2architecture.html#autotoc_md414", null ],
        [ "Concurrency", "d2/d64/md_docs_2architecture.html#autotoc_md415", null ],
        [ "Network", "d2/d64/md_docs_2architecture.html#autotoc_md416", null ]
      ] ],
      [ "Fault Tolerance", "d2/d64/md_docs_2architecture.html#autotoc_md417", [
        [ "Error Recovery", "d2/d64/md_docs_2architecture.html#autotoc_md418", null ],
        [ "High Availability", "d2/d64/md_docs_2architecture.html#autotoc_md419", null ]
      ] ],
      [ "Security Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md420", [
        [ "Transport Security", "d2/d64/md_docs_2architecture.html#autotoc_md421", null ],
        [ "Application Security", "d2/d64/md_docs_2architecture.html#autotoc_md422", null ]
      ] ],
      [ "Monitoring and Observability", "d2/d64/md_docs_2architecture.html#autotoc_md423", [
        [ "Metrics Collection", "d2/d64/md_docs_2architecture.html#autotoc_md424", null ],
        [ "Distributed Tracing", "d2/d64/md_docs_2architecture.html#autotoc_md425", null ],
        [ "Logging", "d2/d64/md_docs_2architecture.html#autotoc_md426", null ]
      ] ],
      [ "Configuration Management", "d2/d64/md_docs_2architecture.html#autotoc_md427", [
        [ "Static Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md428", null ],
        [ "Dynamic Configuration", "d2/d64/md_docs_2architecture.html#autotoc_md429", null ]
      ] ],
      [ "Deployment Architecture", "d2/d64/md_docs_2architecture.html#autotoc_md430", [
        [ "Container Deployment", "d2/d64/md_docs_2architecture.html#autotoc_md431", null ],
        [ "Kubernetes Integration", "d2/d64/md_docs_2architecture.html#autotoc_md432", null ]
      ] ],
      [ "Future Considerations", "d2/d64/md_docs_2architecture.html#autotoc_md433", [
        [ "Planned Enhancements", "d2/d64/md_docs_2architecture.html#autotoc_md434", null ],
        [ "Technology Evaluation", "d2/d64/md_docs_2architecture.html#autotoc_md435", null ]
      ] ]
    ] ],
    [ "Messaging System 아키텍처", "de/de3/md_docs_2ARCHITECTURE__KO.html", [
      [ "개요", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md438", null ],
      [ "목차", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md440", null ],
      [ "시스템 개요", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md442", null ],
      [ "컴포넌트 아키텍처", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md444", [
        [ "핵심 컴포넌트", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md445", null ],
        [ "메시징 패턴", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md446", null ]
      ] ],
      [ "의존성 구조", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md448", [
        [ "버전 매트릭스", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md449", null ]
      ] ],
      [ "데이터 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md451", [
        [ "메시지 발행 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md452", null ],
        [ "Request/Reply 흐름", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md453", null ]
      ] ],
      [ "설계 원칙", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md455", [
        [ "1. 느슨한 결합", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md456", null ],
        [ "2. 비동기 우선", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md457", null ],
        [ "3. 스레드 안전", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md458", null ],
        [ "4. 확장성", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md459", null ]
      ] ],
      [ "관련 문서", "de/de3/md_docs_2ARCHITECTURE__KO.html#autotoc_md461", null ]
    ] ],
    [ "Messaging System Performance Benchmarks", "de/d93/md_docs_2BENCHMARKS.html", [
      [ "Executive Summary", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md464", null ],
      [ "Table of Contents", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md466", null ],
      [ "Core Performance Metrics", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md468", [
        [ "Reference Performance Metrics", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md469", null ]
      ] ],
      [ "Message Creation Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md471", [
        [ "Message Builder Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md472", null ]
      ] ],
      [ "Queue Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md474", [
        [ "Priority Queue Operations", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md475", null ],
        [ "Queue Contention Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md476", null ]
      ] ],
      [ "Topic Router Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md478", [
        [ "Pattern Matching Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md479", null ],
        [ "Subscription Count Impact", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md480", null ]
      ] ],
      [ "Pub/Sub Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md482", [
        [ "End-to-End Throughput", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md483", null ],
        [ "Backend Impact", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md484", null ]
      ] ],
      [ "Request/Reply Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md486", [
        [ "Synchronous RPC Latency", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md487", null ],
        [ "Concurrent Request Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md488", null ]
      ] ],
      [ "Task Module Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md490", [
        [ "Task Queue Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md491", null ],
        [ "Worker Throughput", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md492", null ],
        [ "Worker Scalability", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md493", null ],
        [ "Result Backend Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md494", null ],
        [ "Scheduler Performance", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md495", null ],
        [ "Performance Targets Summary", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md496", null ]
      ] ],
      [ "Memory Usage", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md498", [
        [ "Per-Component Memory Overhead", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md499", null ],
        [ "Memory Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md500", null ]
      ] ],
      [ "Latency Analysis", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md502", [
        [ "Latency Distribution (Pub/Sub)", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md503", null ],
        [ "Latency Breakdown", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md504", null ]
      ] ],
      [ "Scalability", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md506", [
        [ "Thread Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md507", null ],
        [ "Publisher/Subscriber Scaling", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md508", null ]
      ] ],
      [ "Optimization Insights", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md510", [
        [ "Performance Tuning Recommendations", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md511", null ],
        [ "Common Bottlenecks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md512", null ]
      ] ],
      [ "Running Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md514", [
        [ "Prerequisites", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md515", null ],
        [ "Available Benchmarks", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md516", null ],
        [ "Benchmark Output Format", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md517", null ]
      ] ],
      [ "Continuous Performance Monitoring", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md519", null ],
      [ "Comparison with Other Systems", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md521", [
        [ "Feature Comparison", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md522", null ]
      ] ],
      [ "Conclusion", "de/d93/md_docs_2BENCHMARKS.html#autotoc_md524", null ]
    ] ],
    [ "Messaging System 성능 벤치마크", "d5/ddb/md_docs_2BENCHMARKS__KO.html", [
      [ "요약", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md528", null ],
      [ "목차", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md530", null ],
      [ "핵심 성능 메트릭", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md532", [
        [ "요약 테이블", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md533", null ]
      ] ],
      [ "메시지 버스 벤치마크", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md535", [
        [ "처리량", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md536", null ],
        [ "토픽 라우팅 성능", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md537", null ]
      ] ],
      [ "패턴별 성능", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md539", [
        [ "Pub/Sub", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md540", null ],
        [ "Request/Reply", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md541", null ]
      ] ],
      [ "확장성", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md543", [
        [ "수평 확장", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md544", null ]
      ] ],
      [ "벤치마크 방법론", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md546", null ],
      [ "관련 문서", "d5/ddb/md_docs_2BENCHMARKS__KO.html#autotoc_md548", null ]
    ] ],
    [ "Messaging System Changelog", "da/dee/md_docs_2CHANGELOG.html", [
      [ "[Unreleased]", "da/dee/md_docs_2CHANGELOG.html#autotoc_md550", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md551", null ],
        [ "Fixed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md552", null ],
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md553", null ],
        [ "Fixed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md554", null ],
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md555", null ],
        [ "Fixed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md556", null ],
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md557", null ],
        [ "Fixed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md558", null ],
        [ "Changed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md559", null ],
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md560", null ]
      ] ],
      [ "[1.0.0] - 2025-11-17", "da/dee/md_docs_2CHANGELOG.html#autotoc_md562", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md563", null ],
        [ "Dependencies", "da/dee/md_docs_2CHANGELOG.html#autotoc_md564", null ]
      ] ],
      [ "[0.9.0] - 2025-10-20", "da/dee/md_docs_2CHANGELOG.html#autotoc_md566", [
        [ "Added", "da/dee/md_docs_2CHANGELOG.html#autotoc_md567", null ],
        [ "Changed", "da/dee/md_docs_2CHANGELOG.html#autotoc_md568", null ]
      ] ]
    ] ],
    [ "Messaging System 변경 이력", "db/d9f/md_docs_2CHANGELOG__KO.html", [
      [ "[Unreleased]", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md571", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md572", null ],
        [ "수정됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md573", null ],
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md574", null ],
        [ "수정됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md575", null ],
        [ "변경됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md576", null ],
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md577", null ]
      ] ],
      [ "[1.0.0] - 2025-11-17", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md579", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md580", null ],
        [ "의존성", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md581", null ]
      ] ],
      [ "[0.9.0] - 2025-10-20", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md583", [
        [ "추가됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md584", null ],
        [ "변경됨", "db/d9f/md_docs_2CHANGELOG__KO.html#autotoc_md585", null ]
      ] ]
    ] ],
    [ "Contributing to Messaging System", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html", [
      [ "Table of Contents", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md588", null ],
      [ "Code of Conduct", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md590", null ],
      [ "Getting Started", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md592", [
        [ "Prerequisites", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md593", null ],
        [ "Building for Development", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md594", null ]
      ] ],
      [ "Development Workflow", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md596", null ],
      [ "Coding Standards", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md598", null ],
      [ "Testing Requirements", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md600", null ],
      [ "Pull Request Process", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md602", [
        [ "PR Title Format", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md603", null ]
      ] ],
      [ "Questions?", "d7/d4b/md_docs_2contributing_2CONTRIBUTING.html#autotoc_md605", null ]
    ] ],
    [ "Message Broker", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html", [
      [ "Overview", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md608", null ],
      [ "Table of Contents", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md610", null ],
      [ "Features", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md612", [
        [ "Implemented", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md613", null ],
        [ "Planned", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md614", null ],
        [ "Recently Added", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md615", null ]
      ] ],
      [ "Architecture", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md617", [
        [ "Component Diagram", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md618", null ],
        [ "Class Structure", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md619", null ]
      ] ],
      [ "Quick Start", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md621", [
        [ "Basic Usage", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md622", null ],
        [ "With Custom Configuration", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md623", null ]
      ] ],
      [ "Configuration", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md625", [
        [ "broker_config", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md626", null ],
        [ "Example Configuration", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md627", null ]
      ] ],
      [ "Route Management", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md629", [
        [ "Adding Routes", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md630", null ],
        [ "Removing Routes", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md631", null ],
        [ "Enabling/Disabling Routes", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md632", null ],
        [ "Querying Routes", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md633", null ]
      ] ],
      [ "Message Routing", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md635", [
        [ "How Routing Works", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md636", null ],
        [ "Topic Pattern Matching", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md637", null ],
        [ "Routing Example", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md638", null ],
        [ "Error Handling", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md639", null ]
      ] ],
      [ "Content-Based Routing", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md641", [
        [ "Content Filter Types", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md642", null ],
        [ "Adding Content Routes", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md643", null ],
        [ "Combining Filters", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md644", null ],
        [ "Custom Content Filters", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md645", null ],
        [ "Managing Content Routes", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md646", null ],
        [ "Routing by Content", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md647", null ],
        [ "Content vs Topic Routing", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md648", null ],
        [ "Best Practices", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md649", null ]
      ] ],
      [ "Statistics", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md651", [
        [ "Available Statistics", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md652", null ],
        [ "Using Statistics", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md653", null ]
      ] ],
      [ "Best Practices", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md655", [
        [ "1. Use Meaningful Route IDs", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md656", null ],
        [ "2. Set Appropriate Priorities", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md657", null ],
        [ "3. Handle Errors in Handlers", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md658", null ],
        [ "4. Monitor Statistics", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md659", null ],
        [ "5. Graceful Shutdown", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md660", null ]
      ] ],
      [ "Migration from topic_router", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md662", [
        [ "Before (topic_router)", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md663", null ],
        [ "After (message_broker)", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md664", null ],
        [ "Key Differences", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md665", null ]
      ] ],
      [ "Dead Letter Queue", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md667", [
        [ "DLQ Configuration", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md668", null ],
        [ "DLQ Configuration Options", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md669", null ],
        [ "Overflow Policies", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md670", null ],
        [ "Moving Messages to DLQ", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md671", null ],
        [ "Querying DLQ", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md672", null ],
        [ "Replaying Messages", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md673", null ],
        [ "Purging DLQ", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md674", null ],
        [ "DLQ Statistics", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md675", null ],
        [ "DLQ Event Callbacks", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md676", null ]
      ] ],
      [ "Planned Features", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md678", [
        [ "Transformation Pipeline (#183)", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md679", null ]
      ] ],
      [ "See Also", "d8/dc1/md_docs_2core_2MESSAGE__BROKER.html#autotoc_md681", null ]
    ] ],
    [ "Messaging System Features", "da/db6/md_docs_2FEATURES.html", [
      [ "Overview", "da/db6/md_docs_2FEATURES.html#autotoc_md685", null ],
      [ "Table of Contents", "da/db6/md_docs_2FEATURES.html#autotoc_md687", null ],
      [ "Core Messaging", "da/db6/md_docs_2FEATURES.html#autotoc_md689", [
        [ "Message Bus", "da/db6/md_docs_2FEATURES.html#autotoc_md690", null ],
        [ "Message Broker", "da/db6/md_docs_2FEATURES.html#autotoc_md691", [
          [ "Transformation Pipeline", "da/db6/md_docs_2FEATURES.html#autotoc_md692", null ]
        ] ]
      ] ],
      [ "Messaging Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md694", [
        [ "Pub/Sub Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md695", null ],
        [ "Request/Reply Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md696", null ],
        [ "Event Streaming", "da/db6/md_docs_2FEATURES.html#autotoc_md697", null ],
        [ "Message Pipeline", "da/db6/md_docs_2FEATURES.html#autotoc_md698", null ]
      ] ],
      [ "Task Queue System", "da/db6/md_docs_2FEATURES.html#autotoc_md700", [
        [ "Overview", "da/db6/md_docs_2FEATURES.html#autotoc_md701", null ],
        [ "Task System Facade", "da/db6/md_docs_2FEATURES.html#autotoc_md702", null ],
        [ "Task Client", "da/db6/md_docs_2FEATURES.html#autotoc_md703", null ],
        [ "Worker Pool", "da/db6/md_docs_2FEATURES.html#autotoc_md704", null ],
        [ "Task Scheduler", "da/db6/md_docs_2FEATURES.html#autotoc_md705", null ],
        [ "Async Result", "da/db6/md_docs_2FEATURES.html#autotoc_md706", null ],
        [ "Result Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md707", null ],
        [ "Task Monitor", "da/db6/md_docs_2FEATURES.html#autotoc_md708", null ],
        [ "Chain and Chord Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md709", null ],
        [ "Retry Mechanism", "da/db6/md_docs_2FEATURES.html#autotoc_md710", null ],
        [ "Task Timeout", "da/db6/md_docs_2FEATURES.html#autotoc_md711", null ]
      ] ],
      [ "C++20 Concepts", "da/db6/md_docs_2FEATURES.html#autotoc_md713", [
        [ "Overview", "da/db6/md_docs_2FEATURES.html#autotoc_md714", null ],
        [ "TaskHandlerCallable", "da/db6/md_docs_2FEATURES.html#autotoc_md715", null ],
        [ "TaskHandlerLike", "da/db6/md_docs_2FEATURES.html#autotoc_md716", null ],
        [ "ScheduleEventCallable", "da/db6/md_docs_2FEATURES.html#autotoc_md717", null ],
        [ "MessageProcessorCallable", "da/db6/md_docs_2FEATURES.html#autotoc_md718", null ],
        [ "MessageFilterCallable", "da/db6/md_docs_2FEATURES.html#autotoc_md719", null ],
        [ "MessageTransformerCallable", "da/db6/md_docs_2FEATURES.html#autotoc_md720", null ],
        [ "SubscriptionCallable", "da/db6/md_docs_2FEATURES.html#autotoc_md721", null ],
        [ "Benefits", "da/db6/md_docs_2FEATURES.html#autotoc_md722", null ]
      ] ],
      [ "Backend Support", "da/db6/md_docs_2FEATURES.html#autotoc_md724", [
        [ "Backend Interface", "da/db6/md_docs_2FEATURES.html#autotoc_md725", null ],
        [ "Executor Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md726", null ],
        [ "Standalone Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md727", null ],
        [ "Integration Backend", "da/db6/md_docs_2FEATURES.html#autotoc_md728", null ],
        [ "Auto-Detection", "da/db6/md_docs_2FEATURES.html#autotoc_md729", null ]
      ] ],
      [ "Transport Adapters", "da/db6/md_docs_2FEATURES.html#autotoc_md731", [
        [ "Overview", "da/db6/md_docs_2FEATURES.html#autotoc_md732", null ],
        [ "Transport Interface", "da/db6/md_docs_2FEATURES.html#autotoc_md733", null ],
        [ "WebSocket Transport", "da/db6/md_docs_2FEATURES.html#autotoc_md734", null ],
        [ "Configuration Options", "da/db6/md_docs_2FEATURES.html#autotoc_md735", null ],
        [ "Build Configuration", "da/db6/md_docs_2FEATURES.html#autotoc_md736", null ],
        [ "HTTP Transport", "da/db6/md_docs_2FEATURES.html#autotoc_md737", null ],
        [ "HTTP Configuration Options", "da/db6/md_docs_2FEATURES.html#autotoc_md738", null ]
      ] ],
      [ "Message Types", "da/db6/md_docs_2FEATURES.html#autotoc_md740", [
        [ "Message Structure", "da/db6/md_docs_2FEATURES.html#autotoc_md741", null ],
        [ "Message Builder", "da/db6/md_docs_2FEATURES.html#autotoc_md742", null ],
        [ "Message Serialization", "da/db6/md_docs_2FEATURES.html#autotoc_md743", null ]
      ] ],
      [ "Topic Routing", "da/db6/md_docs_2FEATURES.html#autotoc_md745", [
        [ "Wildcard Patterns", "da/db6/md_docs_2FEATURES.html#autotoc_md746", null ],
        [ "Subscription Management", "da/db6/md_docs_2FEATURES.html#autotoc_md747", null ]
      ] ],
      [ "Message Queue", "da/db6/md_docs_2FEATURES.html#autotoc_md749", [
        [ "Queue Types", "da/db6/md_docs_2FEATURES.html#autotoc_md750", null ],
        [ "Queue Configuration", "da/db6/md_docs_2FEATURES.html#autotoc_md751", null ]
      ] ],
      [ "Dependency Injection", "da/db6/md_docs_2FEATURES.html#autotoc_md753", [
        [ "DI Container", "da/db6/md_docs_2FEATURES.html#autotoc_md754", null ]
      ] ],
      [ "Error Handling", "da/db6/md_docs_2FEATURES.html#autotoc_md756", [
        [ "Error Codes", "da/db6/md_docs_2FEATURES.html#autotoc_md757", null ],
        [ "Result<T> Pattern", "da/db6/md_docs_2FEATURES.html#autotoc_md758", null ]
      ] ],
      [ "Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md760", [
        [ "Thread System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md761", null ],
        [ "Logger System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md762", null ],
        [ "Monitoring System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md763", null ],
        [ "Container System Integration", "da/db6/md_docs_2FEATURES.html#autotoc_md764", null ]
      ] ],
      [ "Production Features", "da/db6/md_docs_2FEATURES.html#autotoc_md766", [
        [ "Reliability", "da/db6/md_docs_2FEATURES.html#autotoc_md767", null ],
        [ "Observability", "da/db6/md_docs_2FEATURES.html#autotoc_md768", null ],
        [ "Testing Support", "da/db6/md_docs_2FEATURES.html#autotoc_md769", null ]
      ] ],
      [ "Feature Matrix", "da/db6/md_docs_2FEATURES.html#autotoc_md771", null ],
      [ "Getting Started", "da/db6/md_docs_2FEATURES.html#autotoc_md773", null ]
    ] ],
    [ "Messaging System 기능", "d4/d0c/md_docs_2FEATURES__KO.html", [
      [ "개요", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md777", null ],
      [ "목차", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md779", null ],
      [ "핵심 메시징", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md781", [
        [ "메시지 버스", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md782", null ],
        [ "토픽 라우터", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md783", null ],
        [ "메시지 브로커", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md784", null ],
        [ "메시지 큐", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md785", null ],
        [ "메시지 직렬화", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md786", null ]
      ] ],
      [ "고급 패턴", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md788", [
        [ "Pub/Sub", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md789", null ],
        [ "Request/Reply", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md790", null ],
        [ "Event Streaming", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md791", null ],
        [ "Message Pipeline", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md792", null ]
      ] ],
      [ "태스크 큐 시스템", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md794", [
        [ "개요", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md795", null ],
        [ "태스크 시스템 파사드", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md796", null ],
        [ "태스크 클라이언트", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md797", null ],
        [ "워커 풀", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md798", null ],
        [ "태스크 스케줄러", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md799", null ],
        [ "비동기 결과", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md800", null ],
        [ "결과 백엔드", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md801", null ],
        [ "태스크 모니터", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md802", null ],
        [ "Chain 및 Chord 패턴", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md803", null ],
        [ "재시도 메커니즘", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md804", null ],
        [ "태스크 타임아웃", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md805", null ]
      ] ],
      [ "C++20 Concepts", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md807", [
        [ "개요", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md808", null ],
        [ "TaskHandlerCallable", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md809", null ],
        [ "TaskHandlerLike", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md810", null ],
        [ "ScheduleEventCallable", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md811", null ],
        [ "MessageProcessorCallable", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md812", null ],
        [ "MessageFilterCallable", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md813", null ],
        [ "MessageTransformerCallable", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md814", null ],
        [ "SubscriptionCallable", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md815", null ],
        [ "장점", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md816", null ]
      ] ],
      [ "백엔드 지원", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md818", [
        [ "독립 실행", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md819", null ],
        [ "스레드 풀 통합", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md820", null ],
        [ "런타임 백엔드 선택", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md821", null ]
      ] ],
      [ "프로덕션 기능", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md823", [
        [ "스레드 안전", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md824", null ],
        [ "타입 안전", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md825", null ],
        [ "테스트", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md826", null ]
      ] ],
      [ "기능 매트릭스", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md828", null ],
      [ "시작하기", "d4/d0c/md_docs_2FEATURES__KO.html#autotoc_md830", null ]
    ] ],
    [ "Build Troubleshooting Guide", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html", [
      [ "Common Build Issues and Solutions", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md833", [
        [ "Issue 1: Target Name Conflicts", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md834", null ],
        [ "Issue 2: GTest Not Found in External Systems", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md836", null ],
        [ "Issue 3: grep -P Not Supported (macOS)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md838", null ],
        [ "Issue 4: yaml-cpp Not Found", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md840", null ]
      ] ],
      [ "Recommended Build Workflow", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md842", [
        [ "For Development (FetchContent Mode)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md843", null ],
        [ "For Production (find_package Mode)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md844", null ]
      ] ],
      [ "Alternative: Minimal Build (No External Systems)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md846", null ],
      [ "Verifying Successful Build", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md848", null ],
      [ "Getting Help", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md850", null ],
      [ "Known Limitations", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md852", null ],
      [ "Platform-Specific Notes", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md854", [
        [ "macOS", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md855", null ],
        [ "Linux (Ubuntu/Debian)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md856", null ],
        [ "Linux (Fedora/RHEL)", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md857", null ]
      ] ],
      [ "Quick Fix Summary", "d6/d3f/md_docs_2guides_2BUILD__TROUBLESHOOTING.html#autotoc_md859", null ]
    ] ],
    [ "Deployment Guide", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html", [
      [ "System Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md862", [
        [ "Hardware Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md863", [
          [ "Minimum Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md864", null ],
          [ "Recommended Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md865", null ],
          [ "Production Requirements", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md866", null ]
        ] ],
        [ "Operating System Support", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md867", null ],
        [ "Software Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md868", [
          [ "Build Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md869", null ],
          [ "Runtime Dependencies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md870", null ]
        ] ]
      ] ],
      [ "Installation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md871", [
        [ "From Source", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md872", [
          [ "1. Clone Repository", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md873", null ],
          [ "2. Configure Build", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md874", null ],
          [ "3. Build and Install", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md875", null ]
        ] ],
        [ "Using Package Managers", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md876", [
          [ "APT Repository (Ubuntu/Debian)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md877", null ],
          [ "YUM Repository (RHEL/CentOS)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md878", null ]
        ] ],
        [ "Docker Installation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md879", [
          [ "Pull Official Image", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md880", null ],
          [ "Build Custom Image", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md881", null ]
        ] ]
      ] ],
      [ "Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md882", [
        [ "Configuration File Structure", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md883", null ],
        [ "Environment Variables", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md884", null ],
        [ "Secrets Management", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md885", [
          [ "Using HashiCorp Vault", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md886", null ],
          [ "Using Kubernetes Secrets", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md887", null ]
        ] ]
      ] ],
      [ "Deployment Scenarios", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md888", [
        [ "Single Server Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md889", null ],
        [ "High Availability Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md890", [
          [ "Architecture", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md891", null ],
          [ "HAProxy Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md892", null ]
        ] ],
        [ "Kubernetes Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md893", [
          [ "Namespace and ConfigMap", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md894", null ],
          [ "Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md895", null ],
          [ "Service and Ingress", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md896", null ],
          [ "Horizontal Pod Autoscaler", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md897", null ]
        ] ],
        [ "Docker Compose Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md898", null ]
      ] ],
      [ "Performance Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md899", [
        [ "System Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md900", [
          [ "Linux Kernel Parameters", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md901", null ],
          [ "ulimit Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md902", null ]
        ] ],
        [ "Application Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md903", [
          [ "Thread Pool Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md904", null ],
          [ "Memory Management", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md905", null ],
          [ "Network Optimization", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md906", null ]
        ] ],
        [ "Database Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md907", [
          [ "PostgreSQL Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md908", null ],
          [ "Connection Pool Tuning", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md909", null ]
        ] ]
      ] ],
      [ "Monitoring Setup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md910", [
        [ "Prometheus Integration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md911", [
          [ "Prometheus Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md912", null ],
          [ "Metrics Exposed", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md913", null ]
        ] ],
        [ "Grafana Dashboard", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md914", null ],
        [ "Logging Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md915", [
          [ "Structured Logging", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md916", null ],
          [ "Log Aggregation (ELK Stack)", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md917", null ]
        ] ]
      ] ],
      [ "Scaling Strategies", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md918", [
        [ "Horizontal Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md919", [
          [ "Load Balancer Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md920", null ],
          [ "Auto-scaling Rules", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md921", null ]
        ] ],
        [ "Vertical Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md922", [
          [ "Resource Allocation", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md923", null ]
        ] ],
        [ "Database Scaling", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md924", [
          [ "Read Replicas", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md925", null ],
          [ "Sharding Strategy", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md926", null ]
        ] ]
      ] ],
      [ "Backup and Recovery", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md927", [
        [ "Backup Strategy", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md928", [
          [ "Application State Backup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md929", null ],
          [ "Database Backup", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md930", null ]
        ] ],
        [ "Recovery Procedures", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md931", [
          [ "Service Recovery", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md932", null ],
          [ "Disaster Recovery Plan", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md933", null ]
        ] ]
      ] ],
      [ "Security Hardening", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md934", [
        [ "Network Security", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md935", [
          [ "Firewall Rules", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md936", null ],
          [ "SSL/TLS Configuration", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md937", null ]
        ] ],
        [ "Application Security", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md938", [
          [ "Authentication", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md939", null ],
          [ "Rate Limiting", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md940", null ]
        ] ],
        [ "Compliance", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md941", [
          [ "Audit Logging", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md942", null ],
          [ "Data Encryption", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md943", null ]
        ] ]
      ] ],
      [ "Troubleshooting Deployment", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md944", [
        [ "Common Issues", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md945", [
          [ "Service Won't Start", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md946", null ],
          [ "High Memory Usage", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md947", null ],
          [ "Performance Issues", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md948", null ]
        ] ],
        [ "Health Checks", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md949", [
          [ "Application Health", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md950", null ],
          [ "System Health", "d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md951", null ]
        ] ]
      ] ]
    ] ],
    [ "Developer Guide", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html", [
      [ "Quick Start", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md953", [
        [ "Prerequisites", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md954", null ],
        [ "1. Clone and Setup", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md955", null ],
        [ "2. Build the Project", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md956", null ],
        [ "3. Your First Application", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md957", null ]
      ] ],
      [ "Development Setup", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md958", [
        [ "IDE Configuration", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md959", [
          [ "Visual Studio Code", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md960", null ],
          [ "CLion", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md961", null ],
          [ "Visual Studio", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md962", null ]
        ] ],
        [ "Development Dependencies", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md963", null ]
      ] ],
      [ "Project Structure", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md964", [
        [ "Directory Layout", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md965", null ],
        [ "Module Organization", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md966", null ]
      ] ],
      [ "Coding Standards", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md967", [
        [ "C++ Style Guide", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md968", [
          [ "Naming Conventions", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md969", null ],
          [ "File Organization", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md970", null ],
          [ "Best Practices", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md971", null ]
        ] ],
        [ "Documentation Standards", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md972", [
          [ "Code Documentation", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md973", null ],
          [ "Comment Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md974", null ]
        ] ]
      ] ],
      [ "Testing Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md975", [
        [ "Unit Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md976", [
          [ "Test Structure", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md977", null ],
          [ "Test Coverage", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md978", null ]
        ] ],
        [ "Integration Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md979", null ],
        [ "Performance Testing", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md980", null ]
      ] ],
      [ "Debugging", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md981", [
        [ "Using GDB", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md982", null ],
        [ "Using Valgrind", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md983", null ],
        [ "Using AddressSanitizer", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md984", null ],
        [ "Using ThreadSanitizer", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md985", null ]
      ] ],
      [ "Performance Profiling", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md986", [
        [ "Using perf", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md987", null ],
        [ "Using Instruments (macOS)", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md988", null ],
        [ "Using Intel VTune", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md989", null ]
      ] ],
      [ "Contributing Guidelines", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md990", [
        [ "Workflow", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md991", null ],
        [ "Commit Message Format", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md992", null ],
        [ "Code Review Checklist", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md993", null ]
      ] ],
      [ "Build System", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md994", [
        [ "CMake Configuration", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md995", null ],
        [ "Creating New Modules", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md996", null ]
      ] ],
      [ "Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md997", [
        [ "Docker Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md998", null ],
        [ "Kubernetes Deployment", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md999", null ]
      ] ],
      [ "Security Best Practices", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1000", [
        [ "Input Validation", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1001", null ],
        [ "Secure Communication", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1002", null ],
        [ "Rate Limiting", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1003", null ]
      ] ],
      [ "Troubleshooting Common Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1004", [
        [ "Build Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1005", null ],
        [ "Runtime Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1006", null ],
        [ "Performance Issues", "da/d7c/md_docs_2guides_2DEVELOPER__GUIDE.html#autotoc_md1007", null ]
      ] ]
    ] ],
    [ "Frequently Asked Questions", "d4/da1/md_docs_2guides_2FAQ.html", [
      [ "General Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1010", [
        [ "Q: What is messaging_system?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1011", null ],
        [ "Q: What C++ standard is required?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1012", null ],
        [ "Q: Which platforms are supported?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1013", null ]
      ] ],
      [ "Installation & Setup", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1015", [
        [ "Q: How do I install the library?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1016", null ],
        [ "Q: What dependencies are required?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1017", null ]
      ] ],
      [ "Usage Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1019", [
        [ "Q: How do I create a message bus?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1020", null ],
        [ "Q: How do I subscribe to topics?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1021", null ],
        [ "Q: How do I use wildcards?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1022", null ]
      ] ],
      [ "Performance Questions", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1024", [
        [ "Q: What throughput can I expect?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1025", null ],
        [ "Q: What is the latency?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1026", null ]
      ] ],
      [ "More Questions?", "d4/da1/md_docs_2guides_2FAQ.html#autotoc_md1028", null ]
    ] ],
    [ "Migration Guide", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html", [
      [ "Table of Contents", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1030", null ],
      [ "Overview", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1032", [
        [ "Migration Timeline", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1033", null ],
        [ "Compatibility", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1034", null ]
      ] ],
      [ "Breaking Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1036", [
        [ "1. Namespace Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1037", null ],
        [ "2. Error Code Range", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1038", null ],
        [ "3. Message Structure", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1039", null ],
        [ "4. Message Bus Interface", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1040", null ],
        [ "5. Topic Router", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1041", null ]
      ] ],
      [ "Step-by-Step Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1043", [
        [ "Step 1: Update Dependencies", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1044", [
          [ "Update CMakeLists.txt", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1045", null ],
          [ "Update Include Paths", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1046", null ]
        ] ],
        [ "Step 2: Update Includes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1047", null ],
        [ "Step 3: Migrate Message Creation", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1048", null ],
        [ "Step 4: Migrate Message Bus Usage", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1049", null ],
        [ "Step 5: Migrate to Patterns (Optional but Recommended)", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1050", [
          [ "Pub/Sub Pattern", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1051", null ],
          [ "Request-Reply Pattern", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1052", null ]
        ] ],
        [ "Step 6: Update Error Handling", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1053", null ],
        [ "Step 7: Integration with Other Systems", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1054", null ]
      ] ],
      [ "API Changes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1056", [
        [ "Message API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1057", null ],
        [ "Message Bus API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1058", null ],
        [ "Topic Router API", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1059", null ]
      ] ],
      [ "Code Examples", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1061", [
        [ "Example 1: Simple Pub/Sub Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1062", null ],
        [ "Example 2: Request-Reply Migration", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1063", null ],
        [ "Example 3: Event Streaming", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1064", null ],
        [ "Example 4: Message Pipeline", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1065", null ]
      ] ],
      [ "Troubleshooting", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1067", [
        [ "Issue: Compilation errors with old includes", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1068", null ],
        [ "Issue: Error code conflicts", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1069", null ],
        [ "Issue: Message bus doesn't start", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1070", null ],
        [ "Issue: Messages not being delivered", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1071", null ],
        [ "Issue: Build performance degradation", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1072", null ]
      ] ],
      [ "FAQ", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1074", [
        [ "Q: Can I use v1.x and v2.0 together?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1075", null ],
        [ "Q: Do I need to migrate all code at once?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1076", null ],
        [ "Q: What about performance?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1077", null ],
        [ "Q: Are there any runtime dependencies?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1078", null ],
        [ "Q: How do I enable lock-free queues?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1079", null ],
        [ "Q: Can I still use simple publish/subscribe?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1080", null ],
        [ "Q: How do I handle migration testing?", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1081", null ]
      ] ],
      [ "Additional Resources", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1083", null ],
      [ "Support", "dc/d15/md_docs_2guides_2MIGRATION__GUIDE.html#autotoc_md1085", null ]
    ] ],
    [ "Getting Started", "d6/d63/md_docs_2guides_2QUICK__START.html", [
      [ "Table of Contents", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1088", null ],
      [ "Prerequisites", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1089", [
        [ "System Requirements", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1090", null ],
        [ "Development Dependencies", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1091", null ],
        [ "Runtime Dependencies", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1092", null ]
      ] ],
      [ "Installation", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1093", [
        [ "1. Clone the Repository", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1094", null ],
        [ "2. Platform-Specific Setup", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1095", [
          [ "Linux (Ubuntu/Debian)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1096", null ],
          [ "macOS", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1097", null ],
          [ "Windows", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1098", null ]
        ] ],
        [ "3. Build the System", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1099", [
          [ "Quick Build (Recommended)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1100", null ],
          [ "Custom Build Options", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1101", null ]
        ] ],
        [ "4. Verify Installation", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1102", null ]
      ] ],
      [ "Quick Start", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1103", [
        [ "1. Basic Message Bus Usage", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1104", null ],
        [ "2. Container-Based Data Handling", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1105", null ],
        [ "3. Network Client/Server", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1106", null ]
      ] ],
      [ "Basic Usage", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1107", [
        [ "Project Structure", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1108", null ],
        [ "Environment Configuration", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1109", null ],
        [ "Basic Configuration File", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1110", null ]
      ] ],
      [ "First Application", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1111", [
        [ "1. Chat Server (chat_server.cpp)", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1112", null ],
        [ "2. Build and Run", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1113", null ],
        [ "3. Test with Sample Client", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1114", null ]
      ] ],
      [ "Next Steps", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1115", [
        [ "1. Explore Sample Applications", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1116", null ],
        [ "2. Advanced Features", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1117", null ],
        [ "3. Development", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1118", null ],
        [ "4. Community", "d6/d63/md_docs_2guides_2QUICK__START.html#autotoc_md1119", null ]
      ] ]
    ] ],
    [ "Troubleshooting Guide", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1122", null ],
      [ "FAQ", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1123", [
        [ "General Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1124", null ],
        [ "Performance Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1125", null ],
        [ "Configuration Questions", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1126", null ]
      ] ],
      [ "Debug Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1127", [
        [ "Enable Debug Logging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1128", [
          [ "Runtime Configuration", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1129", null ],
          [ "Environment Variables", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1130", null ],
          [ "Programmatic", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1131", null ]
        ] ],
        [ "Using GDB for Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1132", [
          [ "Attach to Running Process", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1133", null ],
          [ "Debug Core Dumps", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1134", null ]
        ] ],
        [ "Memory Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1135", [
          [ "Using Valgrind", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1136", null ],
          [ "Using AddressSanitizer", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1137", null ],
          [ "Using HeapTrack", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1138", null ]
        ] ],
        [ "Network Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1139", [
          [ "TCP Dump", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1140", null ],
          [ "Network Statistics", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1141", null ],
          [ "Test Connectivity", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1142", null ]
        ] ],
        [ "Thread Debugging", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1143", [
          [ "Thread Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1144", null ],
          [ "Detect Deadlocks", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1145", null ],
          [ "ThreadSanitizer", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1146", null ]
        ] ],
        [ "Tracing and Profiling", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1147", [
          [ "System Tracing with strace", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1148", null ],
          [ "Performance Profiling with perf", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1149", null ],
          [ "Application Tracing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1150", null ]
        ] ]
      ] ],
      [ "Performance Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1151", [
        [ "CPU Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1152", [
          [ "Identify CPU Bottlenecks", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1153", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1154", null ]
        ] ],
        [ "Memory Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1155", [
          [ "Memory Usage Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1156", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1157", null ]
        ] ],
        [ "I/O Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1158", [
          [ "Disk I/O Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1159", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1160", null ]
        ] ],
        [ "Network Optimization", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1161", [
          [ "Network Analysis", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1162", null ],
          [ "Optimization Techniques", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1163", null ]
        ] ]
      ] ],
      [ "Known Issues", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1164", [
        [ "Issue 1: High CPU Usage with Small Messages", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1165", null ],
        [ "Issue 2: Memory Leak in Long-Running Connections", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1166", null ],
        [ "Issue 3: Database Connection Pool Exhaustion", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1167", null ],
        [ "Issue 4: Deadlock in Message Processing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1168", null ],
        [ "Issue 5: Performance Degradation with Many Topics", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1169", null ]
      ] ],
      [ "Error Messages", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1170", [
        [ "Connection Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1171", [
          [ "\"Connection refused\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1172", null ],
          [ "\"Connection timeout\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1173", null ]
        ] ],
        [ "Database Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1174", [
          [ "\"Database connection failed\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1175", null ],
          [ "\"Deadlock detected\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1176", null ]
        ] ],
        [ "Memory Errors", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1177", [
          [ "\"Out of memory\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1178", null ],
          [ "\"Segmentation fault\"", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1179", null ]
        ] ]
      ] ],
      [ "Common Problems", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1180", [
        [ "Problem: Service Won't Start", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1181", null ],
        [ "Problem: Slow Message Processing", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1182", null ],
        [ "Problem: Connection Drops", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1183", null ],
        [ "Problem: High Memory Usage", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1184", null ],
        [ "Problem: Database Bottleneck", "d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1185", null ]
      ] ]
    ] ],
    [ "Performance Baseline", "d9/df1/md_docs_2performance_2BASELINE.html", [
      [ "Overview", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1232", null ],
      [ "Baseline Metrics", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1234", [
        [ "Message Bus Performance", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1235", null ],
        [ "Pattern Performance", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1236", null ]
      ] ],
      [ "Test Environment", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1238", [
        [ "Reference Platform", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1239", null ]
      ] ],
      [ "Benchmark Commands", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1241", null ],
      [ "Regression Detection", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1243", null ],
      [ "Historical Data", "d9/df1/md_docs_2performance_2BASELINE.html#autotoc_md1245", null ]
    ] ],
    [ "Performance Guide", "d9/dd2/md_docs_2performance_2PERFORMANCE.html", [
      [ "Table of Contents", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1247", null ],
      [ "Performance Overview", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1248", [
        [ "Design Goals", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1249", null ],
        [ "Key Performance Features", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1250", null ]
      ] ],
      [ "Benchmark Results", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1251", [
        [ "Test Environment", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1252", null ],
        [ "Overall System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1253", null ],
        [ "Latency Measurements", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1254", null ],
        [ "Memory Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1255", null ]
      ] ],
      [ "Component Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1256", [
        [ "Thread System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1257", [
          [ "Lock-free vs Mutex Comparison", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1258", null ],
          [ "Scaling Characteristics", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1259", null ]
        ] ],
        [ "Container System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1260", [
          [ "Serialization Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1261", null ],
          [ "SIMD Optimization Impact", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1262", null ]
        ] ],
        [ "Network System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1263", [
          [ "Connection Scaling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1264", null ],
          [ "Protocol Overhead", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1265", null ]
        ] ],
        [ "Database System Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1266", [
          [ "Query Performance", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1267", null ],
          [ "Connection Pool Impact", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1268", null ]
        ] ]
      ] ],
      [ "Optimization Techniques", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1269", [
        [ "1. Memory Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1270", [
          [ "Object Pooling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1271", null ],
          [ "Custom Allocators", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1272", null ]
        ] ],
        [ "2. CPU Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1273", [
          [ "SIMD Utilization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1274", null ],
          [ "Cache Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1275", null ]
        ] ],
        [ "3. Network Optimization", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1276", [
          [ "Batching and Pipelining", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1277", null ]
        ] ]
      ] ],
      [ "Performance Monitoring", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1278", [
        [ "1. Built-in Metrics", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1279", null ],
        [ "2. Performance Profiling", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1280", null ]
      ] ],
      [ "Tuning Guidelines", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1281", [
        [ "1. Thread Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1282", null ],
        [ "2. Memory Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1283", null ],
        [ "3. Network Configuration", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1284", null ]
      ] ],
      [ "Troubleshooting Performance Issues", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1285", [
        [ "1. Common Performance Problems", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1286", [
          [ "High CPU Usage", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1287", null ],
          [ "Memory Leaks", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1288", null ],
          [ "Network Bottlenecks", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1289", null ]
        ] ],
        [ "2. Performance Monitoring Dashboard", "d9/dd2/md_docs_2performance_2PERFORMANCE.html#autotoc_md1290", null ]
      ] ]
    ] ],
    [ "Messaging System Production Quality", "dd/def/md_docs_2PRODUCTION__QUALITY.html", [
      [ "Overview", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1294", null ],
      [ "Quality Metrics", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1296", [
        [ "Test Coverage", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1297", null ],
        [ "CI/CD Pipeline", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1298", null ]
      ] ],
      [ "Code Quality", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1300", [
        [ "Static Analysis", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1301", null ],
        [ "Memory Safety", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1302", null ],
        [ "Code Reviews", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1303", null ]
      ] ],
      [ "Testing Strategy", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1305", [
        [ "Unit Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1306", null ],
        [ "Integration Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1307", null ],
        [ "Performance Tests", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1308", null ]
      ] ],
      [ "Reliability Features", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1310", [
        [ "Error Handling", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1311", null ],
        [ "Fault Tolerance", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1312", null ],
        [ "Monitoring", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1313", null ]
      ] ],
      [ "Production Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1315", [
        [ "Configuration", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1316", null ],
        [ "Resource Requirements", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1317", null ],
        [ "Scalability", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1318", null ]
      ] ],
      [ "Security", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1320", [
        [ "Input Validation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1321", null ],
        [ "Thread Safety", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1322", null ],
        [ "Resource Management", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1323", null ]
      ] ],
      [ "Documentation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1325", [
        [ "API Documentation", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1326", null ],
        [ "Guides", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1327", null ]
      ] ],
      [ "Industry Standards", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1329", [
        [ "Compliance", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1330", null ],
        [ "Best Practices", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1331", null ]
      ] ],
      [ "Known Limitations", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1333", [
        [ "Current Limitations", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1334", null ],
        [ "Planned Improvements", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1335", null ]
      ] ],
      [ "Production Checklist", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1337", [
        [ "Pre-Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1338", null ],
        [ "Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1339", null ],
        [ "Post-Deployment", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1340", null ]
      ] ],
      [ "Support", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1342", [
        [ "Getting Help", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1343", null ],
        [ "Reporting Bugs", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1344", null ]
      ] ],
      [ "Conclusion", "dd/def/md_docs_2PRODUCTION__QUALITY.html#autotoc_md1346", null ]
    ] ],
    [ "Messaging System 프로덕션 품질", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html", [
      [ "요약", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1350", null ],
      [ "목차", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1352", null ],
      [ "CI/CD 인프라", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1354", [
        [ "빌드 매트릭스", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1355", null ]
      ] ],
      [ "스레드 안전성 검증", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1357", [
        [ "ThreadSanitizer 결과", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1358", null ]
      ] ],
      [ "테스트 커버리지", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1360", [
        [ "컴포넌트별 커버리지", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1361", null ]
      ] ],
      [ "플랫폼 지원", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1363", [
        [ "지원 플랫폼", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1364", null ],
        [ "컴파일러 지원", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1365", null ]
      ] ],
      [ "관련 문서", "d2/d74/md_docs_2PRODUCTION__QUALITY__KO.html#autotoc_md1367", null ]
    ] ],
    [ "Messaging System Project Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html", [
      [ "Overview", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1370", null ],
      [ "Directory Layout", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1372", null ],
      [ "Component Organization", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1374", [
        [ "Core Components (<tt>include/kcenon/messaging/core/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1375", null ],
        [ "Interfaces (<tt>include/kcenon/messaging/interfaces/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1376", null ],
        [ "Backends (<tt>include/kcenon/messaging/backends/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1377", null ],
        [ "Patterns (<tt>include/kcenon/messaging/patterns/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1378", null ],
        [ "Task Queue (<tt>include/kcenon/messaging/task/</tt>)", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1379", null ]
      ] ],
      [ "Build System", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1381", [
        [ "CMake Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1382", null ],
        [ "Build Targets", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1383", null ]
      ] ],
      [ "Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1385", [
        [ "Required Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1386", null ],
        [ "Optional Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1387", null ]
      ] ],
      [ "Include Patterns", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1389", [
        [ "Application Code", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1390", null ],
        [ "Internal Implementation", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1391", null ]
      ] ],
      [ "Naming Conventions", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1393", [
        [ "Files", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1394", null ],
        [ "Classes", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1395", null ],
        [ "Namespaces", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1396", null ]
      ] ],
      [ "Code Organization Principles", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1398", [
        [ "Separation of Concerns", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1399", null ],
        [ "Modularity", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1400", null ],
        [ "Dependencies", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1401", null ]
      ] ],
      [ "Future Structure", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1403", [
        [ "Planned Additions", "d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1404", null ]
      ] ]
    ] ],
    [ "Messaging System 프로젝트 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html", [
      [ "개요", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1408", null ],
      [ "디렉토리 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1410", null ],
      [ "컴포넌트 구성", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1412", [
        [ "핵심 컴포넌트 (<tt>include/kcenon/messaging/core/</tt>)", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1413", null ],
        [ "인터페이스 (<tt>include/kcenon/messaging/interfaces/</tt>)", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1414", null ],
        [ "백엔드 (<tt>include/kcenon/messaging/backends/</tt>)", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1415", null ],
        [ "패턴 (<tt>include/kcenon/messaging/patterns/</tt>)", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1416", null ],
        [ "태스크 큐 (<tt>include/kcenon/messaging/task/</tt>)", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1417", null ]
      ] ],
      [ "빌드 시스템", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1419", [
        [ "CMake 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1420", null ],
        [ "빌드 타겟", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1421", null ]
      ] ],
      [ "의존성", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1423", [
        [ "필수 의존성", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1424", null ],
        [ "선택적 의존성", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1425", null ]
      ] ],
      [ "Include 패턴", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1427", [
        [ "애플리케이션 코드", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1428", null ],
        [ "내부 구현", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1429", null ]
      ] ],
      [ "명명 규칙", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1431", [
        [ "파일", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1432", null ],
        [ "클래스", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1433", null ],
        [ "네임스페이스", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1434", null ]
      ] ],
      [ "코드 구성 원칙", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1436", [
        [ "관심사 분리", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1437", null ],
        [ "모듈성", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1438", null ],
        [ "의존성", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1439", null ]
      ] ],
      [ "향후 구조", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1441", [
        [ "계획된 추가 사항", "d5/d19/md_docs_2PROJECT__STRUCTURE__KO.html#autotoc_md1442", null ]
      ] ]
    ] ],
    [ "Messaging System 문서", "d8/d0a/md_docs_2README__KO.html", [
      [ "문서 인덱스", "d8/d0a/md_docs_2README__KO.html#autotoc_md1465", null ],
      [ "시작하기", "d8/d0a/md_docs_2README__KO.html#autotoc_md1467", null ],
      [ "핵심 문서", "d8/d0a/md_docs_2README__KO.html#autotoc_md1469", null ],
      [ "품질 및 운영", "d8/d0a/md_docs_2README__KO.html#autotoc_md1471", null ],
      [ "Task 큐 시스템", "d8/d0a/md_docs_2README__KO.html#autotoc_md1473", null ],
      [ "아키텍처 결정 기록 (ADR)", "d8/d0a/md_docs_2README__KO.html#autotoc_md1475", null ],
      [ "디렉토리 구조", "d8/d0a/md_docs_2README__KO.html#autotoc_md1477", null ],
      [ "관련 링크", "d8/d0a/md_docs_2README__KO.html#autotoc_md1479", null ]
    ] ],
    [ "Task Module API Reference", "df/d18/md_docs_2task_2API__REFERENCE.html", [
      [ "Table of Contents", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1481", null ],
      [ "task", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1483", [
        [ "Enumerations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1484", [
          [ "task_state", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1485", null ]
        ] ],
        [ "Constructors", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1486", null ],
        [ "Identification Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1487", null ],
        [ "State Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1488", null ],
        [ "Configuration Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1489", null ],
        [ "Execution Tracking", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1490", null ],
        [ "Progress Tracking", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1491", null ],
        [ "Result/Error Handling", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1492", null ],
        [ "Retry Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1493", null ],
        [ "Serialization", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1494", null ]
      ] ],
      [ "task_config", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1496", [
        [ "Fields", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1497", null ]
      ] ],
      [ "task_builder", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1499", [
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1500", null ],
        [ "Builder Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1501", null ],
        [ "Build Method", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1502", null ],
        [ "Example", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1503", null ]
      ] ],
      [ "task_handler_interface", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1505", [
        [ "Pure Virtual Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1506", null ],
        [ "Virtual Lifecycle Hooks", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1507", null ],
        [ "Example", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1508", null ],
        [ "Helper Functions", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1509", null ]
      ] ],
      [ "task_context", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1511", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1512", [
          [ "progress_info", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1513", null ],
          [ "task_log_entry", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1514", null ]
        ] ],
        [ "Progress Tracking", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1515", null ],
        [ "Checkpoint Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1516", null ],
        [ "Subtask Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1517", null ],
        [ "Cancellation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1518", null ],
        [ "Logging", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1519", null ]
      ] ],
      [ "task_queue", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1521", [
        [ "Configuration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1522", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1523", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1524", null ],
        [ "Enqueue Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1525", null ],
        [ "Dequeue Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1526", null ],
        [ "Cancellation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1527", null ],
        [ "Query Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1528", null ]
      ] ],
      [ "worker_pool", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1530", [
        [ "Configuration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1531", null ],
        [ "Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1532", null ],
        [ "Handler Registration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1533", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1534", null ],
        [ "Status", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1535", null ],
        [ "Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1536", null ]
      ] ],
      [ "task_client", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1538", [
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1539", null ],
        [ "Immediate Execution", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1540", null ],
        [ "Delayed Execution", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1541", null ],
        [ "Batch Operations", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1542", null ],
        [ "Workflow Patterns", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1543", null ],
        [ "Result/Cancellation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1544", null ],
        [ "Status", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1545", null ]
      ] ],
      [ "async_result", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1547", [
        [ "Status Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1548", null ],
        [ "Progress Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1549", null ],
        [ "Blocking Result Retrieval", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1550", null ],
        [ "Callback-Based", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1551", null ],
        [ "Task Control", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1552", null ]
      ] ],
      [ "result_backend_interface", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1554", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1555", null ],
        [ "Pure Virtual Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1556", null ],
        [ "Optional Virtual Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1557", null ]
      ] ],
      [ "memory_result_backend", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1559", [
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1560", null ],
        [ "Additional Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1561", null ]
      ] ],
      [ "task_scheduler", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1563", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1564", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1565", null ],
        [ "Schedule Registration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1566", null ],
        [ "Schedule Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1567", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1568", null ],
        [ "Query", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1569", null ],
        [ "Event Callbacks", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1570", null ]
      ] ],
      [ "cron_parser", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1572", [
        [ "Cron Expression Format", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1573", null ],
        [ "Supported Syntax", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1574", null ],
        [ "Structure", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1575", null ],
        [ "Static Methods", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1576", null ],
        [ "Examples", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1577", null ]
      ] ],
      [ "task_monitor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1579", [
        [ "Structures", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1580", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1581", null ],
        [ "Queue Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1582", null ],
        [ "Worker Status", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1583", null ],
        [ "Task Queries", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1584", null ],
        [ "Task Management", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1585", null ],
        [ "Event Subscriptions", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1586", null ]
      ] ],
      [ "task_system", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1588", [
        [ "Configuration", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1589", null ],
        [ "Constructor", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1590", null ],
        [ "Lifecycle", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1591", null ],
        [ "Component Access", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1592", null ],
        [ "Handler Registration (Convenience)", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1593", null ],
        [ "Task Submission (Convenience)", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1594", null ],
        [ "Scheduling (Convenience)", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1595", null ],
        [ "Statistics", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1596", null ]
      ] ],
      [ "Related Documentation", "df/d18/md_docs_2task_2API__REFERENCE.html#autotoc_md1598", null ]
    ] ],
    [ "Task 모듈 API 레퍼런스", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html", [
      [ "목차", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1600", null ],
      [ "task", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1602", [
        [ "열거형", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1603", [
          [ "task_state", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1604", null ]
        ] ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1605", null ],
        [ "식별 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1606", null ],
        [ "상태 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1607", null ],
        [ "설정 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1608", null ],
        [ "실행 추적", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1609", null ],
        [ "진행 상황 추적", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1610", null ],
        [ "결과/에러 처리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1611", null ],
        [ "재시도 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1612", null ],
        [ "직렬화", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1613", null ]
      ] ],
      [ "task_config", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1615", [
        [ "필드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1616", null ]
      ] ],
      [ "task_builder", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1618", [
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1619", null ],
        [ "빌더 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1620", null ],
        [ "빌드 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1621", null ],
        [ "예제", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1622", null ]
      ] ],
      [ "task_handler_interface", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1624", [
        [ "순수 가상 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1625", null ],
        [ "가상 라이프사이클 훅", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1626", null ],
        [ "예제", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1627", null ],
        [ "헬퍼 함수", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1628", null ]
      ] ],
      [ "task_context", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1630", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1631", [
          [ "progress_info", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1632", null ],
          [ "task_log_entry", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1633", null ]
        ] ],
        [ "진행 상황 추적", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1634", null ],
        [ "체크포인트 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1635", null ],
        [ "하위 작업 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1636", null ],
        [ "취소", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1637", null ],
        [ "로깅", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1638", null ]
      ] ],
      [ "task_queue", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1640", [
        [ "설정", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1641", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1642", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1643", null ],
        [ "인큐 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1644", null ],
        [ "디큐 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1645", null ],
        [ "취소", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1646", null ],
        [ "쿼리 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1647", null ]
      ] ],
      [ "worker_pool", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1649", [
        [ "설정", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1650", null ],
        [ "통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1651", null ],
        [ "핸들러 등록", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1652", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1653", null ],
        [ "상태", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1654", null ],
        [ "통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1655", null ]
      ] ],
      [ "task_client", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1657", [
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1658", null ],
        [ "즉시 실행", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1659", null ],
        [ "지연 실행", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1660", null ],
        [ "배치 작업", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1661", null ],
        [ "워크플로우 패턴", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1662", null ],
        [ "결과/취소", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1663", null ],
        [ "상태", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1664", null ]
      ] ],
      [ "async_result", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1666", [
        [ "상태 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1667", null ],
        [ "진행 상황 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1668", null ],
        [ "블로킹 결과 조회", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1669", null ],
        [ "콜백 기반", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1670", null ],
        [ "작업 제어", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1671", null ]
      ] ],
      [ "result_backend_interface", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1673", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1674", null ],
        [ "순수 가상 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1675", null ],
        [ "선택적 가상 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1676", null ]
      ] ],
      [ "memory_result_backend", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1678", [
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1679", null ],
        [ "추가 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1680", null ]
      ] ],
      [ "task_scheduler", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1682", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1683", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1684", null ],
        [ "스케줄 등록", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1685", null ],
        [ "스케줄 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1686", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1687", null ],
        [ "쿼리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1688", null ],
        [ "이벤트 콜백", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1689", null ]
      ] ],
      [ "cron_parser", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1691", [
        [ "Cron 표현식 형식", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1692", null ],
        [ "지원 구문", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1693", null ],
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1694", null ],
        [ "정적 메서드", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1695", null ],
        [ "예제", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1696", null ]
      ] ],
      [ "task_monitor", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1698", [
        [ "구조체", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1699", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1700", null ],
        [ "큐 통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1701", null ],
        [ "워커 상태", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1702", null ],
        [ "작업 쿼리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1703", null ],
        [ "작업 관리", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1704", null ],
        [ "이벤트 구독", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1705", null ]
      ] ],
      [ "task_system", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1707", [
        [ "설정", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1708", null ],
        [ "생성자", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1709", null ],
        [ "라이프사이클", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1710", null ],
        [ "컴포넌트 접근", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1711", null ],
        [ "핸들러 등록 (편의 메서드)", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1712", null ],
        [ "작업 제출 (편의 메서드)", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1713", null ],
        [ "스케줄링 (편의 메서드)", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1714", null ],
        [ "통계", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1715", null ]
      ] ],
      [ "관련 문서", "dd/db9/md_docs_2task_2API__REFERENCE__KO.html#autotoc_md1717", null ]
    ] ],
    [ "Task Module Architecture", "d1/d5e/md_docs_2task_2ARCHITECTURE.html", [
      [ "Overview", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1719", null ],
      [ "System Architecture", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1720", null ],
      [ "Core Components", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1721", [
        [ "1. task_system (Facade)", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1722", null ],
        [ "2. task", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1723", null ],
        [ "3. task_queue", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1724", null ],
        [ "4. worker_pool", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1725", null ],
        [ "5. task_handler", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1726", null ],
        [ "6. task_context", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1727", null ],
        [ "7. result_backend", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1728", null ],
        [ "8. async_result", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1729", null ],
        [ "9. task_scheduler", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1730", null ],
        [ "10. task_monitor", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1731", null ]
      ] ],
      [ "Data Flow", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1732", [
        [ "Task Submission Flow", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1733", null ],
        [ "Retry Flow", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1734", null ]
      ] ],
      [ "Thread Safety", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1735", [
        [ "thread_system Integration", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1736", null ]
      ] ],
      [ "Extension Points", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1737", [
        [ "Custom Result Backend", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1738", null ],
        [ "Custom Task Handler", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1739", null ]
      ] ],
      [ "Design Decisions", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1740", [
        [ "1. Message-Based Tasks", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1741", null ],
        [ "2. Result Type Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1742", null ],
        [ "3. Builder Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1743", null ],
        [ "4. Facade Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1744", null ],
        [ "5. Strategy Pattern", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1745", null ]
      ] ],
      [ "Testing", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1746", [
        [ "Running Tests", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1747", null ]
      ] ],
      [ "Related Documentation", "d1/d5e/md_docs_2task_2ARCHITECTURE.html#autotoc_md1748", null ]
    ] ],
    [ "Task 모듈 아키텍처", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html", [
      [ "개요", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1750", null ],
      [ "시스템 아키텍처", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1751", null ],
      [ "핵심 컴포넌트", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1752", [
        [ "1. task_system (파사드)", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1753", null ],
        [ "2. task", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1754", null ],
        [ "3. task_queue", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1755", null ],
        [ "4. worker_pool", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1756", null ],
        [ "5. task_handler", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1757", null ],
        [ "6. task_context", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1758", null ],
        [ "7. result_backend", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1759", null ],
        [ "8. async_result", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1760", null ],
        [ "9. task_scheduler", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1761", null ],
        [ "10. task_monitor", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1762", null ]
      ] ],
      [ "데이터 흐름", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1763", [
        [ "작업 제출 흐름", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1764", null ],
        [ "재시도 흐름", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1765", null ]
      ] ],
      [ "스레드 안전성", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1766", [
        [ "thread_system 통합", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1767", null ]
      ] ],
      [ "확장 포인트", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1768", [
        [ "커스텀 Result Backend", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1769", null ],
        [ "커스텀 Task Handler", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1770", null ]
      ] ],
      [ "설계 결정", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1771", [
        [ "1. 메시지 기반 작업", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1772", null ],
        [ "2. Result 타입 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1773", null ],
        [ "3. 빌더 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1774", null ],
        [ "4. 파사드 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1775", null ],
        [ "5. 전략 패턴", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1776", null ]
      ] ],
      [ "테스트", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1777", [
        [ "테스트 실행", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1778", null ]
      ] ],
      [ "관련 문서", "d9/db8/md_docs_2task_2ARCHITECTURE__KO.html#autotoc_md1779", null ]
    ] ],
    [ "Task Module Configuration Guide", "dc/d1e/md_docs_2task_2CONFIGURATION.html", [
      [ "Table of Contents", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1781", null ],
      [ "Task System Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1783", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1784", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1785", null ],
        [ "Example", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1786", null ]
      ] ],
      [ "Task Queue Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1788", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1789", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1790", null ],
        [ "Persistence Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1791", null ],
        [ "Delayed Queue Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1792", null ]
      ] ],
      [ "Worker Pool Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1794", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1795", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1796", null ],
        [ "Concurrency Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1797", null ],
        [ "Queue Priority", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1798", null ],
        [ "Prefetch Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1799", null ]
      ] ],
      [ "Task Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1801", [
        [ "Structure", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1802", null ],
        [ "Options", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1803", null ],
        [ "Using Task Builder", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1804", null ],
        [ "Priority Levels", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1805", null ]
      ] ],
      [ "Environment-Specific Settings", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1807", [
        [ "Development Environment", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1808", null ],
        [ "Staging Environment", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1809", null ],
        [ "Production Environment", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1810", null ]
      ] ],
      [ "Performance Tuning", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1812", [
        [ "High Throughput Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1813", null ],
        [ "Low Latency Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1814", null ],
        [ "Resource-Constrained Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1815", null ],
        [ "Long-Running Tasks Configuration", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1816", null ],
        [ "Memory Optimization", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1817", null ]
      ] ],
      [ "Configuration Recommendations", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1819", [
        [ "By Workload Type", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1820", null ],
        [ "By Scale", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1821", null ]
      ] ],
      [ "Related Documentation", "dc/d1e/md_docs_2task_2CONFIGURATION.html#autotoc_md1823", null ]
    ] ],
    [ "Task 모듈 설정 가이드", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html", [
      [ "목차", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1825", null ],
      [ "Task 시스템 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1827", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1828", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1829", null ],
        [ "예제", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1830", null ]
      ] ],
      [ "Task 큐 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1832", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1833", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1834", null ],
        [ "영속성 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1835", null ],
        [ "지연 큐 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1836", null ]
      ] ],
      [ "워커 풀 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1838", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1839", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1840", null ],
        [ "동시성 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1841", null ],
        [ "큐 우선순위", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1842", null ],
        [ "프리페치 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1843", null ]
      ] ],
      [ "Task 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1845", [
        [ "구조체", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1846", null ],
        [ "옵션", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1847", null ],
        [ "Task 빌더 사용", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1848", null ],
        [ "우선순위 수준", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1849", null ]
      ] ],
      [ "환경별 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1851", [
        [ "개발 환경", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1852", null ],
        [ "스테이징 환경", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1853", null ],
        [ "프로덕션 환경", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1854", null ]
      ] ],
      [ "성능 튜닝", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1856", [
        [ "고처리량 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1857", null ],
        [ "저지연 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1858", null ],
        [ "리소스 제한 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1859", null ],
        [ "장기 실행 작업 설정", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1860", null ],
        [ "메모리 최적화", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1861", null ]
      ] ],
      [ "설정 권장 사항", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1863", [
        [ "워크로드 유형별", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1864", null ],
        [ "규모별", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1865", null ]
      ] ],
      [ "관련 문서", "d4/de7/md_docs_2task_2CONFIGURATION__KO.html#autotoc_md1867", null ]
    ] ],
    [ "Task Module Migration Guide", "d2/d8d/md_docs_2task_2MIGRATION.html", [
      [ "Table of Contents", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1869", null ],
      [ "Version Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1871", [
        [ "Migrating to 1.0", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1872", [
          [ "API Changes", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1873", null ],
          [ "Configuration Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1874", null ]
        ] ]
      ] ],
      [ "Migration from Custom Solutions", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1876", [
        [ "From Thread Pool Implementation", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1877", [
          [ "Before (Custom Thread Pool)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1878", null ],
          [ "After (Task Module)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1879", null ],
          [ "Benefits of Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1880", null ]
        ] ],
        [ "From Message Queue Systems", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1881", [
          [ "Mapping Concepts", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1882", null ],
          [ "Migration Example", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1883", null ]
        ] ],
        [ "From Cron Jobs", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1884", [
          [ "Before (System Cron)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1885", null ],
          [ "After (Task Scheduler)", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1886", null ],
          [ "Benefits", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1887", null ]
        ] ]
      ] ],
      [ "Migration Checklist", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1889", [
        [ "Pre-Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1890", null ],
        [ "During Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1891", null ],
        [ "Post-Migration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1892", null ],
        [ "Testing Checklist", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1893", null ]
      ] ],
      [ "Common Migration Issues", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1895", [
        [ "Handler Throws Instead of Returning Error", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1896", null ],
        [ "Missing Queue Configuration", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1897", null ],
        [ "Timeout Too Short", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1898", null ]
      ] ],
      [ "Related Documentation", "d2/d8d/md_docs_2task_2MIGRATION.html#autotoc_md1900", null ]
    ] ],
    [ "Task 모듈 마이그레이션 가이드", "d7/d10/md_docs_2task_2MIGRATION__KO.html", [
      [ "목차", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1902", null ],
      [ "버전 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1904", [
        [ "1.0으로 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1905", [
          [ "API 변경 사항", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1906", null ],
          [ "설정 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1907", null ]
        ] ]
      ] ],
      [ "커스텀 솔루션에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1909", [
        [ "스레드 풀 구현에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1910", [
          [ "이전 (커스텀 스레드 풀)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1911", null ],
          [ "이후 (Task 모듈)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1912", null ],
          [ "마이그레이션의 이점", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1913", null ]
        ] ],
        [ "메시지 큐 시스템에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1914", [
          [ "개념 매핑", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1915", null ],
          [ "마이그레이션 예제", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1916", null ]
        ] ],
        [ "Cron 작업에서 마이그레이션", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1917", [
          [ "이전 (시스템 Cron)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1918", null ],
          [ "이후 (Task 스케줄러)", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1919", null ],
          [ "이점", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1920", null ]
        ] ]
      ] ],
      [ "마이그레이션 체크리스트", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1922", [
        [ "마이그레이션 전", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1923", null ],
        [ "마이그레이션 중", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1924", null ],
        [ "마이그레이션 후", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1925", null ],
        [ "테스트 체크리스트", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1926", null ]
      ] ],
      [ "일반적인 마이그레이션 문제", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1928", [
        [ "핸들러가 에러 반환 대신 예외를 던짐", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1929", null ],
        [ "누락된 큐 설정", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1930", null ],
        [ "타임아웃이 너무 짧음", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1931", null ]
      ] ],
      [ "관련 문서", "d7/d10/md_docs_2task_2MIGRATION__KO.html#autotoc_md1933", null ]
    ] ],
    [ "Task Module Workflow Patterns", "d5/dd2/md_docs_2task_2PATTERNS.html", [
      [ "Table of Contents", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1935", null ],
      [ "Chain Pattern", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1937", [
        [ "Concept", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1938", null ],
        [ "Usage", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1939", null ],
        [ "Handler Implementation", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1940", null ],
        [ "Error Handling", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1941", null ],
        [ "Use Cases", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1942", null ]
      ] ],
      [ "Chord Pattern", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1944", [
        [ "Concept", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1945", null ],
        [ "Usage", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1946", null ],
        [ "Callback Handler", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1947", null ],
        [ "Error Handling", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1948", null ],
        [ "Use Cases", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1949", null ]
      ] ],
      [ "Retry Strategies", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1951", [
        [ "Default Configuration", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1952", null ],
        [ "Retry Delay Calculation", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1953", null ],
        [ "Using Task Builder", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1954", null ],
        [ "Custom Retry Logic", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1955", null ],
        [ "Retry vs. No Retry Errors", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1956", null ]
      ] ],
      [ "Priority Queues", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1958", [
        [ "Priority Levels", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1959", null ],
        [ "Setting Priority", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1960", null ],
        [ "Priority Queue Setup", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1961", null ],
        [ "Submitting to Priority Queues", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1962", null ],
        [ "Best Practices", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1963", null ]
      ] ],
      [ "Scheduled Tasks", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1965", [
        [ "Periodic Execution", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1966", null ],
        [ "Cron-Based Scheduling", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1967", null ],
        [ "Cron Expression Format", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1968", null ],
        [ "Managing Schedules", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1969", null ],
        [ "Delayed Task Execution", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1970", null ]
      ] ],
      [ "Progress Tracking", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1972", [
        [ "Reporting Progress", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1973", null ],
        [ "Monitoring Progress", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1974", null ],
        [ "Progress History", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1975", null ]
      ] ],
      [ "Checkpoint and Recovery", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1977", [
        [ "Saving Checkpoints", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1978", null ],
        [ "Checkpoint Data", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1979", null ]
      ] ],
      [ "Subtask Spawning", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1981", [
        [ "Spawning Subtasks", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1982", null ],
        [ "Tracking Subtasks", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1983", null ],
        [ "Use Cases", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1984", null ]
      ] ],
      [ "Combining Patterns", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1986", [
        [ "Chain with Retry", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1987", null ],
        [ "Scheduled Chord", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1988", null ]
      ] ],
      [ "Related Documentation", "d5/dd2/md_docs_2task_2PATTERNS.html#autotoc_md1990", null ]
    ] ],
    [ "Task 모듈 워크플로우 패턴", "dd/d35/md_docs_2task_2PATTERNS__KO.html", [
      [ "목차", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1992", null ],
      [ "Chain 패턴", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1994", [
        [ "개념", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1995", null ],
        [ "사용법", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1996", null ],
        [ "핸들러 구현", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1997", null ],
        [ "에러 처리", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1998", null ],
        [ "사용 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md1999", null ]
      ] ],
      [ "Chord 패턴", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2001", [
        [ "개념", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2002", null ],
        [ "사용법", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2003", null ],
        [ "콜백 핸들러", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2004", null ],
        [ "에러 처리", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2005", null ],
        [ "사용 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2006", null ]
      ] ],
      [ "재시도 전략", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2008", [
        [ "기본 설정", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2009", null ],
        [ "재시도 지연 계산", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2010", null ],
        [ "Task 빌더 사용", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2011", null ],
        [ "커스텀 재시도 로직", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2012", null ],
        [ "재시도 vs 재시도하지 않을 에러", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2013", null ]
      ] ],
      [ "우선순위 큐", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2015", [
        [ "우선순위 수준", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2016", null ],
        [ "우선순위 설정", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2017", null ],
        [ "우선순위 큐 설정", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2018", null ],
        [ "우선순위 큐에 제출", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2019", null ],
        [ "모범 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2020", null ]
      ] ],
      [ "예약 작업", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2022", [
        [ "주기적 실행", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2023", null ],
        [ "Cron 기반 스케줄링", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2024", null ],
        [ "Cron 표현식 형식", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2025", null ],
        [ "스케줄 관리", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2026", null ],
        [ "지연 작업 실행", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2027", null ]
      ] ],
      [ "진행 상황 추적", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2029", [
        [ "진행 상황 보고", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2030", null ],
        [ "진행 상황 모니터링", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2031", null ],
        [ "진행 기록", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2032", null ]
      ] ],
      [ "체크포인트와 복구", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2034", [
        [ "체크포인트 저장", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2035", null ],
        [ "체크포인트 데이터", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2036", null ]
      ] ],
      [ "하위 작업 생성", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2038", [
        [ "하위 작업 생성", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2039", null ],
        [ "하위 작업 추적", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2040", null ],
        [ "사용 사례", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2041", null ]
      ] ],
      [ "패턴 조합", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2043", [
        [ "재시도가 있는 Chain", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2044", null ],
        [ "예약된 Chord", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2045", null ]
      ] ],
      [ "관련 문서", "dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2047", null ]
    ] ],
    [ "Task Module Quick Start Guide", "d9/dff/md_docs_2task_2QUICK__START.html", [
      [ "Prerequisites", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2049", null ],
      [ "Installation", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2050", null ],
      [ "Basic Usage", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2051", [
        [ "1. Include Headers", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2052", null ],
        [ "2. Create and Configure the Task System", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2053", null ],
        [ "3. Register a Task Handler", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2054", null ],
        [ "4. Start the System", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2055", null ],
        [ "5. Submit a Task", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2056", null ],
        [ "6. Stop the System", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2057", null ]
      ] ],
      [ "Complete Example", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2058", null ],
      [ "Using the Task Builder", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2059", null ],
      [ "Scheduling Tasks", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2060", [
        [ "Periodic Execution", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2061", null ],
        [ "Cron-based Execution", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2062", null ]
      ] ],
      [ "Monitoring Progress", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2063", null ],
      [ "Error Handling", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2064", null ],
      [ "Next Steps", "d9/dff/md_docs_2task_2QUICK__START.html#autotoc_md2065", null ]
    ] ],
    [ "Task 모듈 빠른 시작 가이드", "da/dee/md_docs_2task_2QUICK__START__KO.html", [
      [ "사전 요구사항", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2067", null ],
      [ "설치", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2068", null ],
      [ "기본 사용법", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2069", [
        [ "1. 헤더 포함", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2070", null ],
        [ "2. Task 시스템 생성 및 설정", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2071", null ],
        [ "3. Task 핸들러 등록", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2072", null ],
        [ "4. 시스템 시작", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2073", null ],
        [ "5. Task 제출", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2074", null ],
        [ "6. 시스템 중지", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2075", null ]
      ] ],
      [ "전체 예제", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2076", null ],
      [ "Task 빌더 사용하기", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2077", null ],
      [ "Task 스케줄링", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2078", [
        [ "주기적 실행", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2079", null ],
        [ "Cron 기반 실행", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2080", null ]
      ] ],
      [ "진행 상황 모니터링", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2081", null ],
      [ "에러 처리", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2082", null ],
      [ "다음 단계", "da/dee/md_docs_2task_2QUICK__START__KO.html#autotoc_md2083", null ]
    ] ],
    [ "Task Module Troubleshooting Guide", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html", [
      [ "Table of Contents", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2085", null ],
      [ "Common Issues", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2087", [
        [ "Tasks Not Being Processed", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2088", null ],
        [ "Tasks Failing Immediately", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2089", null ],
        [ "Tasks Timing Out", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2090", null ],
        [ "Retry Not Working", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2091", null ],
        [ "Memory Usage Growing", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2092", null ]
      ] ],
      [ "Debugging Methods", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2094", [
        [ "Enable Logging", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2095", null ],
        [ "Monitor Events", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2096", null ],
        [ "Check Queue Status", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2097", null ],
        [ "Check Worker Status", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2098", null ],
        [ "View Task Logs", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2099", null ]
      ] ],
      [ "Performance Issues", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2101", [
        [ "High CPU Usage", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2102", null ],
        [ "High Memory Usage", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2103", null ],
        [ "Low Throughput", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2104", null ],
        [ "High Latency", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2105", null ]
      ] ],
      [ "FAQ", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2107", [
        [ "Q: How do I cancel a running task?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2108", null ],
        [ "Q: How do I get task progress from outside the handler?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2109", null ],
        [ "Q: How do I handle task dependencies?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2110", null ],
        [ "Q: How do I process tasks in order?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2111", null ],
        [ "Q: How do I prioritize certain tasks?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2112", null ],
        [ "Q: How do I limit retry attempts?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2113", null ],
        [ "Q: How do I handle poison messages (tasks that always fail)?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2114", null ],
        [ "Q: How do I gracefully shut down the system?", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2115", null ]
      ] ],
      [ "Related Documentation", "dd/df5/md_docs_2task_2TROUBLESHOOTING.html#autotoc_md2117", null ]
    ] ],
    [ "Task 모듈 문제 해결 가이드", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html", [
      [ "목차", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2119", null ],
      [ "일반적인 문제", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2121", [
        [ "작업이 처리되지 않음", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2122", null ],
        [ "작업이 즉시 실패함", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2123", null ],
        [ "작업 타임아웃", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2124", null ],
        [ "재시도가 작동하지 않음", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2125", null ],
        [ "메모리 사용량 증가", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2126", null ]
      ] ],
      [ "디버깅 방법", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2128", [
        [ "로깅 활성화", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2129", null ],
        [ "이벤트 모니터링", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2130", null ],
        [ "큐 상태 확인", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2131", null ],
        [ "워커 상태 확인", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2132", null ],
        [ "작업 로그 보기", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2133", null ]
      ] ],
      [ "성능 문제", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2135", [
        [ "높은 CPU 사용량", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2136", null ],
        [ "높은 메모리 사용량", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2137", null ],
        [ "낮은 처리량", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2138", null ],
        [ "높은 지연 시간", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2139", null ]
      ] ],
      [ "FAQ", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2141", [
        [ "Q: 실행 중인 작업을 어떻게 취소하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2142", null ],
        [ "Q: 핸들러 외부에서 작업 진행 상황을 어떻게 얻나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2143", null ],
        [ "Q: 작업 종속성을 어떻게 처리하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2144", null ],
        [ "Q: 작업을 순서대로 처리하려면 어떻게 하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2145", null ],
        [ "Q: 특정 작업의 우선순위를 어떻게 높이나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2146", null ],
        [ "Q: 재시도 횟수를 어떻게 제한하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2147", null ],
        [ "Q: 독약 메시지(항상 실패하는 작업)를 어떻게 처리하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2148", null ],
        [ "Q: 시스템을 우아하게 종료하려면 어떻게 하나요?", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2149", null ]
      ] ],
      [ "관련 문서", "de/da3/md_docs_2task_2TROUBLESHOOTING__KO.html#autotoc_md2151", null ]
    ] ],
    [ "Distributed Task Queue System - Improvement Plan", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html", [
      [ "Executive Summary", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2153", null ],
      [ "Phase 1: Task Core (기반 확장)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2155", [
        [ "1.1 Task 정의 확장", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2156", null ],
        [ "1.2 Task Builder", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2157", null ]
      ] ],
      [ "Phase 2: Worker System", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2159", [
        [ "2.1 Task Handler Interface", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2160", null ],
        [ "2.2 Worker Pool", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2161", null ]
      ] ],
      [ "Phase 3: Task Queue (확장)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2163", [
        [ "3.1 Priority Task Queue", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2164", null ]
      ] ],
      [ "Phase 4: Result Backend", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2166", [
        [ "4.1 Result Backend Interface", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2167", null ],
        [ "4.2 In-Memory Result Backend", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2168", null ],
        [ "4.3 Redis Result Backend (선택적)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2169", null ]
      ] ],
      [ "Phase 5: Task Client (Producer)", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2171", [
        [ "5.1 Task Client", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2172", null ]
      ] ],
      [ "Phase 6: Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2174", [
        [ "6.1 Periodic Task Scheduler", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2175", null ]
      ] ],
      [ "Phase 7: Monitoring & Management", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2177", [
        [ "7.1 Task Monitor", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2178", null ]
      ] ],
      [ "Phase 8: Integration", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2180", [
        [ "8.1 통합 서비스 컨테이너", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2181", null ]
      ] ],
      [ "Implementation Roadmap", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2183", [
        [ "Sprint 1: Core Task Infrastructure ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2184", null ],
        [ "Sprint 2: Queue & Result Backend ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2185", null ],
        [ "Sprint 3: Worker System ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2186", null ],
        [ "Sprint 4: Client & Async Result ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2187", null ],
        [ "Sprint 5: Scheduler ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2188", null ],
        [ "Sprint 6: Monitoring & System Integration ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2189", null ],
        [ "Sprint 7: Testing & Documentation ✅", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2190", null ]
      ] ],
      [ "File Structure", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2192", null ],
      [ "Usage Example", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2194", null ],
      [ "Reusing Existing Components", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2196", null ],
      [ "Key Design Decisions", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2198", [
        [ "1. Task는 Message를 상속", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2199", null ],
        [ "2. Result Backend 분리", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2200", null ],
        [ "3. 핸들러 등록 방식", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2201", null ],
        [ "4. 비동기 우선 설계", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2202", null ]
      ] ],
      [ "Version", "d1/db3/md_docs_2TASK__QUEUE__IMPROVEMENT__PLAN.html#autotoc_md2204", null ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"d1/d5e/md_docs_2task_2ARCHITECTURE.html",
"d4/d22/md_docs_2PROJECT__STRUCTURE.html#autotoc_md1387",
"d6/dab/md_docs_2guides_2TROUBLESHOOTING.html#autotoc_md1124",
"d9/d77/md_docs_2guides_2DEPLOYMENT__GUIDE.html#autotoc_md939",
"da/dee/md_docs_2CHANGELOG.html#autotoc_md552",
"dd/d35/md_docs_2task_2PATTERNS__KO.html#autotoc_md2020",
"de/d93/md_docs_2BENCHMARKS.html#autotoc_md500",
"df/dde/md_docs_2advanced_2PATTERNS__API.html#autotoc_md114"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';