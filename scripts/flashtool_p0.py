#!/usr/bin/python3
from enum import Enum, auto
import argparse
import hexformat
import os
import serial
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

def CreateSbFile(workdir, elftosb_path, srec_path, itcm_bdfile_path, spinand_bdfile_path, wifi_firmware_path, wifi_clm_blob_path):
    ivt_bin_path = os.path.join(workdir, 'ivt_program.bin')
    sbfile_path = os.path.join(workdir, 'program.sb')
    subprocess.check_call([elftosb_path, '-f', 'imx', '-V', '-c', itcm_bdfile_path, '-o', ivt_bin_path, srec_path])
    subprocess.check_call([elftosb_path, '-f', 'kinetis', '-V', '-c', spinand_bdfile_path, '-o', sbfile_path, ivt_bin_path, wifi_firmware_path, wifi_clm_blob_path])
    return sbfile_path

class FlashtoolStates(Enum):
    DONE = auto()
    ERROR = auto()
    CHECK_FOR_ANY = auto()
    CHECK_FOR_VALIANT = auto()
    RESET_TO_SDP = auto()
    CHECK_FOR_SDP = auto()
    LOAD_FLASHLOADER = auto()
    CHECK_FOR_FLASHLOADER = auto()
    PROGRAM = auto()
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

def CheckForFlashloader(**kwargs):
    for i in range(10):
        if is_flashloader_connected():
            return FlashtoolStates.PROGRAM
        time.sleep(1)
    return FlashtoolStates.ERROR

def Program(**kwargs):
    if kwargs['ram']:
        start_address = hexformat.srecord.SRecord.fromsrecfile(kwargs['srec_path']).startaddress
        subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'flash-image', kwargs['srec_path']])
        subprocess.call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'call', hex(start_address), '0'])
        return FlashtoolStates.DONE
    else:
        subprocess.check_call([kwargs['blhost_path'], '-u', flashloader_vidpid(), 'receive-sb-file', kwargs['sbfile_path']])
        return FlashtoolStates.RESET

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
    FlashtoolStates.PROGRAM: Program,
    FlashtoolStates.RESET: Reset,
}

def main():
    parser = argparse.ArgumentParser(description='Valiant flashtool',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--srec', type=str, required=True)
    parser.add_argument('--ram', dest='ram', action='store_true')
    parser.add_argument('--noram', dest='ram', action='store_false')
    parser.set_defaults(ram=False)
    args = parser.parse_args()

    if not os.path.exists(args.srec):
        print('SREC does not exist!')
        return
    blhost_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'blhost', 'bin', 'linux', 'amd64', 'blhost')
    if not os.path.exists(blhost_path):
        print('Unable to locate blhost!')
        return
    flashloader_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin')
    if not os.path.exists(flashloader_path):
        print('Unable to locate flashloader!')
        return
    elftosb_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'elftosb', 'elftosb')
    if not os.path.exists(elftosb_path):
        print('Unable to locate elftosb!')
        return
    itcm_bdfile_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'elftosb', 'imx-itcm-unsigned.bd')
    if not os.path.exists(itcm_bdfile_path):
        print('Unable to locate ITCM bdfile!')
        return
    spinand_bdfile_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'elftosb', 'program_flexspinand_image.bd')
    if not os.path.exists(spinand_bdfile_path):
        print('Unable to locate SPI NAND bdfile!')
        return
    wifi_firmware_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'firmware', 'cypress', '43455C0.bin')
    if not os.path.exists(wifi_firmware_path):
        print('Unable to locate SPI NAND bdfile!')
        return
    wifi_clm_blob_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'firmware', 'cypress', '43455C0.clm_blob')
    if not os.path.exists(wifi_clm_blob_path):
        print('Unable to locate SPI NAND bdfile!')
        return

    with tempfile.TemporaryDirectory() as workdir:
        sbfile_path = None
        if not args.ram:
            sbfile_path = CreateSbFile(workdir, elftosb_path, args.srec, itcm_bdfile_path, spinand_bdfile_path, wifi_firmware_path, wifi_clm_blob_path)
        state = FlashtoolStates.CHECK_FOR_ANY
        while True:
            print(state)
            state = state_handlers[state](blhost_path=blhost_path, flashloader_path=flashloader_path, sbfile_path=sbfile_path, ram=args.ram, srec_path=args.srec)
            if state is FlashtoolStates.DONE:
                break

if __name__ == '__main__':
    main()
