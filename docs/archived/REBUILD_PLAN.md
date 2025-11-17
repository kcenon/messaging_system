# Messaging System 재구성 실행 계획

## 개요 및 우선순위

본 계획은 7개 외부 시스템(common_system, thread_system, logger_system, monitoring_system, container_system, database_system, network_system)을 활용한 messaging_system 재구성을 위한 실행 계획이다.

**핵심 원칙:**
- **common_system 최우선 통합**: 모든 시스템이 의존하는 핵심 추상화 계층
- **단계적 마이그레이션**: 점진적 전환으로 리스크 최소화
- **성능 목표 명확화**: 각 Phase별 정량적 검증 기준 제시
- **병렬 작업 최대화**: Phase 2/3 일부 Task 동시 진행 가능

**성능 목표:**
- 메시지 처리량: 100K msg/s (최소), 200K msg/s (목표)
- P95 지연시간: <10ms
- 메모리 사용량: <500MB (10K 동시 연결)
- CPU 사용률: <80% (최대 부하)

## Phase 0. 준비 및 기반 정비

### **Task 0.0 – common_system 즉시 통합 (최우선)**
**목적:** 모든 후속 작업의 기반이 되는 핵심 추상화 계층 확보
**작업 내용:**
- `common_system` 패키지 설치 또는 FetchContent 설정
- CMakeLists.txt에 `find_package(CommonSystem 1.0 REQUIRED)` 추가
- 핵심 인터페이스 확인:
  - `Result<T>` / `VoidResult` (에러 처리)
  - `event_bus` (Pub/Sub)
  - `IExecutor`, `ILogger`, `IMonitor`, `IDatabase` (계약)
- 전역 include 경로 설정: `include/kcenon/common/`

*결과물*: common_system 통합 완료, 빌드 성공 확인
*선행 조건*: 없음 (최우선 작업)
*예상 소요*: 0.5일

### **Task 0.1 – 시스템 인터페이스 맵핑**
**목적:** 레거시 타입과 새 인터페이스 간 명확한 대응 관계 정립
**작업 내용:**
- `common_system`의 Result, 이벤트 버스, IExecutor/ILogger/IMonitor 계약을 목록화
- 기존 메시징 코어가 사용하는 내부 타입 대응표 정리
- 에러 코드 범위 문서화:
  - -1 ~ -99: common_system
  - -100 ~ -199: thread_system
  - -200 ~ -299: messaging_system (신규 정의)
  - -300 ~ -399: logger_system
  - 기타 시스템별 범위
- `exception_mapper` 활용 방안 문서화

*결과물*: 인터페이스 매핑 문서, 레거시 의존성 제거 리스트, 에러 코드 정의 문서
*선행 조건*: Task 0.0 완료
*예상 소요*: 1일
### **Task 0.2 – 빌드 환경 감사**
**목적:** 모든 외부 시스템의 통합 전략 확정
**작업 내용:**
- `messaging_system/CMakeLists.txt` 및 하위 `libraries/*` CMake 구성 분석
- 서브모듈 의존 경로와 Fetch/Find 전략 정의:
  - **FetchContent 방식**: 개발 환경, 빠른 반복
  - **find_package 방식**: 배포 환경, 안정성 우선
- 공유 옵션 전달 방법 표준화:
  - `MESSAGING_USE_LOCKFREE`: 락프리 구조 사용 여부
  - `MESSAGING_ENABLE_MONITORING`: 모니터링 활성화
  - `MESSAGING_ENABLE_LOGGING`: 로깅 활성화
  - `MESSAGING_ENABLE_TLS`: TLS/SSL 지원
  - `MESSAGING_BUILD_PROFILE`: Debug/Release/ASAN/TSAN
- CMake Presets 작성 (cmake-presets.json)

*결과물*: 빌드 구성 설계 노트, CMake Presets 파일, 의존성 설치 가이드
*선행 조건*: Task 0.0 완료
*예상 소요*: 1일

### **Task 0.3 – 위험 및 마이그레이션 전략 확정**
**목적:** 안전한 점진적 전환 경로 확보
**작업 내용:**
- 레거시 API와 외부 통합 지점 식별
- 호환성 레이어 설계:
  - Feature Flag: `MESSAGING_LEGACY_API_COMPAT`
  - 예외 기반 API → Result<T> 래퍼 제공
  - 2 릴리스 사이클 동시 지원 계획
- 점진적 마이그레이션 시나리오:
  - **v1.x**: 레거시 + 신규 API 동시 지원 (deprecation warning)
  - **v1.x+1**: 레거시 API 제거 예고
  - **v2.x**: 신규 API만 지원
- 롤백 시나리오:
  - Blue-Green 배포 전략
  - DB 마이그레이션 롤백 스크립트
  - 설정 파일 버전 관리

*결과물*: 리스크 레지스터, 롤백/전환 전략 문서, 호환성 레이어 설계
*선행 조건*: Task 0.1 완료
*예상 소요*: 1.5일

### **Task 0.4 – 레거시 구현 차단 계획 수립**
**목적:** 명확한 코드 제거 로드맵 확보
**작업 내용:**
- 레거시 코드 목록 작성:
  - 내장 container 구현 → `container_system` 대체
  - 내장 network 구현 → `network_system` 대체
  - 내장 thread pool → `thread_system` 대체
  - 예외 기반 에러 처리 → `common_system::Result<T>` 대체
- 책임 매트릭스 정의:
  ```
  | 기능 영역          | 레거시 담당     | 신규 담당           | 인터페이스       |
  |-------------------|----------------|--------------------|--------------------|
  | 에러 처리         | std::exception | common_system      | Result<T>          |
  | 비동기 실행       | 내장 thread    | thread_system      | IExecutor          |
  | 메시지 직렬화     | 내장 container | container_system   | value_container    |
  | 네트워크 I/O      | 내장 network   | network_system     | messaging_client   |
  | 영속화            | 직접 SQL       | database_system    | IDatabase, ORM     |
  | 로깅              | std::cout      | logger_system      | ILogger            |
  | 메트릭            | 없음           | monitoring_system  | IMonitor           |
  ```
- 빌드 가드 코드 설계:
  ```cpp
  #ifndef MESSAGING_USE_EXTERNAL_SYSTEMS
    #error "Legacy internal systems deprecated. Set MESSAGING_USE_EXTERNAL_SYSTEMS=ON"
  #endif
  ```
- 아카이브 이동 스크립트 작성

*결과물*: 레거시 제거 체크리스트, 책임 매트릭스, 빌드 가드 헤더, 아카이브 스크립트
*선행 조건*: Task 0.2 완료
*예상 소요*: 1일

## Phase 1. 빌드 및 의존성 리팩터링

**Phase 목표:** 모든 외부 시스템을 빌드 시스템에 통합하고 레거시 코드 제거

### **Task 1.1 – 외부 모듈 통합 재구성**
**목적:** 7개 외부 시스템의 빌드 시스템 통합
**작업 내용:**
- CMakeLists.txt 수정:
  ```cmake
  # FetchContent 방식 (개발 환경)
  include(FetchContent)
  FetchContent_Declare(CommonSystem GIT_REPOSITORY ... GIT_TAG main)
  FetchContent_Declare(ThreadSystem GIT_REPOSITORY ... GIT_TAG main)
  # ... 기타 시스템
  FetchContent_MakeAvailable(CommonSystem ThreadSystem ...)

  # find_package 방식 (배포 환경)
  find_package(CommonSystem 1.0 REQUIRED)
  find_package(ThreadSystem 1.0 REQUIRED)
  find_package(LoggerSystem 1.0 REQUIRED)
  find_package(MonitoringSystem 1.0 REQUIRED)
  find_package(ContainerSystem 1.0 REQUIRED)
  find_package(DatabaseSystem 1.0 REQUIRED)
  find_package(NetworkSystem 1.0 REQUIRED)
  ```
- ALIAS 타깃 정의:
  ```cmake
  if(NOT TARGET common)
    add_library(common ALIAS CommonSystem::common)
  endif()
  if(NOT TARGET thread_pool)
    add_library(thread_pool ALIAS ThreadSystem::Core)
  endif()
  # ... 기타 ALIAS
  ```
- 통합 의존성 타깃 생성:
  ```cmake
  add_library(messaging_system_deps INTERFACE)
  target_link_libraries(messaging_system_deps INTERFACE
    CommonSystem::common
    ThreadSystem::Core
    LoggerSystem::logger
    MonitoringSystem::monitoring
    ContainerSystem::container
    DatabaseSystem::database
    NetworkSystem::network
  )
  ```

*결과물*: 업데이트된 CMakeLists.txt, ALIAS 타깃 정의, 의존성 설치 가이드
*선행 조건*: Phase 0 완료
*예상 소요*: 2일

### **Task 1.2 – 컴파일 옵션 및 피처 플래그 통합**
**목적:** 일관된 빌드 설정 전파 메커니즘 구축
**작업 내용:**
- 통합 CMake 캐시 변수 정의:
  ```cmake
  # 공통 옵션
  set(MESSAGING_CXX_STANDARD 20 CACHE STRING "C++ standard")
  set(MESSAGING_WARNING_LEVEL "High" CACHE STRING "Warning level")

  # 피처 플래그
  option(MESSAGING_USE_LOCKFREE "Enable lock-free structures" OFF)
  option(MESSAGING_ENABLE_MONITORING "Enable monitoring" ON)
  option(MESSAGING_ENABLE_LOGGING "Enable logging" ON)
  option(MESSAGING_ENABLE_METRICS "Enable metrics" ON)
  option(MESSAGING_ENABLE_TLS "Enable TLS/SSL" OFF)

  # 빌드 프로파일
  set(MESSAGING_BUILD_PROFILE "Release" CACHE STRING "Build profile")
  set_property(CACHE MESSAGING_BUILD_PROFILE PROPERTY STRINGS
    "Debug" "Release" "RelWithDebInfo" "ASAN" "TSAN" "UBSAN")

  # 프로파일별 플래그
  if(MESSAGING_BUILD_PROFILE STREQUAL "ASAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  elseif(MESSAGING_BUILD_PROFILE STREQUAL "TSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
  elseif(MESSAGING_BUILD_PROFILE STREQUAL "UBSAN")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
  endif()

  # 하위 모듈 전파
  set(USE_LOCKFREE_BY_DEFAULT ${MESSAGING_USE_LOCKFREE} CACHE BOOL "" FORCE)
  set(ENABLE_MONITORING ${MESSAGING_ENABLE_MONITORING} CACHE BOOL "" FORCE)
  set(ENABLE_LOGGING ${MESSAGING_ENABLE_LOGGING} CACHE BOOL "" FORCE)
  ```
- CI 프로파일별 문서화 (README.md)

*결과물*: 통합 빌드 옵션 정의, CI 프로파일 문서, 빌드 설정 가이드
*선행 조건*: Task 1.1 완료
*예상 소요*: 1일

### **Task 1.3 – 구성 검증 자동화**
**목적:** 빌드 시 의존성 누락 조기 발견
**작업 내용:**
- CMake 검증 함수 작성 (cmake/validate_dependencies.cmake):
  ```cmake
  function(validate_messaging_dependencies)
    set(REQUIRED_TARGETS
      CommonSystem::common
      ThreadSystem::Core
      LoggerSystem::logger
      MonitoringSystem::monitoring
      ContainerSystem::container
      DatabaseSystem::database
      NetworkSystem::network
    )

    foreach(target ${REQUIRED_TARGETS})
      if(NOT TARGET ${target})
        message(FATAL_ERROR
          "Required dependency ${target} not found. "
          "Please install all external modules or use FetchContent.")
      endif()
    endforeach()

    message(STATUS "✅ All messaging_system dependencies validated")
  endfunction()

  validate_messaging_dependencies()
  ```
- CMake Presets 작성 (CMakePresets.json):
  ```json
  {
    "version": 3,
    "configurePresets": [
      {
        "name": "default",
        "binaryDir": "${sourceDir}/build",
        "cacheVariables": {
          "CMAKE_BUILD_TYPE": "Release"
        }
      },
      {
        "name": "asan",
        "inherits": "default",
        "cacheVariables": {
          "MESSAGING_BUILD_PROFILE": "ASAN"
        }
      },
      {
        "name": "tsan",
        "inherits": "default",
        "cacheVariables": {
          "MESSAGING_BUILD_PROFILE": "TSAN"
        }
      }
    ]
  }
  ```

*결과물*: 검증 스크립트, CMake Presets, 설정 가이드
*선행 조건*: Task 1.2 완료
*예상 소요*: 0.5일

### **Task 1.4 – 레거시 코드 비활성화 및 제거**
**목적:** 레거시 코드 빌드 제외 및 안전한 아카이브
**작업 내용:**
- 빌드 가드 헤더 추가 (include/messaging_system/legacy_guard.h):
  ```cpp
  #ifndef MESSAGING_USE_EXTERNAL_SYSTEMS
    #error "Legacy internal systems deprecated. Set MESSAGING_USE_EXTERNAL_SYSTEMS=ON"
  #endif

  #if !defined(HAS_COMMON_SYSTEM)
    #error "CommonSystem not found. Install common_system package."
  #endif

  #if !defined(HAS_THREAD_SYSTEM)
    #error "ThreadSystem not found. Install thread_system package."
  #endif
  // ... 기타 시스템 검증
  ```
- 아카이브 스크립트 작성 (scripts/archive_legacy.sh):
  ```bash
  #!/bin/bash
  LEGACY_DIRS=(
    "src/legacy_container"
    "src/legacy_network"
    "src/legacy_thread"
  )

  mkdir -p _archived/$(date +%Y%m%d)
  for dir in "${LEGACY_DIRS[@]}"; do
    if [ -d "$dir" ]; then
      mv "$dir" "_archived/$(date +%Y%m%d)/"
      echo "Archived: $dir"
    fi
  done
  ```
- CMakeLists.txt에서 레거시 소스 제외

*결과물*: 빌드 가드 헤더, 아카이브 스크립트, 업데이트된 빌드 파일
*선행 조건*: Task 1.3 완료
*예상 소요*: 1일

**Phase 1 완료 기준:**
- ✅ 모든 외부 시스템 빌드 성공
- ✅ 레거시 코드 빌드 대상에서 제외
- ✅ CI 파이프라인 통과 (모든 프로파일)

## Phase 2. 메시징 코어 재설계

**Phase 목표:** Result<T> 기반 메시징 코어 구축 및 DI 아키텍처 적용
**병렬 작업:** Task 2.1-2.2는 Task 3.1(네트워크 경계)과 병렬 진행 가능

### **Task 2.1 – 메시지 컨테이너 DSL 정립**
**목적:** container_system 기반 메시지 구조 표준화
**작업 내용:**
- 메시지 구조 정의:
  ```cpp
  // Header: source, target, topic, trace_id, timestamp, version
  // Body: operation, parameters (타입별 value_container)
  // Metadata: priority, ttl, retry_count
  ```
- Fluent Builder API 작성:
  ```cpp
  auto msg = messaging_container_builder()
    .source("client_123")
    .target("service_xyz")
    .topic("user.events.created")
    .trace_id(generate_trace_id())
    .add_header("version", "1.0")
    .add_body("operation", "create_user")
    .add_body("user_id", 12345)
    .add_metadata("priority", "HIGH")
    .optimize_for_speed()  // optimized_container 사용
    .build();
  ```
- 직렬화 포맷 지원:
  - Binary (기본): 2M containers/s
  - JSON (디버깅): 800K containers/s
- 성능 검증: 메시지 생성 5M/s, 직렬화 2M/s

*결과물*: messaging_container.h, messaging_container_builder.h, 단위 테스트
*선행 조건*: Phase 1 완료
*예상 소요*: 2일
*성능 목표*: 메시지 생성 5M/s, 직렬화 2M/s

### **Task 2.2 – Result 기반 플로우 제로화**
**목적:** 예외 없는 에러 처리로 전환
**작업 내용:**
- 에러 코드 정의 (include/messaging_system/error_codes.h):
  ```cpp
  namespace messaging::error {
    constexpr int INVALID_MESSAGE = -200;
    constexpr int ROUTING_FAILED = -201;
    constexpr int SERIALIZATION_ERROR = -202;
    constexpr int NETWORK_ERROR = -203;
    constexpr int DATABASE_ERROR = -204;
    constexpr int QUEUE_FULL = -205;
    constexpr int TIMEOUT = -206;
    constexpr int AUTHENTICATION_FAILED = -207;
    constexpr int AUTHORIZATION_FAILED = -208;
  }
  ```
- API 변환 패턴:
  ```cpp
  // 레거시
  void MessageBus::publish(const Message& msg) { throw ...; }

  // 신규
  Result<void> MessageBus::publish(const Message& msg) {
    auto validation = validate(msg);
    RETURN_IF_ERROR(validation);

    return network->send(msg)
      .and_then([](auto) { return persist(msg); })
      .map([](auto) { return std::monostate{}; });
  }
  ```
- 헬퍼 매크로 활용:
  - `RETURN_IF_ERROR(result)`
  - `ASSIGN_OR_RETURN(var, result)`
  - `RETURN_ERROR_IF(condition, code, message)`
- exception_mapper 통합 (레거시 호환)

*결과물*: error_codes.h, Result 기반 API, 마이그레이션 가이드
*선행 조건*: Task 2.1 완료
*예상 소요*: 3일

### **Task 2.3 – IExecutor 주입식 아키텍처 구현**
**목적:** DI 기반 실행기 주입으로 유연성 확보
**작업 내용:**
- service_registry 활용:
  ```cpp
  // 실행기 등록
  auto io_executor = std::make_shared<thread_pool>(2);  // I/O 전용
  auto work_executor = std::make_shared<typed_thread_pool>(8);  // 비즈니스 로직
  service_registry::register_service<IExecutor>("io", io_executor);
  service_registry::register_service<IExecutor>("work", work_executor);

  // MessageBus 생성
  class MessageBus {
    std::shared_ptr<IExecutor> io_executor_;
    std::shared_ptr<IExecutor> work_executor_;

  public:
    MessageBus(std::shared_ptr<IExecutor> io, std::shared_ptr<IExecutor> work)
      : io_executor_(io), work_executor_(work) {}

    Result<void> publish_async(Message msg) {
      auto job = std::make_unique<MessageProcessJob>(std::move(msg));
      return work_executor_->execute(std::move(job));
    }
  };
  ```
- typed_thread_pool 활용 (메시지 타입별 라우팅):
  ```cpp
  typed_pool->register_type<HighPriorityMessage>(2);
  typed_pool->register_type<NormalMessage>(6);
  ```
- 성능 검증: 1.24M jobs/s (typed_thread_pool)

*결과물*: DI 기반 MessageBus, IExecutor 통합, 성능 테스트
*선행 조건*: Task 2.2 완료
*예상 소요*: 2일
*성능 목표*: 1.2M+ jobs/s

### **Task 2.4 – 라우팅/토픽 엔진 정비**
**목적:** 고성능 Pub/Sub 엔진 구축
**작업 내용:**
- TopicRouter 구현:
  ```cpp
  class TopicRouter {
    using SubscriberCallback = std::function<Result<void>(const value_container&)>;
    using Filter = std::function<bool(const value_container&)>;

    struct Subscription {
      std::string topic_pattern;  // "user.*.created", "order.#"
      Filter filter;
      SubscriberCallback callback;
      int priority;  // 0-10, 높을수록 우선
    };

  public:
    Result<void> subscribe(const std::string& topic,
                           SubscriberCallback cb,
                           Filter filter = nullptr,
                           int priority = 5);

    Result<void> publish(const std::string& topic,
                         const value_container& msg);

  private:
    std::unordered_map<std::string, std::vector<Subscription>> subscriptions_;
    std::shared_ptr<IExecutor> executor_;
    std::shared_ptr<bounded_job_queue> queue_;  // 백프레셔 지원
  };
  ```
- 백프레셔 정책 통합:
  ```cpp
  auto bounded_queue = std::make_shared<bounded_job_queue>(10000);
  bounded_queue->set_overflow_policy(overflow_policy::drop_oldest);

  // 메트릭 수집
  auto metrics = bounded_queue->get_metrics();
  if (metrics.utilization > 0.8) {
    logger->log(log_level::warning, "Queue 80% full");
  }
  ```
- 패턴 매칭 엔진:
  - `*`: 단일 레벨 와일드카드 (user.*.created)
  - `#`: 다중 레벨 와일드카드 (order.#)
  - 정규식 지원 (옵션)

*결과물*: TopicRouter, 패턴 매칭 엔진, 백프레셔 통합, 벤치마크
*선행 조건*: Task 2.3 완료
*예상 소요*: 3일
*성능 목표*: 100K+ msg/s 라우팅 처리량

**Phase 2 완료 기준:**
- ✅ 메시지 컨테이너 생성 5M/s
- ✅ 직렬화 2M/s
- ✅ 모든 API Result<T> 반환
- ✅ 라우팅 처리량 100K+ msg/s

## Phase 3. 인프라 통합 계층 구축

**Phase 목표:** 네트워크/DB/로깅/모니터링 시스템 통합
**병렬 작업:** Task 3.1은 Task 2.1-2.2와 병렬, Task 3.2는 Task 2.3-2.4와 병렬 가능

### **Task 3.1 – 네트워크 경계 계층 구현**
**목적:** network_system 기반 고성능 메시지 송수신 파이프라인
**작업 내용:**
- MessagingNetworkBridge 구현:
  ```cpp
  class MessagingNetworkBridge {
    std::shared_ptr<messaging_server> server_;
    std::shared_ptr<IExecutor> io_executor_;    // I/O 전용 (1-2 쓰레드)
    std::shared_ptr<IExecutor> work_executor_;  // 비즈니스 로직 (N 쓰레드)
    std::shared_ptr<TopicRouter> router_;

  public:
    Result<void> start(uint16_t port);

  private:
    Result<void> on_message_received(
      std::shared_ptr<messaging_session> session,
      std::vector<uint8_t> data);

    Result<void> process_message(
      std::shared_ptr<messaging_session> session,
      const value_container& container);
  };
  ```
- I/O와 워크 쓰레드 분리:
  - I/O 쓰레드: 네트워크 이벤트 처리 (비차단)
  - 워크 쓰레드: 역직렬화 + 비즈니스 로직 (병렬)
- 파이프라인 구성:
  1. 수신 → 2. 역직렬화 (I/O) → 3. 라우팅 (Worker) → 4. 직렬화 (Worker) → 5. 송신 (I/O)
- 성능 목표: 네트워크 계층 305K msg/s → 전체 파이프라인 100K+ msg/s

*결과물*: MessagingNetworkBridge, 파이프라인 구현, 성능 벤치마크
*선행 조건*: Task 2.1-2.2 완료 (병렬 가능)
*예상 소요*: 3일
*성능 목표*: 100K+ msg/s 처리량, P95 <10ms

### **Task 3.2 – 영속성 및 재처리 경로 구성**
**목적:** database_system 기반 메시지 영속화 및 재처리
**작업 내용:**
- ORM Entity 정의:
  ```cpp
  struct MessageEntity : entity_base {
    int64_t id;
    std::string topic;
    std::string payload;
    std::string status;  // PENDING, PROCESSING, COMPLETED, FAILED
    std::chrono::system_clock::time_point created_at;
    int retry_count;

    static entity_metadata metadata();
  };

  struct ConsumerOffset : entity_base {
    std::string consumer_group;
    std::string topic;
    int64_t offset;

    static entity_metadata metadata();
  };
  ```
- PersistentMessageQueue 구현:
  ```cpp
  class PersistentMessageQueue {
    std::shared_ptr<connection_pool> pool_;
    std::shared_ptr<entity_manager> em_;

  public:
    Result<void> enqueue(const value_container& msg);
    Result<std::vector<MessageEntity>> dequeue_batch(int limit = 100);
    Result<void> mark_completed(int64_t message_id);
    Result<void> mark_failed(int64_t message_id);
    Result<void> reprocess_failed(int max_retries = 3);
  };
  ```
- 트랜잭션 경계:
  ```cpp
  RETURN_IF_ERROR(conn->begin_transaction());
  auto result = em_->save(entity);
  if (result.is_error()) {
    conn->rollback_transaction();
    return result;
  }
  return conn->commit_transaction();
  ```
- 커넥션 풀 설정:
  - min_connections: 5, max_connections: 50
  - idle_timeout: 30s, acquire_timeout: 5s

*결과물*: ORM Entities, PersistentMessageQueue, 재처리 로직, 통합 테스트
*선행 조건*: Task 2.3-2.4 완료 (병렬 가능)
*예상 소요*: 3일
*성능 목표*: P95 영속화 지연 <50ms

### **Task 3.3 – 로깅/모니터링 데이터 흐름 통합**
**목적:** 분산 추적 및 실시간 메트릭 수집
**작업 내용:**
- trace_id 전파 메커니즘:
  ```cpp
  class TraceContext {
    static thread_local std::string current_trace_id;

  public:
    static void set_trace_id(const std::string& id);
    static std::string get_trace_id();
  };

  // 사용 예시
  Result<void> handle_request(const value_container& req) {
    auto trace_id = req.get_value("header.trace_id")->to_string();
    TraceContext::set_trace_id(trace_id);

    logger_->log(log_level::info,
      fmt::format("[{}] Request received", TraceContext::get_trace_id()));

    // 비즈니스 로직 ...

    // 다음 서비스 호출 시 trace_id 전파
    auto next_req = messaging_container_builder()
      .add_value("header.trace_id", TraceContext::get_trace_id())
      .build();

    return send_to_next_service(next_req);
  }
  ```
- logger_system 통합:
  ```cpp
  auto logger = logger_builder()
    .with_async(true)
    .with_buffer_size(16384)
    .with_batch_size(100)
    .add_writer("console", std::make_unique<console_writer>())
    .add_writer("file", std::make_unique<rotating_file_writer>("messaging.log"))
    .build();
  ```
- monitoring_system 통합:
  ```cpp
  // 메시지 처리 시작
  PERF_TIMER("message_processing");

  // 메트릭 수집
  monitor->record_metric("messages_sent_total", 1.0, {{"topic", topic}});
  monitor->record_metric("message_latency_us", latency.count(), {{"topic", topic}});
  monitor->record_metric("queue_depth", queue_depth, {{"queue", queue_name}});
  ```
- 로깅 레벨: info (프로덕션), debug (개발)
- 메트릭 수집 간격: 1000ms

*결과물*: TraceContext, 로깅/모니터링 통합, 대시보드 구성
*선행 조건*: Task 3.1-3.2 완료
*예상 소요*: 2일

### **Task 3.4 – 구성 및 정책 레이어**
**목적:** YAML 기반 통합 구성 시스템
**작업 내용:**
- 구성 파일 스키마 (config/messaging_system.yaml):
  ```yaml
  messaging_system:
    version: "2.0"

    network:
      server:
        port: 8080
        max_connections: 10000
      client:
        timeout_ms: 5000
        retry_attempts: 3

    thread_pools:
      io_pool:
        workers: 2
        queue_size: 100000
      work_pool:
        workers: 16
        queue_size: 100000
        lockfree: true

    database:
      type: "postgresql"
      connection_string: "host=localhost port=5432 dbname=messaging"
      pool:
        min_connections: 5
        max_connections: 50

    logging:
      level: "info"
      async: true

    monitoring:
      enabled: true
      metrics_interval_ms: 1000

    policies:
      backpressure:
        enabled: true
        strategy: "drop_oldest"
        threshold: 0.8
      retry:
        max_attempts: 3
        backoff_ms: [100, 500, 2000]
  ```
- MessagingSystemConfig 클래스:
  ```cpp
  class MessagingSystemConfig {
  public:
    static Result<MessagingSystemConfig> load_from_file(const std::string& path);
    Result<void> watch_for_changes(std::function<void(const MessagingSystemConfig&)> callback);

    uint16_t network_port;
    int max_connections;
    int io_workers;
    int work_workers;
    bool use_lockfree;
    std::string db_type;
    connection_pool_config db_pool_config;
    log_level log_level;
    // ... 기타 설정
  };
  ```
- Watch 메커니즘 (inotify/FSEvents)
- 구성 검증: 스키마 유효성 검사

*결과물*: 구성 로더, YAML 스키마, Watch 메커니즘, 검증 로직
*선행 조건*: Task 3.3 완료
*예상 소요*: 2일

**Phase 3 완료 기준:**
- ✅ 네트워크 파이프라인 100K+ msg/s
- ✅ DB 영속화 P95 <50ms
- ✅ 분산 추적 trace_id 전파 동작
- ✅ YAML 구성 로더 동작

## Phase 4. 검증·배포 준비

**Phase 목표:** 프로덕션 배포를 위한 검증 및 문서화

### **Task 4.1 – 교차 모듈 테스트 수립**
**목적:** End-to-End 통합 테스트 및 CI/CD 구성
**작업 내용:**
- 통합 테스트 시나리오:
  1. **End-to-End 파이프라인**: 메시지 발행 → 라우팅 → 영속화 → 수신
  2. **네트워크 왕복**: 클라이언트 송신 → 서버 수신 → 응답 → 클라이언트 수신
  3. **DB 영속화**: 메시지 저장 → 조회 → 상태 업데이트
  4. **로깅/모니터링**: trace_id 전파, 메트릭 수집
- CI/CD 파이프라인 (.github/workflows/ci.yml):
  ```yaml
  jobs:
    build-and-test:
      strategy:
        matrix:
          build_type: [Debug, Release, ASAN, TSAN]
      steps:
        - Configure: cmake -DMESSAGING_BUILD_PROFILE=${{matrix.build_type}}
        - Build: cmake --build build -j$(nproc)
        - Test: ctest --parallel $(nproc)
        - Coverage: lcov (Debug only)
  ```
- 커버리지 목표: 80%+
- 성능 벤치마크:
  - 메시지 처리량: 100K+ msg/s
  - P95 지연시간: <10ms
  - 메모리 사용량: <500MB (10K 연결)

*결과물*: 통합 테스트 Suite, CI/CD 구성, 커버리지 리포트
*선행 조건*: Phase 3 완료
*예상 소요*: 3일

### **Task 4.2 – 마이그레이션 가이드 및 API 문서화**
**목적:** 사용자 대상 문서 완성
**작업 내용:**
- 마이그레이션 가이드 (docs/MIGRATION_GUIDE.md):
  - Breaking Changes 목록
  - API 변환 패턴 (예외 → Result<T>)
  - Step-by-Step 마이그레이션 절차
  - 호환성 레이어 사용법
  - FAQ
- API 레퍼런스:
  - Doxygen 주석 추가
  - 모든 public API 문서화
  - 사용 예제 코드
- 배포 가이드 (docs/DEPLOYMENT_GUIDE.md):
  - 시스템 요구사항
  - 설치 절차
  - 설정 파일 작성 가이드
  - 모니터링 대시보드 구성
- 성능 튜닝 가이드 (docs/PERFORMANCE_TUNING.md):
  - 스레드 풀 크기 조정
  - DB 커넥션 풀 설정
  - 네트워크 버퍼 크기
  - 백프레셔 정책 선택

*결과물*: 마이그레이션 가이드, API 레퍼런스, 배포 가이드, 튜닝 가이드
*선행 조건*: Task 4.1 완료
*예상 소요*: 2일

### **Task 4.3 – 성능 및 회귀 검증**
**목적:** 성능 목표 달성 검증 및 회귀 방지
**작업 내용:**
- 통합 벤치마크 Suite:
  ```cpp
  // benchmarks/end_to_end_benchmark.cpp
  BENCHMARK(BM_MessagePublish)->Threads(1)->Threads(4)->Threads(8);
  BENCHMARK(BM_MessageRoundtrip);
  BENCHMARK(BM_DatabasePersistence);
  BENCHMARK(BM_TopicRouting);
  ```
- 성능 목표 검증:
  | 메트릭 | 목표 | 측정 방법 |
  |--------|------|-----------|
  | 메시지 처리량 | 100K msg/s | BM_MessagePublish |
  | P95 지연시간 | <10ms | BM_MessageRoundtrip |
  | P95 DB 영속화 | <50ms | BM_DatabasePersistence |
  | 메모리 사용량 | <500MB | Valgrind massif |
  | CPU 사용률 | <80% | perf stat |
- 회귀 테스트:
  - 기준 성능 메트릭 저장 (baselines/)
  - CI에서 자동 비교
  - 10% 이상 저하 시 빌드 실패
- 프로파일링:
  - CPU: perf record/report
  - 메모리: Valgrind massif
  - 락 경합: perf lock

*결과물*: 벤치마크 Suite, 성능 리포트, 회귀 테스트 설정
*선행 조건*: Task 4.2 완료
*예상 소요*: 2일
*검증 기준*: 모든 성능 목표 달성

### **Task 4.4 – 보안 검토 및 강화**
**목적:** 프로덕션 배포를 위한 보안 강화 (추가 제안)
**작업 내용:**
- TLS/SSL 지원:
  - network_system의 pipeline.encrypt() 활용
  - OpenSSL 통합
  - 인증서 관리
- 인증/권한 시스템:
  - database_system의 access_control 활용
  - RBAC (Role-Based Access Control)
  - API 키 관리
- 감사 로그:
  - database_system의 audit_logger 활용
  - 모든 메시지 송수신 기록
  - 민감한 작업 추적
- 보안 취약점 스캔:
  - 정적 분석: clang-tidy, cppcheck
  - 동적 분석: ASAN, UBSAN
  - 의존성 취약점: OWASP Dependency-Check

*결과물*: TLS 구성, 인증 시스템, 감사 로그, 보안 스캔 리포트
*선행 조건*: Task 4.3 완료
*예상 소요*: 3일

### **Task 4.5 – 출시 체크리스트/런북 작성**
**목적:** 운영 준비 완료
**작업 내용:**
- 배포 전 체크리스트:
  - [ ] 모든 CI 테스트 통과 (모든 빌드 타입)
  - [ ] 성능 벤치마크 목표 달성
  - [ ] 보안 감사 완료
  - [ ] DB 마이그레이션 스크립트 검증
  - [ ] 백업 및 롤백 절차 준비
  - [ ] 모니터링 대시보드 구성
  - [ ] 알림 임계값 설정
  - [ ] 문서 업데이트
  - [ ] 스테이징 환경 배포 및 검증
  - [ ] 부하 테스트 (예상 트래픽의 150%)
- 배포 절차:
  1. DB 마이그레이션
  2. 설정 파일 배포
  3. Rolling/Blue-Green 배포
  4. Health Check 검증
- 모니터링 체크리스트:
  - [ ] 메시지 처리율 >50K msg/s
  - [ ] P95 지연시간 <50ms
  - [ ] CPU 사용률 <80%
  - [ ] 메모리 사용률 <80%
  - [ ] DB 커넥션 풀 >20% 여유
  - [ ] 큐 깊이 <5000 메시지
  - [ ] 에러율 <1%
- 롤백 절차:
  1. 트래픽 차단
  2. 이전 버전 복구
  3. DB 롤백 (필요 시)
  4. 설정 복원
  5. 검증
- 인시던트 대응:
  - 높은 지연시간: 큐 깊이/DB 상태 확인
  - 메시지 손실: DB PENDING 조회, DLQ 확인
  - 메모리 누수: Heap 프로파일링

*결과물*: 배포 체크리스트, 배포 스크립트, 모니터링 설정, 런북
*선행 조건*: Task 4.4 완료
*예상 소요*: 1일

**Phase 4 완료 기준:**
- ✅ 모든 통합 테스트 통과
- ✅ 성능 목표 달성 (100K msg/s, P95 <10ms)
- ✅ 커버리지 80%+ 달성
- ✅ 보안 감사 완료
- ✅ 문서 완성
- ✅ 운영 런북 작성 완료

---

## 전체 일정 요약

| Phase | 예상 소요 | 누적 소요 | 주요 산출물 |
|-------|----------|----------|------------|
| **Phase 0** | 4일 | 4일 | common_system 통합, 인터페이스 매핑, 빌드 환경 설계 |
| **Phase 1** | 4.5일 | 8.5일 | 외부 모듈 통합, CMake 구성, 레거시 제거 |
| **Phase 2** | 10일 | 18.5일 | 메시지 컨테이너, Result<T> API, DI 아키텍처, TopicRouter |
| **Phase 3** | 10일 | 28.5일 | 네트워크 브리지, DB 영속화, 로깅/모니터링, YAML 구성 |
| **Phase 4** | 11일 | 39.5일 | 통합 테스트, 문서화, 성능 검증, 보안 강화, 런북 |

**총 예상 소요:** 약 40일 (8주, 병렬 작업 고려 시 ~6주)

## 병렬 작업 최적화

**Phase 2 + Phase 3 병렬 구간:**
- Task 2.1-2.2 (메시지 컨테이너, Result 전환) + Task 3.1 (네트워크 브리지)
- Task 2.3-2.4 (DI, 라우팅) + Task 3.2 (DB 영속화)
- 병렬 진행 시 약 2주 단축 가능

**실제 예상 소요:** 6주 (병렬 최적화 적용)

## 성공 기준 (최종)

✅ **기능 완성도:**
- 모든 Phase 완료
- 모든 Task 산출물 검수 완료
- 통합 테스트 100% 통과

✅ **성능 목표:**
- 메시지 처리량: 100K+ msg/s (목표 200K msg/s)
- P95 지연시간: <10ms
- 메모리 사용량: <500MB (10K 연결)
- CPU 사용률: <80%

✅ **품질 기준:**
- 테스트 커버리지: 80%+
- 모든 빌드 프로파일 통과 (Debug, Release, ASAN, TSAN)
- Zero 메모리 누수 (ASAN)
- Zero 데이터 레이스 (TSAN)

✅ **운영 준비:**
- 문서 완성 (마이그레이션, API, 배포, 튜닝)
- 모니터링 대시보드 구성
- 보안 감사 완료
- 런북 작성 완료

---

**본 계획은 Phase별 선행 조건을 포함하고 있으므로 각 Phase 완료 시 산출물을 검토한 뒤 다음 Phase를 착수한다.**

**핵심 개선사항 요약:**
1. ✅ common_system 최우선 통합 (Task 0.0 신설)
2. ✅ 성능 목표 명확화 (각 Phase별 정량적 목표)
3. ✅ 병렬 작업 가이드라인 (Phase 2/3 병렬 가능)
4. ✅ 보안 검토 추가 (Task 4.4 신설)
5. ✅ 상세한 코드 예제 및 구현 가이드
6. ✅ 예상 소요 시간 및 일정 명시
