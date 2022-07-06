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

#include "libs/base/check.h"
#include "libs/base/filesystem.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

using coralmicro::filesystem::Lfs;

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
void PrintDirectory(lfs_dir_t* dir, int num_tabs) {
    PrintDirectory(dir, "", num_tabs);
}

void PrintFilesystemContents() {
    lfs_dir_t root;
    CHECK(lfs_dir_open(Lfs(), &root, "/") >= 0);
    printf("Printing filesystem:\r\n");
    PrintDirectory(&root, 0);
    printf("Finished printing filesystem.\r\n");
    CHECK(lfs_dir_close(Lfs(), &root) >= 0);
}

bool MkdirOrExists(const char* path) {
    int ret = lfs_mkdir(Lfs(), path);
    return (ret == LFS_ERR_OK || ret == LFS_ERR_EXIST);
}

}  // namespace
void Main() {
    printf("Begin filesystem example\r\n");
    PrintFilesystemContents();

    printf("Creating some sample directories\r\n");
    CHECK(MkdirOrExists("/dir1"));
    CHECK(MkdirOrExists("/dir2"));
    CHECK(MkdirOrExists("/dir1/dir11"));
    CHECK(MkdirOrExists("/dir1/dir13"));
    CHECK(MkdirOrExists("/dir2/dir21"));

    CHECK(coralmicro::filesystem::DirExists("/dir1"));
    CHECK(coralmicro::filesystem::DirExists("/dir2"));
    CHECK(coralmicro::filesystem::DirExists("/dir1/dir11"));
    CHECK(coralmicro::filesystem::DirExists("/dir1/dir13"));
    CHECK(coralmicro::filesystem::DirExists("/dir2/dir21"));

    CHECK(!coralmicro::filesystem::DirExists("/nonexistent"));

    PrintFilesystemContents();

    constexpr char kFile1Str[] = "This is the file content of file1";
    constexpr char kFile2Str[] = "Content for file2";
    lfs_file_t file1;
    CHECK(lfs_file_open(Lfs(), &file1, "/dir1/file1",
                        LFS_O_WRONLY | LFS_O_TRUNC | LFS_O_CREAT) >= 0);
    CHECK(lfs_file_write(Lfs(), &file1, kFile1Str, std::strlen(kFile1Str)) ==
          static_cast<lfs_ssize_t>(std::strlen(kFile1Str)));
    CHECK(lfs_file_close(Lfs(), &file1) >= 0);
    CHECK(coralmicro::filesystem::WriteFile(
        "/dir2/file2", reinterpret_cast<const uint8_t*>(kFile2Str),
        std::strlen(kFile2Str)));

    char readbuf[255];
    CHECK(lfs_file_open(Lfs(), &file1, "/dir1/file1", LFS_O_RDONLY) >= 0);
    CHECK(lfs_file_read(Lfs(), &file1, readbuf, sizeof(readbuf)) ==
          static_cast<lfs_ssize_t>(std::strlen(kFile1Str)));
    CHECK(lfs_file_close(Lfs(), &file1) >= 0);

    std::string readstr;
    CHECK(coralmicro::filesystem::ReadFile("/dir2/file2", &readstr));
    CHECK(readstr.length() == std::strlen(kFile2Str));

    constexpr lfs_soff_t kSeekOffset = 6;
    CHECK(lfs_file_open(Lfs(), &file1, "/dir1/file1", LFS_O_RDONLY) >= 0);
    CHECK(lfs_file_seek(Lfs(), &file1, kSeekOffset, LFS_SEEK_SET) ==
          kSeekOffset);
    CHECK(lfs_file_tell(Lfs(), &file1) == kSeekOffset);
    char readchar = 0;
    CHECK(lfs_file_read(Lfs(), &file1, &readchar, 1) == 1);
    CHECK(readchar == kFile1Str[kSeekOffset]);
    CHECK(lfs_file_close(Lfs(), &file1) >= 0);

    lfs_file_t file2;
    constexpr char kFile2Append[] = "123456";
    CHECK(lfs_file_open(Lfs(), &file2, "/dir2/file2", LFS_O_WRONLY) >= 0);
    CHECK(lfs_file_seek(Lfs(), &file2, 0, LFS_SEEK_END) ==
          lfs_file_size(Lfs(), &file2));
    CHECK(lfs_file_write(Lfs(), &file2, kFile2Append, std::strlen(kFile2Append)) ==
          static_cast<lfs_ssize_t>(std::strlen(kFile2Append)));
    CHECK(lfs_file_size(Lfs(), &file2) ==
          static_cast<lfs_ssize_t>(std::strlen(kFile2Str) + std::strlen(kFile2Append)));
    CHECK(lfs_file_close(Lfs(), &file2) >= 0);

    PrintFilesystemContents();

    CHECK(lfs_remove(Lfs(), "/dir1/file1") >= 0);
    CHECK(lfs_remove(Lfs(), "/dir2/file2") >= 0);
    CHECK(lfs_remove(Lfs(), "/dir2/dir21") >= 0);
    CHECK(lfs_remove(Lfs(), "/dir1/dir13") >= 0);
    CHECK(lfs_remove(Lfs(), "/dir1/dir11") >= 0);
    CHECK(lfs_remove(Lfs(), "/dir2") >= 0);
    CHECK(lfs_remove(Lfs(), "/dir1") >= 0);

    PrintFilesystemContents();
    printf("End filesystem example\r\n");
}

}  // namespace coralmicro
extern "C" void app_main(void* param) {
    (void)param;
    coralmicro::Main();
    vTaskSuspend(nullptr);
}
