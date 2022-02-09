#ifndef _LIBS_BASE_FILESYSTEM_H_
#define _LIBS_BASE_FILESYSTEM_H_

#include "third_party/littlefs/lfs.h"
#include <cstdlib>
#include <memory>

namespace valiant {
namespace filesystem {

bool Init();
bool Open(lfs_file_t *handle, const char *path);
bool Open(lfs_file_t* handle, const char *path, bool writable);
int Read(lfs_file_t *handle, void *buffer, size_t size);
int Write(lfs_file_t *handle, const void *buffer, size_t size);
bool MakeDirs(const char *path);
std::unique_ptr<char[]> Dirname(const char *path);
bool Seek(lfs_file_t *handle, size_t off, int whence);
bool Close(lfs_file_t *handle);
bool Remove(const char *path);
lfs_soff_t Size(lfs_file_t* handle);
ssize_t Size(const char *path);
std::unique_ptr<uint8_t[]> ReadToMemory(const char *path, size_t* size_bytes);
bool ReadToMemory(const char *path, uint8_t* data, size_t* size_bytes);
bool Stat(const char *path, struct lfs_info *info);
bool DirOpen(lfs_dir_t *dir, const char *path);
int DirRead(lfs_dir_t *dir, struct lfs_info *info);
bool DirClose(lfs_dir_t *dir);

}  // namespace filesystem
}  // namespace valiant

#endif  // _LIBS_BASE_FILESYSTEM_H_
