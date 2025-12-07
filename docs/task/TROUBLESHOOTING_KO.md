# Task 모듈 문제 해결 가이드

이 가이드는 Task 모듈의 일반적인 문제를 진단하고 해결하는 데 도움을 줍니다.

## 목차

- [일반적인 문제](#일반적인-문제)
- [디버깅 방법](#디버깅-방법)
- [성능 문제](#성능-문제)
- [FAQ](#faq)

---

## 일반적인 문제

### 작업이 처리되지 않음

**증상:**
- 작업이 큐에 남아 있음
- `async_result.is_ready()`가 `true`를 반환하지 않음
- 큐 크기가 계속 증가함

**가능한 원인과 해결책:**

1. **작업 이름에 대한 핸들러가 등록되지 않음**

   ```cpp
   // 핸들러 존재 확인
   if (!system.workers().has_handler("my.task")) {
       std::cerr << "핸들러 'my.task'가 등록되지 않았습니다!\n";
   }

   // 등록된 모든 핸들러 목록
   auto handlers = system.workers().list_handlers();
   for (const auto& h : handlers) {
       std::cout << "등록됨: " << h << "\n";
   }
   ```

2. **시스템이 시작되지 않음**

   ```cpp
   // 시스템이 실행 중인지 확인
   if (!system.is_running()) {
       auto result = system.start();
       if (!result) {
           std::cerr << "시작 실패: " << result.error().message << "\n";
       }
   }
   ```

3. **잘못된 큐 이름**

   ```cpp
   // 작업이 워커가 모니터링하지 않는 큐에 제출됨
   auto task = task_builder("my.task")
       .queue("custom-queue")  // 워커가 이 큐를 듣고 있지 않을 수 있음
       .build();

   // 워커가 해당 큐를 모니터링하도록 보장
   worker_config config;
   config.queues = {"default", "custom-queue"};  // 큐 추가
   ```

4. **모든 워커가 바쁨**

   ```cpp
   // 워커 상태 확인
   auto stats = system.get_statistics();
   std::cout << "활성: " << system.active_workers()
             << " / " << system.total_workers() << "\n";
   ```

### 작업이 즉시 실패함

**증상:**
- `async_result.is_failed()`가 `true` 반환
- 작업이 너무 빨리 완료됨
- 에러 메시지 출현

**가능한 원인과 해결책:**

1. **핸들러가 예외를 던짐**

   ```cpp
   // 핸들러는 throw 대신 Result를 반환해야 함
   system.register_handler("safe.task", [](const task& t, task_context& ctx) {
       try {
           // 로직
           return common::ok(result);
       } catch (const std::exception& e) {
           return common::error(error_code::execution_failed, e.what());
       }
   });
   ```

2. **잘못된 페이로드 데이터**

   ```cpp
   // 처리 전 페이로드 유효성 검사
   auto required_field = t.payload().get_string("required");
   if (!required_field) {
       return common::error(error_code::validation_failed,
                          "필수 필드 누락");
   }
   ```

3. **리소스 사용 불가**

   ```cpp
   // 외부 종속성 확인
   if (!database.is_connected()) {
       ctx.log_error("데이터베이스 연결 끊김");
       return common::error(error_code::resource_unavailable,
                          "데이터베이스 사용 불가");
   }
   ```

### 작업 타임아웃

**증상:**
- 작업이 전체 타임아웃 시간 동안 실행된 후 실패
- `error_message()`에 타임아웃 언급

**가능한 원인과 해결책:**

1. **핸들러가 너무 오래 걸림**

   ```cpp
   // 긴 작업에 대해 타임아웃 증가
   auto task = task_builder("long.task")
       .timeout(std::chrono::minutes(30))
       .build();
   ```

2. **핸들러가 블로킹됨**

   핸들러가 다음 상태인지 확인:
   - 응답하지 않는 외부 서비스를 대기 중
   - 로직에 데드락이 있음
   - 무한 루프에 빠짐

3. **블로킹 작업 확인**

   ```cpp
   // 가능하면 비동기 작업 사용
   // 긴 루프에서 취소 확인
   for (size_t i = 0; i < items.size(); ++i) {
       if (ctx.is_cancelled()) {
           return common::error("취소됨");
       }
       process_item(items[i]);
       ctx.update_progress(double(i) / items.size(), "처리 중...");
   }
   ```

### 재시도가 작동하지 않음

**증상:**
- 실패한 작업이 재시도되지 않음
- 재시도 횟수가 0으로 유지됨

**가능한 원인과 해결책:**

1. **max_retries가 0으로 설정됨**

   ```cpp
   auto task = task_builder("retryable.task")
       .retries(3)  // 재시도 활성화
       .retry_delay(std::chrono::seconds(5))
       .build();
   ```

2. **재시도 불가능한 에러 반환**

   일부 에러는 재시도를 건너뛰도록 설정될 수 있습니다. 핸들러가 적절한 에러를 반환하는지 확인하세요.

3. **작업이 이미 최대 재시도에 도달**

   ```cpp
   // 시도 횟수 확인
   std::cout << "시도: " << t.attempt_count()
             << " / " << t.config().max_retries << "\n";
   ```

### 메모리 사용량 증가

**증상:**
- 시간이 지남에 따라 프로세스 메모리가 증가
- 시스템이 느려짐
- 결국 메모리 부족

**가능한 원인과 해결책:**

1. **결과가 정리되지 않음**

   ```cpp
   // 주기적으로 오래된 결과 정리
   system.results()->cleanup_expired(std::chrono::hours(24));
   ```

2. **큐가 무제한 증가**

   ```cpp
   // 최대 큐 크기 설정
   task_queue_config queue_config;
   queue_config.max_size = 100000;
   ```

3. **진행 기록 축적**

   진행 업데이트가 메모리에 저장됩니다. 자주 업데이트되는 매우 긴 작업의 경우 축적될 수 있습니다.

4. **메모리 오프로드를 위해 영속성 활성화**

   ```cpp
   task_queue_config queue_config;
   queue_config.enable_persistence = true;
   queue_config.persistence_path = "/var/lib/task_queue/";
   ```

---

## 디버깅 방법

### 로깅 활성화

task context를 사용하여 정보 로깅:

```cpp
system.register_handler("debug.task", [](const task& t, task_context& ctx) {
    ctx.log_info("작업 시작: " + t.task_id());
    ctx.log_info("페이로드: " + t.payload().to_string());

    // ... 처리 ...

    ctx.log_info("작업 성공적으로 완료");
    return common::ok(result);
});
```

### 이벤트 모니터링

디버깅을 위해 작업 이벤트 구독:

```cpp
auto monitor = system.monitor();
if (monitor) {
    monitor->on_task_started([](const task& t) {
        std::cout << "[시작됨] " << t.task_name()
                  << " (ID: " << t.task_id() << ")\n";
    });

    monitor->on_task_completed([](const task& t, bool success) {
        std::cout << "[" << (success ? "성공" : "실패") << "] "
                  << t.task_name() << "\n";
    });

    monitor->on_task_failed([](const task& t, const std::string& error) {
        std::cout << "[실패] " << t.task_name()
                  << ": " << error << "\n";
    });
}
```

### 큐 상태 확인

```cpp
// 큐 통계 가져오기
auto monitor = system.monitor();
if (monitor) {
    auto stats = monitor->get_queue_stats();
    for (const auto& q : stats) {
        std::cout << "큐 '" << q.name << "': "
                  << "대기=" << q.pending_count
                  << ", 실행중=" << q.running_count
                  << ", 지연=" << q.delayed_count << "\n";
    }
}
```

### 워커 상태 확인

```cpp
auto stats = system.get_statistics();
std::cout << "처리된 작업: " << stats.total_tasks_processed << "\n";
std::cout << "성공한 작업: " << stats.total_tasks_succeeded << "\n";
std::cout << "실패한 작업: " << stats.total_tasks_failed << "\n";
std::cout << "재시도한 작업: " << stats.total_tasks_retried << "\n";
std::cout << "평균 실행 시간: " << stats.avg_execution_time.count() << "ms\n";
```

### 작업 로그 보기

완료된 작업에서 로그 접근:

```cpp
// 로그는 task_context에 저장됨
system.register_handler("logging.task", [](const task& t, task_context& ctx) {
    ctx.log_info("단계 1 완료");
    ctx.log_warning("서비스 응답 느림");
    ctx.log_error("연결 실패, 재시도 중");

    // 실행 후 로그 사용 가능
    auto logs = ctx.logs();
    for (const auto& entry : logs) {
        std::cout << "[" << entry.timestamp << "] "
                  << level_to_string(entry.log_level) << ": "
                  << entry.message << "\n";
    }

    return common::ok(result);
});
```

---

## 성능 문제

### 높은 CPU 사용량

**가능한 원인:**

1. **워커가 너무 많음**

   ```cpp
   // 동시성 줄이기
   worker_config config;
   config.concurrency = std::thread::hardware_concurrency() / 2;
   ```

2. **폴링 간격이 너무 짧음**

   ```cpp
   // 폴링 간격 늘리기
   worker_config config;
   config.poll_interval = std::chrono::milliseconds(100);
   ```

3. **CPU 집약적 핸들러**

   무거운 계산을 전용 스레드로 이동하거나 작업을 배치로 처리하는 것을 고려하세요.

### 높은 메모리 사용량

**가능한 원인:**

1. **큐에 큰 페이로드**

   ```cpp
   // 큐 크기 제한
   task_queue_config config;
   config.max_size = 10000;
   ```

2. **프리페치된 작업이 너무 많음**

   ```cpp
   // 프리페치 수 줄이기
   worker_config config;
   config.prefetch_count = 5;
   ```

3. **결과 축적**

   ```cpp
   // 오래된 결과 정리
   system.results()->cleanup_expired(std::chrono::hours(1));
   ```

### 낮은 처리량

**가능한 원인:**

1. **워커 부족**

   ```cpp
   // 동시성 늘리기
   worker_config config;
   config.concurrency = std::thread::hardware_concurrency() * 2;
   ```

2. **핸들러에서 I/O 병목**

   가능하면 비동기 I/O 또는 배치 작업을 사용하세요.

3. **프리페치 비활성화**

   ```cpp
   // 프리페칭 활성화
   worker_config config;
   config.prefetch = true;
   config.prefetch_count = 20;
   ```

4. **단일 큐 병목**

   ```cpp
   // 여러 큐 사용
   worker_config config;
   config.queues = {"high", "default", "low"};
   ```

### 높은 지연 시간

**가능한 원인:**

1. **긴 폴링 간격**

   ```cpp
   // 폴링 간격 줄이기
   worker_config config;
   config.poll_interval = std::chrono::milliseconds(10);
   ```

2. **큐 깊이가 너무 높음**

   큐 깊이를 모니터링하고 과부하 시 워커를 확장하거나 새 작업을 거부하세요.

3. **프리페치가 작업을 보유**

   ```cpp
   // 지연에 민감한 큐에서 프리페치 비활성화
   worker_config config;
   config.prefetch = false;
   ```

---

## FAQ

### Q: 실행 중인 작업을 어떻게 취소하나요?

작업은 취소할 수 있지만 핸들러가 취소를 확인해야 합니다:

```cpp
system.register_handler("cancellable.task", [](const task& t, task_context& ctx) {
    while (processing) {
        if (ctx.is_cancelled()) {
            // 정리하고 반환
            return common::error("사용자가 작업 취소함");
        }
        // 처리 계속
    }
    return common::ok(result);
});

// 취소 요청
auto result = system.submit("cancellable.task", payload);
result.revoke();  // 취소 플래그 설정
```

### Q: 핸들러 외부에서 작업 진행 상황을 어떻게 얻나요?

```cpp
auto result = system.submit("long.task", payload);

// 진행 상황 폴링
while (!result.is_ready()) {
    double progress = result.progress();
    std::string message = result.progress_message();
    std::cout << "진행률: " << (progress * 100) << "% - " << message << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

### Q: 작업 종속성을 어떻게 처리하나요?

Chain 패턴 사용:

```cpp
auto step1 = task_builder("step1").payload(data).build().value();
auto step2 = task_builder("step2").build().value();
auto step3 = task_builder("step3").build().value();

auto result = client.chain({step1, step2, step3});
// step2는 step1의 결과를 받고, step3는 step2의 결과를 받음
```

### Q: 작업을 순서대로 처리하려면 어떻게 하나요?

엄격한 순서를 위해 `concurrency = 1` 설정:

```cpp
worker_config config;
config.concurrency = 1;  // 단일 워커가 순서 보장
config.queues = {"ordered-queue"};
```

### Q: 특정 작업의 우선순위를 어떻게 높이나요?

우선순위 수준과 큐 이름 사용:

```cpp
auto urgent = task_builder("urgent.task")
    .priority(message_priority::highest)
    .queue("critical")
    .build();

auto normal = task_builder("normal.task")
    .priority(message_priority::normal)
    .queue("default")
    .build();

// critical 큐를 먼저 처리하도록 워커 설정
worker_config config;
config.queues = {"critical", "default", "background"};
```

### Q: 재시도 횟수를 어떻게 제한하나요?

task_config를 통해 설정:

```cpp
auto task = task_builder("limited.retry")
    .retries(1)  // 한 번만 재시도
    .build();

// 또는 재시도 완전히 비활성화
auto task = task_builder("no.retry")
    .retries(0)  // 재시도 없음
    .build();
```

### Q: 독약 메시지(항상 실패하는 작업)를 어떻게 처리하나요?

`on_failure` 훅을 사용하여 실패 추적:

```cpp
class monitored_handler : public task_handler_interface {
    std::unordered_map<std::string, size_t> failure_counts_;

public:
    void on_failure(const task& t, const std::string& error) override {
        failure_counts_[t.task_name()]++;

        if (failure_counts_[t.task_name()] > 10) {
            // 데드 레터 큐로 이동하거나 알림
            log_alert("작업 " + t.task_name() + " 반복적으로 실패 중");
        }
    }
};
```

### Q: 시스템을 우아하게 종료하려면 어떻게 하나요?

```cpp
// 우아한 종료는 현재 작업이 완료될 때까지 대기
system.shutdown_graceful(std::chrono::seconds(30));

// 이것은:
// 1. 새 작업 수락 중지
// 2. 실행 중인 작업이 완료될 때까지 최대 30초 대기
// 3. 타임아웃에 도달하면 강제 중지
```

---

## 관련 문서

- [빠른 시작 가이드](QUICK_START_KO.md)
- [아키텍처 가이드](ARCHITECTURE_KO.md)
- [API 레퍼런스](API_REFERENCE_KO.md)
- [패턴 가이드](PATTERNS_KO.md)
- [설정 가이드](CONFIGURATION_KO.md)
