cmake_minimum_required(VERSION 3.13)
project(EdgetpuDfu)

add_executable_m7(EdgetpuDfu
    apex_latest_single_ep_bin.c
    edgetpu_dfu.cc
)

target_include_directories(EdgetpuDfu PRIVATE
    .
)

target_link_libraries(EdgetpuDfu
    libs_nxp_rt1176-sdk_bm
    libs_tasks_EdgeTpuDfuTask_bm
    libs_tasks_UsbHostTask_bm
)
target_sources(EdgetpuDfu PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)
