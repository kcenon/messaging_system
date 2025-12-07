# Task 모듈 마이그레이션 가이드

이 가이드는 Task 모듈의 이전 버전이나 다른 작업 큐 시스템에서 마이그레이션하는 데 도움을 줍니다.

## 목차

- [버전 마이그레이션](#버전-마이그레이션)
- [커스텀 솔루션에서 마이그레이션](#커스텀-솔루션에서-마이그레이션)
- [마이그레이션 체크리스트](#마이그레이션-체크리스트)

---

## 버전 마이그레이션

### 1.0으로 마이그레이션

이전 개발 버전에서 업그레이드하는 경우 다음 단계를 따르세요:

#### API 변경 사항

1. **Result 타입 패턴**

   이제 모든 작업이 `Result<T>` 또는 `VoidResult`를 반환합니다:

   ```cpp
   // 이전 (가상의 이전 API)
   task t = create_task("name", payload);  // 예외를 던질 수 있음

   // 이후
   auto result = task_builder("name").payload(payload).build();
   if (!result) {
       std::cerr << result.error().message << "\n";
       return;
   }
   task t = result.value();
   ```

2. **작업용 빌더 패턴**

   ```cpp
   // 이전
   task t;
   t.set_name("task.name");
   t.set_timeout(5000);

   // 이후
   auto t = task_builder("task.name")
       .timeout(std::chrono::milliseconds(5000))
       .build()
       .value();
   ```

3. **설정 구조**

   ```cpp
   // 이전
   system.set_worker_count(4);
   system.add_queue("default");

   // 이후
   task_system_config config;
   config.worker.concurrency = 4;
   config.worker.queues = {"default"};
   task_system system(config);
   ```

4. **핸들러 등록**

   ```cpp
   // 이전
   system.add_handler("name", handler_function);

   // 이후
   system.register_handler("name", [](const task& t, task_context& ctx) {
       // 구현
       return common::ok(result);
   });
   ```

#### 설정 마이그레이션

이전 설정을 새 구조에 매핑:

| 이전 설정 | 새 설정 |
|-----------|---------|
| `worker_threads` | `config.worker.concurrency` |
| `queue_size` | `config.queue.max_size` |
| `retry_count` | `task_config.max_retries` |
| `timeout_ms` | `task_config.timeout` |

---

## 커스텀 솔루션에서 마이그레이션

### 스레드 풀 구현에서 마이그레이션

커스텀 스레드 풀이 있다면 Task 모듈로 마이그레이션:

#### 이전 (커스텀 스레드 풀)

```cpp
class ThreadPool {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;

public:
    void enqueue(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        tasks.push(std::move(task));
    }
};

ThreadPool pool(4);
pool.enqueue([data]() {
    process_data(data);
});
```

#### 이후 (Task 모듈)

```cpp
task_system_config config;
config.worker.concurrency = 4;
task_system system(config);

system.register_handler("process.data", [](const task& t, task_context& ctx) {
    auto data = t.payload();
    process_data(data);

    container_module::value_container result;
    result.add("status", "processed");
    return common::ok(result);
});

system.start();

container_module::value_container payload;
payload.add("data", data);
auto result = system.submit("process.data", payload);
```

#### 마이그레이션의 이점

- 지수 백오프를 통한 자동 재시도
- 진행 상황 추적
- 작업 영속성
- 모니터링 및 통계
- 예약 실행
- Chain 및 Chord 패턴

### 메시지 큐 시스템에서 마이그레이션

#### 개념 매핑

| 메시지 큐 개념 | Task 모듈 동등물 |
|----------------|------------------|
| 메시지 | `task` |
| 큐 | `task_queue`의 명명된 큐 |
| 컨슈머 | `task_handler` |
| 프로듀서 | `task_client` |
| 승인 | 핸들러 완료 시 자동 |
| 데드 레터 큐 | 결과 백엔드의 실패한 작업 |

#### 마이그레이션 예제

```cpp
// 이전 (메시지 큐 의사 코드)
producer.send("queue-name", message);
consumer.subscribe("queue-name", [](Message& msg) {
    process(msg);
    msg.ack();
});

// 이후
system.register_handler("process", [](const task& t, task_context& ctx) {
    process(t.payload());
    return common::ok(result);
});

auto task = task_builder("process")
    .payload(message_data)
    .queue("queue-name")
    .build().value();

system.submit(task);
```

### Cron 작업에서 마이그레이션

#### 이전 (시스템 Cron)

```bash
# /etc/crontab
0 3 * * * /usr/bin/my-daily-job
*/15 * * * * /usr/bin/my-sync-job
```

#### 이후 (Task 스케줄러)

```cpp
// 매일 오전 3시
system.schedule_cron(
    "daily-job",
    task_builder("daily.task").build().value(),
    "0 3 * * *"
);

// 15분마다
system.schedule_cron(
    "sync-job",
    task_builder("sync.task").build().value(),
    "*/15 * * * *"
);
```

#### 이점

- 중앙화된 스케줄링
- 실패 시 재시도
- 진행 모니터링
- 외부 종속성 없음
- 프로그래밍 방식 스케줄 관리

---

## 마이그레이션 체크리스트

### 마이그레이션 전

- [ ] 모든 기존 작업/잡 목록화
- [ ] 현재 재시도 및 타임아웃 동작 문서화
- [ ] 모든 큐 이름 및 우선순위 기록
- [ ] 현재 스케줄링 패턴 기록
- [ ] 기존 데이터 및 설정 백업

### 마이그레이션 중

- [ ] 모든 작업에 대한 동등한 핸들러 생성
- [ ] 적절한 설정으로 task_system 구성
- [ ] 동등한 큐 구조 설정
- [ ] 스케줄링 설정 마이그레이션
- [ ] 필요한 곳에 진행 추적 구현
- [ ] 모니터링 및 로깅 설정

### 마이그레이션 후

- [ ] 모든 작업이 올바르게 실행되는지 확인
- [ ] 재시도 동작이 예상과 일치하는지 확인
- [ ] 스케줄링이 예상대로 작동하는지 확인
- [ ] 성능 모니터링 및 설정 조정
- [ ] 이전 작업 인프라 제거
- [ ] 문서 업데이트

### 테스트 체크리스트

- [ ] 모든 핸들러에 대한 단위 테스트
- [ ] 작업 흐름에 대한 통합 테스트
- [ ] 재시도 시나리오 테스트
- [ ] 타임아웃 동작 테스트
- [ ] Chain 및 Chord 패턴 테스트
- [ ] 예약 작업 테스트
- [ ] 성능/부하 테스트

---

## 일반적인 마이그레이션 문제

### 핸들러가 에러 반환 대신 예외를 던짐

```cpp
// 잘못됨 - 예기치 않은 동작 유발
system.register_handler("bad", [](const task& t, task_context& ctx) {
    if (error_condition) {
        throw std::runtime_error("에러!");
    }
    return common::ok(result);
});

// 올바름 - 에러 결과 반환
system.register_handler("good", [](const task& t, task_context& ctx) {
    if (error_condition) {
        return common::error(error_code::execution_failed, "에러!");
    }
    return common::ok(result);
});
```

### 누락된 큐 설정

```cpp
// 작업이 "special-queue"로 감
auto task = task_builder("task")
    .queue("special-queue")
    .build().value();

// 하지만 워커는 "default"만 모니터링
worker_config config;
config.queues = {"default"};  // "special-queue" 누락!

// 수정: 필요한 모든 큐 추가
config.queues = {"default", "special-queue"};
```

### 타임아웃이 너무 짧음

```cpp
// 이전 시스템은 타임아웃이 없어 작업이 영원히 실행됨
// 새 시스템은 기본값이 5분

// 긴 작업의 경우 타임아웃 증가
auto task = task_builder("long.task")
    .timeout(std::chrono::hours(2))
    .build().value();
```

---

## 관련 문서

- [빠른 시작 가이드](QUICK_START_KO.md)
- [아키텍처 가이드](ARCHITECTURE_KO.md)
- [API 레퍼런스](API_REFERENCE_KO.md)
- [설정 가이드](CONFIGURATION_KO.md)
- [문제 해결](TROUBLESHOOTING_KO.md)
