# Messaging System 기능

**버전**: 1.0
**최종 수정일**: 2025-11-30
**언어**: [English](FEATURES.md) | [한국어]

---

## 개요

이 문서는 messaging_system의 모든 기능을 포괄적으로 다룹니다.

---

## 목차

1. [핵심 메시징](#핵심-메시징)
2. [고급 패턴](#고급-패턴)
3. [백엔드 지원](#백엔드-지원)
4. [프로덕션 기능](#프로덕션-기능)

---

## 핵심 메시징

### 메시지 버스

중앙 pub/sub 코디네이터:
- 토픽 기반 라우팅
- 다중 구독자 지원
- 비동기 메시지 전달

### 토픽 라우터

와일드카드 패턴 매칭:
- `*`: 단일 레벨 와일드카드
- `#`: 다중 레벨 와일드카드
- 정확한 매칭

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

### 트레이스 컨텍스트

분산 추적 지원:
- 요청 상관 관계
- 스팬 전파
- OpenTelemetry 호환

---

## 고급 패턴

### Pub/Sub

퍼블리셔와 구독자 헬퍼:
```cpp
auto publisher = messaging::publisher(bus, "events");
auto subscriber = messaging::subscriber(bus, "events.#");
```

### Request/Reply

비동기 메시징 위의 동기 RPC:
```cpp
auto client = messaging::request_client(bus, "services.api");
auto response = client.request(request_msg);
```

### Event Streaming

리플레이 기능의 이벤트 소싱:
```cpp
auto stream = messaging::event_stream(bus, "events");
stream.replay_from(timestamp);
```

### Message Pipeline

파이프-필터 처리:
```cpp
auto pipeline = messaging::pipeline()
    .add_filter(validate)
    .add_transform(enrich)
    .add_sink(store);
```

---

## 백엔드 지원

### 독립 실행

자체 포함 실행:
- 내장 스레드 풀
- 자동 리소스 관리

### 스레드 풀 통합

외부 실행자 사용:
- IExecutor 인터페이스
- 커스텀 스레드 풀

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

### 테스트

100+ 단위/통합 테스트:
- 단위 테스트
- 통합 테스트
- 성능 테스트

---

## 관련 문서

- [API 레퍼런스](API_REFERENCE_KO.md)
- [아키텍처](ARCHITECTURE_KO.md)
- [빠른 시작](guides/QUICK_START.md)
