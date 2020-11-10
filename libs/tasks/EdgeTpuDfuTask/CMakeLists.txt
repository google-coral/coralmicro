cmake_minimum_required(VERSION 3.13)

set(libs_tasks_EdgeTpuDfuTask_SOURCES
    edgetpu_dfu_task.cc
)

add_library(libs_tasks_EdgeTpuDfuTask_bm STATIC
    ${libs_tasks_EdgeTpuDfuTask_SOURCES}
)
target_link_libraries(libs_tasks_EdgeTpuDfuTask_bm
    libs_nxp_rt1176-sdk_bm
)

add_library(libs_tasks_EdgeTpuDfuTask_freertos STATIC
    ${libs_tasks_EdgeTpuDfuTask_SOURCES}
)
target_link_libraries(libs_tasks_EdgeTpuDfuTask_freertos
    libs_nxp_rt1176-sdk_freertos
)