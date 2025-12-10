# Messaging System 문서

**언어**: [English](README.md) | [한국어]

---

## 문서 인덱스

이 디렉토리에는 messaging_system의 모든 문서가 포함되어 있습니다.

---

## 시작하기

| 문서 | 설명 |
|------|------|
| [빠른 시작](guides/QUICK_START.md) | 5분 만에 시작하기 |
| [개발자 가이드](guides/DEVELOPER_GUIDE.md) | 상세 개발 가이드 |
| [마이그레이션 가이드](guides/MIGRATION_GUIDE.md) | 버전 업그레이드 |

---

## 핵심 문서

| 문서 | 설명 |
|------|------|
| [기능](FEATURES_KO.md) | 전체 기능 설명 |
| [API 레퍼런스](API_REFERENCE_KO.md) | API 상세 문서 |
| [아키텍처](ARCHITECTURE_KO.md) | 시스템 설계 |
| [벤치마크](BENCHMARKS_KO.md) | 성능 측정 결과 |

---

## 품질 및 운영

| 문서 | 설명 |
|------|------|
| [프로덕션 품질](PRODUCTION_QUALITY_KO.md) | 품질 보증 |
| [변경 이력](CHANGELOG_KO.md) | 버전별 변경 사항 |
| [프로젝트 구조](PROJECT_STRUCTURE_KO.md) | 저장소 구조 |

---

## Task 큐 시스템

| 문서 | 설명 |
|------|------|
| [빠른 시작](task/QUICK_START_KO.md) | 5분 만에 Task 모듈 시작하기 |
| [아키텍처](task/ARCHITECTURE_KO.md) | 시스템 설계 및 컴포넌트 개요 |
| [API 레퍼런스](task/API_REFERENCE_KO.md) | 완전한 API 문서 |
| [워크플로우 패턴](task/PATTERNS_KO.md) | Chain, Chord, 재시도, 스케줄링 패턴 |
| [설정 가이드](task/CONFIGURATION_KO.md) | 설정 및 성능 튜닝 |
| [문제 해결](task/TROUBLESHOOTING_KO.md) | 일반적인 문제와 디버깅 |
| [마이그레이션](task/MIGRATION_KO.md) | 다른 시스템에서 마이그레이션 |

---

## 아키텍처 결정 기록 (ADR)

| 문서 | 설명 |
|------|------|
| [ADR 인덱스](adr/README.md) | ADR 목록 및 템플릿 |
| [ADR-001: 로깅 아키텍처](adr/001-logging-architecture_KO.md) | ILogger 인터페이스 vs logger_system |

---

## 디렉토리 구조

```
docs/
├── advanced/           # 고급 주제
│   ├── DESIGN_PATTERNS.md
│   ├── SYSTEM_ARCHITECTURE.md
│   └── ...
│
├── contributing/       # 기여 가이드
│   └── CONTRIBUTING.md
│
├── guides/             # 사용자 가이드
│   ├── QUICK_START.md
│   ├── DEVELOPER_GUIDE.md
│   ├── FAQ.md
│   └── TROUBLESHOOTING.md
│
├── integration/        # 통합 문서
│   └── README.md
│
├── performance/        # 성능 문서
│   ├── PERFORMANCE.md
│   └── BASELINE.md
│
├── task/               # Task 큐 시스템 문서
│   ├── QUICK_START.md
│   ├── ARCHITECTURE.md
│   ├── API_REFERENCE.md
│   ├── PATTERNS.md
│   ├── CONFIGURATION.md
│   ├── TROUBLESHOOTING.md
│   └── MIGRATION.md
│
└── adr/                # 아키텍처 결정 기록
    ├── README.md
    └── 001-logging-architecture.md
```

---

## 관련 링크

- [GitHub 저장소](https://github.com/kcenon/messaging_system)
- [이슈 트래커](https://github.com/kcenon/messaging_system/issues)
