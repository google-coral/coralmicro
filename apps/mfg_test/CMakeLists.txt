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

add_executable_m7(mfg_test
    mfg_test.cc
    mfg_test_iperf.cc
)

target_link_libraries(mfg_test
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_a71ch
    libs_rpc_http_server
    libs_rpc_utils
    libs_testlib
)

add_executable_m7(mfg_test_wifi
    mfg_test_wifi.cc
    mfg_test_iperf.cc
)

target_link_libraries(mfg_test_wifi
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_rpc_utils
    libs_mjson
    libs_testlib
    libs_base-m7_wifi
    edgefast_bluetooth
)

add_executable_m7(mfg_test_poe
    mfg_test_poe.cc
    mfg_test_iperf.cc
)

target_link_libraries(mfg_test_poe
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_rpc_http_server
    libs_rpc_utils
    libs_testlib
)
