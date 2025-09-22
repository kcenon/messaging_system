# gRPC Adapter Implementation Guide

**Zero Modification Approach - Technical Specification**

---

## 1. Complete Adapter Implementation

### 1.1 Value Mapper (Type Conversion)

```cpp
// grpc/adapters/value_mapper.cpp
#include "value_mapper.h"
#include "container/values/bool_value.h"
#include "container/values/numeric_value.h"
#include "container/values/string_value.h"
#include "container/values/bytes_value.h"
#include "container/values/container_value.h"

namespace container_grpc {

GrpcValue value_mapper::to_grpc_value(
    const std::shared_ptr<container_module::value>& val) {
    
    GrpcValue grpc_val;
    grpc_val.set_key(val->name());
    
    // Determine type from serialized format or type info
    auto type = val->type();
    
    switch(type) {
        case container_module::value_types::bool_value: {
            auto bool_val = std::dynamic_pointer_cast<
                container_module::bool_value>(val);
            if (bool_val) {
                grpc_val.set_bool_val(bool_val->data());
            }
            break;
        }
        
        case container_module::value_types::int_value: {
            auto int_val = std::dynamic_pointer_cast<
                container_module::numeric_value<int>>(val);
            if (int_val) {
                grpc_val.set_int_val(int_val->data());
            }
            break;
        }
        
        case container_module::value_types::long_value: {
            auto long_val = std::dynamic_pointer_cast<
                container_module::numeric_value<long>>(val);
            if (long_val) {
                grpc_val.set_long_val(long_val->data());
            }
            break;
        }
        
        case container_module::value_types::float_value: {
            auto float_val = std::dynamic_pointer_cast<
                container_module::numeric_value<float>>(val);
            if (float_val) {
                grpc_val.set_float_val(float_val->data());
            }
            break;
        }
        
        case container_module::value_types::double_value: {
            auto double_val = std::dynamic_pointer_cast<
                container_module::numeric_value<double>>(val);
            if (double_val) {
                grpc_val.set_double_val(double_val->data());
            }
            break;
        }
        
        case container_module::value_types::string_value: {
            auto str_val = std::dynamic_pointer_cast<
                container_module::string_value>(val);
            if (str_val) {
                grpc_val.set_string_val(str_val->data());
            }
            break;
        }
        
        case container_module::value_types::bytes_value: {
            auto bytes_val = std::dynamic_pointer_cast<
                container_module::bytes_value>(val);
            if (bytes_val) {
                grpc_val.set_bytes_val(
                    bytes_val->data(), bytes_val->size());
            }
            break;
        }
        
        case container_module::value_types::container_value: {
            auto container_val = std::dynamic_pointer_cast<
                container_module::container_value>(val);
            if (container_val && container_val->get_container()) {
                *grpc_val.mutable_nested_container() = 
                    container_adapter::to_grpc(*container_val->get_container());
            }
            break;
        }
        
        default:
            // Handle as string fallback
            grpc_val.set_string_val(val->to_string());
            break;
    }
    
    return grpc_val;
}

std::shared_ptr<container_module::value> 
value_mapper::from_grpc_value(const GrpcValue& grpc_val) {
    
    const std::string& key = grpc_val.key();
    
    switch(grpc_val.value_type_case()) {
        case GrpcValue::kBoolVal:
            return std::make_shared<container_module::bool_value>(
                key, grpc_val.bool_val());
            
        case GrpcValue::kIntVal:
            return std::make_shared<container_module::numeric_value<int>>(
                key, grpc_val.int_val());
            
        case GrpcValue::kLongVal:
            return std::make_shared<container_module::numeric_value<long>>(
                key, grpc_val.long_val());
            
        case GrpcValue::kFloatVal:
            return std::make_shared<container_module::numeric_value<float>>(
                key, grpc_val.float_val());
            
        case GrpcValue::kDoubleVal:
            return std::make_shared<container_module::numeric_value<double>>(
                key, grpc_val.double_val());
            
        case GrpcValue::kStringVal:
            return std::make_shared<container_module::string_value>(
                key, grpc_val.string_val());
            
        case GrpcValue::kBytesVal: {
            const std::string& bytes = grpc_val.bytes_val();
            return std::make_shared<container_module::bytes_value>(
                key, bytes.data(), bytes.size());
        }
        
        case GrpcValue::kNestedContainer: {
            auto nested = container_adapter::from_grpc(
                grpc_val.nested_container());
            return std::make_shared<container_module::container_value>(
                key, nested);
        }
        
        default:
            // Return null value
            return std::make_shared<container_module::value>(key);
    }
}

} // namespace container_grpc
```

### 1.2 Container Adapter (Main Conversion)

```cpp
// grpc/adapters/container_adapter.cpp
#include "container_adapter.h"
#include "value_mapper.h"

namespace container_grpc {

GrpcContainer container_adapter::to_grpc(
    const container_module::value_container& container) {
    
    GrpcContainer grpc_container;
    
    // Copy header information
    grpc_container.set_source_id(container.source_id());
    grpc_container.set_target_id(container.target_id());
    grpc_container.set_message_type(container.message_type());
    
    // Optional: Add sub-ids as metadata
    if (!container.source_sub_id().empty()) {
        (*grpc_container.mutable_metadata())["source_sub_id"] = 
            container.source_sub_id();
    }
    if (!container.target_sub_id().empty()) {
        (*grpc_container.mutable_metadata())["target_sub_id"] = 
            container.target_sub_id();
    }
    
    // Convert all values
    // Note: We use the public API to iterate values
    for (size_t i = 0; i < container.size(); ++i) {
        auto val = container.get_value_by_index(i);
        if (val) {
            *grpc_container.add_values() = 
                value_mapper::to_grpc_value(val);
        }
    }
    
    return grpc_container;
}

std::shared_ptr<container_module::value_container> 
container_adapter::from_grpc(const GrpcContainer& grpc) {
    
    auto container = std::make_shared<
        container_module::value_container>();
    
    // Set header information
    container->set_source(grpc.source_id(), "");
    container->set_target(grpc.target_id(), "");
    container->set_message_type(grpc.message_type());
    
    // Restore sub-ids from metadata if present
    auto metadata = grpc.metadata();
    if (metadata.count("source_sub_id")) {
        container->set_source(grpc.source_id(), 
            metadata.at("source_sub_id"));
    }
    if (metadata.count("target_sub_id")) {
        container->set_target(grpc.target_id(), 
            metadata.at("target_sub_id"));
    }
    
    // Convert and add all values
    std::vector<std::shared_ptr<container_module::value>> values;
    for (const auto& grpc_val : grpc.values()) {
        auto val = value_mapper::from_grpc_value(grpc_val);
        if (val) {
            values.push_back(val);
        }
    }
    
    if (!values.empty()) {
        container->set_units(values);
    }
    
    return container;
}

// Helper: Convert using serialization (alternative approach)
GrpcContainer container_adapter::to_grpc_via_serialization(
    const container_module::value_container& container) {
    
    GrpcContainer grpc;
    
    // Use JSON as intermediate format (already supported)
    std::string json = container.to_json();
    grpc.set_serialized_json(json);
    
    return grpc;
}

std::shared_ptr<container_module::value_container>
container_adapter::from_grpc_via_serialization(
    const GrpcContainer& grpc) {
    
    if (grpc.has_serialized_json()) {
        auto container = std::make_shared<
            container_module::value_container>();
        container->from_json(grpc.serialized_json());
        return container;
    }
    
    return nullptr;
}

} // namespace container_grpc
```

---

## 2. Service Implementation Example

### 2.1 Complete gRPC Service

```cpp
// grpc/server/container_service_impl.cpp
#include "container_service_impl.h"
#include "grpc/adapters/container_adapter.h"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace container_grpc {

class container_service_impl final 
    : public ContainerService::Service {
private:
    // Optional: Cache for performance
    std::unordered_map<std::string, 
        std::shared_ptr<container_module::value_container>> cache_;
    std::mutex cache_mutex_;
    
public:
    // Simple unary RPC
    grpc::Status Process(
        grpc::ServerContext* context,
        const GrpcContainer* request,
        GrpcContainer* response) override {
        
        try {
            // Convert from gRPC to container
            auto container = container_adapter::from_grpc(*request);
            
            // Process the container (example logic)
            process_business_logic(container);
            
            // Convert back to gRPC
            *response = container_adapter::to_grpc(*container);
            
            return grpc::Status::OK;
            
        } catch (const std::exception& e) {
            return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
        }
    }
    
    // Server streaming RPC
    grpc::Status StreamContainers(
        grpc::ServerContext* context,
        const FilterRequest* request,
        grpc::ServerWriter<GrpcContainer>* writer) override {
        
        // Example: Stream cached containers matching filter
        std::lock_guard<std::mutex> lock(cache_mutex_);
        
        for (const auto& [id, container] : cache_) {
            if (matches_filter(container, request)) {
                GrpcContainer grpc = container_adapter::to_grpc(*container);
                writer->Write(grpc);
            }
        }
        
        return grpc::Status::OK;
    }
    
    // Bidirectional streaming RPC
    grpc::Status ProcessStream(
        grpc::ServerContext* context,
        grpc::ServerReaderWriter<GrpcContainer, GrpcContainer>* stream) 
        override {
        
        GrpcContainer request;
        while (stream->Read(&request)) {
            // Convert and process
            auto container = container_adapter::from_grpc(request);
            process_business_logic(container);
            
            // Send response
            GrpcContainer response = container_adapter::to_grpc(*container);
            stream->Write(response);
        }
        
        return grpc::Status::OK;
    }
    
private:
    void process_business_logic(
        std::shared_ptr<container_module::value_container> container) {
        
        // Example processing using only public APIs
        auto msg_type = container->message_type();
        
        if (msg_type == "echo") {
            // Echo: swap source and target
            container->swap_header();
            
        } else if (msg_type == "transform") {
            // Transform: add timestamp
            auto timestamp = std::make_shared<container_module::string_value>(
                "timestamp", std::to_string(std::time(nullptr)));
            container->add(timestamp);
            
        } else if (msg_type == "store") {
            // Store in cache
            std::lock_guard<std::mutex> lock(cache_mutex_);
            cache_[container->source_id()] = container;
        }
        
        // Add processing timestamp
        container->set_value("processed", 
            std::make_shared<container_module::bool_value>("processed", true));
    }
    
    bool matches_filter(
        const std::shared_ptr<container_module::value_container>& container,
        const FilterRequest* filter) {
        
        if (filter->has_message_type() && 
            container->message_type() != filter->message_type()) {
            return false;
        }
        
        if (filter->has_source_id() && 
            container->source_id() != filter->source_id()) {
            return false;
        }
        
        return true;
    }
};

} // namespace container_grpc
```

### 2.2 Server Runner

```cpp
// grpc/server/grpc_server.cpp
#include "grpc_server.h"
#include "container_service_impl.cpp"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

namespace container_grpc {

class grpc_server {
private:
    std::unique_ptr<grpc::Server> server_;
    std::string address_;
    
public:
    explicit grpc_server(const std::string& address) 
        : address_(address) {}
    
    void start() {
        container_service_impl service;
        
        grpc::ServerBuilder builder;
        builder.AddListeningPort(address_, 
            grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        
        server_ = builder.BuildAndStart();
        std::cout << "gRPC server listening on " << address_ << std::endl;
    }
    
    void wait() {
        if (server_) {
            server_->Wait();
        }
    }
    
    void shutdown() {
        if (server_) {
            server_->Shutdown();
        }
    }
};

} // namespace container_grpc
```

---

## 3. Client Implementation

```cpp
// grpc/client/grpc_client.cpp
#include "grpc_client.h"
#include "grpc/adapters/container_adapter.h"
#include <grpcpp/grpcpp.h>

namespace container_grpc {

class grpc_client {
private:
    std::unique_ptr<ContainerService::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel_;
    
public:
    explicit grpc_client(const std::string& address) {
        channel_ = grpc::CreateChannel(address, 
            grpc::InsecureChannelCredentials());
        stub_ = ContainerService::NewStub(channel_);
    }
    
    // Send single container
    std::shared_ptr<container_module::value_container> 
    process(const std::shared_ptr<container_module::value_container>& container) {
        
        // Convert to gRPC
        GrpcContainer request = container_adapter::to_grpc(*container);
        
        // Make RPC call
        GrpcContainer response;
        grpc::ClientContext context;
        
        grpc::Status status = stub_->Process(&context, request, &response);
        
        if (status.ok()) {
            // Convert response back
            return container_adapter::from_grpc(response);
        } else {
            std::cerr << "RPC failed: " << status.error_message() << std::endl;
            return nullptr;
        }
    }
    
    // Stream containers
    void process_stream(
        const std::vector<std::shared_ptr<container_module::value_container>>& 
            containers,
        std::function<void(std::shared_ptr<container_module::value_container>)> 
            callback) {
        
        grpc::ClientContext context;
        auto stream = stub_->ProcessStream(&context);
        
        // Writer thread
        std::thread writer([&]() {
            for (const auto& container : containers) {
                GrpcContainer request = container_adapter::to_grpc(*container);
                stream->Write(request);
            }
            stream->WritesDone();
        });
        
        // Reader thread
        GrpcContainer response;
        while (stream->Read(&response)) {
            auto container = container_adapter::from_grpc(response);
            callback(container);
        }
        
        writer.join();
        stream->Finish();
    }
};

} // namespace container_grpc
```

---

## 4. Usage Examples

### 4.1 Server Usage

```cpp
// grpc/examples/server_main.cpp
#include "grpc/server/grpc_server.h"

int main(int argc, char** argv) {
    // Start gRPC server
    container_grpc::grpc_server server("0.0.0.0:50051");
    server.start();
    
    // Server runs independently of container system
    server.wait();
    
    return 0;
}
```

### 4.2 Client Usage

```cpp
// grpc/examples/client_main.cpp
#include "grpc/client/grpc_client.h"
#include "container/core/container.h"

int main(int argc, char** argv) {
    // Create container using existing API
    auto container = std::make_shared<container_module::value_container>();
    container->set_source("client", "01");
    container->set_target("server", "main");
    container->set_message_type("echo");
    
    // Add some values
    container->add(std::make_shared<container_module::string_value>(
        "message", "Hello gRPC!"));
    container->add(std::make_shared<container_module::numeric_value<int>>(
        "count", 42));
    
    // Send via gRPC
    container_grpc::grpc_client client("localhost:50051");
    auto response = client.process(container);
    
    if (response) {
        std::cout << "Response type: " << response->message_type() << std::endl;
        std::cout << "Response from: " << response->source_id() << std::endl;
        
        // Access values using existing API
        auto msg = response->get_value("message");
        if (msg) {
            std::cout << "Message: " << msg->to_string() << std::endl;
        }
    }
    
    return 0;
}
```

---

## 5. Build Instructions

```bash
# From container_system root directory

# Step 1: Create gRPC directory structure
mkdir -p grpc/{proto,adapters,server,client,examples,tests}

# Step 2: Add proto file
# Copy the proto definition to grpc/proto/container_service.proto

# Step 3: Build only gRPC extension (no changes to main build)
mkdir build_grpc
cd build_grpc
cmake ../grpc -DCMAKE_PREFIX_PATH=/path/to/vcpkg
make -j4

# Step 4: Run examples
./grpc_server_example &
./grpc_client_example

# Original container system remains unchanged
cd ../build
./bin/container_test  # Still works exactly as before
```

---

## 6. Testing Without Modification

```cpp
// grpc/tests/integration_test.cpp
#include <gtest/gtest.h>
#include "grpc/adapters/container_adapter.h"
#include "container/core/container.h"

class GrpcIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No modifications to container system needed
    }
};

TEST_F(GrpcIntegrationTest, RoundTripConversion) {
    // Create container with existing API
    auto original = std::make_shared<container_module::value_container>();
    original->set_message_type("test");
    original->add(std::make_shared<container_module::string_value>(
        "key", "value"));
    
    // Convert to gRPC and back
    auto grpc = container_grpc::container_adapter::to_grpc(*original);
    auto restored = container_grpc::container_adapter::from_grpc(grpc);
    
    // Verify
    EXPECT_EQ(original->message_type(), restored->message_type());
    EXPECT_EQ(original->serialize(), restored->serialize());
}

TEST_F(GrpcIntegrationTest, PerformanceBenchmark) {
    auto container = std::make_shared<container_module::value_container>();
    // Add 1000 values
    for (int i = 0; i < 1000; ++i) {
        container->add(std::make_shared<container_module::numeric_value<int>>(
            "val_" + std::to_string(i), i));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Benchmark conversion
    for (int i = 0; i < 100; ++i) {
        auto grpc = container_grpc::container_adapter::to_grpc(*container);
        auto restored = container_grpc::container_adapter::from_grpc(grpc);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start);
    
    std::cout << "100 round-trips: " << duration.count() << "ms" << std::endl;
    EXPECT_LT(duration.count(), 1000);  // Should be under 1 second
}
```

---

**이 구현은 기존 Container System 코드를 전혀 수정하지 않고 gRPC 기능을 추가합니다.**