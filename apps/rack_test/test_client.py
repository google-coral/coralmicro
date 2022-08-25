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
"""Quick test script to run the RackTest locally without involving g3. Note: this script is only meant for quick local
prototyping, the real test are on google3.

- classification:
  python3 apps/RackTest/test_client.py --model models/mobilenet_v1_1.0_224_quant_edgetpu.tflite --test_image test_data/cat.bmp --test classification
- detection:
  python3 apps/RackTest/test_client.py --model models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite --test_image test_data/cat.bmp --test detection
- wifi_tests:
  python3 apps/RackTest/test_client.py --test wifi_tests
- stress_test:
  python3 apps/RackTest/test_client.py --test stress_test
- ble_tests:
  python3 apps/RackTest/test_client.py --test ble_tests
"""
import argparse
import os
import json
import time
from rpc_helper import CoralMicroRPCHelper
from rpc_helper import Antenna

parser = argparse.ArgumentParser(
    description='Local client for the rack test to quickly test on local devices.')
parser.add_argument('--host', type=str, default='10.10.10.1',
                    help='Ip address of the Dev Board Micro')
parser.add_argument('--port', type=int, default=80,
                    help='Port of the Dev Board Micro')
parser.add_argument('--test', type=str, default='detection',
                    help='Test to run, currently support ["detection", "classification", "segmentation", "wifi_tests", "stress_test", "crypto_tests", "ble_tests"]')
parser.add_argument('--test_image', type=str, default='test_data/cat.bmp')
parser.add_argument('--model', type=str,
                    default='models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite')
args = parser.parse_args()


def run_model(url, test):
  rpc_helper = CoralMicroRPCHelper(url)
  test_image = args.test_image
  if not os.path.exists(test_image):
    print(f"Test image: {test_image} doesn't exist")
    return

  rpc_helper.upload_test_image(args.test_image)
  model_path = args.model
  if not os.path.exists(model_path):
    print(f"{model_path} doesn't exist")
    return
  with open(model_path, "rb") as f:
    model_data = f.read()
  model_data_len = len(model_data)
  model_name = model_path.split('/')[-1]
  rpc_helper.upload_resource(model_name, model_data, model_data_len)
  if test == 'detection':
    response = rpc_helper.run_model('run_detection_model', model_name, rpc_helper.image_resource_name,
                                    rpc_helper.image_width, rpc_helper.image_height, 3)
  elif test == 'classification':
    response = rpc_helper.run_model('run_classification_model', model_name, rpc_helper.image_resource_name,
                                    rpc_helper.image_width, rpc_helper.image_height, 3)
  elif test == 'segmentation':
    response = rpc_helper.run_model('run_segmentation_model', model_name, rpc_helper.image_resource_name,
                                    rpc_helper.image_width, rpc_helper.image_height, 3)
  else:
    raise ValueError(f'{test} not supported')
  print(f'Success: {response}')
  rpc_helper.delete_resource(rpc_helper.image_resource_name)
  rpc_helper.delete_resource(model_name)


def run_wifi_test(url):
  rpc_helper = CoralMicroRPCHelper(url)
  print('Setting external antenna')
  print(rpc_helper.wifi_set_antenna(Antenna.EXTERNAL))
  print('Setting internal antenna')
  print(rpc_helper.wifi_set_antenna(Antenna.INTERNAL))
  print('Wifi Scan')
  print(json.dumps(rpc_helper.wifi_scan(), indent=2))
  for ssid, password in {'GoogleGuest-Legacy': '', 'GoogleGuest-IPv4': ''}.items():
    print(f'Connecting to {ssid}:{password}')
    print(rpc_helper.wifi_connect(ssid=ssid, psk=password, retries=5))
    time.sleep(10)
    print('Checking wifi status')
    print(rpc_helper.wifi_get_status())
    print(f'Getting wifi ip')
    print(rpc_helper.wifi_get_ip())
    print(f'Disconnecting from {ssid}')
    print(rpc_helper.wifi_disconnect())
    time.sleep(1)
    print('Checking wifi status')
    print(rpc_helper.wifi_get_status())


def run_stress_test(url):
  rpc_helper = CoralMicroRPCHelper(url, print_payloads=True)
  result = rpc_helper.call_tpu_stress_test(500)
  print(result)


def run_crypto_test(url):
  rpc_helper = CoralMicroRPCHelper(url)
  print('Init Crypto')
  print(rpc_helper.call_rpc_method('a71ch_init'))
  print('Getting Crypto UID')
  print(rpc_helper.call_rpc_method('a71ch_get_uid'))
  for num_bytes in (2**p for p in range(1, 7)):
    print(f'Getting {num_bytes} random bytes')
    print(rpc_helper.a71ch_get_random(num_bytes))

  file_name = '/models/testconv1-edgetpu.tflite'
  stored_sha_name = 'testconv1-stored-sha'
  print(f'Getting sha for {file_name}')
  print(rpc_helper.a71ch_get_sha_256(file_name, stored_sha_name))
  stored_signature_name = 'testconv1-stored-signature'
  for key_index in range(4):
    print(f'key_index: {key_index}')
    print('Getting Crypto Ecc pubkey')
    print(rpc_helper.a71ch_get_public_ecc_key(idx=key_index))
    print('Getting Signature')
    print(rpc_helper.a71ch_get_ecc_signature(stored_sha_name=stored_sha_name,
                                             stored_signature_name=stored_signature_name,
                                             idx=key_index))
    print(f'Verifying signature...')
    print(rpc_helper.a71ch_ecc_verify(stored_sha_name=stored_sha_name,
                                      stored_signature_name=stored_signature_name,
                                      idx=key_index))


def run_ble_test(url):
  rpc_helper = CoralMicroRPCHelper(url)
  print('Ble Scan')
  print(json.dumps(rpc_helper.ble_scan(), indent=2))


def main():
  url = f"http://{args.host}:{args.port}/jsonrpc"
  print(f"Dev Board Micro url: {url}")
  if args.test in ["classification", "detection", "segmentation"]:
    run_model(url, args.test)
  elif args.test == "wifi_tests":
    run_wifi_test(url)
  elif args.test == "stress_test":
    run_stress_test(url)
  elif args.test == "crypto_tests":
    run_crypto_test(url)
  elif args.test == "ble_tests":
    run_ble_test(url)
  else:
    print('Test not supported')
    parser.print_help()
    exit(1)


if __name__ == '__main__':
  main()
