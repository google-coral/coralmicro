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
import json
import sys
import time
from PIL import Image
import rpc_helper

parser = argparse.ArgumentParser(description='Image server client')
parser.add_argument('--usb_ip', type=str, required=True,
                    help='Dev Board Micro USB IP address.')
parser.add_argument('--ethernet', action='store_true',
                    help='Get image from ethernet ip.')
parser.add_argument('--ethernet_ip', type=str, required=False,
                    help='Dev Board Micro Ethernet IP address.')
parser.add_argument('--image_width', type=int, default=700,
                    help='Specify image width.')
parser.add_argument('--image_height', type=int,
                    default=700, help='Specify image height.')
parser.add_argument('--image_rotation', type=int, default=270,
                    choices=[0, 90, 180, 270], help='Image rotation (degrees)')
parser.add_argument('--image_format', type=str, default='RGB',
                    choices=['RGB', 'GRAY', 'RAW'], help='Pixel format')
parser.add_argument('--image_filter', type=str, default='BILINEAR',
                    choices=['BILINEAR', 'NEAREST_NEIGHBOR'], help='Demosaic interpolation method')
parser.add_argument('--auto_white_balance',
                    action='store_true', help='Enable white balancing')
parser.add_argument('--noauto_white_balance',
                    action='store_false', dest='auto_white_balance')
parser.set_defaults(auto_white_balance=True)
parser.add_argument('--wifi', action='store_true',
                    help='Get image from wifi ip.')
parser.add_argument('--wifi_ssid', type=str,
                    required=False, help='wifi network name')
parser.add_argument('--wifi_psk', type=str, required=False,
                    help='wifi network password')


def display_image(response, width, height, format):
  result = get_field_or_die(response, 'result')
  image_data_base64 = get_field_or_die(result, 'base64_data')
  image_data = base64.b64decode(image_data_base64)

  if format == 'RAW':
    import cv2
    import numpy as np
    np_data = np.frombuffer(image_data, dtype=np.uint8)
    np_data = np_data.reshape(width, height)
    debayered = cv2.cvtColor(np_data, cv2.COLOR_BAYER_BG2BGR)
    im = Image.fromarray(debayered)
    im.show()
    return

  if format == 'RGB':
    format = 'RGB'
  elif format == 'GRAY':
    format = 'L'
  else:
    raise ValueError

  im = Image.frombytes(format, (width, height), image_data)
  im.show()


def parse_ethernet_ip(response):
  result = get_field_or_die(response, 'result')
  return get_field_or_die(result, 'ethernet_ip')


def parse_wifi_ip(response):
  print(f'response:\n{json.dumps(response, indent=2)}')
  result = get_field_or_die(response, 'result')
  return get_field_or_die(result, 'wifi_ip')


def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    sys.exit(1)
  return data[field_name]


def main():
  args = parser.parse_args()
  width = args.image_width
  height = args.image_height
  format = args.image_format
  filter = args.image_filter
  rotation = args.image_rotation
  auto_white_balance = args.auto_white_balance

  # RAW images can only be requested at native resolution
  if format == 'RAW':
    width = 324
    height = 324

  if args.ethernet:
    ethernet_ip = args.ethernet_ip
    if ethernet_ip is None:
      ethernet_ip = parse_ethernet_ip(rpc_helper.get_ethernet_ip(args.usb_ip))
    print(f'Ethernet ip: {ethernet_ip}')
    display_image(rpc_helper.get_image_from_camera(ethernet_ip, width, height,
                  format, filter, rotation, auto_white_balance), width, height, format)
  elif args.wifi:
    wifi_ip = parse_wifi_ip(rpc_helper.wifi_get_ip(args.usb_ip))
    print(f'Wifi ip: {wifi_ip}')
    display_image(rpc_helper.get_image_from_camera(wifi_ip, width, height,
                  format, filter, rotation, auto_white_balance), width, height, format)
  else:  # USB
    display_image(rpc_helper.get_image_from_camera(args.usb_ip, width, height,
                  format, filter, rotation, auto_white_balance), width, height, format)


if __name__ == '__main__':
  main()
