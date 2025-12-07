# Task 모듈 워크플로우 패턴

이 가이드는 Task 모듈의 일반적인 워크플로우 패턴을 다룹니다.

## 목차

- [Chain 패턴](#chain-패턴)
- [Chord 패턴](#chord-패턴)
- [재시도 전략](#재시도-전략)
- [우선순위 큐](#우선순위-큐)
- [예약 작업](#예약-작업)
- [진행 상황 추적](#진행-상황-추적)
- [체크포인트와 복구](#체크포인트와-복구)
- [하위 작업 생성](#하위-작업-생성)

---

## Chain 패턴

Chain 패턴은 작업을 순차적으로 실행하며, 하나의 작업 결과가 다음 작업의 입력이 됩니다.

### 개념

```
작업 A → 작업 B → 작업 C
          ↑          ↑
      A의 결과    B의 결과
```

### 사용법

```cpp
#include <kcenon/messaging/task/task_client.h>

// 작업 정의
auto task_a = task_builder("step.one")
    .payload({{"input", "data"}})
    .build().value();

auto task_b = task_builder("step.two").build().value();
auto task_c = task_builder("step.three").build().value();

// 체인 실행
auto result = client.chain({task_a, task_b, task_c});

// 최종 결과 대기
auto outcome = result.get(std::chrono::minutes(5));
if (outcome.is_ok()) {
    // task_c의 결과입니다
    auto final_result = outcome.value();
}
```

### 핸들러 구현

```cpp
// 각 핸들러는 이전 작업의 결과를 받습니다
system.register_handler("step.two", [](const task& t, task_context& ctx) {
    // 이전 작업(step.one)의 결과 가져오기
    auto previous_result = t.payload().get_string("_chain_result");

    // 처리 후 다음 작업을 위한 결과 반환
    container_module::value_container result;
    result.add("processed", "data from step two");

    return common::ok(result);
});
```

### 에러 처리

체인의 어떤 작업이든 실패하면 전체 체인이 중지되고 에러가 전파됩니다:

```cpp
auto result = client.chain({task_a, task_b, task_c});
auto outcome = result.get(timeout);

if (outcome.is_error()) {
    // 체인이 어느 시점에서 실패함
    std::cerr << "체인 실패: " << result.error_message() << "\n";
}
```

### 사용 사례

- 다단계 데이터 처리 파이프라인
- 순차적 워크플로우 실행
- 순서대로 실행해야 하는 의존적 작업

---

## Chord 패턴

Chord 패턴은 여러 작업을 병렬로 실행한 후, 집계된 결과로 콜백 작업을 실행합니다.

### 개념

```
작업 A ─┐
작업 B ─┼→ 콜백 작업 (모든 결과 포함)
작업 C ─┘
```

### 사용법

```cpp
// 병렬 작업 정의
auto task_a = task_builder("process.part1")
    .payload({{"data", "chunk1"}})
    .build().value();

auto task_b = task_builder("process.part2")
    .payload({{"data", "chunk2"}})
    .build().value();

auto task_c = task_builder("process.part3")
    .payload({{"data", "chunk3"}})
    .build().value();

// 콜백 작업 정의
auto callback = task_builder("aggregate.results").build().value();

// 코드 실행
auto result = client.chord({task_a, task_b, task_c}, callback);

// 최종 결과 대기
auto outcome = result.get(std::chrono::minutes(10));
```

### 콜백 핸들러

콜백은 모든 결과를 배열로 받습니다:

```cpp
system.register_handler("aggregate.results", [](const task& t, task_context& ctx) {
    // 병렬 작업의 모든 결과 가져오기
    auto results = t.payload().get_array("_chord_results");

    // 집계된 결과 처리
    int total = 0;
    for (const auto& r : results) {
        total += r.get_int("count").value_or(0);
    }

    container_module::value_container result;
    result.add("total", total);

    return common::ok(result);
});
```

### 에러 처리

병렬 작업 중 하나라도 실패하면 코드가 실패합니다:

```cpp
auto result = client.chord(tasks, callback);
auto outcome = result.get(timeout);

if (outcome.is_error()) {
    // 하나 이상의 병렬 작업이 실패함
    std::cerr << "코드 실패: " << result.error_message() << "\n";
}
```

### 사용 사례

- Map-reduce 작업
- 병렬 데이터 처리
- 여러 소스의 결과 집계
- Fan-out/fan-in 워크플로우

---

## 재시도 전략

Task 모듈은 지수 백오프를 사용한 자동 재시도를 지원합니다.

### 기본 설정

```cpp
task_config config;
config.max_retries = 3;
config.retry_delay = std::chrono::milliseconds(1000);
config.retry_backoff_multiplier = 2.0;
```

### 재시도 지연 계산

```
지연 = retry_delay * (backoff_multiplier ^ 시도 횟수)

기본값 예시:
- 시도 1 실패 → 1000ms 대기
- 시도 2 실패 → 2000ms 대기
- 시도 3 실패 → 4000ms 대기
- 시도 4 실패 → 작업이 실패로 표시됨
```

### Task 빌더 사용

```cpp
auto task = task_builder("email.send")
    .payload(email_data)
    .retries(5)                              // 최대 5회 재시도
    .retry_delay(std::chrono::seconds(2))    // 2초 지연으로 시작
    .backoff(1.5)                            // 1.5배 승수
    .build();
```

### 커스텀 재시도 로직

커스텀 동작을 위해 `on_retry` 훅 구현:

```cpp
class resilient_handler : public task_handler_interface {
public:
    std::string name() const override { return "resilient.task"; }

    common::Result<value_container> execute(
        const task& t, task_context& ctx) override
    {
        // 작업 구현
    }

    void on_retry(const task& t, size_t attempt) override {
        // 재시도 시도 로깅
        std::cout << "작업 " << t.task_id()
                  << " 재시도 중 (시도 " << attempt << ")\n";

        // 메트릭 업데이트, 알림 전송 등도 가능
    }
};
```

### 재시도 vs 재시도하지 않을 에러

특정 에러에서 재시도를 방지하려면:

```cpp
common::Result<value_container> execute(const task& t, task_context& ctx) override {
    auto validation = validate_input(t.payload());
    if (!validation) {
        // 재시도하면 안 되는 에러 반환
        // 작업이 즉시 실패로 표시됨
        return common::error(error_code::validation_failed, validation.error());
    }

    // 일시적 에러는 자동으로 재시도됨
    return process_data(t.payload());
}
```

---

## 우선순위 큐

작업에 다른 우선순위 수준을 할당하여 실행 순서를 제어할 수 있습니다.

### 우선순위 수준

```cpp
enum class message_priority {
    lowest = 1,
    low = 3,
    normal = 5,    // 기본값
    high = 7,
    highest = 9
};
```

### 우선순위 설정

```cpp
// task 빌더 사용
auto urgent_task = task_builder("notification.send")
    .payload(notification_data)
    .priority(message_priority::highest)
    .build();

// task_config 사용
task t("report.generate", payload);
t.config().priority = message_priority::low;
```

### 우선순위 큐 설정

```cpp
task_system_config config;
config.worker.queues = {"critical", "high", "default", "low"};
config.worker.concurrency = 8;

task_system system(config);
```

### 우선순위 큐에 제출

```cpp
// 중요 알림은 critical 큐로
auto critical = task_builder("alert.send")
    .queue("critical")
    .priority(message_priority::highest)
    .build();

// 백그라운드 작업은 low 우선순위 큐로
auto background = task_builder("cleanup.logs")
    .queue("low")
    .priority(message_priority::lowest)
    .build();
```

### 모범 사례

1. **highest 우선순위를 남용하지 마세요** - 목적을 무효화합니다
2. **큐와 우선순위를 일치시키세요** - 높은 우선순위 작업은 높은 우선순위 큐로
3. **큐 깊이를 모니터링하세요** - 높은 우선순위 큐가 고갈되지 않도록
4. **적절한 타임아웃을 설정하세요** - 낮은 우선순위 작업은 더 오래 대기할 수 있습니다

---

## 예약 작업

### 주기적 실행

고정 간격으로 작업 실행:

```cpp
// 5분마다
system.schedule_periodic(
    "cleanup-temp-files",
    task_builder("cleanup.temp").build().value(),
    std::chrono::minutes(5)
);

// 매 시간
system.schedule_periodic(
    "sync-data",
    task_builder("sync.external").build().value(),
    std::chrono::hours(1)
);
```

### Cron 기반 스케줄링

복잡한 스케줄을 위한 cron 표현식 사용:

```cpp
// 매일 오전 3시
system.schedule_cron(
    "daily-report",
    task_builder("report.daily").build().value(),
    "0 3 * * *"
);

// 매주 월요일 오전 9시
system.schedule_cron(
    "weekly-summary",
    task_builder("report.weekly").build().value(),
    "0 9 * * 1"
);

// 평일 업무 시간(9-17시) 동안 15분마다
system.schedule_cron(
    "business-sync",
    task_builder("sync.crm").build().value(),
    "*/15 9-17 * * 1-5"
);
```

### Cron 표현식 형식

```
* * * * *
│ │ │ │ │
│ │ │ │ └─ 요일 (0-6, 0=일요일)
│ │ │ └─── 월 (1-12)
│ │ └───── 일 (1-31)
│ └─────── 시 (0-23)
└───────── 분 (0-59)

지원 구문:
- *     모든 값
- 5     특정 값
- */15  15 단위마다
- 1-5   범위
- 1,3,5 목록
```

### 스케줄 관리

```cpp
// 스케줄 일시 비활성화
system.scheduler()->disable("daily-report");

// 다시 활성화
system.scheduler()->enable("daily-report");

// 즉시 트리거 (스케줄 외)
system.scheduler()->trigger_now("daily-report");

// 간격 업데이트
system.scheduler()->update_interval("cleanup-temp-files",
    std::chrono::minutes(10));

// 스케줄 제거
system.scheduler()->remove("obsolete-job");
```

### 지연 작업 실행

지연 후 작업 실행:

```cpp
// 30초 후 실행
auto result = client.send_later(
    task_builder("reminder.send").payload(data).build().value(),
    std::chrono::seconds(30)
);

// 특정 시간에 실행
auto tomorrow_9am = /* time_point 계산 */;
auto result = client.send_at(
    task_builder("meeting.reminder").payload(data).build().value(),
    tomorrow_9am
);
```

---

## 진행 상황 추적

실행 중 작업 진행 상황을 보고하고 모니터링합니다.

### 진행 상황 보고

```cpp
system.register_handler("large-file.process", [](const task& t, task_context& ctx) {
    auto file_path = t.payload().get_string("path").value();
    auto total_lines = count_lines(file_path);

    size_t processed = 0;
    for (const auto& line : read_lines(file_path)) {
        process_line(line);
        processed++;

        // 진행 상황 업데이트
        double progress = static_cast<double>(processed) / total_lines;
        ctx.update_progress(progress,
            "라인 " + std::to_string(processed) +
            " / " + std::to_string(total_lines) + " 처리 중");
    }

    ctx.update_progress(1.0, "완료");

    container_module::value_container result;
    result.add("lines_processed", processed);
    return common::ok(result);
});
```

### 진행 상황 모니터링

```cpp
auto result = system.submit("large-file.process", payload);

// 진행 상황 폴링
while (!result.is_ready()) {
    double progress = result.progress();
    std::string message = result.progress_message();

    std::cout << "\r진행률: " << std::fixed << std::setprecision(1)
              << (progress * 100) << "% - " << message << std::flush;

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

std::cout << "\n완료!\n";
```

### 진행 기록

```cpp
// 상세 진행 기록 가져오기
auto history = result.progress_history();
for (const auto& entry : history) {
    std::cout << entry.timestamp << ": "
              << (entry.progress * 100) << "% - "
              << entry.message << "\n";
}
```

---

## 체크포인트와 복구

실패 후 복구를 위해 진행 상황을 저장합니다.

### 체크포인트 저장

```cpp
system.register_handler("batch.process", [](const task& t, task_context& ctx) {
    auto items = t.payload().get_array("items");

    // 이전 시도의 체크포인트가 있는지 확인
    size_t start_index = 0;
    if (ctx.has_checkpoint()) {
        auto checkpoint = ctx.load_checkpoint();
        start_index = checkpoint.get_size_t("last_processed").value_or(0);
        ctx.log_info("인덱스 " + std::to_string(start_index) +
                     "에서 체크포인트 재개");
    }

    // 항목 처리
    for (size_t i = start_index; i < items.size(); ++i) {
        // 취소 확인
        if (ctx.is_cancelled()) {
            return common::error("작업 취소됨");
        }

        process_item(items[i]);

        // 10개 항목마다 체크포인트 저장
        if ((i + 1) % 10 == 0) {
            container_module::value_container checkpoint;
            checkpoint.add("last_processed", i + 1);
            ctx.save_checkpoint(checkpoint);
        }

        ctx.update_progress(
            static_cast<double>(i + 1) / items.size(),
            std::to_string(i + 1) + "개 항목 처리됨");
    }

    ctx.clear_checkpoint();

    container_module::value_container result;
    result.add("processed_count", items.size());
    return common::ok(result);
});
```

### 체크포인트 데이터

복구에 필요한 모든 데이터 저장:

```cpp
container_module::value_container checkpoint;
checkpoint.add("current_page", page_number);
checkpoint.add("processed_ids", processed_ids);
checkpoint.add("intermediate_results", partial_results);
ctx.save_checkpoint(checkpoint);
```

---

## 하위 작업 생성

핸들러 내에서 자식 작업을 생성합니다.

### 하위 작업 생성

```cpp
system.register_handler("order.process", [](const task& t, task_context& ctx) {
    auto order_items = t.payload().get_array("items");

    std::vector<std::string> subtask_ids;

    // 각 항목에 대해 하위 작업 생성
    for (const auto& item : order_items) {
        auto subtask = task_builder("item.process")
            .payload(item)
            .build().value();

        auto result = ctx.spawn_subtask(subtask);
        if (result.is_ok()) {
            subtask_ids.push_back(result.value());
        }
    }

    ctx.log_info(std::to_string(subtask_ids.size()) + "개 하위 작업 생성됨");

    // 부모는 계속하거나 하위 작업을 대기할 수 있음
    container_module::value_container result;
    result.add("subtask_ids", subtask_ids);
    return common::ok(result);
});
```

### 하위 작업 추적

```cpp
auto result = system.submit("order.process", order_data);
auto outcome = result.get(timeout);

// 자식 작업 결과 가져오기
for (const auto& child : result.children()) {
    if (child.is_ready()) {
        auto child_result = child.get(std::chrono::seconds(0));
        // 자식 결과 처리
    }
}
```

### 사용 사례

- 대규모 작업을 작은 단위로 분할
- 작업 내 병렬 처리
- 동적 워크플로우 생성

---

## 패턴 조합

### 재시도가 있는 Chain

```cpp
// 체인의 각 작업은 자체 재시도 정책을 가짐
auto step1 = task_builder("extract")
    .retries(3)
    .build().value();

auto step2 = task_builder("transform")
    .retries(5)
    .retry_delay(std::chrono::seconds(5))
    .build().value();

auto step3 = task_builder("load")
    .retries(3)
    .build().value();

auto result = client.chain({step1, step2, step3});
```

### 예약된 Chord

```cpp
// 병렬 처리 작업 예약
auto header_tasks = create_header_tasks();
auto callback = task_builder("aggregate.headers").build().value();

system.schedule_cron(
    "hourly-header-check",
    // chord 래퍼 작업 생성
    task_builder("chord.wrapper")
        .payload({{"header_tasks", header_tasks}, {"callback", callback}})
        .build().value(),
    "0 * * * *"  // 매 시간
);
```

---

## 관련 문서

- [빠른 시작 가이드](QUICK_START_KO.md)
- [아키텍처 가이드](ARCHITECTURE_KO.md)
- [API 레퍼런스](API_REFERENCE_KO.md)
- [설정 가이드](CONFIGURATION_KO.md)
- [문제 해결](TROUBLESHOOTING_KO.md)
