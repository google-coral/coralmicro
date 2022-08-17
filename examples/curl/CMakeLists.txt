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

set(CURL_SRC curl.cc)
set(CURL_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_curl
    libs_a71ch
)

add_executable_m7(curl_ethernet
    ${CURL_SRC}
)

target_link_libraries(curl_ethernet
    ${CURL_LINK_LIBRARIES}
    libs_base-m7_ethernet
)

target_compile_definitions(curl_ethernet PRIVATE
    CURL_ETHERNET
)

add_executable_m7(curl_wifi
    ${CURL_SRC}
)

target_link_libraries(curl_wifi
    ${CURL_LINK_LIBRARIES}
    libs_base-m7_wifi
)

target_compile_definitions(curl_wifi PRIVATE
    CURL_WIFI
)
