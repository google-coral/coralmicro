#!/usr/bin/python3

import base64
import requests

from PIL import Image

def main():
    url = "http://10.10.10.1:80/jsonrpc"

    payload = {
        "method": "serial_number",
        "params": [],
        "jsonrpc": "2.0",
        "id": 0,
    }

    response = requests.post(url, json=payload).json()
    print(response)

    payload = {
        "method": "take_picture",
        "params": [],
        "jsonrpc": "2.0",
        "id": 0,
    }

    response = requests.post(url, json=payload).json()
    assert(response['result']['base64_data'])
    image_data_base64 = response['result']['base64_data']
    width = response['result']['width']
    height = response['result']['height']
    image_data = base64.b64decode(image_data_base64)
    im = Image.frombytes('RGB', (width, height), image_data, 'raw')
    print(im)
    im.show()

if __name__ == '__main__':
    main()
