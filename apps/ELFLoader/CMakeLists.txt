add_executable_m7(ELFLoader
    LINKER_SCRIPT ${VALIANT_SOURCE_DIR}/libs/nxp/rt1176-sdk/MIMXRT1176xxxxx_cm7_elfloader.ld
    elf_loader.cc
    usb_data.cc
    ${VALIANT_SOURCE_DIR}/libs/base/filesystem.cc
    ${VALIANT_SOURCE_DIR}/libs/base/reset.cc
    ${VALIANT_SOURCE_DIR}/libs/base/utils.cc
    ${VALIANT_SOURCE_DIR}/libs/tasks/UsbDeviceTask/usb_device_task.cc
)

target_compile_definitions(ELFLoader PUBLIC
    ELFLOADER
)

target_link_libraries(ELFLoader
    libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
    libs_littlefs
    libs_nxp_rt1176-sdk_lwip
)
target_sources(ELFLoader PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)
target_sources(ELFLoader PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
