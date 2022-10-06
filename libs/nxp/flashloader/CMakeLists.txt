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

add_executable_m7(flashloader
    LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader/cm7/armgcc/MIMXRT1176_ram_flashloader.ld
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader/cm7/pin_mux.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader/cm7/usb_device_ch9.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader/cm7/usb_device_class.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader/cm7/usb_device_hid.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists/fsl_component_generic_list.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa/fsl_os_abstraction_bm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_clock.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_common_arm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_gpio.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpspi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpuart.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pmu.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/authentication/key_store_puf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/autobaud/src/autobaud_irq.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bm_usb/composite.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bm_usb/hid_bootloader.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bm_usb/usb_descriptor.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_app_crc_check.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_command.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_context.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_exception_handler.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_main.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_misc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_shutdown_cleanup.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_tree_root.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/bl_user_entry.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/lpuart_peripheral_interface.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader/src/usb_hid_msc_peripheral_interface.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/crc/src/crc16.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/crc/src/crc32.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/flexspi/bl_flexspi.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/flexspi_nand/flexspi_nand_flash.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/flexspi_nor/flexspi_nor_flash.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/microseconds/src/microseconds_pit.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/nand_ecc/nand_ecc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/ocotp/bl_ocotp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/sdmmc/port/usdhc/polling/fsl_sdmmc_host.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/sdmmc/src/fsl_mmc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/sdmmc/src/fsl_sd.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/sdmmc/src/fsl_sdmmc_common.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/semc/bl_semc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/semc_nand/semc_nand_flash.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/serial_nor_eeprom/serial_nor_eeprom.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/usdhc/bl_usdhc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/flexspi_nor_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/mmc_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/normal_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/pattern_fill_gcc.S
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/sd_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/semc_nand_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/spi_nor_eeprom_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src/spinand_memory.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/packet/src/serial_packet.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/property/src/property_imx.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/sbloader/src/sbloader.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/security/keyblob/src/keyblob.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/security/keyblob/src/keyblob_caam.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/startup/crt0_gcc.S
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/startup/startup.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/utilities/src/debug_util.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/utilities/src/fsl_assert.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/utilities/src/fsl_rtos_abstraction.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/bl_lpuart_irq_config_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/clock_cfg_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/external_memory_property_map_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/flexspi_config_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/flexspi_nand_config_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/hardware_init_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/memory_map_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/periph_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/sdmmc_config_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/sdmmc_convert_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/security_config_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/semc_nand_config_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/sram_init_MIMXRT1176.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/startup/gcc/startup_MIMXRT1176_cm7.S
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src/startup/system_MIMXRT1176_cm7.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/common/src/pinmux_utility_imxrt_series.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_dci.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device/usb_device_ehci.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy/usb_phy.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/platform/drivers/puf/fsl_puf.c
)

target_include_directories(flashloader PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/CMSIS/CMSIS/Core/Include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/boards/evkmimxrt1170/bootloader_examples/flashloader/cm7
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/lists
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/components/osa
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/authentication
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/autobaud
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bm_usb
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/bootloader
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/crc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/flexspi
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/flexspi_nand
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/flexspi_nor
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/microseconds
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/microseconds
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/nand_ecc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/ocotp
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/sdmmc/inc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/semc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/semc_nand
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/serial_nor_eeprom
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/drivers/usdhc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/memory/src
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/packet
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/property
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/sbloader
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/security/keyblob
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/startup
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/src/utilities
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/mcu-boot/targets/MIMXRT1176/src
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/device
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/usb/phy
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/platform/drivers/puf
)

target_compile_definitions(flashloader PUBLIC
    NDEBUG
    CPU_MIMXRT1176DVMAA_cm7
    __STARTUP_CLEAR_BSS
    CPU_IS_ARM_CORTEX_M7=1
    BL_TARGET_RAM
    MIMXRT1176
    USB_STACK_BM
)
