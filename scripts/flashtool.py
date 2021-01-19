#!/usr/bin/python3
from retry import retry
import argparse
import os
import re
import subprocess
import tempfile
import usb.core

SDP_VID = 0x1fc9
SDP_PID = 0x013d

FLASHLOADER_VID = 0x15a2
FLASHLOADER_PID = 0x0073

FLASH_BASE_ADDRESS = 0x30000000

MEMORY_SIZE_RE = re.compile('Total Size = ([0-9]+) MB')

def sdp_vidpid():
    return '{},{}'.format(hex(SDP_VID), hex(SDP_PID))

def flashloader_vidpid():
    return '{},{}'.format(hex(FLASHLOADER_VID), hex(FLASHLOADER_PID))


@retry(tries=10, delay=1)
def load_flashloader(blhost_path, flashloader_path):
    subprocess.check_call([blhost_path, '-u', sdp_vidpid(), '--', 'load-image', flashloader_path], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def is_sdp_connected():
    return usb.core.find(idVendor=SDP_VID, idProduct=SDP_PID) is not None

@retry(tries=10, delay=1)
def is_flashloader_connected():
    flashloader = usb.core.find(idVendor=FLASHLOADER_VID, idProduct=FLASHLOADER_PID)
    if flashloader is None:
        raise ValueError
    return flashloader

@retry(tries=10, delay=1)
def prepare_flash(blhost_path):
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), '--timeout=30000', 'fill-memory', '0x2000', '4', '0xc0000007'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), '--timeout=30000', 'configure-memory', '0x9', '0x2000'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def get_memory_size(blhost_path):
    handle = subprocess.Popen([blhost_path, '-u', flashloader_vidpid(), 'list-memory'], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    (stdout, _) = handle.communicate()
    for line in stdout.decode().splitlines():
        match = MEMORY_SIZE_RE.search(line)
        if match:
            return int(match.group(1)) * 1024 * 1024
    return 0

def erase_flash(blhost_path):
    print('Erasing all flash (may take a bit, please be patient!)')
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), '--timeout=2000000', 'flash-erase-all', '0x9'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def flash_image(blhost_path, binary_to_flash):
    subprocess.check_call([blhost_path, '-u', flashloader_vidpid(), '--timeout=60000', 'flash-image', binary_to_flash, 'erase'])

def flash_binary(blhost_path, binary_to_flash):
    print('Flashing device with {}'.format(binary_to_flash))
    flash_image(blhost_path, binary_to_flash)

def elf_to_binary(toolchain_path, elf_to_flash):
    binary = tempfile.NamedTemporaryFile(suffix='.hex')
    subprocess.check_call([os.path.join(toolchain_path, 'arm-none-eabi-objcopy'), '-O', 'ihex', elf_to_flash, binary.name])
    return binary

def flash_elf(blhost_path, toolchain_path, elf_to_flash):
    with elf_to_binary(toolchain_path, elf_to_flash) as binary_to_flash:
      flash_binary(blhost_path, binary_to_flash.name)
    subprocess.check_call(['pyocd', 'reset'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

def main():
    parser = argparse.ArgumentParser(description='Valiant flashtool',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    erase_parser = parser.add_mutually_exclusive_group(required=False)
    erase_parser.add_argument('--erase', dest='erase', action='store_true')
    erase_parser.add_argument('--noerase', dest='erase', action='store_false')
    parser.add_argument('--elf', type=str)
    parser.set_defaults(erase=False)
    args = parser.parse_args()

    blhost_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'blhost', 'bin', 'linux', 'amd64', 'blhost')
    if not os.path.exists(blhost_path):
        print('Unable to locate blhost!')
        return
    flashloader_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin')
    if not os.path.exists(flashloader_path):
        print('Unable to locate flashloader!')
        return
    toolchain_path = os.path.join(os.path.dirname(__file__), '..', 'third_party', 'toolchain', 'gcc-arm-none-eabi-9-2020-q2-update', 'bin')
    if not os.path.exists(toolchain_path):
        print('Unable to locate toolchain!')
        return

    if is_sdp_connected():
        print('Found SDP, load flashloader')
        load_flashloader(blhost_path, flashloader_path)

    if is_flashloader_connected():
        print('Found flashloader, prepare to flash')
        prepare_flash(blhost_path)
        memory_size = get_memory_size(blhost_path)
    else:
        print('No flashloader found.')
        return

    if args.erase:
        erase_flash(blhost_path)

    if args.elf:
        if os.path.exists(args.elf):
            elf_to_flash = args.elf
        else:
            print('{} not found'.format(args.elf))

        flash_elf(blhost_path, toolchain_path, elf_to_flash)

if __name__ == '__main__':
    main()
