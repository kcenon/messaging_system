from concurrent.futures import thread
import re
import sys
import socket
import logging
import threading

class value:
    
    parent = None
    name_string = None
    type_string = None
    value_string = None
    values = []
        
    def __init__(self, name_string, type_string, value_string, values = []):
        self.name_string = name_string
        self.type_string = type_string
        self.value_string = value_string
        self.values = values
        
    def append(self, child_value):
        child_value.parent = self
        self.values.append(child_value)
        
    def remove(self, name_string):
        result = []
        
        for current in self.values:
            if not current:
                continue
            
            if current.name_string == name_string:
                continue
            
            result.append(current)
            
        self.values = result
        
    def get(self, name_string):
        if not name_string:
            return self.values
        
        result = []
        
        for current in self.values:
            if not current:
                continue
            
            if current.name_string != name_string:
                continue
            
            result.append(current)
            
        return result
    
    def serialize(self):
        result = "[{},{},{}];".format(self.name_string, self.type_string, self.value_string)
        
        for current in self.values:
            if not current:
                continue
            
            result = result + current.serialize()
            
        return result

class container:
    
    headers = {}
    values = []
        
    def __init__(self, message = ''):
        self.headers = { '1':"", '2':"", '3':"", '4':"", '5':"", '6':"1.0.0.0" }
        self.values = []
        self.parse(message, False)
        
    def create(self, target_id = '', target_sub_id = '', source_id = '', source_sub_id = '', message_type = '', values = []):
        self.headers['1'] = target_id
        self.headers['2'] = target_sub_id
        self.headers['3'] = source_id
        self.headers['4'] = source_sub_id
        self.headers['5'] = message_type
        self.headers['6'] = "1.0.0.0"
        self.data_string = ''
        self.values = values
        self.deserialized = True
        
    def parse(self, message, parsing):
        self.data_string = message
        self.deserialized = parsing
        
        if message == '':
            return
        
        message = re.sub(r'\r\n?|\n', '', message)
        
        self._parse_header(re.search(r'@header=[\s?]*\{[\s?]*(.*?)[\s?]*\};', message).group())
        self._parse_data(re.search(r'@data=[\s?]*\{[\s?]*(.*?)[\s?]*\};', message).group(), parsing)

    def append(self, child_value):
        self.deserialized = True
        child_value.parent = None
        self.values.append(child_value)
        
    def get(self, name_string):
        if not self.deserialized:
            self._parse_data(self.data_string, True)
            
        if not name_string:
            return self.values
        
        result = []
        
        for current in self.values:
            if not current:
                continue
            
            if current.name_string != name_string:
                continue
            
            result.append(current)
            
        return result
        
    def serialize(self):
        if self.deserialized:
            self.data_string = self._make_string()
            self.deserialized = False
        
        result = "{}[1,{}];[2,{}];[3,{}];[4,{}];[5,{}];[6,{}];{}{}".format(
            "@header={", self.headers['1'], self.headers['2'], self.headers['3'], self.headers['4'],
            self.headers['5'], self.headers['6'], "};", self.data_string)
        
        return result
    
    def target_id(self):
        return self.headers['1']
    
    def target_sub_id(self):
        return self.headers['2']
    
    def source_id(self):
        return self.headers['3']
    
    def source_sub_id(self):
        return self.headers['4']
    
    def message_type(self):
        return self.headers['5']
    
    def version(self):
        return self.headers['6']
    
    def set_target_id(self, value_string):
        self.headers['1'] = value_string
    
    def set_target_sub_id(self, value_string):
        self.headers['2'] = value_string
    
    def set_source_id(self, value_string):
        self.headers['3'] = value_string
    
    def set_source_sub_id(self, value_string):
        self.headers['4'] = value_string
    
    def set_message_type(self, value_string):
        self.headers['5'] = value_string
        
    def _parse_header(self, header_string):
        if not header_string:
            return
        
        results = re.findall(r'\[(\w+),(.*?)\];', header_string)
        for result in results:
            type_string, data_string = result
            
            try:
                self.headers[type_string] = data_string
            except:
                logging.error("cannot parse header with unknown type: {}".format(type_string))
    
    def _parse_data(self, data_string, parsing):
        self.data_string = data_string
        self.deserialized = parsing
        if not parsing:
            return
        
        value_list = []
        results = re.findall(r'\[(\w+),[\s?]*(\w+),[\s?]*(.*?)\];', data_string)
        for result in results:
            name_string, type_string, value_string = result
            value_list.append(value(name_string, type_string, value_string))
            
        previous_value = None
        for current_value in value_list:
            if not previous_value:
                self.append(current_value)
                if current_value.type_string == 'e':
                    previous_value = current_value
                continue

            previous_value.append(current_value)
            if current_value.type_string == 'e':
                previous_value = current_value
                continue

            if "{}".format(len(previous_value.values)) == previous_value.value_string:
                previous_value = previous_value.parent
    
    def _make_string(self):
        result = "@data={"
        
        for current in self.values:
            result = result + current.serialize()
            
        result = result + "};"
            
        return result

class messaging_client:
        
    def __init__(self, source_id, connection_key, start_number = 231, end_number = 67, conn_callback = None, recv_callback = None):
        self.source_id = source_id
        self.source_sub_id = ''
        self.connection_key = connection_key
        self.start_code = bytes([start_number, start_number, start_number, start_number])
        self.end_code = bytes([end_number, end_number, end_number, end_number])
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.conn_callback = conn_callback
        self.recv_callback = recv_callback
        
    def start(self, server_ip, server_port):
        server_address = (server_ip, server_port)

        try:
            self.sock.connect(server_address)
        except:
            return False
              
        self.recv_thread = threading.Thread(target=self.recv)
        self.recv_thread.daemon = True
        self.recv_thread.start()
        
        self._send_connection()
        
        return True
        
    def stop(self):
        if self.sock is not None:
            self.sock.close()
            self.sock = None
        
    def send_packet(self, packet):
        if not packet.target_id:
            logging.error("cannot send with null target id")
            return
            
        if packet.source_id() == '':
            packet.set_source_id(self.source_id)
            packet.set_source_sub_id(self.source_sub_id)
                    
        data_array = bytes(packet.serialize(), 'utf-8')
        len_data = len(data_array).to_bytes(4, byteorder='little')
    
        self.sock.send(self.start_code)
        self.sock.send(bytes([2]))
        self.sock.send(len_data)
        self.sock.send(data_array)
        self.sock.send(self.end_code)

        logging.debug("[sent]=> {}".format(packet.serialize()))
    
    def recv(self):
        logging.info("start messaging_client: {}".format(self.source_id))
        
        while self.sock is not None:
            
            try:
                message = self._recv_packet()
            except:
                break
            
            if message is None:
                continue
            
            if message.message_type() == "confirm_connection":
                
                confirm = message.get('confirm')
                if not confirm:
                    logging.error("cannot parse confirm message from {}".format(message.source_id()))
                    
                    if self.conn_callback is not None:
                        self.conn_callback(message.source_id(), message.source_sub_id(), False)
                        
                    break

                self.source_id = message.target_id()
                self.source_sub_id = message.target_sub_id()
                logging.info("received connection message from {}: confirm [{}]".format(message.source_id(), confirm[0].value_string))
                
                if self.conn_callback is not None:
                    self.conn_callback(message.source_id(), message.source_sub_id(), True)
                        
                continue
            
            if self.recv_callback is not None:
                self.recv_callback(message)
        
        logging.info("stop messaging_client: {}".format(self.source_id))
            
    def _recv_packet(self):
        x = 0
        while (x < 4):
            if self.start_code[0:1] != self.sock.recv(1):
                x = 0
                continue
            x = x + 1
            
        type_code = self.sock.recv(1)
        if type_code != bytes([2]):
            return None
        
        len_data = self.sock.recv(4)
        received_data = self.sock.recv(int.from_bytes(len_data, "little"))
        
        x = 0
        while (x < 4):
            if self.end_code[0:1] != self.sock.recv(1):
                break
            x = x + 1
        
        if (x < 4):
            return None
        
        packet_string = received_data.decode('utf-8')
        logging.debug("[received]=> {}".format(packet_string))

        return container(packet_string)
    
    def _send_connection(self):
        connection_packet = container()
        connection_packet.create('server', '', self.source_id, self.source_sub_id, 
                                'request_connection', 
                                [ value('connection_key', 'd', self.connection_key),
                                value('auto_echo', '1', 'false'),
                                value('auto_echo_interval_seconds', '3', '1'),
                                value('session_type', '2', '1'),
                                value('bridge_mode', '1', 'false'),
                                value('snipping_targets', 'e', '0') ])

        self.send_packet(connection_packet)
        