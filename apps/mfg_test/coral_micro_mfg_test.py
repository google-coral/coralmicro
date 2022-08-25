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
import enum
import inspect
import itertools
import requests


class PMICRails(enum.IntEnum):
  CAM_2V8 = 0
  CAM_1V8 = 1
  MIC_1V8 = 2


class LEDs(enum.IntEnum):
  POWER = 0
  USER = 1
  TPU = 2


class Antenna(enum.IntEnum):
  INTERNAL = 0
  EXTERNAL = 1


def rpc(func):
  def rpc_impl(*args, **kwargs):
    params = inspect.getcallargs(func, *args, **kwargs)
    self = params.pop('self')
    return self.send_rpc(func.__name__, params)
  return rpc_impl


class CoralMicroMFGTest(object):
  """Manufacturing test runner for Dev Board Micro.

  Implements methods for sending JSON-RPC requests to a Dev Board Micro device,
  and retrieving the response data.
  """

  def __init__(self, url, print_payloads=False):
    self.url = url
    self.print_payloads = print_payloads
    self.ids = itertools.count()

  def send_rpc(self, method, params=None):
    if not method:
      raise ValueError('Missing "method"')
    payload = {
        'jsonrpc': '2.0',
        'method': method,
        'params': [params or {}],
        'id': next(self.ids),
    }
    if self.print_payloads:
      print(payload)
    return requests.post(self.url, json=payload).json()

  @rpc
  def get_serial_number(self):
    """Gets the serial number from the device

    Returns:
      A JSON-RPC result packet, with a 'serial_number' result field, or a JSON-RPC error.
      Example:
        {'id': 0, 'result': {'serial_number': '350a280e828f2c4f'}}
    """

  def set_pmic_rail_state(self, rail, enable):
    """Sets the state of a PMIC rail.

    We can't use decorator here because Manufacturing framework uses Python 2.7, and the
    decorator produces incompatible output for enum parameters. Create the RPC manually.

    Args:
      rail: A value from the PMICRails enum
      enable: A boolean value representing the desired rail state.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
      Example:
        {'id': 1, 'result': {}}
    """
    return self.send_rpc('set_pmic_rail_state', {
        'rail': rail.value,
        'enable': enable,
    })

  def set_led_state(self, led, enable):
    """Sets the state of an LED.

    We can't use decorator here because Manufacturing framework uses Python 2.7, and the
    decorator produces incompatible output for enum parameters. Create the RPC manually.

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
    return self.send_rpc('set_led_state', {
        'led': led.value,
        'enable': enable,
    })

  @rpc
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

  @rpc
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

  @rpc
  def get_gpio(self, pin):
    """Gets a GPIO's input value.

    Args:
      pin: Pin to set value for.

    Returns:
      A JSON-RPC result packet with a value parameter, or JSON-RPC error.
      Example:
        {'id': 40, 'result': {'value': 1}}
    """

  @rpc
  def capture_test_pattern(self):
    """Captures a test pattern from the camera sensor.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
      Example:
        {'id': 1, 'result': {}}
    """

  @rpc
  def capture_audio(self, sample_rate_hz=16000, duration_ms=1000,
                    num_buffers=2, buffer_size_ms=50):
    """Captures one second of audio.

    Returns:
      A JSON-RPC result packet with a data parameter containing base64-encoded audio data (32-bit signed PCM @ 16000Hz), or JSON-RPC error.
    """

  @rpc
  def set_tpu_power_state(self, enable):
    """Sets the power state of the TPU.

    Args:
      enable: Whether to enable or disable the power.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
      Example:
        {'id': 1, 'result': {}}
    """

  @rpc
  def run_testconv1(self):
    """Executes the TestConv1 model on TPU.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
      Example:
        {'id': 1, 'result': {}}

    Note:
      The TPU power must be enabled before calling this.
    """

  @rpc
  def get_tpu_chip_ids(self):
    """Not implemented"""

  @rpc
  def check_tpu_alarm(self):
    """Not implemented"""

  @rpc
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

  @rpc
  def test_sdram_pattern(self):
    """Runs a pattern test in SDRAM.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
      Example:
        {'id': 1, 'result': {}}
    """

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
    return self.send_rpc('write_file', {
        'filename': filename,
        'data': base64.b64encode(data.encode()).decode(),
    })

  @rpc
  def read_file(self, filename):
    """Reads data from filesystem.

    Args:
      filename: Path to file

    Returns:
      A JSON-RPC result packed with data param, containing base64-encoded data from the filesystem, or JSON-RPC error.
      Example:
        {'id': 24, 'result': {'data': 'aGVsbG8gbWZndGVzdA=='}}
    """

  @rpc
  def check_a71ch(self):
    """Checks the status of the A71CH peripheral.

    Returns:
      A JSON-RPC result packet.
    """

  @rpc
  def wifi_scan(self):
    """Scans for all SSIDs.

    Returns: A JSON-RPC result with a list of SSIDs, or JSON-RPC error. Example: {'id': 1, 'result': {'SSIDs': [
    'GoogleGuest', 'GoogleGuest-IPv4', ...]}}
    """

  @rpc
  def wifi_get_ap(self, name):
    """Scans for an access point of the provided name.

    Args:
      name: Name of the access point to search for.

    Returns:
      A JSON-RPC result with the signal strength of the network, or JSON-RPC error.
      Example:
        {'id': 1, 'result': {'signal_strength': -52}}
    """

  def wifi_set_antenna(self, antenna):
    """Sets which antenna to use for WiFi.

    We can't use decorator here because Manufacturing framework uses Python 2.7, and the
    decorator produces incompatible output for enum parameters. Create the RPC manually.

    Args:
      antenna: A value from the Antenna Enum.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
      Example:
        {'id': 2, 'result': {}}
    """
    return self.send_rpc('wifi_set_antenna', {
        'antenna': antenna.value,
    })

  @rpc
  def ble_find(self, address):
    """Scans for a BLE device with the given MAC address.

    Args:
      address: The MAC address to search for (as a string).

    Returns:
      A JSON-RPC result packet with signal strength, or JSON-RPC error.
      Example:
        {'id':  4, 'result': {'signal_strength': -58}}
    """

  @rpc
  def ble_scan(self):
    """Scans for a BLE devices.

    Args:
      address: The MAC address to search for (as a string).

    Returns:
      A JSON-RPC result packet with device found and it's signal strength, or JSON-RPC error.
      Example:
        {'id':  4, 'result': {'address': '73:85:B5:4E:E0:24', 'signal_strength': -99}}
    """

  @rpc
  def eth_get_ip(self):
    """Gets the IP address assigned to the Ethernet interface.

    Returns:
      A JSON-RPC result packet with the ip address, or JSON-RPC error.
      Example:
        {'id': 0, 'result': {'ip': '172.16.243.96'}}
    """

  @rpc
  def eth_write_phy(self, reg, val):
    """Writes `val` to the Ethernet PHY at location `reg`.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
    """

  @rpc
  def fuse_mac_address(self, address):
    """Writes a MAC address into device fuses.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
    """

  @rpc
  def read_mac_address(self):
    """Reads the MAC address from device fuses.

    Returns:
      A JSON-RPC result packet with the address. An all-zero
      address indicates that no address has been fused.
      Example:
        {'id': 44, 'result': {'address': '7C:D9:5C:AA:BB:CC'}}
    """

  @rpc
  def iperf_start(self, is_server, server_ip_address=None, duration=10, udp=False, bandwidth=1024*1024):
    """Starts Iperf.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
    """

  @rpc
  def iperf_stop(self):
    """Stops Iperf.

    Returns:
      A JSON-RPC result packet with no extra data, or JSON-RPC error.
    """


if __name__ == '__main__':
  """
  Sample runner for the RPCs implemented in this class.
  """
  import argparse
  parser = argparse.ArgumentParser(description='Dev Board Micro MFGTest')
  parser.add_argument('--ip_address', type=str,
                      required=False, default='10.10.10.1')
  args = parser.parse_args()
  mfg_test = CoralMicroMFGTest(
      f'http://{args.ip_address}:80/jsonrpc', print_payloads=True)
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
  # Should return failure, as power is off.
  print(mfg_test.run_testconv1())
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
  print(mfg_test.check_a71ch())

  capture_audio_result = mfg_test.capture_audio()
  import tempfile
  with tempfile.NamedTemporaryFile(mode='w+b', delete=False) as f:
    print(f.name)
    f.write(base64.b64decode(capture_audio_result['result']['data']))

  test_file_data = 'hello mfgtest'
  print(mfg_test.write_file('/mfgtest.txt', test_file_data))
  read_file_result = mfg_test.read_file('/mfgtest.txt')
  print(read_file_result)
  assert(base64.b64decode(
      read_file_result['result']['data']).decode() == test_file_data)

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

  print(mfg_test.read_mac_address())
