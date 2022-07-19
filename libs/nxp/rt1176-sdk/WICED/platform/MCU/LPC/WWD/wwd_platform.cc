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
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/include/wwd_assert.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/internal/chips/43455/resources.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/include/wiced_resource.h"

// State to hold file handles for the firmware and CLM blob.
// Unfortunately, the WICED api doesn't seems to provide a way to store
// state between calls to get chunks of the file -- so we either must
// make some ourselves, or open/close on each call. That's mega slow
// so just put handles here. Revisit this if we update WICED.
static bool resources_initialized = false;
static lfs_file_t firmware;
static lfs_file_t clm_blob;

extern "C" resource_result_t platform_read_external_resource(
    const resource_hnd_t* resource, uint32_t offset, uint32_t maxsize,
    uint32_t* size, void* buffer) {
  using coralmicro::Lfs;

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
