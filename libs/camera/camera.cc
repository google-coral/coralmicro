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

#include <cstring>
#include <memory>

namespace coralmicro {
namespace {
constexpr uint8_t kCameraAddress = 0x24;
constexpr int kFramebufferCount = 4;
constexpr float kRedCoefficient = .2126;
constexpr float kGreenCoefficient = .7152;
constexpr float kBlueCoefficient = .0722;
constexpr float kUint8Max = 255.0;

constexpr uint8_t kModelIdHExpected = 0x01;
constexpr uint8_t kModelIdLExpected = 0xB0;

// CSI driver wants width to be divisible by 8, and 324 is not.
// 324 * 324 == 13122 * 8 -- this makes the CSI driver happy!
constexpr size_t kCsiWidth = 8;
constexpr size_t kCsiHeight = 13122;

struct CameraRegisters {
  enum : uint16_t {
    kModelIdH = 0x0000,
    kModelIdL = 0x0001,
    kModeSelect = 0x0100,
    kSwReset = 0x0103,
    kAnalogGain = 0x0205,
    kDigitalGainH = 0x020E,
    kDigitalGainL = 0x020F,
    kDgainControl = 0x0350,
    kTestPatternMode = 0x0601,
    kBlcCfg = 0x1000,
    kBlcDither = 0x1001,
    kBlcDarkpixel = 0x1002,
    kBlcTgt = 0x1003,
    kBliEn = 0x1006,
    kBlc2Tgt = 0x1007,
    kDpcCtrl = 0x1008,
    kClusterThrHot = 0x1009,
    kClusterThrCold = 0x100A,
    kSingleThrHot = 0x100B,
    kSingleThrCold = 0x100C,
    kVsyncHsyncPixelShiftEn = 0x1012,
    kStatisticCtrl = 0x2000,
    kMdLroiXStartH = 0x2011,
    kMdLroiXStartL = 0x2012,
    kMdLroiYStartH = 0x2013,
    kMdLroiYStartL = 0x2014,
    kMdLroiXEndH = 0x2015,
    kMdLroiXEndL = 0x2016,
    kMdLroiYEndH = 0x2017,
    kMdLroiYEndL = 0x2018,
    kAeCtrl = 0x2100,
    kAeTargetMean = 0x2101,
    kAeMinMean = 0x2102,
    kConvergeInTh = 0x2103,
    kConvergeOutTh = 0x2104,
    kMaxIntgH = 0x2105,
    kMaxIntgL = 0x2106,
    kMinIntg = 0x2107,
    kMaxAgainFull = 0x2108,
    kMaxAgainBin2 = 0x2109,
    kMinAgain = 0x210A,
    kMaxDgain = 0x210B,
    kMinDgain = 0x210C,
    kDampingFactor = 0x210D,
    kFsCtrl = 0x210E,
    kFs60HzH = 0x210F,
    kFs60HzL = 0x2110,
    kFs50HzH = 0x2111,
    kFs50HzL = 0x2112,
    kMdCtrl = 0x2150,
    kMdThl = 0x215B,
    kI2cClear = 0x2153,
    kBitControl = 0x3059,
    kOscClkDiv = 0x3060,
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

void ResizeNearestNeighbor(const uint8_t* src, int src_w, int src_h,
                           uint8_t* dst, int dst_w, int dst_h, int comps,
                           bool preserve_aspect) {
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
      std::memset(dst, 0, dst_p);
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

template <typename Callback>
void BayerInternal(const uint8_t* camera_raw, int width, int height,
                   CameraFilterMethod filter, Callback callback) {
  if (filter == CameraFilterMethod::kNearestNeighbor) {
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
  } else if (filter == CameraFilterMethod::kBilinear) {
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

void RotateXY(CameraRotation rotation, int in_x, int in_y, int* out_x,
              int* out_y) {
  CHECK(out_x);
  CHECK(out_y);

  // Short-circuit for no rotation
  if (rotation == CameraRotation::k0) {
    *out_x = in_x;
    *out_y = in_y;
    return;
  }

  // Shift our coordinates so that the center of the image is 0,0
  in_x = in_x - (CameraTask::kWidth / 2);
  in_y = in_y - (CameraTask::kHeight / 2);

  // Simple rotation around origin
  switch (rotation) {
    case CameraRotation::k90:
      *out_x = -in_y;
      *out_y = in_x;
      break;
    case CameraRotation::k180:
      *out_x = -in_x;
      *out_y = -in_y;
      break;
    case CameraRotation::k270:
      *out_x = in_y;
      *out_y = -in_x;
      break;
    case CameraRotation::k0:
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

void BayerToRgb(const uint8_t* camera_raw, uint8_t* camera_rgb, int width,
                int height, CameraFilterMethod filter,
                CameraRotation rotation) {
  std::memset(camera_rgb, 0, width * height * 3);
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

void BayerToGrayscale(const uint8_t* camera_raw, uint8_t* camera_grayscale,
                      int width, int height, CameraFilterMethod filter,
                      CameraRotation rotation) {
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

void RgbToGrayscale(const uint8_t* camera_rgb, uint8_t* camera_grayscale,
                    int width, int height) {
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

void AutoWhiteBalance(uint8_t* camera_rgb, int width, int height) {
  unsigned int r_sum = 0, g_sum = 0, b_sum = 0;
  float r_sum_f = 0.0, g_sum_f = 0.0, b_sum_f = 0.0;
  float threshold = 0.9f;
  uint16_t threshold16 = static_cast<uint16_t>(threshold * 255);
  uint16_t min_rgb, max_rgb;
  for (int i = 0; i < width * height; ++i) {
    uint8_t r = camera_rgb[i * 3 + 0];
    uint8_t g = camera_rgb[i * 3 + 1];
    uint8_t b = camera_rgb[i * 3 + 2];
    min_rgb = static_cast<uint16_t>(std::min(r, std::min(g, b)));
    max_rgb = static_cast<uint16_t>(std::max(r, std::max(g, b)));
    if (((max_rgb - min_rgb) * 255) > (threshold16 * max_rgb)) {
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
}  // namespace

extern "C" void CSI_DriverIRQHandler(void);
extern "C" void CSI_IRQHandler(void) {
  CSI_DriverIRQHandler();
  __DSB();
}

int CameraFormatBpp(CameraFormat fmt) {
  switch (fmt) {
    case CameraFormat::kRgb:
      return 3;
    case CameraFormat::kRaw:
    case CameraFormat::kY8:
      return 1;
  }
  return 0;
}

bool CameraTask::GetFrame(const std::vector<CameraFrameFormat>& fmts) {
  if (!enabled_) {
    printf("Camera is not enabled, cannot capture frame.\r\n");
    return false;
  }
  if (mode_ == CameraMode::kTrigger && !GpioGet(Gpio::kCameraTrigger)) {
    printf("Camera is in trigger mode but was never triggered\r\n");
    return false;
  }

  bool ret = true;
  uint8_t* raw = nullptr;
  int index = GetFrame(&raw, true);
  if (!raw) {
    return false;
  }
  if (mode_ == CameraMode::kTrigger) {
    GpioSet(Gpio::kCameraTrigger, false);
  }

  for (const CameraFrameFormat& fmt : fmts) {
    switch (fmt.fmt) {
      case CameraFormat::kRgb: {
        if (fmt.width == kWidth && fmt.height == kHeight) {
          BayerToRgb(raw, fmt.buffer, fmt.width, fmt.height, fmt.filter,
                     fmt.rotation);
          if (fmt.white_balance &&
              GetSingleton()->test_pattern_ == CameraTestPattern::kNone) {
            AutoWhiteBalance(fmt.buffer, fmt.width, fmt.height);
          }
        } else {
          auto buffer_rgb = std::make_unique<uint8_t[]>(
              CameraFormatBpp(CameraFormat::kRgb) * kWidth * kHeight);
          BayerToRgb(raw, buffer_rgb.get(), kWidth, kHeight, fmt.filter,
                     fmt.rotation);
          if (fmt.white_balance &&
              GetSingleton()->test_pattern_ == CameraTestPattern::kNone) {
            AutoWhiteBalance(buffer_rgb.get(), kWidth, kHeight);
          }
          ResizeNearestNeighbor(buffer_rgb.get(), kWidth, kHeight, fmt.buffer,
                                fmt.width, fmt.height,
                                CameraFormatBpp(CameraFormat::kRgb),
                                fmt.preserve_ratio);
        }
        break;
        case CameraFormat::kY8: {
          if (fmt.width == kWidth && fmt.height == kHeight) {
            BayerToGrayscale(raw, fmt.buffer, kWidth, kHeight, fmt.filter,
                             fmt.rotation);
          } else {
            auto buffer_rgb = std::make_unique<uint8_t[]>(
                CameraFormatBpp(CameraFormat::kRgb) * kWidth * kHeight);
            auto buffer_rgb_scaled = std::make_unique<uint8_t[]>(
                CameraFormatBpp(CameraFormat::kRgb) * fmt.width * fmt.height);
            BayerToRgb(raw, buffer_rgb.get(), kWidth, kHeight, fmt.filter,
                       fmt.rotation);
            ResizeNearestNeighbor(
                buffer_rgb.get(), kWidth, kHeight, buffer_rgb_scaled.get(),
                fmt.width, fmt.height, CameraFormatBpp(CameraFormat::kRgb),
                fmt.preserve_ratio);
            RgbToGrayscale(buffer_rgb_scaled.get(), fmt.buffer, fmt.width,
                           fmt.height);
          }
        } break;
        case CameraFormat::kRaw:
          if (fmt.width != kWidth || fmt.height != kHeight) {
            ret = false;
            break;
          }
          std::memcpy(fmt.buffer, raw,
                      kWidth * kHeight * CameraFormatBpp(CameraFormat::kRaw));
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
  enabled_ = false;
  GetMotionDetectionConfigDefault(md_config_);
  md_config_.enable = false;
  GpioConfigureInterrupt(
      Gpio::kCameraInt, GpioInterruptMode::kIntModeRising, [this]() {
        camera::Request req;
        req.type = camera::RequestType::kMotionDetectionInterrupt;
        this->SendRequestAsync(req);
      });
}

int CameraTask::GetFrame(uint8_t** buffer, bool block) {
  camera::Request req;
  req.type = camera::RequestType::kFrame;
  req.request.frame.index = -1;
  camera::Response resp;
  do {
    resp = SendRequest(req);
  } while (block && resp.response.frame.index == -1);
  *buffer = IndexToFramebufferPtr(resp.response.frame.index);
  return resp.response.frame.index;
}

void CameraTask::ReturnFrame(int index) {
  camera::Request req;
  req.type = camera::RequestType::kFrame;
  req.request.frame.index = index;
  SendRequest(req);
}

bool CameraTask::Enable(CameraMode mode) {
  camera::Request req;
  req.type = camera::RequestType::kEnable;
  req.request.mode = mode;
  auto resp = SendRequest(req);
  enabled_ = resp.response.enable.success;
  return enabled_;
}

void CameraTask::Disable() {
  camera::Request req;
  req.type = camera::RequestType::kDisable;
  SendRequest(req);
}

bool CameraTask::SetPower(bool enable) {
  camera::Request req;
  req.type = camera::RequestType::kPower;
  req.request.power.enable = enable;
  camera::Response resp = SendRequest(req);
  return resp.response.power.success;
}

void CameraTask::SetTestPattern(CameraTestPattern pattern) {
  camera::Request req;
  req.type = camera::RequestType::kTestPattern;
  req.request.test_pattern.pattern = pattern;
  SendRequest(req);
}

void CameraTask::Trigger() { GpioSet(Gpio::kCameraTrigger, true); }

void CameraTask::DiscardFrames(int count) {
  camera::Request req;
  req.type = camera::RequestType::kDiscard;
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

  camera::PowerRequest req;
  req.enable = false;
  HandlePowerRequest(req);
}

void CameraTask::SetMotionDetectionRegisters() {
  if (md_config_.enable) {
    Write(CameraRegisters::kMdCtrl, 3);
    Write(CameraRegisters::kMdThl, 1);
    Write(CameraRegisters::kMdLroiXStartH, md_config_.x0 >> 8);
    Write(CameraRegisters::kMdLroiXStartL, md_config_.x0 & 0xFF);
    Write(CameraRegisters::kMdLroiYStartH, md_config_.y0 >> 8);
    Write(CameraRegisters::kMdLroiYStartL, md_config_.y0 & 0xFF);
    Write(CameraRegisters::kMdLroiXEndH, md_config_.x1 >> 8);
    Write(CameraRegisters::kMdLroiXEndL, md_config_.x1 & 0xFF);
    Write(CameraRegisters::kMdLroiYEndH, md_config_.y1 >> 8);
    Write(CameraRegisters::kMdLroiYEndL, md_config_.y1 & 0xFF);
    Write(CameraRegisters::kI2cClear, 1);
  } else {
    Write(CameraRegisters::kMdCtrl, 0);
  }
}

void CameraTask::SetDefaultRegisters() {
  // Taken from Tensorflow's configuration in the person detection sample
  /* Analog settings */
  Write(CameraRegisters::kBlcTgt, 0x08);
  Write(CameraRegisters::kBlc2Tgt, 0x08);
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
  Write(CameraRegisters::kBitControl, 0x1E);
  /* Digital settings */
  Write(CameraRegisters::kBlcCfg, 0x43);
  Write(CameraRegisters::kBlcDither, 0x40);
  Write(CameraRegisters::kBlcDarkpixel, 0x32);
  Write(CameraRegisters::kDgainControl, 0x7F);
  Write(CameraRegisters::kBliEn, 0x01);
  Write(CameraRegisters::kDpcCtrl, 0x00);
  Write(CameraRegisters::kClusterThrHot, 0xA0);
  Write(CameraRegisters::kClusterThrCold, 0x60);
  Write(CameraRegisters::kSingleThrHot, 0x90);
  Write(CameraRegisters::kSingleThrCold, 0x40);
  /* AE settings */
  Write(CameraRegisters::kStatisticCtrl, 0x07);
  Write(CameraRegisters::kAeCtrl, 0x01);
  Write(CameraRegisters::kAeTargetMean, 0x5F);
  Write(CameraRegisters::kAeMinMean, 0x0A);
  Write(CameraRegisters::kConvergeInTh, 0x03);
  Write(CameraRegisters::kConvergeOutTh, 0x05);
  Write(CameraRegisters::kMaxIntgH, 0x02);
  Write(CameraRegisters::kMaxIntgL, 0x14);
  Write(CameraRegisters::kMinIntg, 0x02);
  Write(CameraRegisters::kMaxAgainFull, 0x03);
  Write(CameraRegisters::kMaxAgainBin2, 0x03);
  Write(CameraRegisters::kMinAgain, 0x00);
  Write(CameraRegisters::kMaxDgain, 0x80);
  Write(CameraRegisters::kMinDgain, 0x40);
  Write(CameraRegisters::kDampingFactor, 0x20);
  /* 60Hz flicker */
  Write(CameraRegisters::kFsCtrl, 0x03);
  Write(CameraRegisters::kFs60HzH, 0x00);
  Write(CameraRegisters::kFs60HzL, 0x85);
  Write(CameraRegisters::kFs50HzH, 0x00);
  Write(CameraRegisters::kFs50HzL, 0xA0);

  SetMotionDetectionRegisters();
}

camera::EnableResponse CameraTask::HandleEnableRequest(const CameraMode& mode) {
  camera::EnableResponse resp;
  status_t status;

  // Gated clock mode
  uint8_t osc_clk_div;
  Read(CameraRegisters::kOscClkDiv, &osc_clk_div);
  osc_clk_div |= 1 << 5;
  Write(CameraRegisters::kOscClkDiv, osc_clk_div);

  SetDefaultRegisters();

  // Shifting
  Write(CameraRegisters::kVsyncHsyncPixelShiftEn, 0x0);

  status = CSI_TransferCreateHandle(CSI, &csi_handle_, nullptr, 0);

  int framebuffer_count = kFramebufferCount;
  if (mode == CameraMode::kTrigger) {
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
  enabled_ = false;
  Write(CameraRegisters::kModeSelect, 0);
  CSI_TransferStop(CSI, &csi_handle_);
}

camera::PowerResponse CameraTask::HandlePowerRequest(
    const camera::PowerRequest& power) {
  camera::PowerResponse resp;
  resp.success = true;
  PmicTask::GetSingleton()->SetRailState(PmicRail::kCam2V8, power.enable);
  PmicTask::GetSingleton()->SetRailState(PmicRail::kCam1V8, power.enable);
  vTaskDelay(pdMS_TO_TICKS(10));

  if (power.enable) {
    uint8_t model_id_h = 0xff, model_id_l = 0xff;
    for (int i = 0; i < 10; ++i) {
      Read(CameraRegisters::kModelIdH, &model_id_h);
      Read(CameraRegisters::kModelIdL, &model_id_l);
      Write(CameraRegisters::kSwReset, 0x00);
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

camera::FrameResponse CameraTask::HandleFrameRequest(
    const camera::FrameRequest& frame) {
  camera::FrameResponse resp;
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
    const camera::TestPatternRequest& test_pattern) {
  if (test_pattern.pattern == CameraTestPattern::kNone) {
    SetDefaultRegisters();
  } else {
    Write(CameraRegisters::kAeCtrl, 0x00);
    Write(CameraRegisters::kBlcCfg, 0x00);
    Write(CameraRegisters::kDpcCtrl, 0x00);
    Write(CameraRegisters::kAnalogGain, 0x00);
    Write(CameraRegisters::kDigitalGainH, 0x01);
    Write(CameraRegisters::kDigitalGainL, 0x00);
  }
  Write(CameraRegisters::kTestPatternMode,
        static_cast<uint8_t>(test_pattern.pattern));
  test_pattern_ = test_pattern.pattern;
}

void CameraTask::HandleDiscardRequest(const camera::DiscardRequest& discard) {
  int discarded = 0;
  while (discarded < discard.count) {
    camera::FrameRequest request;
    request.index = -1;
    camera::FrameResponse resp = HandleFrameRequest(request);
    if (resp.index != -1) {
      // Return the frame, and increment the discard counter.
      discarded++;
      request.index = resp.index;
      HandleFrameRequest(request);
    }
  }
}

void CameraTask::GetMotionDetectionConfigDefault(
    CameraMotionDetectionConfig& config) {
  config.cb = nullptr;
  config.cb_param = nullptr;
  config.enable = true;
  config.x0 = 0;
  config.y0 = 0;
  config.x1 = kWidth - 1;
  config.y1 = kHeight - 1;
}

void CameraTask::SetMotionDetectionConfig(
    const CameraMotionDetectionConfig& config) {
  camera::Request req;
  req.type = camera::RequestType::kMotionDetectionConfig;
  req.request.motion_detection_config = config;
  SendRequest(req);
}

void CameraTask::HandleMotionDetectionConfig(
    const CameraMotionDetectionConfig& config) {
  md_config_ = config;
  SetMotionDetectionRegisters();
}

void CameraTask::HandleMotionDetectionInterrupt() {
  Write(CameraRegisters::kI2cClear, 1);
  if (md_config_.cb) {
    md_config_.cb(md_config_.cb_param);
  }
}

void CameraTask::SetMode(const CameraMode& mode) {
  Write(CameraRegisters::kModeSelect, static_cast<uint8_t>(mode));
  mode_ = mode;
}

void CameraTask::RequestHandler(camera::Request* req) {
  camera::Response resp;
  resp.type = req->type;
  switch (req->type) {
    case camera::RequestType::kEnable:
      resp.response.enable = HandleEnableRequest(req->request.mode);
      break;
    case camera::RequestType::kDisable:
      HandleDisableRequest();
      break;
    case camera::RequestType::kPower:
      resp.response.power = HandlePowerRequest(req->request.power);
      break;
    case camera::RequestType::kFrame:
      resp.response.frame = HandleFrameRequest(req->request.frame);
      break;
    case camera::RequestType::kTestPattern:
      HandleTestPatternRequest(req->request.test_pattern);
      break;
    case camera::RequestType::kDiscard:
      HandleDiscardRequest(req->request.discard);
      break;
    case camera::RequestType::kMotionDetectionInterrupt:
      HandleMotionDetectionInterrupt();
      break;
    case camera::RequestType::kMotionDetectionConfig:
      HandleMotionDetectionConfig(req->request.motion_detection_config);
      break;
  }
  if (req->callback) req->callback(resp);
}

}  // namespace coralmicro
