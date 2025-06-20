"""
Value types enumeration for the container system.
"""

from enum import Enum


class ValueType(Enum):
    """Enumeration of supported value types."""
    
    NULL_VALUE = '0'
    BOOL_VALUE = '1'
    CHAR_VALUE = '2'
    INT8_VALUE = '3'
    UINT8_VALUE = '4'
    INT16_VALUE = '5'
    UINT16_VALUE = '6'
    INT32_VALUE = '7'
    UINT32_VALUE = '8'
    INT64_VALUE = '9'
    UINT64_VALUE = 'a'
    FLOAT_VALUE = 'b'
    DOUBLE_VALUE = 'c'
    BYTES_VALUE = 'd'
    CONTAINER_VALUE = 'e'
    STRING_VALUE = 'f'
    
    @classmethod
    def from_string(cls, type_string: str) -> 'ValueType':
        """Convert string representation to ValueType."""
        for value_type in cls:
            if value_type.value == type_string:
                return value_type
        return cls.NULL_VALUE
    
    def is_numeric(self) -> bool:
        """Check if the value type is numeric."""
        numeric_types = {
            self.CHAR_VALUE, self.INT8_VALUE, self.UINT8_VALUE,
            self.INT16_VALUE, self.UINT16_VALUE, self.INT32_VALUE,
            self.UINT32_VALUE, self.INT64_VALUE, self.UINT64_VALUE,
            self.FLOAT_VALUE, self.DOUBLE_VALUE
        }
        return self in numeric_types
    
    def is_container(self) -> bool:
        """Check if the value type is a container."""
        return self == self.CONTAINER_VALUE