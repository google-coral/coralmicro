#!/usr/bin/python3
from enum import Enum, auto
from pathlib import Path
from progress.bar import Bar
import argparse
import hexformat
import hid
import ipaddress
import os
import serial
import signal
import struct
import subprocess
import sys
import tempfile
import time
import usb.core

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

VALIANT_UART_PATH = '/dev/valiant_UART'
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

def sdp_vidpid():
    return '{},{}'.format(hex(SDP_VID), hex(SDP_PID))

def flashloader_vidpid():
    return '{},{}'.format(hex(FLASHLOADER_VID), hex(FLASHLOADER_PID))

def is_valiant_connected(serial_number):
    for device in usb.core.find(find_all=True):
        if device.idVendor == VALIANT_VID and device.idProduct == VALIANT_PID:
            if not serial_number or device.serial_number == serial_number:
                return True
    return False

def is_elfloader_connected(serial_number):
    for device in usb.core.find(find_all=True):
        if device.idVendor == ELFLOADER_VID and device.idProduct == ELFLOADER_PID:
            if not serial_number or device.serial_number == serial_number:
                return True
    return False

def is_sdp_connected():
    for device in usb.core.find(find_all=True):
        if device.idVendor == SDP_VID and device.idProduct == SDP_PID:
            return True
    return False

def is_flashloader_connected():
    for device in usb.core.find(find_all=True):
        if device.idVendor == FLASHLOADER_VID and device.idProduct == FLASHLOADER_PID:
            return True
    return False

def EnumerateSDP():
    return hid.enumerate(SDP_VID, SDP_PID)

def EnumerateFlashloader():
    return hid.enumerate(FLASHLOADER_VID, FLASHLOADER_PID)

def EnumerateElfloader():
    return hid.enumerate(ELFLOADER_VID, ELFLOADER_PID)

def EnumerateValiant():
    return usb.core.find(find_all=True, idVendor=VALIANT_VID, idProduct=VALIANT_PID)

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
                        if (file == lib + '.libs'):
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
        spinand_bdfile.write('sources {\n')
        spinand_bdfile.write('bootImageFile = extern (0);\n')
        spinand_bdfile.write('}\n')
        spinand_bdfile.write('section (0) {\n')
        spinand_bdfile.write('load 0xC2000105 > 0x10000;\n')
        spinand_bdfile.write('load 0x00010020 > 0x10004;\n')
        spinand_bdfile.write('load 0x00040004 > 0x10008;\n')
        spinand_bdfile.write('load 0x00080004 > 0x1000C;\n')
        spinand_bdfile.write('load 0xC0010021 > 0x10020;\n')
        spinand_bdfile.write('enable spinand 0x10000;\n')
        spinand_bdfile.write('erase spinand 0x4..0x8;\n')
        spinand_bdfile.write('erase spinand 0x8..0xc;\n')
        spinand_bdfile.write('erase spinand 0xc..0x4c;\n')
        spinand_bdfile.write('load spinand bootImageFile > 0x4;\n')
        spinand_bdfile.write('load spinand bootImageFile > 0x8;\n')
        spinand_bdfile.write('}\n')
    with open(itcm_bdfile_path, 'w') as itcm_bdfile:
        itcm_bdfile.write('options {\n')
        itcm_bdfile.write('flags = 0x00;\n')
        itcm_bdfile.write('startAddress = ' + hex(itcm_startaddress) + ';\n')
        itcm_bdfile.write('ivtOffset = 0x400;\n')
        itcm_bdfile.write('initialLoadSize = 0x1000;\n')
        itcm_bdfile.write('}\n')
        itcm_bdfile.write('sources {\n')
        itcm_bdfile.write('elfFile = extern(0);\n')
        itcm_bdfile.write('}\n')
        itcm_bdfile.write('section (0) {\n')
        itcm_bdfile.write('}\n')
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

def FlashtoolError(**kwargs):
    return FlashtoolStates.DONE

def CheckForAny(**kwargs):
    if is_valiant_connected(kwargs.get('serial', None)):
        return FlashtoolStates.CHECK_FOR_VALIANT
    if is_elfloader_connected(kwargs.get('serial', None)):
        return FlashtoolStates.CHECK_FOR_ELFLOADER
    if is_sdp_connected():
        return FlashtoolStates.CHECK_FOR_SDP
    if is_flashloader_connected():
        return FlashtoolStates.CHECK_FOR_FLASHLOADER
    return FlashtoolStates.ERROR

def CheckForValiant(**kwargs):
    if is_valiant_connected(kwargs.get('serial', None)):
        return FlashtoolStates.RESET_TO_SDP
    # If we don't see Valiant on the bus, just check for SDP.
    return FlashtoolStates.CHECK_FOR_SDP

def CheckForElfloader(**kwargs):
    if is_elfloader_connected(kwargs.get('serial', None)):
        return FlashtoolStates.RESET_ELFLOADER
    return FlashtoolStates.ERROR

def ResetToSdp(**kwargs):
    try:
        s = serial.Serial(VALIANT_UART_PATH, baudrate=1200)
        s.dtr = False
        s.close()
        return FlashtoolStates.CHECK_FOR_SDP
    except Exception:
        print('Unable to open %s' % VALIANT_UART_PATH)
        return FlashtoolStates.ERROR

def CheckForSdp(**kwargs):
    for i in range(10):
        if is_sdp_connected():
            return FlashtoolStates.LOAD_FLASHLOADER
        time.sleep(1)
    return FlashtoolStates.ERROR

def LoadFlashloader(**kwargs):
    subprocess.check_call([kwargs['blhost_path'], '-u', sdp_vidpid(), '--', 'load-image', kwargs['flashloader_path']], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return FlashtoolStates.CHECK_FOR_FLASHLOADER

def LoadElfloader(**kwargs):
    start_address = hexformat.srecord.SRecord.fromsrecfile(kwargs['elfloader_path']).startaddress
    subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'flash-image', kwargs['elfloader_path']], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'call', hex(start_address), '0'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    if kwargs.get('target_elfloader', None):
        return FlashtoolStates.DONE
    if kwargs.get('ram'):
        return FlashtoolStates.PROGRAM_ELFLOADER
    return FlashtoolStates.PROGRAM_DATA_FILES

def CheckForFlashloader(**kwargs):
    for i in range(10):
        if is_flashloader_connected():
            if kwargs['ram'] or kwargs.get('target_elfloader', False):
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


def ResetElfloader(**kwargs):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, kwargs.get('serial', None))
    h.write(struct.pack(ELFLOADER_CMD_HEADER, 0, ELFLOADER_RESET))
    h.close()
    return FlashtoolStates.CHECK_FOR_SDP

def Program(**kwargs):
    subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'receive-sb-file', kwargs['sbfile_path']])
    return FlashtoolStates.LOAD_ELFLOADER

def ElfloaderTransferData(h, data, target, bar=None):
    warned = False
    def read_byte():
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

def ProgramElfloader(**kwargs):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, kwargs.get('serial', None))
    with open(kwargs['elf_path'], 'rb') as elf:
        ElfloaderTransferData(h, elf.read(), ELFLOADER_TARGET_RAM)
    h.close()
    if not kwargs.get('debug', False):
        return FlashtoolStates.DONE
    return FlashtoolStates.START_GDB

def ProgramDataFiles(**kwargs):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, kwargs.get('serial', None))
    data_files = kwargs.get('data_files')
    data_files.append((kwargs.get('elf_path'), '/default.elf'))
    data_files.append((USB_IP_ADDRESS_FILE, USB_IP_ADDRESS_FILE))
    if kwargs['wifi_ssid'] is not None:
        data_files.append((WIFI_SSID_FILE, WIFI_SSID_FILE))
    if kwargs['wifi_psk'] is not None:
        data_files.append((WIFI_PSK_FILE, WIFI_PSK_FILE))
    data_files = sorted(data_files, key=lambda x: x[1])
    for (src_file, target_file) in data_files:
        if target_file[0] != '/':
            target_file = '/' + target_file
        # If we are running on something with the wrong directory separator, fix it.
        target_file.replace('\\', '/')

        def write_file(f):
            bar = Bar(target_file, max=os.fstat(f.fileno()).st_size)
            ElfloaderTransferData(h, bytes(target_file, encoding='utf-8'), ELFLOADER_TARGET_PATH)
            ElfloaderTransferData(h, f.read(), ELFLOADER_TARGET_FILESYSTEM, bar=bar)

        if src_file is USB_IP_ADDRESS_FILE:
            with tempfile.NamedTemporaryFile() as f:
                f.write(str(kwargs['usb_ip_address']).encode())
                f.flush()
                f.seek(0)
                write_file(f)
                continue
        if src_file is WIFI_SSID_FILE and kwargs['wifi_ssid'] is not None:
            with tempfile.NamedTemporaryFile() as f:
                f.write(str(kwargs['wifi_ssid']).encode())
                f.flush()
                f.seek(0)
                write_file(f)
                continue
        if src_file is WIFI_PSK_FILE and kwargs['wifi_psk'] is not None:
            with tempfile.NamedTemporaryFile() as f:
                f.write(str(kwargs['wifi_psk']).encode())
                f.flush()
                f.seek(0)
                write_file(f)
                continue

        with open(src_file, 'rb') as f:
            write_file(f)
    h.close()
    return FlashtoolStates.RESET

def Reset(**kwargs):
    h = OpenHidDevice(ELFLOADER_VID, ELFLOADER_PID, kwargs.get('serial', None))
    h.write(struct.pack(ELFLOADER_CMD_HEADER, 0, ELFLOADER_RESET))
    h.close()
    return FlashtoolStates.DONE

def StartGdb(**kwargs):
    jlink_gdbserver = os.path.join(kwargs.get('jlink_path'), 'JLinkGDBServerCLExe')
    gdb_exe = os.path.join(kwargs.get('toolchain_path'), 'arm-none-eabi-gdb')
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
                'file ' + kwargs.get('unstripped_elf_path') + '\n',
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
    app_elf_group.add_argument('--elf_path', type=str)

    args = parser.parse_args()

    usb_ip_address = ipaddress.ip_address(args.usb_ip_address)

    build_dir = os.path.abspath(args.build_dir) if args.build_dir else None
    if args.cached:
        cached_files_path = os.path.join(build_dir, 'cached_files.txt')
        if os.path.exists(cached_files_path):
            with open(cached_files_path, 'r') as file_list:
                cached_files = file_list.read().splitlines()
    else:
        cached_files = None
    app_dir = os.path.join(build_dir, 'apps', args.app) if args.build_dir and args.app else None
    elf_path = args.elf_path if args.elf_path else os.path.join(app_dir, (args.subapp if args.subapp else args.app) + '.stripped')
    unstripped_elf_path = args.elf_path if args.elf_path else os.path.join(app_dir, (args.subapp if args.subapp else args.app))
    print('Finding all necessary files')
    elfloader_path = args.elfloader_path if args.elfloader_path else FindElfloader(build_dir, cached_files)

    if os.name == 'nt':
      blhost_path = os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', 'win', 'blhost.exe')
    else:
      blhost_path = os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', 'linux', 'amd64', 'blhost')

    flashloader_path = os.path.join(root_dir, 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin')

    if os.name == 'nt':
      elftosb_path = os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', 'win', 'elftosb.exe')
    else:
      elftosb_path = os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', 'linux', 'amd64', 'elftosb')
    toolchain_path = args.toolchain if args.toolchain else os.path.join(root_dir, 'third_party', 'toolchain', 'gcc-arm-none-eabi-9-2020-q2-update', 'bin')
    paths_to_check = [
        elf_path,
        unstripped_elf_path,
        elfloader_path,
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
        state = FlashtoolStates.CHECK_FOR_SDP
        while True:
            state = state_handlers[state](
                    target_elfloader=True,
                    **state_machine_args,
                    )
            if state is FlashtoolStates.DONE:
                break
    for _ in range(flashloader_devices):
        state = FlashtoolStates.CHECK_FOR_FLASHLOADER
        while True:
            state = state_handlers[state](
                    target_elfloader=True,
                    **state_machine_args,
                    )
            if state is FlashtoolStates.DONE:
                break

    # Sleep to allow time for the last device we threw into elfloader
    # to enumerate.
    time.sleep(1.0)

    serial_list = []
    for elfloader in EnumerateElfloader():
        serial_list.append(elfloader['serial_number'])
    for valiant in EnumerateValiant():
        serial_list.append(valiant.serial_number)

    if args.list:
        print(serial_list)
        return

    serial = os.getenv('VALIANT_SERIAL')
    if not serial:
        serial = args.serial
    if len(serial_list) > 1 and not serial:
        print('Multiple valiants detected, please provide a serial number.')
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
        state = FlashtoolStates.CHECK_FOR_ANY
        while True:
            print(state)
            state = state_handlers[state](
                    sbfile_path=sbfile_path,
                    data_files=data_files,
                    serial=serial,
                    **state_machine_args,
                    )
            if state is FlashtoolStates.DONE:
                break

if __name__ == '__main__':
    main()
