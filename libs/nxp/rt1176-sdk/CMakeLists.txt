cmake_minimum_required(VERSION 3.13)

enable_language(ASM)

add_library(libs_nxp_rt1176-sdk STATIC
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_port_uart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart/fsl_adapter_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str/fsl_str.c
)

target_compile_definitions(libs_nxp_rt1176-sdk PUBLIC
    SERIAL_PORT_TYPE_UART
)

target_include_directories(libs_nxp_rt1176-sdk PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str
)

target_link_libraries(libs_nxp_rt1176-sdk
    libs_CMSIS
)
