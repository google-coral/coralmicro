#include <cstddef>

namespace valiant {

enum class IOStatus { kOk, kEof, kError };

IOStatus ReadBytes(int fd, void* bytes, size_t size);

template <typename T>
IOStatus ReadArray(int fd, T* array, size_t array_size) {
    return ReadBytes(fd, array, array_size * sizeof(T));
}

IOStatus WriteBytes(int fd, const void* bytes, size_t size,
                    size_t chunk_size = 1024);

template <typename T>
IOStatus WriteArray(int fd, const T* array, size_t array_size) {
    return WriteBytes(fd, array, array_size * sizeof(T));
}

int SocketServer(int port, int backlog);

}  // namespace valiant
