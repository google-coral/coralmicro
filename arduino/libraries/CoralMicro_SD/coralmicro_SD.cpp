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

#include "coralmicro_SD.h"

namespace coralmicro {
namespace arduino {
namespace SDLib {
using coralmicro::Lfs;

bool SDClass::begin(uint8_t csPin) {
  // We ignore csPin, since we don't have a real SD card
  return coralmicro::LfsInit();
}

File SDClass::open(const char *filename, uint8_t mode) {
  std::shared_ptr<lfs_file_t> file_handle = std::make_shared<lfs_file_t>();
  std::shared_ptr<lfs_dir_t> dir_handle = std::make_shared<lfs_dir_t>();

  if (lfs_dir_open(Lfs(), dir_handle.get(), filename) >= 0) {
    return SDLib::File(dir_handle, filename);
  } else if (lfs_file_open(Lfs(), file_handle.get(), filename,
                           mode == FILE_WRITE
                               ? LFS_O_APPEND | LFS_O_CREAT | LFS_O_RDWR
                               : LFS_O_RDONLY) >= 0) {
    return SDLib::File(file_handle, filename, mode == FILE_WRITE);
  } else {
    return SDLib::File();
  }
}

bool SDClass::exists(const char *filepath) {
  lfs_dir_t dir_handle;
  lfs_file_t file_handle;
  bool retval = false;

  if (lfs_dir_open(Lfs(), &dir_handle, filepath) >= 0) {
    lfs_dir_close(Lfs(), &dir_handle);
    retval = true;
  } else if (lfs_file_open(Lfs(), &file_handle, filepath, LFS_O_RDONLY) >= 0) {
    lfs_file_close(Lfs(), &file_handle);
    retval = true;
  }
  return retval;
}

bool SDClass::mkdir(const char *filepath) {
  return coralmicro::LfsMakeDirs(filepath);
}

bool SDClass::remove(const char *filepath) {
  return lfs_remove(Lfs(), filepath) >= 0;
}

bool SDClass::rmdir(const char *filepath) {
  return lfs_remove(Lfs(), filepath) >= 0;
}

}  // namespace SDLib
}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::SDLib::SDClass SD;
