#!/usr/bin/python3
# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from pathlib import Path
from progress.bar import Bar
import argparse
import contextlib
import hexformat
import hid
import inspect
import ipaddress
import os
import platform
import re
import serial
import serial.tools.list_ports
import signal
import struct
import subprocess
import sys
import tempfile
import time
import usb.core

platform_dir = ''
toolchain_dir = ''
exe_extension = ''
skip_hid_readback = False
system_name = platform.system()
if system_name == 'Windows':
   platform_dir = 'win'
   toolchain_dir = 'toolchain-win'
   exe_extension = '.exe'
   skip_hid_readback = True
elif system_name == 'Darwin':
   platform_dir = 'mac'
   toolchain_dir = 'toolchain-mac'
   skip_hid_readback = True
elif system_name == 'Linux':
   platform_dir = 'linux/amd64'
   toolchain_dir = 'toolchain'
else:
   print('Unknown operating system!' + system_name)
   raise OSError

SDP_VID = 0x1fc9
SDP_PID = 0x013d

FLASHLOADER_VID = 0x15a2
FLASHLOADER_PID = 0x0073

ELFLOADER_VID = 0x18d1
ELFLOADER_PID = 0x9307
ELFLOADER_OLD_PID = 0x93fe

OPEN_HID_RETRY_INTERVAL_S = 0.1
OPEN_HID_RETRY_TIME_S = 15

BLOCK_SIZE = 2048 * 64
BLOCK_COUNT = 64

USB_IP_ADDRESS_FILE = '/usb_ip_address'
WIFI_SSID_FILE = '/wifi_ssid'
WIFI_PSK_FILE = '/wifi_psk'
WIFI_COUNTRY_FILE = '/wifi_country'
WIFI_REVISION_FILE = '/wifi_revision'
ETHERNET_SPEED_FILE = '/ethernet_speed'

ELFLOADER_SETSIZE = 0
ELFLOADER_BYTES = 1
ELFLOADER_DONE = 2
ELFLOADER_RESET_TO_BOOTLOADER = 3
ELFLOADER_TARGET = 4
ELFLOADER_RESET_TO_FLASH = 5
ELFLOADER_FORMAT = 6

ELFLOADER_TARGET_RAM = 0
ELFLOADER_TARGET_PATH = 1
ELFLOADER_TARGET_FILESYSTEM = 2

def elfloader_msg_setsize(size):
    return struct.pack('=BBl', 0, ELFLOADER_SETSIZE, size)

# 64 bytes HID packet, adjust for header and padding
ELFLOADER_MAX_BYTES_PER_PACKET = 64 - struct.calcsize('=BBll') + 1

def elfloader_msg_bytes(offset, data):
    return struct.pack('=BBll%ds' % len(data), 0, ELFLOADER_BYTES,
                       len(data), offset, data);
def elfloader_msg_done():
    return struct.pack('=BB', 0, ELFLOADER_DONE)

def elfloader_msg_reset_to_bootloader():
    return struct.pack('=BB', 0, ELFLOADER_RESET_TO_BOOTLOADER)

def elfloader_msg_target(target):
    return struct.pack('=BBB', 0, ELFLOADER_TARGET, target)

def elfloader_msg_reset_to_flash():
    return struct.pack('=BB', 0, ELFLOADER_RESET_TO_FLASH)

def elfloader_msg_format():
    return struct.pack('=BB', 0, ELFLOADER_FORMAT)

def read_file(path):
    with open(path, 'rb') as f:
        return f.read()

def sdp_vidpid():
    return '{},{}'.format(hex(SDP_VID), hex(SDP_PID))

def flashloader_vidpid():
    return '{},{}'.format(hex(FLASHLOADER_VID), hex(FLASHLOADER_PID))

def is_coral_micro_connected(serial_number):
    return serial_number in EnumerateCoralMicro()

def is_elfloader_connected(serial_number):
    for device in usb.core.find(idVendor=ELFLOADER_VID, idProduct=ELFLOADER_PID,
                                find_all=True):
        if not serial_number or device.serial_number == serial_number:
            return True
    for device in usb.core.find(idVendor=ELFLOADER_VID, idProduct=ELFLOADER_OLD_PID,
                                find_all=True):
        if not serial_number or device.serial_number == serial_number:
            return True
    return False

def is_sdp_connected():
    return bool(usb.core.find(idVendor=SDP_VID, idProduct=SDP_PID))

def is_flashloader_connected():
    return bool(usb.core.find(idVendor=FLASHLOADER_VID, idProduct=FLASHLOADER_PID))

def EnumerateSDP():
    return hid.enumerate(SDP_VID, SDP_PID)

def EnumerateFlashloader():
    return hid.enumerate(FLASHLOADER_VID, FLASHLOADER_PID)

def EnumerateElfloader():
    return hid.enumerate(ELFLOADER_VID, ELFLOADER_PID)

SERIAL_PORT_RE = re.compile(f'USB VID:PID=18D1:(9308|93FF) SER=([0-9A-Fa-f]+)')
def EnumerateCoralMicro():
    serial_list = []
    for port in serial.tools.list_ports.comports():
        matches = SERIAL_PORT_RE.match(port.hwid)
        if matches:
            serial_list.append(matches.group(2).lower())
    return serial_list

def FindElfloader(build_dir, cached_files):
    default_path = os.path.join(build_dir, 'apps', 'ELFLoader', 'image.srec')
    if os.path.exists(default_path):
        return default_path
    if cached_files:
        for f in cached_files:
            if 'ELFLoader/image.srec' in f:
                return f
    else:
        for root, dirs, files in os.walk(build_dir):
            if os.path.split(root)[1] == "ELFLoader":
                for file in files:
                    if file == "image.srec":
                        return os.path.join(root, file)
    return None



"""
Gets the full set of libraries that comprise a target executable.

Args:
  build_dir: path to CMake output directory
  libs_path: path to .libs file of target executable
  scanned: set of already scanned libraries. Pass set() for initial call,
           recursive calls use this data to break cycles.
  cached_files: list of file locations (to avoid using os.walk).

Returns:
  A set containing .libs file of all library dependencies
"""
def GetLibs(build_dir, libs_path, scanned, cached_files):
    libs_found = set()
    with open(libs_path, 'r') as f:
        libs = f.readline().split(';')
        for lib in libs:
            if cached_files:
                for f in cached_files:
                        if lib + '.libs' in f:
                            if f not in scanned:
                                libs_found.add(f)
                            break
            else:
                for root, dirs, files in os.walk(build_dir):
                    for file in files:
                        libs_path = os.path.join(root, file)
                        if libs_path in scanned:
                            continue
                        if file == lib + '.libs':
                            libs_found.add(libs_path)
    sublibs = set()
    for lib in libs_found:
        sublibs |= GetLibs(build_dir, lib, scanned | libs_found, cached_files)
    return libs_found | sublibs

def CreateFilesystem(workdir, root_dir, build_dir, elf_path, cached_files, is_arduino, data_dir, data):
    if not data:
        return dict()
    if is_arduino:
        if not data_dir:
            return dict()
        folder_resources = dict()
        for root, dirs, files in os.walk(data_dir):
            for file in files:
                src_path = os.path.join(root, file)
                tgt_path = src_path.replace(data_dir, '')
                folder_resources[src_path] = tgt_path
        return folder_resources
    libs_path = os.path.splitext(elf_path)[0] + '.libs'
    m4_exe_path = os.path.splitext(elf_path)[0] + '.m4_executable'
    libs = GetLibs(build_dir, libs_path, set(), cached_files)
    libs.add(libs_path)
    if os.path.exists(m4_exe_path):
        with open(m4_exe_path) as f:
            m4_exe = f.readline()
        m4_exe_libs = os.path.join(os.path.dirname(m4_exe_path), m4_exe) + '.libs'
        libs |= GetLibs(build_dir,
                           m4_exe_libs,
                           set(), cached_files)
        libs.add(m4_exe_libs)
    data_files = set()
    for lib in libs:
        data_path = os.path.splitext(lib)[0] + '.data'
        with open(data_path, 'r') as f:
            data_files |= set(f.readline().split(';'))
    try:
        data_files.remove('')
    except KeyError:
        # If the key is not found, don't panic.
        pass

    data_files = dict((file.split('>')[0], file.split('>')[0] if len(file.split('>')) == 1 else file.split('>')[1])
                      for file in data_files)
    all_files_exist = True
    for relative_src in list(data_files.keys()):
        absolute_src = os.path.join(build_dir, relative_src)
        if not os.path.exists(absolute_src):
            print('%s does not exist!' % absolute_src)
            all_files_exist = False
        else:
            data_files[absolute_src] = data_files.pop(relative_src)
    if not all_files_exist:
        return None

    return data_files

def CreateSbFile(workdir, elftosb_path, srec_path, erase):
    spinand_bdfile_path = os.path.join(workdir, 'program_flexspinand_image.bd')
    itcm_bdfile_path = os.path.join(workdir, 'imx-itcm-unsigned.bd')

    srec_obj = hexformat.srecord.SRecord.fromsrecfile(srec_path)
    assert len(srec_obj.parts()) == 1
    itcm_startaddress = srec_obj.parts()[0][0] & 0xFFFF0000

    with open(spinand_bdfile_path, 'w') as spinand_bdfile:
        erase_cmd = 'erase spinand 0xc..0x4c;'
        spinand_bdfile.write(
'''sources {
  bootImageFile = extern (0);
}
section (0) {
  load 0xC2000105 > 0x10000;
  load 0x00010020 > 0x10004;
  load 0x00040004 > 0x10008;
  load 0x00080004 > 0x1000C;
  load 0xC0010021 > 0x10020;
  enable spinand 0x10000;
  erase spinand 0x4..0x8;
  erase spinand 0x8..0xc;
''' +
(erase_cmd if erase else '') +
'''
  load spinand bootImageFile > 0x4;
  load spinand bootImageFile > 0x8;
}
''')

    with open(itcm_bdfile_path, 'w') as itcm_bdfile:
        itcm_bdfile.write(
'''options {
  flags = 0x00;
  startAddress = %s;
  ivtOffset = 0x400;
  initialLoadSize = 0x1000;
}
sources {
  elfFile = extern(0);
}
section (0) {
}
''' % hex(itcm_startaddress))
    ivt_bin_path = os.path.join(workdir, 'ivt_program.bin')
    sbfile_path = os.path.join(workdir, 'program.sb')
    subprocess.check_call([elftosb_path, '-f', 'imx', '-V', '-c', itcm_bdfile_path, '-o', ivt_bin_path, srec_path])
    args = [elftosb_path, '-f', 'kinetis', '-V', '-c', spinand_bdfile_path, '-o', sbfile_path, ivt_bin_path]
    subprocess.check_call(args)
    return sbfile_path

def FlashtoolDone(success_msg=None):
    if success_msg:
        print(success_msg)
    return StateDone

def StateDone():
    pass

def FlashtoolError(error_msg='Unknown error'):
    print(f'Encountered an error during flashing: {error_msg}')
    return StateFlashtoolError

def StateFlashtoolError():
    pass

def StateCheckForAny(serial_number=None):
    for i in range(10):
        if is_coral_micro_connected(serial_number):
            return StateCheckForCoralMicro
        if is_elfloader_connected(serial_number):
            return StateCheckForElfloader
        if is_sdp_connected():
            return StateCheckForSdp
        if is_flashloader_connected():
            return StateCheckForFlashloader
        time.sleep(1)
    return FlashtoolError('Unable to find device in any recognized mode.')

def StateCheckForCoralMicro(serial_number=None):
    for i in range(10):
        if is_coral_micro_connected(serial_number):
            # port is needed later as well, wait for it.
            port = FindSerialPortForDevice(serial_number)
            if port:
                return StateResetToSdp
        time.sleep(1)
    # If we don't see a Dev Board Micro on the bus, just check for SDP.
    return StateCheckForSdp

def StateCheckForElfloader(serial_number=None):
    for i in range(10):
        if is_elfloader_connected(serial_number):
            return StateResetElfloader
        time.sleep(1)
    return FlashtoolError('Unable to find device in ELFLoader mode.')

def FindSerialPortForDevice(serial_number=None):
    for port in serial.tools.list_ports.comports():
        try:
            if port.serial_number and port.serial_number.lower() == serial_number:
                return port.device
        except ValueError:
            pass
    return None

def StateResetToSdp(serial_number=None):
    port = FindSerialPortForDevice(serial_number)
    if port is None:
        return FlashtoolError('Device serial port not found.')

    # Port could be enumerated at this point but udev rules are still to get applied.
    # Typically this results in exceptions like "[Errno 13] Permission denied: '/dev/ttyACM0'"
    for i in range(10):
        with serial.Serial(baudrate=1200) as s:
            try:
                s.port = port
                s.dtr = False
                s.open()
                s.write(b'42')  ## Dummy write to force apply s.dtr.
                return StateCheckForSdp
            except serial.SerialException as e:
                # If the port goes away, we get SerialException.
                # Assume that this means the device reset into SDP, and try to proceed.
                return StateCheckForSdp
            except:
                pass
        time.sleep(1)
    return FlashtoolError(f'Unable to open serial port at {port}')

def StateCheckForSdp():
    for i in range(10):
        if is_sdp_connected():
            return StateLoadFlashloader
        time.sleep(1)
    return FlashtoolError('Unable to find device in SDP mode.')

def StateLoadFlashloader(blhost_path, flashloader_path):
    subprocess.check_call([blhost_path, '-u', sdp_vidpid(), '--', 'load-image', flashloader_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return StateCheckForFlashloader

def StateLoadElfloader(toolchain_path, elfloader_path, elfloader_elf_path, blhost_path, ram, target_elfloader=False):
    symbols = subprocess.check_output('{} -t {}'.format(os.path.join(toolchain_path, 'arm-none-eabi-objdump') + exe_extension, elfloader_elf_path), shell=True, text=True)
    disable_usb_timeout_address = 0
    for symbol in symbols.splitlines():
        if not 'disable_usb_timeout' in symbol:
            continue
        disable_usb_timeout_address = int(symbol.split()[0], 16)
        break
    if not disable_usb_timeout_address:
        raise Exception('Failed to find disable_usb_timeout symbol in {}'.format(elfloader_elf_path))
    start_address = hexformat.srecord.SRecord.fromsrecfile(elfloader_path).startaddress
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), 'flash-image', elfloader_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.check_output('{} -u {} write-memory {} {{{{ffffffff}}}}'.format(blhost_path, flashloader_vidpid(), hex(disable_usb_timeout_address)), shell=True, text=True)
    subprocess.call([blhost_path, '-u', flashloader_vidpid(), 'call', hex(start_address), '0'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if target_elfloader:
        # We omit a success message here, since this branch is only used internally for putting devices in a known good state.
        return StateDone
    if ram:
        return StateProgramElfloader
    return StateProgramDataFiles

def StateCheckForFlashloader(ram, target_elfloader=False):
    for i in range(10):
        if is_flashloader_connected():
            if ram or target_elfloader:
                return StateLoadElfloader
            else:
                return StateProgram
        time.sleep(1)
    return FlashtoolError(f'Unable to find device in flashloader mode.')

@contextlib.contextmanager
def OpenHidDevice(vid, pid, serial_number):
    h = hid.device()
    # First few tries to open the HID device can fail,
    # if Python is faster than the device. So retry a bit.
    print('OpenHidDevice vid={:x} pid={:x} ...'.format(vid, pid))
    for _ in range(round(OPEN_HID_RETRY_TIME_S / OPEN_HID_RETRY_INTERVAL_S)):
        try:
            h.open(vid, pid, serial_number=serial_number)
            print('OpenHidDevice vid={:x} pid={:x} opened'.format(vid, pid, serial_number))
            try:
                yield h
            finally:
                h.close()
            return
        except:
            time.sleep(OPEN_HID_RETRY_INTERVAL_S)

    raise Exception('Failed to open Dev Board Micro HID device')

def StateResetElfloader(serial_number=None):
    with OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number) as h:
        h.write(elfloader_msg_reset_to_bootloader())
    return StateCheckForSdp

def StateProgram(blhost_path, sbfile_path):
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), 'receive-sb-file', sbfile_path])
    return StateLoadElfloader

def ElfloaderTransferData(h, data, target, bar=None):
    warned = False
    def read_byte():
        if skip_hid_readback is True:
            return
        try:
            h.read(1, timeout_ms=1000)
        except:
            nonlocal warned
            if not warned:
                print('Warning: failed to read ACK byte after TX')
                warned = True

    total_bytes = len(data)
    h.write(elfloader_msg_target(target))
    read_byte()

    h.write(elfloader_msg_setsize(total_bytes))
    read_byte()

    bytes_transferred = 0
    while bytes_transferred < total_bytes:
        bytes_this_packet = min(ELFLOADER_MAX_BYTES_PER_PACKET,
                                total_bytes - bytes_transferred)
        h.write(elfloader_msg_bytes(bytes_transferred,
            data[bytes_transferred:bytes_transferred + bytes_this_packet]))
        read_byte()
        bytes_transferred += bytes_this_packet
        if bar:
            bar.goto(bytes_transferred)
    h.write(elfloader_msg_done())
    read_byte()
    if bar:
        bar.finish()

def StateProgramElfloader(elf_path, debug=False, serial_number=None):
    with OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number) as h:
        data = read_file(elf_path)
        ElfloaderTransferData(h, data, ELFLOADER_TARGET_RAM,
                              bar=Bar(elf_path, max=len(data)))
        if not debug:
            return FlashtoolDone('Flashing to RAM is complete, your application should be executing.')
        return StateStartGdb

def StateProgramDataFiles(elf_path, data_files, usb_ip_address, wifi_ssid=None, wifi_psk=None, wifi_country=None, wifi_revision=None, serial_number=None, ethernet_speed=None, program=True, data=True):
    with OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number) as h:
        if program and data:
            h.write(elfloader_msg_format())
        if program:
            data_files[elf_path] = '/default.elf'
        data_files[str(usb_ip_address).encode()] = USB_IP_ADDRESS_FILE
        if wifi_ssid is not None:
            data_files[wifi_ssid.encode()] = WIFI_SSID_FILE
        if wifi_psk is not None:
            data_files[wifi_psk.encode()] = WIFI_PSK_FILE
        if wifi_country is not None:
            data_files[wifi_country.encode()] = WIFI_COUNTRY_FILE
        if wifi_revision is not None:
            data_files[struct.pack('<H', wifi_revision)] = WIFI_REVISION_FILE
        if ethernet_speed is not None:
            data_files[struct.pack('<H', ethernet_speed)] = ETHERNET_SPEED_FILE
        for src_file, target_file in data_files.items():
            # If we are running on something with the wrong directory separator, fix it.
            target_file = target_file.replace('\\', '/')
            if target_file[0] != '/':
                target_file = '/' + target_file

            if isinstance(src_file, str):
                data = read_file(src_file)
            elif isinstance(src_file, bytes):
                data = src_file
            else:
                raise RuntimeError('src_file must be "str" or "bytes"')

            ElfloaderTransferData(h, target_file.encode(), ELFLOADER_TARGET_PATH)
            ElfloaderTransferData(h, data, ELFLOADER_TARGET_FILESYSTEM,
                                  bar=Bar(target_file, max=len(data)))

        return StateResetToFlash

def StateResetToBootloader(serial_number=None, reset=False):
    if reset:
        with OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number) as h:
            h.write(elfloader_msg_reset_to_bootloader())
    return FlashtoolDone('Resetting device to SDP mode complete.')

def StateResetToFlash(serial_number=None, reset=False):
    if reset:
        with OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number) as h:
            h.write(elfloader_msg_reset_to_flash())
            return FlashtoolDone('Flashing to flash storage complete, the device is restarting to execute your application.')
    return FlashtoolDone('Flashing to flash storage complete, restart the device to execute your application.')

def StateStartGdb(jlink_path, toolchain_path, unstripped_elf_path):
    jlink_gdbserver = os.path.join(jlink_path, 'JLinkGDBServerCLExe')
    gdb_exe = os.path.join(toolchain_path, 'arm-none-eabi-gdb')
    if not os.path.exists(jlink_gdbserver):
        return FlashtoolError(f'Failed to start debug session: {jlink_gdbserver} does not exist.')
    if not os.path.exists(gdb_exe):
        return FlashtoolError(f'Failed to start debug session: {gdb_exe} does not exist.')

    signal.signal(signal.SIGINT, signal.SIG_IGN)
    with subprocess.Popen(
        [jlink_gdbserver, '-select', 'USB', '-device', 'MIMXRT1176xxx8_M7', '-endian', 'little', '-if', 'JTAG', '-speed', '4000', '-noir', '-noLocalhostOnly', '-rtos', 'GDBServer/RTOSPlugin_FreeRTOS.so'],
        stdin=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL, preexec_fn=os.setpgrp) as gdbserver:
        with tempfile.NamedTemporaryFile('w') as gdb_commands:
            gdb_commands.writelines([
                'file ' + unstripped_elf_path + '\n',
                'target remote localhost:2331\n',
                'continue\n'
            ])
            gdb_commands.flush()
            with subprocess.Popen([gdb_exe, '-x', gdb_commands.name]) as gdb:
                gdb.communicate()
        gdbserver.terminate()
    return FlashtoolDone('Debug session complete.')

def state_name(state):
    return ''.join('_' + c if c.isupper() else c for c in state.__name__)[1:].upper()

def RunFlashtool(state, **kwargs):
    prev_state = None
    while True:
        if state is not StateFlashtoolError and state is not StateDone:
          print(state_name(state))
        if state is StateDone or state is StateFlashtoolError:
            break
        params = inspect.signature(state).parameters.values()
        state_kwargs =  {param.name : kwargs.get(param.name, param.default) for param in params}
        (prev_state, state) = (state, state(**state_kwargs))

def main():
    # Check if we're running from inside a pyinstaller binary.
    # The true branch is pyinstaller, false branch is executing directly.
    if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
        root_dir = os.path.abspath(os.path.dirname(__file__))
    else:
        root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

    parser = argparse.ArgumentParser(description='Coral Dev Board Micro flashtool',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    basic_group = parser.add_argument_group('Flashing options')
    basic_group.add_argument(
        '--build_dir', '-b', type=str, required=True,
        help='Path to the coralmicro build output.')
    basic_group.add_argument(
        '--subapp', type=str, required=False,
        help='The target name you want to flash. This is needed only when \
        using --app or --example and the target name is different than the \
        name given to those arguments.')
    basic_group.add_argument(
        '--ram', dest='ram', action='store_true',
        help='Flashes the app to the RAM only, instead of to flash memory. \
        This means the app will be overrwritten upon reset. Without this flag, \
        it flashes to the flash memory and resets the board to load it.')
    basic_group.add_argument(
        '--noram', dest='ram', action='store_false',
        help=argparse.SUPPRESS)
    basic_group.add_argument(
        '--noreset', dest='reset', action='store_false',
        help='Prevents resetting the board after flashing the board.')
    basic_group.add_argument(
        '--reset', dest='reset', action='store_true',
        help=argparse.SUPPRESS)
    parser.add_argument(
        '--elfloader_path', type=str, required=False,
        help=argparse.SUPPRESS)
    basic_group.add_argument(
        '--serial', type=str, required=False,
        help='The board serial number you want to flash. This is necessary \
        only if you have multiple Coral Dev Board Micro devices connected.')
    parser.add_argument(
        '--list', dest='list', action='store_true',
        help='Prints all detected Coral Dev Board Micro devices.')

    network_group = parser.add_argument_group('Board network settings')
    network_group.add_argument(
        '--usb_ip_address', type=str, required=False, default='10.10.10.1',
        help='The board IP address for Ethernet-over-USB connections.')
    network_group.add_argument(
        '--wifi_ssid', type=str, required=False, default=None,
        help='The default Wi-Fi SSID for the board to use. \
        Requires the Coral Wireless Add-on Board (or similar).')
    network_group.add_argument(
        '--wifi_psk', type=str, required=False, default=None,
        help='The Wi-Fi password to use with --wifi_ssid.')
    network_group.add_argument(
        '--wifi_country', type=str, required=False, default=None,
        help='The Wi-Fi country code to use with --wifi_ssid.')
    network_group.add_argument(
        '--wifi_revision', type=int, required=False, default=None,
        help='The Wi-Fi revision to use with --wifi_ssid.')
    network_group.add_argument(
        '--ethernet_speed', type=int, choices=[10, 100],
        required=False, default=None,
        help='The maximum ethernet speed in Mbps. Must be one of: \
        %(choices)s. Requires the Coral PoE Add-on Board (or similar).')

    debug_group = parser.add_argument_group('Debugging options')
    debug_group.add_argument(
        '--strip', dest='strip', action='store_true',
        help='If you specify an unstripped binary with --elf_path to use for \
        debugging, this argument adds the stripped file to the default.elf \
        package that gets flashed.')
    parser.add_argument(
        '--toolchain', type=str, required=False,
        help=argparse.SUPPRESS)
    debug_group.add_argument(
        '--debug', dest='debug', action='store_true',
        help='Loads the app into RAM, starts the JLink debug server, and \
        attaches GDB.')
    debug_group.add_argument(
        '--jlink_path', type=str, default='/opt/SEGGER/JLink',
        help='Path to JLink if --debug is enabled.')

    advanced_group = parser.add_argument_group('Advanced options')
    advanced_group.add_argument(
        '--cached', dest='cached', action='store_true',
        help='Flashes using cached target files, which can speed up the \
        flashing process. You must first run scripts/cache_build.py.')
    advanced_group.add_argument(
        '--data_dir', type=str, required=False,
        help=argparse.SUPPRESS)
    parser.add_argument(
        '--program', dest='program', action='store_true',
        help=argparse.SUPPRESS)
    advanced_group.add_argument(
        '--noprogram', dest='program', action='store_false',
        help='Prevents flashing the program code (only data is flashed, \
        unless --nodata is also specified).')
    parser.add_argument(
        '--data', dest='data', action='store_true',
        help=argparse.SUPPRESS)
    advanced_group.add_argument(
        '--nodata', dest='data', action='store_false',
        help='Prevents flashing the app data (only program code is flashed, \
        unless --noprogram is also specified).')
    parser.add_argument(
        '--arduino', dest='arduino', action='store_true',
        help=argparse.SUPPRESS)
    parser.set_defaults(program=True, data=True, arduino=False)

    app_elf_group = basic_group.add_mutually_exclusive_group(required=True)
    app_elf_group.add_argument(
        '--app', '-a', type=str,
        help='Name of the coralmicro "app" to flash. Must be used with \
        --build_dir.')
    app_elf_group.add_argument(
        '--example', '-e', type=str,
        help='Name of the coralmicro "example" to flash. Must be used with \
        --build_dir.')
    app_elf_group.add_argument(
        '--elf_path', type=str,
        help='Path to your project\'s .stripped binary (ELF file). This must \
        be the full path and filename.')

    args = parser.parse_args()

    usb_ip_address = ipaddress.ip_address(args.usb_ip_address)

    build_dir = os.path.abspath(args.build_dir) if args.build_dir else None
    cached_files = None
    if args.cached:
        cached_files_path = os.path.join(build_dir, 'cached_files.txt')
        if os.path.exists(cached_files_path):
            with open(cached_files_path, 'r') as file_list:
                cached_files = file_list.read().splitlines()
    else:
        cached_files = None

    data_dir = None
    if args.data_dir:
      if not os.path.exists(args.data_dir):
        print(f'Resource directory {args.data_dir} not found!')
        return
      data_dir = os.path.abspath(args.data_dir)

    elf_path = args.elf_path if args.elf_path else None
    unstripped_elf_path = args.elf_path if args.elf_path else None
    if args.build_dir:
        app_dir = None
        elf_name = None
        if args.app:
            elf_name = args.app
            app_dir = os.path.join(build_dir, 'apps', args.app)
        elif args.example:
            elf_name = args.example
            app_dir = os.path.join(build_dir, 'examples', args.example)
        elf_path = os.path.join(app_dir, (
            args.subapp if args.subapp else elf_name) + '.stripped') if elf_path is None else elf_path
        unstripped_elf_path = os.path.join(app_dir, (
            args.subapp if args.subapp else elf_name)) if unstripped_elf_path is None else unstripped_elf_path

    print('Finding all necessary files')
    elfloader_path = args.elfloader_path if args.elfloader_path else FindElfloader(build_dir, cached_files)
    elfloader_elf_path = os.path.join(os.path.dirname(elfloader_path), 'ELFLoader')

    blhost_path = os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', platform_dir, 'blhost' + exe_extension)
    flashloader_path = os.path.join(root_dir, 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin')
    elftosb_path = os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', platform_dir, 'elftosb' + exe_extension)
    toolchain_path = args.toolchain if args.toolchain else os.path.join(root_dir, 'third_party', toolchain_dir, 'gcc-arm-none-eabi-9-2020-q2-update', 'bin')
    paths_to_check = [
        elf_path,
        unstripped_elf_path,
        elfloader_path,
        elfloader_elf_path,
        blhost_path,
        flashloader_path,
        elftosb_path,
    ]

    if not elfloader_path:
        print('No elfloader found or provided!')
        return

    if args.strip or args.debug:
        paths_to_check.append(toolchain_path)

    if not args.elfloader_path or not args.elf_path:
        paths_to_check.append(build_dir)
        if app_dir:
            paths_to_check.append(app_dir)

    all_paths_exist = True
    for path in paths_to_check:
        if not os.path.exists(path):
            print('%s does not exist' % path)
            all_paths_exist = False
    if not all_paths_exist:
        return

    if args.strip:
        subprocess.check_call([os.path.join(toolchain_path, 'arm-none-eabi-strip'), '-s', elf_path, '-o', elf_path + '.stripped'])
        elf_path = elf_path + '.stripped'

    state_machine_args = {
        'blhost_path': blhost_path,
        'flashloader_path': flashloader_path,
        'ram': args.ram,
        'reset': args.reset,
        'elfloader_path': elfloader_path,
        'elfloader_elf_path': elfloader_elf_path,
        'elf_path': elf_path,
        'unstripped_elf_path': unstripped_elf_path,
        'root_dir': root_dir,
        'usb_ip_address': usb_ip_address,
        'wifi_ssid': args.wifi_ssid,
        'wifi_psk': args.wifi_psk,
        'wifi_country': args.wifi_country,
        'wifi_revision': args.wifi_revision,
        'ethernet_speed': args.ethernet_speed,
        'debug': args.debug,
        'jlink_path': args.jlink_path,
        'toolchain_path': toolchain_path,
        'program': args.program,
        'data': args.data,
    }

    sdp_devices = len(EnumerateSDP())
    flashloader_devices = len(EnumerateFlashloader())
    for _ in range(sdp_devices):
        RunFlashtool(StateCheckForSdp,
                     target_elfloader=True, **state_machine_args)

    for _ in range(flashloader_devices):
        RunFlashtool(StateCheckForFlashloader,
                     target_elfloader=True, **state_machine_args)

    # Sleep to allow time for the last device we threw into elfloader
    # to enumerate.
    time.sleep(1.0)

    serial_list = []
    for elfloader in EnumerateElfloader():
        serial_list.append(elfloader['serial_number'])

    serial_list += EnumerateCoralMicro()

    if args.list:
        print(serial_list)
        return

    serial_number = os.getenv('CORAL_MICRO_SERIAL')
    if not serial_number:
        serial_number = args.serial
    if len(serial_list) > 1 and not serial_number:
        print('Multiple Dev Board Micros detected, please provide a serial number.')
        return
    if not serial_number and len(serial_list) == 1:
        serial_number = serial_list[0]
    if not serial_number:
        print('No Dev Board Micro devices detected!')
        return

    with tempfile.TemporaryDirectory() as workdir:
        sbfile_path = None
        data_files = None
        if not args.ram:
            print('Creating Filesystem')
            data_files = CreateFilesystem(workdir, root_dir, build_dir, elf_path, cached_files, args.arduino, data_dir, args.data)
            if data_files is None:
                print('Creating filesystem failed, exit')
                return
            sbfile_path = CreateSbFile(workdir, elftosb_path, elfloader_path, args.program and args.data)
            if not sbfile_path:
                print('Creating sbfile failed, exit')
                return

        RunFlashtool(StateCheckForAny, sbfile_path=sbfile_path,
                     data_files=data_files, serial_number=serial_number,
                     **state_machine_args)

if __name__ == '__main__':
    main()
