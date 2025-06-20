"""
Container usage demonstration.
"""

from ..container import Container, Value, ValueType


def print_value(value: Value, indent: int = 0):
    """Pretty print a value with indentation."""
    prefix = "  " * indent
    print(f"{prefix}├─ {value.name}: {value.value_string} (type: {value.value_type.name})")
    
    # Print children
    for child in value.children:
        print_value(child, indent + 1)


def main():
    """Demonstrate container functionality."""
    print("📦 Container Module Demo\n")
    
    # Create a simple container
    print("1️⃣ Creating a simple container:")
    container = Container()
    container.create(
        target_id="server",
        source_id="client_001",
        message_type="user_data",
        values=[
            Value("username", ValueType.STRING_VALUE.value, "john_doe"),
            Value("age", ValueType.INT32_VALUE.value, "25"),
            Value("active", ValueType.BOOL_VALUE.value, "true"),
            Value("balance", ValueType.DOUBLE_VALUE.value, "1234.56")
        ]
    )
    
    print(f"   Target: {container.target_id}")
    print(f"   Source: {container.source_id}")
    print(f"   Type: {container.message_type}")
    print("   Values:")
    for value in container.values:
        print_value(value, 1)
    
    # Serialize and deserialize
    print("\n2️⃣ Serialization test:")
    serialized = container.serialize()
    print(f"   Serialized length: {len(serialized)} bytes")
    print(f"   First 100 chars: {serialized[:100]}...")
    
    # Parse back
    container2 = Container(serialized)
    print(f"   Parsed successfully: {container2.message_type == container.message_type}")
    
    # Nested containers
    print("\n3️⃣ Nested container example:")
    nested_container = Container()
    
    # Create a user profile with nested data
    profile = Value("profile", ValueType.CONTAINER_VALUE.value, "3")
    profile.append(Value("name", ValueType.STRING_VALUE.value, "Alice"))
    profile.append(Value("email", ValueType.STRING_VALUE.value, "alice@example.com"))
    
    # Settings container
    settings = Value("settings", ValueType.CONTAINER_VALUE.value, "2")
    settings.append(Value("theme", ValueType.STRING_VALUE.value, "dark"))
    settings.append(Value("notifications", ValueType.BOOL_VALUE.value, "true"))
    profile.append(settings)
    
    nested_container.create(
        message_type="user_profile",
        values=[profile]
    )
    
    print("   Structure:")
    for value in nested_container.values:
        print_value(value, 1)
    
    # Type-safe value access
    print("\n4️⃣ Type-safe value access:")
    test_container = Container()
    test_container.create(
        message_type="test_data",
        values=[
            Value("count", ValueType.INT32_VALUE.value, "42"),
            Value("pi", ValueType.DOUBLE_VALUE.value, "3.14159"),
            Value("enabled", ValueType.BOOL_VALUE.value, "true"),
            Value("name", ValueType.STRING_VALUE.value, "Test"),
        ]
    )
    
    # Access with automatic type conversion
    count = test_container.get_value("count", 0)
    pi = test_container.get_value("pi", 0.0)
    enabled = test_container.get_value("enabled", False)
    name = test_container.get_value("name", "")
    missing = test_container.get_value("missing", "default")
    
    print(f"   count: {count} (type: {type(count).__name__})")
    print(f"   pi: {pi} (type: {type(pi).__name__})")
    print(f"   enabled: {enabled} (type: {type(enabled).__name__})")
    print(f"   name: {name} (type: {type(name).__name__})")
    print(f"   missing: {missing} (type: {type(missing).__name__})")
    
    # Special character handling
    print("\n5️⃣ Special character handling:")
    special_container = Container()
    special_container.create(
        message_type="special_chars",
        values=[
            Value("multiline", ValueType.STRING_VALUE.value, "Line 1\nLine 2\nLine 3"),
            Value("tabs", ValueType.STRING_VALUE.value, "Col1\tCol2\tCol3"),
            Value("mixed", ValueType.STRING_VALUE.value, "Hello\n\tWorld!\r\n"),
        ]
    )
    
    # Serialize and parse back
    special_serialized = special_container.serialize()
    special_parsed = Container(special_serialized)
    
    for value in special_parsed.values:
        print(f"   {value.name}:")
        print(f"     Original: {repr(value.value_string)}")
        print(f"     Preserved: {value.value_string == special_container.get(value.name)[0].value_string}")
    
    print("\n✅ Container demo completed!")


if __name__ == "__main__":
    main()