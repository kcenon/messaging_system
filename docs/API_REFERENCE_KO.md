# Messaging System API 레퍼런스

**버전**: 0.1.0
**최종 수정일**: 2025-11-30
**언어**: [English](API_REFERENCE.md) | [한국어]

---

## 개요

이 문서는 messaging_system의 공개 API에 대한 포괄적인 레퍼런스를 제공합니다.

---

## 목차

1. [핵심 클래스](#핵심-클래스)
2. [메시지 버스](#메시지-버스)
3. [토픽 라우터](#토픽-라우터)
4. [메시징 패턴](#메시징-패턴)
5. [직렬화](#직렬화)

---

## 핵심 클래스

### message_bus

중앙 pub/sub 코디네이터:

```cpp
namespace messaging {

class message_bus {
public:
    // 구독
    auto subscribe(const std::string& topic,
                   message_handler handler) -> subscription_id;
    auto unsubscribe(subscription_id id) -> void;

    // 발행
    auto publish(const std::string& topic,
                 const message& msg) -> result_void;

    // 생명주기
    auto start() -> result_void;
    auto stop() -> result_void;
};

} // namespace messaging
```

---

## 메시지 버스

### 사용 예제

```cpp
auto bus = messaging::message_bus::create();
bus->start();

// 구독
auto sub_id = bus->subscribe("events.*", [](const message& msg) {
    // 메시지 처리
});

// 발행
bus->publish("events.user.created", message{user_data});

// 정리
bus->unsubscribe(sub_id);
bus->stop();
```

---

## 토픽 라우터

### 와일드카드 패턴

| 패턴 | 설명 | 예제 매칭 |
|------|------|----------|
| `*` | 단일 레벨 | `events.*` → `events.user` |
| `#` | 다중 레벨 | `events.#` → `events.user.created` |

---

## 메시징 패턴

### Pub/Sub

```cpp
// 퍼블리셔
auto publisher = messaging::publisher(bus, "notifications");
publisher.publish(notification_msg);

// 구독자
auto subscriber = messaging::subscriber(bus, "notifications");
subscriber.on_message([](const message& msg) { /* 처리 */ });
```

### Request/Reply

```cpp
auto client = messaging::request_client(bus, "services.calculator");
auto result = client.request(calculate_request);

if (result) {
    auto response = result.value();
}
```

---

## 직렬화

### 메시지 구조

```cpp
struct message {
    std::string topic;
    container_system::value payload;
    std::optional<trace_context> trace;
    std::chrono::system_clock::time_point timestamp;
};
```

---

## 관련 문서

- [기능](FEATURES_KO.md)
- [아키텍처](ARCHITECTURE_KO.md)
- [빠른 시작](guides/QUICK_START.md)
