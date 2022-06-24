add_library_m7(libs_usb_device_task_freertos STATIC
    usb_device_task.cc
)

target_link_libraries(libs_usb_device_task_freertos
    libs_base-m7_freertos
)

add_library_m7(libs_usb_host_task_freertos STATIC
    usb_host_task.cc
)
target_link_libraries(libs_usb_host_task_freertos
    libs_nxp_rt1176-sdk_freertos
)
