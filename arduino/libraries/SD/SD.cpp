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

#include "SD.h"

namespace coralmicro {
namespace arduino {
namespace SDLib {
using coralmicro::filesystem::Lfs;

bool SDClass::begin(uint8_t csPin) {
  // We ignore csPin, since we don't have a real SD card
  return coralmicro::filesystem::Init();
}

File SDClass::open(const char *filename, uint8_t mode) {
  std::shared_ptr<lfs_file_t> file_handle = std::make_shared<lfs_file_t>();
  std::shared_ptr<lfs_dir_t> dir_handle = std::make_shared<lfs_dir_t>();

  if (lfs_dir_open(Lfs(), dir_handle.get(), filename) >= 0) {
    return File(dir_handle, filename);
  } else if (coralmicro::filesystem::Open(file_handle.get(), filename,
                                       mode == FILE_WRITE, true)) {
    return SDLib::File(file_handle, filename, mode == FILE_WRITE);
  } else {
    return File();
  }
}

bool SDClass::exists(const char *filepath) {
  lfs_dir_t dir_handle;
  lfs_file_t file_handle;
  bool retval = false;

  if (lfs_dir_open(Lfs(), &dir_handle, filepath) >= 0) {
    lfs_dir_close(Lfs(), &dir_handle);
    retval = true;
  } else if (coralmicro::filesystem::Open(&file_handle, filepath, false)) {
    coralmicro::filesystem::Close(&file_handle);
    retval = true;
  }
  return retval;
}

bool SDClass::mkdir(const char *filepath) {
  return coralmicro::filesystem::MakeDirs(filepath);
}

bool SDClass::remove(const char *filepath) {
  return coralmicro::filesystem::Remove(filepath);
}

bool SDClass::rmdir(const char *filepath) {
  return coralmicro::filesystem::Remove(filepath);
}

}  // namespace SDLib
}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::SDLib::SDClass SD;
