import sys
import socket

class messaging_client:
    
    start_code = []
    end_code = []
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    def __init__(self, connection_key, start_number, end_number):
        self.connection_key = connection_key
        self.start_code = bytes([start_number, start_number, start_number, start_number])
        self.end_code = bytes([end_number, end_number, end_number, end_number])
        
    def start(self, server_address):
        self.sock.connect(server_address)
        
    def stop(self):
        self.sock.close()
        
    def send_packet(self, message):
        data = bytes(message, 'utf-8')
        len_data = len(data).to_bytes(4, byteorder='little')
    
        self.sock.send(self.start_code)
        self.sock.send(bytes([2]))
        self.sock.send(len_data)
        self.sock.send(data)
        self.sock.send(self.end_code)
        
    def recv_packet(self):
        x = 0
        while (x < 4):
            if self.start_code[0:1] != self.sock.recv(1):
                x = 0
                continue
            x = x + 1
            
        type_code = self.sock.recv(1)
        len_data = self.sock.recv(4)
        received_data = self.sock.recv(int.from_bytes(len_data, "little"))
        
        x = 0
        while (x < 4):
            if self.end_code[0:1] != self.sock.recv(1):
                break
            x = x + 1
        
        if (x < 4):
            return ''
    
        return received_data.decode('utf-8')