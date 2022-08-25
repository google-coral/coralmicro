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
  import argparse
  parser = argparse.ArgumentParser(description='Dev Board Micro IPerf')
  mode_group = parser.add_mutually_exclusive_group(required=True)
  mode_group.add_argument('-s', action='store_true')
  mode_group.add_argument('-c', type=str, help='IP address of iPerf server')
  parser.add_argument('--device_ip_address', type=str,
                      required=False, default='10.10.10.1')
  parser.add_argument('-t', type=int, required=False, default=10)
  parser.add_argument('-u', action='store_true')
  parser.add_argument('-b', type=str, required=False, default='1M')
  args = parser.parse_args()

  bandwidth_suffix = args.b[-1]
  if not bandwidth_suffix.isdigit():
    bandwidth_multiplier = 1
    bandwidth = args.b[0:-1]
    if bandwidth_suffix == 'k':
      bandwidth_multiplier = 10 ** 3
    elif bandwidth_suffix == 'm':
      bandwidth_multiplier = 10 ** 6
    elif bandwidth_suffix == 'g':
      bandwidth_multiplier = 10 ** 9
    elif bandwidth_suffix == 'K':
      bandwidth_multiplier = 2 ** 10
    elif bandwidth_suffix == 'M':
      bandwidth_multiplier = 2 ** 20
    elif bandwidth_suffix == 'G':
      bandwidth_multiplier = 2 ** 30
    else:
      bandwidth = args.b
    bandwidth = int(bandwidth) * bandwidth_multiplier
  else:
    bandwidth = int(args.b)

  mfg_test = CoralMicroMFGTest(
      f'http://{args.device_ip_address}:80/jsonrpc', print_payloads=True)
  if args.s:
    print(f'Device ({args.device_ip_address}) acting as iPerf server')
    response = mfg_test.iperf_start(
        is_server=True, udp=args.u, bandwidth=bandwidth)
    print(response)
  else:  # args.c
    print(
        f'Device ({args.device_ip_address}) acting as iPerf client, connecting to {args.c}')
    response = mfg_test.iperf_start(
        is_server=False, server_ip_address=args.c, duration=args.t, udp=args.u, bandwidth=bandwidth)
    print(response)

  # response = mfg_test.iperf_stop()
  # print(response)
