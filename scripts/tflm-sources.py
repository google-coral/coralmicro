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

import glob
import os
import os.path
import re


def kernel_optimizations_except(files, optimization):
  def matched():
    for f in files:
      match = re.match(r".*/kernels/([^/]*)/.*", f)
      if match:
        yield match.group(1)

  return set(matched()) - set(['internal', optimization])


def optimized_kernel_files(files, optimization):
  return [os.path.join('kernels', os.path.basename(f))
          for f in files if optimization in f]


def exclude(files, word):
  return [f for f in files if word not in f]


def main():
  project_root = os.path.join(os.path.dirname(
      __file__), '..', 'third_party', 'tflite-micro')
  os.chdir(project_root)
  c_files = glob.glob('**/*.c', recursive=True)
  cc_files = glob.glob('**/*.cc', recursive=True)
  files = c_files + cc_files

  for word in ['test',
               'examples',
               'benchmarks',
               'third_party',
               'generated_data',
               '_main',
               'kiss_fft_int16',
               'debug_log',
               'frontend_memmap_generator',
               'python',
               'tools']:
    files = exclude(files, word)

  for word in ['arc_custom',
               'arc_emsdp',
               'bluepill',
               'chre',
               'cortex_m_corstone_300',
               'cortex_m_generic',
               'hexagon',
               'riscv32_mcu',
               'stm32f4']:
    files = exclude(files, word)

  for kernel_optimization in kernel_optimizations_except(files, 'cmsis_nn'):
    files = exclude(files, kernel_optimization)

  for kernel_file in optimized_kernel_files(files, 'cmsis_nn'):
    files = exclude(files, kernel_file)

  print('    debug_log.c')
  print('    ${PROJECT_SOURCE_DIR}/third_party/tflite-micro/tensorflow/lite/micro/test_helpers.cc  # IntArrayFromInts()')
  for f in sorted(files):
    print('    ' + os.path.join('${PROJECT_SOURCE_DIR}',
          'third_party', 'tflite-micro', f))


if __name__ == '__main__':
  main()
