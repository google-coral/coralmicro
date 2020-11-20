cmake_minimum_required(VERSION 3.13)
project(EdgetpuDfuFreeRTOS)

add_executable_m7(EdgetpuDfuFreeRTOS
    edgetpu_dfu.cc
)

target_include_directories(EdgetpuDfuFreeRTOS PRIVATE
    .
)

target_link_libraries(EdgetpuDfuFreeRTOS
    libs_base-m7_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
)
