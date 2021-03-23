cmake_minimum_required(VERSION 3.13)

add_library_m7(libs_tasks_UsbDeviceTask_freertos STATIC
    usb_device_task.cc
)

target_link_libraries(libs_tasks_UsbDeviceTask_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_CdcEem_freertos
)
