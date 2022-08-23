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

// Returns littlefs instance to use with functions from `lfs.h`.
//
// @returns Pointer to littlefs instance.
lfs_t* Lfs();

// @cond Do not generate docs
bool LfsInit(bool force_format = false);
// @endcond

// Creates directory, similar to `mkdir -p <path>`.
//
// @param path Directory path.
// @returns True upon success, false otherwise.
bool LfsMakeDirs(const char* path);

// Returns the string up to, but not including, the final '/' in the path.
//
// @param path Path.
// @returns The string up to, but not including, the final '/' in the path.
std::string LfsDirname(const char* path);

// Returns file size.
//
// @param path File path.
// @returns File size in bytes.
ssize_t LfsSize(const char* path);

// Returns whether path exists and represents a directory.
//
// @param path Path to check
// @returns True if path exists and represents a directory, false otherwise.
bool LfsDirExists(const char* path);

// Returns whether path exists and represents a file.
//
// @param path Path to check
// @returns true if path exists and represents a file, false otherwise.
bool LfsFileExists(const char* path);

// Reads content of the file and stores it in `std::vector<uint8_t>`.
//
// @param path File path to read.
// @param buf Instance of `std::vector<uint8_t>` to read data in.
// @returns True upon success, false otherwise.
bool LfsReadFile(const char* path, std::vector<uint8_t>* buf);

// Reads content of the file and stores it in `std::string`.
//
// @param path File path to read.
// @param str Instance of `std::string` to read data in.
// @returns True upon success, false otherwise.
bool LfsReadFile(const char* path, std::string* str);

// Reads content of the file and stores it in memory buffer.
//
// @param path File path to read.
// @param buf Memory buffer to read data in.
// @param size Memory buffer size.
// @returns Number of read bytes not exceeding `size`.
size_t LfsReadFile(const char* path, uint8_t* buf, size_t size);

// Writes content of the memory buffer to file.
//
// @param path File path to write.
// @param buf Memory buffer to write data from.
// @param size Memory buffer size.
// @returns True upon success, false otherwise.
bool LfsWriteFile(const char* path, const uint8_t* buf, size_t size);

// Writes content of the string to file.
//
// @param path File path to write.
// @param str String to write data from.
// @returns True upon success, false otherwise.
bool LfsWriteFile(const char* path, const std::string& str);

}  // namespace coralmicro

#endif  // LIBS_BASE_FILESYSTEM_H_
