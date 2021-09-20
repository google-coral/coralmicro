cmake_minimum_required(VERSION 3.13)
project(ELFLoader)

include_directories(.)

add_executable_m7(ELFLoader
    LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm7_elfloader.ld
    elf_loader.cc
    usb_data.cc
    ${CMAKE_SOURCE_DIR}/libs/base/filesystem.cc
    ${CMAKE_SOURCE_DIR}/libs/base/reset.cc
    ${CMAKE_SOURCE_DIR}/libs/base/utils.cc
    ${CMAKE_SOURCE_DIR}/libs/tasks/UsbDeviceTask/usb_device_task.cc
)

target_compile_definitions(ELFLoader PUBLIC
    ELFLOADER
)

target_include_directories(ELFLoader PRIVATE
    .
)

target_link_libraries(ELFLoader
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
    libs_nxp_rt1176-sdk_littlefs
)
target_sources(ELFLoader PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
target_sources(ELFLoader PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
