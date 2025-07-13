"""
Value class for storing typed data in the container system.
"""

from typing import List, Optional, Any
from .value_types import ValueType


class Value:
    """A typed value that can be stored in a container."""
    
    def __init__(self, name: str, value_type: str, value: str, children: Optional[List['Value']] = None):
        """
        Initialize a Value instance.
        
        Args:
            name: The name of the value
            value_type: The type string (from ValueType enum)
            value: The string representation of the value
            children: Optional list of child values for container types
        """
        self.name = name
        self.type_string = value_type
        self.value_type = ValueType.from_string(value_type)
        self.value_string = self._decode_value(value)
        self.children: List[Value] = children or []
        self.parent: Optional[Value] = None
    
    def _encode_value(self, value: str) -> str:
        """Encode special characters in the value string."""
        return (value.replace("\r", "</0x0A;>")
                     .replace("\n", "</0x0B;>")
                     .replace(" ", "</0x0C;>")
                     .replace("\t", "</0x0D;>"))
    
    def _decode_value(self, value: str) -> str:
        """Decode special characters in the value string."""
        return (value.replace("</0x0A;>", "\r")
                     .replace("</0x0B;>", "\n")
                     .replace("</0x0C;>", " ")
                     .replace("</0x0D;>", "\t"))
    
    def append(self, child: 'Value') -> None:
        """
        Append a child value to this value.
        
        Args:
            child: The child value to append
        """
        child.parent = self
        self.children.append(child)
    
    def remove(self, name: str) -> None:
        """
        Remove all child values with the given name.
        
        Args:
            name: The name of values to remove
        """
        self.children = [child for child in self.children if child.name != name]
    
    def get(self, name: Optional[str] = None) -> List['Value']:
        """
        Get child values by name.
        
        Args:
            name: The name to search for. If None, returns all children.
            
        Returns:
            List of matching values
        """
        if not name:
            return self.children
        
        return [child for child in self.children if child.name == name]
    
    def get_value(self, default: Any = None) -> Any:
        """
        Get the typed value with automatic conversion.
        
        Args:
            default: Default value if conversion fails
            
        Returns:
            The value converted to the appropriate type
        """
        try:
            if self.value_type == ValueType.NULL_VALUE:
                return None
            elif self.value_type == ValueType.BOOL_VALUE:
                return self.value_string.lower() == 'true'
            elif self.value_type.is_numeric():
                if self.value_type in (ValueType.FLOAT_VALUE, ValueType.DOUBLE_VALUE):
                    return float(self.value_string)
                else:
                    return int(self.value_string)
            else:
                return self.value_string
        except (ValueError, AttributeError):
            return default
    
    def serialize(self) -> str:
        """
        Serialize the value to string format.
        
        Returns:
            Serialized string representation
        """
        encoded_value = self._encode_value(self.value_string)
        result = f"[{self.name},{self.type_string},{encoded_value}];"
        
        # Add serialized children
        for child in self.children:
            result += child.serialize()
        
        return result
    
    def __repr__(self) -> str:
        """String representation of the value."""
        return f"Value(name='{self.name}', type={self.value_type.name}, value='{self.value_string}')"