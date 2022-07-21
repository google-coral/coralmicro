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
import base64
import inspect
import itertools
import json
import requests
import sys
from PIL import Image

parser = argparse.ArgumentParser(description='Image server client')
parser.add_argument('--usb_ip', type=str, default='10.10.10.1', help='Dev Board Micro USB IP address.')
parser.add_argument('--image_width', type=int, default=700, help='Specify image width.')
parser.add_argument('--image_height', type=int, default=700, help='Specify image height.')

rpc_id = itertools.count()

def rpc(func):
  def rpc_impl(*args, **kwargs):
    params = inspect.getcallargs(func, *args, **kwargs)
    url = f"http://{params.pop('ip')}:80/jsonrpc"
    payload = {
      'id': next(rpc_id),
      'jsonrpc': '2.0',
      'method': func.__name__,
      'params': [params or {}],
    }
    print(f'url: {url}')
    print(f'payload:\n{json.dumps(payload, indent=2)}')
    return requests.post(url, json=payload).json()

  return rpc_impl


@rpc
def get_captured_image(ip, width, height):
  """Get image from camera.
  Args:
    width: Width of the output image, in pixels.
    height: Height of the output image, in pixels.
  Returns:
    A JSON-RPC result packet with image data, or JSON-RPC error.
    Example:
      {'id': 1, 'result': {'width': 324, 'height': 324, base64_data: '<snip>'}}
  """

def display_image(response, width, height):
  result = get_field_or_die(response, 'result')
  image_data_base64 = get_field_or_die(result, 'base64_data')
  image_data = base64.b64decode(image_data_base64)
  im = Image.frombytes('RGB', (width, height), image_data)
  im.show()


def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    sys.exit(1)
  return data[field_name]


def main():
  args = parser.parse_args()
  width = args.image_width
  height = args.image_height
  display_image(get_captured_image(args.usb_ip, width, height), width, height)

if __name__ == '__main__':
  main()
