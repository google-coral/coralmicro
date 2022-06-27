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

set(libs_usb_host_edgetpu_SOURCES
    usb_host_edgetpu.c
)

add_library_m7(libs_usb_host_edgetpu_bm STATIC
    ${libs_usb_host_edgetpu_SOURCES}
)
target_link_libraries(libs_usb_host_edgetpu_bm
    libs_nxp_rt1176-sdk_bm
)

add_library_m7(libs_usb_host_edgetpu_freertos STATIC
    ${libs_usb_host_edgetpu_SOURCES}
)
target_link_libraries(libs_usb_host_edgetpu_freertos
    libs_nxp_rt1176-sdk_freertos
)
