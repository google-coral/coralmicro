#!/usr/bin/python3
"""Quick test script to run the RackTest locally without involving g3. Note: this script is only meant for quick local
prototyping, the real test are on google3.

- classification:
  python3 apps/RackTest/test_client.py --model models/mobilenet_v1_1.0_224_quant_edgetpu.tflite --test_image test_data/cat.bmp --test classification
- detection:
  python3 apps/RackTest/test_client.py --model models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite --test_image test_data/cat.bmp --test detection
- wifi_tests:
  python3 apps/RackTest/test_client.py --test wifi_tests
"""
import argparse
import os
import json
import time
from rpc_helper import CoralMicroRPCHelper
from rpc_helper import Antenna

parser = argparse.ArgumentParser(
    description='Local client for the rack test to quickly test on local devices.')
parser.add_argument('--host', type=str, default='10.10.10.1', help='Ip address of the Dev Board Micro')
parser.add_argument('--port', type=int, default=80, help='Port of the Dev Board Micro')
parser.add_argument('--test', type=str, default='detection',
                    help='Test to run, currently support ["detection", "classification", "wifi_tests]')
parser.add_argument('--test_image', type=str, default='test_data/cat.bmp')
parser.add_argument('--model', type=str, default='models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite')
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
    response = {}
    if test == 'detection':
        response = rpc_helper.run_model('run_detection_model', model_name, rpc_helper.image_resource_name,
                                        rpc_helper.image_width, rpc_helper.image_height, 3)
    elif test == 'classification':
        response = rpc_helper.run_model('run_classification_model', model_name, rpc_helper.image_resource_name,
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

def main():
    url = f"http://{args.host}:{args.port}/jsonrpc"
    print(f"Dev Board Micro url: {url}")
    if args.test in ["classification", "detection"]:
        run_model(url, args.test)
    elif args.test == "wifi_tests":
        run_wifi_test(url)
    else:
        print('Test not supported')
        parser.print_help()
        exit(1)


if __name__ == '__main__':
    main()
