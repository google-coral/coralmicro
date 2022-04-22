set(libs_tasks_UsbHostTask_SOURCES
    usb_host_task.cc
)

add_library_m7(libs_tasks_UsbHostTask_freertos STATIC
    ${libs_tasks_UsbHostTask_SOURCES}
)
target_link_libraries(libs_tasks_UsbHostTask_freertos
    libs_nxp_rt1176-sdk_freertos
)
