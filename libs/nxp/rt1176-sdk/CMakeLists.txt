# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

enable_language(ASM)

set(libs_nxp_rt1176-sdk_SOURCES_REVISION
)

set(libs_nxp_rt1176-sdk_SOURCES
    board_hardware.c
    clock_config.c
    usb_device_cdc_eem.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_msc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_msc_ufi.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/board.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/pin_mux.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/usb_device_class.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/flash/nand/flexspi/fsl_flexspi_nand_flash.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists/fsl_component_generic_list.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/phy/device/phyrtl8211f/fsl_phyrtl8211f.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/phy/mdio/enet/fsl_enet_mdio.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_manager.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager/fsl_component_serial_port_uart.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart/fsl_adapter_lpuart.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_caam.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common_arm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dac12.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dcdc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_dmamux.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_edma.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_enet.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_flexspi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpadc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_mu.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pdm_edma.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pmu.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pwm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pxp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_romapi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_sema4.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_semc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_snvs_hp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_snvs_lp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_soc_src.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_tempsensor.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_wdog.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_xbara.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console/fsl_debug_console.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str/fsl_str.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_dci.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_ehci.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_dfu.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class/usb_host_hub_app.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_devices.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_ehci.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_framework.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/usb_host_hci.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_cdc_acm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class/usb_device_hid.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/usb_device_ch9.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.c
    ${libs_CMSIS_SOURCES}
)

set(libs_nxp_rt1176-sdk-m7_SOURCES
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm7.c
)

set(libs_nxp_rt1176-sdk-m4_SOURCES
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4/fsl_cache.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm4.S
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/system_MIMXRT1176_cm4.c
)

set(libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS
    FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    LWIP_ENET_FLEXIBLE_CONFIGURATION
    LWIP_POSIX_SOCKETS_IO_NAMES=0
    SDK_DEBUGCONSOLE_UART
    SDK_DELAY_USE_DWT
    SERIAL_PORT_TYPE_UART=1
    USB_HOST_CONFIG_DFU
    USE_SDRAM
    __STARTUP_CLEAR_BSS
    __STARTUP_INITIALIZE_NONCACHEDATA
    __USE_SHMEM
    DEBUG_CONSOLE_TRANSFER_NON_BLOCKING
    DATA_SECTION_IS_CACHEABLE=1
)

set(libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS
    CPU_MIMXRT1176CVM8A_cm7
    XIP_BOOT_HEADER_ENABLE=1
    XIP_EXTERNAL_FLASH=1
)

set(libs_nxp_rt1176-sdk-m4_COMPILE_DEFINITIONS
    CPU_MIMXRT1176CVM8A_cm4
)

set(libs_nxp_rt1176-sdk_TARGET_INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/libs/nxp/rt1176-sdk
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/xip
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/common_task
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/flash/nand
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/internal_flash
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/log
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/mem_manager
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/messaging
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/phy
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/serial_manager
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/uart
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/debug_console
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/utilities/str
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/xip
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/host/class
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/output/source/device/class
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy
)

set(libs_nxp_rt1176-sdk-m7_TARGET_INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7
)

set(libs_nxp_rt1176-sdk-m4_TARGET_INCLUDE_DIRECTORIES
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4
)

set(libs_nxp_rt1176-sdk_TARGET_LINK_LIBRARIES
)

set(libs_nxp_rt1176-sdk-mcmgr_SOURCES
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr_internal_core_api_imxrt1170.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/multicore/mcmgr/src/mcmgr_mu_internal.c
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
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
    ${libs_nxp_rt1176-sdk-m7_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
)

add_library_m4(libs_nxp_rt1176-sdk_bm-m4 OBJECT
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
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
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi_freertos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart_freertos.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/fsl_tickless_gpt.c
    ${libs_nxp_rt1176-sdk-m7_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_FreeRTOS_SOURCES}
)

add_library_m4(libs_nxp_rt1176-sdk_freertos-m4 OBJECT
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_free_rtos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi_freertos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart_freertos.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/fsl_tickless_gpt.c
    ${libs_nxp_rt1176-sdk-m4_SOURCES}
    ${libs_nxp_rt1176-sdk_SOURCES}
    ${libs_nxp_rt1176-sdk-mcmgr_SOURCES}
    ${libs_FreeRTOS-m4_SOURCES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos PUBLIC
    SDK_OS_FREE_RTOS
    USE_RTOS=1
    FSL_OSA_TASK_ENABLE
    ${libs_nxp_rt1176-sdk-m7_COMPILE_DEFINITIONS}
    ${libs_nxp_rt1176-sdk_COMPILE_DEFINITIONS}
)

target_compile_definitions(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC
    SDK_OS_FREE_RTOS
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
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/sdmmc_config.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio/fsl_sdio.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_usdhc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/host/usdhc/non_blocking/fsl_sdmmc_host.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/osa/fsl_sdmmc_osa.c
)

target_include_directories(libs_nxp_rt1176-sdk_sdmmc PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/common
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/host/usdhc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/osa
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/sdmmc/sdio
)

target_link_libraries(libs_nxp_rt1176-sdk_sdmmc
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_lwip
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/sntp/sntp.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/dhcp_server.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/lwiperf/lwiperf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/enet_ethernetif.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/enet_ethernetif_kinetis.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port/sys_arch.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/api_lib.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/api_msg.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/err.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netbuf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netdb.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/netifapi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/sockets.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/api/tcpip.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/raw.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/def.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/dns.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/inet_chksum.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/sys.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/init.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ip.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/acd.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/autoip.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/dhcp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/etharp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/icmp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/igmp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4_addr.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/ipv4/ip4_frag.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/mem.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/memp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/netif.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/pbuf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/stats.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp_in.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/tcp_out.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/timeouts.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/core/udp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/netif/ethernet.c
)

target_compile_definitions(libs_nxp_rt1176-sdk_lwip PUBLIC
    SNTP_SERVER_DNS
)

target_include_directories(libs_nxp_rt1176-sdk_lwip PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/port
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/include/compat/posix
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_freertos
)

add_library_m7(libs_nxp_rt1176-sdk_lwip_httpd
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/http/fs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/http/httpd.c
)

target_compile_definitions(libs_nxp_rt1176-sdk_lwip_httpd PUBLIC
    LWIP_HTTPD_CGI_SSI
    LWIP_HTTPD_CUSTOM_FILES
    LWIP_HTTPD_DYNAMIC_FILE_READ
    LWIP_HTTPD_DYNAMIC_HEADERS
    LWIP_HTTPD_FILE_EXTENSION
    LWIP_HTTPD_SUPPORT_POST
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip_httpd
    libs_nxp_rt1176-sdk_lwip
)

add_library_m7(libs_nxp_rt1176-sdk_lwip_mdns
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/mdns/mdns.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/mdns/mdns_domain.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/mdns/mdns_out.c
)

target_link_libraries(libs_nxp_rt1176-sdk_lwip_mdns
    libs_nxp_rt1176-sdk_lwip
)

set(libs_nxp_rt1176-sdk_wiced_SOURCES
    WICED/platform/MCU/LPC/WWD/wwd_platform.cc
    bluetooth.cc
    bt_firmware_controller.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/wwd_SDIO.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/wwd_platform.c
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/rtos/freertos/vendors/nxp/wifi/wiced/iot_wifi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WICED/wiced_rtos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WWD/wwd_rtos.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/wiced_rtos_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO/wwd_bus_protocol.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/wwd_bus_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/wwd_ap.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/wwd_chip_specific_functions.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_ap_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_clm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_debug.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_eapol.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_internal.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_management.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_rtos_interface.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_sdpcm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_thread.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/wwd_wifi_sleep.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/internal/waf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/internal/wiced_core.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/internal/wiced_crypto.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/internal/wifi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WICED/wiced_network.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD/wwd_buffer.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD/wwd_network.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/wiced_network_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/ARM_CM7/host_cm7.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/platform_resource.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/wiced_platform_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/wwd_resources.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/utilities/TLV/tlv.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/platforms/coralmicro/dct_offsets.c
)

set(libs_nxp_rt1176-sdk_wiced_INCLUDE_DIRECTORIES
    WICED/platform
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk
    ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include
    ${PROJECT_SOURCE_DIR}/libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WICED
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/RTOS/FreeRTOS/WWD
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/RTOS
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/network
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/bus_protocols/SDIO
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WICED
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/network/LwIP/WWD
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/ARM_CM7
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/GCC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/security/BESL/P2P
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/security/BESL/WPS
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/security/BESL/crypto_internal
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/security/BESL/host/WICED
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/security/BESL/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/security/BESL/mbedtls_open/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/daemons/DHCP_server
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/protocols/DNS
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/utilities/TLV
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/utilities/linked_list
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/utilities/ring_buffer
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/libraries/utilities/wifi
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/platforms/coralmicro
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/libraries/c_sdk/standard/common/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/rtos/freertos/libraries/c_sdk/standard/common/include/types
)

set(libs_nxp_rt1176-sdk_wiced_COMPILE_DEFINITIONS
    # Wi-Fi
    WLAN_ARM_CR4
    USES_RESOURCES_IN_EXTERNAL_STORAGE
    HIGH_SPEED_SDIO_CLOCK
    WPRINT_PLATFORM_PERMISSION
    WPRINT_ENABLE_WWD_ERROR
    WICED_PAYLOAD_MTU=8320
    WICED
    WWD_DOWNLOAD_CLM_BLOB
    EXTERNAL_DCT
    NETWORK_LwIP
    NO_BUILD_BOOTLOADER
    NO_BOOTLOADER_REQUIRED
    ALTERNATE_MAIN
    WICED_PLATFORM_MASKS_BUS_IRQ
)

set(libs_nxp_rt1176-sdk_wiced_TARGET_LINK_LIBRARIES
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk_sdmmc
)

add_library_m7(libs_nxp_rt1176-sdk_wiced
    WICED/WWD/internal/chips/43455/resources.c
    ${libs_nxp_rt1176-sdk_wiced_SOURCES}
    DATA
    ${PROJECT_SOURCE_DIR}/third_party/firmware/cypress/43455C0.bin>/third_party/firmware/cypress/43455C0.bin
    ${PROJECT_SOURCE_DIR}/third_party/firmware/cypress/43455C0.clm_blob>/third_party/firmware/cypress/43455C0.clm_blob
    ${PROJECT_SOURCE_DIR}/third_party/cyw-bt-patch/BCM4345C0_003.001.025.0144.0266.1MW.hcd>/third_party/cyw-bt-patch/BCM4345C0_003.001.025.0144.0266.1MW.hcd
)

target_include_directories(libs_nxp_rt1176-sdk_wiced PUBLIC
    ${libs_nxp_rt1176-sdk_wiced_INCLUDE_DIRECTORIES}
)

target_compile_definitions(libs_nxp_rt1176-sdk_wiced PUBLIC
    ${libs_nxp_rt1176-sdk_wiced_COMPILE_DEFINITIONS}
)

target_link_libraries(libs_nxp_rt1176-sdk_wiced
    ${libs_nxp_rt1176-sdk_wiced_TARGET_LINK_LIBRARIES}
)

add_library_m7(libs_nxp_rt1176-sdk-mbedtls
    mbedtls.cc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/aes.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/arc4.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/asn1parse.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/asn1write.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/base64.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/bignum.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/camellia.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ccm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/certs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/chacha20.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/chachapoly.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/cipher.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/cipher_wrap.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ctr_drbg.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/debug.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/des.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/dhm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ecdh.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ecdsa.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ecp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/entropy.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/error.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/gcm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/hmac_drbg.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/md.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/md5.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/oid.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pem.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pk.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pk_wrap.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkcs12.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkcs5.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkparse.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkparse.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/pkwrite.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/platform.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/platform_util.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/poly1305.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ripemd160.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ripemd160.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/rsa.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/rsa_internal.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/sha1.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/sha256.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/sha512.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_ciphersuites.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_cli.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_msg.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_srv.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/ssl_tls.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/timing.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/version.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509_crl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509_crt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library/x509_crt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk/ecp_alt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk/ecp_curves_alt.c
)

target_link_libraries(libs_nxp_rt1176-sdk-mbedtls
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk_lwip
)

target_include_directories(libs_nxp_rt1176-sdk-mbedtls PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/port/ksdk
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mbedtls/library
)

target_compile_definitions(libs_nxp_rt1176-sdk-mbedtls PUBLIC
    MBEDTLS_CONFIG_FILE="third_party/modified/nxp/rt1176-sdk/ksdk_mbedtls_config.h"
)

target_sources(libs_nxp_rt1176-sdk_freertos PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS>)
target_sources(libs_nxp_rt1176-sdk_freertos-m4 PUBLIC $<TARGET_OBJECTS:libs_FreeRTOS-m4>)

add_subdirectory(edgefast_bluetooth)
