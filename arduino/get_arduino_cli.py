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

import argparse
import os.path
import platform
import tarfile
import urllib.request
import zipfile


def main():
  parser = argparse.ArgumentParser(
      description='Download Arduino CLI',
      formatter_class=argparse.ArgumentDefaultsHelpFormatter
  )
  parser.add_argument('--system', default=platform.system(),
                      help='Arduino CLI for: Windows, Darwin, Linux')
  parser.add_argument('--version', required=True,
                      help='Arduino CLI version')
  parser.add_argument('--output_dir', required=True,
                      help='Directory to extract Arduino CLI archive')
  args = parser.parse_args()

  version = args.version
  name = {
      'Windows': 'Windows_64bit.zip',
      'Darwin': 'macOS_64bit.tar.gz',
      'Linux': 'Linux_64bit.tar.gz'
  }[args.system]
  url = f'https://github.com/arduino/arduino-cli/releases/download/{version}/arduino-cli_{version}_{name}'
  print(url)

  filename, _ = urllib.request.urlretrieve(url)
  if url.endswith('.zip'):
    with zipfile.ZipFile(filename, 'r') as f:
      f.extractall(args.output_dir)
  else:
    with tarfile.open(name=filename, mode='r:gz') as f:
      f.extractall(args.output_dir)


if __name__ == '__main__':
  main()
