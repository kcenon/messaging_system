"""
Simple server example for the messaging system.
"""

import sys
import time
import logging
from ..container import Container, Value
from ..network import MessagingServer, ConnectionHandler


class SimpleServer:
    """A simple messaging server with basic functionality."""
    
    def __init__(self, server_id: str = "python_server"):
        """Initialize the simple server."""
        self.server = MessagingServer(
            server_id=server_id,
            conn_callback=self.on_client_connected,
            recv_callback=self.on_message_received,
            disc_callback=self.on_client_disconnected
        )
        
        self.handler = ConnectionHandler()
        self._setup_handlers()
        
        # Statistics
        self.message_count = 0
        self.start_time = time.time()
    
    def _setup_handlers(self):
        """Set up message handlers."""
        self.handler.register_handler("test_message", self.handle_test_message)
        self.handler.register_handler("config_update", self.handle_config_update)
        self.handler.register_handler("get_status", self.handle_get_status)
        self.handler.register_handler("broadcast", self.handle_broadcast)
    
    def on_client_connected(self, client_id: str, client_sub_id: str, success: bool):
        """Handle client connection events."""
        if success:
            print(f"✅ Client connected: {client_id}/{client_sub_id}")
            
            # Send welcome message
            welcome = Container()
            welcome.create(
                target_id=client_id,
                target_sub_id=client_sub_id,
                message_type="welcome",
                values=[
                    Value("message", "f", f"Welcome to {self.server.server_id}!"),
                    Value("server_time", "9", str(int(time.time()))),
                    Value("active_clients", "7", str(len(self.server.get_connected_clients())))
                ]
            )
            self.server.send_to_client(client_id, welcome)
        else:
            print(f"❌ Client connection failed: {client_id}")
    
    def on_client_disconnected(self, client_id: str):
        """Handle client disconnection events."""
        print(f"👋 Client disconnected: {client_id}")
        
        # Notify other clients
        notification = Container()
        notification.create(
            message_type="client_disconnected",
            values=[
                Value("client_id", "f", client_id),
                Value("timestamp", "9", str(int(time.time())))
            ]
        )
        self.server.broadcast(notification, exclude={client_id})
    
    def on_message_received(self, client_id: str, message: Container):
        """Handle received messages."""
        self.message_count += 1
        
        print(f"📨 Message from {client_id}:")
        print(f"   Type: {message.message_type}")
        print(f"   Values: {len(message.values)}")
        
        # Route to appropriate handler
        if not self.handler.handle_message(client_id, message):
            print(f"   ⚠️  No handler for message type: {message.message_type}")
            
            # Send error response
            error_response = self.handler.create_response(
                message, "error",
                [Value("error", "f", f"Unknown message type: {message.message_type}")]
            )
            self.server.send_to_client(client_id, error_response)
    
    def handle_test_message(self, client_id: str, message: Container):
        """Handle test messages."""
        greeting = message.get_value("greeting", "")
        timestamp = message.get_value("timestamp", 0)
        
        print(f"   Test message: {greeting} (at {timestamp})")
        
        # Send response
        response = self.handler.create_response(
            message, "test_response",
            [
                Value("echo", "f", greeting),
                Value("server_time", "9", str(int(time.time()))),
                Value("processing_time", "c", "0.001")
            ]
        )
        self.server.send_to_client(client_id, response)
    
    def handle_config_update(self, client_id: str, message: Container):
        """Handle configuration update messages."""
        config_values = message.get("config")
        if config_values and config_values[0].children:
            print("   Configuration update:")
            for value in config_values[0].children:
                print(f"     - {value.name}: {value.value_string}")
        
        # Send acknowledgment
        ack = self.handler.create_response(
            message, "config_acknowledged",
            [Value("status", "f", "Configuration updated successfully")]
        )
        self.server.send_to_client(client_id, ack)
    
    def handle_get_status(self, client_id: str, message: Container):
        """Handle status request messages."""
        uptime = int(time.time() - self.start_time)
        clients = self.server.get_connected_clients()
        
        # Create status response
        status = Container()
        status.create(
            target_id=message.source_id,
            target_sub_id=message.source_sub_id,
            message_type="server_status",
            values=[
                Value("server_id", "f", self.server.server_id),
                Value("uptime_seconds", "9", str(uptime)),
                Value("active_clients", "7", str(len(clients))),
                Value("total_messages", "9", str(self.message_count)),
                Value("messages_per_second", "c", f"{self.message_count / max(uptime, 1):.2f}")
            ]
        )
        
        # Add client list
        client_list = Value("clients", "e", str(len(clients)))
        for cid, info in clients.items():
            client_info = Value(cid, "e", "2")
            client_info.append(Value("address", "f", info["address"]))
            client_info.append(Value("sub_id", "f", info["client_sub_id"]))
            client_list.append(client_info)
        status.append(client_list)
        
        self.server.send_to_client(client_id, status)
    
    def handle_broadcast(self, client_id: str, message: Container):
        """Handle broadcast request messages."""
        broadcast_msg = message.get_value("message", "")
        include_sender = message.get_value("include_sender", True)
        
        print(f"   Broadcasting: {broadcast_msg}")
        
        # Create broadcast message
        broadcast = Container()
        broadcast.create(
            message_type="broadcast_message",
            values=[
                Value("from", "f", client_id),
                Value("message", "f", broadcast_msg),
                Value("timestamp", "9", str(int(time.time())))
            ]
        )
        
        # Send to all clients
        exclude = set() if include_sender else {client_id}
        count = self.server.broadcast(broadcast, exclude=exclude)
        
        # Send confirmation to sender
        confirm = self.handler.create_response(
            message, "broadcast_sent",
            [Value("recipients", "7", str(count))]
        )
        self.server.send_to_client(client_id, confirm)
    
    def run(self, bind_ip: str, bind_port: int):
        """Run the server."""
        print(f"🚀 Starting server on {bind_ip}:{bind_port}")
        
        if not self.server.start(bind_ip, bind_port):
            print("❌ Failed to start server")
            return
        
        print("✅ Server is running. Press Ctrl+C to stop.")
        print("\nAvailable message types:")
        print("  - test_message: Echo test message")
        print("  - config_update: Update configuration")
        print("  - get_status: Get server status")
        print("  - broadcast: Broadcast message to all clients")
        print()
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n🛑 Stopping server...")
        finally:
            self.server.stop()
            print("👋 Server stopped")


def main():
    """Run the simple server example."""
    # Configure logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    # Parse command line arguments
    if len(sys.argv) < 3:
        print("Usage: python -m messaging_system.samples.simple_server <bind_ip> <bind_port> [server_id]")
        print("Example: python -m messaging_system.samples.simple_server 0.0.0.0 8080")
        sys.exit(1)
    
    bind_ip = sys.argv[1]
    bind_port = int(sys.argv[2])
    server_id = sys.argv[3] if len(sys.argv) > 3 else "python_server"
    
    # Create and run server
    server = SimpleServer(server_id)
    server.run(bind_ip, bind_port)


if __name__ == "__main__":
    main()