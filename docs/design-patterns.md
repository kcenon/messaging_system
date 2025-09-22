# Design Patterns and Architectural Decisions

## Overview

This document catalogs the design patterns implemented throughout the Messaging System, explaining why each pattern was chosen and providing implementation examples. These patterns ensure scalability, maintainability, and performance in a distributed messaging environment.

## Creational Patterns

### 1. Singleton Pattern

**Purpose**: Ensure single instances of critical system components like loggers and configuration managers.

**Why Chosen**: Prevents resource conflicts and ensures consistent state across the application.

**Implementation**:
```cpp
class ConfigurationManager {
private:
    static std::unique_ptr<ConfigurationManager> instance_;
    static std::mutex mutex_;

    ConfigurationManager() {
        load_configuration();
    }

public:
    static ConfigurationManager& instance() {
        std::call_once(init_flag_, []() {
            instance_ = std::unique_ptr<ConfigurationManager>(
                new ConfigurationManager()
            );
        });
        return *instance_;
    }

    // Delete copy/move operations
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

private:
    static std::once_flag init_flag_;
};
```

**Usage in System**:
- Logger manager
- Database connection manager
- System configuration
- Metrics collector

### 2. Factory Pattern

**Purpose**: Create objects without specifying exact classes, enabling runtime polymorphism.

**Why Chosen**: Allows flexible service creation based on configuration and supports plugin architecture.

**Implementation**:
```cpp
class ServiceFactory {
public:
    using ServiceCreator = std::function<std::unique_ptr<IService>()>;

    void register_service(const std::string& type, ServiceCreator creator) {
        creators_[type] = creator;
    }

    std::unique_ptr<IService> create_service(const std::string& type) {
        auto it = creators_.find(type);
        if (it != creators_.end()) {
            return it->second();
        }
        throw std::runtime_error("Unknown service type: " + type);
    }

private:
    std::unordered_map<std::string, ServiceCreator> creators_;
};

// Registration
factory.register_service("network", []() {
    return std::make_unique<NetworkService>();
});

factory.register_service("database", []() {
    return std::make_unique<DatabaseService>();
});

// Usage
auto service = factory.create_service(config.service_type);
```

**Usage in System**:
- Message handler creation
- Protocol adapter instantiation
- Storage backend selection
- Serializer selection

### 3. Builder Pattern

**Purpose**: Construct complex objects step by step with fluent interface.

**Why Chosen**: Simplifies creation of messages and configurations with many optional parameters.

**Implementation**:
```cpp
class MessageBuilder {
private:
    std::unique_ptr<Message> message_;

public:
    MessageBuilder() : message_(std::make_unique<Message>()) {}

    MessageBuilder& with_topic(const std::string& topic) {
        message_->set_topic(topic);
        return *this;
    }

    MessageBuilder& with_payload(const Payload& payload) {
        message_->set_payload(payload);
        return *this;
    }

    MessageBuilder& with_priority(Priority priority) {
        message_->set_priority(priority);
        return *this;
    }

    MessageBuilder& with_timeout(std::chrono::milliseconds timeout) {
        message_->set_timeout(timeout);
        return *this;
    }

    std::unique_ptr<Message> build() {
        validate();
        return std::move(message_);
    }

private:
    void validate() {
        if (message_->topic().empty()) {
            throw std::runtime_error("Message must have a topic");
        }
    }
};

// Usage
auto message = MessageBuilder()
    .with_topic("user.created")
    .with_payload({"user_id", 12345})
    .with_priority(Priority::HIGH)
    .with_timeout(5s)
    .build();
```

**Usage in System**:
- Complex message construction
- Database query builders
- Configuration builders
- Network request builders

### 4. Object Pool Pattern

**Purpose**: Reuse expensive objects to reduce allocation overhead.

**Why Chosen**: Minimizes memory allocation in hot paths, crucial for high-performance messaging.

**Implementation**:
```cpp
template<typename T>
class ObjectPool {
private:
    std::queue<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
    std::function<std::unique_ptr<T>()> creator_;
    size_t max_size_;

public:
    class PooledObject {
        ObjectPool* pool_;
        std::unique_ptr<T> object_;

    public:
        PooledObject(ObjectPool* pool, std::unique_ptr<T> obj)
            : pool_(pool), object_(std::move(obj)) {}

        ~PooledObject() {
            if (object_ && pool_) {
                pool_->return_object(std::move(object_));
            }
        }

        T* operator->() { return object_.get(); }
        T& operator*() { return *object_; }
    };

    ObjectPool(size_t max_size, std::function<std::unique_ptr<T>()> creator)
        : max_size_(max_size), creator_(creator) {
        // Pre-allocate objects
        for (size_t i = 0; i < max_size / 2; ++i) {
            pool_.push(creator_());
        }
    }

    PooledObject acquire() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (pool_.empty()) {
            return PooledObject(this, creator_());
        }

        auto obj = std::move(pool_.front());
        pool_.pop();
        return PooledObject(this, std::move(obj));
    }

private:
    void return_object(std::unique_ptr<T> obj) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (pool_.size() < max_size_) {
            obj->reset();  // Reset object state
            pool_.push(std::move(obj));
        }
    }
};

// Usage
ObjectPool<Buffer> buffer_pool(100, []() {
    return std::make_unique<Buffer>(4096);
});

auto buffer = buffer_pool.acquire();
buffer->write(data);
// Buffer automatically returned to pool when destroyed
```

**Usage in System**:
- Message object pooling
- Buffer management
- Database connection pooling
- Thread-local storage pools

## Structural Patterns

### 5. Adapter Pattern

**Purpose**: Allow incompatible interfaces to work together.

**Why Chosen**: Enables integration with various external systems and protocols.

**Implementation**:
```cpp
// Target interface expected by our system
class IMessageQueue {
public:
    virtual void send(const Message& msg) = 0;
    virtual Message receive() = 0;
    virtual ~IMessageQueue() = default;
};

// External RabbitMQ library with different interface
class RabbitMQClient {
public:
    void publish(const std::string& exchange, const std::string& data);
    std::string consume(const std::string& queue);
};

// Adapter
class RabbitMQAdapter : public IMessageQueue {
private:
    std::unique_ptr<RabbitMQClient> client_;
    std::string exchange_;
    std::string queue_;

public:
    RabbitMQAdapter(const std::string& exchange, const std::string& queue)
        : client_(std::make_unique<RabbitMQClient>())
        , exchange_(exchange)
        , queue_(queue) {}

    void send(const Message& msg) override {
        client_->publish(exchange_, msg.serialize());
    }

    Message receive() override {
        auto data = client_->consume(queue_);
        return Message::deserialize(data);
    }
};

// Usage
std::unique_ptr<IMessageQueue> queue =
    std::make_unique<RabbitMQAdapter>("events", "worker_queue");
queue->send(message);
```

**Usage in System**:
- External message queue adapters (RabbitMQ, Kafka)
- Database adapters (PostgreSQL, MySQL)
- Protocol adapters (HTTP, WebSocket)
- Logging adapters (syslog, file, console)

### 6. Decorator Pattern

**Purpose**: Add responsibilities to objects dynamically without altering structure.

**Why Chosen**: Allows flexible message processing pipelines with composable middleware.

**Implementation**:
```cpp
class IMessageProcessor {
public:
    virtual void process(Message& msg) = 0;
    virtual ~IMessageProcessor() = default;
};

class BaseProcessor : public IMessageProcessor {
public:
    void process(Message& msg) override {
        // Basic processing
    }
};

// Decorator base
class ProcessorDecorator : public IMessageProcessor {
protected:
    std::unique_ptr<IMessageProcessor> processor_;

public:
    ProcessorDecorator(std::unique_ptr<IMessageProcessor> processor)
        : processor_(std::move(processor)) {}

    void process(Message& msg) override {
        processor_->process(msg);
    }
};

// Concrete decorators
class LoggingDecorator : public ProcessorDecorator {
public:
    using ProcessorDecorator::ProcessorDecorator;

    void process(Message& msg) override {
        logger_.info("Processing message: {}", msg.id());
        processor_->process(msg);
        logger_.info("Processed message: {}", msg.id());
    }
};

class CompressionDecorator : public ProcessorDecorator {
public:
    using ProcessorDecorator::ProcessorDecorator;

    void process(Message& msg) override {
        decompress(msg);
        processor_->process(msg);
        compress(msg);
    }
};

class EncryptionDecorator : public ProcessorDecorator {
public:
    using ProcessorDecorator::ProcessorDecorator;

    void process(Message& msg) override {
        decrypt(msg);
        processor_->process(msg);
        encrypt(msg);
    }
};

// Build processing pipeline
auto processor = std::make_unique<BaseProcessor>();
processor = std::make_unique<LoggingDecorator>(std::move(processor));
processor = std::make_unique<CompressionDecorator>(std::move(processor));
processor = std::make_unique<EncryptionDecorator>(std::move(processor));

processor->process(message);
```

**Usage in System**:
- Message processing pipelines
- Request/response interceptors
- Metrics collection
- Security layers

### 7. Proxy Pattern

**Purpose**: Provide placeholder or surrogate for another object to control access.

**Why Chosen**: Implements lazy loading, caching, and access control for remote services.

**Implementation**:
```cpp
class IRemoteService {
public:
    virtual Response execute(const Request& req) = 0;
    virtual ~IRemoteService() = default;
};

class RemoteService : public IRemoteService {
public:
    Response execute(const Request& req) override {
        // Actual remote call
        return make_remote_call(req);
    }
};

class CachingProxy : public IRemoteService {
private:
    std::unique_ptr<IRemoteService> service_;
    std::unordered_map<Request, Response, RequestHash> cache_;
    std::mutex cache_mutex_;
    std::chrono::seconds ttl_{60};

    struct CacheEntry {
        Response response;
        std::chrono::steady_clock::time_point timestamp;
    };

public:
    CachingProxy(std::unique_ptr<IRemoteService> service)
        : service_(std::move(service)) {}

    Response execute(const Request& req) override {
        // Check cache
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = cache_.find(req);
            if (it != cache_.end()) {
                auto age = std::chrono::steady_clock::now() - it->second.timestamp;
                if (age < ttl_) {
                    return it->second.response;  // Cache hit
                }
                cache_.erase(it);  // Expired
            }
        }

        // Cache miss - execute request
        auto response = service_->execute(req);

        // Update cache
        {
            std::lock_guard<std::mutex> lock(cache_mutex_);
            cache_[req] = {response, std::chrono::steady_clock::now()};
        }

        return response;
    }
};

// Usage
auto service = std::make_unique<RemoteService>();
auto proxy = std::make_unique<CachingProxy>(std::move(service));
auto response = proxy->execute(request);  // Cached for subsequent calls
```

**Usage in System**:
- Remote service proxies
- Database query caching
- Authentication proxies
- Rate limiting proxies

### 8. Composite Pattern

**Purpose**: Compose objects into tree structures to represent part-whole hierarchies.

**Why Chosen**: Enables hierarchical message routing and complex filter compositions.

**Implementation**:
```cpp
class IMessageFilter {
public:
    virtual bool matches(const Message& msg) = 0;
    virtual ~IMessageFilter() = default;
};

// Leaf filters
class TopicFilter : public IMessageFilter {
    std::string pattern_;

public:
    TopicFilter(const std::string& pattern) : pattern_(pattern) {}

    bool matches(const Message& msg) override {
        return std::regex_match(msg.topic(), std::regex(pattern_));
    }
};

class PriorityFilter : public IMessageFilter {
    Priority min_priority_;

public:
    PriorityFilter(Priority min) : min_priority_(min) {}

    bool matches(const Message& msg) override {
        return msg.priority() >= min_priority_;
    }
};

// Composite filters
class CompositeFilter : public IMessageFilter {
protected:
    std::vector<std::unique_ptr<IMessageFilter>> filters_;

public:
    void add(std::unique_ptr<IMessageFilter> filter) {
        filters_.push_back(std::move(filter));
    }
};

class AndFilter : public CompositeFilter {
public:
    bool matches(const Message& msg) override {
        return std::all_of(filters_.begin(), filters_.end(),
            [&msg](const auto& filter) {
                return filter->matches(msg);
            });
    }
};

class OrFilter : public CompositeFilter {
public:
    bool matches(const Message& msg) override {
        return std::any_of(filters_.begin(), filters_.end(),
            [&msg](const auto& filter) {
                return filter->matches(msg);
            });
    }
};

// Build complex filter
auto filter = std::make_unique<AndFilter>();
filter->add(std::make_unique<TopicFilter>("user\\..*"));
filter->add(std::make_unique<PriorityFilter>(Priority::HIGH));

auto or_filter = std::make_unique<OrFilter>();
or_filter->add(std::make_unique<TopicFilter>("system\\..*"));
or_filter->add(std::move(filter));

if (or_filter->matches(message)) {
    // Process message
}
```

**Usage in System**:
- Message routing rules
- Complex query filters
- Permission hierarchies
- Configuration trees

## Behavioral Patterns

### 9. Observer Pattern

**Purpose**: Define one-to-many dependency between objects for event notification.

**Why Chosen**: Core to pub/sub messaging system, enables loose coupling between components.

**Implementation**:
```cpp
template<typename EventType>
class EventBus {
public:
    using Handler = std::function<void(const EventType&)>;
    using HandlerId = size_t;

private:
    struct Subscription {
        HandlerId id;
        Handler handler;
        std::weak_ptr<void> lifetime;
    };

    std::vector<Subscription> subscribers_;
    std::mutex mutex_;
    std::atomic<HandlerId> next_id_{0};

public:
    HandlerId subscribe(Handler handler,
                       std::shared_ptr<void> lifetime = nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        HandlerId id = next_id_++;
        subscribers_.push_back({id, handler, lifetime});
        return id;
    }

    void unsubscribe(HandlerId id) {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.erase(
            std::remove_if(subscribers_.begin(), subscribers_.end(),
                [id](const auto& sub) { return sub.id == id; }),
            subscribers_.end()
        );
    }

    void publish(const EventType& event) {
        std::vector<Handler> handlers;

        // Copy handlers to avoid holding lock during callbacks
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Clean up expired subscriptions
            subscribers_.erase(
                std::remove_if(subscribers_.begin(), subscribers_.end(),
                    [](const auto& sub) { return sub.lifetime.expired(); }),
                subscribers_.end()
            );

            // Collect active handlers
            for (const auto& sub : subscribers_) {
                handlers.push_back(sub.handler);
            }
        }

        // Execute handlers
        for (const auto& handler : handlers) {
            try {
                handler(event);
            } catch (const std::exception& e) {
                // Log error but continue notifying others
                logger_.error("Handler error: {}", e.what());
            }
        }
    }
};

// Usage
EventBus<UserCreatedEvent> user_events;

// Subscribe
auto id = user_events.subscribe([](const UserCreatedEvent& event) {
    std::cout << "New user: " << event.username << std::endl;
});

// Publish
user_events.publish(UserCreatedEvent{"john_doe", "john@example.com"});

// Unsubscribe
user_events.unsubscribe(id);
```

**Usage in System**:
- Message bus pub/sub
- Connection state changes
- Configuration updates
- System event notifications

### 10. Strategy Pattern

**Purpose**: Define family of algorithms and make them interchangeable.

**Why Chosen**: Allows runtime selection of serialization formats, compression algorithms, and routing strategies.

**Implementation**:
```cpp
class ISerializationStrategy {
public:
    virtual std::string serialize(const Message& msg) = 0;
    virtual Message deserialize(const std::string& data) = 0;
    virtual ~ISerializationStrategy() = default;
};

class BinarySerializer : public ISerializationStrategy {
public:
    std::string serialize(const Message& msg) override {
        // Binary serialization logic
        return binary_encode(msg);
    }

    Message deserialize(const std::string& data) override {
        return binary_decode(data);
    }
};

class JsonSerializer : public ISerializationStrategy {
public:
    std::string serialize(const Message& msg) override {
        nlohmann::json j;
        j["id"] = msg.id();
        j["topic"] = msg.topic();
        j["payload"] = msg.payload();
        return j.dump();
    }

    Message deserialize(const std::string& data) override {
        auto j = nlohmann::json::parse(data);
        Message msg;
        msg.set_id(j["id"]);
        msg.set_topic(j["topic"]);
        msg.set_payload(j["payload"]);
        return msg;
    }
};

class ProtobufSerializer : public ISerializationStrategy {
public:
    std::string serialize(const Message& msg) override {
        // Protobuf serialization
        return protobuf_encode(msg);
    }

    Message deserialize(const std::string& data) override {
        return protobuf_decode(data);
    }
};

// Context class
class MessageSerializer {
    std::unique_ptr<ISerializationStrategy> strategy_;

public:
    void set_strategy(std::unique_ptr<ISerializationStrategy> strategy) {
        strategy_ = std::move(strategy);
    }

    std::string serialize(const Message& msg) {
        if (!strategy_) {
            throw std::runtime_error("No serialization strategy set");
        }
        return strategy_->serialize(msg);
    }

    Message deserialize(const std::string& data) {
        if (!strategy_) {
            throw std::runtime_error("No serialization strategy set");
        }
        return strategy_->deserialize(data);
    }
};

// Usage
MessageSerializer serializer;

// Select strategy based on configuration
switch (config.format) {
    case Format::BINARY:
        serializer.set_strategy(std::make_unique<BinarySerializer>());
        break;
    case Format::JSON:
        serializer.set_strategy(std::make_unique<JsonSerializer>());
        break;
    case Format::PROTOBUF:
        serializer.set_strategy(std::make_unique<ProtobufSerializer>());
        break;
}

auto data = serializer.serialize(message);
```

**Usage in System**:
- Serialization strategies
- Compression algorithms
- Load balancing strategies
- Retry strategies

### 11. Chain of Responsibility Pattern

**Purpose**: Pass requests along a chain of handlers until one handles it.

**Why Chosen**: Perfect for message processing pipelines with middleware.

**Implementation**:
```cpp
class MessageHandler {
protected:
    std::unique_ptr<MessageHandler> next_;

public:
    void set_next(std::unique_ptr<MessageHandler> next) {
        if (next_) {
            next_->set_next(std::move(next));
        } else {
            next_ = std::move(next);
        }
    }

    virtual void handle(Message& msg) {
        if (!process(msg) && next_) {
            next_->handle(msg);
        }
    }

protected:
    virtual bool process(Message& msg) = 0;
};

class AuthenticationHandler : public MessageHandler {
protected:
    bool process(Message& msg) override {
        if (!msg.has_auth_token()) {
            throw std::runtime_error("Authentication required");
        }

        if (!validate_token(msg.auth_token())) {
            throw std::runtime_error("Invalid authentication token");
        }

        return false;  // Continue to next handler
    }
};

class RateLimitHandler : public MessageHandler {
    RateLimiter limiter_;

protected:
    bool process(Message& msg) override {
        if (!limiter_.allow_request(msg.sender_id())) {
            throw std::runtime_error("Rate limit exceeded");
        }
        return false;  // Continue to next handler
    }
};

class ValidationHandler : public MessageHandler {
protected:
    bool process(Message& msg) override {
        if (msg.size() > MAX_MESSAGE_SIZE) {
            throw std::runtime_error("Message too large");
        }

        if (!is_valid_topic(msg.topic())) {
            throw std::runtime_error("Invalid topic");
        }

        return false;  // Continue to next handler
    }
};

class RouterHandler : public MessageHandler {
    MessageRouter router_;

protected:
    bool process(Message& msg) override {
        router_.route(msg);
        return true;  // Message handled, stop chain
    }
};

// Build processing chain
auto chain = std::make_unique<AuthenticationHandler>();
chain->set_next(std::make_unique<RateLimitHandler>());
chain->set_next(std::make_unique<ValidationHandler>());
chain->set_next(std::make_unique<RouterHandler>());

// Process message through chain
try {
    chain->handle(message);
} catch (const std::exception& e) {
    logger_.error("Message processing failed: {}", e.what());
}
```

**Usage in System**:
- Request processing pipeline
- Error handling chain
- Message transformation pipeline
- Security filter chain

### 12. Command Pattern

**Purpose**: Encapsulate requests as objects for parameterization and queuing.

**Why Chosen**: Enables undo operations, request queuing, and transaction support.

**Implementation**:
```cpp
class ICommand {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~ICommand() = default;
};

class DatabaseCommand : public ICommand {
protected:
    DatabaseManager& db_;

public:
    DatabaseCommand(DatabaseManager& db) : db_(db) {}
};

class InsertCommand : public DatabaseCommand {
    std::string table_;
    std::map<std::string, std::any> data_;
    int64_t inserted_id_ = -1;

public:
    InsertCommand(DatabaseManager& db, const std::string& table,
                  const std::map<std::string, std::any>& data)
        : DatabaseCommand(db), table_(table), data_(data) {}

    void execute() override {
        inserted_id_ = db_.insert(table_, data_);
    }

    void undo() override {
        if (inserted_id_ != -1) {
            db_.delete_by_id(table_, inserted_id_);
        }
    }
};

class UpdateCommand : public DatabaseCommand {
    std::string table_;
    int64_t id_;
    std::map<std::string, std::any> new_data_;
    std::map<std::string, std::any> old_data_;

public:
    UpdateCommand(DatabaseManager& db, const std::string& table,
                  int64_t id, const std::map<std::string, std::any>& data)
        : DatabaseCommand(db), table_(table), id_(id), new_data_(data) {}

    void execute() override {
        old_data_ = db_.select_by_id(table_, id_);
        db_.update(table_, id_, new_data_);
    }

    void undo() override {
        db_.update(table_, id_, old_data_);
    }
};

class TransactionManager {
    std::vector<std::unique_ptr<ICommand>> commands_;
    std::stack<ICommand*> executed_;

public:
    void add_command(std::unique_ptr<ICommand> cmd) {
        commands_.push_back(std::move(cmd));
    }

    void execute_all() {
        try {
            for (auto& cmd : commands_) {
                cmd->execute();
                executed_.push(cmd.get());
            }
        } catch (...) {
            rollback();
            throw;
        }
    }

    void rollback() {
        while (!executed_.empty()) {
            executed_.top()->undo();
            executed_.pop();
        }
    }
};

// Usage
TransactionManager transaction;

transaction.add_command(
    std::make_unique<InsertCommand>(db, "users", {
        {"name", "John Doe"},
        {"email", "john@example.com"}
    })
);

transaction.add_command(
    std::make_unique<UpdateCommand>(db, "accounts", account_id, {
        {"balance", new_balance}
    })
);

try {
    transaction.execute_all();
} catch (const std::exception& e) {
    // Automatic rollback
    logger_.error("Transaction failed: {}", e.what());
}
```

**Usage in System**:
- Database transactions
- Message queue operations
- Configuration changes
- Batch processing

### 13. Template Method Pattern

**Purpose**: Define skeleton of algorithm in base class, subclasses override specific steps.

**Why Chosen**: Standardizes service lifecycle and message processing flow.

**Implementation**:
```cpp
class Service {
public:
    void start() {
        logger_.info("Starting service: {}", name());

        if (!initialize()) {
            throw std::runtime_error("Initialization failed");
        }

        if (!connect()) {
            cleanup();
            throw std::runtime_error("Connection failed");
        }

        run();

        logger_.info("Service started: {}", name());
    }

    void stop() {
        logger_.info("Stopping service: {}", name());

        shutdown();
        disconnect();
        cleanup();

        logger_.info("Service stopped: {}", name());
    }

protected:
    // Template methods to be overridden
    virtual std::string name() const = 0;
    virtual bool initialize() = 0;
    virtual bool connect() = 0;
    virtual void run() = 0;
    virtual void shutdown() = 0;
    virtual void disconnect() = 0;
    virtual void cleanup() = 0;
};

class NetworkService : public Service {
    boost::asio::io_context io_context_;
    std::unique_ptr<tcp::acceptor> acceptor_;

protected:
    std::string name() const override {
        return "NetworkService";
    }

    bool initialize() override {
        // Initialize networking resources
        return true;
    }

    bool connect() override {
        acceptor_ = std::make_unique<tcp::acceptor>(io_context_,
            tcp::endpoint(tcp::v4(), port_));
        return true;
    }

    void run() override {
        accept_connections();
    }

    void shutdown() override {
        io_context_.stop();
    }

    void disconnect() override {
        acceptor_.reset();
    }

    void cleanup() override {
        // Clean up resources
    }
};

// Usage
auto service = std::make_unique<NetworkService>();
service->start();  // Follows template method pattern
// ...
service->stop();
```

**Usage in System**:
- Service lifecycle management
- Connection establishment
- Request processing flow
- Resource initialization

## Concurrency Patterns

### 14. Producer-Consumer Pattern

**Purpose**: Decouple production and consumption of data with buffering.

**Why Chosen**: Essential for async message processing and work distribution.

**Implementation**:
```cpp
template<typename T>
class ProducerConsumerQueue {
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    size_t max_size_;
    std::atomic<bool> shutdown_{false};

public:
    explicit ProducerConsumerQueue(size_t max_size)
        : max_size_(max_size) {}

    void produce(T item) {
        std::unique_lock<std::mutex> lock(mutex_);

        not_full_.wait(lock, [this] {
            return queue_.size() < max_size_ || shutdown_;
        });

        if (shutdown_) {
            throw std::runtime_error("Queue is shutting down");
        }

        queue_.push(std::move(item));
        not_empty_.notify_one();
    }

    std::optional<T> consume() {
        std::unique_lock<std::mutex> lock(mutex_);

        not_empty_.wait(lock, [this] {
            return !queue_.empty() || shutdown_;
        });

        if (queue_.empty()) {
            return std::nullopt;  // Shutdown case
        }

        T item = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();

        return item;
    }

    void shutdown() {
        shutdown_ = true;
        not_empty_.notify_all();
        not_full_.notify_all();
    }
};

// Producer
void producer(ProducerConsumerQueue<Message>& queue) {
    for (int i = 0; i < 1000; ++i) {
        queue.produce(create_message(i));
    }
}

// Consumer
void consumer(ProducerConsumerQueue<Message>& queue) {
    while (auto msg = queue.consume()) {
        process_message(*msg);
    }
}

// Usage
ProducerConsumerQueue<Message> queue(100);

std::vector<std::thread> producers;
for (int i = 0; i < 2; ++i) {
    producers.emplace_back(producer, std::ref(queue));
}

std::vector<std::thread> consumers;
for (int i = 0; i < 4; ++i) {
    consumers.emplace_back(consumer, std::ref(queue));
}
```

**Usage in System**:
- Message queue implementation
- Work distribution
- Async I/O buffering
- Log message processing

### 15. Thread Pool Pattern

**Purpose**: Reuse fixed number of threads for executing tasks.

**Why Chosen**: Reduces thread creation overhead and controls resource usage.

**Implementation**:
```cpp
class ThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> active_tasks_{0};

public:
    explicit ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this, i] {
                worker_thread(i);
            });
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) {
        using ReturnType = decltype(f(args...));

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        auto future = task->get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex_);

            if (stop_) {
                throw std::runtime_error("ThreadPool is stopped");
            }

            tasks_.emplace([task]() { (*task)(); });
        }

        condition_.notify_one();
        return future;
    }

    void wait_all() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        condition_.wait(lock, [this] {
            return tasks_.empty() && active_tasks_ == 0;
        });
    }

private:
    void worker_thread(size_t id) {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);

                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
                active_tasks_++;
            }

            try {
                task();
            } catch (const std::exception& e) {
                logger_.error("Task failed in worker {}: {}", id, e.what());
            }

            active_tasks_--;
            condition_.notify_all();
        }
    }

    void shutdown() {
        stop_ = true;
        condition_.notify_all();

        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};
```

**Usage in System**:
- Message processing workers
- Database query execution
- Network I/O handling
- Background tasks

## Architectural Patterns

### 16. Microkernel Pattern

**Purpose**: Separate minimal core functionality from extended features via plugins.

**Why Chosen**: Allows extensible system with pluggable components.

**Implementation**:
```cpp
class MessageKernel {
    std::vector<std::unique_ptr<IPlugin>> plugins_;
    std::unordered_map<std::string, IPlugin*> plugin_registry_;

public:
    void register_plugin(std::unique_ptr<IPlugin> plugin) {
        auto name = plugin->name();
        plugin->initialize(this);
        plugin_registry_[name] = plugin.get();
        plugins_.push_back(std::move(plugin));
    }

    template<typename T>
    T* get_plugin(const std::string& name) {
        auto it = plugin_registry_.find(name);
        if (it != plugin_registry_.end()) {
            return dynamic_cast<T*>(it->second);
        }
        return nullptr;
    }

    void process_message(Message& msg) {
        for (auto& plugin : plugins_) {
            if (plugin->can_handle(msg)) {
                plugin->handle(msg);
            }
        }
    }
};

class IPlugin {
public:
    virtual std::string name() const = 0;
    virtual void initialize(MessageKernel* kernel) = 0;
    virtual bool can_handle(const Message& msg) = 0;
    virtual void handle(Message& msg) = 0;
};

class LoggingPlugin : public IPlugin {
    MessageKernel* kernel_;

public:
    std::string name() const override { return "logging"; }

    void initialize(MessageKernel* kernel) override {
        kernel_ = kernel;
    }

    bool can_handle(const Message& msg) override {
        return true;  // Log all messages
    }

    void handle(Message& msg) override {
        logger_.info("Message: {}", msg.id());
    }
};
```

**Usage in System**:
- Plugin architecture
- Service extensions
- Protocol handlers
- Custom processors

### 17. Event-Driven Architecture

**Purpose**: System components communicate through events rather than direct calls.

**Why Chosen**: Enables loose coupling and scalable distributed systems.

**Implementation**:
```cpp
class EventDrivenSystem {
    struct EventHandler {
        std::string event_type;
        std::function<void(const Event&)> handler;
        Priority priority;
    };

    std::multimap<std::string, EventHandler> handlers_;
    std::priority_queue<Event> event_queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> running_{true};

public:
    void register_handler(const std::string& event_type,
                         std::function<void(const Event&)> handler,
                         Priority priority = Priority::NORMAL) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers_.emplace(event_type, EventHandler{event_type, handler, priority});
    }

    void emit(const Event& event) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            event_queue_.push(event);
        }
        cv_.notify_one();
    }

    void start(size_t num_workers) {
        for (size_t i = 0; i < num_workers; ++i) {
            workers_.emplace_back([this] { process_events(); });
        }
    }

private:
    void process_events() {
        while (running_) {
            Event event;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] {
                    return !event_queue_.empty() || !running_;
                });

                if (!running_) break;

                event = event_queue_.top();
                event_queue_.pop();
            }

            auto range = handlers_.equal_range(event.type());
            for (auto it = range.first; it != range.second; ++it) {
                try {
                    it->second.handler(event);
                } catch (const std::exception& e) {
                    logger_.error("Event handler failed: {}", e.what());
                }
            }
        }
    }
};
```

**Usage in System**:
- Core messaging infrastructure
- System event handling
- Async processing
- Service communication

## Summary

The Messaging System leverages these design patterns to achieve:

1. **Flexibility**: Factory and Strategy patterns enable runtime configuration
2. **Scalability**: Producer-Consumer and Thread Pool patterns handle load
3. **Maintainability**: Template Method and Decorator patterns provide structure
4. **Performance**: Object Pool and Proxy patterns optimize resources
5. **Reliability**: Command and Observer patterns ensure consistency
6. **Extensibility**: Plugin and Adapter patterns allow easy integration

Each pattern is carefully chosen to solve specific architectural challenges while maintaining system coherence and performance requirements.