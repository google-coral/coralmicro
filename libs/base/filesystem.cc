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

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>

#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/components/flash/nand/fsl_nand_flash.h"

extern "C" nand_handle_t* BOARD_GetNANDHandle(void);

namespace coralmicro {
namespace {
lfs_t g_lfs;
lfs_config g_lfs_config;
SemaphoreHandle_t g_lfs_mutex;

constexpr int kPagesPerBlock = 64;
constexpr int kFilesystemBaseBlock = 12;
constexpr lfs_size_t kPageSize = 2048;

struct AutoClose {
  lfs_file_t* file;
  ~AutoClose() { lfs_file_close(&g_lfs, file); }
};

int LfsRead(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
            void* buffer, lfs_size_t size) {
  nand_handle_t* nand = BOARD_GetNANDHandle();
  if (!nand) return LFS_ERR_IO;

  auto* buf = reinterpret_cast<uint8_t*>(buffer);
  while (size != 0) {
    auto page_index = off / kPageSize;
    auto read_size = std::min(kPageSize, size);
    status_t status = Nand_Flash_Read_Page(
        nand, (kFilesystemBaseBlock + block) * kPagesPerBlock + page_index, buf,
        read_size);
    if (status != kStatus_Success) return LFS_ERR_IO;

    off += read_size;
    buf += read_size;
    size -= read_size;
  }
  return LFS_ERR_OK;
}

int LfsProg(const struct lfs_config* c, lfs_block_t block, lfs_off_t off,
            const void* buffer, lfs_size_t size) {
  nand_handle_t* nand = BOARD_GetNANDHandle();
  if (!nand) return LFS_ERR_IO;

  auto* buf = reinterpret_cast<const uint8_t*>(buffer);
  while (size != 0) {
    auto page_index = off / kPageSize;
    auto write_size = std::min(kPageSize, size);
    status_t status = Nand_Flash_Page_Program(
        nand, (kFilesystemBaseBlock + block) * kPagesPerBlock + page_index, buf,
        write_size);
    if (status != kStatus_Success) return LFS_ERR_IO;

    off += write_size;
    buf += write_size;
    size -= write_size;
  }

  return LFS_ERR_OK;
}

int LfsErase(const struct lfs_config* c, lfs_block_t block) {
  nand_handle_t* nand = BOARD_GetNANDHandle();
  if (!nand) return LFS_ERR_IO;
  status_t status = Nand_Flash_Erase_Block(nand, kFilesystemBaseBlock + block);
  if (status != kStatus_Success) return LFS_ERR_IO;
  return LFS_ERR_OK;
}

int LfsSync(const struct lfs_config* c) { return LFS_ERR_OK; }

int LfsLock(const struct lfs_config* c) {
  if (xSemaphoreTake(g_lfs_mutex, portMAX_DELAY) != pdTRUE) return -1;
  return 0;
}

int LfsUnlock(const struct lfs_config* c) {
  if (xSemaphoreGive(g_lfs_mutex) != pdTRUE) return -1;
  return 0;
}
}  // namespace

lfs_t* Lfs() { return &g_lfs; }

bool LfsInit(bool force_format) {
  if (g_lfs_mutex) vSemaphoreDelete(g_lfs_mutex);

  g_lfs_mutex = xSemaphoreCreateMutex();
  if (!g_lfs_mutex) return false;

  std::memset(&g_lfs_config, 0, sizeof(g_lfs_config));
  g_lfs_config.read = LfsRead;
  g_lfs_config.prog = LfsProg;
  g_lfs_config.erase = LfsErase;
  g_lfs_config.sync = LfsSync;
  g_lfs_config.lock = LfsLock;
  g_lfs_config.unlock = LfsUnlock;
  g_lfs_config.read_size = kPageSize;
  g_lfs_config.prog_size = kPageSize;
  g_lfs_config.block_size = 131072;
  g_lfs_config.block_count = 512;
  g_lfs_config.block_cycles = 250;
  g_lfs_config.cache_size = kPageSize;
  g_lfs_config.lookahead_size = kPageSize;

  if (force_format) {
    int ret = lfs_format(&g_lfs, &g_lfs_config);
    if (ret < 0) {
      return false;
    }
  }

  int ret = lfs_mount(&g_lfs, &g_lfs_config);
  if (ret < 0) {
    // No filesystem detected, go ahead and format.
    ret = lfs_format(&g_lfs, &g_lfs_config);
    if (ret < 0) {
      return false;
    }

    // Filesystem should exist now, mount up.
    // If this fails, return an error.
    ret = lfs_mount(&g_lfs, &g_lfs_config);
    if (ret < 0) {
      return false;
    }
  }
  return true;
}

// Matches the specification of
// https://docs.python.org/3/library/os.path.html#os.path.dirname
// except it assumes only a single '/' in a row.
//
// assert(Dirname("")                == "");
// assert(Dirname("/")               == "/");
// assert(Dirname("/file")           == "/");
// assert(Dirname("file")            == "");
// assert(Dirname("/dir/")           == "/dir");
// assert(Dirname("dir/")            == "dir");
// assert(Dirname("/dir/file")       == "/dir");
// assert(Dirname("dir/file")        == "dir");
// assert(Dirname("/dir1/dir2/")     == "/dir1/dir2");
// assert(Dirname("dir1/dir2/")      == "dir1/dir2");
// assert(Dirname("/dir1/dir2/file") == "/dir1/dir2");
// assert(Dirname("dir1/dir2/file")  == "dir1/dir2");
std::string LfsDirname(const char* path) {
  const std::string s(path);
  if (s.empty()) return "";

  const auto index = s.find_last_of('/');
  if (index == std::string::npos) return "";
  if (index == 0) return "/";  // e.g. "/", "/file"
  return s.substr(0, index);   // e.g. "/dir/", "/dir/file"
}

bool LfsMakeDirs(const char* path) {
  int ret;
  size_t path_len = strlen(path);
  if (path_len > g_lfs.name_max) {
    return false;
  }

  std::unique_ptr<char[]> path_copy(new char[path_len + 1]);
  std::memset(path_copy.get(), 0, path_len + 1);
  std::memcpy(path_copy.get(), path, path_len);

  for (size_t i = 1; i < path_len; ++i) {
    if (path_copy[i] == '/') {
      path_copy[i] = 0;
      ret = lfs_mkdir(&g_lfs, path_copy.get());
      if (ret < 0 && ret != LFS_ERR_EXIST) {
        return false;
      }
      path_copy[i] = '/';
    }
  }
  ret = lfs_mkdir(&g_lfs, path_copy.get());
  if (ret < 0 && ret != LFS_ERR_EXIST) {
    return false;
  }

  return true;
}

ssize_t LfsSize(const char* path) {
  lfs_file_t file;
  if (lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY) < 0) return -1;
  AutoClose close{&file};
  return lfs_file_size(&g_lfs, &file);
}

bool LfsDirExists(const char* path) {
  lfs_dir_t dir;
  if (lfs_dir_open(&g_lfs, &dir, path) < 0) return false;
  lfs_dir_close(&g_lfs, &dir);
  return true;
}

bool LfsFileExists(const char* path) {
  lfs_file_t file;
  if (lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY) < 0) return false;
  lfs_file_close(&g_lfs, &file);
  return true;
}

bool LfsReadFile(const char* path, std::vector<uint8_t>* buf) {
  lfs_file_t file;
  if (lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY) < 0) return false;
  AutoClose close{&file};

  auto file_size = lfs_file_size(&g_lfs, &file);
  if (file_size < 0) return false;
  buf->resize(file_size);

  auto n = lfs_file_read(&g_lfs, &file, buf->data(), buf->size());
  return n >= 0 && static_cast<size_t>(n) == buf->size();
}

bool LfsReadFile(const char* path, std::string* str) {
  lfs_file_t file;
  if (lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY) < 0) return false;
  AutoClose close{&file};

  auto file_size = lfs_file_size(&g_lfs, &file);
  if (file_size < 0) return false;
  str->resize(file_size);

  auto n = lfs_file_read(&g_lfs, &file, str->data(), str->size());
  return n >= 0 && static_cast<size_t>(n) == str->size();
}

size_t LfsReadFile(const char* path, uint8_t* buf, size_t size) {
  lfs_file_t file;
  if (lfs_file_open(&g_lfs, &file, path, LFS_O_RDONLY) < 0) return 0;
  AutoClose close{&file};

  auto file_size = lfs_file_size(&g_lfs, &file);
  if (file_size < 0) return 0;
  size = std::min(size, static_cast<size_t>(file_size));

  auto n = lfs_file_read(&g_lfs, &file, buf, size);
  if (n >= 0) return n;

  return 0;
}

bool LfsWriteFile(const char* path, const uint8_t* buf, size_t size) {
  lfs_file_t file;
  if (lfs_file_open(&g_lfs, &file, path,
                    LFS_O_WRONLY | LFS_O_TRUNC | LFS_O_CREAT) < 0)
    return false;
  AutoClose close{&file};

  auto n = lfs_file_write(&g_lfs, &file, buf, size);
  return n >= 0 && static_cast<size_t>(n) == size;
}

bool LfsWriteFile(const char* path, const std::string& str) {
  return LfsWriteFile(path, reinterpret_cast<const uint8_t*>(str.c_str()),
                      str.size());
}
}  // namespace coralmicro
