"""
Messaging client implementation for network communication.
"""

import socket
import threading
import logging
from typing import Optional, Callable, Tuple
from ..container import Container, Value


class MessagingClient:
    """A client for messaging system network communication."""
    
    def __init__(self, source_id: str, connection_key: str, 
                 start_number: int = 231, end_number: int = 67,
                 conn_callback: Optional[Callable[[str, str, bool], None]] = None,
                 recv_callback: Optional[Callable[[Container], None]] = None):
        """
        Initialize a MessagingClient instance.
        
        Args:
            source_id: Unique identifier for this client
            connection_key: Key for connection authentication
            start_number: Start byte for packet framing (default: 231)
            end_number: End byte for packet framing (default: 67)
            conn_callback: Callback for connection status changes
            recv_callback: Callback for received messages
        """
        self.source_id = source_id
        self.source_sub_id = ''
        self.connection_key = connection_key
        self.start_code = bytes([start_number] * 4)
        self.end_code = bytes([end_number] * 4)
        self.sock: Optional[socket.socket] = None
        self.conn_callback = conn_callback
        self.recv_callback = recv_callback
        self.recv_thread: Optional[threading.Thread] = None
        self._running = False
        
        # Configure logging
        self.logger = logging.getLogger(f"MessagingClient.{source_id}")
    
    def start(self, server_ip: str, server_port: int, 
              auto_echo: bool = False, 
              auto_echo_interval_seconds: int = 1) -> bool:
        """
        Start the client and connect to the server.
        
        Args:
            server_ip: Server IP address
            server_port: Server port number
            auto_echo: Enable automatic echo messages
            auto_echo_interval_seconds: Interval for auto echo
            
        Returns:
            True if connection successful, False otherwise
        """
        try:
            # Create socket and connect
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((server_ip, server_port))
            
            # Start receive thread
            self._running = True
            self.recv_thread = threading.Thread(target=self._receive_loop)
            self.recv_thread.daemon = True
            self.recv_thread.start()
            
            # Send connection request
            self._send_connection_request(auto_echo, auto_echo_interval_seconds)
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to connect: {e}")
            return False
    
    def stop(self) -> None:
        """Stop the client and close the connection."""
        self._running = False
        
        if self.sock:
            try:
                self.sock.shutdown(socket.SHUT_RDWR)
            except:
                pass
            self.sock.close()
            self.sock = None
        
        if self.recv_thread and self.recv_thread.is_alive():
            self.recv_thread.join(timeout=5.0)
    
    def send_packet(self, packet: Container) -> bool:
        """
        Send a packet to the server.
        
        Args:
            packet: The container to send
            
        Returns:
            True if sent successfully, False otherwise
        """
        if not self.sock:
            self.logger.error("Cannot send packet: not connected")
            return False
        
        if not packet.target_id:
            self.logger.error("Cannot send packet: no target_id specified")
            return False
        
        # Set source information if not present
        if not packet.source_id:
            packet.source_id = self.source_id
            packet.source_sub_id = self.source_sub_id
        
        try:
            # Serialize packet
            data_string = packet.serialize()
            data_bytes = data_string.encode('utf-8')
            data_length = len(data_bytes).to_bytes(4, byteorder='little')
            
            # Send packet with framing
            self.sock.send(self.start_code)
            self.sock.send(bytes([2]))  # Type code
            self.sock.send(data_length)
            self.sock.send(data_bytes)
            self.sock.send(self.end_code)
            
            self.logger.debug(f"[SENT] => {data_string}")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to send packet: {e}")
            return False
    
    def send_message(self, target_id: str, message_type: str, 
                     values: Optional[list] = None) -> bool:
        """
        Convenience method to send a message.
        
        Args:
            target_id: Target identifier
            message_type: Type of message
            values: Optional list of values
            
        Returns:
            True if sent successfully, False otherwise
        """
        container = Container()
        container.create(
            target_id=target_id,
            message_type=message_type,
            values=values or []
        )
        return self.send_packet(container)
    
    def _receive_loop(self) -> None:
        """Main receive loop running in separate thread."""
        self.logger.info(f"Starting messaging client: {self.source_id}")
        
        while self._running and self.sock:
            try:
                message = self._receive_packet()
                if message:
                    self._handle_message(message)
            except Exception as e:
                if self._running:
                    self.logger.error(f"Receive error: {e}")
                break
        
        self.logger.info(f"Stopping messaging client: {self.source_id}")
    
    def _receive_packet(self) -> Optional[Container]:
        """Receive a single packet from the socket."""
        if not self.sock:
            return None
        
        try:
            # Read start code
            for i in range(4):
                byte = self.sock.recv(1)
                if not byte or byte != self.start_code[i:i+1]:
                    return None
            
            # Read type code
            type_code = self.sock.recv(1)
            if not type_code or type_code != bytes([2]):
                return None
            
            # Read length
            length_bytes = self.sock.recv(4)
            if len(length_bytes) != 4:
                return None
            
            length = int.from_bytes(length_bytes, 'little')
            
            # Read data
            data = b''
            while len(data) < length:
                chunk = self.sock.recv(min(4096, length - len(data)))
                if not chunk:
                    return None
                data += chunk
            
            # Read end code
            for i in range(4):
                byte = self.sock.recv(1)
                if not byte or byte != self.end_code[i:i+1]:
                    return None
            
            # Parse packet
            packet_string = data.decode('utf-8')
            self.logger.debug(f"[RECEIVED] => {packet_string}")
            
            return Container(packet_string)
            
        except Exception as e:
            self.logger.error(f"Error receiving packet: {e}")
            return None
    
    def _handle_message(self, message: Container) -> None:
        """Handle a received message."""
        if message.message_type == "confirm_connection":
            self._handle_connection_confirmation(message)
        elif self.recv_callback:
            self.recv_callback(message)
    
    def _handle_connection_confirmation(self, message: Container) -> None:
        """Handle connection confirmation message."""
        confirm_values = message.get('confirm')
        
        if not confirm_values:
            self.logger.error(f"Invalid confirm message from {message.source_id}")
            if self.conn_callback:
                self.conn_callback(message.source_id, message.source_sub_id, False)
            return
        
        # Update our IDs from server assignment
        self.source_id = message.target_id
        self.source_sub_id = message.target_sub_id
        
        confirm_value = confirm_values[0].value_string
        self.logger.info(f"Connection confirmed from {message.source_id}: {confirm_value}")
        
        if self.conn_callback:
            self.conn_callback(message.source_id, message.source_sub_id, True)
    
    def _send_connection_request(self, auto_echo: bool, 
                                 auto_echo_interval_seconds: int) -> None:
        """Send initial connection request to server."""
        values = [
            Value('connection_key', 'd', self.connection_key),
            Value('auto_echo', '1', str(auto_echo).lower()),
            Value('auto_echo_interval_seconds', '3', str(auto_echo_interval_seconds)),
            Value('session_type', '2', '1'),
            Value('bridge_mode', '1', 'false'),
            Value('snipping_targets', 'e', '0')
        ]
        
        self.send_message('server', 'request_connection', values)