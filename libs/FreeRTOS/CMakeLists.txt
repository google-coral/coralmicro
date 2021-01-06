cmake_minimum_required(VERSION 3.13)

set(libs_FreeRTOS_SOURCES
    app_callbacks.c
    multicore.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/event_groups.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/list.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F/port.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/MemMang/heap_4.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/queue.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/stream_buffer.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/tasks.c
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/timers.c
)

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC
    .
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/include
    ${CMAKE_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F
)

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/freertos_kernel/include/
)

add_library_m7(libs_FreeRTOS OBJECT
    ${libs_FreeRTOS_SOURCES}
)

target_include_directories(libs_FreeRTOS PUBLIC
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC}
    PRIVATE
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE}
)

add_library_m4(libs_FreeRTOS-m4 OBJECT
    ${libs_FreeRTOS_SOURCES}
)

target_include_directories(libs_FreeRTOS-m4 PUBLIC
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC}
    PRIVATE
    ${libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE}
)
