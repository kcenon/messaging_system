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
        
    def recv(self):
        compare_start_code = self.sock.recv(4)
        type_code = self.sock.recv(1)
        len_data = self.sock.recv(4)
        received_data = self.sock.recv(int.from_bytes(len_data, "little"))
        compare_end_code = self.sock.recv(4)
    
        return type_code, received_data