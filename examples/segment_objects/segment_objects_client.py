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
import numpy as np
import requests
import sys

from PIL import Image

"""
Displays object segmentation results received from the segment_objects server
running on a connected Dev Board Micro.

First load segment_objects example onto the Dev Board Micro:

    python3 scripts/flashtool.py -e segment_objects

Then start this client on your connected Linux computer:

    python3 -m pip install examples/segment_objects/requirements.txt
    python3 examples/segment_objects/segment_objects_client.py

You should see the image result appear in a new window.

When using the default model (keras_post_training_unet_mv2_128_quant_edgetpu.tflite),
it is trained on the Pets Dataset as described in https://www.tensorflow.org/tutorials/images/segmentation.
This means that the it will show:

Class 1 (Black): Pixel belonging to the pet.
Class 2 (Green): Pixel bordering the pet.
Class 3 (Red): None of the above/a surrounding pixel.
"""


def create_pascal_label_colormap():
  """Creates a label colormap used in PASCAL VOC segmentation benchmark.
  Returns:
    A Colormap for visualizing segmentation results.
  """
  colormap = np.zeros((256, 3), dtype=int)
  indices = np.arange(256, dtype=int)

  for shift in reversed(range(8)):
    for channel in range(3):
      colormap[:, channel] |= ((indices >> channel) & 1) << shift
    indices >>= 3

  return colormap


def label_to_color_image(label):
  """Adds color defined by the dataset colormap to the label.
  Args:
    label: A 2D array with integer type, storing the segmentation label.
  Returns:
    result: A 2D array with floating type. The element of the array
      is the color indexed by the corresponding element in the input label
      to the PASCAL color map.
  Raises:
    ValueError: If label is not of rank 2 or its value is larger than color
      map maximum entry.
  """
  if label.ndim != 2:
    raise ValueError('Expect 2-D input label')

  colormap = create_pascal_label_colormap()

  if np.max(label) >= len(colormap):
    raise ValueError('label value too large.')

  return colormap[label]


def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    exit(1)
  return data[field_name]


def main():
  parser = argparse.ArgumentParser(
      description='Segment Camera Example',
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--host', type=str, default='10.10.10.1',
                      help='Hostname or IP Address of Coral Dev Board Micro')
  args = parser.parse_args()

  # Send RPC request
  response = requests.post(f'http://{args.host}:80/jsonrpc', json={
      'method': 'segment_from_camera',
      'jsonrpc': '2.0',
      'id': 0,
  }, timeout=10).json()

  # Get the image size
  result = get_field_or_die(response, 'result')
  width = get_field_or_die(result, 'width')
  height = get_field_or_die(result, 'height')

  # Decode the image and mask data
  image_data_base64 = get_field_or_die(result, 'base64_data')
  image_data = base64.b64decode(image_data_base64)
  im = Image.frombytes('RGB', (width, height), image_data, 'raw')

  mask_data_base64 = get_field_or_die(result, 'output_mask')
  mask_data = base64.b64decode(mask_data_base64)

  num_classes = len(mask_data) / (width * height)

  predicted_mask = np.frombuffer(
      mask_data, dtype=np.uint8, count=-1).reshape(width,
                                                   height,
                                                   int(num_classes))
  np.set_printoptions(threshold=sys.maxsize)
  predicted_mask = np.argmax(predicted_mask, axis=-1)
  mask_img = Image.fromarray(
      label_to_color_image(predicted_mask).astype(np.uint8))

  # Display the input image and segmentation results.
  output_img = Image.new('RGB', (2 * width, height))
  output_img.paste(im, (0, 0))
  output_img.paste(mask_img, (width, 0))
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
