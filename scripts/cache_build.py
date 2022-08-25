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

import argparse
import os


def main():
  parser = argparse.ArgumentParser(description='Valaint Build Directory Cache Creator',
                                   formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument('--build_dir', type=str, required=True)
  args = parser.parse_args()

  build_dir = os.path.relpath(args.build_dir)
  with open(os.path.join(build_dir, 'cached_files.txt'), 'w') as f:
    for dirname, dirnames, filenames in os.walk(build_dir):
      for filename in filenames:
        f.write(os.path.join(dirname, filename) + '\n')


if __name__ == '__main__':
  main()
