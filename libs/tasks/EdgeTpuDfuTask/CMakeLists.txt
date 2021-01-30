cmake_minimum_required(VERSION 3.13)

set(libs_tasks_EdgeTpuDfuTask_SOURCES
    apex_latest_single_ep_bin.c
    edgetpu_dfu_task.cc
)

add_library_m7(libs_tasks_EdgeTpuDfuTask_freertos STATIC
    ${libs_tasks_EdgeTpuDfuTask_SOURCES}
)
target_link_libraries(libs_tasks_EdgeTpuDfuTask_freertos
    libs_FreeRTOS
    libs_nxp_rt1176-sdk_freertos
)
