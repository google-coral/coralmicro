cmake_minimum_required(VERSION 3.13)

set(libs_tasks_PmicTask_SOURCES
    pmic_task.cc
)

add_library(libs_tasks_PmicTask_freertos STATIC
    ${libs_tasks_PmicTask_SOURCES}
)
target_link_libraries(libs_tasks_PmicTask_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
)
