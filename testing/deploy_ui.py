# SPDX-License-Identifier: BSD-3-Clause
# 
# Copyright (c) 2025, Dennis B. Lewis
# All rights reserved.
#
# This file is part of the Aetherion-2350 project.
# Licensed under the BSD 3-Clause License. See LICENSE file for full license text.

import os
import shutil
from time import sleep, time
import serial
import customtkinter as ctk
import threading
import serial

OUTPUT_FILE = "tune_file.bin"
COMPORT = "COM20"
BAUDRATE = 115200

class DownloadBin():

    def __init__(self) -> None:
        #               Z       R     16    0   x0:x7   cs
        self.request = [0x5A, 0x52, 0x10, 0x00, 0x80, 0x00]  # start at offset 0x8000 else firmware will subtract 0x8000 per bank 0
        self.response = bytearray()
        self.file_data = bytearray()
        self.blocks = 8

    def create_checksum(self, trunicate_this:list) -> int:
        return sum(trunicate_this) % 256
    
    def validate_data(self, connection:serial.Serial) -> None:
        verified_checksum, device_checksum = 1, 0
        while device_checksum != verified_checksum:
            connection.write(bytes(self.request))
            self.response = connection.read(4097)
            response = list(self.response)
            if not len(response): continue
            device_checksum = response[-1]
            verified_checksum = self.create_checksum(response[:4096])
        self.file_data.extend(self.response)

    def request_data(self) -> None:
        with serial.Serial(port=COMPORT, baudrate=BAUDRATE, timeout=1) as connection:
            print(f'Connected to {COMPORT} at {BAUDRATE} baud.')
            for _ in range(self.blocks):
                self.request[5] = self.create_checksum(self.request)
                self.validate_data(connection)
                self.request[4] += 0x10
            with open(OUTPUT_FILE, "wb") as file:
                file.write(self.file_data)
            self.request[4] = 0x80
            print(f'\033[92mData written to {os.getcwd()}\\{OUTPUT_FILE}\033[0m')


class CleanBuild:

    def __init__(self):
        self.create_files = 'cmake -G "MinGW Makefiles" ..'
        self.build_firmware = 'mingw32-make'
        self.dev_com_port = 'COM19'
        self.drive_path = 'D:/'
        self.uf2_file = 'Aetherion-v1.0.uf2'
        self.root_dir()
        self.home_directory = os.getcwd()
        self.build_directory = f'{self.home_directory}/build'

    def root_dir(self):
        self.run_command('cd ..')
    
    def drive_available(self):
        start = time()
        available = False
        while not available and ((time() - start) < 5):
            available = os.path.exists(self.drive_path)
        return available

    def delete_build(self):
        try:
            if os.path.isdir(self.build_directory):
                shutil.rmtree(self.build_directory)

            if os.path.isdir(self.build_directory):
                os.rmdir('build')
                
            os.makedirs('build')
        except Exception as e:
            print(e)

    def build(self):
        try:
            os.chdir(self.build_directory)
            self.run_command(self.create_files)
            self.run_command(self.build_firmware)
            os.chdir(self.home_directory)
        except:
            print('Could not make files or build firmware...')

    def clean_pico(self):
        try:
            with serial.Serial(self.dev_com_port, baudrate=115200, timeout=1) as ser:
                ser.write((0x22, 0x01))
        except serial.SerialException as e:
            print(f'\033[91mSerial exception occurred:'+
                  f' {e}\nIs PORT open or device on and functioning?'+
                  f'Perhaps the device is already connected to another software.\033[0m')

    def reset_pico(self):
        try:
            with serial.Serial(self.dev_com_port, baudrate=115200, timeout=1) as ser:
                ser.write((0x22, 0x02))
        except serial.SerialException as e:
            print(f'\033[91mSerial exception occurred:'+
                  f' {e}\nIs PORT open or device on and functioning or in BOOTLOADER mode?\n'+
                  f'Perhaps the device is already connected to another software.'+
                  f'\nUpload Aetherion Firmware and activate DEVELOPER_CONSOLE.\033[0m')

    def deploy_firmware(self):
        uf2_source = os.path.abspath(os.path.join(self.build_directory, self.uf2_file))
        uf2_target = os.path.join(self.drive_path, self.uf2_file)

        if not os.path.exists(uf2_source):
            print(f'Error: UF2 file "{uf2_source}" not found.')
            return False

        if not self.drive_available():
            print(f'Pico Drive Not Found.')
            return False
        
        try:
            shutil.copy(uf2_source, uf2_target)
            print('Deployment: \033[92mSUCCEEDED\033[0m')
        except Exception as e:
            print(f'Error moving UF2 file: {e}')
            return False
        return True

    def run_command(self, command:list):
        os.system(command)

    def upload(self):
        if not os.path.exists(self.drive_path): self.reset_pico()
        while not os.path.exists(self.drive_path):
            os.system('cls')
            print(f'Waiting for {self.drive_path} to open...')
            self.reset_pico()
        self.deploy_firmware()

    def clean_build(self):
        if not os.path.exists(self.drive_path): self.reset_pico()
        self.delete_build()
        self.build()
        while not os.path.exists(self.drive_path):
            os.system('cls')
            print(f'Waiting for {self.drive_path} to open...')
            self.reset_pico()
        self.deploy_firmware()

    def reuse_build(self):
        self.build()
        if not os.path.exists(self.drive_path): self.reset_pico()
        while not os.path.exists(self.drive_path):
            os.system('cls')
            print(f'Waiting for {self.drive_path} to open...')
            self.reset_pico()
        self.deploy_firmware()


class FirmwareUploader(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.firmware = CleanBuild()
        self.tune_download = DownloadBin()
        self.home_directory = os.getcwd()
        self.build_directory = f'{self.home_directory}/build'
        self.title('Deploy UF2')
        self.geometry('300x200')
        theme_color = '#1ee89b'
        hover_color = '#70fac6'
        text_color = '#206b4f'

        self.reset_button = ctk.CTkButton(self, text='Reset Device',
                                          command=self.firmware.reset_pico,
                                          fg_color=theme_color,
                                          corner_radius=0, text_color=text_color,
                                          hover_color=hover_color,
                                          border_width=1,
                                          border_color='black')
        self.reset_button.pack(expand=True, fill='both')

        self.build_upload_button = ctk.CTkButton(self, text='Clean Device (total wipe)',
                                                 command=self.firmware.clean_pico,
                                                 fg_color=theme_color,
                                                 corner_radius=0,
                                                 text_color=text_color,
                                                 hover_color=hover_color,
                                                 border_width=1,
                                                 border_color='black')
        self.build_upload_button.pack(expand=True, fill='both')

        self.build_full_upload_button = ctk.CTkButton(self, text='Full Build and Upload',
                                                 command=self.full_build_and_upload,
                                                 fg_color=theme_color,
                                                 corner_radius=0,
                                                 text_color=text_color,
                                                 hover_color=hover_color,
                                                 border_width=1,
                                                 border_color='black')
        self.build_full_upload_button.pack(expand=True, fill='both')

        self.build_quick_upload_button = ctk.CTkButton(self, text='Quick Build and Upload',
                                                 command=self.quick_build_and_upload,
                                                 fg_color=theme_color,
                                                 corner_radius=0,
                                                 text_color=text_color,
                                                 hover_color=hover_color,
                                                 border_width=1,
                                                 border_color='black')
        self.build_quick_upload_button.pack(expand=True, fill='both')

        self.upload_button = ctk.CTkButton(self, text='Upload Firmware',
                                           command=self.upload_firmware,
                                           fg_color=theme_color,
                                           corner_radius=0,
                                           text_color=text_color,
                                           hover_color=hover_color,
                                           border_width=1,
                                           border_color='black')
        self.upload_button.pack(expand=True, fill='both')

        self.download_button = ctk.CTkButton(self, text='Download BIN',
                                           command=self.download_tune,
                                           fg_color=theme_color,
                                           corner_radius=0,
                                           text_color=text_color,
                                           hover_color=hover_color,
                                           border_width=1,
                                           border_color='black')
        self.download_button.pack(expand=True, fill='both')

    def upload_firmware(self):
        thread = threading.Thread(target=self.firmware.upload, daemon=True)
        thread.start()

    def full_build_and_upload(self):
        thread = threading.Thread(target=self.firmware.clean_build, daemon=True)
        thread.start()
    
    def quick_build_and_upload(self):
        thread = threading.Thread(target=self.firmware.reuse_build, daemon=True)
        thread.start()

    def download_tune(self):
        try:
            self.tune_download.request_data()
        except serial.SerialException:
            print(f'{COMPORT} is already connected to another software or device. i.e. (BMTune, PuTTY)'+
                  f'\ndisconnect that software or device to use {COMPORT} or Reset Device!')
            

if __name__ == '__main__':
    FirmwareUploader().mainloop()

