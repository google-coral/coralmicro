#!/usr/bin/python3

import argparse
import base64
import inspect
import itertools
import json
import requests
import sys
import time
from PIL import Image

parser = argparse.ArgumentParser(description='Image server client')
parser.add_argument('--usb_ip', type=str, required=True, help='Dev Board Micro USB IP address.')
parser.add_argument('--ethernet', action='store_true', help='Get image from ethernet ip.')
parser.add_argument('--ethernet_ip', type=str, required=False, help='Dev Board Micro Ethernet IP address.')
parser.add_argument('--image_width', type=int, default=700, help="Specify image width.")
parser.add_argument('--image_height', type=int, default=700, help="Specify image height.")
parser.add_argument('--wifi', action='store_true', help='Get image from wifi ip.')
parser.add_argument('--wifi_ssid', type=str, required=False, help='wifi network name')
parser.add_argument('--wifi_psk', type=str, required=False, help='wifi network password')

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
def wifi_connect(ip, ssid, password='', retries=5):
  """Connect to wifi
  Args:
    ip: the usb ip address
    ssid: the network name
    password: the network password
    retries: the amount of connection attempts
  Returns:
    A JSON-RPC result packet with no extra data, or JSON-RPC error.
    Example:
      {'id': 1, 'result': {}}
  """

@rpc
def wifi_get_ip(ip):
  """Get wifi ip
  Args:
    ip: the usb ip address.
  Returns:
    A JSON-RPC result packet with the ethernet ip address.
    Example:
       {'id': 24, 'result': {'ip': '192.168.1.117'}}
  """

@rpc
def wifi_get_status(ip):
  """Ge wifi status
  Args:
    ip: the usb ip address
  Returns:
    A JSON-RPC result packet with the connection status.
    Example:
      {'id': 24, 'result': {'status': true}}
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

def parse_wifi_ip(response):
  print(f'response:\n{json.dumps(response, indent=2)}')
  result = get_field_or_die(response, 'result')
  return get_field_or_die(result, 'ip')

def get_field_or_die(data, field_name):
  if field_name not in data:
    print(f'Unable to parse {field_name} from data: {data}\r\n')
    sys.exit(1)
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
  elif args.wifi:
    wifi_ssid = args.wifi_ssid
    wifi_psk = args.wifi_psk
    if wifi_psk is None:
      wifi_psk = ""
    print(wifi_connect(args.usb_ip, wifi_ssid, wifi_psk, 20))
    time.sleep(15)

    status = get_field_or_die(get_field_or_die(wifi_get_status(args.usb_ip), 'result'), 'status')
    if not status:
      print(f'Failed to connect to Wi-Fi!')
      sys.exit(1)

    wifi_ip = parse_wifi_ip(wifi_get_ip(args.usb_ip))
    print(f'Wifi ip: {wifi_ip}')
    display_image(get_image_from_camera(wifi_ip, width, height), width, height)
  else:  # USB
    display_image(get_image_from_camera(args.usb_ip, width, height), width, height)


if __name__ == '__main__':
  main()
