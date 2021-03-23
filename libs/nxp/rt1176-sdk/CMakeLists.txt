cmake_minimum_required(VERSION 3.13)

enable_language(ASM)

if(VALIANT_BOARD_REVISION STREQUAL EVK)
    set(libs_nxp_rt1176-sdk_SOURCES_REVISION
        evkmimxrt1170_flexspi_nor_config.c
        fsl_flexspi_nor_boot.c
        pin_mux_evk.c
    )
elseif(VALIANT_BOARD_REVISION STREQUAL P0)
    set(libs_nxp_rt1176-sdk_SOURCES_REVISION
        pin_mux_p0.c
    )
else()
    message(FATAL_ERROR "No revision selected.")
endif()

set(libs_nxp_rt1176-sdk_SOURCES
    board.c
    board_hardware.c
    clock_config.c
    dlmalloc.c
    peripherals.c
    usb_device_class.c
    usb_device_cdc_eem.c
    ${libs_nxp_rt1176-sdk_SOURCES_REVISION}
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/flash/nand/flexspi/fsl_flexspi_nand_flash.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists/fsl_component_generic_list.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_port_uart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart/fsl_adapter_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_flexspi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_mu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pmu.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_romapi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_semc.c
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
    DRIVER_CACHE_CONTROL=1
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
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/flash/nand
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
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.c
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

add_library_m7(libs_nxp_rt1176-sdk_sdmmc
    sdmmc_config.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio/fsl_sdio.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_usdhc.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/osa/fsl_sdmmc_osa.c
)

target_include_directories(libs_nxp_rt1176-sdk_sdmmc PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/common
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/host/usdhc
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/osa
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio
)

target_link_libraries(libs_nxp_rt1176-sdk_sdmmc
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_lwip
    dhcp_server.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/sys_arch.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/api_lib.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/api_msg.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/err.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netbuf.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netifapi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/sockets.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/tcpip.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/def.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/dns.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/inet_chksum.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/init.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ip.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/acd.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/dhcp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/etharp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/icmp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4_addr.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4_frag.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/mem.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/memp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/netif.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/pbuf.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/stats.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp_in.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp_out.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/timeouts.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/udp.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/netif/ethernet.c
)

target_include_directories(libs_nxp_rt1176-sdk_lwip PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/include
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_wiced
    WICED/WWD/internal/chips/43455/resources.c
    WICED/platform/MCU/LPC/WWD/wwd_platform.c
    rtos/freertos/vendors/nxp/wifi/wiced/iot_wifi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WWD/wwd_rtos.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO/wwd_bus_protocol.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/wwd_bus_common.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/wwd_ap.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/wwd_chip_specific_functions.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/tlv.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_ap_common.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_clm.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_debug.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_internal.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_management.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_sdpcm.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_thread.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi_sleep.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/internal/wifi.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD/wwd_buffer.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD/wwd_network.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/LPC/WWD/wwd_SDIO.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/LPC/WWD/wwd_platform.c
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/wwd_resources.c
)

target_include_directories(libs_nxp_rt1176-sdk_wiced PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WWD
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/RTOS
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/network
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/LPC
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/include
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/platforms/MURATA_TYPE1MW
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include
    ${CMAKE_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/libraries/c_sdk/standard/common/include
)

target_compile_definitions(libs_nxp_rt1176-sdk_wiced PUBLIC
    WLAN_ARM_CR4
    USES_RESOURCES_IN_EXTERNAL_STORAGE
    HIGH_SPEED_SDIO_CLOCK
    WPRINT_PLATFORM_PERMISSION
)

target_link_libraries(libs_nxp_rt1176-sdk_wiced
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_sdmmc
)

target_sources(libs_nxp_rt1176-sdk_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)
