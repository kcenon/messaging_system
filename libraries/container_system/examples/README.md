# Container System Examples

This directory contains example applications demonstrating the enhanced container system features.

## Examples

### messaging_integration_example

Demonstrates the new messaging integration features including:

- **Enhanced Container Creation**: Optimized containers for messaging workloads
- **Builder Pattern**: Fluent API for container construction
- **Performance Monitoring**: Real-time metrics collection (if enabled)
- **External Integration**: Callback system for external systems (if enabled)
- **Compatibility**: Seamless integration with messaging systems

## Building Examples

Examples are built automatically when `BUILD_CONTAINER_SAMPLES` is enabled:

```bash
mkdir build && cd build
cmake .. -DBUILD_CONTAINER_SAMPLES=ON
cmake --build .
```

## Running Examples

After building, examples are available in the `bin/examples` directory:

```bash
# Run the messaging integration example
./bin/examples/messaging_integration_example
```

## Configuration Options

Examples support various build-time configurations:

### Messaging Features
```bash
cmake .. -DENABLE_MESSAGING_FEATURES=ON
```
Enables enhanced container creation, builder pattern, and optimized serialization.

### Performance Metrics
```bash
cmake .. -DENABLE_PERFORMANCE_METRICS=ON
```
Enables real-time performance monitoring and metrics collection.

### External Integration
```bash
cmake .. -DENABLE_EXTERNAL_INTEGRATION=ON
```
Enables callback system for external system integration.

### Combined Configuration
```bash
cmake .. \
  -DENABLE_MESSAGING_FEATURES=ON \
  -DENABLE_PERFORMANCE_METRICS=ON \
  -DENABLE_EXTERNAL_INTEGRATION=ON
```

## Expected Output

The messaging integration example will show different features based on the build configuration:

- **Basic features**: Always available, shows standard container operations
- **Enhanced features**: Available when messaging features are enabled
- **Performance metrics**: Available when metrics are enabled
- **External callbacks**: Available when external integration is enabled

## Integration with Messaging Systems

These examples demonstrate how the container system can be used both:

1. **Standalone**: As an independent container library
2. **Integrated**: As part of a larger messaging system

The dual alias system (`ContainerSystem::container` and `MessagingSystem::container`) ensures compatibility with both usage patterns.