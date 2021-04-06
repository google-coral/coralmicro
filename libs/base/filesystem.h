#ifndef _LIBS_BASE_FILESYSTEM_H_
#define _LIBS_BASE_FILESYSTEM_H_

#include "third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.h"
#include <cstdlib>

namespace valiant {
namespace filesystem {

bool Init();
bool Open(lfs_file_t *handle, const char *path);
int Read(lfs_file_t *handle, void *buffer, size_t size);
bool Seek(lfs_file_t *handle, size_t off, int whence);
bool Close(lfs_file_t *handle);
uint8_t *ReadToMemory(const char *path, size_t* size_bytes);
bool ReadToMemory(const char *path, uint8_t* data, size_t* size_bytes);

}  // namespace filesystem
}  // namespace valiant

#endif  // _LIBS_BASE_FILESYSTEM_H_
