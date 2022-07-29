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

add_executable_m7(MFGTest
    mfg_test.cc
    mfg_test_iperf.cc
)

target_link_libraries(MFGTest
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_a71ch
    libs_rpc_http_server
    libs_rpc_utils
    libs_mjson
    libs_testlib
    libs_tpu_freertos
)

add_executable_m7(MFGTestWiFi
    mfg_test_wifi.cc
    mfg_test_iperf.cc
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/wiced_bt_cfg.c
)

target_link_libraries(MFGTestWiFi
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_rpc_utils
    libs_mjson
    libs_testlib
    # put me before wiced include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/BTE/libwicedble.a
    libs_base-m7_wifi
)

add_executable_m7(MFGTestPOE
    mfg_test_poe.cc
    mfg_test_iperf.cc
)

target_link_libraries(MFGTestPOE
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_rpc_http_server
    libs_rpc_utils
    libs_mjson
    libs_testlib
)
