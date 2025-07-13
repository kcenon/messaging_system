# Messaging System Python Module

Python implementation of the messaging system for network communication with type-safe containers.

## Features

- **Type-safe containers**: Structured data serialization with strong typing
- **Network communication**: TCP/IP client-server architecture
- **Asynchronous messaging**: Non-blocking message handling
- **Nested containers**: Support for hierarchical data structures
- **Special character handling**: Proper encoding/decoding of control characters

## Installation

### From source
```bash
cd python
pip install -e .
```

### For development
```bash
cd python
pip install -e ".[dev]"
```

## Quick Start

### Container Usage

```python
from messaging_system import Container, Value

# Create a container
container = Container()
container.create(
    target_id="server",
    source_id="client",
    message_type="greeting",
    values=[
        Value("message", "f", "Hello, World!"),
        Value("timestamp", "9", str(int(time.time())))
    ]
)

# Serialize
serialized = container.serialize()

# Parse
parsed = Container(serialized)
print(parsed.get_value("message"))  # "Hello, World!"
```

### Client Usage

```python
from messaging_system import MessagingClient, Container, Value

# Create client
client = MessagingClient(
    source_id="python_client",
    connection_key="my_key",
    recv_callback=handle_message
)

# Connect
if client.start("localhost", 8080):
    # Send message
    client.send_message("server", "test", [
        Value("data", "f", "Test message")
    ])
```

## Examples

### Running the Simple Client
```bash
python -m messaging_system.samples.simple_client localhost 8080 my_client_id
```

### Running the Container Demo
```bash
python -m messaging_system.samples.container_demo
```

## Value Types

The messaging system supports the following value types:

- `NULL_VALUE` ('0'): Null/empty value
- `BOOL_VALUE` ('1'): Boolean true/false
- `CHAR_VALUE` ('2'): Single character
- `INT8_VALUE` ('3'): 8-bit signed integer
- `UINT8_VALUE` ('4'): 8-bit unsigned integer
- `INT16_VALUE` ('5'): 16-bit signed integer
- `UINT16_VALUE` ('6'): 16-bit unsigned integer
- `INT32_VALUE` ('7'): 32-bit signed integer
- `UINT32_VALUE` ('8'): 32-bit unsigned integer
- `INT64_VALUE` ('9'): 64-bit signed integer
- `UINT64_VALUE` ('a'): 64-bit unsigned integer
- `FLOAT_VALUE` ('b'): 32-bit floating point
- `DOUBLE_VALUE` ('c'): 64-bit floating point
- `BYTES_VALUE` ('d'): Raw byte array
- `CONTAINER_VALUE` ('e'): Nested container
- `STRING_VALUE` ('f'): UTF-8 string

## Development

### Running Tests
```bash
pytest
```

### Code Formatting
```bash
black messaging_system
```

### Type Checking
```bash
mypy messaging_system
```

## License

BSD 3-Clause License - see LICENSE file for details.