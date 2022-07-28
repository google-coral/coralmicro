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

add_library_m7(libs_libjpeg
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jaricom.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcapimin.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcapistd.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcarith.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jccoefct.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jccolor.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcdctmgr.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jchuff.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcinit.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcmainct.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcmarker.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcmaster.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcomapi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcparam.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcprepct.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jcsample.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jctrans.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdapimin.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdapistd.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdarith.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdatadst.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdatasrc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdcoefct.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdcolor.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jddctmgr.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdhuff.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdinput.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdmainct.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdmarker.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdmaster.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdmerge.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdpostct.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdsample.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jdtrans.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jerror.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jfdctflt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jfdctfst.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jfdctint.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jidctflt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jidctfst.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jidctint.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jmemmgr.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jmemnobs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jquant1.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jquant2.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/src/jutils.c
)

target_include_directories(libs_libjpeg PUBLIC
    .
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/middleware/libjpeg/template
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/libjpeg/inc
)

add_library_m7(libs_jpeg_m7 STATIC
    jpeg.cc
)

target_link_libraries(libs_jpeg_m7
    libs_base-m7_freertos
    libs_libjpeg
)

add_library_m4(libs_jpeg_m4 STATIC
    jpeg.cc
)

target_link_libraries(libs_jpeg_m4
    libs_base-m4_freertos
    libs_libjpeg
)