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

import itertools
import inspect
import requests
import json

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
def get_ethernet_ip(ip):
  """Get ethernet ip address.
  Args:
    ip: the usb ip address.
  Returns:
    A JSON-RPC result packet with the ethernet ip address.
    Example:
       {'id': 24, 'result': {'ethernet_ip': '192.168.1.117'}}
  """


@rpc
def wifi_connect(ip, ssid, password='', retries=5):
  """Connect to wifi
  Args:
    ip: the usb ip address
    ssid: the network name
    password: the network password
    retries: the amount of connection attempts
  Returns:
    A JSON-RPC result packet with no extra data, or JSON-RPC error.
    Example:
      {'id': 1, 'result': {}}
  """


@rpc
def wifi_get_ip(ip):
  """Get wifi ip
  Args:
    ip: the usb ip address.
  Returns:
    A JSON-RPC result packet with the ethernet ip address.
    Example:
       {'id': 24, 'result': {'ip': '192.168.1.117'}}
  """


@rpc
def wifi_get_status(ip):
  """Ge wifi status
  Args:
    ip: the usb ip address
  Returns:
    A JSON-RPC result packet with the connection status.
    Example:
      {'id': 24, 'result': {'status': true}}
  """


@rpc
def get_image_from_camera(ip, width, height, format, filter, rotation, auto_white_balance):
  """Get image from camera.
  Args:
    width: Width of the output image, in pixels.
    height: Height of the output image, in pixels.
    format: Pixel format of the output image (RGB, GRAY, RAW).
    filter: Demosaic filter applied to the output image (BILINEAR, NEAREST_NEIGHBOR).
    rotation: Degrees of rotation applied to the output image (0, 90, 180, 270).
    auto_white_balance: White balance the image as true; do nothing otherwise.
  Returns:
    A JSON-RPC result packet with image data, or JSON-RPC error.
    Example:
      {'id': 1, 'result': {'width': 324, 'height': 324, base64_data: '<snip>'}}
  """
