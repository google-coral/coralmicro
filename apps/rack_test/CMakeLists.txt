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

set(RACK_TEST_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_coremark-m7
    libs_rpc_http_server
    libs_rpc_utils
    libs_testlib
)

add_executable_m7(rack_test
    rack_test.cc
    M4_EXECUTABLE
    rack_test_m4
)

target_link_libraries(rack_test
    ${RACK_TEST_LINK_LIBRARIES}
)

add_executable_m4(rack_test_m4
    rack_test_m4.cc
)

target_link_libraries(rack_test_m4
    libs_base-m4_freertos
    libs_coremark-m4
)


# For wifi tests.
add_executable_m7(rack_test_wifi
    rack_test.cc
    M4_EXECUTABLE
    rack_test_m4
)

# For bluetooth tests.
add_executable_m7(rack_test_ble
     rack_test.cc
     M4_EXECUTABLE
     rack_test_m4
)

target_compile_definitions(rack_test_wifi PUBLIC TEST_WIFI)
target_compile_definitions(rack_test_ble PUBLIC TEST_BLE)

target_link_libraries(rack_test_wifi
    ${RACK_TEST_LINK_LIBRARIES}
    libs_base-m7_wifi
)

target_link_libraries(rack_test_ble
    ${RACK_TEST_LINK_LIBRARIES}
)