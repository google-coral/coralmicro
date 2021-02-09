cmake_minimum_required(VERSION 3.13)

enable_language(ASM)

set(libs_nxp_rt1176-sdk_SOURCES
    board.c
    board_hardware.c
    clock_config.c
    dlmalloc.c
    evkmimxrt1170_flexspi_nor_config.c
    fsl_flexspi_nor_boot.c
    peripherals.c
    pin_mux.c
    usb_device_class.c
    usb_device_cdc_eem.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists/fsl_component_generic_list.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_port_uart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart/fsl_adapter_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_mu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pmu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_semc.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/romapi/fsl_romapi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str/fsl_str.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1170.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr_mu_internal.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_dci.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_ehci.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub_app.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_devices.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_ehci.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_framework.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/usb_device_ch9.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.c
    ${libs_CMSIS_SOURCES}
)

set(libs_nxp_rt1176-sdk-m7_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
)

set(libs_nxp_rt1176-sdk-m4_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4/fsl_cache.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm4.S
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm4.c
)

set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS
    # dlmalloc defines
    HAVE_MMAP=0
    ONLY_MSPACES
    USE_DL_PREFIX
    malloc_getpagesize=0x800
    # Other defines
    MCMGR_HANDLE_EXCEPTIONS=1
    SDK_DEBUGCONSOLE_UART
    SERIAL_PORT_TYPE_UART=1
    USB_HOST_CONFIG_DFU
    USE_SDRAM
    __STARTUP_CLEAR_BSS
    __STARTUP_INITIALIZE_NONCACHEDATA
    __USE_SHMEM
)

set(libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS
    CPU_MIMXRT1176DVMAA_cm7
    XIP_BOOT_HEADER_ENABLE=1
    XIP_EXTERNAL_FLASH=1
)

set(libs_nxp_rt1176-sdk-m4_COMPILE_DEFINITIONS
    CPU_MIMXRT1176DVMAA_cm4
)

set(libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/libs/nxp/rt1176-sdk
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/xip
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/common_task
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/xip
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/include
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy
)

set(libs_nxp_rt1176-sdk-m7_TARGET_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7
)

set(libs_nxp_rt1176-sdk-m4_TARGET_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4
)

set(libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES
    libs_CMSIS
)

add_library_m7(libs_nxp_rt1176-sdk_bm OBJECT
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
    ${libs_nxp_rt1176-sdk-m7_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
)

add_library_m4(libs_nxp_rt1176-sdk_bm-m4 OBJECT
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
    ${libs_nxp_rt1176-sdk-m4_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_bm PUBLIC
    FSL_OSA_BM_TIMER_CONFIG=FSL_OSA_BM_TIMER_SYSTICK
    ${libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_compile_definitions(libs_nxp_rt1176-sdk_bm-m4 PUBLIC
    FSL_OSA_BM_TIMER_CONFIG=FSL_OSA_BM_TIMER_SYSTICK
    ${libs_nxp_rt1176-sdk-m4_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_include_directories(libs_nxp_rt1176-sdk_bm PUBLIC
    ${libs_nxp_rt1176-sdk-m7_TARGET_INCLUDE_DIRECTORIES}
    ${libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES}
)

target_include_directories(libs_nxp_rt1176-sdk_bm-m4 PUBLIC
    ${libs_nxp_rt1176-sdk-m4_TARGET_INCLUDE_DIRECTORIES}
    ${libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_bm
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_bm-m4
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

add_library_m7(libs_nxp_rt1176-sdk_freertos OBJECT
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${libs_nxp_rt1176-sdk-m7_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_FreeRTOS_SOURCES}
)

add_library_m4(libs_nxp_rt1176-sdk_freertos-m4 OBJECT
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${libs_nxp_rt1176-sdk-m4_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_FreeRTOS-m4_SOURCES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos PUBLIC
    FSL_RTOS_FREE_RTOS
    ${libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC
    FSL_RTOS_FREE_RTOS
    ${libs_nxp_rt1176-sdk-m4_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_include_directories(libs_nxp_rt1176-sdk_freertos PUBLIC
    ${libs_nxp_rt1176-sdk-m7_TARGET_INCLUDE_DIRECTORIES}
    ${libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES}
)

target_include_directories(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC
    ${libs_nxp_rt1176-sdk-m4_TARGET_INCLUDE_DIRECTORIES}
    ${libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_freertos-m4
    libs_FreeRTOS-m4
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

target_sources(libs_nxp_rt1176-sdk_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)
