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

add_library_m7(libs_coremark-m7 STATIC
    core_coral_micro.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_list_join.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_matrix.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_state.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_util.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/coremark/core_main.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/coremark/core_portme.c
)

target_link_libraries(libs_coremark-m7
    libs_base-m7_freertos
)

target_include_directories(libs_coremark-m7 PRIVATE
    .
    ${PROJECT_SOURCE_DIR}/third_party/coremark
    ${PROJECT_SOURCE_DIR}/third_party/modified/coremark
)

target_compile_definitions(libs_coremark-m7 PRIVATE
    main=coremark_main
)

add_library_m4(libs_coremark-m4 STATIC
    core_coral_micro.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_list_join.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_matrix.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_state.c
    ${PROJECT_SOURCE_DIR}/third_party/coremark/core_util.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/coremark/core_main.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/coremark/core_portme.c
)

target_link_libraries(libs_coremark-m4
    libs_base-m4_freertos
)

target_include_directories(libs_coremark-m4 PRIVATE
    .
    ${PROJECT_SOURCE_DIR}/third_party/coremark
    ${PROJECT_SOURCE_DIR}/third_party/modified/coremark
)

target_compile_definitions(libs_coremark-m4 PRIVATE
    main=coremark_main
)
