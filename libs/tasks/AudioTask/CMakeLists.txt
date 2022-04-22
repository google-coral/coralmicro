set(libs_tasks_AudioTask_SOURCES
    audio_reader.cc
    audio_task.cc
)

add_library_m7(libs_tasks_AudioTask_freertos STATIC
    ${libs_tasks_AudioTask_SOURCES}
)
target_link_libraries(libs_tasks_AudioTask_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
    libs_tasks_PmicTask_freertos
)

