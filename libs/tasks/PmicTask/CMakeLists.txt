set(libs_tasks_PmicTask_SOURCES
    pmic_task.cc
)

add_library_m7(libs_tasks_PmicTask_freertos STATIC
    ${libs_tasks_PmicTask_SOURCES}
)
target_link_libraries(libs_tasks_PmicTask_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
)

add_library_m4(libs_tasks_PmicTask_freertos-m4 STATIC
    ${libs_tasks_PmicTask_SOURCES}
)
target_link_libraries(libs_tasks_PmicTask_freertos-m4
    libs_nxp_rt1176-sdk_freertos-m4
    libs_FreeRTOS-m4
)
