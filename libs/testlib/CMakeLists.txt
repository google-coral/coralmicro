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

add_library_m7(libs_testlib STATIC
    test_lib.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/testconv1-edgetpu.tflite
    ${PROJECT_SOURCE_DIR}/models/testconv1-expected-output.bin
    ${PROJECT_SOURCE_DIR}/models/testconv1-test-input.bin
)

target_link_libraries(libs_testlib PUBLIC
    libs_base-m7_freertos
    libs_a71ch
    libs_mjson
    libs_audio_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
    libs_base-m7_wifi
    edgefast_bluetooth
)
