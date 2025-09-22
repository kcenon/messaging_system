/**
 * @file distributed_tracing_example.cpp
 * @brief Example demonstrating distributed tracing across services
 * 
 * This example shows how to:
 * - Create and manage trace spans
 * - Propagate trace context between services
 * - Add tags and baggage to spans
 * - Export traces to external systems
 */

#include <iostream>
#include <thread>
#include <future>
#include <map>

#include "monitoring/tracing/distributed_tracer.h"
#include "monitoring/context/thread_context.h"

using namespace monitoring_system;
using namespace std::chrono_literals;

// Simulate a web service that processes requests
class WebService {
private:
    distributed_tracer& tracer_;
    std::string service_name_;
    
public:
    WebService(distributed_tracer& tracer, const std::string& name)
        : tracer_(tracer), service_name_(name) {}
    
    // Simulate handling an HTTP request
    void handle_request(const std::string& request_id, 
                       const std::map<std::string, std::string>& headers) {
        std::cout << "[" << service_name_ << "] Processing request: " << request_id << std::endl;
        
        // Extract trace context from incoming headers
        auto context_result = tracer_.extract_context_from_carrier(headers);
        
        std::shared_ptr<trace_span> span;
        
        if (context_result) {
            // Continue existing trace
            auto span_result = tracer_.start_span_from_context(
                context_result.value(),
                service_name_ + "_handler"
            );
            if (span_result) {
                span = span_result.value();
                std::cout << "[" << service_name_ << "] Continuing trace: " 
                         << context_result.value().trace_id << std::endl;
            }
        } else {
            // Start new trace
            auto span_result = tracer_.start_span(
                service_name_ + "_handler",
                service_name_
            );
            if (span_result) {
                span = span_result.value();
                std::cout << "[" << service_name_ << "] Started new trace: " 
                         << span->trace_id << std::endl;
            }
        }
        
        if (span) {
            // Add tags to the span
            span->tags["service"] = service_name_;
            span->tags["request_id"] = request_id;
            span->tags["http.method"] = "GET";
            span->tags["http.url"] = "/api/process";
            span->tags["user.id"] = "user123";
            
            // Simulate processing
            process_business_logic(span);
            
            // Simulate calling downstream service
            call_downstream_service(span);
            
            // Mark span as successful
            span->status = trace_span::status_code::ok;
            
            // Finish the span
            tracer_.finish_span(span);
            
            std::cout << "[" << service_name_ << "] Span completed: " 
                     << span->span_id << std::endl;
        }
    }
    
private:
    void process_business_logic(std::shared_ptr<trace_span> parent_span) {
        // Create a child span for business logic
        auto span_result = tracer_.start_child_span(parent_span, "business_logic");
        if (!span_result) return;
        
        auto span = span_result.value();
        span->tags["operation"] = "data_processing";
        
        std::cout << "[" << service_name_ << "] Processing business logic..." << std::endl;
        
        // Simulate some work
        std::this_thread::sleep_for(50ms);
        
        // Simulate database query
        query_database(span);
        
        tracer_.finish_span(span);
    }
    
    void query_database(std::shared_ptr<trace_span> parent_span) {
        // Create a child span for database query
        auto span_result = tracer_.start_child_span(parent_span, "database_query");
        if (!span_result) return;
        
        auto span = span_result.value();
        span->tags["db.type"] = "postgresql";
        span->tags["db.statement"] = "SELECT * FROM users WHERE id = ?";
        
        std::cout << "[" << service_name_ << "] Querying database..." << std::endl;
        
        // Simulate database query
        std::this_thread::sleep_for(20ms);
        
        tracer_.finish_span(span);
    }
    
    void call_downstream_service(std::shared_ptr<trace_span> parent_span) {
        // Create a child span for downstream call
        auto span_result = tracer_.start_child_span(parent_span, "downstream_call");
        if (!span_result) return;
        
        auto span = span_result.value();
        span->tags["peer.service"] = "downstream_service";
        span->tags["span.kind"] = "client";
        
        // Extract context to propagate to downstream service
        auto context = tracer_.get_context_from_span(span);
        
        // Prepare headers for downstream call
        std::map<std::string, std::string> headers;
        tracer_.inject_context_into_carrier(context, headers);
        
        std::cout << "[" << service_name_ << "] Calling downstream service..." << std::endl;
        std::cout << "  Propagating trace: " << context.trace_id << std::endl;
        
        // Simulate HTTP call with headers
        std::this_thread::sleep_for(30ms);
        
        tracer_.finish_span(span);
    }
};

// Simulate distributed system with multiple services
void simulate_distributed_system() {
    distributed_tracer tracer;
    
    // Create services
    WebService frontend(tracer, "frontend");
    WebService backend(tracer, "backend");
    WebService database_service(tracer, "database_service");
    
    std::cout << "\n=== Simulating distributed request flow ===" << std::endl;
    
    // Simulate incoming request to frontend
    std::map<std::string, std::string> initial_headers;
    initial_headers["user-agent"] = "example-client";
    
    // Process request through frontend
    frontend.handle_request("req-001", initial_headers);
    
    std::cout << "\n=== Simulating request with existing trace ===" << std::endl;
    
    // Simulate request with existing trace context (e.g., from another service)
    std::map<std::string, std::string> traced_headers;
    traced_headers["traceparent"] = "00-4bf92f3577b34da6a3ce929d0e0e4736-00f067aa0ba902b7-01";
    traced_headers["baggage-user-id"] = "user456";
    
    backend.handle_request("req-002", traced_headers);
}

// Demonstrate trace analysis
void analyze_traces(distributed_tracer& tracer) {
    std::cout << "\n=== Analyzing traces ===" << std::endl;
    
    // Create a sample trace for analysis
    auto root_span_result = tracer.start_span("analyze_operation", "analyzer");
    if (!root_span_result) return;
    
    auto root_span = root_span_result.value();
    root_span->tags["analysis.type"] = "performance";
    
    // Create multiple child spans to simulate complex operation
    std::vector<std::shared_ptr<trace_span>> child_spans;
    
    for (int i = 0; i < 5; ++i) {
        auto child_result = tracer.start_child_span(root_span, 
            "sub_operation_" + std::to_string(i));
        
        if (child_result) {
            auto child = child_result.value();
            child->tags["index"] = std::to_string(i);
            child->tags["complexity"] = (i % 2 == 0) ? "low" : "high";
            
            // Simulate varying durations
            std::this_thread::sleep_for(std::chrono::milliseconds(10 * (i + 1)));
            
            child_spans.push_back(child);
        }
    }
    
    // Finish all child spans
    for (auto& child : child_spans) {
        tracer.finish_span(child);
    }
    
    // Simulate an error in one operation
    auto error_span_result = tracer.start_child_span(root_span, "failing_operation");
    if (error_span_result) {
        auto error_span = error_span_result.value();
        error_span->status = trace_span::status_code::error;
        error_span->status_message = "Database connection timeout";
        error_span->tags["error"] = "true";
        error_span->tags["error.type"] = "timeout";
        
        tracer.finish_span(error_span);
    }
    
    tracer.finish_span(root_span);
    
    // Get all spans for the trace
    auto trace_result = tracer.get_trace(root_span->trace_id);
    if (trace_result) {
        auto& spans = trace_result.value();
        
        std::cout << "Trace ID: " << root_span->trace_id << std::endl;
        std::cout << "Total spans in trace: " << spans.size() << std::endl;
        
        // Calculate total duration
        if (!spans.empty()) {
            auto min_time = spans[0]->start_time;
            auto max_time = spans[0]->end_time;
            
            for (const auto& span : spans) {
                if (span->start_time < min_time) min_time = span->start_time;
                if (span->end_time > max_time) max_time = span->end_time;
            }
            
            auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                max_time - min_time
            );
            
            std::cout << "Total trace duration: " << total_duration.count() << " ms" << std::endl;
        }
        
        // Count errors
        int error_count = 0;
        for (const auto& span : spans) {
            if (span->status == trace_span::status_code::error) {
                error_count++;
                std::cout << "Error in span: " << span->operation_name 
                         << " - " << span->status_message << std::endl;
            }
        }
        
        std::cout << "Spans with errors: " << error_count << std::endl;
    }
}

int main() {
    std::cout << "=== Distributed Tracing Example ===" << std::endl;
    
    try {
        // Part 1: Basic span creation and management
        std::cout << "\n--- Part 1: Basic Span Management ---" << std::endl;
        
        distributed_tracer tracer;
        
        // Create a root span
        auto root_span_result = tracer.start_span("main_operation", "example_service");
        if (!root_span_result) {
            std::cerr << "Failed to create root span" << std::endl;
            return 1;
        }
        
        auto root_span = root_span_result.value();
        
        std::cout << "Created root span:" << std::endl;
        std::cout << "  Trace ID: " << root_span->trace_id << std::endl;
        std::cout << "  Span ID: " << root_span->span_id << std::endl;
        std::cout << "  Operation: " << root_span->operation_name << std::endl;
        
        // Add tags
        root_span->tags["version"] = "1.0.0";
        root_span->tags["environment"] = "development";
        
        // Add baggage (propagated to child spans)
        root_span->baggage["user.id"] = "user789";
        root_span->baggage["session.id"] = "sess123";
        
        // Create child spans
        std::cout << "\nCreating child spans..." << std::endl;
        
        auto child1_result = tracer.start_child_span(root_span, "child_operation_1");
        auto child2_result = tracer.start_child_span(root_span, "child_operation_2");
        
        if (child1_result && child2_result) {
            auto child1 = child1_result.value();
            auto child2 = child2_result.value();
            
            std::cout << "  Child 1 span ID: " << child1->span_id << std::endl;
            std::cout << "  Child 2 span ID: " << child2->span_id << std::endl;
            
            // Simulate some work
            std::this_thread::sleep_for(10ms);
            
            // Finish child spans
            tracer.finish_span(child1);
            tracer.finish_span(child2);
        }
        
        // Finish root span
        tracer.finish_span(root_span);
        
        std::cout << "All spans finished" << std::endl;
        
        // Part 2: Distributed system simulation
        std::cout << "\n--- Part 2: Distributed System Simulation ---" << std::endl;
        simulate_distributed_system();
        
        // Part 3: Trace analysis
        std::cout << "\n--- Part 3: Trace Analysis ---" << std::endl;
        analyze_traces(tracer);
        
        // Part 4: Context propagation demonstration
        std::cout << "\n--- Part 4: Context Propagation ---" << std::endl;
        
        auto demo_span = tracer.start_span("propagation_demo");
        if (demo_span) {
            
            // Extract context for propagation
            auto context = tracer.get_context_from_span(demo_span);
            
            // Simulate HTTP headers
            std::map<std::string, std::string> http_headers;
            tracer.inject_context_into_carrier(context, http_headers);
            
            std::cout << "Context injected into headers:" << std::endl;
            for (const auto& [key, value] : http_headers) {
                std::cout << "  " << key << ": " << value << std::endl;
            }
            
            // Simulate receiving headers in another service
            auto extracted_context = tracer.extract_context_from_carrier(http_headers);
            if (extracted_context) {
                std::cout << "\nContext extracted from headers:" << std::endl;
                std::cout << "  Trace ID: " << extracted_context.value().trace_id << std::endl;
                std::cout << "  Span ID: " << extracted_context.value().span_id << std::endl;
                
                // Continue trace in new service
                auto continued_span = tracer.start_span_from_context(
                    extracted_context.value(),
                    "continued_operation"
                );
                
                if (continued_span) {
                    std::cout << "  Continued span ID: " 
                             << continued_span.value()->span_id << std::endl;
                    tracer.finish_span(continued_span.value());
                }
            }
            
            tracer.finish_span(demo_span);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Example completed successfully ===" << std::endl;
    
    return 0;
}