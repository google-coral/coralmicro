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

#ifndef LIBS_BASE_FILESYSTEM_H_
#define LIBS_BASE_FILESYSTEM_H_

#include <cstdint>
#include <string>
#include <vector>

#include "third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.h"

namespace coralmicro {

lfs_t* Lfs();

bool LfsInit(bool force_format = false);

bool LfsMakeDirs(const char* path);
std::string LfsDirname(const char* path);
ssize_t LfsSize(const char* path);

bool LfsDirExists(const char* path);
bool LfsFileExists(const char* path);
bool LfsReadFile(const char* path, std::vector<uint8_t>* buf);
bool LfsReadFile(const char* path, std::string* str);
size_t LfsReadFile(const char* path, uint8_t* buf, size_t size);
bool LfsWriteFile(const char* path, const uint8_t* buf, size_t size);

}  // namespace coralmicro

#endif  // LIBS_BASE_FILESYSTEM_H_
