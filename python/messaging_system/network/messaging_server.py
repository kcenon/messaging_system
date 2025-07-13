"""
Messaging server implementation for network communication.
"""

import socket
import threading
import logging
import time
import uuid
from typing import Dict, Optional, Callable, Tuple, Set
from ..container import Container, Value


class ClientSession:
    """Represents a connected client session."""
    
    def __init__(self, session_id: str, socket: socket.socket, address: Tuple[str, int]):
        """Initialize a client session."""
        self.session_id = session_id
        self.socket = socket
        self.address = address
        self.client_id = ""
        self.client_sub_id = ""
        self.connection_key = ""
        self.auto_echo = False
        self.auto_echo_interval = 1
        self.last_echo_time = time.time()
        self.connected = True
        self.lock = threading.Lock()
    
    def send(self, data: bytes) -> bool:
        """Send data to the client."""
        try:
            with self.lock:
                if self.connected and self.socket:
                    self.socket.sendall(data)
                    return True
        except Exception:
            self.connected = False
        return False
    
    def close(self):
        """Close the client session."""
        self.connected = False
        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
            except:
                pass
            self.socket.close()
            self.socket = None


class MessagingServer:
    """A server for messaging system network communication."""
    
    def __init__(self, server_id: str = "server",
                 start_number: int = 231, end_number: int = 67,
                 conn_callback: Optional[Callable[[str, str, bool], None]] = None,
                 recv_callback: Optional[Callable[[str, Container], None]] = None,
                 disc_callback: Optional[Callable[[str], None]] = None):
        """
        Initialize a MessagingServer instance.
        
        Args:
            server_id: Unique identifier for this server
            start_number: Start byte for packet framing (default: 231)
            end_number: End byte for packet framing (default: 67)
            conn_callback: Callback for new client connections
            recv_callback: Callback for received messages
            disc_callback: Callback for client disconnections
        """
        self.server_id = server_id
        self.start_code = bytes([start_number] * 4)
        self.end_code = bytes([end_number] * 4)
        self.conn_callback = conn_callback
        self.recv_callback = recv_callback
        self.disc_callback = disc_callback
        
        self.server_socket: Optional[socket.socket] = None
        self.clients: Dict[str, ClientSession] = {}
        self.client_threads: Dict[str, threading.Thread] = {}
        self._running = False
        self._accept_thread: Optional[threading.Thread] = None
        self._echo_thread: Optional[threading.Thread] = None
        self.lock = threading.Lock()
        
        # Configure logging
        self.logger = logging.getLogger(f"MessagingServer.{server_id}")
    
    def start(self, bind_ip: str = "0.0.0.0", bind_port: int = 8080, 
              max_connections: int = 100) -> bool:
        """
        Start the server and begin listening for connections.
        
        Args:
            bind_ip: IP address to bind to
            bind_port: Port number to bind to
            max_connections: Maximum number of connections
            
        Returns:
            True if server started successfully, False otherwise
        """
        try:
            # Create server socket
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((bind_ip, bind_port))
            self.server_socket.listen(max_connections)
            
            # Start accept thread
            self._running = True
            self._accept_thread = threading.Thread(target=self._accept_loop)
            self._accept_thread.daemon = True
            self._accept_thread.start()
            
            # Start echo thread
            self._echo_thread = threading.Thread(target=self._echo_loop)
            self._echo_thread.daemon = True
            self._echo_thread.start()
            
            self.logger.info(f"Server started on {bind_ip}:{bind_port}")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to start server: {e}")
            return False
    
    def stop(self) -> None:
        """Stop the server and close all connections."""
        self._running = False
        
        # Close server socket
        if self.server_socket:
            self.server_socket.close()
            self.server_socket = None
        
        # Close all client connections
        with self.lock:
            for session in list(self.clients.values()):
                self._disconnect_client(session.session_id)
        
        # Wait for threads
        if self._accept_thread and self._accept_thread.is_alive():
            self._accept_thread.join(timeout=5.0)
        if self._echo_thread and self._echo_thread.is_alive():
            self._echo_thread.join(timeout=5.0)
        
        self.logger.info("Server stopped")
    
    def send_to_client(self, client_id: str, packet: Container) -> bool:
        """
        Send a packet to a specific client.
        
        Args:
            client_id: Target client ID
            packet: The container to send
            
        Returns:
            True if sent successfully, False otherwise
        """
        session = self._find_session_by_client_id(client_id)
        if not session:
            self.logger.warning(f"Client not found: {client_id}")
            return False
        
        return self._send_packet(session, packet)
    
    def broadcast(self, packet: Container, exclude: Optional[Set[str]] = None) -> int:
        """
        Broadcast a packet to all connected clients.
        
        Args:
            packet: The container to broadcast
            exclude: Set of client IDs to exclude
            
        Returns:
            Number of clients that received the message
        """
        exclude = exclude or set()
        count = 0
        
        with self.lock:
            for session in list(self.clients.values()):
                if session.client_id not in exclude:
                    if self._send_packet(session, packet):
                        count += 1
        
        return count
    
    def get_connected_clients(self) -> Dict[str, Dict[str, str]]:
        """
        Get information about all connected clients.
        
        Returns:
            Dictionary mapping client_id to client info
        """
        with self.lock:
            return {
                session.client_id: {
                    "session_id": session.session_id,
                    "address": f"{session.address[0]}:{session.address[1]}",
                    "client_sub_id": session.client_sub_id,
                    "auto_echo": str(session.auto_echo)
                }
                for session in self.clients.values()
                if session.client_id
            }
    
    def _accept_loop(self) -> None:
        """Accept incoming connections."""
        self.logger.info("Starting accept loop")
        
        while self._running and self.server_socket:
            try:
                client_socket, client_address = self.server_socket.accept()
                self._handle_new_connection(client_socket, client_address)
            except Exception as e:
                if self._running:
                    self.logger.error(f"Accept error: {e}")
    
    def _handle_new_connection(self, client_socket: socket.socket, 
                              client_address: Tuple[str, int]) -> None:
        """Handle a new client connection."""
        session_id = str(uuid.uuid4())
        session = ClientSession(session_id, client_socket, client_address)
        
        with self.lock:
            self.clients[session_id] = session
        
        # Start client thread
        thread = threading.Thread(target=self._client_loop, args=(session,))
        thread.daemon = True
        thread.start()
        
        with self.lock:
            self.client_threads[session_id] = thread
        
        self.logger.info(f"New connection from {client_address} (session: {session_id})")
    
    def _client_loop(self, session: ClientSession) -> None:
        """Handle communication with a single client."""
        try:
            while self._running and session.connected:
                message = self._receive_packet(session)
                if message:
                    self._handle_message(session, message)
                else:
                    break
        except Exception as e:
            self.logger.error(f"Client loop error for {session.session_id}: {e}")
        finally:
            self._disconnect_client(session.session_id)
    
    def _receive_packet(self, session: ClientSession) -> Optional[Container]:
        """Receive a packet from a client session."""
        try:
            # Read start code
            for i in range(4):
                byte = session.socket.recv(1)
                if not byte or byte != self.start_code[i:i+1]:
                    return None
            
            # Read type code
            type_code = session.socket.recv(1)
            if not type_code or type_code != bytes([2]):
                return None
            
            # Read length
            length_bytes = session.socket.recv(4)
            if len(length_bytes) != 4:
                return None
            
            length = int.from_bytes(length_bytes, 'little')
            
            # Read data
            data = b''
            while len(data) < length:
                chunk = session.socket.recv(min(4096, length - len(data)))
                if not chunk:
                    return None
                data += chunk
            
            # Read end code
            for i in range(4):
                byte = session.socket.recv(1)
                if not byte or byte != self.end_code[i:i+1]:
                    return None
            
            # Parse packet
            packet_string = data.decode('utf-8')
            self.logger.debug(f"[RECEIVED from {session.session_id}] => {packet_string}")
            
            return Container(packet_string)
            
        except Exception as e:
            if session.connected:
                self.logger.error(f"Error receiving from {session.session_id}: {e}")
            return None
    
    def _send_packet(self, session: ClientSession, packet: Container) -> bool:
        """Send a packet to a client session."""
        if not packet.source_id:
            packet.source_id = self.server_id
            packet.source_sub_id = ""
        
        try:
            # Serialize packet
            data_string = packet.serialize()
            data_bytes = data_string.encode('utf-8')
            data_length = len(data_bytes).to_bytes(4, byteorder='little')
            
            # Build packet with framing
            packet_data = (
                self.start_code +
                bytes([2]) +
                data_length +
                data_bytes +
                self.end_code
            )
            
            # Send to client
            if session.send(packet_data):
                self.logger.debug(f"[SENT to {session.session_id}] => {data_string}")
                return True
            
        except Exception as e:
            self.logger.error(f"Failed to send to {session.session_id}: {e}")
        
        return False
    
    def _handle_message(self, session: ClientSession, message: Container) -> None:
        """Handle a received message from a client."""
        if message.message_type == "request_connection":
            self._handle_connection_request(session, message)
        elif self.recv_callback:
            self.recv_callback(session.client_id or session.session_id, message)
    
    def _handle_connection_request(self, session: ClientSession, 
                                  message: Container) -> None:
        """Handle a connection request from a client."""
        # Extract connection parameters
        session.client_id = message.source_id
        session.client_sub_id = message.source_sub_id or str(uuid.uuid4())[:8]
        
        # Get connection key
        key_values = message.get("connection_key")
        if key_values:
            session.connection_key = key_values[0].value_string
        
        # Get auto echo settings
        auto_echo_values = message.get("auto_echo")
        if auto_echo_values:
            session.auto_echo = auto_echo_values[0].value_string.lower() == "true"
        
        interval_values = message.get("auto_echo_interval_seconds")
        if interval_values:
            try:
                session.auto_echo_interval = int(interval_values[0].value_string)
            except:
                pass
        
        # Send confirmation
        confirm_packet = Container()
        confirm_packet.create(
            target_id=session.client_id,
            target_sub_id=session.client_sub_id,
            source_id=self.server_id,
            message_type="confirm_connection",
            values=[
                Value("confirm", "1", "true"),
                Value("session_id", "f", session.session_id),
                Value("server_version", "f", "2.0.0")
            ]
        )
        
        self._send_packet(session, confirm_packet)
        
        self.logger.info(f"Client connected: {session.client_id}/{session.client_sub_id}")
        
        if self.conn_callback:
            self.conn_callback(session.client_id, session.client_sub_id, True)
    
    def _echo_loop(self) -> None:
        """Send echo messages to clients that have auto-echo enabled."""
        while self._running:
            current_time = time.time()
            
            with self.lock:
                for session in list(self.clients.values()):
                    if (session.auto_echo and session.client_id and
                        current_time - session.last_echo_time >= session.auto_echo_interval):
                        
                        echo_packet = Container()
                        echo_packet.create(
                            target_id=session.client_id,
                            target_sub_id=session.client_sub_id,
                            message_type="echo",
                            values=[
                                Value("timestamp", "9", str(int(current_time))),
                                Value("echo_count", "8", "1")
                            ]
                        )
                        
                        if self._send_packet(session, echo_packet):
                            session.last_echo_time = current_time
            
            time.sleep(0.1)  # Check every 100ms
    
    def _disconnect_client(self, session_id: str) -> None:
        """Disconnect a client session."""
        with self.lock:
            session = self.clients.pop(session_id, None)
            thread = self.client_threads.pop(session_id, None)
        
        if session:
            client_id = session.client_id or session_session_id
            session.close()
            
            self.logger.info(f"Client disconnected: {client_id}")
            
            if self.disc_callback:
                self.disc_callback(client_id)
    
    def _find_session_by_client_id(self, client_id: str) -> Optional[ClientSession]:
        """Find a session by client ID."""
        with self.lock:
            for session in self.clients.values():
                if session.client_id == client_id:
                    return session
        return None