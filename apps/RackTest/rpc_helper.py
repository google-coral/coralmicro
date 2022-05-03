"""Establishes communication to the RPC server on valiant device."""

import base64
import copy
import enum
import math
import os
from typing import Any

from PIL import Image
import requests


class Antenna(enum.IntEnum):
    INTERNAL = 0
    EXTERNAL = 1


class ValiantRPCHelper(object):
    """Test rack runner for Valiant.

      Implements methods for sending JSON-RPC requests to a Valiant device,
      and retrieving the response data.
    """

    def __init__(self, url, print_payloads=False):
        self.url = url
        self.print_payloads = print_payloads
        self.next_id = 0
        self.payload_template = {
            'jsonrpc': '2.0',
            'params': [],
            'method': '',
            'id': -1,
        }
        self.model_name = None
        self.image_resource_name = None
        self.image_width = None
        self.image_height = None

    def get_next_id(self):
        next_id = self.next_id
        self.next_id += 1
        return next_id

    def get_new_payload(self):
        new_payload = copy.deepcopy(self.payload_template)
        new_payload['id'] = self.get_next_id()
        return new_payload

    def send_rpc(self, json):
        if json['method'] and (json['jsonrpc']
                               == '2.0') and (json['id'] != -1) and (json['params']
                                                                     is not None):
            if self.print_payloads:
                print(json)
            return requests.post(self.url, json=json, timeout=20).json()
        raise ValueError('Missing key in RPC')

    def call_rpc_method(self, method):
        """Calls specified method in RPC server."""
        payload = self.get_new_payload()
        payload['method'] = method
        return self.send_rpc(payload)

    def wifi_set_antenna(self, antenna: Antenna):
        payload = self.get_new_payload()
        payload['method'] = 'wifi_set_antenna'
        payload['params'].append({'antenna': antenna})
        return self.send_rpc(payload)


    def wifi_scan(self):
        payload = self.get_new_payload()
        payload['method'] = 'wifi_scan'
        return self.send_rpc(payload)

    def set_tpu_power_state(self, enable):
        """Calls set_tpu_power_state to either turn on or off tpu power."""
        payload = self.get_new_payload()
        payload['method'] = 'set_tpu_power_state'
        payload['params'].append({'enable': enable})
        return self.send_rpc(payload)

    def call_tpu_stress_test(self, iterations):
        """Calls Posenet with specified number of iterations."""
        payload = self.get_new_payload()
        payload['method'] = 'posenet_stress_run'
        payload['params'].append({'iterations': iterations})
        print(f'Running: {payload}')
        return self.send_rpc(payload)

    def resource_max_chunk_size(self):
        return 2 ** 13

    def begin_upload_resource(self, resource_name, resource_size):
        """Begins the process of uploading a resource to the device.

        Informs the device of the name of the resource, and how large it will be.

        Args:
          resource_name: Name of the resource to be opened.
          resource_size: Number of bytes that the resource is.

        Returns:
          A JSON-RPC response.
        """
        payload = self.get_new_payload()
        payload['method'] = 'begin_upload_resource'
        payload['params'].append({
            'name': resource_name,
            'size': resource_size,
        })
        print(f'Uploading: {payload}')
        return self.send_rpc(payload)

    def upload_resource_chunk(self, resource_name, resource_data, offset):
        """Uploads a chunk of a resource to device.

        Uploads the data starting at `offset` in `resource_data` to the device.
        If the `begin_upload_resource` has not been invoked for the resource,
        the device should return an error.

        Args:
          resource_name: Name of an open resource.
          resource_data: Byte array containing the resource's data.
          offset: Offset of the chunk to upload.

        Returns:
          A JSON-RPC response.
        """
        max_chunk_size = self.resource_max_chunk_size()
        resource_size = len(resource_data)
        chunk_size = min(resource_size - offset, max_chunk_size)
        payload = self.get_new_payload()
        payload['method'] = 'upload_resource_chunk'
        payload['params'].append({
            'name':
                resource_name,
            'offset':
                offset,
            'data':
                base64.b64encode(resource_data[offset:offset + chunk_size]
                                 ).decode()
        })
        return self.send_rpc(payload)

    def upload_resource(self, resource_name: str, resource_data: Any,
                        resource_size: int) -> None:
        """Uploads resource to device.

        Args:
          resource_name: Name of an open resource.
          resource_data: Byte array containing the resource's data.
          resource_size: The size of the resource.

        Returns:
          None
        """
        self.begin_upload_resource(resource_name, resource_size)
        uploads = math.ceil(resource_size / self.resource_max_chunk_size())
        for x in range(0, uploads):
            if x % 50 == 0:
                print('[{} - {:.2f}% ({} of {})]'.format(resource_name, (x / uploads) * 100, x, uploads))
            self.upload_resource_chunk(resource_name, resource_data,
                                       x * self.resource_max_chunk_size())
        print(f'Finished uploading: {resource_name} to the valiant.')

    def upload_test_image(self, image_file_path: str) -> None:
        """Uploads test image to device.

        Args:
          image_file_path: The path to the image file to upload.

        Returns:
          None
        """
        self.image_resource_name = os.path.basename(image_file_path)
        with Image.open(image_file_path) as im:
            image_resource_data = im.convert('RGB').tobytes()
            (self.image_width, self.image_height) = im.size
        image_resource_size = len(image_resource_data)
        self.upload_resource(self.image_resource_name, image_resource_data,
                             image_resource_size)

    def delete_resource(self, resource_name):
        """Deletes an open resource from the device.

        Instructs the device that the resource is no longer needed, so it can
        reclaim the space.

        Args:
          resource_name: Name of the resource to be deleted.

        Returns:
          A JSON-RPC response.
        """
        payload = self.get_new_payload()
        payload['method'] = 'delete_resource'
        payload['params'].append({
            'name': resource_name,
        })
        print(f'Deleting: {payload}')
        return self.send_rpc(payload)

    def run_model(self, model_run_type: str, model_resource_name: str,
                  image_resource_name: str, image_width: int, image_height: int,
                  image_depth: int) -> Any:
        """Runs a model on the device.

        Instructs the device execute a previously uploaded model, using a previously
        uploaded image as the input tensor.

        Args:
          model_run_type: The type of the model ['run_classification_model',
            'run_detection_model'].
          model_resource_name: Name of the previously uploaded model.
          image_resource_name: Name of the previously uploaded image.
          image_width: Width of the previously uploaded image.
          image_height: Height of the previously uploaded image.
          image_depth: Depth of the previously uploaded image.

        Returns:
          A JSON-RPC response.
        """
        payload = self.get_new_payload()
        payload['method'] = model_run_type
        payload['params'].append({
            'model_resource_name': model_resource_name,
            'image_resource_name': image_resource_name,
            'image_width': image_width,
            'image_height': image_height,
            'image_depth': image_depth,
        })
        self.model_name = model_resource_name
        print(f'Running: {payload}')
        return self.send_rpc(payload)

    def run_m4_xor(self, value_to_xor):
        """Asks the secondary core to XOR a value.

        Args:
          value_to_xor: Value to be XOR'd.

        Returns:
          A JSON-RPC response.
        """
        payload = self.get_new_payload()
        payload['method'] = 'm4_xor'
        payload['params'].append({
            'value': str(value_to_xor),
        })
        return self.send_rpc(payload)

    def check_result_for_error(self, result):
        """Checks if the payload returned from RPC server includes an error."""
        print('Checking result for error message...')
        if 'error' in result:
            print('Error found. Printing message and returning false...')
            print(result['error']['message'])
            return False
        else:
            print('No errors in payload, returning true...')
            return True
