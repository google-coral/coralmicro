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

add_definitions(-DFIXED_POINT=16)
add_library_m7(libs_kissfft-m7 STATIC
    ${PROJECT_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${PROJECT_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

target_include_directories(libs_kissfft-m7 PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/kissfft
)

add_library_m4(libs_kissfft-m4 STATIC
    ${PROJECT_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${PROJECT_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

target_include_directories(libs_kissfft-m4 PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/kissfft
)
