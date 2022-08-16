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

add_library(libs_arduino_coral_micro STATIC
    CMakeLists.txt)
target_link_libraries(libs_arduino_coral_micro
    libs_base-m7_freertos
    libs_flatbuffers
    libs_jpeg_m7
    libs_yamnet
)
bundle_static_library(libs_arduino_coral_micro libs_arduino_coral_micro_bundled)

add_library(libs_arduino_coral_micro_wifi STATIC
    CMakeLists.txt)
target_link_libraries(libs_arduino_coral_micro_wifi
    libs_base-m7_freertos
    libs_base-m7_wifi
    libs_jpeg_m7
    libs_yamnet
)
bundle_static_library(libs_arduino_coral_micro_wifi libs_arduino_coral_micro_wifi_bundled)

add_library(libs_arduino_coral_micro_poe STATIC
    CMakeLists.txt)
target_link_libraries(libs_arduino_coral_micro_poe
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_jpeg_m7
    libs_yamnet
)
bundle_static_library(libs_arduino_coral_micro_poe libs_arduino_coral_micro_poe_bundled)
