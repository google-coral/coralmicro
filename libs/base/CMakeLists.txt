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

add_library_m7(libs_base-m7_bm STATIC
    main_bm.cc
)

target_link_libraries(libs_base-m7_bm
    libs_nxp_rt1176-sdk_bm
)

add_library_m4(libs_base-m4_bm STATIC
    main_bm.cc
)

target_link_libraries(libs_base-m4_bm
    libs_nxp_rt1176-sdk_bm-m4
)

add_library_m7(libs_base-m7_freertos STATIC
    analog.cc
    console_m7.cc
    filesystem.cc
    gpio.cc
    i2c.cc
    ipc.cc
    ipc_m7.cc
    led.cc
    main_freertos_m7.cc
    network.cc
    ntp.cc
    pwm.cc
    random.cc
    reset.cc
    strings.cc
    spi.cc
    tempsense.cc
    timer.cc
    utils.cc
    watchdog.cc
)

set(libs_base-m7_freertos_LINK_LIBRARIES_REVISION
    libs_audio_freertos
    libs_camera_freertos
    libs_pmic_freertos
    libs_littlefs
    libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_lwip_httpd
)

target_link_libraries(libs_base-m7_freertos
    libs_nxp_rt1176-sdk-mbedtls
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk-mcmgr_m7
    libs_cdc_acm_freertos
    libs_cdc_eem_freertos
    libs_tpu_dfu_task_freertos
    libs_tpu_task_freertos
    libs_usb_device_task_freertos
    libs_usb_host_task_freertos
    ${libs_base-m7_freertos_LINK_LIBRARIES_REVISION}
)

add_library_m7(libs_base-m7_wifi STATIC
    wifi.cc
)
target_link_libraries(libs_base-m7_wifi
    libs_nxp_rt1176-sdk_wiced
)

add_library_m7(libs_base-m7_ethernet STATIC
    ethernet.cc
)
target_link_libraries(libs_base-m7_ethernet
    libs_base-m7_freertos
)

add_library_m7(libs_base-m7_http_server STATIC
    http_server.cc
    http_server_handlers.cc
)
target_link_libraries(libs_base-m7_http_server
    libs_nxp_rt1176-sdk_lwip_httpd
)

add_library_m7(libs_base-ums_freertos STATIC
    console_m7.cc
    gpio.cc
    main_freertos_ums.cc
    random.cc
    reset.cc
    tempsense.cc
    timer.cc
    utils.cc
)

target_link_libraries(libs_base-ums_freertos
    libs_nxp_rt1176-sdk-mbedtls
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk-mcmgr_m7
    libs_usb_device_task_freertos
    libs_msc_ums_freertos
    ${libs_base-m7_freertos_LINK_LIBRARIES_REVISION}
)

add_library_m4(libs_base-m4_freertos STATIC
    console_m4.cc
    filesystem.cc
    gpio.cc
    ipc.cc
    ipc_m4.cc
    led.cc
    main_freertos_m4.cc
    timer.cc
)

target_link_libraries(libs_base-m4_freertos
    libs_nxp_rt1176-sdk_freertos-m4
    libs_camera_freertos-m4
    libs_pmic_freertos-m4
    libs_littlefs-m4
)

target_sources(libs_base-m7_bm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)
target_sources(libs_base-m4_bm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
target_sources(libs_base-m4_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_base-m4_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk-mcmgr_m7>)
