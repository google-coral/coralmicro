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

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.h"

namespace coralmicro {
namespace filesystem {

lfs_t* Lfs();
bool Init(bool force_format = false);
bool Open(lfs_file_t* handle, const char* path);
bool Open(lfs_file_t* handle, const char* path, bool writable,
          bool append = false);
int Read(lfs_file_t* handle, void* buffer, size_t size);
int Write(lfs_file_t* handle, const void* buffer, size_t size);
bool MakeDirs(const char* path);
std::string Dirname(const char* path);
bool Seek(lfs_file_t* handle, size_t off, int whence);
size_t Position(lfs_file_t* handle);
bool Close(lfs_file_t* handle);
bool Remove(const char* path);
lfs_soff_t Size(lfs_file_t* handle);
ssize_t Size(const char* path);

bool DirExists(const char* path);
bool FileExists(const char* path);
bool ReadFile(const char* path, std::vector<uint8_t>* buf);
bool ReadFile(const char* path, std::string* str);
size_t ReadFile(const char* path, uint8_t* buf, size_t size);
bool WriteFile(const char* path, const uint8_t* buf, size_t size);

}  // namespace filesystem
}  // namespace coralmicro

#endif  // LIBS_BASE_FILESYSTEM_H_
