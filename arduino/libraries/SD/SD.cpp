#include "SD.h"

namespace coral::micro {
namespace arduino {
namespace SDLib {
bool SDClass::begin(uint8_t csPin) {
  // We ignore csPin, since we don't have a real SD card
  return coral::micro::filesystem::Init();
}

File SDClass::open(const char *filename, uint8_t mode) {
  std::shared_ptr<lfs_file_t> file_handle = std::make_shared<lfs_file_t>();
  std::shared_ptr<lfs_dir_t> dir_handle = std::make_shared<lfs_dir_t>();

  if (coral::micro::filesystem::OpenDir(dir_handle.get(), filename)) {
    return File(dir_handle, filename);
  } else if (coral::micro::filesystem::Open(file_handle.get(), filename,
                                       mode == FILE_WRITE, true)) {
    return SDLib::File(file_handle, filename, mode == FILE_WRITE);
  } else {
    return File();
  }
}

bool SDClass::exists(const char *filepath) {
  lfs_dir_t dir_handle;
  lfs_file_t file_handle;
  bool retval = false;

  if (coral::micro::filesystem::OpenDir(&dir_handle, filepath)) {
    coral::micro::filesystem::CloseDir(&dir_handle);
    retval = true;
  } else if (coral::micro::filesystem::Open(&file_handle, filepath, false)) {
    coral::micro::filesystem::Close(&file_handle);
    retval = true;
  }
  return retval;
}

bool SDClass::mkdir(const char *filepath) {
  return coral::micro::filesystem::MakeDirs(filepath);
}

bool SDClass::remove(const char *filepath) {
  return coral::micro::filesystem::Remove(filepath);
}

bool SDClass::rmdir(const char *filepath) {
  return coral::micro::filesystem::Remove(filepath);
}

}  // namespace SDLib
}  // namespace arduino
}  // namespace coral::micro

coral::micro::arduino::SDLib::SDClass SD;
