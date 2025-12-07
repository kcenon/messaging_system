# Messaging System 변경 이력

이 프로젝트의 모든 주요 변경 사항은 이 파일에 문서화됩니다.

이 형식은 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/)를 기반으로 하며,
이 프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

## [Unreleased]

### 추가됨
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
