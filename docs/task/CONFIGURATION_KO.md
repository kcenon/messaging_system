# Task 모듈 설정 가이드

이 가이드는 Task 모듈의 모든 설정 옵션을 다룹니다.

## 목차

- [Task 시스템 설정](#task-시스템-설정)
- [Task 큐 설정](#task-큐-설정)
- [워커 풀 설정](#워커-풀-설정)
- [Task 설정](#task-설정)
- [환경별 설정](#환경별-설정)
- [성능 튜닝](#성능-튜닝)

---

## Task 시스템 설정

`task_system_config`는 전체 Task 시스템의 메인 설정 구조체입니다.

### 구조체

```cpp
struct task_system_config {
    task_queue_config queue;
    worker_config worker;
    bool enable_scheduler = true;
    bool enable_monitoring = true;
    std::string result_backend_type = "memory";
};
```

### 옵션

| 옵션 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| `queue` | `task_queue_config` | (아래 참조) | 큐 설정 |
| `worker` | `worker_config` | (아래 참조) | 워커 풀 설정 |
| `enable_scheduler` | `bool` | `true` | 작업 스케줄러 활성화 |
| `enable_monitoring` | `bool` | `true` | 모니터링 컴포넌트 활성화 |
| `result_backend_type` | `std::string` | `"memory"` | 결과 저장소 백엔드 타입 |

### 예제

```cpp
task_system_config config;

// 큐 설정
config.queue.max_size = 50000;
config.queue.enable_delayed_queue = true;

// 워커 설정
config.worker.concurrency = 8;
config.worker.queues = {"default", "high-priority", "background"};

// 컴포넌트 활성화
config.enable_scheduler = true;
config.enable_monitoring = true;
config.result_backend_type = "memory";

task_system system(config);
```

---

## Task 큐 설정

`task_queue_config`는 작업 큐 동작을 제어합니다.

### 구조체

```cpp
struct task_queue_config {
    size_t max_size = 100000;
    bool enable_persistence = false;
    std::string persistence_path;
    bool enable_delayed_queue = true;
    std::chrono::milliseconds delayed_poll_interval{1000};
};
```

### 옵션

| 옵션 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| `max_size` | `size_t` | `100000` | 큐의 최대 작업 수 |
| `enable_persistence` | `bool` | `false` | 큐 영속성 활성화 |
| `persistence_path` | `std::string` | `""` | 영속성 저장소 경로 |
| `enable_delayed_queue` | `bool` | `true` | 지연 작업 스케줄링 활성화 |
| `delayed_poll_interval` | `milliseconds` | `1000` | 지연 작업 폴링 간격 |

### 영속성 설정

영속성을 활성화하면 프로세스 재시작 시에도 작업이 유지됩니다:

```cpp
task_queue_config queue_config;
queue_config.enable_persistence = true;
queue_config.persistence_path = "/var/lib/task_queue/";
queue_config.max_size = 500000;
```

### 지연 큐 설정

나중에 실행할 작업 스케줄링용:

```cpp
task_queue_config queue_config;
queue_config.enable_delayed_queue = true;
queue_config.delayed_poll_interval = std::chrono::milliseconds(500);
```

---

## 워커 풀 설정

`worker_config`는 작업 실행 방식을 제어합니다.

### 구조체

```cpp
struct worker_config {
    size_t concurrency = std::thread::hardware_concurrency();
    std::vector<std::string> queues = {"default"};
    std::chrono::milliseconds poll_interval{100};
    bool prefetch = true;
    size_t prefetch_count = 10;
};
```

### 옵션

| 옵션 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| `concurrency` | `size_t` | CPU 코어 수 | 워커 스레드 수 |
| `queues` | `vector<string>` | `{"default"}` | 처리할 큐 |
| `poll_interval` | `milliseconds` | `100` | 작업 폴링 간 시간 |
| `prefetch` | `bool` | `true` | 작업 프리페치 활성화 |
| `prefetch_count` | `size_t` | `10` | 프리페치할 작업 수 |

### 동시성 설정

워크로드에 맞게 동시성 조정:

```cpp
worker_config worker;

// CPU 바운드 작업: CPU 코어 수 사용
worker.concurrency = std::thread::hardware_concurrency();

// I/O 바운드 작업: 더 많은 스레드 사용 가능
worker.concurrency = std::thread::hardware_concurrency() * 2;

// 제한된 리소스: 스레드 제한
worker.concurrency = 4;
```

### 큐 우선순위

큐는 지정된 순서대로 처리됩니다:

```cpp
worker_config worker;

// critical 먼저, 그 다음 high, 그 다음 default 처리
worker.queues = {"critical", "high", "default", "low"};
```

### 프리페치 설정

프리페칭은 짧은 작업의 처리량을 개선합니다:

```cpp
worker_config worker;

// 20개 작업을 미리 프리페치
worker.prefetch = true;
worker.prefetch_count = 20;

// 장기 실행 작업에서는 비활성화
worker.prefetch = false;
```

---

## Task 설정

`task_config`를 통한 개별 작업 설정.

### 구조체

```cpp
struct task_config {
    std::chrono::milliseconds timeout{300000};
    size_t max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
    double retry_backoff_multiplier = 2.0;
    message_priority priority = message_priority::normal;
    std::optional<std::chrono::system_clock::time_point> eta;
    std::optional<std::chrono::milliseconds> expires;
    std::string queue_name = "default";
    std::vector<std::string> tags;
};
```

### 옵션

| 옵션 | 타입 | 기본값 | 설명 |
|------|------|--------|------|
| `timeout` | `milliseconds` | `300000` (5분) | 작업 실행 타임아웃 |
| `max_retries` | `size_t` | `3` | 최대 재시도 횟수 |
| `retry_delay` | `milliseconds` | `1000` | 초기 재시도 지연 |
| `retry_backoff_multiplier` | `double` | `2.0` | 지수 백오프 승수 |
| `priority` | `message_priority` | `normal` | 작업 우선순위 수준 |
| `eta` | `optional<time_point>` | `nullopt` | 예약 실행 시간 |
| `expires` | `optional<milliseconds>` | `nullopt` | 작업 만료 기간 |
| `queue_name` | `string` | `"default"` | 대상 큐 |
| `tags` | `vector<string>` | `{}` | 작업 태그 |

### Task 빌더 사용

```cpp
auto task = task_builder("process.data")
    .payload(data)
    .timeout(std::chrono::minutes(10))
    .retries(5)
    .retry_delay(std::chrono::seconds(5))
    .backoff(1.5)
    .priority(message_priority::high)
    .queue("processing")
    .tag("batch")
    .tag("important")
    .expires_in(std::chrono::hours(1))
    .build();
```

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

---

## 환경별 설정

### 개발 환경

```cpp
task_system_config dev_config;

// 디버깅을 위해 워커 수 줄임
dev_config.worker.concurrency = 2;
dev_config.worker.queues = {"default"};

// 빠른 피드백을 위해 작은 큐
dev_config.queue.max_size = 1000;

// 응답성을 위한 짧은 폴링 간격
dev_config.worker.poll_interval = std::chrono::milliseconds(50);

// 모든 모니터링 활성화
dev_config.enable_monitoring = true;
dev_config.enable_scheduler = true;
```

### 스테이징 환경

```cpp
task_system_config staging_config;

// 중간 수준의 리소스
staging_config.worker.concurrency = std::thread::hardware_concurrency() / 2;
staging_config.worker.queues = {"critical", "default", "background"};

// 프로덕션과 유사한 큐 설정
staging_config.queue.max_size = 50000;
staging_config.queue.enable_delayed_queue = true;

// 테스트를 위한 영속성 활성화
staging_config.queue.enable_persistence = true;
staging_config.queue.persistence_path = "/tmp/task_queue/";
```

### 프로덕션 환경

```cpp
task_system_config prod_config;

// 전체 리소스
prod_config.worker.concurrency = std::thread::hardware_concurrency();
prod_config.worker.queues = {"critical", "high", "default", "low", "background"};

// 큰 큐 용량
prod_config.queue.max_size = 500000;
prod_config.queue.enable_delayed_queue = true;

// 영속성 활성화
prod_config.queue.enable_persistence = true;
prod_config.queue.persistence_path = "/var/lib/messaging/task_queue/";

// 처리량을 위한 프리페치
prod_config.worker.prefetch = true;
prod_config.worker.prefetch_count = 50;

// 모니터링 활성화
prod_config.enable_monitoring = true;
prod_config.enable_scheduler = true;
```

---

## 성능 튜닝

### 고처리량 설정

많은 짧은 작업 처리용:

```cpp
task_system_config high_throughput;

// 더 많은 워커
high_throughput.worker.concurrency = std::thread::hardware_concurrency() * 2;

// 적극적인 프리페칭
high_throughput.worker.prefetch = true;
high_throughput.worker.prefetch_count = 100;

// 빠른 폴링
high_throughput.worker.poll_interval = std::chrono::milliseconds(10);

// 큰 큐
high_throughput.queue.max_size = 1000000;

// 빠른 지연 큐 폴링
high_throughput.queue.delayed_poll_interval = std::chrono::milliseconds(100);
```

### 저지연 설정

최소 지연으로 빠른 작업 실행:

```cpp
task_system_config low_latency;

// 전용 워커
low_latency.worker.concurrency = std::thread::hardware_concurrency();

// 빠른 폴링
low_latency.worker.poll_interval = std::chrono::milliseconds(1);

// 즉시 처리를 위해 프리페치 비활성화
low_latency.worker.prefetch = false;

// 단일 고우선순위 큐
low_latency.worker.queues = {"realtime"};
```

### 리소스 제한 설정

제한된 CPU/메모리 환경용:

```cpp
task_system_config constrained;

// 제한된 워커
constrained.worker.concurrency = 2;

// 단일 큐
constrained.worker.queues = {"default"};

// 작은 큐
constrained.queue.max_size = 10000;

// 덜 적극적인 프리페칭
constrained.worker.prefetch = true;
constrained.worker.prefetch_count = 5;

// 긴 폴링 간격
constrained.worker.poll_interval = std::chrono::milliseconds(500);

// 선택적 컴포넌트 비활성화
constrained.enable_scheduler = false;
constrained.enable_monitoring = false;
```

### 장기 실행 작업 설정

몇 분에서 몇 시간 걸리는 작업용:

```cpp
task_system_config long_running;

// 적지만 전용 워커
long_running.worker.concurrency = 4;

// 프리페치 비활성화 (워커당 하나의 작업으로 충분)
long_running.worker.prefetch = false;

// 긴 폴링 간격 (작업이 어차피 길기 때문)
long_running.worker.poll_interval = std::chrono::seconds(1);

// 장기 작업을 위한 기본값
// (개별 작업에는 task_builder 사용)
```

### 메모리 최적화

```cpp
task_system_config memory_optimized;

// 큐 크기 제한
memory_optimized.queue.max_size = 10000;

// 작은 프리페치 버퍼
memory_optimized.worker.prefetch_count = 5;

// 메모리 오프로드를 위해 영속성 활성화
memory_optimized.queue.enable_persistence = true;
memory_optimized.queue.persistence_path = "/var/lib/task_queue/";
```

---

## 설정 권장 사항

### 워크로드 유형별

| 워크로드 | 동시성 | 프리페치 | 폴링 간격 | 큐 크기 |
|----------|--------|----------|-----------|---------|
| CPU 바운드 | CPU 코어 수 | 중간 | 100ms | 중간 |
| I/O 바운드 | 2x CPU 코어 | 높음 | 50ms | 큼 |
| 혼합 | 1.5x CPU 코어 | 중간 | 100ms | 중간 |
| 장기 실행 | 낮음 (2-4) | 비활성화 | 1초 | 작음 |
| 실시간 | CPU 코어 수 | 비활성화 | 1-10ms | 작음 |

### 규모별

| 규모 | 워커 | 큐 크기 | 영속성 | 모니터링 |
|------|------|---------|--------|----------|
| 소규모 (<100 작업/분) | 2-4 | 10,000 | 선택 | 선택 |
| 중규모 (100-1000/분) | 4-8 | 50,000 | 권장 | 권장 |
| 대규모 (1000-10000/분) | 8-16 | 500,000 | 필수 | 필수 |
| 초대규모 (>10000/분) | 16+ | 1,000,000+ | 필수 | 필수 |

---

## 관련 문서

- [빠른 시작 가이드](QUICK_START_KO.md)
- [아키텍처 가이드](ARCHITECTURE_KO.md)
- [API 레퍼런스](API_REFERENCE_KO.md)
- [패턴 가이드](PATTERNS_KO.md)
- [문제 해결](TROUBLESHOOTING_KO.md)
