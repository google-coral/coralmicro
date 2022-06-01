add_library_m7(libs_audio_freertos STATIC
    audio_driver.cc
    audio_service.cc
)
target_link_libraries(libs_audio_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
    libs_tasks_PmicTask_freertos
)
