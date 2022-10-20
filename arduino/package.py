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
"""
Generate package for Coral Dev Board Micro Arduino core.
"""

import argparse
import git
import hashlib
import json
import os
import tempfile
import platform
import py7zr
import semver
import shutil
import subprocess
import tarfile
import urllib.request
import PyInstaller.__main__

platform_dir = ''
toolchain_dir = ''
exe_extension = ''
pyinstaller_separator = ':'
platform_name = ''
system_name = platform.system()
if system_name == 'Windows':
  platform_dir = 'win'
  toolchain_dir = 'toolchain-win'
  exe_extension = '.exe'
  platform_name = 'windows'
  pyinstaller_separator = ';'
elif system_name == 'Darwin':
  platform_dir = 'mac'
  toolchain_dir = 'toolchain-mac'
  platform_name = 'osx'
else:
  platform_dir = 'linux/amd64'
  toolchain_dir = 'toolchain-linux'
  platform_name = 'linux64'


def file_sha256(filename):
  h = hashlib.sha256()
  with open(filename, 'rb') as f:
    for block in iter(lambda: f.read(32 * 1024), b''):
      h.update(block)
    return h.hexdigest()


def CreateFlashtoolExe(core_out_dir, root_dir):
  platform_flags = []
  if system_name == 'Windows':
    try:
      libusb_1_0_25_7z = 'https://github.com/libusb/libusb/releases/download/v1.0.25/libusb-1.0.25.7z'
      libusb_1_0_25_7z_sha256 = '3d1c98416f454026034b2b5d67f8a294053898cb70a8b489874e75b136c6674d'
      filename, _ = urllib.request.urlretrieve(libusb_1_0_25_7z)
      if file_sha256(filename) != libusb_1_0_25_7z_sha256:
        raise RuntimeError('libusb checksum mismatch!')

      with py7zr.SevenZipFile(filename, 'r') as archive:
        print(archive.extract(path=core_out_dir, targets=[
              'VS2019/MS64/Release/dll/libusb-1.0.dll']))
    finally:
      urllib.request.urlcleanup()
    platform_flags = [
        '--add-binary',
        core_out_dir + '/VS2019/MS64/Release/dll/libusb-1.0.dll' +
        pyinstaller_separator + '.',
    ]
  pyinstaller_dist_path = core_out_dir
  PyInstaller.__main__.run([
      '-F',
      '--noupx',
      '--distpath', pyinstaller_dist_path,
      '--add-binary',
      os.path.join(root_dir, 'third_party', 'nxp', 'blhost', 'bin', platform_dir, 'blhost' + exe_extension) +
      pyinstaller_separator +
      os.path.join('third_party', 'nxp', 'blhost', 'bin', platform_dir),
      '--add-binary',
      os.path.join(root_dir, 'third_party', 'nxp', 'elftosb', platform_dir, 'elftosb' + exe_extension) +
      pyinstaller_separator +
      os.path.join('third_party', 'nxp', 'elftosb', platform_dir),
      '--add-binary',
      os.path.join(root_dir, 'third_party', toolchain_dir, 'gcc-arm-none-eabi-9-2020-q2-update', 'bin', 'arm-none-eabi-objdump' +
                   exe_extension) + pyinstaller_separator + os.path.join('third_party', toolchain_dir, 'gcc-arm-none-eabi-9-2020-q2-update', 'bin'),
      '--add-binary',
      os.path.join(root_dir, 'third_party', toolchain_dir, 'gcc-arm-none-eabi-9-2020-q2-update', 'bin', 'arm-none-eabi-strip' +
                   exe_extension) + pyinstaller_separator + os.path.join('third_party', toolchain_dir, 'gcc-arm-none-eabi-9-2020-q2-update', 'bin'),
      '--hidden-import', 'progress.bar',
      '--hidden-import', 'progress',
      '--hidden-import', 'hexformat',
      '--hidden-import', 'hid',
      '--hidden-import', 'ipaddress',
      '--hidden-import', 'serial',
      '--hidden-import', 'signal',
      '--hidden-import', 'struct',
      '--hidden-import', 'sys',
      '--hidden-import', 'time',
      '--hidden-import', 'usb.core'
  ] + platform_flags + [
      os.path.join(root_dir, 'scripts', 'flashtool.py'),
  ])


def main():
  parser = argparse.ArgumentParser(
      description='Coral Dev Board Micro Arduino packager',
      formatter_class=argparse.ArgumentDefaultsHelpFormatter
  )
  parser.add_argument('--output_dir', type=str, required=True)
  main_group = parser.add_mutually_exclusive_group(required=True)
  main_group.add_argument('--core', action='store_true')
  main_group.add_argument('--flashtool', action='store_true')
  main_group.add_argument('--manifest', action='store_true')

  parser.add_argument('--core_url', type=str, default=None)
  parser.add_argument('--win_flashtool_url', type=str, default=None)
  parser.add_argument('--mac_flashtool_url', type=str, default=None)
  parser.add_argument('--linux_flashtool_url', type=str, default=None)
  parser.add_argument('--manifest_revision', type=str, default=None)
  args = parser.parse_args()

  arduino_dir = os.path.abspath(os.path.dirname(__file__))
  root_dir = os.path.abspath(os.path.join(arduino_dir, '..'))
  repo = git.Repo(root_dir)
  git_revision = str(repo.head.commit)
  common_kwargs = {
      'arduino_dir': arduino_dir,
      'root_dir': root_dir,
      'git_revision': git_revision,
  }

  # Ensure the `output_dir` exists.
  os.makedirs(args.output_dir, exist_ok=True)
  if args.core:
    return core_main(args, **common_kwargs)
  elif args.flashtool:
    return flashtool_main(args, **common_kwargs)
  elif args.manifest:
    return manifest_main(args, **common_kwargs)


def flashtool_main(args, **kwargs):
  root_dir = kwargs.get('root_dir')
  git_revision = kwargs.get('git_revision')

  with tempfile.TemporaryDirectory() as tmpdir:
    flashtool_out_dir = os.path.join(
        tmpdir, 'coral-flashtool-%s-%s' % (platform_name, git_revision))
    CreateFlashtoolExe(flashtool_out_dir, root_dir)

    flashtool_tar_path = os.path.join(
        args.output_dir, 'coral-flashtool-%s-%s.tar.bz2' % (platform_name, git_revision))
    with tarfile.open(name=flashtool_tar_path, mode='w:bz2') as flashtool_tar:
      flashtool_tar.add(flashtool_out_dir, arcname='coral-flashtool-%s-%s' %
                        (platform_name, git_revision))
    flashtool_sha256sum = hashlib.sha256()
    with open(flashtool_tar_path, 'rb') as f:
      flashtool_sha256sum.update(f.read())


def GetDownloadMetadata(url):
  try:
    filename, _ = urllib.request.urlretrieve(url)
    return file_sha256(filename), os.path.getsize(filename)
  finally:
    urllib.request.urlcleanup()


def manifest_main(args, **kwargs):
  manifest_revision = args.manifest_revision
  if not manifest_revision:
    print('Missing manifest revision!')
    return
  semver.VersionInfo.parse(manifest_revision)

  git_revision = kwargs.get('git_revision')

  core_url = args.core_url
  win_flashtool_url = args.win_flashtool_url
  mac_flashtool_url = args.mac_flashtool_url
  linux_flashtool_url = args.linux_flashtool_url

  systems_json = []
  if linux_flashtool_url:
    sha256sum, size = GetDownloadMetadata(linux_flashtool_url)
    systems_json.append({
        'host': 'x86_64-pc-linux-gnu',
        'url': linux_flashtool_url,
        'archiveFileName': 'coral-flashtool-linux64.tar.bz2',
        'checksum': 'SHA-256:%s' % sha256sum,
        'size': str(size)
    })

  if mac_flashtool_url:
    sha256sum, size = GetDownloadMetadata(mac_flashtool_url)
    systems_json.append({
        'host': 'x86_64-apple-darwin',
        'url': mac_flashtool_url,
        'archiveFileName': 'coral-flashtool-osx.tar.bz2',
        'checksum': 'SHA-256:%s' % sha256sum,
        'size': str(size)
    })

  if win_flashtool_url:
    sha256sum, size = GetDownloadMetadata(win_flashtool_url)
    systems_json.append({
        'host': 'i686-mingw32',
        'url': win_flashtool_url,
        'archiveFileName': 'coral-flashtool-windows.tar.bz2',
        'checksum': 'SHA-256:%s' % sha256sum,
        'size': str(size)
    })

  core_json = None
  if core_url:
    sha256sum, size = GetDownloadMetadata(core_url)
    core_json = {
        'name': 'Coral',
        'architecture': 'coral_micro',
        'version': manifest_revision,
        'url': core_url,
        'archiveFileName': 'coral-micro.tar.bz2',
        'checksum': 'SHA-256:%s' % sha256sum,
        'size': str(size),
        'boards': [
            {
                'name': 'Dev Board Micro',
            },
            {
                'name': 'Dev Board Micro + Wireless Add-on',
            },
            {
                'name': 'Dev Board Micro + PoE Add-on',
            },
        ],
        'toolsDependencies': [
            {
                'packager': 'coral',
                'name': 'arm-none-eabi-gcc',
                'version': '9-2020q2'
            },
            {
                'packager': 'coral',
                'name': 'flashtool',
                'version': manifest_revision
            }
        ],
    },

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
                          },
                          {
                              'host': 'x86_64-apple-darwin',
                              'url': 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-mac.tar.bz2',
                              'archiveFileName': 'gcc-arm-none-eabi-9-2020-q2-update-mac.tar.bz2',
                              'checksum': 'MD5:75a171beac35453fd2f0f48b3cb239c3',
                              'size': '142999997'
                          },
                          {
                              'host': 'i686-mingw32',
                              'url': 'https://dl.google.com/coral/gcc-arm-none-eabi-9-2020-q2-update-win32-arduino1.zip',
                              'archiveFileName': 'gcc-arm-none-eabi-9-2020-q2-update-win32-arduino1.zip',
                              'checksum': 'SHA-256:daa13799151d05adb5c37016010e5ff649941aab4dac150a3ad649749cde4896',
                              'size': '182850168'
                          }
                      ]
                  },
                  {
                      'name': 'flashtool',
                      'version': manifest_revision,
                      'systems': systems_json
                  }
              ],
              'email': 'coral-support@google.com',
              'platforms': core_json,
          },
      ],
  }
  with open(os.path.join(args.output_dir, 'package_coral_index.json'), 'w') as f:
    json.dump(json_obj, f, indent=2)


def read_lines(filename):
  with open(filename) as f:
    for line in f:
      yield line.rstrip()


def check_output_lines(args):
  return subprocess.check_output(args).decode("utf-8").strip().split('\n')


def core_main(args, **kwargs):
  root_dir = kwargs['root_dir']
  arduino_dir = kwargs['arduino_dir']
  git_revision = kwargs['git_revision']

  with tempfile.TemporaryDirectory() as tmp_dir:
    # Build Arduino artifacts.
    build_dir = os.path.join(tmp_dir, 'build')
    os.makedirs(build_dir)
    subprocess.check_call(['cmake', root_dir, '-DCORAL_MICRO_ARDUINO=1',
                           '-G', 'Ninja'], cwd=build_dir)
    subprocess.check_call(['ninja', '-C', build_dir,
                           'bundling_target_libs_arduino_coral_micro',
                           'bundling_target_libs_arduino_coral_micro_poe',
                           'bundling_target_libs_arduino_coral_micro_wifi',
                           'elf_loader',
                           'flashloader'])

    # Copy main files.
    core_name = f'coral-micro-{git_revision}'
    core_out_dir = os.path.join(tmp_dir, core_name)
    shutil.copytree(arduino_dir, core_out_dir,
                    ignore=shutil.ignore_patterns('*.py', 'requirements.txt'))

    # Copy variant libraries.
    ar_path = os.path.join(root_dir, 'third_party', toolchain_dir,
                           'gcc-arm-none-eabi-9-2020-q2-update', 'bin',
                           'arm-none-eabi-ar' + exe_extension)
    for variant in ['coral_micro', 'coral_micro_poe', 'coral_micro_wifi']:
      libs_dir = os.path.join(core_out_dir, 'variants', variant, 'libs')
      os.makedirs(libs_dir, exist_ok=True)
      bundled_lib_path = os.path.join(
          libs_dir, f'liblibs_arduino_{variant}_bundled.a')

      obj_paths = set()
      for path in read_lines(os.path.join(build_dir, f'libs_arduino_{variant}_bundled.txt')):
        if 'prebuilt:' in path:
          prebuilt_path = path.strip('prebuilt:')
          # For prebuilt static libs, we need to first extracts the objs.
          subprocess.check_call([ar_path, 'x', prebuilt_path],
                                cwd=build_dir, stderr=subprocess.STDOUT)
          for obj in check_output_lines([ar_path, 't', prebuilt_path]):
            # For some reason, there is multiple definitions for _sbrk.
            if obj != 'fsl_sbrk.c.obj':
              obj_paths.add(os.path.join(build_dir, obj))
        else:
          obj_paths.update(check_output_lines([ar_path, 't', path]))

      for obj_path in obj_paths:
        subprocess.check_call([ar_path, 'q', bundled_lib_path,
                               os.path.join(build_dir, obj_path)],
                              cwd=build_dir, stderr=subprocess.STDOUT)
    # Copy bootloaders.
    bootloader_dir = os.path.join(core_out_dir, 'bootloaders', 'coral_micro')
    os.makedirs(bootloader_dir)
    shutil.copyfile(os.path.join(build_dir, 'apps', 'elf_loader', 'image.srec'),
                    os.path.join(bootloader_dir, 'elfloader.srec'))
    shutil.copyfile(os.path.join(build_dir, 'apps', 'elf_loader', 'elf_loader'),
                    os.path.join(bootloader_dir, 'elf_loader'))
    shutil.copyfile(os.path.join(build_dir, 'libs', 'nxp', 'flashloader', 'image.srec'),
                    os.path.join(bootloader_dir, 'flashloader.srec'))

    # Copy license files
    shutil.copyfile(os.path.join(arduino_dir, 'LICENSE'),
                    os.path.join(core_out_dir, 'LICENSE'))
    shutil.copyfile(os.path.join(arduino_dir, 'THIRD_PARTY_NOTICES.txt'),
                    os.path.join(core_out_dir, 'THIRD_PARTY_NOTICES.txt'))
    # Archive core.
    tar_path = os.path.join(args.output_dir, f'{core_name}.tar.bz2')
    with tarfile.open(name=tar_path, mode='w:bz2') as tar:
      tar.add(core_out_dir, arcname=core_name)


if __name__ == "__main__":
  main()
