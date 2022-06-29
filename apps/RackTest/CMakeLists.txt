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
    libs_mjson
    libs_rpc_http_server
    libs_tpu_dfu_task_freertos
    libs_tpu_task_freertos
    libs_usb_host_task_freertos
    libs_tensorflow-m7
    libs_testlib
    libs_tpu_freertos
)

add_executable_m7(RackTest
    rack_test.cc
    M4_EXECUTABLE
    RackTestM4
)

target_link_libraries(RackTest
    ${RACK_TEST_LINK_LIBRARIES}
)

add_executable_m4(RackTestM4
    rack_test_m4.cc
)

target_link_libraries(RackTestM4
    libs_base-m4_freertos
    libs_coremark-m4
)


# For wifi tests.
add_executable_m7(RackTestWifi
    rack_test.cc
    M4_EXECUTABLE
    RackTestM4
)

target_compile_definitions(RackTestWifi PUBLIC TEST_WIFI)

target_link_libraries(RackTestWifi
    ${RACK_TEST_LINK_LIBRARIES}
    libs_base-m7_wifi
)
