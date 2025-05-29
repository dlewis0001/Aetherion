import board
import digitalio
import board
import digitalio
import usb_cdc
from time import sleep


class DeveloperTool:
    
    def __init__(self):
        self.usb_serial = usb_cdc.console
        self.command_struct = {
            'r' : self.restart_Emulation,
            'b' : self.bootload_Emulation
        }

    def default_pin(self):
        self.run = digitalio.DigitalInOut(board.GP12)  # you can change your pin as you see fit.
        self.run.direction = digitalio.Direction.INPUT
        self.run.pull = digitalio.Pull.DOWN

        self.qspi_ss = digitalio.DigitalInOut(board.GP9)  # you can also change this pin!
        self.qspi_ss.direction = digitalio.Direction.INPUT
        self.qspi_ss.pull = digitalio.Pull.DOWN

    def restart_Emulation(self):
        self.default_pin()
        self.run.direction = digitalio.Direction.OUTPUT
        self.run.value = False

        sleep(0.5)
        self.run.value = True
        self.run.direction = digitalio.Direction.INPUT
        self.run.pull = digitalio.Pull.DOWN
        
        self.run.deinit()
        self.qspi_ss.deinit()

        print('Restart  Request Sent to Emulation Device.')

    def bootload_Emulation(self):
        self.default_pin()
        self.run.direction = digitalio.Direction.OUTPUT
        self.qspi_ss.direction = digitalio.Direction.OUTPUT 

        self.run.value = False   
        self.qspi_ss.value = False

        sleep(0.5)
        self.run.value = True
        self.run.direction = digitalio.Direction.INPUT
        self.run.pull = digitalio.Pull.DOWN
        while not self.run.value: pass
        self.run.deinit()
        
        sleep(0.5)
        self.qspi_ss.value = True
        self.qspi_ss.direction = digitalio.Direction.INPUT
        self.qspi_ss.pull = digitalio.Pull.DOWN
        while not self.qspi_ss.value: pass
        self.qspi_ss.deinit()

        print('Bootload Request Sent to Emulation Device.')

    def console_read(self):
        return self.usb_serial.read(1)

    def process_command(self, command):
        if command in self.command_struct:
            self.command_struct[command]()

    def run_console(self):
        command = ""
        while True:
            char = self.console_read()
            if char == b'\r':
                self.process_command(command)
                command = ""
            else:
                command += char.decode('utf-8')


DeveloperTool().run_console()

#type: ignore