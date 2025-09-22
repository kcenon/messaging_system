# Logger Integration Template

This directory provides a template for integrating external logger implementations with the thread_system.

## Integration Steps

1. **Implement the Interface**
   ```cpp
   #include <thread_system/interfaces/logger_interface.h>
   
   class MyLogger : public thread_module::logger_interface {
   public:
       void log(log_level level, const std::string& message) override {
           // Your implementation
       }
       
       void log(log_level level, const std::string& message,
                const std::string& file, int line,
                const std::string& function) override {
           // Your implementation with source location
       }
       
       bool is_enabled(log_level level) const override {
           // Return whether logging is enabled for this level
       }
       
       void flush() override {
           // Flush any buffered messages
       }
   };
   ```

2. **Register with Thread System**
   ```cpp
   #include <thread_system/interfaces/logger_interface.h>
   
   // Create your logger instance
   auto logger = std::make_shared<MyLogger>();
   
   // Register globally
   thread_module::logger_registry::set_logger(logger);
   
   // Or pass via thread_context
   thread_module::thread_context context;
   context.set_logger(logger);
   ```

3. **CMake Integration**
   ```cmake
   find_package(ThreadSystemCore REQUIRED)
   
   add_library(my_logger_integration
       src/my_logger.cpp
   )
   
   target_link_libraries(my_logger_integration
       PUBLIC
           ThreadSystem::Core
           MyLoggerLibrary
   )
   ```

## Example: spdlog Integration

```cpp
#include <thread_system/interfaces/logger_interface.h>
#include <spdlog/spdlog.h>

class SpdlogAdapter : public thread_module::logger_interface {
private:
    std::shared_ptr<spdlog::logger> logger_;
    
public:
    explicit SpdlogAdapter(std::shared_ptr<spdlog::logger> logger)
        : logger_(std::move(logger)) {}
    
    void log(log_level level, const std::string& message) override {
        switch(level) {
            case log_level::critical:
                logger_->critical(message);
                break;
            case log_level::error:
                logger_->error(message);
                break;
            case log_level::warning:
                logger_->warn(message);
                break;
            case log_level::info:
                logger_->info(message);
                break;
            case log_level::debug:
                logger_->debug(message);
                break;
            case log_level::trace:
                logger_->trace(message);
                break;
        }
    }
    
    void log(log_level level, const std::string& message,
             const std::string& file, int line,
             const std::string& function) override {
        log(level, fmt::format("[{}:{}:{}] {}", file, line, function, message));
    }
    
    bool is_enabled(log_level level) const override {
        return logger_->should_log(convert_level(level));
    }
    
    void flush() override {
        logger_->flush();
    }
    
private:
    spdlog::level::level_enum convert_level(log_level level) const {
        switch(level) {
            case log_level::critical: return spdlog::level::critical;
            case log_level::error: return spdlog::level::err;
            case log_level::warning: return spdlog::level::warn;
            case log_level::info: return spdlog::level::info;
            case log_level::debug: return spdlog::level::debug;
            case log_level::trace: return spdlog::level::trace;
        }
        return spdlog::level::info;
    }
};
```

## Best Practices

1. **Thread Safety**: Ensure your logger implementation is thread-safe
2. **Performance**: Minimize overhead in the `is_enabled()` check
3. **Buffering**: Consider async logging for high-throughput scenarios
4. **Error Handling**: Don't throw exceptions from logger methods
5. **Resource Management**: Properly handle file handles and network connections