"""
Simple client example for the messaging system.
"""

import sys
import time
import logging
from ..container import Container, Value
from ..network import MessagingClient


def connection_callback(server_id: str, server_sub_id: str, connected: bool):
    """Handle connection status changes."""
    if connected:
        print(f"✅ Connected to server: {server_id}/{server_sub_id}")
    else:
        print(f"❌ Connection failed to server: {server_id}")


def message_callback(message: Container):
    """Handle received messages."""
    print(f"📨 Received message:")
    print(f"   From: {message.source_id}/{message.source_sub_id}")
    print(f"   Type: {message.message_type}")
    print(f"   Values: {len(message.values)}")
    
    # Print values
    for value in message.values:
        print(f"   - {value.name}: {value.value_string} (type: {value.value_type.name})")


def main():
    """Run the simple client example."""
    # Configure logging
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    
    # Parse command line arguments
    if len(sys.argv) < 3:
        print("Usage: python -m messaging_system.samples.simple_client <server_ip> <server_port> [client_id]")
        sys.exit(1)
    
    server_ip = sys.argv[1]
    server_port = int(sys.argv[2])
    client_id = sys.argv[3] if len(sys.argv) > 3 else "python_client"
    
    # Create client
    client = MessagingClient(
        source_id=client_id,
        connection_key="sample_key",
        conn_callback=connection_callback,
        recv_callback=message_callback
    )
    
    # Connect to server
    print(f"🔌 Connecting to {server_ip}:{server_port}...")
    if not client.start(server_ip, server_port, auto_echo=True):
        print("❌ Failed to connect to server")
        sys.exit(1)
    
    try:
        # Wait for connection confirmation
        time.sleep(1)
        
        # Send some test messages
        print("\n📤 Sending test messages...")
        
        # Send a simple message
        values = [
            Value("greeting", "f", "Hello from Python!"),
            Value("timestamp", "9", str(int(time.time()))),
            Value("version", "f", "2.0.0")
        ]
        client.send_message("server", "test_message", values)
        
        # Send a container with nested values
        nested_container = Value("config", "e", "2")
        nested_container.append(Value("debug", "1", "true"))
        nested_container.append(Value("timeout", "7", "30"))
        
        client.send_message("server", "config_update", [nested_container])
        
        # Keep running
        print("\n⌛ Client running. Press Ctrl+C to stop...")
        while True:
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\n🛑 Stopping client...")
    finally:
        client.stop()
        print("👋 Client stopped")


if __name__ == "__main__":
    main()