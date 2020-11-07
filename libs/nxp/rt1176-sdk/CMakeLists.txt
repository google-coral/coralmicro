cmake_minimum_required(VERSION 3.13)

enable_language(ASM)

set(libs_nxp_rt1176-sdk_SOURCES
    board.c
    clock_config.c
    peripherals.c
    pin_mux.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_port_uart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart/fsl_adapter_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pmu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str/fsl_str.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub_app.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_devices.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_ehci.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_framework.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.c
    ${libs_CMSIS_SOURCES}
)

set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS
    SERIAL_PORT_TYPE_UART=1
    USB_HOST_CONFIG_DFU
)

set(libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/libs/nxp/rt1176-sdk
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/xip
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/xip
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/include
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy
)

set(libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES
    libs_CMSIS
)

add_library(libs_nxp_rt1176-sdk_bm OBJECT
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
    ${libs_nxp_rt1176-sdk_SOURCES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_bm PUBLIC
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_include_directories(libs_nxp_rt1176-sdk_bm PUBLIC
    ${libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_bm
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

add_library(libs_nxp_rt1176-sdk_freertos OBJECT
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_FreeRTOS_SOURCES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos PUBLIC
    FSL_RTOS_FREE_RTOS
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_include_directories(libs_nxp_rt1176-sdk_freertos PUBLIC
    ${libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_freertos
    libs_FreeRTOS
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)
target_sources(libs_nxp_rt1176-sdk_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
