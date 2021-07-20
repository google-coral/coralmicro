#!/usr/bin/python3
"""
Generate package for Valiant Arduino core.
"""

import argparse
import hashlib
import json
import multiprocessing
import os
import tempfile
import shutil
import subprocess
import tarfile
import PyInstaller.__main__

def main():
    parser = argparse.ArgumentParser(
        description='Valiant Arduino packager',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument('--output_dir', type=str, required=True)
    args = parser.parse_args()

    arduino_dir = os.path.abspath(os.path.dirname(__file__))
    root_dir = os.path.abspath(os.path.join(arduino_dir, '..'))
    with tempfile.TemporaryDirectory() as tmpdir:
        # Copy out arduino core to temp directory
        core_out_dir = os.path.join(tmpdir, 'coral-valiant-1.0.0')
        shutil.copytree(arduino_dir, core_out_dir)

        # Create a CMake build directory in the temp directory,
        # and build the arduino library bundle.
        build_dir = os.path.join(tmpdir, 'build')
        os.makedirs(build_dir)
        subprocess.check_call(['cmake', root_dir, '-DVALIANT_ARDUINO=1'], cwd=build_dir)
        subprocess.check_call(['make', '-C', build_dir, '-j', str(multiprocessing.cpu_count()), 'bundling_target', 'ELFLoader'])
        libs_dir = os.path.join(core_out_dir, 'variants', 'VALIANT', 'libs')
        os.makedirs(libs_dir, exist_ok=True)

        # Copy the arduino library bundle into the core.
        shutil.copy(os.path.join(build_dir, 'liblibs_arduino_bundled.a'), libs_dir)

        bootloader_dir = os.path.join(core_out_dir, 'bootloaders', 'VALIANT')
        os.makedirs(bootloader_dir)
        shutil.copy(os.path.join(build_dir, 'apps', 'ELFLoader', 'image.srec'), bootloader_dir)

        # Leverage pyinstaller to package flashtool.
        pyinstaller_dist_path = os.path.join(core_out_dir, 'tools', 'flashtool')
        PyInstaller.__main__.run([
            '-F',
            '--distpath', pyinstaller_dist_path,
            '--add-binary',
            os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', 'linux', 'amd64', 'blhost') + ':' + os.path.join('third_party', 'nxp', 'blhost', 'bin', 'linux', 'amd64'),
            '--add-binary',
            os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', 'elftosb') + ':' + os.path.join('third_party', 'nxp', 'elftosb'),
            '--add-binary',
            os.path.join(root_dir, 'third_party', 'toolchain', 'gcc-arm-none-eabi-9-2020-q2-update', 'bin', 'arm-none-eabi-strip') + ':' + os.path.join('third_party', 'toolchain', 'gcc-arm-none-eabi-9-2020-q2-update', 'bin'),
            '--add-binary',
            os.path.join(root_dir, 'third_party', 'nxp', 'flashloader', 'ivt_flashloader.bin') + ':' + os.path.join('third_party', 'nxp', 'flashloader'),
            os.path.join(root_dir, 'scripts', 'flashtool.py'),
        ])

        # tbz2 everything
        tar_path = os.path.join(args.output_dir, 'coral-valiant-1.0.0.tar.bz2')
        with tarfile.open(name=tar_path, mode='w:bz2') as arduino_tar:
            arduino_tar.add(core_out_dir, arcname='coral-valiant-1.0.0')

        tar_sha256sum = hashlib.sha256()
        with open(tar_path, 'rb') as f:
            tar_sha256sum.update(f.read())

        # Make an arduino package manifest.
        json_obj = {
            'packages': [
                {
                    'name': 'coral',
                    'maintainer': 'Coral Team',
                    'websiteURL': 'https://coral.ai',
                    'tools': [
                        {
                            'name': 'arm-none-eabi-gcc',
                            'version': '9-2020q2',
                            'systems': [
                                {
                                    'host': 'x86_64-pc-linux-gnu',
                                    'url': 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2',
                                    'archiveFileName': 'gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2',
                                    'checksum': 'MD5:2b9eeccc33470f9d3cda26983b9d2dc6',
                                    'size': '140360119'
                                }
                            ]
                        }
                    ],
                    'email': 'coral-support@google.com',
                    'platforms': [
                        {
                            'name': 'Valiant',
                            'architecture': 'valiant',
                            'version': '1.0.0',
                            'url': 'http://localhost:8000/coral-valiant-1.0.0.tar.bz2',
                            'archiveFileName': 'coral-valiant-1.0.0.tar.bz2',
                            'checksum': 'SHA-256:%s' % tar_sha256sum.hexdigest(),
                            'size': os.path.getsize(tar_path),
                            'boards': [
                                {
                                    'name': 'Coral Valiant',
                                }
                            ],
                            'toolsDependencies': [
                                {
                                    'packager': 'coral',
                                    'name': 'arm-none-eabi-gcc',
                                    'version': '9-2020q2'
                                }
                            ],
                        },
                    ],
                },
            ],
        }
        with open(os.path.join(args.output_dir, 'package_coral_index.json'), 'w') as f:
            json.dump(json_obj, f, indent=2)

if __name__ == "__main__":
    main()