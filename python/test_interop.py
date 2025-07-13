#!/usr/bin/env python3
"""
Test interoperability between Python client and C++ server/client
"""

import sys
import socket
import time
import struct
sys.path.insert(0, '.')

from messaging_system.container import Container, Value

def test_raw_socket():
    """Test raw socket communication to understand the protocol."""
    print("=== Raw Socket Test ===")
    
    # Create socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # Connect to server
        sock.connect(("127.0.0.1", 8080))
        print("Connected to server")
        
        # Create connection request
        container = Container()
        container.create(
            source_id="python_test_client",
            source_sub_id="test_sub",
            message_type="request_connection",
            values=[
                Value("connection_key", "f", "test_key"),
                Value("auto_echo", "1", "false")
            ]
        )
        
        # Serialize
        data = container.serialize().encode('utf-8')
        
        # Send with framing
        packet = bytearray()
        packet.extend([231, 231, 231, 231])  # Start code
        packet.append(2)  # Type code
        packet.extend(struct.pack('<I', len(data)))  # Length (little-endian)
        packet.extend(data)  # Data
        packet.extend([67, 67, 67, 67])  # End code
        
        sock.sendall(packet)
        print(f"Sent connection request: {len(packet)} bytes")
        
        # Try to receive response
        sock.settimeout(5.0)
        
        # Read start code
        start = sock.recv(4)
        if start == bytes([231, 231, 231, 231]):
            print("Received start code")
            
            # Read type
            type_code = sock.recv(1)
            print(f"Type code: {type_code[0]}")
            
            # Read length
            length_bytes = sock.recv(4)
            length = struct.unpack('<I', length_bytes)[0]
            print(f"Data length: {length}")
            
            # Read data
            data = b''
            while len(data) < length:
                chunk = sock.recv(min(4096, length - len(data)))
                if not chunk:
                    break
                data += chunk
            
            print(f"Received data: {data.decode('utf-8', errors='ignore')}")
            
            # Read end code
            end = sock.recv(4)
            if end == bytes([67, 67, 67, 67]):
                print("Received end code")
                
                # Parse response
                try:
                    response = Container(data.decode('utf-8'))
                    print(f"Response type: {response.message_type}")
                    print(f"Response from: {response.source_id}")
                except Exception as e:
                    print(f"Failed to parse response: {e}")
        else:
            print(f"Invalid start code: {start}")
            
    except socket.timeout:
        print("Timeout waiting for response")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()
        print("Socket closed")

def test_python_client():
    """Test using Python messaging client."""
    print("\n=== Python Client Test ===")
    
    from messaging_system.network import MessagingClient
    
    messages_received = []
    
    def recv_callback(msg):
        messages_received.append(msg)
        print(f"Received: {msg.message_type}")
        for value in msg.values:
            print(f"  {value.name}: {value.value_string}")
    
    # Create client
    client = MessagingClient(
        source_id="python_interop_test",
        connection_key="test_key",
        recv_callback=recv_callback
    )
    
    # Connect
    print("Connecting to server...")
    if not client.start("127.0.0.1", 8080):
        print("Failed to connect")
        return
    
    print("Connected successfully")
    time.sleep(1)
    
    # Send test message
    test_msg = Container()
    test_msg.create(
        target_id="server",
        message_type="test_message",
        values=[
            Value("greeting", "f", "Hello from Python!"),
            Value("timestamp", "9", str(int(time.time())))
        ]
    )
    
    print("\nSending test message...")
    if client.send_packet(test_msg):
        print("Message sent")
    else:
        print("Failed to send message")
    
    # Wait for response
    time.sleep(2)
    
    # Disconnect
    client.stop()
    print(f"\nTotal messages received: {len(messages_received)}")

def main():
    print("Python-C++ Interoperability Test")
    print("================================\n")
    
    if len(sys.argv) > 1 and sys.argv[1] == "raw":
        test_raw_socket()
    else:
        test_python_client()

if __name__ == "__main__":
    main()