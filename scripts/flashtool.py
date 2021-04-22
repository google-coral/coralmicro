#!/usr/bin/python3
from enum import Enum, auto
from pathlib import Path
import argparse
import hexformat
import hid
import os
import serial
import shutil
import struct
import subprocess
import tempfile
import time
import usb.core

SDP_VID = 0x1fc9
SDP_PID = 0x013d

FLASHLOADER_VID = 0x15a2
FLASHLOADER_PID = 0x0073

VALIANT_VID = 0x18d1
VALIANT_PID = 0x93ff

VALIANT_UART_PATH = '/dev/valiant_UART'
BLOCK_SIZE = 2048 * 64
BLOCK_COUNT = 64

ELFLOADER_SETSIZE = 0
ELFLOADER_BYTES = 1
ELFLOADER_DONE = 2

# Key: path in Valiant source tree
# Value: path in on-device filesystem
FILESYSTEM_FILES = {
    os.path.join('third_party', 'firmware', 'cypress', '43455C0.bin'): os.path.join('firmware', '43455C0.bin'),
    os.path.join('third_party', 'firmware', 'cypress', '43455C0.clm_blob'): os.path.join('firmware', '43455C0.clm_blob'),
    os.path.join('third_party', 'cyw-bt-patch', 'BCM4345C0_003.001.025.0144.0266.1MW.hcd'): os.path.join('firmware', 'BCM4345C0_003.001.025.0144.0266.1MW.hcd'),
    os.path.join('models', 'person_detect_model.tflite'): os.path.join('models', 'person_detect_model.tflite'),
    os.path.join('models', 'testconv1-edgetpu.tflite'): os.path.join('models', 'testconv1-edgetpu.tflite'),
    os.path.join('models', 'testconv1-expected-output.bin'): os.path.join('models', 'testconv1-expected-output.bin'),
    os.path.join('models', 'testconv1-test-input.bin'): os.path.join('models', 'testconv1-test-input.bin'),
    os.path.join('models', 'posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu.tflite'): os.path.join('models', 'posenet_mobilenet_v1_075_353_481_quant_decoder_edgetpu.tflite'),
    os.path.join('models', 'posenet_test_input.bin'): os.path.join('models', 'posenet_test_input.bin'),
}

def sdp_vidpid():
    return '{},{}'.format(hex(SDP_VID), hex(SDP_PID))

def flashloader_vidpid():
    return '{},{}'.format(hex(FLASHLOADER_VID), hex(FLASHLOADER_PID))

def is_valiant_connected():
    for device in usb.core.find(find_all=True):
        if device.idVendor == VALIANT_VID and device.idProduct == VALIANT_PID:
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

def CreateFilesystem(workdir, root_dir, mklfs_path, files, elf_path):
    filesystem_dir = os.path.join(workdir, 'filesystem')
    filesystem_bin = os.path.join(workdir, 'filesystem.bin')
    all_files_exist = True
    for k, _ in files.items():
        path = os.path.join(root_dir, k)
        if not os.path.exists(path):
            print('%s does not exist!' % path)
            all_files_exist = False
    if not all_files_exist:
        return None

    for k, v in files.items():
        source_path = os.path.join(root_dir, k)
        target_path = os.path.join(filesystem_dir, v)
        os.makedirs(os.path.dirname(target_path), exist_ok=True)
        shutil.copyfile(source_path, target_path)
    shutil.copyfile(elf_path, os.path.join(filesystem_dir, 'default.elf'))

    filesystem_size = sum(file.stat().st_size for file in Path(filesystem_dir).rglob('*'))
    filesystem_size = max(filesystem_size, BLOCK_SIZE * BLOCK_COUNT)
    if (filesystem_size > BLOCK_SIZE * BLOCK_COUNT):
        print('Filesystem too large!')
        return None
    subprocess.check_call([mklfs_path, '-c', 'filesystem', '-b', str(BLOCK_SIZE), '-r', '2048', '-p', '2048', '-s', str(filesystem_size), '-i', filesystem_bin], cwd=workdir)
    return filesystem_bin

def CreateSbFile(workdir, elftosb_path, srec_path, filesystem_path):
    spinand_bdfile_path = os.path.join(workdir, 'program_flexspinand_image.bd')
    itcm_bdfile_path = os.path.join(workdir, 'imx-itcm-unsigned.bd')

    srec_obj = hexformat.srecord.SRecord.fromsrecfile(srec_path)
    assert len(srec_obj.parts()) == 1
    itcm_startaddress = srec_obj.parts()[0][0] & 0xFFFF0000

    with open(spinand_bdfile_path, 'w') as spinand_bdfile:
        spinand_bdfile.write('sources {\n')
        spinand_bdfile.write('bootImageFile = extern (0);\n')
        if filesystem_path:
            spinand_bdfile.write('filesystemFile = extern (1);\n')
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
        if filesystem_path:
            spinand_bdfile.write('erase spinand 0xc..0x4c;\n')
        spinand_bdfile.write('load spinand bootImageFile > 0x4;\n')
        spinand_bdfile.write('load spinand bootImageFile > 0x8;\n')
        if filesystem_path:
            spinand_bdfile.write('load spinand filesystemFile > 0xc;\n')
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
    if filesystem_path:
        args.append(filesystem_path)
    subprocess.check_call(args)
    return sbfile_path

class FlashtoolStates(Enum):
    DONE = auto()
    ERROR = auto()
    CHECK_FOR_ANY = auto()
    CHECK_FOR_VALIANT = auto()
    RESET_TO_SDP = auto()
    CHECK_FOR_SDP = auto()
    LOAD_FLASHLOADER = auto()
    LOAD_ELFLOADER = auto()
    CHECK_FOR_FLASHLOADER = auto()
    PROGRAM = auto()
    PROGRAM_ELFLOADER = auto()
    RESET = auto()

def FlashtoolError(**kwargs):
    return FlashtoolStates.DONE

def CheckForAny(**kwargs):
    if is_valiant_connected():
        return FlashtoolStates.CHECK_FOR_VALIANT
    if is_sdp_connected():
        return FlashtoolStates.CHECK_FOR_SDP
    if is_flashloader_connected():
        return FlashtoolStates.CHECK_FOR_FLASHLOADER
    return FlashtoolStates.ERROR

def CheckForValiant(**kwargs):
    if is_valiant_connected():
        return FlashtoolStates.RESET_TO_SDP
    # If we don't see Valiant on the bus, just check for SDP.
    return FlashtoolStates.CHECK_FOR_SDP

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
    subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'flash-image', kwargs['elfloader_path']])
    subprocess.call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'call', hex(start_address), '0'])
    return FlashtoolStates.PROGRAM_ELFLOADER

def CheckForFlashloader(**kwargs):
    for i in range(10):
        if is_flashloader_connected():
            if kwargs['ram']:
                return FlashtoolStates.LOAD_ELFLOADER
            else:
                return FlashtoolStates.PROGRAM
        time.sleep(1)
    return FlashtoolStates.ERROR

def Program(**kwargs):
    subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'receive-sb-file', kwargs['sbfile_path']])
    return FlashtoolStates.RESET

def ProgramElfloader(**kwargs):
    h = hid.device()
    # First few tries to open the HID device can fail,
    # if Python is faster than the device. So retry a bit.
    opened = False
    for _ in range(100):
        try:
            h.open(VALIANT_VID, VALIANT_PID)
            opened = True
            break
        except:
            pass
    if not opened:
        print('Failed to open Valiant HID device')
        return FlashtoolStates.ERROR
    with open(kwargs['elf_path'], 'rb') as elf:
        elf_data = elf.read()
        total_bytes = len(elf_data)
        h.write(struct.pack('=BBl', 0, ELFLOADER_SETSIZE, total_bytes))

        data_packet_header = '=BBll'
        bytes_per_packet = 64 - struct.calcsize(data_packet_header) + 1 # 64 bytes HID packet, adjust for header and padding
        bytes_transferred = 0
        while bytes_transferred < total_bytes:
            bytes_this_packet = min(bytes_per_packet, (total_bytes - bytes_transferred))
            h.write(
                struct.pack((data_packet_header + '%ds') % bytes_this_packet,
                0, ELFLOADER_BYTES, bytes_this_packet, bytes_transferred,
                elf_data[bytes_transferred:bytes_transferred+bytes_this_packet]))
            bytes_transferred += bytes_this_packet
        h.write(struct.pack('=BB', 0, ELFLOADER_DONE))
    h.close()
    return FlashtoolStates.DONE

def Reset(**kwargs):
    subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'reset'])
    return FlashtoolStates.DONE

state_handlers = {
    FlashtoolStates.ERROR: FlashtoolError,
    FlashtoolStates.CHECK_FOR_ANY: CheckForAny,
    FlashtoolStates.CHECK_FOR_VALIANT: CheckForValiant,
    FlashtoolStates.RESET_TO_SDP: ResetToSdp,
    FlashtoolStates.CHECK_FOR_SDP: CheckForSdp,
    FlashtoolStates.LOAD_FLASHLOADER: LoadFlashloader,
    FlashtoolStates.CHECK_FOR_FLASHLOADER: CheckForFlashloader,
    FlashtoolStates.LOAD_ELFLOADER: LoadElfloader,
    FlashtoolStates.PROGRAM: Program,
    FlashtoolStates.PROGRAM_ELFLOADER: ProgramElfloader,
    FlashtoolStates.RESET: Reset,
}

def main():
    parser = argparse.ArgumentParser(description='Valiant flashtool',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--build_dir', type=str, required=True)
    parser.add_argument('--app', type=str, required=True)
    parser.add_argument('--ram', dest='ram', action='store_true')
    parser.add_argument('--noram', dest='ram', action='store_false')
    parser.set_defaults(ram=False)
    args = parser.parse_args()

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    build_dir = os.path.abspath(args.build_dir)
    app_dir = os.path.join(build_dir, 'apps', args.app)
    elf_path = os.path.join(app_dir, args.app + '.stripped')
    elfloader_path = os.path.join(build_dir, 'apps', 'ELFLoader', 'image.srec')
    blhost_path = os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', 'linux', 'amd64', 'blhost')
    flashloader_path = os.path.join(root_dir, 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin')
    elftosb_path = os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', 'elftosb')
    mklfs_path = os.path.join(root_dir, 'third_party', 'mklfs', 'mklfs')
    paths_to_check = [
        build_dir,
        app_dir,
        elf_path,
        elfloader_path,
        blhost_path,
        flashloader_path,
        elftosb_path,
        mklfs_path,
    ]
    all_paths_exist = True
    for path in paths_to_check:
        if not os.path.exists(path):
            print('%s does not exist' % path)
            all_paths_exist = False
    if not all_paths_exist:
        return

    with tempfile.TemporaryDirectory() as workdir:
        sbfile_path = None
        if not args.ram:
            filesystem_path = CreateFilesystem(workdir, root_dir, mklfs_path, FILESYSTEM_FILES, elf_path)
            if not filesystem_path:
                print('Creating filesystem failed, exit')
                return
            sbfile_path = CreateSbFile(workdir, elftosb_path, elfloader_path, filesystem_path)
            if not sbfile_path:
                print('Creating sbfile failed, exit')
                return
        state = FlashtoolStates.CHECK_FOR_ANY
        while True:
            print(state)
            state = state_handlers[state](
                    blhost_path=blhost_path,
                    flashloader_path=flashloader_path,
                    sbfile_path=sbfile_path,
                    ram=args.ram,
                    elfloader_path=elfloader_path,
                    elf_path=elf_path)
            if state is FlashtoolStates.DONE:
                break

if __name__ == '__main__':
    main()
