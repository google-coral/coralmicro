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

import base64
import requests

from PIL import Image

"""
Fetches the serial number and photo from the rpc_server
running on the Dev Board Micro.

First, load the rpc_server example onto the Dev Board Micro:

    python3 scripts/flashtool.py -b build -e rpc_server

Then start this client on your computer:

    python3 examples/rpc_server/rpc_client.py

You should see an image from the camera appear in a new window, and the
serial console prints the board serial number.
"""

def main():
    url = "http://10.10.10.1:80/jsonrpc"

    payload = {
        "method": "serial_number",
        "params": [],
        "jsonrpc": "2.0",
        "id": 0,
    }

    response = requests.post(url, json=payload).json()
    print(response)

    payload = {
        "method": "take_picture",
        "params": [],
        "jsonrpc": "2.0",
        "id": 0,
    }

    response = requests.post(url, json=payload).json()
    assert(response['result']['base64_data'])
    image_data_base64 = response['result']['base64_data']
    width = response['result']['width']
    height = response['result']['height']
    image_data = base64.b64decode(image_data_base64)
    im = Image.frombytes('RGB', (width, height), image_data, 'raw')
    print(im)
    im.show()

if __name__ == '__main__':
    main()
