# Messaging System 프로젝트 구조

**버전**: 1.1
**최종 수정일**: 2025-12-10
**언어**: [English](PROJECT_STRUCTURE.md) | [한국어]

---

## 개요

이 문서는 messaging_system 프로젝트의 전체 디렉토리 구조와 구성에 대해 설명합니다.

---

## 디렉토리 구조

```
messaging_system/
├── include/kcenon/messaging/          # 공개 API 헤더
│   ├── core/                          # 핵심 메시징 컴포넌트
│   │   ├── message.h                  # 메시지 구조 및 빌더
│   │   ├── message_bus.h              # 중앙 메시지 허브
│   │   ├── message_broker.h           # 메시지 브로커 로직
│   │   ├── message_queue.h            # 스레드 안전 큐
│   │   └── topic_router.h             # 토픽 기반 라우팅
│   │
│   ├── interfaces/                    # 추상 인터페이스
│   │   ├── message_handler_interface.h
│   │   ├── message_router_interface.h
│   │   ├── publisher_interface.h
│   │   ├── subscriber_interface.h
│   │   └── queue_interface.h
│   │
│   ├── backends/                      # 실행 백엔드
│   │   ├── backend_interface.h
│   │   ├── standalone_backend.h       # 독립 실행
│   │   └── integration_backend.h      # 외부 통합
│   │
│   ├── patterns/                      # 메시징 패턴
│   │   ├── pub_sub.h                  # Pub/Sub 패턴
│   │   ├── request_reply.h            # Request/Reply 패턴
│   │   ├── event_streaming.h          # 이벤트 소싱
│   │   └── message_pipeline.h         # 메시지 파이프라인
│   │
│   ├── task/                          # 분산 태스크 큐 시스템
│   │   ├── task.h                     # 태스크 정의 및 빌더
│   │   ├── task_handler.h             # C++20 Concepts 핸들러 인터페이스
│   │   ├── task_context.h             # 실행 컨텍스트
│   │   ├── task_queue.h               # 우선순위 태스크 큐
│   │   ├── worker_pool.h              # 워커 스레드 풀
│   │   ├── result_backend.h           # 결과 저장소 인터페이스
│   │   ├── memory_result_backend.h    # 인메모리 결과 백엔드
│   │   ├── async_result.h             # 비동기 결과 핸들
│   │   ├── task_client.h              # 태스크 제출 클라이언트
│   │   ├── scheduler.h                # 주기적/크론 스케줄러
│   │   ├── cron_parser.h              # 크론 표현식 파서
│   │   ├── monitor.h                  # 태스크 모니터링
│   │   └── task_system.h              # 통합 파사드
│   │
│   └── utils/                         # 유틸리티
│       └── integration_detector.h     # 시스템 통합 감지
│
├── src/impl/                          # 구현 파일
│   ├── core/                          # 핵심 구현
│   │   ├── message_impl.cpp
│   │   ├── message_bus_impl.cpp
│   │   ├── message_broker_impl.cpp
│   │   ├── message_queue_impl.cpp
│   │   └── topic_router_impl.cpp
│   │
│   ├── backends/                      # 백엔드 구현
│   │   ├── standalone_backend_impl.cpp
│   │   └── integration_backend_impl.cpp
│   │
│   ├── patterns/                      # 패턴 구현
│   │   ├── pub_sub_impl.cpp
│   │   ├── request_reply_impl.cpp
│   │   ├── event_streaming_impl.cpp
│   │   └── message_pipeline_impl.cpp
│   │
│   ├── task/                          # 태스크 큐 구현
│   │   ├── task.cpp
│   │   ├── task_context.cpp
│   │   ├── task_queue.cpp
│   │   ├── worker_pool.cpp
│   │   ├── memory_result_backend.cpp
│   │   ├── async_result.cpp
│   │   ├── task_client.cpp
│   │   ├── scheduler.cpp
│   │   ├── cron_parser.cpp
│   │   ├── monitor.cpp
│   │   └── task_system.cpp
│   │
│   └── di/                            # 의존성 주입
│       └── messaging_di_container.cpp
│
├── test/                              # 테스트 스위트
│   ├── unit/                          # 유닛 테스트
│   │   ├── core/                      # 핵심 컴포넌트 테스트
│   │   │   ├── test_message.cpp
│   │   │   ├── test_message_bus.cpp
│   │   │   ├── test_message_queue.cpp
│   │   │   └── test_topic_router.cpp
│   │   │
│   │   ├── backends/                  # 백엔드 테스트
│   │   │   ├── test_standalone_backend.cpp
│   │   │   └── test_integration_backend.cpp
│   │   │
│   │   ├── patterns/                  # 패턴 테스트
│   │   │   ├── test_pub_sub.cpp
│   │   │   ├── test_request_reply.cpp
│   │   │   ├── test_event_streaming.cpp
│   │   │   └── test_message_pipeline.cpp
│   │   │
│   │   ├── task/                      # 태스크 모듈 테스트
│   │   │   ├── test_task.cpp
│   │   │   ├── test_task_context.cpp
│   │   │   ├── test_task_handler.cpp
│   │   │   ├── test_task_queue.cpp
│   │   │   ├── test_worker_pool.cpp
│   │   │   ├── test_result_backend.cpp
│   │   │   ├── test_async_result.cpp
│   │   │   ├── test_task_client.cpp
│   │   │   ├── test_scheduler.cpp
│   │   │   ├── test_cron_parser.cpp
│   │   │   ├── test_monitor.cpp
│   │   │   ├── test_task_system.cpp
│   │   │   └── test_task_event_bridge.cpp
│   │   │
│   │   └── di/                        # DI 컨테이너 테스트
│   │       └── test_di_container.cpp
│   │
│   └── benchmarks/                    # 성능 벤치마크
│       ├── bench_message_creation.cpp
│       ├── bench_message_queue.cpp
│       ├── bench_topic_router.cpp
│       ├── bench_pub_sub_throughput.cpp
│       ├── bench_request_reply_latency.cpp
│       └── task/                      # 태스크 모듈 벤치마크
│           ├── bench_task_queue.cpp
│           ├── bench_worker_throughput.cpp
│           ├── bench_result_backend.cpp
│           └── bench_scheduler.cpp
│
├── integration_tests/                 # 통합 테스트 스위트
│   ├── framework/                     # 테스트 프레임워크
│   │   ├── messaging_fixture.h
│   │   └── test_helpers.h
│   │
│   ├── test_message_bus_router.cpp    # 버스 + 라우터 통합
│   ├── test_priority_queue.cpp        # 우선순위 큐 시나리오
│   ├── test_backend_integration.cpp   # 백엔드 통합
│   ├── test_full_integration.cpp      # 엔드투엔드 테스트
│   │
│   └── task/                          # 태스크 모듈 통합 테스트
│       ├── task_fixture.h             # 태스크 테스트 픽스처
│       ├── test_task_lifecycle.cpp    # 전체 태스크 생명주기
│       ├── test_worker_scenarios.cpp  # 워커 풀 시나리오
│       ├── test_scheduling.cpp        # 스케줄러 통합
│       ├── test_concurrent_load.cpp   # 동시 로드 테스트
│       └── test_failure_recovery.cpp  # 실패 및 복구
│
├── examples/                          # 예제 애플리케이션
│   ├── basic_pub_sub/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   ├── request_reply/
│   │   ├── CMakeLists.txt
│   │   ├── server.cpp
│   │   └── client.cpp
│   │
│   ├── event_streaming/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   ├── message_pipeline/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   │
│   └── task/                          # 태스크 큐 예제
│       ├── CMakeLists.txt
│       ├── README.md                  # 태스크 예제 가이드
│       ├── simple_worker/             # 기본 태스크 처리
│       ├── priority_tasks/            # 우선순위 기반 실행
│       ├── scheduled_tasks/           # 주기적/크론 스케줄링
│       ├── chain_workflow/            # 태스크 체이닝
│       ├── chord_aggregation/         # 병렬 태스크 집계
│       ├── progress_tracking/         # 진행 상태 모니터링
│       └── monitoring_dashboard/      # 실시간 모니터링
│
├── docs/                              # 문서
│   ├── README.md                      # 문서 인덱스
│   ├── BENCHMARKS.md                  # 성능 벤치마크
│   ├── FEATURES.md                    # 기능 문서
│   ├── PRODUCTION_QUALITY.md          # 품질 보증
│   ├── PROJECT_STRUCTURE.md           # 이 파일
│   ├── API_REFERENCE.md               # API 문서
│   ├── MIGRATION_GUIDE.md             # 마이그레이션 가이드
│   ├── PATTERNS_API.md                # 패턴 문서
│   ├── DESIGN_PATTERNS.md             # 아키텍처 패턴
│   └── task/                          # 태스크 모듈 문서
│       └── ARCHITECTURE.md            # 태스크 시스템 아키텍처
│
├── .github/                           # GitHub 설정
│   └── workflows/                     # CI/CD 워크플로우
│       ├── ci.yml                     # 메인 CI 파이프라인
│       ├── coverage.yml               # 코드 커버리지
│       ├── static-analysis.yml        # 정적 분석
│       └── documentation.yml          # 문서 생성
│
├── CMakeLists.txt                     # 루트 CMake 설정
├── README.md                          # 프로젝트 README
└── LICENSE                            # BSD 3-Clause 라이선스
```

---

## 컴포넌트 구성

### 핵심 컴포넌트 (`include/kcenon/messaging/core/`)

**목적**: 기본 메시징 인프라

| 파일 | 설명 | 의존성 |
|------|------|--------|
| `message.h` | 메시지 구조, 메타데이터, 빌더 | container_system |
| `message_bus.h` | 중앙 pub/sub 코디네이터 | 모든 핵심 컴포넌트 |
| `message_broker.h` | 고급 라우팅 및 필터링 | topic_router, queue |
| `message_queue.h` | 스레드 안전 우선순위 큐 | common_system |
| `topic_router.h` | 패턴 기반 토픽 라우팅 | common_system |

### 인터페이스 (`include/kcenon/messaging/interfaces/`)

**목적**: 확장성을 위한 추상 인터페이스

| 파일 | 설명 | 구현 |
|------|------|------|
| `message_handler_interface.h` | 메시지 처리 추상화 | 사용자 정의 |
| `message_router_interface.h` | 라우팅 추상화 | topic_router |
| `publisher_interface.h` | 발행 추상화 | publisher (patterns) |
| `subscriber_interface.h` | 구독 추상화 | subscriber (patterns) |
| `queue_interface.h` | 큐 추상화 | message_queue |

### 백엔드 (`include/kcenon/messaging/backends/`)

**목적**: 플러그형 실행 전략

| 파일 | 설명 | 사용 사례 |
|------|------|----------|
| `backend_interface.h` | 백엔드 추상화 | 모든 백엔드 |
| `standalone_backend.h` | thread_system 내부 사용 | 테스트, 간단한 앱 |
| `integration_backend.h` | 외부 스레드 풀 | 프로덕션, 통합 |

### 패턴 (`include/kcenon/messaging/patterns/`)

**목적**: 고수준 메시징 패턴

| 파일 | 설명 | 패턴 유형 |
|------|------|----------|
| `pub_sub.h` | Publisher와 Subscriber | 클래식 pub/sub |
| `request_reply.h` | Request 클라이언트와 서버 | 동기 RPC |
| `event_streaming.h` | 리플레이가 있는 이벤트 소싱 | 이벤트 기반 |
| `message_pipeline.h` | 스테이지 기반 처리 | 파이프-필터 |

### 태스크 큐 (`include/kcenon/messaging/task/`)

**목적**: 스케줄링과 모니터링이 포함된 분산 태스크 큐 시스템

| 파일 | 설명 | 의존성 |
|------|------|--------|
| `task.h` | 태스크 정의 및 빌더 | container_system |
| `task_handler.h` | C++20 Concepts 핸들러 인터페이스 | common_system |
| `task_context.h` | 진행 상태가 있는 실행 컨텍스트 | common_system |
| `task_queue.h` | 우선순위 기반 태스크 큐 | thread_system |
| `worker_pool.h` | 워커 스레드 풀 관리 | thread_system |
| `result_backend.h` | 결과 저장소 인터페이스 | container_system |
| `memory_result_backend.h` | 인메모리 결과 백엔드 | result_backend |
| `async_result.h` | 진행 상태가 있는 비동기 결과 핸들 | result_backend |
| `task_client.h` | 태스크 제출 클라이언트 | task_queue, async_result |
| `scheduler.h` | 주기적/크론 스케줄러 | cron_parser |
| `cron_parser.h` | 크론 표현식 파서 | - |
| `monitor.h` | 실시간 태스크 모니터링 | worker_pool |
| `task_system.h` | 통합 파사드 | 모든 태스크 컴포넌트 |

---

## 빌드 시스템

### CMake 구조

```
CMakeLists.txt                  # 루트 설정
├── Library: messaging_system_core
├── Library: messaging_system_patterns
├── Library: messaging_system_backends
├── Library: messaging_system_task
├── Executable: test targets
├── Executable: benchmark targets
└── Executable: example targets
```

### 빌드 타겟

| 타겟 | 유형 | 설명 |
|------|------|------|
| `messaging_system_core` | 라이브러리 | 핵심 메시징 |
| `messaging_system_patterns` | 라이브러리 | 메시징 패턴 |
| `messaging_system_backends` | 라이브러리 | 실행 백엔드 |
| `messaging_system_task` | 라이브러리 | 태스크 큐 시스템 |
| `test_*` | 실행파일 | 유닛 테스트 |
| `bench_*` | 실행파일 | 벤치마크 |
| `example_*` | 실행파일 | 예제 |

---

## 의존성

### 필수 의존성

| 시스템 | 목적 | 버전 |
|--------|------|------|
| **common_system** | Result<T>, 에러 처리 | >= 2.0 |
| **container_system** | 메시지 페이로드 | >= 2.0 |

### 선택적 의존성

| 시스템 | 목적 | 버전 |
|--------|------|------|
| **thread_system** | 스레드 풀 | >= 2.0 |
| **logger_system** | 로깅 | >= 2.0 |
| **monitoring_system** | 메트릭 | >= 2.0 |

---

## Include 패턴

### 애플리케이션 코드

```cpp
// 핵심 기능
#include <kcenon/messaging/core/message_bus.h>
#include <kcenon/messaging/core/message.h>

// 패턴
#include <kcenon/messaging/patterns/pub_sub.h>
#include <kcenon/messaging/patterns/request_reply.h>

// 백엔드
#include <kcenon/messaging/backends/standalone_backend.h>
#include <kcenon/messaging/backends/integration_backend.h>

// 태스크 큐 시스템
#include <kcenon/messaging/task/task_system.h>  // 통합 파사드
#include <kcenon/messaging/task/task.h>
#include <kcenon/messaging/task/task_client.h>
#include <kcenon/messaging/task/scheduler.h>
```

### 내부 구현

```cpp
// 구현 파일은 공개 및 비공개 헤더 모두 포함해야 함
#include <kcenon/messaging/core/message_bus.h>
#include "internal_helpers.h"
```

---

## 명명 규칙

### 파일

- **헤더**: `snake_case.h`
- **구현**: `snake_case_impl.cpp` 또는 `snake_case.cpp`
- **테스트**: `test_snake_case.cpp`
- **벤치마크**: `bench_snake_case.cpp`
- **예제**: `snake_case.cpp`

### 클래스

- **인터페이스**: `interface_name_interface`
- **구현체**: `concrete_class_name`
- **패턴**: `pattern_name` (예: `publisher`, `subscriber`)

### 네임스페이스

```cpp
namespace kcenon::messaging {              // 핵심 네임스페이스
namespace kcenon::messaging::patterns {    // 패턴
namespace kcenon::messaging::backends {    // 백엔드
namespace kcenon::messaging::task {        // 태스크 큐 시스템
namespace kcenon::messaging::di {          // DI 컨테이너
}
```

---

## 코드 구성 원칙

### 관심사 분리

1. **공개 API** (`include/`): 사용자 대상 헤더
2. **구현** (`src/impl/`): 비공개 구현
3. **테스트** (`test/`): 검증 코드
4. **예제** (`examples/`): 사용법 데모
5. **문서** (`docs/`): 참조 자료

### 모듈성

- **Core**: 기반 컴포넌트
- **Patterns**: 고수준 추상화
- **Backends**: 실행 전략
- **Task**: 분산 태스크 큐 시스템
- **Interfaces**: 확장 지점
- **Utilities**: 헬퍼 함수

### 의존성

- **Core** 의존: common_system, container_system
- **Patterns** 의존: core
- **Backends** 의존: core, common_system
- **Task** 의존: common_system, container_system, thread_system
- **모든 것** 의존: common_system (Result<T>)

---

## 향후 구조

### 계획된 추가 사항

```
messaging_system/
├── include/kcenon/messaging/
│   ├── reliability/               # 신뢰성 기능
│   │   ├── retry_policy.h
│   │   ├── circuit_breaker.h
│   │   ├── dead_letter_queue.h
│   │   └── message_persistence.h
│   │
│   ├── routing/                   # 고급 라우팅
│   │   ├── content_router.h
│   │   ├── routing_table.h
│   │   └── load_balancer.h
│   │
│   ├── security/                  # 보안 기능
│   │   ├── message_validator.h
│   │   ├── access_control.h
│   │   └── encryption.h
│   │
│   └── monitoring/                # 관측성
│       ├── message_metrics.h
│       └── message_tracer.h
│
└── docs/
    └── guides/                    # 사용자 가이드
        ├── QUICK_START.md
        ├── INTEGRATION.md
        └── TROUBLESHOOTING.md
```

---

**최종 수정일**: 2025-12-10
**버전**: 1.1
