cmake_minimum_required(VERSION 3.13)

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
    console_m7.cc
    ipc_m7.cc
    main_freertos_m7.cc
    network.cc
    random.cc
    reset.cc
)

target_link_libraries(libs_base-m7_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_CdcAcm_freertos
    libs_CdcEem_freertos
    libs_tasks_UsbDeviceTask_freertos
)

add_library_m4(libs_base-m4_freertos STATIC
    console_m4.cc
    ipc_m4.cc
    main_freertos_m4.cc
)

target_link_libraries(libs_base-m4_freertos
    libs_nxp_rt1176-sdk_freertos-m4
)

target_sources(libs_base-m7_bm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)
target_sources(libs_base-m4_bm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
target_sources(libs_base-m4_freertos PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos-m4>)
target_sources(libs_base-m7_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_base-m4_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)
