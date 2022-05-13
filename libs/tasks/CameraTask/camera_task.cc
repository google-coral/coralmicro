#include "libs/base/gpio.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#if (__CORTEX_M == 7)
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"
#elif (__CORTEX_M == 4)
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm4/fsl_cache.h"
#endif

#include <memory>

namespace coral::micro {
using namespace camera;

namespace {
constexpr uint8_t kCameraAddress = 0x24;
constexpr int kFramebufferCount = 4;
constexpr float kRedCoefficient = .2126;
constexpr float kGreenCoefficient = .7152;
constexpr float kBlueCoefficient = .0722;
constexpr float kUint8Max = 255.0;

__attribute__((section(".sdram_bss,\"aw\",%nobits @")))
__attribute__((aligned(64)))
uint8_t framebuffers[kFramebufferCount][CameraTask::kHeight][CameraTask::kWidth];

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

bool CameraTask::GetFrame(const std::list<camera::FrameFormat> &fmts) {
    bool ret = true;
    uint8_t* raw = nullptr;
    int index = GetSingleton()->GetFrame(&raw, true);
    if (!raw) {
        return false;
    }
    if (GetSingleton()->mode_ == Mode::TRIGGER) {
        gpio::SetGpio(gpio::Gpio::kCameraTrigger, false);
    }

    for (const camera::FrameFormat& fmt : fmts) {
        switch (fmt.fmt) {
            case Format::RGB: {
                    if (fmt.width == kWidth && fmt.height == kHeight) {
                        BayerToRGB(raw, fmt.buffer, fmt.width, fmt.height, fmt.filter);
                    } else {
                        auto buffer_rgb = std::make_unique<uint8_t[]>(FormatToBPP(Format::RGB) * kWidth * kHeight);
                        BayerToRGB(raw, buffer_rgb.get(), kWidth, kHeight, fmt.filter);
                        ResizeNearestNeighbor(buffer_rgb.get(), kWidth, kHeight, fmt.buffer,
                                              fmt.width, fmt.height, FormatToBPP(Format::RGB),
                                              fmt.preserve_ratio);
                    }
                }
                break;
            case Format::Y8: {
                    if (fmt.width == kWidth && fmt.height == kHeight) {
                        BayerToGrayscale(raw, fmt.buffer, kWidth, kHeight, fmt.filter);
                    } else {
                        auto buffer_rgb = std::make_unique<uint8_t[]>(FormatToBPP(Format::RGB) * kWidth * kHeight);
                        auto buffer_rgb_scaled = std::make_unique<uint8_t[]>(FormatToBPP(Format::RGB) * fmt.width * fmt.height);
                        BayerToRGB(raw, buffer_rgb.get(), kWidth, kHeight, fmt.filter);
                        ResizeNearestNeighbor(buffer_rgb.get(), kWidth, kHeight, buffer_rgb_scaled.get(),
                                              fmt.width, fmt.height, FormatToBPP(Format::RGB),
                                              fmt.preserve_ratio);
                        RGBToGrayscale(buffer_rgb_scaled.get(), fmt.buffer, fmt.width, fmt.height);
                    }
                }
                break;
            case Format::RAW:
                if (fmt.width != kWidth || fmt.height != kHeight) {
                    ret = false;
                    break;
                }
                memcpy(fmt.buffer, raw, kWidth * kHeight * FormatToBPP(Format::RAW));
                ret = true;
                break;
            default:
                ret = false;
        }
    }

    GetSingleton()->ReturnFrame(index);
    return ret;
}

void CameraTask::ResizeNearestNeighbor(const uint8_t *src, int src_w, int src_h,
                                       uint8_t *dst, int dst_w, int dst_h,
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
      const uint8_t *src_y = src + offset_y;
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
    if (filter == FilterMethod::NEAREST_NEIGHBOR) {
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
    } else if (filter == FilterMethod::BILINEAR) {
        int bayer_stride = width;

        size_t bayer_offset = 0;
        for (int y = 2; y < height - 2; y++) {
            bool odd_row = y & 1;
            int x = 1;
            size_t bayer_end = bayer_offset + (width - 2);

            if (odd_row) {
                uint8_t r =
                    (static_cast<uint32_t>(camera_raw[bayer_offset + 1]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2 + 1)]) +
                     1) >>
                    1;
                uint8_t b =
                    (static_cast<uint32_t>(
                         camera_raw[bayer_offset + bayer_stride]) +
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
                uint8_t t0 =
                    (static_cast<uint32_t>(camera_raw[bayer_offset]) +
                     static_cast<uint32_t>(camera_raw[bayer_offset + 2]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2)]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2 + 2)]) +
                     2) >>
                    2;
                g1 = (static_cast<uint32_t>(camera_raw[bayer_offset + 1]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + bayer_stride]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride + 2)]) +
                      static_cast<uint32_t>(
                          camera_raw[bayer_offset + (bayer_stride * 2 + 1)]) +
                      2) >>
                     2;
                uint8_t t1 =
                    (static_cast<uint32_t>(camera_raw[bayer_offset + 2]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2 + 2)]) +
                     1) >>
                    1;
                uint8_t t2 =
                    (static_cast<uint32_t>(
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
                uint8_t t0 =
                    (static_cast<uint32_t>(camera_raw[bayer_offset]) +
                     static_cast<uint32_t>(camera_raw[bayer_offset + 2]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2)]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + (bayer_stride * 2 + 2)]) +
                     2) >>
                    2;
                uint8_t g =
                    (static_cast<uint32_t>(camera_raw[bayer_offset + 1]) +
                     static_cast<uint32_t>(
                         camera_raw[bayer_offset + bayer_stride]) +
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
}  // namespace

void CameraTask::BayerToRGB(const uint8_t *camera_raw, uint8_t *camera_rgb, int width, int height, FilterMethod filter) {
    memset(camera_rgb, 0, width * height * 3);
    BayerInternal(camera_raw, width, height, filter, [camera_rgb, width, height](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        camera_rgb[(x * 3) + (y * width * 3) + 0] = r;
        camera_rgb[(x * 3) + (y * width * 3) + 1] = g;
        camera_rgb[(x * 3) + (y * width * 3) + 2] = b;
    });
}

void CameraTask::BayerToRGBA(const uint8_t *camera_raw, uint8_t *camera_rgba, int width, int height, FilterMethod filter) {
    memset(camera_rgba, 0, width * height * 4);
    BayerInternal(camera_raw, width, height, filter, [camera_rgba, width, height](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
            camera_rgba[(x * 4) + (y * width * 4) + 0] = r;
            camera_rgba[(x * 4) + (y * width * 4) + 1] = g;
            camera_rgba[(x * 4) + (y * width * 4) + 2] = b;
    });
}

void CameraTask::BayerToGrayscale(const uint8_t *camera_raw, uint8_t *camera_grayscale, int width, int height, FilterMethod filter) {
    BayerInternal(camera_raw, width, height, filter, [camera_grayscale, width, height](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        float r_f = static_cast<float>(r) / kUint8Max;
        float g_f = static_cast<float>(g) / kUint8Max;
        float b_f = static_cast<float>(b) / kUint8Max;
        camera_grayscale[x + (y * width)] = static_cast<uint8_t>(((kRedCoefficient * r_f * r_f) + (kGreenCoefficient * g_f * g_f) + (kBlueCoefficient * b_f * b_f)) * kUint8Max);
    });
}

void CameraTask::RGBToGrayscale(const uint8_t *camera_rgb, uint8_t *camera_grayscale, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float r_f = static_cast<float>(camera_rgb[(x * 3) + (y * width * 3) + 0]) / kUint8Max;
            float g_f = static_cast<float>(camera_rgb[(x * 3) + (y * width * 3) + 1]) / kUint8Max;
            float b_f = static_cast<float>(camera_rgb[(x * 3) + (y * width * 3) + 2]) / kUint8Max;
            camera_grayscale[x + (y * width)] = static_cast<uint8_t>(((kRedCoefficient * r_f * r_f) + (kGreenCoefficient * g_f * g_f) + (kBlueCoefficient * b_f * b_f)) * kUint8Max);
        }
    }
}

int CameraTask::FormatToBPP(Format fmt) {
    switch (fmt) {
        case Format::RGBA:
            return 4;
        case Format::RGB:
            return 3;
        case Format::RAW:
        case Format::Y8:
            return 1;
    }
    return 0;
}

extern "C" void CSI_DriverIRQHandler(void);
extern "C" void CSI_IRQHandler(void) {
    CSI_DriverIRQHandler();
    __DSB();
}

bool CameraTask::Read(CameraRegisters reg, uint8_t *val) {
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

bool CameraTask::Write(CameraRegisters reg, uint8_t val) {
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

void CameraTask::Init(lpi2c_rtos_handle_t *i2c_handle) {
    QueueTask::Init();
    i2c_handle_ = i2c_handle;
    mode_ = Mode::STANDBY;
}

int CameraTask::GetFrame(uint8_t** buffer, bool block) {
    Request req;
    req.type = RequestType::Frame;
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
    req.type = RequestType::Frame;
    req.request.frame.index = index;
    SendRequest(req);
}

void CameraTask::Enable(Mode mode) {
    Request req;
    req.type = RequestType::Enable;
    req.request.mode = mode;
    SendRequest(req);
}

void CameraTask::Disable() {
    Request req;
    req.type = RequestType::Disable;
    SendRequest(req);
}

bool CameraTask::SetPower(bool enable) {
    Request req;
    req.type = RequestType::Power;
    req.request.power.enable = enable;
    Response resp = SendRequest(req);
    return resp.response.power.success;
}

void CameraTask::SetTestPattern(TestPattern pattern) {
    Request req;
    req.type = RequestType::TestPattern;
    req.request.test_pattern.pattern = pattern;
    SendRequest(req);
}

void CameraTask::Trigger() {
    gpio::SetGpio(gpio::Gpio::kCameraTrigger, true);
}

void CameraTask::DiscardFrames(int count) {
    Request req;
    req.type = RequestType::Discard;
    req.request.discard.count = count;
    SendRequest(req);
}

void CameraTask::TaskInit() {
    status_t status;
    csi_config_.width = kCsiWidth;
    csi_config_.height = kCsiHeight;
    csi_config_.polarityFlags = kCSI_HsyncActiveHigh | kCSI_VsyncActiveHigh | kCSI_DataLatchOnRisingEdge;
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
    Write(static_cast<CameraRegisters>(0x3044), 0x0A);
    Write(static_cast<CameraRegisters>(0x3045), 0x00);
    Write(static_cast<CameraRegisters>(0x3047), 0x0A);
    Write(static_cast<CameraRegisters>(0x3050), 0xC0);
    Write(static_cast<CameraRegisters>(0x3051), 0x42);
    Write(static_cast<CameraRegisters>(0x3052), 0x50);
    Write(static_cast<CameraRegisters>(0x3053), 0x00);
    Write(static_cast<CameraRegisters>(0x3054), 0x03);
    Write(static_cast<CameraRegisters>(0x3055), 0xF7);
    Write(static_cast<CameraRegisters>(0x3056), 0xF8);
    Write(static_cast<CameraRegisters>(0x3057), 0x29);
    Write(static_cast<CameraRegisters>(0x3058), 0x1F);
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
    if (mode == Mode::TRIGGER) {
        framebuffer_count = 2;
    }
    for (int i = 0; i < framebuffer_count; i++) {
        status = CSI_TransferSubmitEmptyBuffer(CSI, &csi_handle_, reinterpret_cast<uint32_t>(framebuffers[i]));
    }

    // Streaming
    status = CSI_TransferStart(CSI, &csi_handle_);
    SetMode(mode);
    resp.success = (status == kStatus_Success);
    return resp;
}

void CameraTask::HandleDisableRequest() {
    SetMode(Mode::STANDBY);
    CSI_TransferStop(CSI, &csi_handle_);
}

PowerResponse CameraTask::HandlePowerRequest(const PowerRequest& power) {
    PowerResponse resp;
    resp.success = true;
    PmicTask::GetSingleton()->SetRailState(
            pmic::Rail::CAM_2V8, power.enable);
    PmicTask::GetSingleton()->SetRailState(
            pmic::Rail::CAM_1V8, power.enable);
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
            printf("Camera model id not as expected: 0x%02x%02x\r\n", model_id_h, model_id_l);
            resp.success = false;
        }
    }
    return resp;
}

FrameResponse CameraTask::HandleFrameRequest(const FrameRequest& frame) {
    FrameResponse resp;
    resp.index = -1;
    uint32_t buffer;
    if (frame.index == -1) { // GET
        status_t status = CSI_TransferGetFullBuffer(CSI, &csi_handle_, &buffer);
        if (status == kStatus_Success) {
            DCACHE_InvalidateByRange(buffer, kHeight * kWidth);
            resp.index = FramebufferPtrToIndex(reinterpret_cast<uint8_t*>(buffer));
        }
    } else { // RETURN
        buffer = reinterpret_cast<uint32_t>(IndexToFramebufferPtr(frame.index));
        if (buffer) {
            CSI_TransferSubmitEmptyBuffer(CSI, &csi_handle_, buffer);
        }
    }
    return resp;
}

void CameraTask::HandleTestPatternRequest(const TestPatternRequest& test_pattern) {
    if (test_pattern.pattern == TestPattern::NONE) {
        SetDefaultRegisters();
    } else {
        Write(CameraRegisters::AE_CTRL, 0x00);
        Write(CameraRegisters::BLC_CFG, 0x00);
        Write(CameraRegisters::DPC_CTRL, 0x00);
        Write(CameraRegisters::ANALOG_GAIN, 0x00);
        Write(CameraRegisters::DIGITAL_GAIN_H, 0x01);
        Write(CameraRegisters::DIGITAL_GAIN_L, 0x00);
    }
    Write(CameraRegisters::TEST_PATTERN_MODE, static_cast<uint8_t>(test_pattern.pattern));
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

void CameraTask::RequestHandler(Request *req) {
    Response resp;
    resp.type = req->type;
    switch (req->type) {
        case RequestType::Enable:
            resp.response.enable = HandleEnableRequest(req->request.mode);
            break;
        case RequestType::Disable:
            HandleDisableRequest();
            break;
        case RequestType::Power:
            resp.response.power = HandlePowerRequest(req->request.power);
            break;
        case RequestType::Frame:
            resp.response.frame = HandleFrameRequest(req->request.frame);
            break;
        case RequestType::TestPattern:
            HandleTestPatternRequest(req->request.test_pattern);
            break;
        case RequestType::Discard:
            HandleDiscardRequest(req->request.discard);
            break;
    }
    if (req->callback)
        req->callback(resp);
}

}  // namespace coral::micro
