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

from coral_micro_mfg_test import CoralMicroMFGTest

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
  print(mfg_test.eth_get_ip())

  print(mfg_test.eth_write_phy(31, 0x0))
  print(mfg_test.eth_write_phy(9, 0x0))
  print(mfg_test.eth_write_phy(4, 0x61))
  print(mfg_test.eth_write_phy(25, 0x843))
  print(mfg_test.eth_write_phy(0, 0x9200))
