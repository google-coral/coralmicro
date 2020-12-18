cmake_minimum_required(VERSION 3.13)

set(libs_tasks_UsbHostTask_SOURCES
    usb_host_task.cc
)

add_library_m7(libs_tasks_UsbHostTask_bm STATIC
    ${libs_tasks_UsbHostTask_SOURCES}
)
target_link_libraries(libs_tasks_UsbHostTask_bm
    libs_nxp_rt1176-sdk_bm
)

add_library_m7(libs_tasks_UsbHostTask_freertos STATIC
    ${libs_tasks_UsbHostTask_SOURCES}
)
target_link_libraries(libs_tasks_UsbHostTask_freertos
    libs_nxp_rt1176-sdk_freertos
)
