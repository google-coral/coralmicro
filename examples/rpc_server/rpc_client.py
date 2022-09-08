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
import requests
import sys

"""
Fetches the serial number and photo from the rpc_server
running on the Dev Board Micro.

First, load the rpc_server example onto the Dev Board Micro:

    python3 scripts/flashtool.py -e rpc_server

Then start this client on your computer:

    python3 examples/rpc_server/rpc_client.py

You should see an image from the camera appear in a new window, and the
serial console prints the board serial number.
"""


def main():
  parser = argparse.ArgumentParser(
      description='RPC Client Example',
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--host', type=str, default='10.10.10.1',
                      help='Hostname or IP Address of Coral Dev Board Micro')
  args = parser.parse_args()

  response = requests.post(f'http://{args.host}:80/jsonrpc', json={
      'method': 'serial_number',
      'jsonrpc': '2.0',
      'params': [],
      'id': 0,
  }, timeout=10).json()
  print(response)


if __name__ == '__main__':
  try:
    main()
  except requests.exceptions.ConnectionError:
    msg = 'ERROR: Cannot connect to Coral Dev Board Micro, make sure you specify' \
          ' the correct IP address with --host.'
    if sys.platform == 'darwin':
      msg += ' Network over USB is not supported on macOS.'
    print(msg, file=sys.stderr)
