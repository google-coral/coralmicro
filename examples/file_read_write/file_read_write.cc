// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <cstring>
#include <string>

#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

// Reads and writes files to the filesystem on flash storage.

// [start-sphinx-snippet:file-rw]
namespace coralmicro {
namespace {
void PrintDirectory(lfs_dir_t* dir, const char* path, int num_tabs) {
  constexpr int kMaxDepth = 3;
  if (num_tabs > kMaxDepth) {
    return;
  }

  lfs_info info;
  while (lfs_dir_read(Lfs(), dir, &info) > 0) {
    if (info.name[0] == '.') {
      continue;
    }

    for (int i = 0; i < num_tabs; ++i) {
      printf("\t");
    }

    printf("%s", info.name);

    if (info.type == LFS_TYPE_DIR) {
      char subpath[LFS_NAME_MAX];
      printf("/\r\n");
      lfs_dir_t subdir;
      snprintf(subpath, LFS_NAME_MAX, "%s/%s", path, info.name);
      CHECK(lfs_dir_open(Lfs(), &subdir, subpath) >= 0);
      PrintDirectory(&subdir, subpath, num_tabs + 1);
      CHECK(lfs_dir_close(Lfs(), &subdir) >= 0);
    } else {
      printf("\t\t%ld\r\n", info.size);
    }
  }
}

void PrintFilesystemContents() {
  lfs_dir_t root;
  CHECK(lfs_dir_open(Lfs(), &root, "/") >= 0);
  printf("Printing filesystem:\r\n");
  PrintDirectory(&root, "", 0);
  printf("Finished printing filesystem.\r\n");
  CHECK(lfs_dir_close(Lfs(), &root) >= 0);
}

bool Mkdir(const char* path) {
  int ret = lfs_mkdir(Lfs(), path);
  if (ret == LFS_ERR_EXIST) {
    printf("Error dir exists");
    return false;
  }
  return (ret == LFS_ERR_OK);
}

void Main() {
  printf("Begin filesystem example\r\n");
  // Turn on Status LED to show the board is on.
  LedSet(Led::kStatus, true);
  PrintFilesystemContents();

  printf("Creating some sample directories\r\n");
  CHECK(Mkdir("/dir"));
  CHECK(LfsDirExists("/dir"));
  constexpr char kFileStr[] = "HelloWorld";
  CHECK(LfsWriteFile("/dir/file", reinterpret_cast<const uint8_t*>(kFileStr),
                     std::strlen(kFileStr)));
  PrintFilesystemContents();
  std::string readstr;
  CHECK(LfsReadFile("/dir/file", &readstr));
  CHECK(readstr.length() == std::strlen(kFileStr));
  printf("File contents: %s\r\n", readstr.c_str());
  CHECK(lfs_remove(Lfs(), "/dir/file") >= 0);
  CHECK(lfs_remove(Lfs(), "/dir") >= 0);
  PrintFilesystemContents();
  printf("End filesystem example\r\n");
}

}  // namespace
}  // namespace coralmicro

extern "C" void app_main(void* param) {
  (void)param;
  coralmicro::Main();
  vTaskSuspend(nullptr);
}
// [end-sphinx-snippet:file-rw]
