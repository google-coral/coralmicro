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

add_library_m7(libs_curl STATIC
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/altsvc.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/base64.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/conncache.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/connect.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/content_encoding.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/curl_addrinfo.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/curl_ctype.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/curl_get_line.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/dotdot.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/dynbuf.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/easy.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/escape.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/getenv.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/getinfo.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/hash.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/hostip.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/hostip4.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/hsts.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/http.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/http_chunks.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/http_proxy.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/if2ip.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/llist.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/mprintf.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/multi.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/nonblock.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/parsedate.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/progress.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/rand.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/rename.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/select.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/sendf.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/setopt.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/share.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/slist.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/speedcheck.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/splay.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/strcase.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/strdup.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/strerror.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/strtoofft.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/timeval.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/transfer.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/url.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/urlapi.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/vtls/mbedtls.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/vtls/vtls.c
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib/warnless.c
    DATA
    ${PROJECT_SOURCE_DIR}/ca-certificates.crt
)

target_compile_definitions(libs_curl PRIVATE
    BUILDING_LIBCURL
)

target_compile_definitions(libs_curl PUBLIC
    HAVE_CONFIG_H
)

target_include_directories(libs_curl PUBLIC
    ${PROJECT_SOURCE_DIR}/libs/curl
    ${PROJECT_SOURCE_DIR}/third_party/curl/include
    ${PROJECT_SOURCE_DIR}/third_party/curl/lib
)

target_link_libraries(libs_curl
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk-mbedtls
)
