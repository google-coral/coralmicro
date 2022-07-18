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
import re
import requests
import numpy as np
import scipy.ndimage
import scipy.misc

from PIL import Image, ImageDraw

_HEATMAP_MASK_LENGTH = 21

def get_field_or_die(data, field_name):
    if field_name not in data:
        print(f'Unable to parse {field_name} from data: {data}\r\n')
        exit(1)
    return data[field_name]

def main():
    url = "http://10.10.10.1:80/jsonrpc"

    payload = {
        "method": "run_bodypix",
        "jsonrpc": "2.0",
        "id": 0,
    }

    # Unpack the JSON result
    response = requests.post(url, json=payload).json()
    result = get_field_or_die(response, 'result')

    # Get the image size
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
    predicted_mask = predicted_mask.reshape(21, 21)

    # Blow up the heatmap to match the image size, and turn it red
    rgb_heatmap = np.dstack([predicted_mask[:,:]]*3)
    rgb_heatmap[:,:,1:] = 0

    rescale_factor = [
      width/_HEATMAP_MASK_LENGTH,
      height/_HEATMAP_MASK_LENGTH,
      1]
    rgb_heatmap = scipy.ndimage.zoom(rgb_heatmap, rescale_factor, order=0)

    heatmap_im = Image.fromarray(rgb_heatmap)

    # Display the input image and segmentation results.
    output_img = Image.new('RGB', (2 * width, height))
    output_img.paste(im, (0, 0))
    output_img.paste(heatmap_im, (width, 0))
    output_img.show()

if __name__ == '__main__':
    main()
