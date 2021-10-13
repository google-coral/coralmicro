cmake_minimum_required(VERSION 3.13)

set(libs_FreeRTOS_SOURCES
    app_callbacks.c
    multicore.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/event_groups.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/list.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F/port.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/queue.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/stream_buffer.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/tasks.c
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/timers.c
    ${VALIANT_SOURCE_DIR}/third_party/FreeRTOS_helpers/heap_useNewlib_NXP.c
)

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PUBLIC
    .
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/include
    ${VALIANT_SOURCE_DIR}/third_party/freertos_kernel/portable/GCC/ARM_CM4F
)

set(libs_FreeRTOS_INCLUDE_DIRECTORIES_PRIVATE
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/freertos_kernel/include/
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
