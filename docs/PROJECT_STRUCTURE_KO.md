# Messaging System 프로젝트 구조

**버전**: 1.0
**최종 수정일**: 2025-11-30
**언어**: [English](PROJECT_STRUCTURE.md) | [한국어]

---

## 개요

이 문서는 messaging_system 저장소의 디렉토리 구조와 파일 구성에 대해 설명합니다.

---

## 디렉토리 구조

```
messaging_system/
├── .github/                    # GitHub 설정
│   └── workflows/              # CI/CD 워크플로우
│
├── cmake/                      # CMake 모듈
│
├── docs/                       # 문서
│   ├── README.md               # 문서 인덱스
│   ├── API_REFERENCE.md        # API 레퍼런스
│   ├── ARCHITECTURE.md         # 아키텍처 설계
│   ├── BENCHMARKS.md           # 성능 벤치마크
│   ├── CHANGELOG.md            # 변경 이력
│   ├── FEATURES.md             # 기능 설명
│   ├── PRODUCTION_QUALITY.md   # 품질 문서
│   ├── PROJECT_STRUCTURE.md    # 프로젝트 구조
│   ├── advanced/               # 고급 주제
│   ├── contributing/           # 기여 가이드
│   ├── guides/                 # 사용자 가이드
│   ├── integration/            # 통합 가이드
│   └── performance/            # 성능 문서
│
├── include/                    # 공개 헤더
│   └── messaging_system/
│       ├── message_bus.h
│       ├── topic_router.h
│       └── patterns/
│
├── src/                        # 소스 코드
│   ├── core/                   # 핵심 구현
│   ├── patterns/               # 메시징 패턴
│   └── serialization/          # 직렬화
│
├── tests/                      # 테스트
│   ├── unit/                   # 단위 테스트
│   └── integration/            # 통합 테스트
│
├── examples/                   # 예제 코드
│   ├── basic/                  # 기본 사용법
│   └── patterns/               # 패턴 예제
│
├── CMakeLists.txt              # 메인 CMake 파일
├── README.md                   # 프로젝트 README
└── LICENSE                     # 라이선스
```

---

## 주요 디렉토리 설명

### include/

공개 API 헤더 파일들이 위치합니다.

### src/

라이브러리 구현 코드입니다.

### docs/

프로젝트 문서입니다. 영어와 한국어 버전을 모두 제공합니다.

### tests/

테스트 코드입니다.

### examples/

사용 예제입니다.

---

## 관련 문서

- [아키텍처](ARCHITECTURE_KO.md)
- [기여하기](contributing/CONTRIBUTING.md)
