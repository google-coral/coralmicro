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
import cv2
import eel
import numpy as np
import os
import rpc_helper

from PIL import Image

parser = argparse.ArgumentParser(description='Camera Streaming App')
parser.add_argument(
    '--port',
    type=int,
    default=8888,
    help='Port to host the app.')
parser.add_argument('--host_ip', type=str, default='10.10.10.1',
                    help='Dev Board Micro USB IP address.')
args = parser.parse_args()


@eel.expose
def video_feed():
  """ The main program loop that updates the UI's video-feed. Internally it does 3 things on every loop:
    1) Calls 'getImageConfig()' from javascripts to obtain user input
    2) Make a rpc request to the valiant's camera stream server to obtain the image using the configs from step 1
    3) Save the image as a new source to allow javascript to display the new image.
    :return: None
  """
  host_ip = args.host_ip
  while True:
    # Get user inputs.
    image_config = eel.getImageConfig()()
    eel.updateLog(str(image_config))()

    width = int(image_config['width'])
    height = int(image_config['height'])
    format = image_config['format']
    filter = image_config['filter']
    rotation = int(image_config['rotation'])
    auto_white_balance = image_config['awb']

    # Get the image.
    resp = rpc_helper.get_image_from_camera(
        host_ip, width, height, format, filter, rotation, auto_white_balance)
    frame = base64.b64decode(resp['result']['base64_data'])

    # Save image source for javascript to render.
    if format == 'RGB':
      im = Image.frombytes('RGB', (width, height), frame)
    elif format == 'GRAY':
      im = Image.frombytes('L', (width, height), frame)
    else:  # format == 'RAW'
      np_data = np.frombuffer(frame, dtype=np.uint8)
      np_data = np_data.reshape(width, height)
      debayered = cv2.cvtColor(np_data, cv2.COLOR_BAYER_BG2BGR)
      im = Image.fromarray(debayered)
    # img_path has to be relative to the web directory.
    img_path = os.path.join(
        os.path.dirname(
            os.path.abspath(__file__)),
        'web',
        'img_data.jpg')
    im.save(img_path)
    eel.updateImageSrc(os.path.basename(img_path), width, height)()


def main():
  curr_path = os.path.dirname(os.path.abspath(__file__))
  eel.init(os.path.join(curr_path, 'web'))
  host = '0.0.0.0'
  port = args.port
  print(f'Hosting on {host}:{port}')
  eel.start(
      'index.html',
      size=(
          1005,
          900),
      host=host,
      port=port,
      mode='chrome')


if __name__ == "__main__":
  main()
