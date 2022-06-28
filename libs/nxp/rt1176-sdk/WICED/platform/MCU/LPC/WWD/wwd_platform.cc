/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/nxp/rt1176-sdk/WICED/platform/platform_mcu_peripheral.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_usdhc.h"

/* clang-format off */
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/include/platform_dct.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/platform/wwd_sdio_interface.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/wwd_assert.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/MCU/wiced_dct_common.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/include/platform_constants.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/platform/include/platform_peripheral.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_resource.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_result.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_rtos.h"
/* clang-format on */

// State to hold file handles for the firmware and CLM blob.
// Unfortunately, the WICED api doesn't seems to provide a way to store
// state between calls to get chunks of the file -- so we either must
// make some ourselves, or open/close on each call. That's mega slow
// so just put handles here. Revisit this if we update WICED.
static bool resources_initialized = false;
static lfs_file_t firmware;
static lfs_file_t clm_blob;

using coralmicro::Lfs;
extern const resource_hnd_t wifi_firmware_image;
extern const resource_hnd_t wifi_firmware_clm_blob;

extern "C" resource_result_t platform_read_external_resource(
    const resource_hnd_t* resource, uint32_t offset, uint32_t maxsize,
    uint32_t* size, void* buffer) {
  bool ret;
  int bytes_read;

  if (!resources_initialized) {
    ret = lfs_file_open(Lfs(), &firmware, wifi_firmware_image.val.fs.filename,
                        LFS_O_RDONLY) >= 0;
    if (!ret) {
      return RESOURCE_FILE_OPEN_FAIL;
    }
    ret =
        lfs_file_open(Lfs(), &clm_blob, wifi_firmware_clm_blob.val.fs.filename,
                      LFS_O_RDONLY) >= 0;
    if (!ret) {
      return RESOURCE_FILE_OPEN_FAIL;
    }
    resources_initialized = true;
  }

  resource_result_t result = RESOURCE_SUCCESS;
  lfs_file_t* f;
  if (resource == &wifi_firmware_image) {
    f = &firmware;
  } else if (resource == &wifi_firmware_clm_blob) {
    f = &clm_blob;
  } else {
    return RESOURCE_FILE_OPEN_FAIL;
  }
  ret = lfs_file_seek(Lfs(), f, offset, LFS_SEEK_SET) >= 0;
  if (!ret) {
    result = RESOURCE_FILE_SEEK_FAIL;
    goto exit_close;
  }
  bytes_read = lfs_file_read(Lfs(), f, buffer, maxsize);
  if (bytes_read < 0) {
    result = RESOURCE_FILE_READ_FAIL;
    goto exit_close;
  }

  *size = bytes_read;
exit_close:
  return result;
}

extern "C" int platform_wprint_permission(void) { return 1; }

extern "C" wwd_result_t host_platform_sdio_transfer_internal(
    wwd_bus_transfer_direction_t direction, sdio_command_t command,
    sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument,
    /*@null@*/ uint32_t* data, uint16_t data_size,
    sdio_response_needed_t response_expected,
    usdhc_card_response_type_t response_type, uint32_t response_error_flags,
    /*@out@*/ /*@null@*/ uint32_t* response);

extern "C" wwd_result_t host_platform_sdio_transfer(
    wwd_bus_transfer_direction_t direction, sdio_command_t command,
    sdio_transfer_mode_t mode, sdio_block_size_t block_size, uint32_t argument,
    /*@null@*/ uint32_t* data, uint16_t data_size,
    sdio_response_needed_t response_expected,
    /*@out@*/ /*@null@*/ uint32_t* response) {
  return host_platform_sdio_transfer_internal(
      direction, command, mode, block_size, argument, data, data_size,
      response_expected, kCARD_ResponseTypeR5, 0, response);
}

int platform_write_dct(uint16_t data_start_offset, const void* data,
                       uint16_t data_length, int8_t app_valid,
                       void (*func)(void));

namespace {
const int kDctLength = 32 * 1024;
void* dct_memory = nullptr;
extern "C" uint32_t DCT_section_offsets[];
int dct_inited = 0;
}  // namespace

wiced_result_t dct_init() {
  dct_memory = malloc(kDctLength);
  if (!dct_memory) return WICED_ERROR;
  memset(dct_memory, 0, kDctLength);
  platform_dct_header_t hdr = {.write_incomplete = 0,
                               .app_valid = 1,
                               .mfg_info_programmed = 0,
                               .magic_number = 0x4d435242,
                               .load_app_func = NULL};

  dct_inited = 1;

  int write_result;
  write_result = platform_write_dct(0, &hdr, sizeof(hdr), 1, NULL);
  if (write_result != 0) {
    printf("Error initialising blank DCT");
    return WICED_ERROR;
  }
  return WICED_SUCCESS;
}

platform_dct_data_t* platform_get_dct(void) {
  WICED_DCT_MUTEX_GET();
  if (dct_inited == 0) {
    dct_init();
  }
  WICED_DCT_MUTEX_RELEASE();
  return (platform_dct_data_t*)dct_memory;
}

int platform_write_dct(uint16_t data_start_offset, const void* data,
                       uint16_t data_length, int8_t app_valid,
                       void (*func)(void)) {
  WICED_DCT_MUTEX_GET();
  if (dct_inited == 0) {
    dct_init();
  }
  memcpy((char*)platform_get_dct() + data_start_offset, data, data_length);
  WICED_DCT_MUTEX_RELEASE();
  return 0;
}

wiced_result_t wiced_dct_write(const void* info_ptr, dct_section_t section,
                               uint32_t offset, uint32_t size) {
  int retval;
  WICED_DCT_MUTEX_GET();
  if (dct_inited == 0) {
    dct_init();
  }
  retval = platform_write_dct(DCT_section_offsets[section] + offset, info_ptr,
                              size, 1, NULL);

  WICED_DCT_MUTEX_RELEASE();
  return (retval == 0) ? WICED_SUCCESS : WICED_ERROR;
}

void* wiced_dct_get_current_address(dct_section_t section) {
  return (void*)((char*)platform_get_dct() + DCT_section_offsets[section]);
}

wiced_result_t wiced_dct_read_with_copy(void* info_ptr, dct_section_t section,
                                        uint32_t offset, uint32_t size) {
  char* curr_dct;
  WICED_DCT_MUTEX_GET();
  curr_dct = (char*)wiced_dct_get_current_address(section);
  memcpy(info_ptr, &curr_dct[offset], size);
  WICED_DCT_MUTEX_RELEASE();
  return WICED_SUCCESS;
}

extern "C" wiced_bool_t platform_watchdog_check_last_reset(void) {
  return WICED_FALSE;
}
