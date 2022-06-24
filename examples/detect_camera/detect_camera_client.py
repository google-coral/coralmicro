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

from PIL import Image, ImageDraw

"""
Displays object detection results received from the detect_camera server
running on a connected Dev Board Micro.

First load detect_camera example onto the Dev Board Micro:

    python3 scripts/flashtool.py -b build -e detect_camera

Then start this client on your computer:

    python3 examples/detect_camera/detect_camera_client.py

You should see the image result appear in a new window.

Note that this example shows only the top detection result.
"""

def get_field_or_die(data, field_name):
    if field_name not in data:
        print(f'Unable to parse {field_name} from data: {data}\r\n')
        exit(1)
    return data[field_name]

def main():
    url = "http://10.10.10.1:80/jsonrpc"

    payload = {
        "method": "detect_from_camera",
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
    main()
