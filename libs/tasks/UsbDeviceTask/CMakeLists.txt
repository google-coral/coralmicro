cmake_minimum_required(VERSION 3.13)

set(libs_tasks_UsbDeviceTask_SOURCES
    usb_device_task.cc
)

add_library(libs_tasks_UsbDeviceTask_bm STATIC
    ${libs_tasks_UsbDeviceTask_SOURCES}
)
target_link_libraries(libs_tasks_UsbDeviceTask_bm
    libs_nxp_rt1176-sdk_bm
)

add_library(libs_tasks_UsbDeviceTask_freertos STATIC
    ${libs_tasks_UsbDeviceTask_SOURCES}
)
target_link_libraries(libs_tasks_UsbDeviceTask_freertos
    libs_nxp_rt1176-sdk_freertos
)
