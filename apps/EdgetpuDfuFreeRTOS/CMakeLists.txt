cmake_minimum_required(VERSION 3.13)
project(EdgetpuDfuFreeRTOS)

add_executable_m7(EdgetpuDfuFreeRTOS
    apex_latest_single_ep_bin.c
    edgetpu_dfu.cc
)

target_include_directories(EdgetpuDfuFreeRTOS PRIVATE
    .
)

target_link_libraries(EdgetpuDfuFreeRTOS
    libs_nxp_rt1176-sdk_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
)
target_sources(EdgetpuDfuFreeRTOS PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
