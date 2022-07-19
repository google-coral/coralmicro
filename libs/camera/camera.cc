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

#include "libs/camera/camera.h"

#include "libs/base/check.h"
#include "libs/base/gpio.h"
#include "libs/pmic/pmic.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#if (__CORTEX_M == 7)
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"
#elif (__CORTEX_M == 4)
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4/fsl_cache.h"
#endif

#include <memory>

namespace coralmicro {
using namespace camera;

namespace {
constexpr uint8_t kCameraAddress = 0x24;
constexpr int kFramebufferCount = 4;
constexpr float kRedCoefficient = .2126;
constexpr float kGreenCoefficient = .7152;
constexpr float kBlueCoefficient = .0722;
constexpr float kUint8Max = 255.0;

struct CameraRegisters {
  enum : uint16_t {
    MODEL_ID_H = 0x0000,
    MODEL_ID_L = 0x0001,
    MODE_SELECT = 0x0100,
    SW_RESET = 0x0103,
    ANALOG_GAIN = 0x0205,
    DIGITAL_GAIN_H = 0x020E,
    DIGITAL_GAIN_L = 0x020F,
    DGAIN_CONTROL = 0x0350,
    TEST_PATTERN_MODE = 0x0601,
    BLC_CFG = 0x1000,
    BLC_DITHER = 0x1001,
    BLC_DARKPIXEL = 0x1002,
    BLC_TGT = 0x1003,
    BLI_EN = 0x1006,
    BLC2_TGT = 0x1007,
    DPC_CTRL = 0x1008,
    CLUSTER_THR_HOT = 0x1009,
    CLUSTER_THR_COLD = 0x100A,
    SINGLE_THR_HOT = 0x100B,
    SINGLE_THR_COLD = 0x100C,
    VSYNC_HSYNC_PIXEL_SHIFT_EN = 0x1012,
    AE_CTRL = 0x2100,
    AE_TARGET_MEAN = 0x2101,
    AE_MIN_MEAN = 0x2102,
    CONVERGE_IN_TH = 0x2103,
    CONVERGE_OUT_TH = 0x2104,
    MAX_INTG_H = 0x2105,
    MAX_INTG_L = 0x2106,
    MIN_INTG = 0x2107,
    MAX_AGAIN_FULL = 0x2108,
    MAX_AGAIN_BIN2 = 0x2109,
    MIN_AGAIN = 0x210A,
    MAX_DGAIN = 0x210B,
    MIN_DGAIN = 0x210C,
    DAMPING_FACTOR = 0x210D,
    FS_CTRL = 0x210E,
    FS_60HZ_H = 0x210F,
    FS_60HZ_L = 0x2110,
    FS_50HZ_H = 0x2111,
    FS_50HZ_L = 0x2112,
    BIT_CONTROL = 0x3059,
    OSC_CLK_DIV = 0x3060,
  };
};

__attribute__((section(".sdram_bss,\"aw\",%nobits @")))
__attribute__((aligned(64))) uint8_t
    framebuffers[kFramebufferCount][CameraTask::kHeight][CameraTask::kWidth];

uint8_t* IndexToFramebufferPtr(int index) {
  if (index < 0 || index >= kFramebufferCount) {
    return nullptr;
  }
  return reinterpret_cast<uint8_t*>(framebuffers[index]);
}

int FramebufferPtrToIndex(const uint8_t* framebuffer_ptr) {
  for (int i = 0; i < kFramebufferCount; ++i) {
    if (reinterpret_cast<uint8_t*>(framebuffers[i]) == framebuffer_ptr) {
      return i;
    }
  }
  return -1;
}
}  // namespace

bool CameraTask::GetFrame(const std::list<camera::FrameFormat>& fmts) {
  bool ret = true;
  uint8_t* raw = nullptr;
  int index = GetSingleton()->GetFrame(&raw, true);
  if (!raw) {
    return false;
  }
  if (GetSingleton()->mode_ == Mode::kTrigger) {
    GpioSet(Gpio::kCameraTrigger, false);
  }

  for (const camera::FrameFormat& fmt : fmts) {
    switch (fmt.fmt) {
      case Format::kRgb: {
        if (fmt.width == kWidth && fmt.height == kHeight) {
          BayerToRGB(raw, fmt.buffer, fmt.width, fmt.height, fmt.filter,
                     fmt.rotation);
          if (fmt.white_balance &&
              GetSingleton()->test_pattern_ == TestPattern::kNone) {
            AutoWhiteBalance(fmt.buffer, fmt.width, fmt.height);
          }
        } else {
          auto buffer_rgb = std::make_unique<uint8_t[]>(
              FormatToBPP(Format::kRgb) * kWidth * kHeight);
          BayerToRGB(raw, buffer_rgb.get(), kWidth, kHeight, fmt.filter,
                     fmt.rotation);
          if (fmt.white_balance &&
              GetSingleton()->test_pattern_ == TestPattern::kNone) {
            AutoWhiteBalance(buffer_rgb.get(), kWidth, kHeight);
          }
          ResizeNearestNeighbor(buffer_rgb.get(), kWidth, kHeight, fmt.buffer,
                                fmt.width, fmt.height,
                                FormatToBPP(Format::kRgb), fmt.preserve_ratio);
        }
        break;
        case Format::kY8: {
          if (fmt.width == kWidth && fmt.height == kHeight) {
            BayerToGrayscale(raw, fmt.buffer, kWidth, kHeight, fmt.filter,
                             fmt.rotation);
          } else {
            auto buffer_rgb = std::make_unique<uint8_t[]>(
                FormatToBPP(Format::kRgb) * kWidth * kHeight);
            auto buffer_rgb_scaled = std::make_unique<uint8_t[]>(
                FormatToBPP(Format::kRgb) * fmt.width * fmt.height);
            BayerToRGB(raw, buffer_rgb.get(), kWidth, kHeight, fmt.filter,
                       fmt.rotation);
            ResizeNearestNeighbor(buffer_rgb.get(), kWidth, kHeight,
                                  buffer_rgb_scaled.get(), fmt.width,
                                  fmt.height, FormatToBPP(Format::kRgb),
                                  fmt.preserve_ratio);
            RGBToGrayscale(buffer_rgb_scaled.get(), fmt.buffer, fmt.width,
                           fmt.height);
          }
        } break;
        case Format::kRaw:
          if (fmt.width != kWidth || fmt.height != kHeight) {
            ret = false;
            break;
          }
          memcpy(fmt.buffer, raw, kWidth * kHeight * FormatToBPP(Format::kRaw));
          ret = true;
          break;
        default:
          ret = false;
      }
    }
  }

  GetSingleton()->ReturnFrame(index);
  return ret;
}

void CameraTask::ResizeNearestNeighbor(const uint8_t* src, int src_w, int src_h,
                                       uint8_t* dst, int dst_w, int dst_h,
                                       int comps, bool preserve_aspect) {
  int src_p = src_w * comps;
  int dst_p = dst_w * comps;
  float ratio_src = (float)src_w / src_h;
  float ratio_dst = (float)dst_w / dst_h;
  int scaled_w =
      preserve_aspect
          ? (ratio_dst > ratio_src ? src_w * (float)dst_h / src_h : dst_w)
          : dst_w;
  int scaled_h =
      preserve_aspect
          ? (ratio_dst > ratio_src ? dst_h : src_h * (float)dst_w / src_w)
          : dst_h;
  float ratio_x = (float)src_w / scaled_w;
  float ratio_y = (float)src_h / scaled_h;

  for (int y = 0; y < dst_h; y++) {
    if (y >= scaled_h) {
      memset(dst, 0, dst_p);
      dst += dst_p;
      continue;
    }

    int offset_y = static_cast<int>(y * ratio_y) * src_p;
    for (int x = 0; x < dst_w; x++) {
      int offset_x = static_cast<int>(x * ratio_x) * comps;
      const uint8_t* src_y = src + offset_y;
      for (int i = 0; i < comps; i++) {
        *dst++ = x < scaled_w ? src_y[offset_x + i] : 0;
      }
    }
  }
}

namespace {
template <typename Callback>
void BayerInternal(const uint8_t* camera_raw, int width, int height,
                   FilterMethod filter, Callback callback) {
  if (filter == FilterMethod::kNearestNeighbor) {
    bool blue = true, green = false;
    for (int y = 2; y < height - 2; y++) {
      int start = green ? 3 : 2;
      for (int x = start; x < width - 2; x += 2) {
        int g1x = x + 1, g1y = y;
        int g2x = x + 2, g2y = y + 1;
        int r1x, r1y, r2x, r2y;
        int b1x, b1y, b2x, b2y;
        if (blue) {
          r1x = r2x = x + 1;
          r1y = r2y = y + 1;
          b1x = x;
          b1y = y;
          b2x = x + 2;
          b2y = y;
        } else {
          r1x = x;
          r1y = y;
          r2x = x + 2;
          r2y = y;
          b1x = b2x = x + 1;
          b1y = b2y = y + 1;
        }
        uint8_t r1 = camera_raw[r1x + (r1y * width)];
        uint8_t g1 = camera_raw[g1x + (g1y * width)];
        uint8_t b1 = camera_raw[b1x + (b1y * width)];
        uint8_t r2 = camera_raw[r2x + (r2y * width)];
        uint8_t g2 = camera_raw[g2x + (g2y * width)];
        uint8_t b2 = camera_raw[b2x + (b2y * width)];
        callback(x, y, r1, g1, b1);
        callback(x + 1, y, r2, g2, b2);
      }
      blue = !blue;
      green = !green;
    }
  } else if (filter == FilterMethod::kBilinear) {
    int bayer_stride = width;

    size_t bayer_offset = 0;
    for (int y = 2; y < height - 2; y++) {
      bool odd_row = y & 1;
      int x = 1;
      size_t bayer_end = bayer_offset + (width - 2);

      if (odd_row) {
        uint8_t r = (static_cast<uint32_t>(camera_raw[bayer_offset + 1]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2 + 1)]) +
                     1) >>
                    1;
        uint8_t b =
            (static_cast<uint32_t>(camera_raw[bayer_offset + bayer_stride]) +
             static_cast<uint32_t>(
                 camera_raw[bayer_offset + (bayer_stride + 2)]) +
             1) >>
            1;
        uint8_t g = camera_raw[bayer_offset + (bayer_stride + 1)];
        callback(x, y, r, g, b);
        bayer_offset += 1;
        ++x;
      }

      while (bayer_offset <= (bayer_end - 2)) {
        uint8_t r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0;
        uint8_t t0 = (static_cast<uint32_t>(camera_raw[bayer_offset]) +
                      static_cast<uint32_t>(camera_raw[bayer_offset + 2]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride * 2)]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride * 2 + 2)]) +
                      2) >>
                     2;
        g1 = (static_cast<uint32_t>(camera_raw[bayer_offset + 1]) +
              static_cast<uint32_t>(camera_raw[bayer_offset + bayer_stride]) +
              static_cast<uint32_t>(
                  camera_raw[bayer_offset + (bayer_stride + 2)]) +
              static_cast<uint32_t>(
                  camera_raw[bayer_offset + (bayer_stride * 2 + 1)]) +
              2) >>
             2;
        uint8_t t1 = (static_cast<uint32_t>(camera_raw[bayer_offset + 2]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride * 2 + 2)]) +
                      1) >>
                     1;
        uint8_t t2 = (static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride + 1)]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride + 3)]) +
                      1) >>
                     1;
        uint8_t t3 = camera_raw[bayer_offset + (bayer_stride + 1)];
        g2 = camera_raw[bayer_offset + (bayer_stride + 2)];
        if (odd_row) {
          r1 = t0;
          b1 = t3;

          r2 = t1;
          b2 = t2;
        } else {
          b1 = t0;
          r1 = t3;

          b2 = t1;
          r2 = t2;
        }
        callback(x, y, r1, g1, b1);
        callback(x + 1, y, r2, g2, b2);
        bayer_offset += 2;
        x += 2;
      }

      while (bayer_offset < bayer_end) {
        uint8_t t0 = (static_cast<uint32_t>(camera_raw[bayer_offset]) +
                      static_cast<uint32_t>(camera_raw[bayer_offset + 2]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride * 2)]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride * 2 + 2)]) +
                      2) >>
                     2;
        uint8_t g =
            (static_cast<uint32_t>(camera_raw[bayer_offset + 1]) +
             static_cast<uint32_t>(camera_raw[bayer_offset + bayer_stride]) +
             static_cast<uint32_t>(
                 camera_raw[bayer_offset + (bayer_stride + 2)]) +
             static_cast<uint32_t>(
                 camera_raw[bayer_offset + (bayer_stride * 2 + 1)]) +
             2) >>
            2;
        uint8_t t1 = camera_raw[bayer_offset + bayer_stride + 1];
        if (odd_row) {
          callback(x, y, t0, g, t1);
        } else {
          callback(x, y, t1, g, t0);
        }
        bayer_offset += 1;
        ++x;
      }

      bayer_offset += 2;
    }
  }
}

void RotateXY(Rotation rotation, int in_x, int in_y, int* out_x, int* out_y) {
  CHECK(out_x);
  CHECK(out_y);

  // Short-circuit for no rotation
  if (rotation == Rotation::k0) {
    *out_x = in_x;
    *out_y = in_y;
    return;
  }

  // Shift our coordinates so that the center of the image is 0,0
  in_x = in_x - (CameraTask::kWidth / 2);
  in_y = in_y - (CameraTask::kHeight / 2);

  // Simple rotation around origin
  switch (rotation) {
    case Rotation::k90:
      *out_x = -in_y;
      *out_y = in_x;
      break;
    case Rotation::k180:
      *out_x = -in_x;
      *out_y = -in_y;
      break;
    case Rotation::k270:
      *out_x = in_y;
      *out_y = -in_x;
      break;
    case Rotation::k0:
    default:
      CHECK(false);
  }

  // Undo coordinate space shift
  *out_x = *out_x + (CameraTask::kWidth / 2);
  *out_y = *out_y + (CameraTask::kHeight / 2);
  CHECK(*out_x >= 0);
  CHECK(*out_x < static_cast<int>(CameraTask::kWidth));
  CHECK(*out_y >= 0);
  CHECK(*out_y < static_cast<int>(CameraTask::kHeight));
}

}  // namespace

void CameraTask::BayerToRGB(const uint8_t* camera_raw, uint8_t* camera_rgb,
                            int width, int height, FilterMethod filter,
                            Rotation rotation) {
  memset(camera_rgb, 0, width * height * 3);
  BayerInternal(camera_raw, width, height, filter,
                [camera_rgb, width, height, rotation](int x, int y, uint8_t r,
                                                      uint8_t g, uint8_t b) {
                  int rot_x, rot_y;
                  RotateXY(rotation, x, y, &rot_x, &rot_y);
                  camera_rgb[(rot_x * 3) + (rot_y * width * 3) + 0] = r;
                  camera_rgb[(rot_x * 3) + (rot_y * width * 3) + 1] = g;
                  camera_rgb[(rot_x * 3) + (rot_y * width * 3) + 2] = b;
                });
}

void CameraTask::BayerToRGBA(const uint8_t* camera_raw, uint8_t* camera_rgba,
                             int width, int height, FilterMethod filter,
                             Rotation rotation) {
  memset(camera_rgba, 0, width * height * 4);
  BayerInternal(camera_raw, width, height, filter,
                [camera_rgba, width, height, rotation](int x, int y, uint8_t r,
                                                       uint8_t g, uint8_t b) {
                  int rot_x, rot_y;
                  RotateXY(rotation, x, y, &rot_x, &rot_y);
                  camera_rgba[(rot_x * 4) + (rot_y * width * 4) + 0] = r;
                  camera_rgba[(rot_x * 4) + (rot_y * width * 4) + 1] = g;
                  camera_rgba[(rot_x * 4) + (rot_y * width * 4) + 2] = b;
                });
}

void CameraTask::BayerToGrayscale(const uint8_t* camera_raw,
                                  uint8_t* camera_grayscale, int width,
                                  int height, FilterMethod filter,
                                  Rotation rotation) {
  BayerInternal(camera_raw, width, height, filter,
                [camera_grayscale, width, height, rotation](
                    int x, int y, uint8_t r, uint8_t g, uint8_t b) {
                  int rot_x, rot_y;
                  RotateXY(rotation, x, y, &rot_x, &rot_y);
                  float r_f = static_cast<float>(r) / kUint8Max;
                  float g_f = static_cast<float>(g) / kUint8Max;
                  float b_f = static_cast<float>(b) / kUint8Max;
                  camera_grayscale[rot_x + (rot_y * width)] =
                      static_cast<uint8_t>(((kRedCoefficient * r_f * r_f) +
                                            (kGreenCoefficient * g_f * g_f) +
                                            (kBlueCoefficient * b_f * b_f)) *
                                           kUint8Max);
                });
}

void CameraTask::RGBToGrayscale(const uint8_t* camera_rgb,
                                uint8_t* camera_grayscale, int width,
                                int height) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float r_f =
          static_cast<float>(camera_rgb[(x * 3) + (y * width * 3) + 0]) /
          kUint8Max;
      float g_f =
          static_cast<float>(camera_rgb[(x * 3) + (y * width * 3) + 1]) /
          kUint8Max;
      float b_f =
          static_cast<float>(camera_rgb[(x * 3) + (y * width * 3) + 2]) /
          kUint8Max;
      camera_grayscale[x + (y * width)] = static_cast<uint8_t>(
          ((kRedCoefficient * r_f * r_f) + (kGreenCoefficient * g_f * g_f) +
           (kBlueCoefficient * b_f * b_f)) *
          kUint8Max);
    }
  }
}

void CameraTask::AutoWhiteBalance(uint8_t* camera_rgb, int width, int height) {
  unsigned int r_sum = 0, g_sum = 0, b_sum = 0;
  float r_sum_f = 0.0, g_sum_f = 0.0, b_sum_f = 0.0;
  float threshold = 0.9f;
  uint16_t threshold16 = static_cast<uint16_t>(threshold * 255);
  uint16_t minRGB, maxRGB;
  for (int i = 0; i < width * height; ++i) {
    uint8_t r = camera_rgb[i * 3 + 0];
    uint8_t g = camera_rgb[i * 3 + 1];
    uint8_t b = camera_rgb[i * 3 + 2];
    minRGB = static_cast<uint16_t>(std::min(r, std::min(g, b)));
    maxRGB = static_cast<uint16_t>(std::max(r, std::max(g, b)));
    if (((maxRGB - minRGB) * 255) > (threshold16 * maxRGB)) {
      continue;
    }
    r_sum += r;
    g_sum += g;
    b_sum += b;
  }
  r_sum_f = static_cast<float>(r_sum);
  g_sum_f = static_cast<float>(g_sum);
  b_sum_f = static_cast<float>(b_sum);
  float max_channel = std::max(r_sum_f, std::max(g_sum_f, b_sum_f));
  float epsilon = 0.1;
  float r_gain_f = r_sum_f < epsilon ? 0.0f : max_channel / r_sum_f;
  float g_gain_f = g_sum_f < epsilon ? 0.0f : max_channel / g_sum_f;
  float b_gain_f = b_sum_f < epsilon ? 0.0f : max_channel / b_sum_f;
  uint16_t r_gain_i = static_cast<uint16_t>(r_gain_f * (1 << 8));
  uint16_t g_gain_i = static_cast<uint16_t>(g_gain_f * (1 << 8));
  uint16_t b_gain_i = static_cast<uint16_t>(b_gain_f * (1 << 8));
  for (int i = 0; i < width * height; ++i) {
    uint8_t r = camera_rgb[i * 3 + 0];
    uint8_t g = camera_rgb[i * 3 + 1];
    uint8_t b = camera_rgb[i * 3 + 2];
    camera_rgb[i * 3 + 0] = static_cast<uint8_t>(
        std::min(255UL, (static_cast<uint32_t>(r) * r_gain_i) >> 8));
    camera_rgb[i * 3 + 1] = static_cast<uint8_t>(
        std::min(255UL, (static_cast<uint32_t>(g) * g_gain_i) >> 8));
    camera_rgb[i * 3 + 2] = static_cast<uint8_t>(
        std::min(255UL, (static_cast<uint32_t>(b) * b_gain_i) >> 8));
  }
}

int CameraTask::FormatToBPP(Format fmt) {
  switch (fmt) {
    case Format::kRgba:
      return 4;
    case Format::kRgb:
      return 3;
    case Format::kRaw:
    case Format::kY8:
      return 1;
  }
  return 0;
}

extern "C" void CSI_DriverIRQHandler(void);
extern "C" void CSI_IRQHandler(void) {
  CSI_DriverIRQHandler();
  __DSB();
}

bool CameraTask::Read(uint16_t reg, uint8_t* val) {
  lpi2c_master_transfer_t transfer;
  transfer.flags = kLPI2C_TransferDefaultFlag;
  transfer.slaveAddress = kCameraAddress;
  transfer.direction = kLPI2C_Read;
  transfer.subaddress = static_cast<uint16_t>(reg);
  transfer.subaddressSize = sizeof(reg);
  transfer.data = val;
  transfer.dataSize = sizeof(*val);
  status_t status = LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
  return status == kStatus_Success;
}

bool CameraTask::Write(uint16_t reg, uint8_t val) {
  lpi2c_master_transfer_t transfer;
  transfer.flags = kLPI2C_TransferDefaultFlag;
  transfer.slaveAddress = kCameraAddress;
  transfer.direction = kLPI2C_Write;
  transfer.subaddress = static_cast<uint16_t>(reg);
  transfer.subaddressSize = sizeof(reg);
  transfer.data = &val;
  transfer.dataSize = sizeof(val);
  status_t status = LPI2C_RTOS_Transfer(i2c_handle_, &transfer);
  return status == kStatus_Success;
}

void CameraTask::Init(lpi2c_rtos_handle_t* i2c_handle) {
  QueueTask::Init();
  i2c_handle_ = i2c_handle;
  mode_ = Mode::kStandBy;
}

int CameraTask::GetFrame(uint8_t** buffer, bool block) {
  Request req;
  req.type = RequestType::kFrame;
  req.request.frame.index = -1;
  Response resp;
  do {
    resp = SendRequest(req);
  } while (block && resp.response.frame.index == -1);
  *buffer = IndexToFramebufferPtr(resp.response.frame.index);
  return resp.response.frame.index;
}

void CameraTask::ReturnFrame(int index) {
  Request req;
  req.type = RequestType::kFrame;
  req.request.frame.index = index;
  SendRequest(req);
}

void CameraTask::Enable(Mode mode) {
  Request req;
  req.type = RequestType::kEnable;
  req.request.mode = mode;
  SendRequest(req);
}

void CameraTask::Disable() {
  Request req;
  req.type = RequestType::kDisable;
  SendRequest(req);
}

bool CameraTask::SetPower(bool enable) {
  Request req;
  req.type = RequestType::kPower;
  req.request.power.enable = enable;
  Response resp = SendRequest(req);
  return resp.response.power.success;
}

void CameraTask::SetTestPattern(TestPattern pattern) {
  Request req;
  req.type = RequestType::kTestPattern;
  req.request.test_pattern.pattern = pattern;
  SendRequest(req);
}

void CameraTask::Trigger() { GpioSet(Gpio::kCameraTrigger, true); }

void CameraTask::DiscardFrames(int count) {
  Request req;
  req.type = RequestType::kDiscard;
  req.request.discard.count = count;
  SendRequest(req);
}

void CameraTask::TaskInit() {
  status_t status;
  csi_config_.width = kCsiWidth;
  csi_config_.height = kCsiHeight;
  csi_config_.polarityFlags =
      kCSI_HsyncActiveHigh | kCSI_VsyncActiveHigh | kCSI_DataLatchOnRisingEdge;
  csi_config_.bytesPerPixel = 1;
  csi_config_.linePitch_Bytes = kCsiWidth * 1;
  csi_config_.workMode = kCSI_GatedClockMode;
  csi_config_.dataBus = kCSI_DataBus8Bit;
  csi_config_.useExtVsync = true;
  status = CSI_Init(CSI, &csi_config_);
  if (status != kStatus_Success) {
    return;
  }

  PowerRequest req;
  req.enable = false;
  HandlePowerRequest(req);
}

void CameraTask::SetDefaultRegisters() {
  // Taken from Tensorflow's configuration in the person detection sample
  /* Analog settings */
  Write(CameraRegisters::BLC_TGT, 0x08);
  Write(CameraRegisters::BLC2_TGT, 0x08);
  /* These registers are RESERVED in the datasheet,
   * but without them the picture is bad. */
  Write(0x3044, 0x0A);
  Write(0x3045, 0x00);
  Write(0x3047, 0x0A);
  Write(0x3050, 0xC0);
  Write(0x3051, 0x42);
  Write(0x3052, 0x50);
  Write(0x3053, 0x00);
  Write(0x3054, 0x03);
  Write(0x3055, 0xF7);
  Write(0x3056, 0xF8);
  Write(0x3057, 0x29);
  Write(0x3058, 0x1F);
  Write(CameraRegisters::BIT_CONTROL, 0x1E);
  /* Digital settings */
  Write(CameraRegisters::BLC_CFG, 0x43);
  Write(CameraRegisters::BLC_DITHER, 0x40);
  Write(CameraRegisters::BLC_DARKPIXEL, 0x32);
  Write(CameraRegisters::DGAIN_CONTROL, 0x7F);
  Write(CameraRegisters::BLI_EN, 0x01);
  Write(CameraRegisters::DPC_CTRL, 0x00);
  Write(CameraRegisters::CLUSTER_THR_HOT, 0xA0);
  Write(CameraRegisters::CLUSTER_THR_COLD, 0x60);
  Write(CameraRegisters::SINGLE_THR_HOT, 0x90);
  Write(CameraRegisters::SINGLE_THR_COLD, 0x40);
  /* AE settings */
  Write(CameraRegisters::AE_CTRL, 0x01);
  Write(CameraRegisters::AE_TARGET_MEAN, 0x5F);
  Write(CameraRegisters::AE_MIN_MEAN, 0x0A);
  Write(CameraRegisters::CONVERGE_IN_TH, 0x03);
  Write(CameraRegisters::CONVERGE_OUT_TH, 0x05);
  Write(CameraRegisters::MAX_INTG_H, 0x02);
  Write(CameraRegisters::MAX_INTG_L, 0x14);
  Write(CameraRegisters::MIN_INTG, 0x02);
  Write(CameraRegisters::MAX_AGAIN_FULL, 0x03);
  Write(CameraRegisters::MAX_AGAIN_BIN2, 0x03);
  Write(CameraRegisters::MIN_AGAIN, 0x00);
  Write(CameraRegisters::MAX_DGAIN, 0x80);
  Write(CameraRegisters::MIN_DGAIN, 0x40);
  Write(CameraRegisters::DAMPING_FACTOR, 0x20);
  /* 60Hz flicker */
  Write(CameraRegisters::FS_CTRL, 0x03);
  Write(CameraRegisters::FS_60HZ_H, 0x00);
  Write(CameraRegisters::FS_60HZ_L, 0x85);
  Write(CameraRegisters::FS_50HZ_H, 0x00);
  Write(CameraRegisters::FS_50HZ_L, 0xA0);
}

EnableResponse CameraTask::HandleEnableRequest(const Mode& mode) {
  EnableResponse resp;
  status_t status;

  // Gated clock mode
  uint8_t osc_clk_div;
  Read(CameraRegisters::OSC_CLK_DIV, &osc_clk_div);
  osc_clk_div |= 1 << 5;
  Write(CameraRegisters::OSC_CLK_DIV, osc_clk_div);

  SetDefaultRegisters();

  // Shifting
  Write(CameraRegisters::VSYNC_HSYNC_PIXEL_SHIFT_EN, 0x0);

  status = CSI_TransferCreateHandle(CSI, &csi_handle_, nullptr, 0);

  int framebuffer_count = kFramebufferCount;
  if (mode == Mode::kTrigger) {
    framebuffer_count = 2;
  }
  for (int i = 0; i < framebuffer_count; i++) {
    status = CSI_TransferSubmitEmptyBuffer(
        CSI, &csi_handle_, reinterpret_cast<uint32_t>(framebuffers[i]));
  }

  // Streaming
  status = CSI_TransferStart(CSI, &csi_handle_);
  SetMode(mode);
  resp.success = (status == kStatus_Success);
  return resp;
}

void CameraTask::HandleDisableRequest() {
  SetMode(Mode::kStandBy);
  CSI_TransferStop(CSI, &csi_handle_);
}

PowerResponse CameraTask::HandlePowerRequest(const PowerRequest& power) {
  PowerResponse resp;
  resp.success = true;
  PmicTask::GetSingleton()->SetRailState(pmic::Rail::kCam2V8, power.enable);
  PmicTask::GetSingleton()->SetRailState(pmic::Rail::kCam1V8, power.enable);
  vTaskDelay(pdMS_TO_TICKS(10));

  if (power.enable) {
    uint8_t model_id_h = 0xff, model_id_l = 0xff;
    for (int i = 0; i < 10; ++i) {
      Read(CameraRegisters::MODEL_ID_H, &model_id_h);
      Read(CameraRegisters::MODEL_ID_L, &model_id_l);
      Write(CameraRegisters::SW_RESET, 0x00);
      if (model_id_h == kModelIdHExpected && model_id_l == kModelIdLExpected) {
        break;
      }
    }

    if (model_id_h != kModelIdHExpected || model_id_l != kModelIdLExpected) {
      printf("Camera model id not as expected: 0x%02x%02x\r\n", model_id_h,
             model_id_l);
      resp.success = false;
    }
  }
  return resp;
}

FrameResponse CameraTask::HandleFrameRequest(const FrameRequest& frame) {
  FrameResponse resp;
  resp.index = -1;
  uint32_t buffer;
  if (frame.index == -1) {  // GET
    status_t status = CSI_TransferGetFullBuffer(CSI, &csi_handle_, &buffer);
    if (status == kStatus_Success) {
      DCACHE_InvalidateByRange(buffer, kHeight * kWidth);
      resp.index = FramebufferPtrToIndex(reinterpret_cast<uint8_t*>(buffer));
    }
  } else {  // RETURN
    buffer = reinterpret_cast<uint32_t>(IndexToFramebufferPtr(frame.index));
    if (buffer) {
      CSI_TransferSubmitEmptyBuffer(CSI, &csi_handle_, buffer);
    }
  }
  return resp;
}

void CameraTask::HandleTestPatternRequest(
    const TestPatternRequest& test_pattern) {
  if (test_pattern.pattern == TestPattern::kNone) {
    SetDefaultRegisters();
  } else {
    Write(CameraRegisters::AE_CTRL, 0x00);
    Write(CameraRegisters::BLC_CFG, 0x00);
    Write(CameraRegisters::DPC_CTRL, 0x00);
    Write(CameraRegisters::ANALOG_GAIN, 0x00);
    Write(CameraRegisters::DIGITAL_GAIN_H, 0x01);
    Write(CameraRegisters::DIGITAL_GAIN_L, 0x00);
  }
  Write(CameraRegisters::TEST_PATTERN_MODE,
        static_cast<uint8_t>(test_pattern.pattern));
  test_pattern_ = test_pattern.pattern;
}

void CameraTask::HandleDiscardRequest(const DiscardRequest& discard) {
  int discarded = 0;
  while (discarded < discard.count) {
    FrameRequest request;
    request.index = -1;
    FrameResponse resp = HandleFrameRequest(request);
    if (resp.index != -1) {
      // Return the frame, and increment the discard counter.
      discarded++;
      request.index = resp.index;
      HandleFrameRequest(request);
    }
  }
}

void CameraTask::SetMode(const Mode& mode) {
  Write(CameraRegisters::MODE_SELECT, static_cast<uint8_t>(mode));
  mode_ = mode;
}

void CameraTask::RequestHandler(Request* req) {
  Response resp;
  resp.type = req->type;
  switch (req->type) {
    case RequestType::kEnable:
      resp.response.enable = HandleEnableRequest(req->request.mode);
      break;
    case RequestType::kDisable:
      HandleDisableRequest();
      break;
    case RequestType::kPower:
      resp.response.power = HandlePowerRequest(req->request.power);
      break;
    case RequestType::kFrame:
      resp.response.frame = HandleFrameRequest(req->request.frame);
      break;
    case RequestType::kTestPattern:
      HandleTestPatternRequest(req->request.test_pattern);
      break;
    case RequestType::kDiscard:
      HandleDiscardRequest(req->request.discard);
      break;
  }
  if (req->callback) req->callback(resp);
}

}  // namespace coralmicro
