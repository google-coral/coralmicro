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
    ipc.cc
    ipc_m7.cc
    main_freertos_m7.cc
    pwm.cc
    random.cc
    reset.cc
    tempsense.cc
    timer.cc
    utils.cc
)

if (VALIANT_BOARD_REVISION STREQUAL P0 OR VALIANT_BOARD_REVISION STREQUAL P1)
    set(libs_base-m7_freertos_LINK_LIBRARIES_REVISION
        libs_tasks_AudioTask_freertos
        libs_tasks_CameraTask_freertos
        libs_tasks_PmicTask_freertos
        libs_littlefs
        libs_nxp_rt1176-sdk_lwip
        libs_nxp_rt1176-sdk_lwip_httpd
    )
endif()

target_link_libraries(libs_base-m7_freertos
    libs_nxp_rt1176-sdk-mbedtls
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk-mcmgr_m7
    libs_CdcAcm_freertos
    libs_CdcEem_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbDeviceTask_freertos
    libs_tasks_UsbHostTask_freertos
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

add_library_m4(libs_base-m4_freertos STATIC
    console_m4.cc
    filesystem.cc
    gpio.cc
    ipc.cc
    ipc_m4.cc
    main_freertos_m4.cc
    timer.cc
)

target_link_libraries(libs_base-m4_freertos
    libs_nxp_rt1176-sdk_freertos-m4
    libs_tasks_CameraTask_freertos-m4
    libs_tasks_PmicTask_freertos-m4
    libs_littlefs-m4
)

target_sources(libs_base-m7_bm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)
target_sources(libs_base-m4_bm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
target_sources(libs_base-m4_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_base-m4_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk-mcmgr_m7>)
