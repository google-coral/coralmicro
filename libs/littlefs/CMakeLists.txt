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

add_library_m7(libs_littlefs
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_littlefs PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

target_compile_definitions(libs_littlefs PUBLIC
    LFS_THREADSAFE
)

add_library_m4(libs_littlefs-m4
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_littlefs-m4 PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

target_compile_definitions(libs_littlefs-m4 PUBLIC
    LFS_THREADSAFE
)
