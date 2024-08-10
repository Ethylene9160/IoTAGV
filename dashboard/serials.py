import serial
import serial.tools.list_ports


class SerialManager:
    def __init__(self):
        # self.port: str = None
        # self.baudrate: int = 115200
        # self.bytesize: int = 8
        # self.parity: str = 'N'
        # self.stopbits: float = 1
        self.ser: serial.Serial = None
        
        self.status = False
    
    @staticmethod
    def scan_serials():
        ports = serial.tools.list_ports.comports()
        available_ports = []
        for port, desc, hwid in sorted(ports):
            available_ports.append({
                'port': port,
                'desc': desc,
                'hwid': hwid
            })
        return available_ports
    
    def close(self):
        try:
            if self.ser and self.status:
                self.ser.close()
            self.status = False
            return True
        except Exception as e:
            print(f'{e}')
        return False
    
    def open(self, port: str, baudrate: int, bytesize: int, parity: str, stopbits: int):
        try:
            self.close()
            self.ser = serial.Serial(port = port, baudrate = baudrate, bytesize = bytesize, parity = parity, stopbits = stopbits, timeout = 1)
            self.status = True
            return True
        except Exception as e:
            print(f'{e}')
        return False
    
    def is_ready(self):
        return hasattr(self, 'ser') and self.ser is not None and self.status
    
    def is_received(self):
        if not self.is_ready():
            return False
        return self.ser.in_waiting
    
    def read(self, size = 1):
        if not self.is_ready():
            return False
        return self.ser.read(size = size)
    
    def readline(self):
        if not self.is_ready():
            return False
        return self.ser.readline()

    def readall(self):
        if not self.is_ready():
            return False
        return self.ser.read(self.ser.in_waiting)

    def write(self, data):
        if not self.is_ready():
            return False
        self.ser.write(data)

