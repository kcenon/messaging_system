"""
Unit tests for the container module.
"""

import pytest
from ..container import Container, Value, ValueType


class TestValue:
    """Test cases for the Value class."""
    
    def test_value_creation(self):
        """Test basic value creation."""
        value = Value("test", "f", "Hello")
        assert value.name == "test"
        assert value.type_string == "f"
        assert value.value_type == ValueType.STRING_VALUE
        assert value.value_string == "Hello"
        assert value.children == []
        assert value.parent is None
    
    def test_value_encoding_decoding(self):
        """Test special character encoding/decoding."""
        test_string = "Line1\nLine2\rTab\tSpace Test"
        value = Value("test", "f", test_string)
        
        # Value should be decoded on creation
        assert value.value_string == test_string
        
        # Serialization should encode
        serialized = value.serialize()
        assert "</0x0B;>" in serialized  # \n
        assert "</0x0A;>" in serialized  # \r
        assert "</0x0D;>" in serialized  # \t
        assert "</0x0C;>" in serialized  # space
    
    def test_value_children(self):
        """Test value with children."""
        parent = Value("parent", "e", "2")
        child1 = Value("child1", "f", "value1")
        child2 = Value("child2", "7", "42")
        
        parent.append(child1)
        parent.append(child2)
        
        assert len(parent.children) == 2
        assert child1.parent == parent
        assert child2.parent == parent
        assert parent.get("child1")[0] == child1
        assert parent.get("child2")[0] == child2
    
    def test_value_type_conversion(self):
        """Test automatic type conversion."""
        # Boolean
        bool_value = Value("test", "1", "true")
        assert bool_value.get_value() is True
        
        bool_value2 = Value("test", "1", "false")
        assert bool_value2.get_value() is False
        
        # Integer
        int_value = Value("test", "7", "42")
        assert int_value.get_value() == 42
        assert isinstance(int_value.get_value(), int)
        
        # Float
        float_value = Value("test", "c", "3.14159")
        assert float_value.get_value() == 3.14159
        assert isinstance(float_value.get_value(), float)
        
        # String
        str_value = Value("test", "f", "Hello")
        assert str_value.get_value() == "Hello"
        
        # Null
        null_value = Value("test", "0", "")
        assert null_value.get_value() is None


class TestContainer:
    """Test cases for the Container class."""
    
    def test_container_creation(self):
        """Test basic container creation."""
        container = Container()
        assert container.target_id == ""
        assert container.source_id == ""
        assert container.message_type == ""
        assert container.version == "1.0.0.0"
        assert container.values == []
    
    def test_container_create_method(self):
        """Test container creation with values."""
        values = [
            Value("key1", "f", "value1"),
            Value("key2", "7", "42")
        ]
        
        container = Container()
        container.create(
            target_id="target",
            source_id="source",
            message_type="test",
            values=values
        )
        
        assert container.target_id == "target"
        assert container.source_id == "source"
        assert container.message_type == "test"
        assert len(container.values) == 2
    
    def test_container_serialization(self):
        """Test container serialization and parsing."""
        # Create container
        container = Container()
        container.create(
            target_id="server",
            target_sub_id="sub1",
            source_id="client",
            source_sub_id="sub2",
            message_type="test_message",
            values=[
                Value("string", "f", "Hello World"),
                Value("number", "7", "123"),
                Value("boolean", "1", "true")
            ]
        )
        
        # Serialize
        serialized = container.serialize()
        assert "@header=" in serialized
        assert "@data=" in serialized
        assert "[1,server];" in serialized
        assert "[5,test_message];" in serialized
        
        # Parse back
        parsed = Container(serialized)
        assert parsed.target_id == container.target_id
        assert parsed.source_id == container.source_id
        assert parsed.message_type == container.message_type
        assert len(parsed.values) == 3
        
        # Check values
        assert parsed.get_value("string") == "Hello World"
        assert parsed.get_value("number") == 123
        assert parsed.get_value("boolean") is True
    
    def test_nested_containers(self):
        """Test nested container handling."""
        container = Container()
        
        # Create nested structure
        parent = Value("config", "e", "2")
        parent.append(Value("debug", "1", "true"))
        parent.append(Value("timeout", "7", "30"))
        
        container.create(
            message_type="config_update",
            values=[parent]
        )
        
        # Serialize and parse
        serialized = container.serialize()
        parsed = Container(serialized)
        
        # Check structure
        config_values = parsed.get("config")
        assert len(config_values) == 1
        assert config_values[0].value_type == ValueType.CONTAINER_VALUE
        assert len(config_values[0].children) == 2
        
        # Check nested values
        debug = config_values[0].get("debug")[0]
        assert debug.get_value() is True
        
        timeout = config_values[0].get("timeout")[0]
        assert timeout.get_value() == 30
    
    def test_property_accessors(self):
        """Test property getter/setter methods."""
        container = Container()
        
        # Test setters
        container.target_id = "new_target"
        container.target_sub_id = "sub_target"
        container.source_id = "new_source"
        container.source_sub_id = "sub_source"
        container.message_type = "new_type"
        
        # Test getters
        assert container.target_id == "new_target"
        assert container.target_sub_id == "sub_target"
        assert container.source_id == "new_source"
        assert container.source_sub_id == "sub_source"
        assert container.message_type == "new_type"
    
    def test_get_value_with_default(self):
        """Test get_value with default values."""
        container = Container()
        container.create(
            message_type="test",
            values=[
                Value("exists", "f", "value")
            ]
        )
        
        # Existing value
        assert container.get_value("exists") == "value"
        assert container.get_value("exists", "default") == "value"
        
        # Non-existing value
        assert container.get_value("missing") is None
        assert container.get_value("missing", "default") == "default"
        assert container.get_value("missing", 42) == 42


if __name__ == "__main__":
    pytest.main([__file__])