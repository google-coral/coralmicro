#include <stdint.h>
const char brcm_patch_version[] = "BCM4345C0_003.001.025.0144.0266.hcd";
const uint8_t brcm_patchram_format = 0x01;
unsigned char brcm_patchram_buf[60997] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
unsigned int brcm_patch_ram_length = 60997;
