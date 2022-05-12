#ifndef _LIBS_BASE_FILESYSTEM_H_
#define _LIBS_BASE_FILESYSTEM_H_

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.h"

namespace coral::micro {
namespace filesystem {

lfs_t* Lfs();
bool Init(bool force_format = false);
bool Open(lfs_file_t* handle, const char* path);
bool Open(lfs_file_t* handle, const char* path, bool writable,
          bool append = false);
int Read(lfs_file_t* handle, void* buffer, size_t size);
int Write(lfs_file_t* handle, const void* buffer, size_t size);
bool MakeDirs(const char* path);
bool OpenDir(lfs_dir_t* dir, const char* path);
bool ReadDir(lfs_dir_t* dir, lfs_info* info);
bool CloseDir(lfs_dir_t* dir);
bool RewindDir(lfs_dir_t* dir);
std::string Dirname(const char* path);
bool Seek(lfs_file_t* handle, size_t off, int whence);
size_t Position(lfs_file_t* handle);
bool Close(lfs_file_t* handle);
bool Remove(const char* path);
lfs_soff_t Size(lfs_file_t* handle);
ssize_t Size(const char* path);
bool Stat(const char* path, struct lfs_info* info);
bool DirOpen(lfs_dir_t* dir, const char* path);
int DirRead(lfs_dir_t* dir, struct lfs_info* info);
bool DirClose(lfs_dir_t* dir);

bool FileExists(const char* path);
bool ReadFile(const char* path, std::vector<uint8_t>* buf);
bool ReadFile(const char* path, std::string* str);
size_t ReadFile(const char* path, uint8_t* buf, size_t size);
bool WriteFile(const char* path, const uint8_t* buf, size_t size);

}  // namespace filesystem
}  // namespace coral::micro

#endif  // _LIBS_BASE_FILESYSTEM_H_
