cmake_minimum_required(VERSION 3.13)
project(CdcAcm)

add_executable(CdcAcm
    cdc_acm.cc
)

target_include_directories(CdcAcm PRIVATE
    .
)

target_link_libraries(CdcAcm
    libs_CdcAcm_freertos
    libs_nxp_rt1176-sdk_freertos
    libs_tasks_UsbDeviceTask_freertos
)
target_sources(CdcAcm PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
