cmake_minimum_required(VERSION 3.13)

set(libs_tasks_PowerMonitorTask_SOURCES
    power_monitor_task.cc
)

add_library_m7(libs_tasks_PowerMonitorTask_freertos STATIC
    ${libs_tasks_PowerMonitorTask_SOURCES}
)
target_link_libraries(libs_tasks_PowerMonitorTask_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
)
