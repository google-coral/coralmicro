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

#include "libs/libjpeg/jpeg.h"

#include <algorithm>

#include "third_party/nxp/rt1176-sdk/middleware/libjpeg/inc/jpeglib.h"

namespace coralmicro {
namespace {
constexpr size_t kVectorSizeIncrement = 10 * 1024;

struct vector_destination_mgr {
  struct jpeg_destination_mgr pub;

  std::vector<uint8_t>* out;
};

METHODDEF(void)
init_vector_destination(j_compress_ptr cinfo) { (void)cinfo; }

METHODDEF(boolean)
empty_vector_output_buffer(j_compress_ptr cinfo) {
  auto* dest = reinterpret_cast<vector_destination_mgr*>(cinfo->dest);

  auto size = dest->out->size();
  dest->out->resize(size + kVectorSizeIncrement);

  dest->pub.next_output_byte = dest->out->data() + size;
  dest->pub.free_in_buffer = kVectorSizeIncrement;

  return TRUE;
}

METHODDEF(void)
term_vector_destination(j_compress_ptr cinfo) {
  auto* dest = reinterpret_cast<vector_destination_mgr*>(cinfo->dest);

  dest->out->resize(dest->out->size() - dest->pub.free_in_buffer);
}

void jpeg_vector_dest(j_compress_ptr cinfo, std::vector<uint8_t>* out) {
  if (cinfo->dest == nullptr) {
    cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(vector_destination_mgr));
  }

  out->resize(kVectorSizeIncrement);

  auto* dest = reinterpret_cast<vector_destination_mgr*>(cinfo->dest);
  dest->pub.init_destination = init_vector_destination;
  dest->pub.empty_output_buffer = empty_vector_output_buffer;
  dest->pub.term_destination = term_vector_destination;
  dest->pub.next_output_byte = out->data();
  dest->pub.free_in_buffer = kVectorSizeIncrement;

  dest->out = out;
}

struct buf_destination_mgr {
  struct jpeg_destination_mgr pub;
  unsigned long size;
  unsigned long* out_size;
};

METHODDEF(void)
init_buf_destination(j_compress_ptr cinfo) { (void)cinfo; }

METHODDEF(boolean)
empty_buf_output_buffer(j_compress_ptr cinfo) {
  (void)cinfo;
  return FALSE;
}

METHODDEF(void)
term_buf_destination(j_compress_ptr cinfo) {
  auto* dest = reinterpret_cast<buf_destination_mgr*>(cinfo->dest);
  *dest->out_size = dest->size - dest->pub.free_in_buffer;
}

void jpeg_buf_dest(j_compress_ptr cinfo, unsigned char* buf, unsigned long size,
                   unsigned long* out_size) {
  if (cinfo->dest == nullptr) {
    cinfo->dest = (struct jpeg_destination_mgr*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(buf_destination_mgr));
  }

  auto* dest = reinterpret_cast<buf_destination_mgr*>(cinfo->dest);
  dest->pub.init_destination = init_buf_destination;
  dest->pub.empty_output_buffer = empty_buf_output_buffer;
  dest->pub.term_destination = term_buf_destination;
  dest->pub.next_output_byte = buf;
  dest->pub.free_in_buffer = size;

  dest->size = size;
  dest->out_size = out_size;
}

void JpegCompressImpl(struct jpeg_compress_struct* cinfo, unsigned char* rgb,
                      int quality) {
  jpeg_set_defaults(cinfo);
  jpeg_set_quality(cinfo, quality, TRUE);

  jpeg_start_compress(cinfo, TRUE);

  JSAMPROW row_pointer[1];
  const int row_stride = cinfo->image_width * 3;
  while (cinfo->next_scanline < cinfo->image_height) {
    auto* line = &rgb[cinfo->next_scanline * row_stride];
    for (JDIMENSION j = 0; j < cinfo->image_width; ++j)
      std::swap(line[3 * j], line[3 * j + 2]);
    row_pointer[0] = line;
    jpeg_write_scanlines(cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(cinfo);
  jpeg_destroy_compress(cinfo);
}
}  // namespace

unsigned long JpegCompressRgb(unsigned char* rgb, int width, int height,
                              int quality, unsigned char* buf,
                              unsigned long size) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  unsigned long out_size;
  jpeg_buf_dest(&cinfo, buf, size, &out_size);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  JpegCompressImpl(&cinfo, rgb, quality);
  return out_size;
}

JpegBuffer JpegCompressRgb(unsigned char* rgb, int width, int height,
                           int quality) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  JpegBuffer res{};
  jpeg_mem_dest(&cinfo, &res.data, &res.size);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  JpegCompressImpl(&cinfo, rgb, quality);
  return res;
}

void JpegCompressRgb(unsigned char* rgb, int width, int height, int quality,
                     std::vector<uint8_t>* out) {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  jpeg_vector_dest(&cinfo, out);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  JpegCompressImpl(&cinfo, rgb, quality);
}

}  // namespace coralmicro
