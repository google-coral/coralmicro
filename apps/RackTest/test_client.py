#!/usr/bin/python3
"""Quick test script to run the RackTest locally without involving g3. Currently only support classification and
detection test.

- classification:
  python3 apps/RackTest/test_client.py --model models/mobilenet_v1_1.0_224_quant_edgetpu.tflite --test_image test_data/cat.bmp --test classification
- detection:
  python3 apps/RackTest/test_client.py --model models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite --test_image test_data/cat.bmp --test detection
"""
import argparse
import os
from rpc_helper import ValiantRPCHelper

parser = argparse.ArgumentParser(
    description='Local client for the rack test to quickly test on local valiant devices without g3.')
parser.add_argument('--host', type=str, default='10.10.10.1', help='Ip address of the valiant')
parser.add_argument('--port', type=int, default=80, help='Port of the valiant')
parser.add_argument('--test', type=str, default='detection',
                    help='Test to run, currently support ["detection", "classification"]')
parser.add_argument('--test_image', type=str, default='test_data/cat.bmp')
parser.add_argument('--model', type=str, default='models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite')
args = parser.parse_args()


def display_result(result, test):
    outcome = "failed" if "error" in result else "passed"
    print(f'{test} {outcome}: {result}')


def run_model(url, test):
    rpc_helper = ValiantRPCHelper(url)
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


def main():
    supported_tests = ["classification", "detection"]
    if args.test not in supported_tests:
        parser.print_help()
        exit(1)
    url = f"http://{args.host}:{args.port}/jsonrpc"
    print(f"Valiant url: {url}")
    run_model(url, args.test)


if __name__ == '__main__':
    main()
