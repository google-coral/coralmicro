#include "apps/OOBE/jpeg.h"

#include <algorithm>

#include "third_party/nxp/rt1176-sdk/middleware/libjpeg/inc/jpeglib.h"

namespace coral::micro {
namespace {
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
    for (int j = 0; j < cinfo->image_width; ++j)
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

}  // namespace coral::micro
