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

from PIL import Image, ImageDraw

"""
Displays classification results received from the classify_camera server
running on a connected Dev Board Micro.

First load classify_camera example onto the Dev Board Micro:

    python3 scripts/flashtool.py -e classify_camera

Then start this client on your Linux computer that's connected via USB:

    python3 examples/classify_camera/classify_camera_client.py

You should see the image result appear in a new window.

Note that this example shows only the top classification result.
"""


def read_label_file(file_path):
    """Reads labels from a text file and returns it as a dictionary.
    This function supports label files with the following formats:
    + Each line contains id and description separated by colon or space.
      Example: ``0:cat`` or ``0 cat``.
    + Each line contains a description only. The returned label id's are based on
      the row number.
    Args:
      file_path (str): path to the label file.
    Returns:
      Dict of (int, string) which maps label id to description.
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    ret = {}
    for row_number, content in enumerate(lines):
        pair = re.split(r'[:\s]+', content.strip(), maxsplit=1)
        if len(pair) == 2 and pair[0].strip().isdigit():
            ret[int(pair[0])] = pair[1].strip()
        else:
            ret[row_number] = content.strip()
    return ret


def get_field_or_die(data, field_name):
    if field_name not in data:
        print(f'Unable to parse {field_name} from data: {data}\r\n')
        exit(1)
    return data[field_name]


def main():
    url = "http://10.10.10.1:80/jsonrpc"

    payload = {
        "method": "classify_from_camera",
        "jsonrpc": "2.0",
        "id": 0,
    }

    # Unpack the JSON result
    response = requests.post(url, json=payload).json()
    result = get_field_or_die(response, 'result')

    # Get the image size
    width = get_field_or_die(result, 'width')
    height = get_field_or_die(result, 'height')

    # Decode the image data
    image_data_base64 = get_field_or_die(result, 'base64_data')
    image_data = base64.b64decode(image_data_base64)
    bayered = get_field_or_die(result, 'bayered')
    if bayered:
        import cv2
        import numpy as np
        np_data = np.frombuffer(image_data, dtype=np.uint8)
        np_data = np_data.reshape(width, height)
        debayered = cv2.cvtColor(np_data, cv2.COLOR_BAYER_BG2BGR)
        im = Image.fromarray(debayered)
    else:
        im = Image.frombytes('RGB', (width, height), image_data, 'raw')

    # Display the image with the classification result.
    draw = ImageDraw.Draw(im)
    labels = read_label_file('models/imagenet_labels.txt')
    if 'id' in result:
        text = f'{labels[result["id"]]}\nScore: {result["score"]}'
        print(text)
        draw.text((0, 0), text)

    im.show()


if __name__ == '__main__':
    main()
