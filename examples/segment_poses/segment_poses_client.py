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
import math
import numpy as np
import sys

from PIL import Image

_IMAGE_SIZE = 324
_HEATMAP_MASK_LENGTH = 21


def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    exit(1)
  return data[field_name]


def main():
  parser = argparse.ArgumentParser(
      description='Bodypix Example',
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--host', type=str, default='10.10.10.1',
                      help='Hostname or IP Address of Coral Dev Board Micro')
  args = parser.parse_args()

  # Send RPC request
  print('Asking the device to run BodyPix until we get a high-confidence result.')
  BELOW_CONFIDENCE_THRESHOLD = -2
  while True:
    response = requests.post(f'http://{args.host}:80/jsonrpc', json={
        'method': 'run_bodypix',
        'jsonrpc': '2.0',
        'id': 0,
    }, timeout=(10, 120)).json()
    if 'error' in response:
      error_code = response['error']['code']
      if error_code != BELOW_CONFIDENCE_THRESHOLD:
        print('Tensorflow failed to invoke.')
        return
    else:
      break

  # Get the image size
  result = get_field_or_die(response, 'result')
  width = get_field_or_die(result, 'width')
  height = get_field_or_die(result, 'height')

  # Decode the image and mask data
  image_data_base64 = get_field_or_die(result, 'base64_data')
  image_data = base64.b64decode(image_data_base64)
  im = Image.frombytes('RGB', (width, height), image_data, 'raw')

  mask_segments_base64 = get_field_or_die(result, 'output_mask1')
  mask_segments_data = base64.b64decode(mask_segments_base64)

  predicted_mask = np.frombuffer(
      mask_segments_data, dtype=np.uint8, count=-1)

  # Match the dimensions of the original tensor
  predicted_mask = predicted_mask.reshape(
      _HEATMAP_MASK_LENGTH, _HEATMAP_MASK_LENGTH)

  # Turn the heatmap red
  rgb_heatmap = np.dstack([predicted_mask[:, :]]*3)
  rgb_heatmap[:, :, 1:] = 0

  # Assume image is square, current model is for 324x324 images
  rescale_factor = math.ceil(width/_HEATMAP_MASK_LENGTH)

  # Blow up the heatmap to match the image size
  rgb_heatmap = np.kron(rgb_heatmap, np.ones(
      (rescale_factor, rescale_factor, 1)))

  # Blown up version using ceiling, so clip the edges down to the exact size.
  clip_size = int((rgb_heatmap.shape[0] - _IMAGE_SIZE)/2)
  rgb_heatmap = rgb_heatmap[clip_size:_IMAGE_SIZE +
                            clip_size, clip_size:_IMAGE_SIZE+clip_size]

  heatmap_im = Image.fromarray(rgb_heatmap.astype('uint8'))

  # Display the input image and segmentation results.
  output_img = Image.new('RGB', (2 * width, height))
  output_img.paste(im, (0, 0))
  output_img.paste(heatmap_im, (width, 0))
  output_img.show()


if __name__ == '__main__':
  try:
    main()
  except requests.exceptions.ConnectionError:
    msg = 'ERROR: Cannot connect to Coral Dev Board Micro, make sure you specify' \
          ' the correct IP address with --host.'
    if sys.platform == 'darwin':
      msg += ' Network over USB is not supported on macOS.'
    print(msg, file=sys.stderr)
