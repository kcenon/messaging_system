# Messaging System Samples

This directory contains production-ready sample applications demonstrating the messaging system's capabilities and best practices.

## Sample Applications

### 1. **basic_usage_example.cpp**
- Simple introduction to the messaging system
- Basic publish/subscribe patterns
- System health monitoring

### 2. **production_ready_example.cpp** ⭐ NEW
- **Production-ready features:**
  - Signal handling for graceful shutdown (SIGINT, SIGTERM)
  - Configuration file loading from `messaging_config.ini`
  - Automatic retry mechanisms with exponential backoff
  - Comprehensive metrics collection and reporting
  - Health monitoring with automatic recovery attempts
  - Error handling and logging best practices
- **Usage:**
  ```bash
  ./production_ready_example [config_file]
  ```

### 3. **chat_server.cpp** (Enhanced)
- Real-time chat server with resilience features
- **New enhancements:**
  - Automatic user reconnection handling
  - Message retry queue for failed deliveries
  - Health monitoring and system recovery
  - Graceful shutdown with user notifications
  - Connection state tracking and metrics
- **Usage:**
  ```bash
  ./chat_server [port]
  ```

### 4. **distributed_worker.cpp**
- Distributed task processing system
- Load balancing across workers
- Task queue management

### 5. **iot_monitoring.cpp**
- IoT device management and monitoring
- Telemetry data processing
- Alert threshold management

### 6. **microservices_orchestrator.cpp**
- Service discovery and registration
- Load balancing with circuit breakers
- Health checks and failover

### 7. **event_pipeline.cpp**
- Event-driven data processing
- Multi-stage pipeline transformations
- Error handling and recovery

### 8. **message_bus_benchmark.cpp**
- Performance benchmarking tool
- Throughput and latency measurements
- Scalability testing

## Building the Samples

```bash
mkdir build
cd build
cmake ..
make
```

## Configuration

The samples now support configuration file loading. See `messaging_config.ini` for an example configuration file with all available options.

### Key Configuration Options:

- **environment**: development, staging, or production
- **worker_threads**: Number of worker threads for message processing
- **queue_size**: Maximum message queue size
- **enable_compression**: Enable/disable message compression
- **max_retries**: Maximum retry attempts for failed operations
- **health_check_interval_sec**: Interval between health checks

## Production Features

All samples have been updated with production-ready features:

1. **Proper Logger Integration**: All samples use the logger library with correct include paths
2. **Error Recovery**: Automatic retry mechanisms for transient failures
3. **Health Monitoring**: Regular system health checks with recovery attempts
4. **Graceful Shutdown**: Signal handling for clean application termination
5. **Metrics Collection**: Comprehensive metrics for monitoring and debugging
6. **Configuration Management**: External configuration file support

## Best Practices Demonstrated

1. **Resource Management**: Proper RAII patterns and cleanup
2. **Concurrency**: Thread-safe operations with appropriate synchronization
3. **Error Handling**: Comprehensive exception handling and logging
4. **Performance**: Optimized message processing with batching
5. **Resilience**: Circuit breakers, retries, and fallback mechanisms

## Logging

All samples produce detailed logs in both console and rotating file formats:
- Console output for immediate feedback
- Rotating log files (10MB per file, 5 files maximum)
- Configurable log levels (debug, info, warning, error, critical)

## Monitoring

The production samples include built-in metrics reporting:
- Message throughput
- Error rates
- Retry statistics
- Connection metrics
- System health status

## Running in Production

For production deployment:

1. Create a configuration file based on `messaging_config.ini`
2. Set appropriate log levels and file paths
3. Configure worker threads based on system resources
4. Enable monitoring and metrics export
5. Set up proper signal handling for graceful shutdown
6. Implement health check endpoints for container orchestration

## Troubleshooting

Common issues and solutions:

1. **Port already in use**: Change the port in configuration or command line
2. **High memory usage**: Reduce queue_size and worker_threads
3. **Message loss**: Enable persistence and increase retry attempts
4. **Performance issues**: Enable compression and adjust batch sizes

## Contributing

When adding new samples:
1. Follow the established patterns for error handling and logging
2. Include production-ready features (signals, config, metrics)
3. Update this README with sample description
4. Add appropriate CMake configuration
5. Include comprehensive error recovery mechanisms