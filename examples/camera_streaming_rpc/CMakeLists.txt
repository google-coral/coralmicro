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

set(CAMERA_STREAMING_SRC camera_streaming.cc)

set(CAMERA_STREAMING_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_rpc_utils
)

# USB
add_executable_m7(camera_streaming_rpc_usb
    ${CAMERA_STREAMING_SRC}
)

target_link_libraries(camera_streaming_rpc_usb
    ${CAMERA_STREAMING_LINK_LIBRARIES}
)

# POE
add_executable_m7(camera_streaming_rpc_ethernet
    ${CAMERA_STREAMING_SRC}
)

target_compile_definitions(camera_streaming_rpc_ethernet PUBLIC
    CAMERA_STREAMING_ETHERNET
)

target_link_libraries(camera_streaming_rpc_ethernet
    ${CAMERA_STREAMING_LINK_LIBRARIES}
    libs_base-m7_ethernet
)

# WIFI
add_executable_m7(camera_streaming_rpc_wifi
    ${CAMERA_STREAMING_SRC}
)

target_compile_definitions(camera_streaming_rpc_wifi PUBLIC
    CAMERA_STREAMING_WIFI
)

target_link_libraries(camera_streaming_rpc_wifi
    ${CAMERA_STREAMING_LINK_LIBRARIES}
    libs_nxp_rt1176-sdk_wiced
    libs_base-m7_wifi
)