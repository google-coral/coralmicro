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

import argparse
import base64
import sys
import wave


def main():
  """
  Sample runner for the RPCs implemented in this class.
  """

  parser = argparse.ArgumentParser(description='Dev Board Micro MFGTest')
  parser.add_argument('--ip_address', type=str, default='10.10.10.1')
  parser.add_argument('--output', '-o', type=str, required=True)
  parser.add_argument('--sample_rate_hz', '-r', type=int,
                      choices=(16000, 48000), default=16000)
  parser.add_argument('--duration_ms', '-d', type=int, default=1000)
  parser.add_argument('--num_buffers', '-n', type=int, default=2)
  parser.add_argument('--buffer_size_ms', '-b', type=int, default=500)
  parser.add_argument('--verbose', action='store_true')
  parser.add_argument('--raw', action='store_true')

  args = parser.parse_args()
  mfg_test = CoralMicroMFGTest(f'http://{args.ip_address}:80/jsonrpc',
                               print_payloads=args.verbose)

  result = mfg_test.capture_audio(sample_rate_hz=args.sample_rate_hz,
                                  duration_ms=args.duration_ms,
                                  num_buffers=args.num_buffers,
                                  buffer_size_ms=args.buffer_size_ms)

  error = result.get('error')
  if error:
    print('Error:', error['message'], file=sys.stderr)
    return

  data = base64.b64decode(result['result']['data'])

  if args.raw:
    with open(args.output, 'wb') as f:
      f.write(data)
  else:
    with wave.open(args.output, 'wb') as f:
      f.setnchannels(1)
      f.setsampwidth(4)
      f.setframerate(args.sample_rate_hz)
      f.writeframes(data)

  print('File:', args.output)
  print('Format:', 'RAW' if args.raw else 'WAV')
  print('Sample Rate:', args.sample_rate_hz)
  print('Sample Count:', len(data) / 4)
  print('Duration (ms):', 1000.0 * len(data) / (4 * args.sample_rate_hz))


if __name__ == '__main__':
  main()
