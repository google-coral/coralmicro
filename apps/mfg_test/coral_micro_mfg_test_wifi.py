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

from coral_micro_mfg_test import CoralMicroMFGTest, Antenna

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
  print(mfg_test.wifi_set_antenna(Antenna.INTERNAL))
  print(mfg_test.wifi_scan())
  print(mfg_test.wifi_get_ap('GoogleGuest'))
  print(mfg_test.wifi_set_antenna(Antenna.EXTERNAL))
  print(mfg_test.wifi_get_ap('GoogleGuest'))
  print(mfg_test.ble_scan())
  print(mfg_test.wifi_set_antenna(Antenna.INTERNAL))
  find_addr = mfg_test.ble_scan()['result']['address']
  print(mfg_test.ble_find(find_addr))
