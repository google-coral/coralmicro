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
import requests
import sys

from PIL import Image, ImageDraw

"""
Displays object detection results received from the detect_objects server
running on a connected Dev Board Micro.

First load detect_objects example onto the Dev Board Micro:

    python3 scripts/flashtool.py -e detect_objects

Then start this client on your Linux computer that's connected via USB:

    python3 examples/detect_objects/detect_objects_client.py

You should see the image result appear in a new window.

Note that this example shows only the top detection result.
"""


def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    exit(1)
  return data[field_name]


def main():
  parser = argparse.ArgumentParser(
      description='Detect Camera Example',
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--host', type=str, default='10.10.10.1',
                      help='Hostname or IP Address of Coral Dev Board Micro')
  args = parser.parse_args()

  # Send RPC request
  response = requests.post(f'http://{args.host}:80/jsonrpc', json={
      'method': 'detect_from_camera',
      'jsonrpc': '2.0',
      'id': 0,
  }, timeout=10).json()

  # Get the image size
  result = get_field_or_die(response, 'result')
  width = get_field_or_die(result, 'width')
  height = get_field_or_die(result, 'height')

  # Decode the image data
  image_data_base64 = get_field_or_die(result, 'base64_data')
  image_data = base64.b64decode(image_data_base64)
  im = Image.frombytes('RGB', (width, height), image_data, 'raw')

  # Get the top detection coordinates
  detection = get_field_or_die(result, 'detection')
  left = get_field_or_die(detection, 'xmin') * width
  top = get_field_or_die(detection, 'ymin') * height
  right = get_field_or_die(detection, 'xmax') * width
  bottom = get_field_or_die(detection, 'ymax') * height

  # Draw a bounding-box with the object id and score
  draw = ImageDraw.Draw(im)
  draw.rectangle([left, top, right, bottom])
  text = f'ID: {detection["id"]} Score: {detection["score"]}'
  draw.text((left, bottom), text)

  im.show()


if __name__ == '__main__':
  try:
    main()
  except requests.exceptions.ConnectionError:
    msg = 'ERROR: Cannot connect to Coral Dev Board Micro, make sure you specify' \
          ' the correct IP address with --host.'
    if sys.platform == 'darwin':
      msg += ' Network over USB is not supported on macOS.'
    print(msg, file=sys.stderr)
