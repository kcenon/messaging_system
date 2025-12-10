# Distributed Task Queue System - Improvement Plan

## Executive Summary

messaging_system을 **분산 작업 큐 시스템**으로 개선하기 위한 구현 계획입니다.

**목표**: Celery/Sidekiq 수준의 기능을 갖춘 C++ 분산 작업 큐 시스템 구축

---

## Phase 1: Task Core (기반 확장)

### 1.1 Task 정의 확장

**위치**: `include/kcenon/messaging/task/task.h`

현재 `message` 클래스를 확장하여 Task 전용 구조 생성:

```cpp
namespace kcenon::messaging::task {

enum class task_state {
    pending,      // 대기 중
    queued,       // 큐에 등록됨
    running,      // 실행 중
    succeeded,    // 성공 완료
    failed,       // 실패
    retrying,     // 재시도 중
    cancelled,    // 취소됨
    expired       // 만료됨
};

struct task_config {
    std::chrono::milliseconds timeout{300000};     // 5분 기본
    size_t max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
    double retry_backoff_multiplier = 2.0;
    message_priority priority = message_priority::normal;
    std::optional<std::chrono::system_clock::time_point> eta;  // 예약 실행
    std::optional<std::chrono::milliseconds> expires;          // 만료 시간
    std::string queue_name = "default";                        // 큐 지정
    std::vector<std::string> tags;                             // 그룹화/필터링
};

class task : public message {
    std::string task_id_;
    std::string task_name_;           // 핸들러 식별자 (e.g., "email.send")
    task_state state_ = task_state::pending;
    task_config config_;

    // 실행 추적
    size_t attempt_count_ = 0;
    std::chrono::system_clock::time_point started_at_;
    std::chrono::system_clock::time_point completed_at_;

    // 진행률 (0.0 ~ 1.0)
    std::atomic<double> progress_{0.0};
    std::string progress_message_;

    // 결과/에러
    std::optional<container_module::value_container> result_;
    std::optional<std::string> error_message_;
    std::optional<std::string> error_traceback_;
};
```

### 1.2 Task Builder

```cpp
class task_builder {
public:
    task_builder(const std::string& task_name);

    task_builder& payload(container_module::value_container payload);
    task_builder& priority(message_priority priority);
    task_builder& timeout(std::chrono::milliseconds timeout);
    task_builder& retries(size_t max_retries);
    task_builder& queue(const std::string& queue_name);
    task_builder& eta(std::chrono::system_clock::time_point execute_at);
    task_builder& countdown(std::chrono::milliseconds delay);
    task_builder& expires(std::chrono::milliseconds expires_in);
    task_builder& tag(const std::string& tag);

    common::Result<task> build();
};
```

---

## Phase 2: Worker System

### 2.1 Task Handler Interface

**위치**: `include/kcenon/messaging/task/task_handler.h`

```cpp
/**
 * @brief Task 실행 컨텍스트
 * 핸들러에게 실행 환경 정보와 제어 기능 제공
 */
class task_context {
public:
    // 진행률 업데이트
    void update_progress(double progress, const std::string& message = "");

    // 체크포인트 저장 (재시도 시 복구용)
    void save_checkpoint(const container_module::value_container& state);
    container_module::value_container load_checkpoint() const;

    // 하위 작업 생성
    common::Result<std::string> spawn_subtask(task subtask);

    // 취소 확인
    bool is_cancelled() const;

    // 로깅
    void log_info(const std::string& message);
    void log_warning(const std::string& message);
    void log_error(const std::string& message);

    // 현재 작업 정보
    const task& current_task() const;
    size_t attempt_number() const;
};

/**
 * @brief Task 핸들러 인터페이스
 */
class task_handler_interface {
public:
    virtual ~task_handler_interface() = default;

    // 핸들러 이름 (task_name과 매칭)
    virtual std::string name() const = 0;

    // 작업 실행
    virtual common::Result<container_module::value_container> execute(
        const task& t,
        task_context& ctx
    ) = 0;

    // 선택적: 재시도 전 훅
    virtual void on_retry(const task& t, size_t attempt) {}

    // 선택적: 실패 시 훅
    virtual void on_failure(const task& t, const std::string& error) {}
};

/**
 * @brief Lambda 기반 간편 핸들러
 */
using simple_task_handler = std::function<
    common::Result<container_module::value_container>(
        const task&,
        task_context&
    )
>;
```

### 2.2 Worker Pool

**위치**: `include/kcenon/messaging/task/worker_pool.h`

```cpp
struct worker_config {
    size_t concurrency = std::thread::hardware_concurrency();
    std::vector<std::string> queues = {"default"};  // 처리할 큐 목록
    std::chrono::milliseconds poll_interval{100};
    bool prefetch = true;
    size_t prefetch_count = 10;
};

class worker_pool {
public:
    worker_pool(
        std::shared_ptr<task_queue> queue,
        std::shared_ptr<result_backend> results,
        worker_config config = {}
    );

    // 핸들러 등록
    void register_handler(std::shared_ptr<task_handler_interface> handler);
    void register_handler(const std::string& name, simple_task_handler handler);

    // 수명주기
    common::VoidResult start();
    common::VoidResult stop();
    common::VoidResult shutdown_graceful(std::chrono::milliseconds timeout);

    // 상태
    bool is_running() const;
    size_t active_workers() const;
    size_t idle_workers() const;

    // 통계
    struct worker_statistics {
        uint64_t tasks_processed;
        uint64_t tasks_succeeded;
        uint64_t tasks_failed;
        uint64_t tasks_retried;
        std::chrono::milliseconds avg_execution_time;
        std::chrono::milliseconds max_execution_time;
    };
    worker_statistics get_statistics() const;

private:
    void worker_loop(size_t worker_id);
    common::VoidResult execute_task(task& t);
};
```

---

## Phase 3: Task Queue (확장)

### 3.1 Priority Task Queue

**위치**: `include/kcenon/messaging/task/task_queue.h`

기존 `message_queue` + `message_bus`를 확장:

```cpp
struct task_queue_config {
    size_t max_size = 100000;
    bool enable_persistence = false;
    std::string persistence_path;
    bool enable_delayed_queue = true;       // ETA 지원
    std::chrono::milliseconds delayed_poll_interval{1000};
};

class task_queue {
public:
    task_queue(task_queue_config config = {});

    // 작업 등록
    common::Result<std::string> enqueue(task t);
    common::Result<std::vector<std::string>> enqueue_bulk(std::vector<task> tasks);

    // 작업 가져오기 (워커용)
    common::Result<task> dequeue(
        const std::vector<std::string>& queue_names,
        std::chrono::milliseconds timeout
    );

    // 작업 취소
    common::VoidResult cancel(const std::string& task_id);
    common::VoidResult cancel_by_tag(const std::string& tag);

    // 조회
    common::Result<task> get_task(const std::string& task_id) const;
    size_t queue_size(const std::string& queue_name) const;
    std::vector<std::string> list_queues() const;

    // 지연 작업 관리 (내부)
    void process_delayed_tasks();

private:
    // 큐별 priority queue
    std::unordered_map<std::string, std::unique_ptr<message_queue>> queues_;

    // 지연 작업 (ETA 기반)
    struct delayed_task {
        task t;
        std::chrono::system_clock::time_point execute_at;
    };
    std::priority_queue<
        delayed_task,
        std::vector<delayed_task>,
        /* earliest first */
    > delayed_queue_;
};
```

---

## Phase 4: Result Backend

### 4.1 Result Backend Interface

**위치**: `include/kcenon/messaging/task/result_backend.h`

```cpp
class result_backend_interface {
public:
    virtual ~result_backend_interface() = default;

    // 상태 저장
    virtual common::VoidResult store_state(
        const std::string& task_id,
        task_state state
    ) = 0;

    // 결과 저장
    virtual common::VoidResult store_result(
        const std::string& task_id,
        const container_module::value_container& result
    ) = 0;

    // 에러 저장
    virtual common::VoidResult store_error(
        const std::string& task_id,
        const std::string& error,
        const std::string& traceback = ""
    ) = 0;

    // 진행률 저장
    virtual common::VoidResult store_progress(
        const std::string& task_id,
        double progress,
        const std::string& message = ""
    ) = 0;

    // 조회
    virtual common::Result<task_state> get_state(const std::string& task_id) = 0;
    virtual common::Result<container_module::value_container> get_result(
        const std::string& task_id
    ) = 0;
    virtual common::Result<double> get_progress(const std::string& task_id) = 0;

    // 대기
    virtual common::Result<container_module::value_container> wait_for_result(
        const std::string& task_id,
        std::chrono::milliseconds timeout
    ) = 0;

    // 정리
    virtual common::VoidResult cleanup_expired(
        std::chrono::milliseconds max_age
    ) = 0;
};
```

### 4.2 In-Memory Result Backend

```cpp
class memory_result_backend : public result_backend_interface {
    struct task_result {
        task_state state;
        std::optional<container_module::value_container> result;
        std::optional<std::string> error;
        std::optional<std::string> traceback;
        double progress = 0.0;
        std::string progress_message;
        std::chrono::system_clock::time_point updated_at;
    };

    std::unordered_map<std::string, task_result> results_;
    mutable std::shared_mutex mutex_;
    std::condition_variable_any cv_;
};
```

### 4.3 Redis Result Backend (선택적)

```cpp
// network_system 통합 후 구현
class redis_result_backend : public result_backend_interface {
    // Redis를 통한 분산 결과 저장
};
```

---

## Phase 5: Task Client (Producer)

### 5.1 Task Client

**위치**: `include/kcenon/messaging/task/task_client.h`

```cpp
/**
 * @brief 작업 제출 결과 (비동기 결과 조회용)
 */
class async_result {
public:
    async_result(
        std::string task_id,
        std::shared_ptr<result_backend_interface> backend
    );

    // 작업 ID
    const std::string& task_id() const;

    // 상태 확인
    task_state state() const;
    bool is_ready() const;
    bool is_successful() const;
    bool is_failed() const;

    // 진행률
    double progress() const;
    std::string progress_message() const;

    // 결과 조회 (블로킹)
    common::Result<container_module::value_container> get(
        std::chrono::milliseconds timeout = std::chrono::milliseconds::max()
    );

    // 결과 조회 (콜백)
    void then(
        std::function<void(const container_module::value_container&)> on_success,
        std::function<void(const std::string&)> on_failure = nullptr
    );

    // 취소
    common::VoidResult revoke();

    // 자식 작업
    std::vector<async_result> children() const;
};

/**
 * @brief Task 생성 및 제출 클라이언트
 */
class task_client {
public:
    task_client(
        std::shared_ptr<task_queue> queue,
        std::shared_ptr<result_backend_interface> results
    );

    // 즉시 실행
    async_result send(task t);
    async_result send(const std::string& task_name, container_module::value_container payload);

    // 지연 실행
    async_result send_later(task t, std::chrono::milliseconds delay);
    async_result send_at(task t, std::chrono::system_clock::time_point eta);

    // 그룹 실행
    std::vector<async_result> send_batch(std::vector<task> tasks);

    // 체인 실행 (순차)
    async_result chain(std::vector<task> tasks);

    // 병렬 실행 후 집계
    async_result chord(
        std::vector<task> tasks,
        task callback
    );

    // 결과 조회
    async_result get_result(const std::string& task_id);
};
```

---

## Phase 6: Scheduler

### 6.1 Periodic Task Scheduler

**위치**: `include/kcenon/messaging/task/scheduler.h`

```cpp
struct schedule_entry {
    std::string name;                    // 스케줄 이름
    task task_template;                  // 실행할 작업 템플릿
    std::variant<
        std::chrono::seconds,            // 고정 간격
        std::string                      // Cron 표현식 (e.g., "0 */5 * * *")
    > schedule;
    bool enabled = true;
    std::optional<std::chrono::system_clock::time_point> last_run;
    std::optional<std::chrono::system_clock::time_point> next_run;
};

class task_scheduler {
public:
    task_scheduler(std::shared_ptr<task_client> client);

    // 스케줄 등록
    common::VoidResult add_periodic(
        const std::string& name,
        task task_template,
        std::chrono::seconds interval
    );

    common::VoidResult add_cron(
        const std::string& name,
        task task_template,
        const std::string& cron_expression
    );

    // 스케줄 관리
    common::VoidResult remove(const std::string& name);
    common::VoidResult enable(const std::string& name);
    common::VoidResult disable(const std::string& name);

    // 수명주기
    common::VoidResult start();
    common::VoidResult stop();

    // 조회
    std::vector<schedule_entry> list_schedules() const;

private:
    void scheduler_loop();
    std::chrono::system_clock::time_point calculate_next_run(
        const schedule_entry& entry
    );
};
```

---

## Phase 7: Monitoring & Management

### 7.1 Task Monitor

**위치**: `include/kcenon/messaging/task/monitor.h`

```cpp
struct queue_stats {
    std::string name;
    size_t pending_count;
    size_t running_count;
    size_t delayed_count;
};

struct worker_info {
    std::string worker_id;
    std::vector<std::string> queues;
    size_t active_tasks;
    std::chrono::system_clock::time_point last_heartbeat;
    bool is_healthy;
};

class task_monitor {
public:
    task_monitor(
        std::shared_ptr<task_queue> queue,
        std::shared_ptr<result_backend_interface> results,
        std::shared_ptr<worker_pool> workers = nullptr
    );

    // 큐 통계
    std::vector<queue_stats> get_queue_stats() const;

    // 워커 상태
    std::vector<worker_info> get_workers() const;

    // 작업 조회
    std::vector<task> list_active_tasks() const;
    std::vector<task> list_pending_tasks(const std::string& queue = "default") const;
    std::vector<task> list_failed_tasks(size_t limit = 100) const;

    // 작업 관리
    common::VoidResult cancel_task(const std::string& task_id);
    common::VoidResult retry_task(const std::string& task_id);
    common::VoidResult purge_queue(const std::string& queue_name);

    // 이벤트 구독
    void on_task_started(std::function<void(const task&)> handler);
    void on_task_completed(std::function<void(const task&, bool success)> handler);
    void on_task_failed(std::function<void(const task&, const std::string& error)> handler);
    void on_worker_offline(std::function<void(const std::string& worker_id)> handler);
};
```

---

## Phase 8: Integration

### 8.1 통합 서비스 컨테이너

**위치**: `include/kcenon/messaging/task/task_system.h`

```cpp
struct task_system_config {
    task_queue_config queue;
    worker_config worker;
    bool enable_scheduler = true;
    bool enable_monitoring = true;
    std::string result_backend_type = "memory";  // "memory", "redis"
};

/**
 * @brief 분산 작업 큐 시스템 통합 파사드
 */
class task_system {
public:
    explicit task_system(task_system_config config = {});

    // 수명주기
    common::VoidResult start();
    common::VoidResult stop();

    // 컴포넌트 접근
    task_client& client();
    worker_pool& workers();
    task_scheduler& scheduler();
    task_monitor& monitor();

    // 핸들러 등록 (편의 메서드)
    void register_handler(const std::string& name, simple_task_handler handler);

    // 작업 제출 (편의 메서드)
    async_result submit(const std::string& task_name, container_module::value_container payload);
};
```

---

## Implementation Roadmap

### Sprint 1: Core Task Infrastructure ✅
1. `task.h` / `task.cpp` - Task 클래스 구현 ✅
2. `task_builder.h` - Task 빌더 ✅
3. `task_handler.h` - 핸들러 인터페이스 ✅
4. `task_context.h` - 실행 컨텍스트 ✅

### Sprint 2: Queue & Result Backend ✅
1. `task_queue.h` / `task_queue.cpp` - 작업 큐 (기존 확장) ✅
2. `result_backend.h` - 결과 백엔드 인터페이스 ✅
3. `memory_result_backend.h/.cpp` - 메모리 백엔드 ✅

### Sprint 3: Worker System ✅
1. `worker_pool.h` / `worker_pool.cpp` - 워커 풀 ✅
2. Worker 실행 로직 ✅
3. 재시도 메커니즘 (기존 `resilient_transport` 패턴 활용) ✅

### Sprint 4: Client & Async Result ✅
1. `async_result.h` / `async_result.cpp` - 비동기 결과 ✅
2. `task_client.h` / `task_client.cpp` - 클라이언트 ✅
3. Chain/Chord 패턴 ✅

### Sprint 5: Scheduler ✅
1. `scheduler.h` / `scheduler.cpp` - 스케줄러 ✅
   - Periodic task scheduling with fixed intervals
   - Cron-based scheduling with 5-field expressions
   - Schedule management (add, remove, enable, disable)
   - Background scheduler loop with efficient waiting (#110)
2. Cron 파싱 (간단한 구현 또는 라이브러리) ✅
   - `cron_parser.h` / `cron_parser.cpp` - Cron 표현식 파서 (#111)

### Sprint 6: Monitoring & System Integration ✅
1. `monitor.h` / `monitor.cpp` - 모니터링 ✅
2. `task_system.h` / `task_system.cpp` - 통합 파사드 ✅
3. 이벤트 브리지 통합 ✅

### Sprint 7: Testing & Documentation ✅
1. 단위 테스트 (각 컴포넌트) ✅
2. 통합 테스트 ✅
3. 벤치마크 (Issue #119)
4. 문서화 ✅

---

## File Structure

```
include/kcenon/messaging/
├── task/
│   ├── task.h                    # Task 정의
│   ├── task_builder.h            # Task 빌더
│   ├── task_handler.h            # 핸들러 인터페이스
│   ├── task_context.h            # 실행 컨텍스트
│   ├── task_queue.h              # 작업 큐
│   ├── worker_pool.h             # 워커 풀
│   ├── result_backend.h          # 결과 백엔드 인터페이스
│   ├── memory_result_backend.h   # 메모리 백엔드
│   ├── async_result.h            # 비동기 결과
│   ├── task_client.h             # 클라이언트
│   ├── scheduler.h               # 스케줄러
│   ├── monitor.h                 # 모니터링
│   └── task_system.h             # 통합 파사드
│
src/impl/
├── task/
│   ├── task.cpp
│   ├── task_queue.cpp
│   ├── worker_pool.cpp
│   ├── memory_result_backend.cpp
│   ├── async_result.cpp
│   ├── task_client.cpp
│   ├── scheduler.cpp
│   ├── monitor.cpp
│   └── task_system.cpp
│
test/unit/task/
├── test_task.cpp
├── test_task_queue.cpp
├── test_worker_pool.cpp
├── test_result_backend.cpp
├── test_async_result.cpp
├── test_task_client.cpp
├── test_scheduler.cpp
└── test_monitor.cpp
│
examples/task/
├── simple_worker.cpp             # 기본 워커 예제
├── priority_tasks.cpp            # 우선순위 작업
├── scheduled_tasks.cpp           # 예약 작업
├── chain_workflow.cpp            # 체인 워크플로우
└── monitoring_dashboard.cpp      # 모니터링 예제
```

---

## Usage Example

```cpp
#include <kcenon/messaging/task/task_system.h>

int main() {
    using namespace kcenon::messaging::task;

    // 시스템 초기화
    task_system_config config;
    config.worker.concurrency = 4;
    config.worker.queues = {"default", "high-priority"};

    task_system system(config);

    // 핸들러 등록
    system.register_handler("email.send", [](const task& t, task_context& ctx) {
        auto email = t.payload().get<std::string>("to");
        auto subject = t.payload().get<std::string>("subject");

        ctx.update_progress(0.5, "Sending email...");

        // 이메일 전송 로직
        send_email(email, subject);

        ctx.update_progress(1.0, "Done");

        container_module::value_container result;
        result.set("status", "sent");
        return common::Result<container_module::value_container>::success(result);
    });

    // 시스템 시작
    system.start();

    // 작업 제출
    auto result = system.submit("email.send", {
        {"to", "user@example.com"},
        {"subject", "Hello!"}
    });

    // 결과 대기
    auto outcome = result.get(std::chrono::seconds(30));
    if (outcome.is_ok()) {
        std::cout << "Email sent: " << outcome.value().get<std::string>("status") << "\n";
    }

    // 예약 작업
    system.scheduler().add_periodic(
        "daily-report",
        task_builder("report.generate").build().value(),
        std::chrono::hours(24)
    );

    // 종료 대기
    std::cin.get();
    system.stop();
}
```

---

## Reusing Existing Components

| 신규 컴포넌트 | 기존 컴포넌트 활용 |
|--------------|-------------------|
| `task` | `message` 상속/확장 |
| `task_queue` | `message_queue` + `message_bus` 확장 |
| `worker_pool` | `standalone_backend` 패턴 참조 |
| 재시도 로직 | `resilient_transport` 로직 재사용 |
| 배치 처리 | `event_batch_processor` 패턴 참조 |
| 파이프라인 | `message_pipeline` 패턴 참조 |
| 통계 | `message_bus::statistics` 패턴 확장 |
| 이벤트 | `event_bridge` 통합 |

---

## Key Design Decisions

### 1. Task는 Message를 상속
- 기존 메시징 인프라 재사용
- pub/sub, routing 등 기존 기능 활용 가능

### 2. Result Backend 분리
- 인메모리 → 분산 (Redis) 교체 용이
- 테스트 용이성 확보

### 3. 핸들러 등록 방식
- 인터페이스 기반: 복잡한 핸들러
- Lambda 기반: 간단한 핸들러

### 4. 비동기 우선 설계
- 모든 작업은 기본적으로 비동기
- `async_result`로 결과 조회

---

## Version

- **Plan Version**: 1.1.0
- **Created**: 2025-12-07
- **Completed**: 2025-12-10
- **Target messaging_system Version**: 2.0.0
- **Status**: All sprints completed
