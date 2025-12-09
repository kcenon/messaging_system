# Task 모듈 아키텍처

이 문서는 Task 모듈의 아키텍처와 설계를 설명합니다.

## 개요

Task 모듈은 비동기 작업 실행을 위한 분산 작업 큐 시스템을 제공합니다. 신뢰성, 확장성, 사용 편의성을 위해 설계되었습니다.

## 시스템 아키텍처

```
┌─────────────────────────────────────────────────────────────────────┐
│                          task_system (파사드)                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────────────┐  │
│  │ task_client │    │ task_queue  │    │      worker_pool        │  │
│  │             │───▶│             │───▶│                         │  │
│  │ 제출 API    │    │ 작업 저장소 │    │ ┌─────────────────────┐ │  │
│  └─────────────┘    └─────────────┘    │ │   task_handler(s)   │ │  │
│         │                   ▲          │ └─────────────────────┘ │  │
│         │                   │          │           │             │  │
│         ▼                   │          │           ▼             │  │
│  ┌─────────────┐            │          │ ┌─────────────────────┐ │  │
│  │async_result │            │          │ │   task_context      │ │  │
│  └─────────────┘            │          │ └─────────────────────┘ │  │
│         │                   │          └─────────────────────────┘  │
│         │                   │                      │                │
│         ▼                   │                      ▼                │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                     result_backend                           │   │
│  │                (memory_result_backend)                       │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│  ┌──────────────────┐              ┌──────────────────┐            │
│  │  task_scheduler  │              │   task_monitor   │            │
│  │                  │              │                  │            │
│  │ 주기적/Cron      │              │ 통계 및 이벤트   │            │
│  └──────────────────┘              └──────────────────┘            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

## 핵심 컴포넌트

### 1. task_system (파사드)

`task_system`은 모든 컴포넌트를 통합하는 메인 진입점입니다. 제공 기능:

- 통합 라이프사이클 관리 (시작/중지)
- 일반적인 작업을 위한 편의 메서드
- 필요 시 개별 컴포넌트 접근

```cpp
task_system system(config);
system.register_handler("task.name", handler);
system.start();
auto result = system.submit("task.name", payload);
system.stop();
```

### 2. task

`task` 클래스는 작업 단위를 나타냅니다. 메시징 인프라의 `message` 클래스를 확장합니다.

**주요 속성:**
- `task_id`: 고유 식별자 (UUID)
- `task_name`: 실행할 핸들러 이름
- `payload`: 입력 데이터 (value_container)
- `config`: 실행 설정
- `state`: 현재 라이프사이클 상태

**Task 상태:**

```
                 ┌──────────────────────────────────────────────────┐
                 │                                                  │
                 ▼                                                  │
┌─────────┐   ┌────────┐   ┌─────────┐   ┌───────────┐   ┌─────────┴───┐
│ PENDING │──▶│ QUEUED │──▶│ RUNNING │──▶│ SUCCEEDED │   │   RETRYING  │
│ (대기중)│   │(큐 등록)│   │ (실행중)│   │  (성공)   │   │  (재시도중) │
└─────────┘   └────────┘   └────┬────┘   └───────────┘   └─────────────┘
                 │              │                              ▲
                 │              ▼                              │
                 │        ┌──────────┐                         │
                 │        │  FAILED  │─────────────────────────┘
                 │        │  (실패)  │        (재시도 남음)
                 │        └────┬─────┘
                 │             │
                 │             ▼ (재시도 소진)
                 │        ┌──────────┐
                 │        │  FAILED  │ (최종)
                 │        │  (실패)  │
                 │        └──────────┘
                 │
                 ▼
           ┌───────────┐        ┌──────────┐
           │ CANCELLED │        │ EXPIRED  │
           │  (취소됨) │        │ (만료됨) │
           └───────────┘        └──────────┘
```

### 3. task_queue

`task_queue`는 우선순위를 지원하는 다중 명명 큐를 관리합니다.

**기능:**
- 다중 명명 큐 (예: "default", "high-priority")
- ETA를 사용한 지연 작업 스케줄링
- ID 또는 태그로 작업 취소
- 스레드 안전 작업

**데이터 흐름:**

```
┌──────────────────────────────────────────────────────────┐
│                       task_queue                          │
├──────────────────────────────────────────────────────────┤
│                                                          │
│   ┌─────────────────────────────────────────────────┐   │
│   │              즉시 실행 큐                         │   │
│   │  ┌─────────┐  ┌─────────┐  ┌─────────────────┐  │   │
│   │  │ default │  │  high   │  │ custom-queue    │  │   │
│   │  │ 큐      │  │priority │  │                 │  │   │
│   │  └─────────┘  └─────────┘  └─────────────────┘  │   │
│   └─────────────────────────────────────────────────┘   │
│                          ▲                               │
│                          │ (ETA 도달 시)                 │
│   ┌─────────────────────────────────────────────────┐   │
│   │              지연 큐                              │   │
│   │  예약된 시간을 기다리는 작업들                    │   │
│   └─────────────────────────────────────────────────┘   │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

### 4. worker_pool

`worker_pool`은 스레드 풀을 사용하여 작업을 실행합니다.

**책임:**
- 워커 스레드 관리
- 큐에서 작업 가져오기
- 작업을 등록된 핸들러와 매칭
- 지수 백오프를 사용한 재시도 처리
- 실행 통계 수집

**실행 흐름:**

```
┌────────────────────────────────────────────────────────────────┐
│                         worker_pool                             │
├────────────────────────────────────────────────────────────────┤
│                                                                 │
│   ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐          │
│   │워커 1   │  │워커 2   │  │워커 3   │  │워커 N   │          │
│   └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘          │
│        │            │            │            │                │
│        └────────────┴─────┬──────┴────────────┘                │
│                           │                                    │
│                           ▼                                    │
│                  ┌────────────────┐                            │
│                  │ 작업 꺼내기    │◀───── task_queue           │
│                  └───────┬────────┘                            │
│                          │                                     │
│                          ▼                                     │
│                  ┌────────────────┐                            │
│                  │ 핸들러 찾기    │                            │
│                  └───────┬────────┘                            │
│                          │                                     │
│                          ▼                                     │
│                  ┌────────────────┐                            │
│                  │ 작업 실행      │                            │
│                  └───────┬────────┘                            │
│                          │                                     │
│              ┌───────────┼───────────┐                         │
│              ▼           ▼           ▼                         │
│         ┌────────┐  ┌────────┐  ┌────────┐                    │
│         │ 성공   │  │ 재시도 │  │ 실패   │                    │
│         └───┬────┘  └───┬────┘  └───┬────┘                    │
│             │           │           │                          │
│             └───────────┴───────────┘                          │
│                         │                                      │
│                         ▼                                      │
│                  result_backend                                │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### 5. task_handler

`task_handler_interface`는 작업 실행 방법을 정의합니다.

**인터페이스:**

```cpp
class task_handler_interface {
public:
    virtual std::string name() const = 0;

    virtual common::Result<value_container> execute(
        const task& t,
        task_context& ctx) = 0;

    // 선택적 라이프사이클 훅
    virtual void on_retry(const task& t, size_t attempt) { }
    virtual void on_failure(const task& t, const std::string& error) { }
    virtual void on_success(const task& t, const value_container& result) { }
};
```

**핸들러 유형:**
- 클래스 기반: `task_handler_interface` 상속
- 람다 기반: `make_handler()` 사용 또는 직접 등록

### 6. task_context

`task_context`는 핸들러에게 실행 컨텍스트를 제공합니다.

**기능:**
- 진행 상황 추적 및 보고
- 복구를 위한 체크포인트 관리
- 하위 작업 생성
- 취소 감지
- 로깅

```cpp
common::Result<value_container> execute(const task& t, task_context& ctx) {
    // 진행 상황 업데이트
    ctx.update_progress(0.0, "시작 중...");

    // 취소 확인
    if (ctx.is_cancelled()) {
        return common::error("취소됨");
    }

    // 복구를 위한 체크포인트 저장
    ctx.save_checkpoint({{"step", current_step}});

    // 하위 작업 생성
    ctx.spawn_subtask(subtask);

    // 정보 로깅
    ctx.log_info("처리 완료");

    return common::ok(result);
}
```

### 7. result_backend

`result_backend_interface`는 작업 결과 저장소를 제공합니다.

**구현체:**
- `memory_result_backend`: 인메모리 저장소 (기본값)
- 커스텀 구현 가능 (Redis, 데이터베이스 등)

**저장 데이터:**
- 작업 상태
- 작업 결과
- 에러 정보
- 진행 데이터

### 8. async_result

`async_result`는 작업 실행 추적을 위한 핸들을 제공합니다.

**사용 패턴:**

```cpp
// 폴링
while (!result.is_ready()) {
    std::cout << result.progress() << "%\n";
}

// 블로킹 대기
auto outcome = result.get(timeout);

// 콜백 기반
result.then(
    [](const auto& value) { /* 성공 */ },
    [](const auto& error) { /* 실패 */ }
);
```

### 9. task_scheduler

`task_scheduler`는 주기적 및 cron 기반 작업 실행을 관리합니다.

**스케줄 유형:**
- 주기적: 고정 간격 (예: 5분마다)
- Cron: Cron 표현식 (예: "0 3 * * *" - 매일 오전 3시)

```
┌────────────────────────────────────────┐
│            task_scheduler               │
├────────────────────────────────────────┤
│                                         │
│  ┌───────────────────────────────────┐ │
│  │         스케줄 레지스트리          │ │
│  │                                    │ │
│  │  ┌────────────────────────────┐   │ │
│  │  │ cleanup-job (주기: 5분)    │   │ │
│  │  ├────────────────────────────┤   │ │
│  │  │ daily-report (cron: 0 3 *) │   │ │
│  │  ├────────────────────────────┤   │ │
│  │  │ weekly-summary (cron: ...) │   │ │
│  │  └────────────────────────────┘   │ │
│  └───────────────────────────────────┘ │
│                    │                    │
│                    ▼                    │
│           ┌────────────────┐           │
│           │ 스케줄러 루프  │           │
│           │ (시간 확인)    │           │
│           └───────┬────────┘           │
│                   │                    │
│                   ▼                    │
│            task_client.send()          │
│                                         │
└────────────────────────────────────────┘
```

### 10. task_monitor

`task_monitor`는 시스템 관측성을 제공합니다.

**기능:**
- 큐 통계
- 워커 상태
- 활성/대기/실패 작업 목록
- 이벤트 구독

## 데이터 흐름

### 작업 제출 흐름

```
1. 클라이언트가 페이로드로 작업 생성
         │
         ▼
2. task_client.send(task)
         │
         ▼
3. task_queue.enqueue(task)
   - task_id가 없으면 할당
   - 지연 실행(ETA) 확인
   - 적절한 큐에 추가
         │
         ▼
4. async_result를 클라이언트에 반환
         │
         ▼
5. 워커가 작업을 꺼냄
         │
         ▼
6. 워커가 매칭되는 핸들러 찾음
         │
         ▼
7. 핸들러가 task_context와 함께 실행
         │
         ▼
8. 결과가 result_backend에 저장
         │
         ▼
9. async_result.get()이 결과 반환
```

### 재시도 흐름

```
1. 작업 실행 실패
         │
         ▼
2. 재시도 정책 확인
   - 시도 횟수 < max_retries?
         │
    ┌────┴────┐
    │         │
   예         아니오
    │         │
    ▼         ▼
3a. 재시도   3b. FAILED로
    지연 계산     상태 변경
    │               │
    ▼               ▼
4a. 상태를   4b. 에러를
    RETRYING으로   백엔드에 저장
    변경
    │
    ▼
5a. 지연과 함께 다시 큐에 추가
    │
    ▼
6a. 워커가 다시 작업을 가져감
```

## 스레드 안전성

모든 공개 API는 스레드 안전합니다:

| 컴포넌트 | 스레드 안전 메커니즘 |
|----------|---------------------|
| task_queue | 뮤텍스로 보호되는 내부 상태, thread_system 통합 |
| worker_pool | 동시 워커 실행 |
| async_result | 원자적 상태 + 조건 변수 |
| task_context | 원자적 진행 상황 업데이트 |
| memory_result_backend | R/W 잠금을 위한 shared_mutex |
| task_scheduler | 뮤텍스로 보호되는 스케줄 |

### thread_system 통합

`task_queue`와 `worker_pool` 컴포넌트 모두 스레드 관리를 위해 `thread_system`을 사용합니다. 직접적인 `std::thread` 사용 대신 `kcenon::thread::thread_base`를 활용하여 다음을 제공합니다:

- 표준화된 스레드 수명 주기 관리 (시작/중지)
- 주기적 태스크를 위한 적절한 wake interval 처리
- 일관된 스레드 명명 및 모니터링
- 프로젝트의 스레딩 인프라와의 통합

**task_queue**: 예약된 태스크를 처리하는 지연 태스크 워커 스레드에 `thread_base`를 사용합니다.

**worker_pool**: `task_pool_worker` 클래스를 통해 `thread_base`를 사용합니다. 각 워커 스레드는 `thread_base`를 상속하여 스레드 수명 주기 관리를 `thread_system`에 위임합니다. 이를 통해 직접적인 `std::thread` 사용을 다음으로 대체합니다:
- 태스크별 처리를 위한 `task_pool_worker::do_work()`
- 종료 조정을 위한 `task_pool_worker::should_continue_work()`
- 풀의 poll interval에 맞춘 자동 wake interval 구성

**task_scheduler**: `scheduler_worker` 클래스를 통해 `thread_base`를 사용합니다. 스케줄러의 백그라운드 스레드는 `thread_base`를 상속하여 다음을 제공합니다:
- 예정된 스케줄 확인 및 실행을 위한 `scheduler_worker::do_work()`
- 정상 종료를 위한 `scheduler_worker::should_continue_work()`
- 스케줄 변경 시 즉시 깨어나기 위한 조건 변수 통합
- 다른 컴포넌트와 일관된 스레드 수명 주기 관리

## 확장 포인트

### 커스텀 Result Backend

```cpp
class redis_result_backend : public result_backend_interface {
public:
    common::VoidResult store_result(
        const std::string& task_id,
        const value_container& result) override {
        // Redis에 저장
    }

    common::Result<value_container> get_result(
        const std::string& task_id) override {
        // Redis에서 가져오기
    }

    // ... 다른 메서드 구현
};
```

### 커스텀 Task Handler

```cpp
class custom_handler : public task_handler_interface {
public:
    std::string name() const override { return "custom.task"; }

    common::Result<value_container> execute(
        const task& t,
        task_context& ctx) override {
        // 구현
    }

    void on_retry(const task& t, size_t attempt) override {
        // 커스텀 재시도 로직
    }
};
```

## 설계 결정

### 1. 메시지 기반 작업

작업은 직렬화 및 메타데이터 처리를 위해 기존 메시징 인프라를 활용하도록 `message` 클래스를 확장합니다.

### 2. Result 타입 패턴

모든 작업은 예외 없이 명시적 에러 처리를 위해 `Result<T>` 또는 `VoidResult`를 반환합니다.

### 3. 빌더 패턴

`task_builder`는 복잡한 설정의 작업 구성을 위한 유창한 API를 제공합니다.

### 4. 파사드 패턴

`task_system`은 개별 컴포넌트 접근을 허용하면서 인터페이스를 단순화하는 파사드 역할을 합니다.

### 5. 전략 패턴

`result_backend_interface`는 핵심 로직을 변경하지 않고 다른 저장소 전략을 교체할 수 있게 합니다.

## 테스트

작업 모듈은 `integration_tests/task/`에 위치한 포괄적인 통합 테스트를 제공합니다:

| 테스트 파일 | 설명 |
|-----------|-------------|
| `test_task_lifecycle.cpp` | 제출부터 완료까지 전체 작업 라이프사이클 |
| `test_worker_scenarios.cpp` | 다중 워커, 동시성, 핸들러 매칭 |
| `test_failure_recovery.cpp` | 재시도 메커니즘, 타임아웃 처리, 에러 전파 |
| `test_scheduling.cpp` | 주기적 및 크론 기반 작업 스케줄링 |
| `test_concurrent_load.cpp` | 대용량 처리, 처리량 측정 |

### 테스트 실행

```bash
# 테스트 빌드
cmake --build build --target test_task_lifecycle test_worker_scenarios \
    test_failure_recovery test_scheduling test_concurrent_load

# 모든 작업 통합 테스트 실행
ctest --test-dir build -R "test_(task_lifecycle|worker_scenarios|failure_recovery|scheduling|concurrent_load)"

# 특정 테스트 실행
./build/integration_tests/test_task_lifecycle
```

## 관련 문서

- [빠른 시작 가이드](QUICK_START_KO.md)
- [API 레퍼런스](API_REFERENCE_KO.md)
- [패턴 가이드](PATTERNS_KO.md)
- [설정 가이드](CONFIGURATION_KO.md)
- [문제 해결](TROUBLESHOOTING_KO.md)
