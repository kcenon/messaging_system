"""
Container class for message serialization and deserialization.
"""

import re
from typing import Dict, List, Optional, Union
from .value import Value
from .value_types import ValueType


class Container:
    """A container for storing and serializing messages."""
    
    def __init__(self, message: str = ''):
        """
        Initialize a Container instance.
        
        Args:
            message: Optional serialized message to parse
        """
        self.headers: Dict[str, str] = {
            '1': '',  # target_id
            '2': '',  # target_sub_id
            '3': '',  # source_id
            '4': '',  # source_sub_id
            '5': '',  # message_type
            '6': '1.0.0.0'  # version
        }
        self.values: List[Value] = []
        self._data_string: str = ''
        self._deserialized: bool = False
        
        if message:
            self.parse(message)
    
    def create(self, target_id: str = '', target_sub_id: str = '', 
               source_id: str = '', source_sub_id: str = '', 
               message_type: str = '', values: Optional[List[Value]] = None) -> None:
        """
        Create a new container with the given parameters.
        
        Args:
            target_id: Target identifier
            target_sub_id: Target sub-identifier
            source_id: Source identifier
            source_sub_id: Source sub-identifier
            message_type: Type of the message
            values: List of values to include
        """
        self.headers['1'] = target_id
        self.headers['2'] = target_sub_id
        self.headers['3'] = source_id
        self.headers['4'] = source_sub_id
        self.headers['5'] = message_type
        self.values = values or []
        self._deserialized = True
    
    def parse(self, message: str, parse_data: bool = True) -> None:
        """
        Parse a serialized message.
        
        Args:
            message: The serialized message string
            parse_data: Whether to parse the data section immediately
        """
        # Remove newlines
        message = re.sub(r'\r\n?|\n', '', message)
        
        # Parse header
        header_match = re.search(r'@header=[\s?]*\{[\s?]*(.*?)[\s?]*\};', message)
        if header_match:
            self._parse_header(header_match.group())
        
        # Parse data
        data_match = re.search(r'@data=[\s?]*\{[\s?]*(.*?)[\s?]*\};', message)
        if data_match:
            self._parse_data(data_match.group(), parse_data)
    
    def append(self, value: Value) -> None:
        """
        Append a value to the container.
        
        Args:
            value: The value to append
        """
        if value.value_type != ValueType.NULL_VALUE:
            self._deserialized = True
            value.parent = None
            self.values.append(value)
    
    def get(self, name: str) -> List[Value]:
        """
        Get values by name.
        
        Args:
            name: The name to search for
            
        Returns:
            List of matching values
        """
        if not self._deserialized and self._data_string:
            self._parse_data(self._data_string, True)
        
        if not name:
            return self.values
        
        result = [v for v in self.values if v.name == name]
        
        # Return null value if nothing found
        if not result:
            result.append(Value(name, ValueType.NULL_VALUE.value, ''))
        
        return result
    
    def get_value(self, name: str, default: any = None) -> any:
        """
        Get a single value by name with automatic type conversion.
        
        Args:
            name: The name to search for
            default: Default value if not found
            
        Returns:
            The typed value or default
        """
        values = self.get(name)
        if values and values[0].value_type != ValueType.NULL_VALUE:
            return values[0].get_value(default)
        return default
    
    def serialize(self) -> str:
        """
        Serialize the container to string format.
        
        Returns:
            Serialized string representation
        """
        if self._deserialized:
            self._data_string = self._make_data_string()
            self._deserialized = False
        
        header = f"@header={{{self._serialize_headers()}}};"
        return f"{header}{self._data_string}"
    
    # Property accessors
    @property
    def target_id(self) -> str:
        return self.headers['1']
    
    @target_id.setter
    def target_id(self, value: str) -> None:
        self.headers['1'] = value
    
    @property
    def target_sub_id(self) -> str:
        return self.headers['2']
    
    @target_sub_id.setter
    def target_sub_id(self, value: str) -> None:
        self.headers['2'] = value
    
    @property
    def source_id(self) -> str:
        return self.headers['3']
    
    @source_id.setter
    def source_id(self, value: str) -> None:
        self.headers['3'] = value
    
    @property
    def source_sub_id(self) -> str:
        return self.headers['4']
    
    @source_sub_id.setter
    def source_sub_id(self, value: str) -> None:
        self.headers['4'] = value
    
    @property
    def message_type(self) -> str:
        return self.headers['5']
    
    @message_type.setter
    def message_type(self, value: str) -> None:
        self.headers['5'] = value
    
    @property
    def version(self) -> str:
        return self.headers['6']
    
    def _parse_header(self, header_string: str) -> None:
        """Parse the header section of a message."""
        results = re.findall(r'\[(\w+),(.*?)\];', header_string)
        for type_string, data_string in results:
            if type_string in self.headers:
                self.headers[type_string] = data_string
    
    def _parse_data(self, data_string: str, parsing: bool) -> None:
        """Parse the data section of a message."""
        self._data_string = data_string
        self._deserialized = parsing
        
        if not parsing:
            return
        
        # Parse all values
        value_list = []
        results = re.findall(r'\[(\w+),[\s?]*(\w+),[\s?]*(.*?)\];', data_string)
        for name, type_str, value_str in results:
            value_list.append(Value(name, type_str, value_str))
        
        # Build hierarchy
        previous_value = None
        for current_value in value_list:
            if current_value.value_type == ValueType.NULL_VALUE:
                continue
            
            if not previous_value:
                self.append(current_value)
                if current_value.value_type == ValueType.CONTAINER_VALUE:
                    previous_value = current_value
                continue
            
            previous_value.append(current_value)
            if current_value.value_type == ValueType.CONTAINER_VALUE:
                previous_value = current_value
                continue
            
            # Check if container is complete
            if str(len(previous_value.children)) == previous_value.value_string:
                previous_value = previous_value.parent
    
    def _serialize_headers(self) -> str:
        """Serialize the headers."""
        parts = []
        for key in sorted(self.headers.keys()):
            parts.append(f"[{key},{self.headers[key]}];")
        return ''.join(parts)
    
    def _make_data_string(self) -> str:
        """Create the data string from values."""
        result = "@data={"
        for value in self.values:
            result += value.serialize()
        result += "};"
        return result
    
    def __repr__(self) -> str:
        """String representation of the container."""
        return (f"Container(target='{self.target_id}', source='{self.source_id}', "
                f"type='{self.message_type}', values={len(self.values)})")
    
    def __str__(self) -> str:
        """String representation for printing."""
        return self.serialize()