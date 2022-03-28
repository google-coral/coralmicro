#!/usr/bin/python3
from enum import Enum, auto
from pathlib import Path
from progress.bar import Bar
import argparse
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
ELFLOADER_PID = 0x93fe

VALIANT_VID = 0x18d1
VALIANT_PID = 0x93ff

OPEN_HID_RETRY_INTERVAL_S = 0.1
OPEN_HID_RETRY_TIME_S = 15

BLOCK_SIZE = 2048 * 64
BLOCK_COUNT = 64

USB_IP_ADDRESS_FILE = '/usb_ip_address'
WIFI_SSID_FILE = '/wifi_ssid'
WIFI_PSK_FILE = '/wifi_psk'

ELFLOADER_SETSIZE = 0
ELFLOADER_BYTES = 1
ELFLOADER_DONE = 2
ELFLOADER_RESET = 3
ELFLOADER_TARGET = 4

ELFLOADER_TARGET_RAM = 0
ELFLOADER_TARGET_PATH = 1
ELFLOADER_TARGET_FILESYSTEM = 2

ELFLOADER_CMD_HEADER = '=BB'

def read_file(path):
    with open(path, 'rb') as f:
        return f.read()

def sdp_vidpid():
    return '{},{}'.format(hex(SDP_VID), hex(SDP_PID))

def flashloader_vidpid():
    return '{},{}'.format(hex(FLASHLOADER_VID), hex(FLASHLOADER_PID))

def is_valiant_connected(serial_number):
    return serial_number in EnumerateValiant()

def is_elfloader_connected(serial_number):
    for device in usb.core.find(idVendor=ELFLOADER_VID, idProduct=ELFLOADER_PID,
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

SERIAL_PORT_RE = re.compile('USB VID:PID=18D1:93FF SER=([0-9A-Fa-f]+)')
def EnumerateValiant():
    serial_list = []
    for port in serial.tools.list_ports.comports():
        matches = SERIAL_PORT_RE.match(port.hwid)
        if matches:
            serial_list.append(matches.group(1).lower())
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

def CreateFilesystem(workdir, root_dir, build_dir, elf_path, cached_files):
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


    data_files = list(data_files)
    absolute_file_paths = list()
    all_files_exist = True
    for file in data_files:
        path = os.path.join(build_dir, file)
        if not os.path.exists(path):
            print('%s does not exist!' % path)
            all_files_exist = False
        absolute_file_paths.append(path)
    if not all_files_exist:
        return None

    return list(zip(absolute_file_paths, data_files))

def CreateSbFile(workdir, elftosb_path, srec_path):
    spinand_bdfile_path = os.path.join(workdir, 'program_flexspinand_image.bd')
    itcm_bdfile_path = os.path.join(workdir, 'imx-itcm-unsigned.bd')

    srec_obj = hexformat.srecord.SRecord.fromsrecfile(srec_path)
    assert len(srec_obj.parts()) == 1
    itcm_startaddress = srec_obj.parts()[0][0] & 0xFFFF0000

    with open(spinand_bdfile_path, 'w') as spinand_bdfile:
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
  erase spinand 0xc..0x4c;
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

class FlashtoolStates(Enum):
    DONE = auto()
    ERROR = auto()
    CHECK_FOR_ANY = auto()
    CHECK_FOR_ELFLOADER = auto()
    CHECK_FOR_VALIANT = auto()
    RESET_TO_SDP = auto()
    CHECK_FOR_SDP = auto()
    LOAD_FLASHLOADER = auto()
    LOAD_ELFLOADER = auto()
    RESET_ELFLOADER = auto()
    CHECK_FOR_FLASHLOADER = auto()
    PROGRAM = auto()
    PROGRAM_ELFLOADER = auto()
    PROGRAM_DATA_FILES = auto()
    RESET = auto()
    START_GDB = auto()

def FlashtoolError():
    return FlashtoolStates.DONE

def CheckForAny(serial_number=None):
    for i in range(10):
        if is_valiant_connected(serial_number):
            return FlashtoolStates.CHECK_FOR_VALIANT
        if is_elfloader_connected(serial_number):
            return FlashtoolStates.CHECK_FOR_ELFLOADER
        if is_sdp_connected():
            return FlashtoolStates.CHECK_FOR_SDP
        if is_flashloader_connected():
            return FlashtoolStates.CHECK_FOR_FLASHLOADER
        time.sleep(1)
    return FlashtoolStates.ERROR

def CheckForValiant(serial_number=None):
    for i in range(10):
        if is_valiant_connected(serial_number):
            # port is needed later as well, wait for it.
            port = FindSerialPortForDevice(serial_number)
            if port:
                return FlashtoolStates.RESET_TO_SDP
        time.sleep(1)
    # If we don't see Valiant on the bus, just check for SDP.
    return FlashtoolStates.CHECK_FOR_SDP

def CheckForElfloader(serial_number=None):
    for i in range(10):
        if is_elfloader_connected(serial_number):
            return FlashtoolStates.RESET_ELFLOADER
        time.sleep(1)
    return FlashtoolStates.ERROR

def FindSerialPortForDevice(serial_number=None):
    for port in serial.tools.list_ports.comports():
        try:
            if port.serial_number and port.serial_number.lower() == serial_number:
                return port.device
        except ValueError:
            pass
    return None

def ResetToSdp(serial_number=None):
    port = FindSerialPortForDevice(serial_number)
    if port is None:
        print('serial port not found')
        return FlashtoolStates.ERROR

    # Port could be enumerated at this point but udev rules are still to get applied.
    # Typically this results in exceptions like "[Errno 13] Permission denied: '/dev/ttyACM0'"
    for i in range(10):
        with serial.Serial(baudrate=1200) as s:
            try:
                s.port = port
                s.dtr = False
                s.open()
                s.write(b'42')  ## Dummy write to force apply s.dtr.
                return FlashtoolStates.CHECK_FOR_SDP
            except serial.SerialException as e:
                # If the port goes away, we get SerialException.
                # Assume that this means the device reset into SDP, and try to proceed.
                return FlashtoolStates.CHECK_FOR_SDP
            except:
                pass
        time.sleep(1)
    print('Unable to open %s' % port)
    return FlashtoolStates.ERROR

def CheckForSdp():
    for i in range(10):
        if is_sdp_connected():
            return FlashtoolStates.LOAD_FLASHLOADER
        time.sleep(1)
    return FlashtoolStates.ERROR

def LoadFlashloader(blhost_path, flashloader_path):
    subprocess.check_call([blhost_path, '-u', sdp_vidpid(), '--', 'load-image', flashloader_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return FlashtoolStates.CHECK_FOR_FLASHLOADER

def LoadElfloader(toolchain_path, elfloader_path, elfloader_elf_path, blhost_path, ram, target_elfloader=False):
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
        return FlashtoolStates.DONE
    if ram:
        return FlashtoolStates.PROGRAM_ELFLOADER
    return FlashtoolStates.PROGRAM_DATA_FILES

def CheckForFlashloader(ram, target_elfloader=False):
    for i in range(10):
        if is_flashloader_connected():
            if ram or target_elfloader:
                return FlashtoolStates.LOAD_ELFLOADER
            else:
                return FlashtoolStates.PROGRAM
        time.sleep(1)
    return FlashtoolStates.ERROR


def OpenHidDevice(vid, pid, serial_number):
    h = hid.device()
    # First few tries to open the HID device can fail,
    # if Python is faster than the device. So retry a bit.
    opened = False
    print('OpenHidDevice vid={:x} pid={:x} ...'.format(vid, pid))
    for _ in range(round(OPEN_HID_RETRY_TIME_S / OPEN_HID_RETRY_INTERVAL_S)):
        try:
            h.open(vid, pid, serial_number=serial_number)
            print('OpenHidDevice vid={:x} pid={:x} opened'.format(vid, pid, serial_number))
            opened = True
            break
        except:
            time.sleep(OPEN_HID_RETRY_INTERVAL_S)
            pass
    if not opened:
        raise Exception('Failed to open Valiant HID device')

    return h


def ResetElfloader(serial_number=None):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number)
    h.write(struct.pack(ELFLOADER_CMD_HEADER, 0, ELFLOADER_RESET))
    h.close()
    return FlashtoolStates.CHECK_FOR_SDP

def Program(blhost_path, sbfile_path):
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), 'receive-sb-file', sbfile_path])
    return FlashtoolStates.LOAD_ELFLOADER

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
    h.write(struct.pack(ELFLOADER_CMD_HEADER + 'B', 0, ELFLOADER_TARGET, target))
    read_byte()

    h.write(struct.pack(ELFLOADER_CMD_HEADER + 'l', 0, ELFLOADER_SETSIZE, total_bytes))
    read_byte()

    data_packet_header = ELFLOADER_CMD_HEADER + 'll'
    bytes_per_packet = 64 - struct.calcsize(data_packet_header) + 1 # 64 bytes HID packet, adjust for header and padding
    bytes_transferred = 0
    while bytes_transferred < total_bytes:
        bytes_this_packet = min(bytes_per_packet, (total_bytes - bytes_transferred))
        h.write(
            struct.pack((data_packet_header + '%ds') % bytes_this_packet,
            0, ELFLOADER_BYTES, bytes_this_packet, bytes_transferred,
            data[bytes_transferred:bytes_transferred+bytes_this_packet]))
        read_byte()
        bytes_transferred += bytes_this_packet
        if bar:
            bar.goto(bytes_transferred)
    h.write(struct.pack(ELFLOADER_CMD_HEADER, 0, ELFLOADER_DONE))
    read_byte()
    if bar:
        bar.finish()
    return FlashtoolStates.RESET

def ProgramElfloader(elf_path, debug=False, serial_number=None):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number)
    data = read_file(elf_path)
    ElfloaderTransferData(h, data, ELFLOADER_TARGET_RAM, bar=Bar(elf_path, max=len(data)))
    h.close()
    if not debug:
        return FlashtoolStates.DONE
    return FlashtoolStates.START_GDB

def ProgramDataFiles(elf_path, data_files, usb_ip_address, wifi_ssid=None, wifi_psk=None, serial_number=None):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number)
    data_files.append((elf_path, '/default.elf'))
    data_files.append((str(usb_ip_address).encode(), USB_IP_ADDRESS_FILE))
    if wifi_ssid is not None:
        data_files.append((wifi_ssid.encode(), WIFI_SSID_FILE))
    if wifi_psk is not None:
        data_files.append((wifi_psk.encode(), WIFI_PSK_FILE))
    for src_file, target_file in sorted(data_files, key=lambda x: x[1]):
        if target_file[0] != '/':
            target_file = '/' + target_file
        # If we are running on something with the wrong directory separator, fix it.
        target_file.replace('\\', '/')

        if isinstance(src_file, str):
            data = read_file(src_file)
        elif isinstance(src_file, bytes):
            data = src_file
        else:
            raise RuntimeError('src_file must be "str" or "bytes"')

        ElfloaderTransferData(h, target_file.encode(), ELFLOADER_TARGET_PATH)
        ElfloaderTransferData(h, data, ELFLOADER_TARGET_FILESYSTEM,
                              bar=Bar(target_file, max=len(data)))

    h.close()
    return FlashtoolStates.RESET

def Reset(serial_number=None):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, serial_number)
    h.write(struct.pack(ELFLOADER_CMD_HEADER, 0, ELFLOADER_RESET))
    h.close()
    return FlashtoolStates.DONE

def StartGdb(jlink_path, toolchain_path, unstripped_elf_path):
    jlink_gdbserver = os.path.join(jlink_path, 'JLinkGDBServerCLExe')
    gdb_exe = os.path.join(toolchain_path, 'arm-none-eabi-gdb')
    if not os.path.exists(jlink_gdbserver):
        print(jlink_gdbserver + ' does not exist!')
        return FlashtoolStates.ERROR
    if not os.path.exists(gdb_exe):
        print(gdb_exe + ' does not exist!')
        return FlashtoolStates.ERROR

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
    return FlashtoolStates.DONE

state_handlers = {
    FlashtoolStates.ERROR: FlashtoolError,
    FlashtoolStates.CHECK_FOR_ANY: CheckForAny,
    FlashtoolStates.CHECK_FOR_ELFLOADER: CheckForElfloader,
    FlashtoolStates.CHECK_FOR_VALIANT: CheckForValiant,
    FlashtoolStates.RESET_TO_SDP: ResetToSdp,
    FlashtoolStates.CHECK_FOR_SDP: CheckForSdp,
    FlashtoolStates.LOAD_FLASHLOADER: LoadFlashloader,
    FlashtoolStates.CHECK_FOR_FLASHLOADER: CheckForFlashloader,
    FlashtoolStates.LOAD_ELFLOADER: LoadElfloader,
    FlashtoolStates.RESET_ELFLOADER: ResetElfloader,
    FlashtoolStates.PROGRAM: Program,
    FlashtoolStates.PROGRAM_ELFLOADER: ProgramElfloader,
    FlashtoolStates.PROGRAM_DATA_FILES: ProgramDataFiles,
    FlashtoolStates.RESET: Reset,
    FlashtoolStates.START_GDB: StartGdb,
}

def RunFlashtool(initial_state, **kwargs):
    state = initial_state
    while True:
        print(state)
        handler = state_handlers[state]
        params = inspect.signature(handler).parameters.values()
        handler_kwargs =  {param.name : kwargs.get(param.name, param.default) for param in params}
        state = handler(**handler_kwargs)
        if state is FlashtoolStates.DONE:
            print(state)
            break

def main():
    # Check if we're running from inside a pyinstaller binary.
    # The true branch is pyinstaller, false branch is executing directly.
    if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
        root_dir = os.path.abspath(os.path.dirname(__file__))
    else:
        root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    parser = argparse.ArgumentParser(description='Valiant flashtool',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--build_dir', type=str, required=True)
    parser.add_argument('--subapp', type=str, required=False)
    parser.add_argument('--ram', dest='ram', action='store_true')
    parser.add_argument('--noram', dest='ram', action='store_false')
    parser.add_argument('--elfloader_path', type=str, required=False)
    parser.add_argument('--serial', type=str, required=False)
    parser.add_argument('--list', dest='list', action='store_true')
    parser.add_argument('--usb_ip_address', type=str, required=False, default='10.10.10.1')
    parser.add_argument('--wifi_ssid', type=str, required=False, default=None)
    parser.add_argument('--wifi_psk', type=str, required=False, default=None)
    # --strip and --toolchain are provided for Arduino uses.
    # CMake-built targets already generate a stripped binary.
    parser.add_argument('--strip', dest='strip', action='store_true')
    parser.add_argument('--toolchain', type=str, required=False)
    parser.add_argument('--debug', dest='debug', action='store_true')
    parser.add_argument('--jlink_path', type=str, default='/opt/SEGGER/JLink')
    parser.add_argument('--cached', dest='cached', action='store_true')
    parser.set_defaults(list=False)
    parser.set_defaults(ram=False)
    parser.set_defaults(strip=False)
    parser.set_defaults(debug=False)

    app_elf_group = parser.add_mutually_exclusive_group(required=True)
    app_elf_group.add_argument('--app', type=str)
    app_elf_group.add_argument('--example', type=str)
    app_elf_group.add_argument('--elf_path', type=str)

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

    if os.name == 'nt':
      blhost_path = os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', 'win', 'blhost.exe')
    else:
      blhost_path = os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', platform_dir, 'blhost')

    flashloader_path = os.path.join(root_dir, 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin')

    if os.name == 'nt':
      elftosb_path = os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', 'win', 'elftosb.exe')
    else:
      elftosb_path = os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', platform_dir, 'elftosb')
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
        'elfloader_path': elfloader_path,
        'elfloader_elf_path': elfloader_elf_path,
        'elf_path': elf_path,
        'unstripped_elf_path': unstripped_elf_path,
        'root_dir': root_dir,
        'usb_ip_address': usb_ip_address,
        'wifi_ssid': args.wifi_ssid,
        'wifi_psk': args.wifi_psk,
        'debug': args.debug,
        'jlink_path': args.jlink_path,
        'toolchain_path': toolchain_path,
    }

    sdp_devices = len(EnumerateSDP())
    flashloader_devices = len(EnumerateFlashloader())
    for _ in range(sdp_devices):
        RunFlashtool(FlashtoolStates.CHECK_FOR_SDP,
                     target_elfloader=True, **state_machine_args)

    for _ in range(flashloader_devices):
        RunFlashtool(FlashtoolStates.CHECK_FOR_FLASHLOADER,
                     target_elfloader=True, **state_machine_args)

    # Sleep to allow time for the last device we threw into elfloader
    # to enumerate.
    time.sleep(1.0)

    serial_list = []
    for elfloader in EnumerateElfloader():
        serial_list.append(elfloader['serial_number'])

    serial_list += EnumerateValiant()

    if args.list:
        print(serial_list)
        return

    serial_number = os.getenv('VALIANT_SERIAL')
    if not serial_number:
        serial_number = args.serial
    if len(serial_list) > 1 and not serial_number:
        print('Multiple valiants detected, please provide a serial number.')
        return
    if not serial_number and len(serial_list) == 1:
        serial_number = serial_list[0]
    if not serial_number:
        print('No Valiant devices detected!')
        return

    with tempfile.TemporaryDirectory() as workdir:
        sbfile_path = None
        data_files = None
        if not args.ram:
            print('Creating Filesystem')
            data_files = CreateFilesystem(workdir, root_dir, build_dir, elf_path, cached_files)
            if data_files is None:
                print('Creating filesystem failed, exit')
                return
            sbfile_path = CreateSbFile(workdir, elftosb_path, elfloader_path)
            if not sbfile_path:
                print('Creating sbfile failed, exit')
                return

        RunFlashtool(FlashtoolStates.CHECK_FOR_ANY, sbfile_path=sbfile_path,
                     data_files=data_files, serial_number=serial_number,
                     **state_machine_args)

if __name__ == '__main__':
    main()
