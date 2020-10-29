cmake_minimum_required(VERSION 3.13)

add_library(libs_FreeRTOS STATIC
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/list.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F/port.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/MemMang/heap_4.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/queue.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/tasks.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/timers.c
)

target_include_directories(libs_FreeRTOS PUBLIC
    .
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/include
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F
    PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/freertos_kernel/include/
)
