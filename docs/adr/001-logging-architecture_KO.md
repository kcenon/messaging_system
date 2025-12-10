# ADR-001: 로깅 아키텍처 - ILogger 인터페이스 vs logger_system

## 상태

**승인됨** - 현재 구현이 올바름

## 배경

messaging_system 프로젝트는 코드베이스 전반에 걸쳐 로깅을 사용합니다. 이 ADR은 로깅 통합 방식과 `common_system::ILogger` 및 `logger_system` 간의 관계에 대한 아키텍처 결정을 문서화합니다.

### 배경 설명

프로젝트는 초기에 로깅 기능을 위해 `logger_system`에 직접 의존했습니다. 이로 인해 강한 결합과 잠재적인 순환 의존성 문제가 발생했으며, 특히 `thread_system`과 `logger_system` 사이에서 문제가 되었습니다.

### 문제 정의

1. `logger_system`에 대한 직접 의존성으로 인한 강한 결합
2. 핵심 시스템 간 순환 의존성 가능성
3. 테스트 시 전체 logger_system 초기화 필요
4. 로깅이 필요 없는 애플리케이션도 전체 로거 구현을 포함해야 함

## 결정

`logger_system` 직접 의존성 대신 `GlobalLoggerRegistry`를 통한 런타임 바인딩과 함께 `common_system::ILogger` 인터페이스를 사용합니다.

### 아키텍처 개요

```
messaging_system
    |
    v
common::logging::log_info/error/debug (편의 함수)
    |
    v
GlobalLoggerRegistry::instance() (스레드 안전 싱글톤)
    |
    v
common::interfaces::ILogger (추상 인터페이스)
    |
    v (선택적 런타임 바인딩)
logger_system::logger (구체적 구현)
    |
    v
logger_adapter (ILogger 어댑터)
```

### 구현 세부사항

#### 1. ILogger 인터페이스 (`common_system`)

위치: `common_system/include/kcenon/common/interfaces/logger_interface.h`

```cpp
class ILogger {
public:
    virtual void log(log_level level, std::string_view message,
                     const source_location& loc = source_location::current()) = 0;
    virtual void log(const log_entry& entry) = 0;
    virtual bool is_enabled(log_level level) const = 0;
    virtual void set_level(log_level level) = 0;
    virtual log_level get_level() const = 0;
    virtual void flush() = 0;
};
```

#### 2. GlobalLoggerRegistry (`common_system`)

위치: `common_system/include/kcenon/common/interfaces/global_logger_registry.h`

기능:
- 스레드 안전 싱글톤 (Meyer's 싱글톤 패턴)
- `register_logger()`, `get_logger()`를 통한 명명된 로거 관리
- `set_default_logger()`, `get_default_logger()`를 통한 기본 로거 관리
- `register_factory()`를 통한 팩토리 기반 지연 초기화
- 로거 없이도 안전한 작동을 위한 NullLogger 폴백

#### 3. 편의 함수 (`common_system`)

위치: `common_system/include/kcenon/common/logging/log_functions.h`

```cpp
namespace common::logging {
    void log_info(const std::string& message);
    void log_error(const std::string& message);
    void log_debug(const std::string& message);
    void log_warning(const std::string& message);
    void log_trace(const std::string& message);
    void log_critical(const std::string& message);
}
```

#### 4. CMake 의존성 구성

위치: `messaging_system/CMakeLists.txt` (126-128행)

```cmake
# NOTE: logger_system is no longer a direct dependency (Issue #94).
# Logging is now provided through common_system's ILogger interface
# with runtime binding via GlobalLoggerRegistry.
```

## 결과

### 긍정적

1. **제로 결합**: messaging_system은 logger_system에 대한 컴파일 타임 의존성이 없음
2. **선택적 통합**: logger_system 설치 여부와 관계없이 작동
3. **안전한 기본값**: 로거가 구성되지 않았을 때 NullLogger가 널 포인터 충돌 방지
4. **유연한 바인딩**: 런타임에 다른 ILogger 구현을 바인딩 가능
5. **순환 의존성 해결**: thread_system <-> logger_system 사이클 해소
6. **테스트 용이성**: 전체 로거 초기화 없이 단위 테스트 실행 가능
7. **바이너리 크기 감소**: 로깅을 사용하지 않는 애플리케이션은 logger_system을 포함하지 않음

### 부정적

1. **런타임 오버헤드**: 직접 호출 대신 가상 함수 호출 (최소한의 영향)
2. **구성 복잡성**: 애플리케이션 시작 시 명시적 로거 등록 필요
3. **기능 제한**: 일부 logger_system 전용 기능은 직접 의존성 필요

### 중립적

1. **문서화 필요**: 아키텍처에 대한 명확한 문서화 필요 (이 ADR)
2. **마이그레이션 노력**: logger_system을 직접 사용하는 기존 코드 리팩토링 필요

## 고려된 대안

### 대안 A: logger_system 직접 의존성

**거부됨** 이유:
- 강한 결합 생성
- 선택적 로깅 방지
- 순환 의존성 문제 발생

### 대안 B: 템플릿을 통한 컴파일 타임 로거 선택

**거부됨** 이유:
- 바이너리 크기 증가 (템플릿 인스턴스화)
- 런타임 구성 유연성 감소
- 더 복잡한 API

### 대안 C: 매크로 기반 로깅

**거부됨** 이유:
- 타입 안전성 부족
- 유지보수 어려움
- C++20 기능과의 통합 어려움

## 준수 사항

### 현재 구현 상태

| 요구사항 | 상태 | 비고 |
|---------|------|-----|
| ILogger 인터페이스 사용 | 완료 | 모든 로깅이 편의 함수를 통함 |
| logger_system 직접 의존성 없음 | 완료 | CMakeLists.txt 확인됨 |
| GlobalLoggerRegistry 통합 | 완료 | 스레드 안전 싱글톤 |
| NullLogger 폴백 | 완료 | 안전한 기본 동작 |
| 소스 위치 지원 | 완료 | C++20 source_location |

### messaging_system에서의 사용 예시

```cpp
// src/impl/core/message_bus.cpp
#include <kcenon/common/logging/log_functions.h>

void message_bus::start() {
    common::logging::log_info("Starting message bus with " +
                               std::to_string(worker_count) + " workers");
}

void message_bus::on_error(const std::string& error) {
    common::logging::log_error("Failed to process message: " + error);
}
```

## 관련 결정

- **Issue #94**: logger_system 직접 의존성에서 마이그레이션
- **Issue #139**: 스레드 풀 아키텍처 결정 (유사한 패턴)

## 참조

- `common_system/include/kcenon/common/interfaces/logger_interface.h`
- `common_system/include/kcenon/common/interfaces/global_logger_registry.h`
- `common_system/include/kcenon/common/logging/log_functions.h`
- `messaging_system/CMakeLists.txt`
- `messaging_system/include/kcenon/messaging/utils/integration_detector.h`

## 수정 이력

| 날짜 | 버전 | 작성자 | 변경사항 |
|-----|------|-------|---------|
| 2025-12-10 | 1.0 | - | 초기 ADR 작성 |
