# Messaging System 기능

**버전**: 0.1.2
**최종 수정일**: 2025-12-24
**언어**: [English](FEATURES.md) | [한국어]

---

## 개요

이 문서는 messaging_system의 핵심 메시징 기능, 고급 패턴, 태스크 큐 시스템, 통합 옵션 및 신뢰성 기능을 포함한 모든 기능을 포괄적으로 설명합니다.

---

## 목차

1. [핵심 메시징](#핵심-메시징)
2. [고급 패턴](#고급-패턴)
3. [태스크 큐 시스템](#태스크-큐-시스템)
4. [C++20 Concepts](#c20-concepts)
5. [백엔드 지원](#백엔드-지원)
6. [프로덕션 기능](#프로덕션-기능)
7. [기능 매트릭스](#기능-매트릭스)

---

## 핵심 메시징

### 메시지 버스

중앙 pub/sub 코디네이터:
- 토픽 기반 라우팅
- 다중 구독자 지원
- 비동기 메시지 전달
- 우선순위 큐 지원
- 내장 통계

```cpp
auto bus = std::make_shared<message_bus>(backend, config);
bus->start();

// 비동기 발행
bus->publish(msg);

// 동기 구독
bus->subscribe("user.*", [](const message& msg) {
    return common::VoidResult::ok();
});
```

### 토픽 라우터

와일드카드 패턴 매칭:
- `*`: 단일 레벨 와일드카드
- `#`: 다중 레벨 와일드카드
- 정확한 매칭

### 메시지 브로커

**고급 라우팅 기능을 갖춘 중앙 메시지 라우팅 컴포넌트**

기능:
- **라우트 관리**: 라우트 동적 추가, 제거, 활성화, 비활성화
- **토픽 패턴 매칭**: topic_router 통합을 통한 와일드카드 지원 (`*`, `#`)
- **우선순위 기반 정렬**: 우선순위 순으로 라우트 처리 (높은 순)
- **통계 수집**: 라우팅된 메시지, 전달된 메시지, 실패한 메시지, 미라우팅 메시지 추적
- **스레드 안전 연산**: shared_mutex를 사용한 동시 접근

```cpp
message_broker broker;
broker.start();

// 사용자 이벤트 라우트 추가
broker.add_route("user-handler", "user.*", [](const message& msg) {
    // 사용자 메시지 처리
    return common::ok();
}, 5);  // 우선순위 5

// 메시지 라우팅
message msg("user.created");
broker.route(msg);

// 통계 확인
auto stats = broker.get_statistics();
std::cout << "라우팅됨: " << stats.messages_routed << std::endl;

broker.stop();
```

**계획된 기능** (이슈 #181, #182, #183 참조):
- 콘텐츠 기반 라우팅
- 데드 레터 큐
- 변환 파이프라인

### 메시지 큐

스레드 안전 우선순위 큐:
- 우선순위 기반 정렬
- 용량 제한
- 백프레셔 지원

### 메시지 직렬화

container_system 기반 페이로드:
- 타입 안전 직렬화
- 자동 역직렬화
- 스키마 진화

---

## 고급 패턴

### Pub/Sub

퍼블리셔와 구독자 헬퍼:
```cpp
auto publisher = std::make_shared<patterns::publisher>(bus, "events");
publisher->publish(msg);

auto subscriber = std::make_shared<patterns::subscriber>(bus);
subscriber->subscribe("events.*", callback);
```

### Request/Reply

비동기 메시징 위의 동기 RPC:
```cpp
// 서버
auto server = std::make_shared<request_server>(bus, "calculator");
server->register_handler(handler);

// 클라이언트
auto client = std::make_shared<request_client>(bus);
auto response = client->request(request, timeout);
```

### Event Streaming

리플레이 기능의 이벤트 소싱:
```cpp
auto stream = std::make_shared<event_stream>(bus, "orders");

// 이벤트 발행
stream->publish_event("order.created", data);

// 타임스탬프부터 리플레이
stream->replay_from(timestamp, callback);
```

### Message Pipeline

파이프-필터 처리:
```cpp
auto pipeline = pipeline_builder()
    .add_stage("validate", validate_fn)
    .add_stage("transform", transform_fn)
    .add_stage("enrich", enrich_fn)
    .build();

auto result = pipeline.process(msg);
```

---

## 태스크 큐 시스템

### 개요

**백그라운드 작업 처리를 위한 분산 태스크 큐**

태스크 큐 시스템은 우선순위 큐, 예약 실행, 재시도 메커니즘, 실시간 모니터링 기능을 갖춘 완전한 분산 태스크 처리 솔루션을 제공합니다.

### 태스크 시스템 파사드

**모든 태스크 작업을 위한 통합 인터페이스**

기능:
- **단일 진입점**: 태스크 제출 및 관리를 위한 통합 파사드
- **컴포넌트 조정**: 워커 풀, 스케줄러, 모니터 조정
- **설정**: 모든 컴포넌트의 중앙 집중식 설정
- **생명주기 관리**: 모든 컴포넌트 함께 시작/종료

```cpp
auto system = std::make_shared<task_system>(config);
system->start();

// 태스크 제출
auto result = system->submit("process.image", payload);

// 결과 대기
auto output = result.get();
```

### 태스크 클라이언트

**다양한 실행 모드의 태스크 제출**

기능:
- **즉시 실행**: 즉시 처리를 위한 태스크 제출
- **지연 실행**: 미래 실행을 위한 태스크 예약
- **우선순위 지원**: 다른 우선순위 레벨로 제출
- **비동기 결과**: 결과 추적을 위한 비동기 핸들

```cpp
auto client = std::make_shared<task_client>(queue, backend);

// 우선순위와 함께 제출
auto result = client->submit_with_priority(task, task_priority::high);

// 지연 제출
auto delayed = client->submit_delayed(task, std::chrono::seconds(30));
```

### 워커 풀

**핸들러 등록이 가능한 설정 가능 워커 스레드**

기능:
- **스레드 풀**: 설정 가능한 워커 스레드 수
- **핸들러 등록**: 태스크 이름으로 핸들러 등록
- **로드 밸런싱**: 자동 작업 분배
- **우아한 종료**: 종료 전 대기 중인 태스크 완료

```cpp
auto pool = std::make_shared<worker_pool>(config);

// 핸들러 등록
pool->register_handler("email.send", email_handler);
pool->register_handler("image.resize", image_handler);

pool->start();
```

### 태스크 스케줄러

**주기적 및 크론 기반 태스크 스케줄링**

기능:
- **주기적 스케줄링**: 고정 간격으로 태스크 실행
- **크론 표현식**: 표준 5필드 크론 표현식 지원
- **스케줄 관리**: 스케줄 활성화/비활성화/제거
- **실행 콜백**: 실행 이벤트 훅

```cpp
auto scheduler = std::make_shared<scheduler>(client);

// 주기적 태스크 (5분마다)
scheduler->schedule_periodic("cleanup", task, std::chrono::minutes(5));

// 크론 태스크 (매일 자정)
scheduler->schedule_cron("daily_report", task, "0 0 * * *");
```

### 비동기 결과

**진행 상태 지원이 있는 비동기 결과 추적**

기능:
- **상태 추적**: 태스크 상태 확인 (대기, 실행 중, 완료, 실패)
- **진행률 업데이트**: 실시간 진행률 퍼센티지
- **타임아웃 지원**: 설정 가능한 타임아웃으로 대기
- **체이닝**: 워크플로우 조정을 위한 결과 체이닝

```cpp
auto result = client->submit(task);

// 상태 확인
if (result.is_pending()) {
    // 아직 대기 중
}

// 타임아웃으로 대기
auto output = result.wait_for(std::chrono::seconds(30));

// 진행률 가져오기
double progress = result.progress();
```

### 결과 백엔드

**플러그형 결과 저장소**

기능:
- **메모리 백엔드**: 개발/테스트용 인메모리 저장소
- **인터페이스**: 커스텀 백엔드를 위한 추상 인터페이스
- **TTL 지원**: 자동 결과 만료
- **정리**: 만료된 결과의 주기적 정리

```cpp
auto backend = std::make_shared<memory_result_backend>();

// 결과 저장
backend->store(task_id, result);

// 결과 조회
auto stored = backend->get(task_id);
```

### 태스크 모니터

**실시간 태스크 모니터링 및 통계**

기능:
- **큐 통계**: 대기, 실행 중, 완료 수
- **워커 통계**: 활성 워커, 활용률
- **성능 메트릭**: 처리량, 지연시간
- **이벤트 콜백**: 태스크 이벤트 구독

```cpp
auto monitor = std::make_shared<monitor>(pool, backend);

// 통계 가져오기
auto stats = monitor->get_statistics();
std::cout << "대기 중: " << stats.pending_count << std::endl;
std::cout << "실행 중: " << stats.running_count << std::endl;

// 이벤트 구독
monitor->on_task_completed([](const task& t) {
    std::cout << "태스크 " << t.id() << " 완료" << std::endl;
});
```

### Chain 및 Chord 패턴

**워크플로우 조정 패턴**

기능:
- **Chain**: 순차적 태스크 실행
- **Chord**: 최종 콜백과 함께 병렬 실행
- **에러 처리**: 적절한 에러 전파
- **결과 집계**: 여러 태스크의 결과 결합

```cpp
// Chain: task1 -> task2 -> task3
auto chain_result = client->chain({task1, task2, task3});

// Chord: [task1, task2, task3] -> callback
auto chord_result = client->chord({task1, task2, task3}, callback_task);
```

### 재시도 메커니즘

**지수 백오프가 있는 자동 재시도**

기능:
- **설정 가능한 재시도**: 태스크당 최대 재시도 횟수 설정
- **지수 백오프**: 재시도 간 증가하는 지연
- **재시도 콜백**: 재시도 이벤트 훅
- **최종 실패**: 모든 재시도 소진 시 콜백

```cpp
auto task = task_builder()
    .name("send.email")
    .max_retries(3)
    .retry_delay(std::chrono::seconds(5))
    .build();
```

### 태스크 타임아웃

**장시간 실행 태스크를 위한 타임아웃 처리**

기능:
- **태스크별 타임아웃**: 태스크별 타임아웃 설정
- **취소**: 타임아웃된 태스크 취소
- **타임아웃 콜백**: 타임아웃 이벤트 훅

```cpp
auto task = task_builder()
    .name("process.video")
    .timeout(std::chrono::minutes(10))
    .build();
```

---

## C++20 Concepts

### 개요

**C++20 Concepts를 사용한 타입 안전 콜백 검증**

메시징 시스템은 C++20 Concepts를 사용하여 콜백과 핸들러의 컴파일 타임 타입 검증을 제공합니다. 이를 통해 더 명확한 에러 메시지와 자체 문서화되는 인터페이스 요구사항을 얻을 수 있습니다.

### TaskHandlerCallable

**태스크 핸들러 시그니처 검증**

```cpp
template<typename F>
concept TaskHandlerCallable = std::invocable<F, const task&, task_context&> &&
    std::same_as<std::invoke_result_t<F, const task&, task_context&>,
                 common::Result<container_module::value_container>>;

// 사용법
template<TaskHandlerCallable Handler>
void register_handler(const std::string& name, Handler&& handler);
```

### TaskHandlerLike

**태스크 핸들러 인터페이스 구현 검증**

```cpp
template<typename T>
concept TaskHandlerLike = requires(T t, const task& tsk, task_context& ctx) {
    { t.name() } -> std::convertible_to<std::string>;
    { t.execute(tsk, ctx) } -> std::same_as<common::Result<container_module::value_container>>;
};
```

### ScheduleEventCallable

**스케줄러 이벤트 콜백 검증**

```cpp
template<typename F>
concept ScheduleEventCallable = std::invocable<F, const schedule_entry&>;

// 사용법
template<ScheduleEventCallable Callback>
void on_task_executed(Callback&& callback);
```

### MessageProcessorCallable

**메시지 파이프라인 프로세서 검증**

```cpp
template<typename F>
concept MessageProcessorCallable = std::invocable<F, const message&> &&
    std::same_as<std::invoke_result_t<F, const message&>,
                 common::Result<message>>;
```

### MessageFilterCallable

**메시지 필터링 술어 검증**

```cpp
template<typename F>
concept MessageFilterCallable = std::invocable<F, const message&> &&
    std::convertible_to<std::invoke_result_t<F, const message&>, bool>;
```

### MessageTransformerCallable

**메시지 변환기 검증**

```cpp
template<typename F>
concept MessageTransformerCallable = std::invocable<F, const message&> &&
    std::same_as<std::invoke_result_t<F, const message&>, message>;
```

### SubscriptionCallable

**토픽 구독 콜백 검증**

```cpp
template<typename F>
concept SubscriptionCallable = std::invocable<F, const message&> &&
    std::same_as<std::invoke_result_t<F, const message&>,
                 common::VoidResult>;
```

### 장점

- **컴파일 타임 검증**: 컴파일 시 타입 불일치 감지
- **명확한 에러 메시지**: SFINAE보다 나은 진단
- **자체 문서화**: 인터페이스 요구사항이 코드에 명시적
- **IDE 지원**: 더 나은 자동 완성 및 타입 추론

---

## 백엔드 지원

### 독립 실행

자체 포함 실행:
- 내장 스레드 풀
- 자동 리소스 관리

```cpp
auto backend = std::make_shared<standalone_backend>(num_threads);
backend->initialize();
```

### 스레드 풀 통합

외부 실행자 사용:
- IExecutor 인터페이스
- 커스텀 스레드 풀

```cpp
auto backend = std::make_shared<integration_backend>(
    thread_pool,
    logger,
    monitoring
);
```

### 런타임 백엔드 선택

자동 감지 및 선택:
- 환경 기반
- 설정 기반

---

## 프로덕션 기능

### 스레드 안전

모든 핵심 연산은 스레드 안전:
- 락 기반 동기화
- 원자적 연산
- 락프리 큐

### 타입 안전

Result<T> 에러 처리:
- 예외 없는 에러 처리
- 명시적 에러 타입
- 체이닝 가능한 연산

```cpp
auto result = bus->subscribe("topic", callback);
if (!result.is_ok()) {
    std::cerr << "에러: " << result.error().message << std::endl;
    return;
}

auto sub_id = result.value();
```

### 테스트

포괄적인 테스트 스위트:
- 단위 테스트
- 통합 테스트
- 성능 벤치마크

---

## 기능 매트릭스

| 기능 | Core | Patterns | Task | Backend | 상태 |
|------|------|----------|------|---------|------|
| **메시지 버스** | ✅ | - | - | - | 완료 |
| **메시지 브로커** | ✅ | - | - | - | 완료 |
| **토픽 라우터** | ✅ | - | - | - | 완료 |
| **메시지 큐** | ✅ | - | - | - | 완료 |
| **Pub/Sub** | - | ✅ | - | - | 완료 |
| **Request/Reply** | - | ✅ | - | - | 완료 |
| **Event Streaming** | - | ✅ | - | - | 완료 |
| **Message Pipeline** | - | ✅ | - | - | 완료 |
| **태스크 시스템 (파사드)** | - | - | ✅ | - | 완료 |
| **태스크 큐** | - | - | ✅ | - | 완료 |
| **워커 풀** | - | - | ✅ | - | 완료 |
| **태스크 스케줄러** | - | - | ✅ | - | 완료 |
| **비동기 결과** | - | - | ✅ | - | 완료 |
| **결과 백엔드** | - | - | ✅ | - | 완료 |
| **태스크 모니터** | - | - | ✅ | - | 완료 |
| **크론 파서** | - | - | ✅ | - | 완료 |
| **Chain/Chord** | - | - | ✅ | - | 완료 |
| **재시도 메커니즘** | - | - | ✅ | - | 완료 |
| **태스크 타임아웃** | - | - | ✅ | - | 완료 |
| **C++20 Concepts** | ✅ | ✅ | ✅ | - | 완료 |
| **독립 백엔드** | - | - | - | ✅ | 완료 |
| **통합 백엔드** | - | - | - | ✅ | 완료 |
| **자동 감지** | - | - | - | ✅ | 완료 |
| **DI 컨테이너** | ✅ | - | - | - | 완료 |
| **에러 코드** | ✅ | - | - | - | 완료 |
| **Result<T>** | ✅ | - | - | - | 완료 |
| **직렬화** | ✅ | - | - | - | 완료 |
| **와일드카드** | ✅ | - | - | - | 완료 |
| **우선순위 큐** | ✅ | - | - | - | 완료 |
| **데드 레터 큐** | ✅ | - | - | - | 완료 |
| **트레이싱** | ✅ | - | - | - | 완료 |
| **메트릭** | ✅ | - | - | - | 완료 |

---

## 시작하기

사용 예제 및 시작 가이드:
- [빠른 시작 가이드](guides/QUICK_START.md)
- [패턴 예제](PATTERNS_API.md)
- [API 레퍼런스](API_REFERENCE.md)
- [통합 가이드](guides/INTEGRATION.md)
- [태스크 시스템 아키텍처](task/ARCHITECTURE.md)

---

**최종 수정일**: 2025-12-24
**버전**: 0.1.2
