cmake_minimum_required(VERSION 3.13)

enable_language(ASM)

if(VALIANT_BOARD_REVISION STREQUAL EVK)
    set(libs_nxp_rt1176-sdk_SOURCES_REVISION
        evkmimxrt1170_flexspi_nor_config.c
        fsl_flexspi_nor_boot.c
        pin_mux_evk.c
    )
    set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS_REVISION-m7
        CPU_MIMXRT1176DVMAA_cm7
    )
elseif(VALIANT_BOARD_REVISION STREQUAL P0)
    set(libs_nxp_rt1176-sdk_SOURCES_REVISION
        pin_mux_p0.c
    )
    set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS_REVISION-m7
        CPU_MIMXRT1176DVMAA_cm7
    )
elseif(VALIANT_BOARD_REVISION STREQUAL P1)
    set(libs_nxp_rt1176-sdk_SOURCES_REVISION
        pin_mux_p0.c
    )
    set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS_REVISION-m7
        CPU_MIMXRT1176CVM8A_cm7
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
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/flash/nand/flexspi/fsl_flexspi_nand_flash.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists/fsl_component_generic_list.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/phy/device/phyrtl8211f/fsl_phyrtl8211f.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/phy/mdio/enet/fsl_enet_mdio.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_port_uart.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart/fsl_adapter_lpuart.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dac12.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_enet.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_flexspi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpadc.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_mu.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pmu.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pwm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pxp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_romapi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_semc.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_tempsensor.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_xbara.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str/fsl_str.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_dci.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_ehci.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub_app.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_devices.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_ehci.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_framework.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_hid.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/usb_device_ch9.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.c
    ${libs_CMSIS_SOURCES}
)

set(libs_nxp_rt1176-sdk-m7_SOURCES
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
)

set(libs_nxp_rt1176-sdk-m4_SOURCES
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4/fsl_cache.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm4.S
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm4.c
)

set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS
    # dlmalloc defines
    HAVE_MMAP=0
    ONLY_MSPACES
    USE_DL_PREFIX
    malloc_getpagesize=0x800
    # Other defines
    DRIVER_CACHE_CONTROL=1
    LWIP_ENET_FLEXIBLE_CONFIGURATION
    SDK_DEBUGCONSOLE_UART
    SDK_DELAY_USE_DWT
    SERIAL_PORT_TYPE_UART=1
    USB_HOST_CONFIG_DFU
    USE_SDRAM
    __STARTUP_CLEAR_BSS
    __STARTUP_INITIALIZE_NONCACHEDATA
    __USE_SHMEM
)

set(libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS_REVISION-m7}
    XIP_BOOT_HEADER_ENABLE=1
    XIP_EXTERNAL_FLASH=1
)

set(libs_nxp_rt1176-sdk-m4_COMPILE_DEFINITIONS
    CPU_MIMXRT1176DVMAA_cm4
)

set(libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES
    ${VALIANT_SOURCE_DIR}/libs/nxp/rt1176-sdk
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/xip
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/common_task
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/flash/nand
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/phy
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/xip
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy
)

set(libs_nxp_rt1176-sdk-m7_TARGET_INCLUDE_DIRECTORIES
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7
)

set(libs_nxp_rt1176-sdk-m4_TARGET_INCLUDE_DIRECTORIES
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4
)

set(libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES
)

set(libs_nxp_rt1176-sdk-mcmgr_SOURCES
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1170.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr_mu_internal.c
)

set(libs_nxp_rt1176-sdk-mcmgr_COMPILE_DEFINITIONS
    MCMGR_HANDLE_EXCEPTIONS=1
)

add_library_m7(libs_nxp_rt1176-sdk-mcmgr_m7 OBJECT
    ${libs_nxp_rt1176-sdk-mcmgr_SOURCES}
)
target_compile_definitions(libs_nxp_rt1176-sdk-mcmgr_m7 PUBLIC
    ${libs_nxp_rt1176-sdk-mcmgr_COMPILE_DEFINITIONS}
)
target_link_libraries(libs_nxp_rt1176-sdk-mcmgr_m7 PUBLIC
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_bm OBJECT
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
    ${libs_nxp_rt1176-sdk-m7_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
)

add_library_m4(libs_nxp_rt1176-sdk_bm-m4 OBJECT
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
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
    libs_CMSIS-m7
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_bm-m4
    libs_CMSIS-m4
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

add_library_m7(libs_nxp_rt1176-sdk_freertos OBJECT
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart_freertos.c
    ${libs_nxp_rt1176-sdk-m7_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_FreeRTOS_SOURCES}
)

add_library_m4(libs_nxp_rt1176-sdk_freertos-m4 OBJECT
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart_freertos.c
    ${libs_nxp_rt1176-sdk-m4_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_nxp_rt1176-sdk-mcmgr_SOURCES}
    ${libs_FreeRTOS-m4_SOURCES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos PUBLIC
    FSL_RTOS_FREE_RTOS
    USE_RTOS=1
    ${libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC
    FSL_RTOS_FREE_RTOS
    ${libs_nxp_rt1176-sdk-m4_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk-mcmgr_COMPILE_DEFINITIONS}
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
    libs_CMSIS-m7
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

target_link_libraries(libs_nxp_rt1176-sdk_freertos-m4
    libs_FreeRTOS-m4
    libs_CMSIS-m4
    ${libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES}
)

add_library_m7(libs_nxp_rt1176-sdk_sdmmc
    sdmmc_config.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio/fsl_sdio.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_usdhc.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/osa/fsl_sdmmc_osa.c
)

target_include_directories(libs_nxp_rt1176-sdk_sdmmc PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/common
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/host/usdhc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/osa
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio
)

target_link_libraries(libs_nxp_rt1176-sdk_sdmmc
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_littlefs
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_nxp_rt1176-sdk_littlefs PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

add_library_m4(libs_nxp_rt1176-sdk_littlefs-m4
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_nxp_rt1176-sdk_littlefs-m4 PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

add_library_m7(libs_nxp_rt1176-sdk_lwip
    dhcp_server.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/enet_ethernetif.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/enet_ethernetif_kinetis.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/sys_arch.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/api_lib.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/api_msg.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/err.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netbuf.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netdb.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netifapi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/sockets.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/tcpip.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/raw.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/def.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/dns.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/inet_chksum.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/sys.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/init.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ip.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/acd.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/autoip.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/dhcp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/etharp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/icmp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/igmp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4_addr.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4_frag.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/mem.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/memp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/netif.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/pbuf.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/stats.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp_in.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp_out.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/timeouts.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/udp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/netif/ethernet.c
)

target_include_directories(libs_nxp_rt1176-sdk_lwip PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/include/compat/posix
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_lwip_httpd
    ${VALIANT_SOURCE_DIR}/libs/base/httpd.cc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/http/fs.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/http/httpd.c
)

target_compile_definitions(libs_nxp_rt1176-sdk_lwip_httpd PUBLIC
    LWIP_HTTPD_CGI_SSI
    LWIP_HTTPD_CUSTOM_FILES
    LWIP_HTTPD_DYNAMIC_FILE_READ
    LWIP_HTTPD_DYNAMIC_HEADERS
    LWIP_HTTPD_SUPPORT_POST
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip_httpd
    libs_nxp_rt1176-sdk_lwip
)

add_library_m7(libs_nxp_rt1176-sdk_lwip_mdns
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/mdns/mdns.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/mdns/mdns_domain.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/mdns/mdns_out.c
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip_mdns
    libs_nxp_rt1176-sdk_lwip
)

add_library_m7(libs_nxp_rt1176-sdk_wiced
    bt_firmware_controller.c
    WICED/WWD/internal/chips/43455/resources.c
    WICED/platform/MCU/LPC/WWD/wwd_platform.cc
    rtos/freertos/vendors/nxp/wifi/wiced/iot_wifi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/imxrt_port/platform_bluetooth.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/imxrt_port/platform_imxrt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/imxrt_port/wiced_rtos.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WWD/wwd_rtos.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO/wwd_bus_protocol.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/wwd_bus_common.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/wwd_ap.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/wwd_chip_specific_functions.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/tlv.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_ap_common.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_clm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_debug.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_internal.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_management.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_sdpcm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_thread.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi_sleep.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/internal/wifi.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD/wwd_buffer.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD/wwd_network.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/LPC/WWD/wwd_SDIO.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/LPC/WWD/wwd_platform.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/wwd_resources.c
    DATA
    ${VALIANT_SOURCE_DIR}/third_party/firmware/cypress/43455C0.bin
    ${VALIANT_SOURCE_DIR}/third_party/firmware/cypress/43455C0.clm_blob
    ${VALIANT_SOURCE_DIR}/third_party/cyw-bt-patch/BCM4345C0_003.001.025.0144.0266.1MW.hcd
)

target_include_directories(libs_nxp_rt1176-sdk_wiced PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/bt_app_inc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/BTE/Components/stack/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/BTE/Projects/bte/main
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/BTE/WICED
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/imxrt_port
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WWD
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/RTOS
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/network
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/LPC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/platforms/MURATA_TYPE1MW
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/libraries/c_sdk/standard/common/include
)

target_compile_definitions(libs_nxp_rt1176-sdk_wiced PUBLIC
    # Wi-Fi
    WLAN_ARM_CR4
    USES_RESOURCES_IN_EXTERNAL_STORAGE
    HIGH_SPEED_SDIO_CLOCK
    WPRINT_PLATFORM_PERMISSION

    # Bluetooth
    WICED_BLUETOOTH_PLATFORM
    BOARD_BT_UART_BASEADDR=LPUART2
    BOARD_BT_UART_IRQ=LPUART2_IRQn
    BOARD_INITPINS_BT_REG_ON_GPIO=GPIO10
    BOARD_INITPINS_BT_REG_ON_PIN=2
    BOARD_BT_UART_CLK_FREQ=CLOCK_GetRootClockFreq\(kCLOCK_Root_Lpuart2\)
)

target_link_libraries(libs_nxp_rt1176-sdk_wiced
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_sdmmc
)

add_library_m7(libs_nxp_rt1176-sdk-mbedtls
    mbedtls.cc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/aes.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/arc4.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/asn1parse.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/asn1write.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/base64.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/bignum.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/camellia.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ccm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/certs.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/chacha20.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/chachapoly.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/cipher.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/cipher_wrap.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ctr_drbg.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/debug.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/des.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/dhm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ecdh.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ecdsa.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ecp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/entropy.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/error.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/gcm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/hmac_drbg.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/md.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/md5.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/md_wrap.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/oid.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pem.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pk.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pk_wrap.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkcs12.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkcs5.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkparse.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkparse.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkwrite.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/platform.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/platform_util.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/poly1305.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ripemd160.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ripemd160.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/rsa.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/rsa_internal.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/sha1.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/sha256.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/sha512.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_ciphersuites.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_cli.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_srv.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_tls.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/timing.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/version.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509_crl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509_crt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509_crt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk/ecp_alt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk/ecp_curves_alt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk/ksdk_mbedtls.c
)

target_link_libraries(libs_nxp_rt1176-sdk-mbedtls
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk_lwip
)

target_include_directories(libs_nxp_rt1176-sdk-mbedtls PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk
)

target_compile_definitions(libs_nxp_rt1176-sdk-mbedtls PUBLIC
    MBEDTLS_CONFIG_FILE="libs/nxp/rt1176-sdk/ksdk_mbedtls_config.h"
)

target_sources(libs_nxp_rt1176-sdk_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)
