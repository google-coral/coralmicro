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

#ifndef LIBS_LIBJPEG_JPEG_H_
#define LIBS_LIBJPEG_JPEG_H_

#include <cstdint>
#include <vector>

namespace coralmicro {

unsigned long JpegCompressRgb(unsigned char* rgb, int width, int height,
                              int quality, unsigned char* buf,
                              unsigned long size);

struct JpegBuffer {
  unsigned char* data;
  unsigned long size;
};

// You have to deallocate data from JpegBuffer with `free()`.
JpegBuffer JpegCompressRgb(unsigned char* rgb, int width, int height,
                           int quality);

void JpegCompressRgb(unsigned char* rgb, int width, int height, int quality,
                     std::vector<uint8_t>* out);

};  // namespace coralmicro

#endif  // LIBS_LIBJPEG_JPEG_H_
