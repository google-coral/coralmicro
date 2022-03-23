#include "libs/base/mutex.h"
#include "libs/base/filesystem.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/nxp/rt1176-sdk/components/flash/nand/fsl_nand_flash.h"

#include <cstdio>
#include <memory>

extern "C" nand_handle_t* BOARD_GetNANDHandle(void);

namespace valiant {
namespace filesystem {
namespace {

struct AutoClose {
  lfs_file_t* handle;
  ~AutoClose() { Close(handle); }
};

lfs_t lfs_handle_;
lfs_config lfs_config_;
SemaphoreHandle_t lfs_semaphore_;

constexpr int kPagesPerBlock = 64;
constexpr int kFilesystemBaseBlock = 12;
constexpr lfs_size_t kPageSize = 2048;

int LfsRead(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
            void *buffer, lfs_size_t size) {
    nand_handle_t* nand = BOARD_GetNANDHandle();
    if (!nand) return LFS_ERR_IO;

    auto* buf = reinterpret_cast<uint8_t*>(buffer);
    while (size != 0) {
        auto page_index = off / kPageSize;
        auto read_size = std::min(kPageSize, size);
        status_t status = Nand_Flash_Read_Page(nand,
            (kFilesystemBaseBlock + block) * kPagesPerBlock + page_index,
            buf, read_size);
        if (status != kStatus_Success) return LFS_ERR_IO;

        off += read_size;
        buf += read_size;
        size -= read_size;
    }
    return LFS_ERR_OK;
}

int LfsProg(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
            const void *buffer, lfs_size_t size) {
    nand_handle_t* nand = BOARD_GetNANDHandle();
    if (!nand) return LFS_ERR_IO;

    auto* buf = reinterpret_cast<const uint8_t*>(buffer);
    while (size != 0) {
        auto page_index = off / kPageSize;
        auto write_size = std::min(kPageSize, size);
        status_t status = Nand_Flash_Page_Program(nand,
            (kFilesystemBaseBlock + block) * kPagesPerBlock + page_index,
            buf, write_size);
        if (status != kStatus_Success) return LFS_ERR_IO;

        off += write_size;
        buf += write_size;
        size -= write_size;
    }

    return LFS_ERR_OK;
}

int LfsErase(const struct lfs_config *c, lfs_block_t block) {
    nand_handle_t* nand = BOARD_GetNANDHandle();
    if (!nand) return LFS_ERR_IO;
    status_t status = Nand_Flash_Erase_Block(nand, kFilesystemBaseBlock + block);
    if (status != kStatus_Success) return LFS_ERR_IO;
    return LFS_ERR_OK;
}

int LfsSync(const struct lfs_config *c) {
    return LFS_ERR_OK;
}

}  // namespace

bool Init(bool force_format) {
    memset(&lfs_config_, 0, sizeof(lfs_config_));
    lfs_config_.read = LfsRead;
    lfs_config_.prog = LfsProg;
    lfs_config_.erase = LfsErase;
    lfs_config_.sync = LfsSync;
    lfs_config_.read_size = kPageSize;
    lfs_config_.prog_size = kPageSize;
    lfs_config_.block_size = 131072;
    lfs_config_.block_count = 512;
    lfs_config_.block_cycles = 250;
    lfs_config_.cache_size = kPageSize;
    lfs_config_.lookahead_size = kPageSize;

    if (force_format) {
        int ret = lfs_format(&lfs_handle_, &lfs_config_);
        if (ret < 0) {
            return false;
        }
    }

    int ret = lfs_mount(&lfs_handle_, &lfs_config_);
    if (ret < 0) {
        // No filesystem detected, go ahead and format.
        ret = lfs_format(&lfs_handle_, &lfs_config_);
        if (ret < 0) {
            return false;
        }

        // Filesystem should exist now, mount up.
        // If this fails, return an error.
        ret = lfs_mount(&lfs_handle_, &lfs_config_);
        if (ret < 0) {
            return false;
        }
    }

    lfs_semaphore_ = xSemaphoreCreateMutex();
    if (!lfs_semaphore_) {
        return false;
    }
    return true;
}

bool Open(lfs_file_t* handle, const char *path) {
    return Open(handle, path, false);
}

bool Open(lfs_file_t* handle, const char *path, bool writable, bool append) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_file_open(&lfs_handle_, handle, path, writable ? ((append ? LFS_O_APPEND : LFS_O_TRUNC) | LFS_O_CREAT | LFS_O_RDWR) : LFS_O_RDONLY);

    return (ret == LFS_ERR_OK);
}

int Write(lfs_file_t *handle, const void *buffer, size_t size) {
    MutexLock lock(lfs_semaphore_);
    return lfs_file_write(&lfs_handle_, handle, buffer, size);
}

std::unique_ptr<char[]> Dirname(const char *path) {
    if (path[0] != '/') {
        return nullptr;
    }

    int last_separator = 0;
    for (size_t i = 0; i < strlen(path); ++i) {
        if (path[i] == '/') {
            last_separator = i;
        }
    }

    std::unique_ptr<char[]> dir(new char[last_separator + 1]);
    memset(dir.get(), 0, last_separator + 1);
    memcpy(dir.get(), path, last_separator);
    return dir;
}

bool MakeDirs(const char *path) {
    int ret;
    size_t path_len = strlen(path);
    if (path_len > lfs_handle_.name_max) {
        return false;
    }

    MutexLock lock(lfs_semaphore_);
    std::unique_ptr<char[]> path_copy(new char[path_len + 1]);
    memset(path_copy.get(), 0, path_len + 1);
    memcpy(path_copy.get(), path, path_len);

    for (size_t i = 1; i < path_len; ++i) {
        if (path_copy[i] == '/') {
            path_copy[i] = 0;
            ret = lfs_mkdir(&lfs_handle_, path_copy.get());
            if (ret < 0 && ret != LFS_ERR_EXIST) {
                return false;
            }
            path_copy[i] = '/';
        }
    }
    ret = lfs_mkdir(&lfs_handle_, path_copy.get());
    if (ret < 0 && ret != LFS_ERR_EXIST) {
        return false;
    }

    return true;
}

bool OpenDir(lfs_dir_t *dir, const char *path) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_dir_open(&lfs_handle_, dir, path);

    if (ret < 0) {
        return false;
    }

    return true;
}

bool ReadDir(lfs_dir_t *dir, lfs_info *info)  {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_dir_read(&lfs_handle_, dir, info);

    if (ret <= 0) {
        return false;
    }

    return true;
}


bool CloseDir(lfs_dir_t *dir) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_dir_close(&lfs_handle_, dir);

    if (ret < 0) {
        return false;
    }

    return true;
}

bool RewindDir(lfs_dir_t *dir) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_dir_rewind(&lfs_handle_, dir);

    if (ret < 0) {
        return false;
    }

    return true;
}

int Read(lfs_file_t* handle, void *buffer, size_t size) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_file_read(&lfs_handle_, handle, buffer, size);

    return ret;
}

bool Seek(lfs_file_t* handle, size_t off, int whence) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_file_seek(&lfs_handle_, handle, off, whence);

    return (ret >= 0) ? true : false;
}

size_t Position(lfs_file_t* handle) {
    return lfs_file_seek(&lfs_handle_, handle, 0, LFS_SEEK_CUR);
}

bool Close(lfs_file_t* handle) {
    int ret;
    MutexLock lock(lfs_semaphore_);
    ret = lfs_file_close(&lfs_handle_, handle);

    return (ret == LFS_ERR_OK) ? true : false;
}

ssize_t Size(const char *path) {
    lfs_file_t handle;
    if (!Open(&handle, path)) {
        return -1;
    }

    ssize_t ret = Size(&handle);
    Close(&handle);
    return ret;
}

lfs_soff_t Size(lfs_file_t* handle) {
    MutexLock lock(lfs_semaphore_);
    return lfs_file_size(&lfs_handle_, handle);
}

std::unique_ptr<uint8_t[]> ReadToMemory(const char *path, size_t* size_bytes) {
    if (size_bytes) *size_bytes = 0;

    lfs_file_t handle;
    if (!Open(&handle, path)) return nullptr;
    AutoClose close{&handle};

    auto file_size = Size(&handle);
    auto data = std::make_unique<uint8_t[]>(file_size);

    if (Read(&handle, data.get(), file_size) < 0) return nullptr;

    if (size_bytes) *size_bytes = file_size;
    return data;
}

bool ReadToMemory(const char *path, uint8_t* data, size_t* size_bytes) {
    lfs_file_t handle;
    lfs_soff_t file_size;

    if (!data || !size_bytes) {
        return false;
    }

    if (!Open(&handle, path)) {
        goto fail;
    }

    file_size = Size(&handle);

    if (static_cast<size_t>(file_size) > *size_bytes) {
        goto fail_close;
    }

    if (Read(&handle, data, file_size) < 0) {
        goto fail_close;
    }

    Close(&handle);
    if (size_bytes) {
        *size_bytes = file_size;
    }
    return true;

fail_close:
    Close(&handle);
fail:
    if (size_bytes) {
        *size_bytes = 0;
    }
    return false;
}

bool Remove(const char *path) {
    MutexLock lock(lfs_semaphore_);
    int ret = lfs_remove(&lfs_handle_, path);
    if (ret < 0) {
        return false;
    }
    return true;
}

bool Stat(const char *path, struct lfs_info *info) {
    MutexLock lock(lfs_semaphore_);
    int ret = lfs_stat(&lfs_handle_, path, info);
    if (ret < 0) {
        return false;
    }
    return true;
}

bool DirOpen(lfs_dir_t *dir, const char *path) {
    MutexLock lock(lfs_semaphore_);
    int ret = lfs_dir_open(&lfs_handle_, dir, path);
    if (ret < 0) {
        return false;
    }
    return true;
}

int DirRead(lfs_dir_t *dir, struct lfs_info *info) {
    MutexLock lock(lfs_semaphore_);
    return lfs_dir_read(&lfs_handle_, dir, info);
}

bool DirClose(lfs_dir_t *dir) {
    MutexLock lock(lfs_semaphore_);
    int ret = lfs_dir_close(&lfs_handle_, dir);
    if (ret < 0) {
        return false;
    }
    return true;
}

}  // namespace filesystem
}  // namespace valiant
