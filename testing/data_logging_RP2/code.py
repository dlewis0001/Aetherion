import board
import busio
from random import randint
from lib.abstract import ecu_data

class SimulateEngineUnit:
    def __init__(self, uart: busio.UART):
        self.ecu_data_list = []
        self.uart = uart
        self.ready = 0xCD

    def crc8_checksum(self):
        return sum(self.ecu_data_list) % 256

    def create_random(self):
        self.ecu_data_list = ecu_data()
        checksum = self.crc8_checksum()
        self.ecu_data_list.append(checksum)

    def match_data(self, data):
        if not data:
            return

        if data == b'\x10\x00':
            self.uart.write(bytes([self.ready]))  # send 1-byte "ready" signal
        elif data == b'\x20\x00':
            self.create_random()
            self.uart.write(bytes(self.ecu_data_list))  # send all 52 bytes
        elif data == b'\x50\x00':
            self.uart.write(bytes([0x50]))  # send 1-byte "ready" signal

    def send_ecu_data(self):
        while True:
            data = self.uart.read(2)  # wait for a 2-byte command
            if data:
                print(data, len(data))
            self.match_data(data)

            
uart0 = busio.UART(board.GP0, board.GP1, baudrate=38400)
SimulateEngineUnit(uart0).send_ecu_data()

#type: ignore