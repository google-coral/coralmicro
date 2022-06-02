#!/usr/bin/python3

import base64
import requests

from PIL import Image, ImageDraw


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

    response = requests.post(url, json=payload).json()
    result = get_field_or_die(response, 'result')
    image_data_base64 = get_field_or_die(result, 'base64_data')
    image_data = base64.b64decode(image_data_base64)
    width = get_field_or_die(result, 'width')
    height = get_field_or_die(result, 'height')
    im = Image.frombytes('RGB', (width, height), image_data, 'raw')
    detection = get_field_or_die(result, 'detection')
    left = get_field_or_die(detection, 'xmin') * width
    top = get_field_or_die(detection, 'ymin') * height
    right = get_field_or_die(detection, 'xmax') * width
    bottom = get_field_or_die(detection, 'ymax') * height
    draw = ImageDraw.Draw(im)
    draw.rectangle([left, top, right, bottom])
    text = f'ID: {detection["id"]} Score: {detection["score"]}'
    draw.text((left, bottom), text)

    im.show()


if __name__ == '__main__':
    main()
