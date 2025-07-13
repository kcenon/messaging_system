"""
Connection handler for managing client-server connections.
"""

import threading
from typing import Dict, Optional, Callable, Any
from ..container import Container, Value


class ConnectionHandler:
    """Handles connection management and message routing."""
    
    def __init__(self):
        """Initialize the connection handler."""
        self.connections: Dict[str, Any] = {}
        self.message_handlers: Dict[str, Callable[[str, Container], None]] = {}
        self.lock = threading.Lock()
    
    def register_handler(self, message_type: str, 
                        handler: Callable[[str, Container], None]) -> None:
        """
        Register a message handler for a specific message type.
        
        Args:
            message_type: The message type to handle
            handler: Callback function (client_id, message) -> None
        """
        with self.lock:
            self.message_handlers[message_type] = handler
    
    def unregister_handler(self, message_type: str) -> None:
        """
        Unregister a message handler.
        
        Args:
            message_type: The message type to unregister
        """
        with self.lock:
            self.message_handlers.pop(message_type, None)
    
    def handle_message(self, client_id: str, message: Container) -> bool:
        """
        Route a message to the appropriate handler.
        
        Args:
            client_id: ID of the client that sent the message
            message: The message container
            
        Returns:
            True if a handler was found and called, False otherwise
        """
        handler = None
        with self.lock:
            handler = self.message_handlers.get(message.message_type)
        
        if handler:
            try:
                handler(client_id, message)
                return True
            except Exception as e:
                import logging
                logging.error(f"Handler error for {message.message_type}: {e}")
        
        return False
    
    def add_connection(self, client_id: str, connection_data: Any) -> None:
        """
        Add a connection to the handler.
        
        Args:
            client_id: Client identifier
            connection_data: Any data associated with the connection
        """
        with self.lock:
            self.connections[client_id] = connection_data
    
    def remove_connection(self, client_id: str) -> Optional[Any]:
        """
        Remove a connection from the handler.
        
        Args:
            client_id: Client identifier
            
        Returns:
            The connection data if found, None otherwise
        """
        with self.lock:
            return self.connections.pop(client_id, None)
    
    def get_connection(self, client_id: str) -> Optional[Any]:
        """
        Get connection data for a client.
        
        Args:
            client_id: Client identifier
            
        Returns:
            The connection data if found, None otherwise
        """
        with self.lock:
            return self.connections.get(client_id)
    
    def get_all_connections(self) -> Dict[str, Any]:
        """
        Get all active connections.
        
        Returns:
            Dictionary of all connections
        """
        with self.lock:
            return self.connections.copy()
    
    def create_response(self, original_message: Container, 
                       response_type: str, 
                       values: Optional[list] = None) -> Container:
        """
        Create a response message based on an original message.
        
        Args:
            original_message: The original message to respond to
            response_type: Type of the response message
            values: Optional list of values for the response
            
        Returns:
            A new Container configured as a response
        """
        response = Container()
        response.create(
            target_id=original_message.source_id,
            target_sub_id=original_message.source_sub_id,
            source_id=original_message.target_id,
            source_sub_id=original_message.target_sub_id,
            message_type=response_type,
            values=values or []
        )
        return response