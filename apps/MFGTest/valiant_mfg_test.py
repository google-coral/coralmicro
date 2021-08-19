#!/usr/bin/python3

from copy import deepcopy
from enum import Enum
import base64
import requests

class PMICRails(Enum):
    CAM_2V8 = 0
    CAM_1V8 = 1
    MIC_1V8 = 2

class LEDs(Enum):
    POWER = 0
    USER = 1
    TPU = 2

class ValiantMFGTest(object):
    """Manufacturing test runner for Valiant.

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

    def get_next_id(self):
        next_id = self.next_id
        self.next_id += 1
        return next_id

    def get_new_payload(self):
        new_payload = deepcopy(self.payload_template)
        new_payload['id'] = self.get_next_id()
        return new_payload

    def send_rpc(self, json):
        if json['method'] and (json['jsonrpc'] == '2.0') and (json['id'] != -1) and (json['params'] != None):
            if self.print_payloads:
                print(json)
            return requests.post(self.url, json=json).json()
        raise ValueError('Missing key in RPC')

    def get_serial_number(self):
        """Gets the serial number from the device

        Returns:
          A JSON-RPC result packet, with a 'serial_number' result field, or a JSON-RPC error.
          Example:
            {'id': 0, 'result': {'serial_number': '350a280e828f2c4f'}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'get_serial_number'
        result = mfg_test.send_rpc(payload)
        return result

    def set_pmic_rail_state(self, rail, enable):
        """Sets the state of a PMIC rail.

        Args:
          rail: A value from the PMICRails enum
          enable: A boolean value representing the desired rail state.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'set_pmic_rail_state'
        payload['params'].append({
            'rail': rail.value,
            'enable': enable,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def set_led_state(self, led, enable):
        """Sets the state of an LED.

        Args:
          led: A value from the LEDs enum
          enable: A boolean value representing the desired LED state.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}

        Notes:
          Setting the state of the TPU LED requires the TPU power to be enabled.
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'set_led_state'
        payload['params'].append({
            'led': led.value,
            'enable': enable,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def set_pin_pair_to_gpio(self, output_pin, input_pin):
        """Sets a pair of pins to GPIO mode for loopback testing.

        Args:
          output_pin: Pin to set to output mode
          input_pin: Pin to set to input mode

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'set_pin_pair_to_gpio'
        payload['params'].append({
            'output_pin': output_pin,
            'input_pin': input_pin,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def set_gpio(self, pin, enable):
        """Sets a GPIO's output value.

        Args:
          pin: Pin to set value for.
          enable: Whether to drive high or low.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'set_gpio'
        payload['params'].append({
            'pin': pin,
            'enable': enable,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def get_gpio(self, pin):
        """Gets a GPIO's input value.

        Args:
          pin: Pin to set value for.

        Returns:
          A JSON-RPC result packet with a value parameter, or JSON-RPC error.
          Example:
            {'id': 40, 'result': {'value': 1}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'get_gpio'
        payload['params'].append({
            'pin': pin,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def capture_test_pattern(self):
        """Captures a test pattern from the camera sensor.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'capture_test_pattern'
        result = mfg_test.send_rpc(payload)
        return result

    def capture_audio(self):
        """Captures one second of audio.

        Returns:
          A JSON-RPC result packet with a data parameter containing base64-encoded audio data (16-bit signed PCM @ 16000Hz), or JSON-RPC error.
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'capture_audio'
        result = mfg_test.send_rpc(payload)
        return result

    def set_tpu_power_state(self, enable):
        """Sets the power state of the TPU.

        Args:
          enable: Whether to enable or disable the power.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'set_tpu_power_state'
        payload['params'].append({
            'enable': enable,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def run_testconv1(self):
        """Executes the TestConv1 model on TPU.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}

        Note:
          The TPU power must be enabled before calling this.
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'run_testconv1'
        result = mfg_test.send_rpc(payload)
        return result

    def get_tpu_chip_ids(self):
        """Not implemented"""
        payload = mfg_test.get_new_payload()
        payload['method'] = 'get_tpu_chip_ids'
        result = mfg_test.send_rpc(payload)
        return result

    def check_tpu_alarm(self):
        """Not implemented"""
        payload = mfg_test.get_new_payload()
        payload['method'] = 'check_tpu_alarm'
        result = mfg_test.send_rpc(payload)
        return result

    def set_dac_value(self, counts):
        """Sets the output value of the DAC pin.

        Args:
          counts: A value from 0-4095, representing the desired output voltage.
                  The DAC can generate voltages from 0-1.8V.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'set_dac_value'
        payload['params'].append({
            'counts': counts,
        })
        result = mfg_test.send_rpc(payload)
        return result

    def test_sdram_pattern(self):
        """Runs a pattern test in SDRAM.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'test_sdram_pattern'
        result = mfg_test.send_rpc(payload)
        return result

    def write_file(self, filename, data):
        """Writes data to filesystem.

        Args:
          filename: Path to file
          data: Data to write to file.

        Returns:
          A JSON-RPC result packet with no extra data, or JSON-RPC error.
          Example:
            {'id': 1, 'result': {}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'write_file'
        payload['params'].append({
            'filename': filename,
            'data': base64.b64encode(data.encode()).decode(),
        })
        result = mfg_test.send_rpc(payload)
        return result

    def read_file(self, filename):
        """Reads data from filesystem.

        Args:
          filename: Path to file

        Returns:
          A JSON-RPC result packed with data param, containing base64-encoded data from the filesystem, or JSON-RPC error.
          Example:
            {'id': 24, 'result': {'data': 'aGVsbG8gbWZndGVzdA=='}}
        """
        payload = mfg_test.get_new_payload()
        payload['method'] = 'read_file'
        payload['params'].append({
            'filename': filename
        })
        result = mfg_test.send_rpc(payload)
        return result

if __name__ == '__main__':
    """
    Sample runner for the RPCs implemented in this class.
    """
    mfg_test = ValiantMFGTest('http://10.10.10.1:80/jsonrpc', print_payloads=True)
    print(mfg_test.get_serial_number())
    print(mfg_test.set_pmic_rail_state(PMICRails.CAM_2V8, True))
    print(mfg_test.set_pmic_rail_state(PMICRails.CAM_1V8, True))
    print(mfg_test.set_pmic_rail_state(PMICRails.MIC_1V8, True))
    print(mfg_test.set_pmic_rail_state(PMICRails.MIC_1V8, False))
    print(mfg_test.set_pmic_rail_state(PMICRails.CAM_1V8, False))
    print(mfg_test.set_pmic_rail_state(PMICRails.CAM_2V8, False))
    print(mfg_test.set_led_state(LEDs.POWER, True))
    print(mfg_test.set_led_state(LEDs.USER, True))
    print(mfg_test.set_led_state(LEDs.USER, False))
    print(mfg_test.set_led_state(LEDs.POWER, False))
    print(mfg_test.capture_test_pattern())
    print(mfg_test.set_tpu_power_state(True))
    print(mfg_test.set_led_state(LEDs.TPU, True))
    print(mfg_test.run_testconv1())
    print(mfg_test.set_led_state(LEDs.TPU, False))
    print(mfg_test.set_tpu_power_state(False))
    print(mfg_test.get_tpu_chip_ids())
    print(mfg_test.check_tpu_alarm())
    print(mfg_test.set_dac_value(2048))
    print(mfg_test.set_dac_value(0))
    print(mfg_test.test_sdram_pattern())

    capture_audio_result = mfg_test.capture_audio()
    import tempfile
    with tempfile.NamedTemporaryFile(mode='w+b', delete=False) as f:
        print(f.name)
        f.write(base64.b64decode(capture_audio_result['result']['data']))

    test_file_data = 'hello mfgtest'
    print(mfg_test.write_file('/mfgtest.txt', test_file_data))
    read_file_result = mfg_test.read_file('/mfgtest.txt')
    print(read_file_result)
    assert(base64.b64decode(read_file_result['result']['data']).decode() == test_file_data)

    print(mfg_test.set_pin_pair_to_gpio(169, 171))
    print(mfg_test.set_gpio(169, True))
    print(mfg_test.get_gpio(171))
    print(mfg_test.set_gpio(169, False))
    print(mfg_test.get_gpio(171))
    print(mfg_test.set_gpio(169, True))
    print(mfg_test.get_gpio(171))
    print(mfg_test.set_gpio(169, False))
    print(mfg_test.get_gpio(171))

    print(mfg_test.set_pin_pair_to_gpio(171, 169))
    print(mfg_test.set_gpio(171, True))
    print(mfg_test.get_gpio(169))
    print(mfg_test.set_gpio(171, False))
    print(mfg_test.get_gpio(169))
    print(mfg_test.set_gpio(171, True))
    print(mfg_test.get_gpio(169))
    print(mfg_test.set_gpio(171, False))
    print(mfg_test.get_gpio(169))
