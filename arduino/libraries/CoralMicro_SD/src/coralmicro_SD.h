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

#ifndef SD_h
#define SD_h

#include <cstdint>
#include <memory>

#include "Arduino.h"
#include "libs/base/filesystem.h"

#define FILE_READ 0
#define FILE_WRITE 1
#define DONT_CARE 0

namespace coralmicro {
namespace arduino {

namespace SDLib {

// Represents a file in the flash memory filesystem (represented by `SDClass`).
class File : public Stream {
 private:
  char name_[LFS_NAME_MAX + 1];
  std::shared_ptr<lfs_file_t> file_handle_ = nullptr;
  std::shared_ptr<lfs_dir_t> dir_handle_ = nullptr;
  bool open_ = false;
  bool is_dir_ = false;
  bool is_writable_ = false;

 protected:
  File(std::shared_ptr<lfs_file_t> handle, const char *name, bool writable);
  File(std::shared_ptr<lfs_dir_t> handle, const char *name);

 public:
  File(void);  // 'empty' constructor

  // Writes one byte to the file.
  //
  // @param val The byte to write.
  // @returns The amount of data written.
  virtual size_t write(uint8_t);

  // Writes data to the file.
  //
  // @param buf The data to write.
  // @param size The size of the data.
  // @returns The amount of data written.
  virtual size_t write(const uint8_t *buf, size_t size);

  // Reads one value from the file.
  //
  // @returns The value read from the file.
  virtual int read();

  // Gets the next value in the file, without advancing through the file.
  //
  // @returns The next value in the file.
  virtual int peek();

  // The amount of data left in the file.
  //
  // @returns The amount of data until the end of the file, given the current
  // position.
  virtual int available();

  // Ensures the data is written to the file.
  virtual void flush();

  // Reads data from the file.
  //
  // @param buf The buffer to store the data.
  // @param nbyte The amount of data to read.
  // @returns The amount of data read.
  int read(void *buf, size_t nbyte);

  // Sets the current position of the reader within the file.
  //
  // @param pos The desired index for reading from the file.
  bool seek(size_t pos);

  // Gets the current position of the reader within the file.
  //
  // @returns The position of the reader.
  size_t position();

  // Gets the size of the file.
  //
  // @returns The size of the file.
  size_t size();

  // Closes the file.
  //
  void close();

  // Uses the file object as a bool to determine whether it is open.
  //
  // @returns True if the file is open; false otherwise.
  operator bool() { return open_; }

  // Gets the name of the file.
  //
  // @returns The name of the file.
  char *name();

  // Determines whether the file object is a directory.
  //
  // @returns True if the file object is a directory, false otherwise.
  bool isDirectory(void) { return is_dir_; }

  // Gets the next file or folder if the current file object is a directory
  //
  // @param mode The mode for opening the file.
  // @returns The next file object opened with the given mode if it exists.
  // Otherwise, returns an empty file object.
  File openNextFile(uint8_t mode = FILE_READ);

  // Returns to the first file in the directory.
  //
  void rewindDirectory(void);

  friend class SDClass;
};

// Exposes the Dev Board Micro's internal filesystem, treating it as if it
// were an SD card.
//
// You should not initialize this object yourself; instead
// include `coralmicro_SD.h` and then use the global `SD` instance. `begin()`
// and `end()` are unnecessary calls, because the filesystem is initialized when
// the device boots up and cannot be deinitialized.  Example code can be found
// in `sketches/SDFileSystemTest/`.
class SDClass {
 public:
  // This function is unused and is here to match the Arduino API.
  //
  // @param csPin Unused and is here to match the Arduino API.
  // @returns True if initialization was successful, false otherwise.
  bool begin(uint8_t csPin = DONT_CARE);

  // This function is unused and is here to match the Arduino API.
  //
  // @param csPin Unused and is here to match the Arduino API.
  // @param clock Unused and is here to match the Arduino API.
  // @returns True if initialization was successful, false otherwise.
  bool begin(uint32_t clock, uint8_t csPin) { return begin(); }

  // This function is unused and is here to match the Arduino API.
  //
  void end() {}

  // Opens the file or directory at the given path
  //
  // @param filename The path of the file object.
  // @param mode The mode for opening the file object.
  // @returns The opened file object.
  File open(const char *filename, uint8_t mode = FILE_READ);

  // Opens the file or directory at the given path
  //
  // @param filename The path of the file object.
  // @param mode The mode for opening the file object.
  // @returns The opened file object.
  File open(const String &filename, uint8_t mode = FILE_READ) {
    return open(filename.c_str(), mode);
  }

  // Determines whether a file or directory exists at the given path.
  //
  // @param filepath The path to check for a file.
  // @returns True if a file exists at the path; false otherwise.
  bool exists(const char *filepath);

  // Determines whether a file or directory exists at the given path.
  //
  // @param filepath The path to check for a file.
  // @returns True if a file exists at the path, false otheriwse.
  bool exists(const String &filepath) { return exists(filepath.c_str()); }

  // Creates a directory at the given path.
  //
  // @param filepath The location to make a directory.
  // @returns True if a directory was successfully created, false otherwise.
  bool mkdir(const char *filepath);

  // Creates a directory at the given path.
  //
  // @param filepath The location to make a directory.
  // @returns True if a directory was successfully created, false otherwise.
  bool mkdir(const String &filepath) { return mkdir(filepath.c_str()); }

  // Removes the file at the given path
  //
  // @param filepath The location of the file to remove.
  // @returns True if a file was successfully removed, false otherwise.
  bool remove(const char *filepath);

  // Removes the file at the given path
  //
  // @param filepath The location of the file to remove.
  // @returns True if a file was successfully removed, false otherwise.
  bool remove(const String &filepath) { return remove(filepath.c_str()); }

  // Removes the directory at the given path
  //
  // @param filepath The location of the directory to remove.
  // @returns True if a directory was successfully removed, false otherwise.
  bool rmdir(const char *filepath);

  // Removes the directory at the given path
  //
  // @param filepath The location of the directory to remove.
  // @returns True if a directory was successfully removed, false otherwise.
  bool rmdir(const String &filepath) { return rmdir(filepath.c_str()); }
};

}  // namespace SDLib
}  // namespace arduino
}  // namespace coralmicro
/* using namespace coralmicro::arduino::SDLib; */

typedef coralmicro::arduino::SDLib::File SDFile;

// This is the global `SDClass` instance you should use instead of
// creating your own instance.
extern coralmicro::arduino::SDLib::SDClass SD;

#endif  // SD_h
