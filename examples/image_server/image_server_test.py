#!/usr/bin/python3

import argparse
import base64
import inspect
import json
import requests
import itertools
from PIL import Image

parser = argparse.ArgumentParser(description='Image server client')
parser.add_argument('--usb_ip', type=str, required=True, help='Dev Board Micro USB IP address.')
parser.add_argument('--ethernet', action='store_true', help='Get image from ethernet ip.')
parser.add_argument('--ethernet_ip', type=str, required=False, help='Dev Board Micro Ethernet IP address.')
parser.add_argument('--image_width', type=int, default=700, help="Specify image width.")
parser.add_argument('--image_height', type=int, default=700, help="Specify image height.")

rpc_id = itertools.count()


def rpc(func):
  def rpc_impl(*args, **kwargs):
    params = inspect.getcallargs(func, *args, **kwargs)
    url = f"http://{params.pop('ip')}:80/jsonrpc"
    payload = {
      'id': next(rpc_id),
      'jsonrpc': '2.0',
      'method': func.__name__,
      'params': [params or {}],
    }
    print(f'url: {url}')
    print(f'payload:\n{json.dumps(payload, indent=2)}')
    return requests.post(url, json=payload).json()

  return rpc_impl


@rpc
def get_ethernet_ip(ip):
  """Get ethernet ip address.
  Args:
    ip: the usb ip address.
  Returns:
    A JSON-RPC result packet with the ethernet ip address.
    Example:
       {'id': 24, 'result': {'ethernet_ip': '192.168.1.117'}}
  """


@rpc
def get_image_from_camera(ip, width, height):
  """Get image from camera.
  Args:
    width:
    height: A boolean value representing the desired rail stat
  Returns:
    A JSON-RPC result packet with no extra data, or JSON-RPC error.
    Example:
      {'id': 1, 'result': {}}
  """


def display_image(response, width, height):
  result = get_field_or_die(response, 'result')
  image_data_base64 = get_field_or_die(result, 'base64_data')
  image_data = base64.b64decode(image_data_base64)
  im = Image.frombytes('RGB', (width, height), image_data, 'raw')
  im.show()


def parse_ethernet_ip(response):
  result = get_field_or_die(response, 'result')
  return get_field_or_die(result, 'ethernet_ip')


def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    exit(1)
  return data[field_name]


def main():
  args = parser.parse_args()
  width = args.image_width
  height = args.image_height

  if args.ethernet:
    ethernet_ip = args.ethernet_ip
    if ethernet_ip is None:
      ethernet_ip = parse_ethernet_ip(get_ethernet_ip(args.usb_ip))
    print(f'Ethernet ip: {ethernet_ip}')
    display_image(get_image_from_camera(ethernet_ip, width, height), width, height)
  else:  # USB
    display_image(get_image_from_camera(args.usb_ip, width, height), width, height)


if __name__ == '__main__':
  main()
