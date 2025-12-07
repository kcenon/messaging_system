# Task 모듈 빠른 시작 가이드

5분 만에 Task 모듈을 시작하세요.

## 사전 요구사항

- C++17 이상
- CMake 3.16+
- C++ 컴파일러 (GCC 8+, Clang 10+, 또는 MSVC 2019+)

## 설치

Task 모듈은 messaging_system 라이브러리에 포함되어 있습니다. CMake로 빌드하세요:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## 기본 사용법

### 1. 헤더 포함

```cpp
#include <kcenon/messaging/task/task_system.h>
#include <kcenon/messaging/task/task.h>
```

### 2. Task 시스템 생성 및 설정

```cpp
#include <kcenon/messaging/task/task_system.h>

int main() {
    using namespace kcenon::messaging::task;

    // 시스템 설정
    task_system_config config;
    config.worker.concurrency = 4;  // 4개의 워커 스레드 사용
    config.worker.queues = {"default", "high-priority"};

    // Task 시스템 생성
    task_system system(config);

    // ... 핸들러 등록 및 시작
}
```

### 3. Task 핸들러 등록

```cpp
// 람다 기반 핸들러
system.register_handler("greet", [](const task& t, task_context& ctx) {
    auto name = t.payload().get_string("name").value_or("World");

    container_module::value_container result;
    result.add("message", "안녕하세요, " + name + "님!");

    return common::ok(result);
});
```

### 4. 시스템 시작

```cpp
auto start_result = system.start();
if (!start_result) {
    std::cerr << "시작 실패: " << start_result.error().message << "\n";
    return 1;
}
```

### 5. Task 제출

```cpp
// 페이로드 생성
container_module::value_container payload;
payload.add("name", "홍길동");

// Task 제출 및 비동기 결과 핸들 획득
auto result = system.submit("greet", payload);

// 타임아웃과 함께 결과 대기
auto outcome = result.get(std::chrono::seconds(10));
if (outcome.is_ok()) {
    std::cout << outcome.value().get_string("message").value() << "\n";
    // 출력: 안녕하세요, 홍길동님!
} else {
    std::cerr << "Task 실패: " << outcome.error().message << "\n";
}
```

### 6. 시스템 중지

```cpp
system.stop();
```

## 전체 예제

```cpp
#include <kcenon/messaging/task/task_system.h>
#include <iostream>

int main() {
    using namespace kcenon::messaging::task;

    // 1. 설정
    task_system_config config;
    config.worker.concurrency = 2;

    // 2. 시스템 생성
    task_system system(config);

    // 3. 핸들러 등록
    system.register_handler("add", [](const task& t, task_context& ctx) {
        auto a = t.payload().get_int("a").value_or(0);
        auto b = t.payload().get_int("b").value_or(0);

        ctx.update_progress(0.5, "계산 중...");

        container_module::value_container result;
        result.add("sum", a + b);

        ctx.update_progress(1.0, "완료");
        return common::ok(result);
    });

    // 4. 시작
    system.start();

    // 5. Task 제출
    container_module::value_container payload;
    payload.add("a", 10);
    payload.add("b", 20);

    auto result = system.submit("add", payload);

    // 6. 결과 획득
    auto outcome = result.get(std::chrono::seconds(5));
    if (outcome.is_ok()) {
        std::cout << "합계: " << outcome.value().get_int("sum").value() << "\n";
        // 출력: 합계: 30
    }

    // 7. 중지
    system.stop();

    return 0;
}
```

## Task 빌더 사용하기

복잡한 Task 설정을 위해 Task 빌더를 사용하세요:

```cpp
auto task = task_builder("email.send")
    .payload(email_payload)
    .priority(message_priority::high)
    .timeout(std::chrono::minutes(2))
    .retries(5)
    .queue("email-queue")
    .tag("notifications")
    .build();

if (task) {
    auto result = system.submit(task.value());
}
```

## Task 스케줄링

### 주기적 실행

```cpp
// 5분마다 실행
system.schedule_periodic(
    "cleanup-job",
    task_builder("cleanup.temp").build().value(),
    std::chrono::minutes(5)
);
```

### Cron 기반 실행

```cpp
// 매일 오전 3시에 실행
system.schedule_cron(
    "daily-report",
    task_builder("report.generate").build().value(),
    "0 3 * * *"
);
```

## 진행 상황 모니터링

```cpp
auto result = system.submit("long-running-task", payload);

// 진행 상황 폴링
while (!result.is_ready()) {
    std::cout << "진행률: " << (result.progress() * 100) << "% - "
              << result.progress_message() << "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
```

## 에러 처리

```cpp
auto result = system.submit("risky-task", payload);
auto outcome = result.get(std::chrono::seconds(30));

if (outcome.is_ok()) {
    // 성공 처리
    auto value = outcome.value();
} else {
    // 실패 처리
    std::cerr << "에러: " << outcome.error().message << "\n";

    // 결과에서 상세 에러 정보 획득
    if (result.is_failed()) {
        std::cerr << "트레이스백: " << result.error_traceback() << "\n";
    }
}
```

## 다음 단계

- [아키텍처 가이드](ARCHITECTURE_KO.md) - 시스템 설계 이해하기
- [API 레퍼런스](API_REFERENCE_KO.md) - 완전한 API 문서
- [패턴 가이드](PATTERNS_KO.md) - 워크플로우 패턴 배우기 (chain, chord)
- [설정 가이드](CONFIGURATION_KO.md) - 환경에 맞게 설정하기
- [문제 해결](TROUBLESHOOTING_KO.md) - 일반적인 문제와 해결책
