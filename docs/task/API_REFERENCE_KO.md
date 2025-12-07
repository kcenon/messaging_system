# Task 모듈 API 레퍼런스

Task 모듈의 완전한 API 문서입니다.

## 목차

- [task](#task)
- [task_config](#task_config)
- [task_builder](#task_builder)
- [task_handler_interface](#task_handler_interface)
- [task_context](#task_context)
- [task_queue](#task_queue)
- [worker_pool](#worker_pool)
- [task_client](#task_client)
- [async_result](#async_result)
- [result_backend_interface](#result_backend_interface)
- [memory_result_backend](#memory_result_backend)
- [task_scheduler](#task_scheduler)
- [cron_parser](#cron_parser)
- [task_monitor](#task_monitor)
- [task_system](#task_system)

---

## task

비동기적으로 실행될 작업 단위를 나타냅니다.

**헤더:** `<kcenon/messaging/task/task.h>`

### 열거형

#### task_state

```cpp
enum class task_state {
    pending,    // 큐 대기 중
    queued,     // 큐에 추가됨
    running,    // 현재 실행 중
    succeeded,  // 성공적으로 완료
    failed,     // 실행 실패
    retrying,   // 실패 후 재시도 중
    cancelled,  // 사용자에 의해 취소됨
    expired     // 실행 전 만료됨
};
```

### 생성자

```cpp
task();
task(const std::string& task_name);
task(const std::string& task_name, const container_module::value_container& payload);
task(const std::string& task_name, std::shared_ptr<container_module::value_container> payload);
```

### 식별 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `task_id()` | `const std::string&` | 고유 작업 식별자 반환 |
| `task_name()` | `const std::string&` | 이 작업의 핸들러 이름 반환 |

### 상태 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `state()` | `task_state` | 현재 작업 상태 반환 |
| `set_state(task_state)` | `void` | 작업 상태 설정 |
| `is_terminal_state()` | `bool` | 상태가 최종인지 확인 (succeeded, failed, cancelled, expired) |

### 설정 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `config()` | `const task_config&` | 작업 설정 반환 (const) |
| `config()` | `task_config&` | 작업 설정 반환 (수정 가능) |

### 실행 추적

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `attempt_count()` | `size_t` | 실행 시도 횟수 반환 |
| `increment_attempt()` | `void` | 시도 카운터 증가 |
| `started_at()` | `const time_point&` | 실행 시작 시간 반환 |
| `completed_at()` | `const time_point&` | 실행 완료 시간 반환 |

### 진행 상황 추적

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `progress()` | `double` | 진행률 반환 (0.0~1.0), 스레드 안전 |
| `set_progress(double)` | `void` | 진행률 값 설정 |
| `progress_message()` | `const std::string&` | 진행 메시지 반환 |
| `set_progress_message(const std::string&)` | `void` | 진행 메시지 설정 |

### 결과/에러 처리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `has_result()` | `bool` | 결과 사용 가능 여부 확인 |
| `result()` | `const value_container&` | 작업 결과 반환 |
| `set_result(shared_ptr<value_container>)` | `void` | 작업 결과 설정 |
| `has_error()` | `bool` | 에러 발생 여부 확인 |
| `error_message()` | `const std::string&` | 에러 메시지 반환 |
| `error_traceback()` | `const std::string&` | 에러 트레이스백 반환 |
| `set_error(message, traceback)` | `void` | 에러 정보 설정 |

### 재시도 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `is_expired()` | `bool` | 작업 만료 여부 확인 |
| `should_retry()` | `bool` | 재시도 필요 여부 확인 |
| `get_next_retry_delay()` | `milliseconds` | 백오프를 적용한 다음 재시도 지연 계산 |

### 직렬화

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `serialize()` | `Result<vector<uint8_t>>` | 작업을 바이트로 직렬화 |
| `deserialize(data)` | `Result<task>` | 정적: 바이트에서 작업 역직렬화 |

---

## task_config

작업 실행을 위한 설정 옵션입니다.

**헤더:** `<kcenon/messaging/task/task.h>`

### 필드

| 필드 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| `timeout` | `milliseconds` | 300000 (5분) | 최대 실행 시간 |
| `max_retries` | `size_t` | 3 | 최대 재시도 횟수 |
| `retry_delay` | `milliseconds` | 1000 | 초기 재시도 지연 |
| `retry_backoff_multiplier` | `double` | 2.0 | 백오프 승수 |
| `priority` | `message_priority` | normal | 작업 우선순위 수준 |
| `eta` | `optional<time_point>` | nullopt | 예약 실행 시간 |
| `expires` | `optional<milliseconds>` | nullopt | 만료 기간 |
| `queue_name` | `std::string` | "default" | 대상 큐 이름 |
| `tags` | `vector<string>` | {} | 필터링용 작업 태그 |

---

## task_builder

작업 구성을 위한 플루언트 빌더입니다.

**헤더:** `<kcenon/messaging/task/task.h>`

### 생성자

```cpp
explicit task_builder(const std::string& task_name);
```

### 빌더 메서드

모든 메서드는 체이닝을 위해 `task_builder&`를 반환합니다.

| 메서드 | 매개변수 | 설명 |
|--------|----------|------|
| `payload(const value_container&)` | 페이로드 데이터 | 작업 페이로드 설정 |
| `payload(shared_ptr<value_container>)` | 페이로드 포인터 | 작업 페이로드 설정 |
| `priority(message_priority)` | 우선순위 수준 | 우선순위 설정 |
| `timeout(milliseconds)` | 타임아웃 기간 | 타임아웃 설정 |
| `retries(size_t)` | 최대 재시도 | 최대 재시도 횟수 설정 |
| `retry_delay(milliseconds)` | 지연 기간 | 초기 재시도 지연 설정 |
| `backoff(double)` | 승수 | 백오프 승수 설정 |
| `queue(const string&)` | 큐 이름 | 대상 큐 설정 |
| `eta(time_point)` | 예약 시간 | 예약 실행 시간 설정 |
| `delay(milliseconds)` | 지연 기간 | 현재부터의 실행 지연 설정 |
| `expires_in(milliseconds)` | 기간 | 만료 설정 |
| `tag(const string&)` | 태그 이름 | 태그 추가 |
| `tags(const vector<string>&)` | 태그 목록 | 여러 태그 추가 |

### 빌드 메서드

```cpp
common::Result<task> build();
```

구성된 작업 또는 에러를 포함하는 `Result<task>`를 반환합니다.

### 예제

```cpp
auto result = task_builder("email.send")
    .payload(email_data)
    .priority(message_priority::high)
    .timeout(std::chrono::minutes(2))
    .retries(5)
    .queue("email-queue")
    .tag("notifications")
    .build();

if (result.is_ok()) {
    auto task = result.value();
}
```

---

## task_handler_interface

작업 핸들러를 위한 추상 인터페이스입니다.

**헤더:** `<kcenon/messaging/task/task_handler.h>`

### 순수 가상 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `name()` | `std::string` | 핸들러 이름 반환 |
| `execute(task, context)` | `Result<value_container>` | 작업 실행 |

### 가상 라이프사이클 훅

| 메서드 | 매개변수 | 설명 |
|--------|----------|------|
| `on_retry(task, attempt)` | 작업과 시도 횟수 | 재시도 전 호출 |
| `on_failure(task, error)` | 작업과 에러 메시지 | 실패 시 호출 |
| `on_success(task, result)` | 작업과 결과 | 성공 시 호출 |

### 예제

```cpp
class email_handler : public task_handler_interface {
public:
    std::string name() const override {
        return "email.send";
    }

    common::Result<value_container> execute(
        const task& t,
        task_context& ctx) override
    {
        auto to = t.payload().get_string("to").value();
        // 이메일 전송...
        value_container result;
        result.add("status", "sent");
        return common::ok(result);
    }

    void on_failure(const task& t, const std::string& error) override {
        // 실패 로깅, 알림 전송 등
    }
};
```

### 헬퍼 함수

```cpp
// 람다에서 핸들러 생성
template<typename Func>
std::shared_ptr<task_handler_interface> make_handler(
    const std::string& name,
    Func&& func);
```

---

## task_context

핸들러에 제공되는 실행 컨텍스트입니다.

**헤더:** `<kcenon/messaging/task/task_context.h>`

### 구조체

#### progress_info

```cpp
struct progress_info {
    double progress;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};
```

#### task_log_entry

```cpp
struct task_log_entry {
    enum class level { info, warning, error };
    level log_level;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
};
```

### 진행 상황 추적

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `update_progress(double, string)` | `void` | 진행률 업데이트 (0.0-1.0) 및 메시지 |
| `progress()` | `double` | 현재 진행률 반환 |
| `progress_history()` | `vector<progress_info>` | 진행 기록 반환 |

### 체크포인트 관리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `save_checkpoint(value_container)` | `void` | 체크포인트 상태 저장 |
| `save_checkpoint(shared_ptr<value_container>)` | `void` | 체크포인트 상태 저장 |
| `load_checkpoint()` | `value_container` | 저장된 체크포인트 로드 |
| `has_checkpoint()` | `bool` | 체크포인트 존재 확인 |
| `clear_checkpoint()` | `void` | 체크포인트 삭제 |

### 하위 작업 관리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `set_subtask_spawner(spawner)` | `void` | 스포너 함수 설정 |
| `spawn_subtask(task)` | `Result<string>` | 하위 작업 생성, 작업 ID 반환 |
| `spawned_subtask_ids()` | `vector<string>` | 생성된 하위 작업 ID 반환 |

### 취소

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `is_cancelled()` | `bool` | 취소 요청 확인 |
| `request_cancellation()` | `void` | 취소 요청 |

### 로깅

| 메서드 | 매개변수 | 설명 |
|--------|----------|------|
| `log_info(message)` | 로그 메시지 | info 레벨 메시지 로깅 |
| `log_warning(message)` | 로그 메시지 | warning 레벨 메시지 로깅 |
| `log_error(message)` | 로그 메시지 | error 레벨 메시지 로깅 |
| `logs()` | - | 모든 로그 항목 반환 |

---

## task_queue

다중 명명 큐로 작업 큐잉을 관리합니다.

**헤더:** `<kcenon/messaging/task/task_queue.h>`

### 설정

```cpp
struct task_queue_config {
    size_t max_size = 100000;
    bool enable_persistence = false;
    std::string persistence_path;
    bool enable_delayed_queue = true;
    std::chrono::milliseconds delayed_poll_interval{1000};
};
```

### 생성자

```cpp
explicit task_queue(const task_queue_config& config = {});
```

### 라이프사이클

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `start()` | `VoidResult` | 큐 시작 |
| `stop()` | `void` | 큐 중지 |
| `is_running()` | `bool` | 실행 중인지 확인 |

### 인큐 작업

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `enqueue(task)` | `Result<string>` | 작업 추가, 작업 ID 반환 |
| `enqueue_bulk(vector<task>)` | `Result<vector<string>>` | 여러 작업 추가 |

### 디큐 작업

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `dequeue(queues, timeout)` | `Result<task>` | 큐에서 작업 대기 |
| `try_dequeue(queues)` | `Result<task>` | 논블로킹 디큐 |

### 취소

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `cancel(task_id)` | `VoidResult` | 작업 ID로 취소 |
| `cancel_by_tag(tag)` | `VoidResult` | 태그로 취소 |

### 쿼리 작업

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get_task(task_id)` | `Result<task>` | ID로 작업 반환 |
| `queue_size(queue_name)` | `size_t` | 큐 크기 반환 |
| `total_size()` | `size_t` | 전체 작업 수 반환 |
| `delayed_size()` | `size_t` | 지연된 작업 수 반환 |
| `list_queues()` | `vector<string>` | 큐 이름 목록 반환 |
| `has_queue(name)` | `bool` | 큐 존재 확인 |

---

## worker_pool

작업 실행을 위한 스레드 풀입니다.

**헤더:** `<kcenon/messaging/task/worker_pool.h>`

### 설정

```cpp
struct worker_config {
    size_t concurrency = std::thread::hardware_concurrency();
    std::vector<std::string> queues = {"default"};
    std::chrono::milliseconds poll_interval{100};
    bool prefetch = true;
    size_t prefetch_count = 10;
};
```

### 통계

```cpp
struct worker_statistics {
    size_t total_tasks_processed = 0;
    size_t total_tasks_succeeded = 0;
    size_t total_tasks_failed = 0;
    size_t total_tasks_retried = 0;
    size_t total_tasks_timed_out = 0;
    std::chrono::milliseconds total_execution_time{0};
    std::chrono::milliseconds avg_execution_time{0};
    std::chrono::system_clock::time_point started_at;
    std::chrono::system_clock::time_point last_task_at;
};
```

### 핸들러 등록

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `register_handler(shared_ptr<handler>)` | `void` | 클래스 기반 핸들러 등록 |
| `register_handler(name, func)` | `void` | 람다 핸들러 등록 |
| `unregister_handler(name)` | `bool` | 핸들러 등록 해제 |
| `has_handler(name)` | `bool` | 핸들러 존재 확인 |
| `list_handlers()` | `vector<string>` | 핸들러 이름 목록 반환 |

### 라이프사이클

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `start()` | `VoidResult` | 워커 풀 시작 |
| `stop()` | `VoidResult` | 즉시 중지 |
| `shutdown_graceful(timeout)` | `VoidResult` | 우아한 종료 |

### 상태

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `is_running()` | `bool` | 실행 중인지 확인 |
| `active_workers()` | `size_t` | 활성 워커 수 |
| `idle_workers()` | `size_t` | 유휴 워커 수 |
| `total_workers()` | `size_t` | 전체 워커 수 |

### 통계

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get_statistics()` | `worker_statistics` | 실행 통계 반환 |
| `reset_statistics()` | `void` | 통계 초기화 |

---

## task_client

작업 제출을 위한 고수준 API입니다.

**헤더:** `<kcenon/messaging/task/task_client.h>`

### 생성자

```cpp
task_client(
    std::shared_ptr<task_queue> queue,
    std::shared_ptr<result_backend_interface> backend);
```

### 즉시 실행

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `send(task)` | `async_result` | 작업 제출 |
| `send(name, payload)` | `async_result` | 이름과 페이로드로 제출 |

### 지연 실행

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `send_later(task, delay)` | `async_result` | 지연과 함께 제출 |
| `send_at(task, time_point)` | `async_result` | 특정 시간에 제출 |

### 배치 작업

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `send_batch(vector<task>)` | `vector<async_result>` | 여러 작업 제출 |

### 워크플로우 패턴

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `chain(vector<task>)` | `async_result` | 순차 실행 |
| `chord(tasks, callback)` | `async_result` | 병렬 실행 후 콜백 |

### 결과/취소

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get_result(task_id)` | `async_result` | 결과 핸들 반환 |
| `cancel(task_id)` | `VoidResult` | 작업 취소 |
| `cancel_by_tag(tag)` | `VoidResult` | 태그로 취소 |

### 상태

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `pending_count(queue)` | `size_t` | 대기 중인 작업 수 |
| `is_connected()` | `bool` | 연결 상태 확인 |

---

## async_result

비동기 작업 결과를 위한 핸들입니다.

**헤더:** `<kcenon/messaging/task/async_result.h>`

### 상태 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `task_id()` | `const string&` | 작업 ID 반환 |
| `is_valid()` | `bool` | 핸들 유효 여부 확인 |
| `state()` | `task_state` | 현재 상태 반환 |
| `is_ready()` | `bool` | 완료 여부 확인 |
| `is_successful()` | `bool` | 성공 여부 확인 |
| `is_failed()` | `bool` | 실패 여부 확인 |
| `is_cancelled()` | `bool` | 취소 여부 확인 |

### 진행 상황 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `progress()` | `double` | 진행률 반환 (0.0-1.0) |
| `progress_message()` | `string` | 진행 메시지 반환 |

### 블로킹 결과 조회

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get(timeout)` | `Result<value_container>` | 결과 대기 |
| `wait(timeout)` | `bool` | 완료 대기 |

### 콜백 기반

```cpp
void then(
    std::function<void(const value_container&)> on_success,
    std::function<void(const std::string&)> on_failure = nullptr);
```

### 작업 제어

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `revoke()` | `VoidResult` | 작업 취소 |
| `children()` | `vector<async_result>` | 자식 작업 핸들 반환 |
| `add_child(task_id)` | `void` | 자식 작업 추가 |
| `error_message()` | `string` | 에러 메시지 반환 |
| `error_traceback()` | `string` | 에러 트레이스백 반환 |

---

## result_backend_interface

결과 저장을 위한 추상 인터페이스입니다.

**헤더:** `<kcenon/messaging/task/result_backend.h>`

### 구조체

```cpp
struct progress_data {
    double progress{0.0};
    std::string message;
    std::chrono::system_clock::time_point updated_at;
};

struct error_data {
    std::string message;
    std::string traceback;
    std::chrono::system_clock::time_point occurred_at;
};
```

### 순수 가상 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `store_state(task_id, state)` | `VoidResult` | 작업 상태 저장 |
| `store_result(task_id, result)` | `VoidResult` | 결과 저장 |
| `store_error(task_id, error, traceback)` | `VoidResult` | 에러 저장 |
| `store_progress(task_id, progress, message)` | `VoidResult` | 진행 상황 저장 |
| `get_state(task_id)` | `Result<task_state>` | 상태 반환 |
| `get_result(task_id)` | `Result<value_container>` | 결과 반환 |
| `get_progress(task_id)` | `Result<progress_data>` | 진행 상황 반환 |
| `get_error(task_id)` | `Result<error_data>` | 에러 반환 |
| `wait_for_result(task_id, timeout)` | `Result<value_container>` | 블로킹 대기 |
| `cleanup_expired(max_age)` | `VoidResult` | 오래된 항목 정리 |

### 선택적 가상 메서드

| 메서드 | 반환 타입 | 기본값 | 설명 |
|--------|----------|--------|------|
| `exists(task_id)` | `bool` | false | 존재 여부 확인 |
| `remove(task_id)` | `VoidResult` | 무동작 | 항목 제거 |
| `size()` | `size_t` | 0 | 항목 수 반환 |

---

## memory_result_backend

result_backend_interface의 인메모리 구현입니다.

**헤더:** `<kcenon/messaging/task/memory_result_backend.h>`

### 생성자

```cpp
memory_result_backend();
```

### 추가 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `clear()` | `void` | 저장된 모든 데이터 삭제 |

`result_backend_interface`의 모든 메서드가 구현되어 있습니다.

---

## task_scheduler

주기적 및 cron 기반 작업 스케줄링을 관리합니다.

**헤더:** `<kcenon/messaging/task/scheduler.h>`

### 구조체

```cpp
struct schedule_entry {
    std::string name;
    task task_template;
    std::variant<std::chrono::seconds, std::string> schedule;
    bool enabled = true;
    std::optional<time_point> last_run;
    std::optional<time_point> next_run;
    size_t run_count = 0;
    size_t failure_count = 0;

    bool is_cron() const;
    bool is_periodic() const;
    std::string cron_expression() const;
    std::chrono::seconds interval() const;
};
```

### 생성자

```cpp
explicit task_scheduler(task_client& client);
```

### 스케줄 등록

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `add_periodic(name, task, interval)` | `VoidResult` | 주기적 스케줄 추가 |
| `add_cron(name, task, expression)` | `VoidResult` | cron 스케줄 추가 |

### 스케줄 관리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `remove(name)` | `VoidResult` | 스케줄 제거 |
| `enable(name)` | `VoidResult` | 스케줄 활성화 |
| `disable(name)` | `VoidResult` | 스케줄 비활성화 |
| `trigger_now(name)` | `VoidResult` | 즉시 실행 트리거 |
| `update_interval(name, interval)` | `VoidResult` | 주기적 간격 업데이트 |
| `update_cron(name, expression)` | `VoidResult` | cron 표현식 업데이트 |

### 라이프사이클

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `start()` | `VoidResult` | 스케줄러 시작 |
| `stop()` | `VoidResult` | 스케줄러 중지 |
| `is_running()` | `bool` | 실행 중인지 확인 |

### 쿼리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `list_schedules()` | `vector<schedule_entry>` | 모든 스케줄 목록 반환 |
| `get_schedule(name)` | `Result<schedule_entry>` | 특정 스케줄 반환 |
| `schedule_count()` | `size_t` | 스케줄 수 반환 |
| `has_schedule(name)` | `bool` | 스케줄 존재 확인 |

### 이벤트 콜백

```cpp
using schedule_callback = std::function<void(const schedule_entry&)>;

void on_task_executed(schedule_callback callback);
void on_task_failed(schedule_callback callback);
```

---

## cron_parser

Cron 표현식 파싱 및 평가를 위한 유틸리티입니다.

**헤더:** `<kcenon/messaging/task/cron_parser.h>`

### Cron 표현식 형식

```
* * * * *
│ │ │ │ │
│ │ │ │ └─ 요일 (0-6, 0=일요일)
│ │ │ └─── 월 (1-12)
│ │ └───── 일 (1-31)
│ └─────── 시 (0-23)
└───────── 분 (0-59)
```

### 지원 구문

- `*` - 모든 값
- `5` - 특정 값
- `*/15` - N 단위마다
- `1-5` - 범위
- `1,3,5` - 목록

### 구조체

```cpp
struct cron_expression {
    std::set<int> minutes;   // 0-59
    std::set<int> hours;     // 0-23
    std::set<int> days;      // 1-31
    std::set<int> months;    // 1-12
    std::set<int> weekdays;  // 0-6
};
```

### 정적 메서드

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `parse(expr)` | `Result<cron_expression>` | cron 문자열 파싱 |
| `next_run_time(expr, from)` | `Result<time_point>` | 다음 실행 시간 계산 |
| `is_valid(expr)` | `bool` | 표현식 유효성 검사 |
| `to_string(expr)` | `string` | 문자열로 변환 |

### 예제

```cpp
// 매 시간 0분에 실행
auto expr = cron_parser::parse("0 * * * *").value();

// 매일 오전 3시에 실행
auto expr = cron_parser::parse("0 3 * * *").value();

// 15분마다 실행
auto expr = cron_parser::parse("*/15 * * * *").value();

// 평일 오전 9시에 실행
auto expr = cron_parser::parse("0 9 * * 1-5").value();
```

---

## task_monitor

시스템 모니터링 및 이벤트 구독입니다.

**헤더:** `<kcenon/messaging/task/monitor.h>`

### 구조체

```cpp
struct queue_stats {
    std::string name;
    size_t pending_count = 0;
    size_t running_count = 0;
    size_t delayed_count = 0;
};

struct worker_info {
    std::string worker_id;
    std::vector<std::string> queues;
    size_t active_tasks = 0;
    std::chrono::system_clock::time_point last_heartbeat;
    bool is_healthy = true;
};
```

### 생성자

```cpp
task_monitor(
    std::shared_ptr<task_queue> queue,
    worker_pool* workers);
```

### 큐 통계

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get_queue_stats()` | `vector<queue_stats>` | 모든 큐 통계 |
| `get_queue_stats(name)` | `Result<queue_stats>` | 특정 큐 통계 |

### 워커 상태

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get_workers()` | `vector<worker_info>` | 모든 워커 정보 |
| `get_worker_statistics()` | `optional<worker_statistics>` | 워커 풀 통계 |

### 작업 쿼리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `list_active_tasks()` | `vector<task>` | 실행 중인 작업 |
| `list_pending_tasks(queue)` | `vector<task>` | 대기 중인 작업 |
| `list_failed_tasks(limit)` | `vector<task>` | 실패한 작업 |

### 작업 관리

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `cancel_task(task_id)` | `VoidResult` | 작업 취소 |
| `retry_task(task_id)` | `VoidResult` | 실패한 작업 재시도 |
| `purge_queue(name)` | `VoidResult` | 큐 비우기 |

### 이벤트 구독

```cpp
using task_started_handler = std::function<void(const task&)>;
using task_completed_handler = std::function<void(const task&, bool success)>;
using task_failed_handler = std::function<void(const task&, const std::string& error)>;
using worker_offline_handler = std::function<void(const std::string& worker_id)>;

void on_task_started(task_started_handler handler);
void on_task_completed(task_completed_handler handler);
void on_task_failed(task_failed_handler handler);
void on_worker_offline(worker_offline_handler handler);
```

---

## task_system

모든 작업 컴포넌트를 통합하는 통합 파사드입니다.

**헤더:** `<kcenon/messaging/task/task_system.h>`

### 설정

```cpp
struct task_system_config {
    task_queue_config queue;
    worker_config worker;
    bool enable_scheduler = true;
    bool enable_monitoring = true;
    std::string result_backend_type = "memory";
};
```

### 생성자

```cpp
explicit task_system(const task_system_config& config = {});
```

### 라이프사이클

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `start()` | `VoidResult` | 모든 컴포넌트 시작 |
| `stop()` | `VoidResult` | 모든 컴포넌트 중지 |
| `shutdown_graceful(timeout)` | `VoidResult` | 우아한 종료 |
| `is_running()` | `bool` | 실행 중인지 확인 |

### 컴포넌트 접근

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `client()` | `task_client&` | 작업 클라이언트 반환 |
| `workers()` | `worker_pool&` | 워커 풀 반환 |
| `scheduler()` | `task_scheduler*` | 스케줄러 반환 (nullable) |
| `monitor()` | `task_monitor*` | 모니터 반환 (nullable) |
| `queue()` | `shared_ptr<task_queue>` | 큐 반환 |
| `results()` | `shared_ptr<result_backend>` | 결과 백엔드 반환 |

### 핸들러 등록 (편의 메서드)

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `register_handler(shared_ptr<handler>)` | `void` | 핸들러 등록 |
| `register_handler(name, func)` | `void` | 람다 등록 |
| `unregister_handler(name)` | `bool` | 핸들러 등록 해제 |

### 작업 제출 (편의 메서드)

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `submit(name, payload)` | `async_result` | 작업 제출 |
| `submit(task)` | `async_result` | 작업 제출 |
| `submit_later(task, delay)` | `async_result` | 지연 제출 |
| `submit_batch(tasks)` | `vector<async_result>` | 배치 제출 |

### 스케줄링 (편의 메서드)

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `schedule_periodic(name, task, interval)` | `VoidResult` | 주기적 추가 |
| `schedule_cron(name, task, expression)` | `VoidResult` | cron 추가 |

### 통계

| 메서드 | 반환 타입 | 설명 |
|--------|----------|------|
| `get_statistics()` | `worker_statistics` | 워커 통계 반환 |
| `pending_count(queue)` | `size_t` | 대기 중인 작업 수 |
| `active_workers()` | `size_t` | 활성 워커 수 |
| `total_workers()` | `size_t` | 전체 워커 수 |

---

## 관련 문서

- [빠른 시작 가이드](QUICK_START_KO.md)
- [아키텍처 가이드](ARCHITECTURE_KO.md)
- [패턴 가이드](PATTERNS_KO.md)
- [설정 가이드](CONFIGURATION_KO.md)
- [문제 해결](TROUBLESHOOTING_KO.md)
