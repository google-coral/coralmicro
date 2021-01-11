cmake_minimum_required(VERSION 3.13)
project(CdcAcm)

add_executable_m7(CdcAcm
    cdc_acm.cc
)

target_include_directories(CdcAcm PRIVATE
    .
)

target_link_libraries(CdcAcm
    libs_base-m7_freertos
    libs_CdcAcm_freertos
    libs_tasks_UsbDeviceTask_freertos
)
