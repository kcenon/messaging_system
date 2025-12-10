# Messaging System 변경 이력

이 프로젝트의 모든 주요 변경 사항은 이 파일에 문서화됩니다.

이 형식은 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/)를 기반으로 하며,
이 프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

## [Unreleased]

### 추가됨
- **C++20 Concepts 지원 (Issue #146)**
  - 타입 안전한 콜백 및 핸들러 검증을 위한 C++20 concept 정의 추가
  - task 모듈의 새로운 concepts:
    - `TaskHandlerCallable`: 태스크 핸들러 호출 가능 시그니처 검증
    - `TaskHandlerLike`: 태스크 핸들러 인터페이스 구현 검증
    - `ScheduleEventCallable`: 스케줄러 이벤트 콜백 검증
  - patterns 모듈의 새로운 concepts:
    - `MessageProcessorCallable`: 메시지 파이프라인 프로세서 검증
    - `MessageFilterCallable`: 메시지 필터링 술어 검증
    - `MessageTransformerCallable`: 메시지 변환기 검증
    - `MessageEnricherCallable`: 메시지 보강 호출 가능 타입 검증
  - core 모듈의 새로운 concepts:
    - `SubscriptionCallable`: 토픽 구독 콜백 검증
    - `MessageFilterCallable`: 메시지 필터 술어 검증
  - 장점:
    - 잘못된 콜백 타입에 대한 명확한 컴파일 시간 오류 메시지
    - concepts를 통한 자기 문서화 인터페이스 요구사항
    - 향상된 자동 완성을 통한 더 나은 IDE 지원
    - 런타임 오버헤드 없는 타입 안전한 콜백 등록
  - concept 제약 템플릿 오버로드 추가된 함수:
    - `worker_pool::register_handler()`
    - `task_scheduler::on_task_executed()`, `on_task_failed()`
    - `pipeline_builder::add_stage()`, `add_filter()`, `add_transformer()`
    - `topic_router::subscribe()`
    - `make_task_handler()` 팩토리 함수

- **분산 태스크 큐 시스템 - Sprint 7 단위 테스트 (Issue #117)**
  - `test_task_queue.cpp`: task_queue 컴포넌트의 포괄적인 단위 테스트
    - 큐 라이프사이클 테스트 (생성, 시작/종료, 이동 시맨틱)
    - Enqueue 연산 (단일, 대량, 명명된 큐)
    - Dequeue 연산 (단일 큐, 다중 큐, 타임아웃 처리)
    - 지연 실행 테스트 (ETA 스케줄링, 카운트다운 지연)
    - 취소 테스트 (ID로, 태그로)
    - 조회 연산 (태스크 조회, 큐 목록, 큐 크기)
    - 스레드 안전성 테스트 (동시 enqueue/dequeue)
    - 우선순위 정렬 테스트
    - 29개 포괄적인 테스트
  - `test_scheduler.cpp`: task_scheduler 컴포넌트의 단위 테스트
    - 스케줄러 라이프사이클 테스트 (생성, 시작/종료, 이동 시맨틱)
    - 주기적 스케줄 테스트 (추가, 중복 방지, 실행)
    - Cron 스케줄 테스트 (추가, 잘못된 표현식, 중복 방지)
    - 스케줄 관리 테스트 (제거, 활성화/비활성화, 즉시 트리거)
    - 인터벌 및 cron 업데이트 테스트
    - 조회 연산 (목록, 조회, 존재 여부, 개수)
    - 콜백 테스트 (on_task_executed, 실행 횟수)
    - 비활성화된 스케줄 동작 테스트
    - 스레드 안전성 테스트 (동시 연산)
    - 35개 포괄적인 테스트

- **분산 태스크 큐 시스템 - Sprint 6 (Issue #113)**
  - `task_system`: 모든 태스크 큐 컴포넌트를 통합하는 통합 파사드
    - 분산 태스크 큐 시스템의 단일 진입점
    - 라이프사이클 관리: `start()`, `stop()`, `shutdown_graceful()`
    - 컴포넌트 접근: `client()`, `workers()`, `scheduler()`, `monitor()`
    - 핸들러 등록 및 태스크 제출을 위한 편의 메서드
    - 스케줄링 편의 메서드: `schedule_periodic()`, `schedule_cron()`
    - 통계 및 상태 조회: `get_statistics()`, `pending_count()`, `active_workers()`
    - 지연 초기화를 사용한 스레드 안전 구현
    - 포괄적인 단위 테스트 (26개 테스트)
  - `task_system_config`: 큐, 워커, 스케줄러, 모니터 설정을 결합하는 구성 구조체

- **분산 태스크 큐 시스템 - Sprint 3 (Issue #104)**
  - `worker_pool`: 분산 태스크 실행을 위한 스레드 풀
    - 설정 가능한 동시성 (워커 스레드 수)
    - 우선순위 정렬을 통한 다중 큐 처리
    - 태스크-핸들러 매칭을 위한 핸들러 등록
    - 타임아웃 지원이 있는 정상 종료 (graceful shutdown)
    - 통계 수집 (처리량, 실행 시간, 성공/실패율)
    - task_queue, result_backend, task_handler_interface와의 통합
  - worker_pool 종합 단위 테스트 (15개 테스트)

### 수정됨
- `task`: task_queue 조회를 위해 metadata.id와 task_id 동기화

### 변경됨
- CMake 설정에서 fmt 라이브러리 참조 제거
  - 프로젝트가 이제 thread_system을 통해 C++20 std::format만 사용
  - 의존성 관리 및 빌드 설정 단순화

### 추가됨
- 문서 구조 표준화
- 한국어 문서 지원

---

## [1.0.0] - 2025-11-17

### 추가됨
- **핵심 메시징 인프라**
  - 메시지 버스: 중앙 pub/sub 코디네이터
  - 토픽 라우터: 와일드카드 패턴 매칭 (*, #)
  - 메시지 큐: 스레드 안전 우선순위 큐
  - 메시지 직렬화: 컨테이너 기반 페이로드
  - 트레이스 컨텍스트: 분산 추적 지원

- **고급 메시징 패턴**
  - Pub/Sub: 퍼블리셔 및 구독자 헬퍼
  - Request/Reply: 비동기 메시징 위의 동기 RPC
  - Event Streaming: 리플레이 기능의 이벤트 소싱
  - Message Pipeline: 파이프-필터 처리
  - DI 컨테이너: 의존성 주입 지원

- **백엔드 지원**
  - 독립 실행
  - 스레드 풀 통합
  - 런타임 백엔드 선택
  - IExecutor 추상화
  - 테스트용 Mock 지원

- **프로덕션 품질**
  - 스레드 안전 연산
  - Result<T> 에러 처리
  - 100+ 단위/통합 테스트
  - 성능 벤치마크
  - 포괄적 문서화

### 의존성
- common_system: Result<T>, 인터페이스
- thread_system: 스레드 풀, 실행자
- container_system: 메시지 직렬화
- logger_system: 로깅 인프라
- monitoring_system: 메트릭 수집
- network_system: 네트워크 전송

---

## [0.9.0] - 2025-10-20

### 추가됨
- 초기 메시지 버스 구현
- 기본 pub/sub 패턴
- 와일드카드 토픽 라우팅

### 변경됨
- CMake FetchContent로 마이그레이션

---

*상세한 컴포넌트 변경 사항은 커밋 히스토리를 참조하세요.*
