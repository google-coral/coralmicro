#ifndef APPS_OOBEE_JPEG_H_
#define APPS_OOBEE_JPEG_H_

namespace coral::micro {

unsigned long JpegCompressRgb(unsigned char* rgb, int width, int height,
                              int quality, unsigned char* buf,
                              unsigned long size);

struct JpegBuffer {
  unsigned char* data;
  unsigned long size;
};
JpegBuffer JpegCompressRgb(unsigned char* rgb, int width, int height,
                           int quality);

};  // namespace coral::micro

#endif  // APPS_OOBEE_JPEG_H_
