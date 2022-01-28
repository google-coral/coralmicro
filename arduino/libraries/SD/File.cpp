#include "SD.h"

namespace valiant {
namespace arduino {
namespace SDLib {

File::File(std::shared_ptr<lfs_file_t> handle, const char *name, bool writable)
    : file_handle_(handle), open_(true), is_writable_(writable) {
  strncpy(name_, name, strlen(name));
  name_[strlen(name)] = 0;
}

File::File(std::shared_ptr<lfs_dir_t> handle, const char *name)
    : dir_handle_(handle), open_(true), is_dir_(true) {
  strncpy(name_, name, strlen(name));
  name_[strlen(name)] = 0;
}

File::File() {
  name_[0] = 0;
}

size_t File::write(uint8_t val) { return write(&val, 1); }

size_t File::write(const uint8_t *buf, size_t sz) {
  if (file_handle_) {
    return valiant::filesystem::Write(file_handle_.get(), buf, sz);
  }
  return 0;
}

int File::peek() {
  if (file_handle_) {
    int retval;
    retval = read();
    if (retval != -1) {
      valiant::filesystem::Seek(file_handle_.get(), -1, LFS_SEEK_CUR);
    }
    return retval;
  }
  return 0;
}

int File::read() {
  int retval;
  read(&retval, 1);
  return retval;
}

int File::read(void *buf, uint16_t nbyte) {
  if (file_handle_) {
    return valiant::filesystem::Read(file_handle_.get(), buf, nbyte);
  }
  return -1;
}

int File::available() {
  if (file_handle_) {
    return size() - position();
  }
  return -1;
}

void File::flush() {
  if (file_handle_) {
    valiant::filesystem::Seek(file_handle_.get(), 0, LFS_SEEK_CUR);
  }
}

bool File::seek(uint32_t pos) {
  if (file_handle_ && pos >= 0 && pos <= size()) {
    return valiant::filesystem::Seek(file_handle_.get(), pos, LFS_SEEK_SET);
  }
  return false;
}

uint32_t File::position() {
  if (file_handle_) {
    // We need to write nothing to the file to make sure the internal
    // structures have been updated for a newly opened writable file
    if (is_writable_) {
      uint8_t junk;
      write(&junk, 0);
    }
    return valiant::filesystem::Position(file_handle_.get());
  }
  return -1;
}

uint32_t File::size() {
  if (file_handle_) {
    return valiant::filesystem::Size(file_handle_.get());
  }
  return -1;
}

void File::close() {
  open_ = false;
  name_[0] = 0;
  if (is_dir_) {
    if (dir_handle_) {
      valiant::filesystem::CloseDir(dir_handle_.get());
      dir_handle_ = nullptr;
      is_dir_ = false;
    }
  } else {
    if (file_handle_) {
      valiant::filesystem::Close(file_handle_.get());
      file_handle_ = nullptr;
    }
  }
}

char *File::name() { return name_; }

File File::openNextFile(uint8_t mode) {
  if (is_dir_ && dir_handle_) {
    char buf[255];
    lfs_info info;
    while (valiant::filesystem::ReadDir(dir_handle_.get(), &info)) {
      if (info.name[0] == '.') {
        continue;
      }
      if (strlen(name_) > 1) {
        strcpy(buf, name_);
        auto length = strlen(buf);
        buf[length] = '/';
        strcpy(buf + length + 1, info.name);
      } else {
        buf[0] = '/';
        strcpy(buf + 1, info.name);
      }
      return SD.open(buf);
    }
  }
  return File();
}

void File::rewindDirectory() {
  if (dir_handle_) {
    valiant::filesystem::RewindDir(dir_handle_.get());
  }
}

}  // namespace SDLib
}  // namespace arduino
}  // namespace valiant