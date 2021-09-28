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

namespace valiant {

using namespace camera;

static constexpr int kFramebufferCount = 4;
__attribute__((section(".sdram_bss,\"aw\",%nobits @")))
__attribute__((aligned(64)))
uint8_t framebuffers[kFramebufferCount][CameraTask::kHeight][CameraTask::kWidth];

namespace {
uint8_t* IndexToFramebufferPtr(int index) {
    if (index < 0 || index >= kFramebufferCount) {
        return nullptr;
    }
    return reinterpret_cast<uint8_t*>(framebuffers[index]);
}

int FramebufferPtrToIndex(uint8_t* framebuffer_ptr) {
    for (int i = 0; i < kFramebufferCount; ++i) {
        if (reinterpret_cast<uint8_t*>(framebuffers[i]) == framebuffer_ptr) {
            return i;
        }
    }
    return -1;
}
}

extern "C" void PXP_IRQHandler(void) {
    CameraTask::GetSingleton()->PXP_IRQHandler();
}

void CameraTask::PXP_IRQHandler() {
    if (kPXP_CompleteFlag & PXP_GetStatusFlags(PXP)) {
        BaseType_t reschedule = pdFALSE;
        PXP_ClearStatusFlags(PXP, kPXP_CompleteFlag);
        xSemaphoreGiveFromISR(pxp_semaphore_, &reschedule);
        portYIELD_FROM_ISR(reschedule);
    }
}

bool CameraTask::GetFrame(std::list<camera::FrameFormat> fmts) {
    bool ret = true;
    uint8_t *raw = nullptr;
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
                    if (fmt.width == kWidth && fmt.width == kHeight) {
                        BayerToRGB(raw, fmt.buffer, fmt.width, fmt.height);
                    } else {
                        std::unique_ptr<uint8_t> buffer_rgba(reinterpret_cast<uint8_t*>(malloc(FormatToBPP(Format::RGBA) * kWidth * kHeight)));
                        BayerToRGBA(raw, buffer_rgba.get(), kWidth, kHeight);
                        PXPConfiguration input;
                        input.data = buffer_rgba.get();
                        input.fmt = Format::RGBA;
                        input.width = kWidth;
                        input.height = kHeight;
                        PXPConfiguration output;
                        output.data = fmt.buffer;
                        output.fmt = Format::RGB;
                        output.width = fmt.width;
                        output.height = fmt.height;
                        ret = GetSingleton()->PXPOperation(input, output, fmt.preserve_ratio);
                    }
                }
                break;
            case Format::Y8: {
                    std::unique_ptr<uint8_t> buffer_rgba(reinterpret_cast<uint8_t*>(malloc(FormatToBPP(Format::RGBA) * kWidth * kHeight)));
                    if (!buffer_rgba) {
                        ret = false;
                        break;
                    }
                    BayerToRGBA(raw, buffer_rgba.get(), kWidth, kHeight);
                    PXPConfiguration input;
                    input.data = buffer_rgba.get();
                    input.fmt = Format::RGBA;
                    input.width = kWidth;
                    input.height = kHeight;
                    PXPConfiguration output;
                    output.data = fmt.buffer;
                    output.fmt = Format::Y8;
                    output.width = fmt.width;
                    output.height = fmt.height;
                    ret = GetSingleton()->PXPOperation(input, output, fmt.preserve_ratio);
                }
                break;
            default:
                ret = false;
        }
    }

    GetSingleton()->ReturnFrame(index);
    return ret;
}

namespace {
void BayerInternal(const uint8_t *camera_raw, int width, int height, std::function<void(int x, int y, uint8_t r, uint8_t g, uint8_t b)> callback) {
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
}
}  // namespace

void CameraTask::BayerToRGB(const uint8_t *camera_raw, uint8_t *camera_rgb, int width, int height) {
    memset(camera_rgb, 0, width * height * 3);
    BayerInternal(camera_raw, width, height, [camera_rgb, width, height](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        camera_rgb[(x * 3) + (y * width * 3) + 0] = r;
        camera_rgb[(x * 3) + (y * width * 3) + 1] = g;
        camera_rgb[(x * 3) + (y * width * 3) + 2] = b;
    });
}

void CameraTask::BayerToRGBA(const uint8_t *camera_raw, uint8_t *camera_rgba, int width, int height) {
    memset(camera_rgba, 0, width * height * 4);
    BayerInternal(camera_raw, width, height, [camera_rgba, width, height](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
            camera_rgba[(x * 4) + (y * width * 4) + 0] = r;
            camera_rgba[(x * 4) + (y * width * 4) + 1] = g;
            camera_rgba[(x * 4) + (y * width * 4) + 2] = b;
    });
}

int CameraTask::FormatToBPP(Format fmt) {
    switch (fmt) {
        case Format::RGBA:
            return 4;
        case Format::RGB:
            return 3;
        case Format::Y8:
            return 1;
    }
    return 0;
}

pxp_ps_pixel_format_t CameraTask::FormatToPXPPSFormat(Format fmt) {
    switch (fmt) {
        case Format::RGBA:
            return kPXP_PsPixelFormatRGB888;
        case Format::Y8:
            return kPXP_PsPixelFormatY8;
        default:
            assert(false);
    }
}

pxp_output_pixel_format_t CameraTask::FormatToPXPOutputFormat(Format fmt) {
    switch (fmt) {
        case Format::RGBA:
            return kPXP_OutputPixelFormatRGB888;
        case Format::RGB:
            return kPXP_OutputPixelFormatRGB888P;
        case Format::Y8:
            return kPXP_OutputPixelFormatY8;
        default:
            assert(false);
    }
}

bool CameraTask::PXPOperation(const PXPConfiguration& input, const PXPConfiguration& output, bool preserve_ratio) {
    int output_width = output.width, output_height = output.height;
    if (preserve_ratio) {
        float ratio_width = (float)output.width / (float)input.width;
        float ratio_height = (float)output.height / (float)input.height;
        float scaling_ratio = MIN(ratio_width, ratio_height);
        output_width = (int)((float)input.width * scaling_ratio);
        output_height = (int)((float)input.height * scaling_ratio);
    }
    int input_bpp = FormatToBPP(input.fmt);
    int output_bpp = FormatToBPP(output.fmt);
    pxp_ps_buffer_config_t ps_buffer_config;
    ps_buffer_config.pixelFormat = FormatToPXPPSFormat(input.fmt);
    ps_buffer_config.swapByte = false;
    ps_buffer_config.bufferAddr = reinterpret_cast<uint32_t>(input.data);
    ps_buffer_config.bufferAddrU = 0;
    ps_buffer_config.bufferAddrV = 0;
    ps_buffer_config.pitchBytes = input.width * input_bpp;

    pxp_output_buffer_config_t output_buffer_config;
    output_buffer_config.pixelFormat = FormatToPXPOutputFormat(output.fmt);
    output_buffer_config.interlacedMode = kPXP_OutputProgressive;
    output_buffer_config.buffer0Addr = reinterpret_cast<uint32_t>(output.data);
    output_buffer_config.buffer1Addr = 0;
    output_buffer_config.pitchBytes = output.width * output_bpp;
    output_buffer_config.width = output_width;
    output_buffer_config.height = output_height;

    PXP_SetProcessSurfaceScaler(PXP, input.width, input.height, output_width, output_height);
    PXP_SetProcessSurfacePosition(PXP, 0, 0, output_width - 1, output_height - 1);
    PXP_SetProcessSurfaceBufferConfig(PXP, &ps_buffer_config);
    PXP_SetOutputBufferConfig(PXP, &output_buffer_config);
    PXP_Start(PXP);
    if (xSemaphoreTake(pxp_semaphore_, pdMS_TO_TICKS(200)) == pdFALSE) {
        return false;
    }

    return true;
}

constexpr const uint8_t kCameraAddress = 0x24;
constexpr const char kCameraTaskName[] = "camera_task";

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
    pxp_semaphore_ = xSemaphoreCreateBinary();
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

    PXP_Init(PXP);
    NVIC_SetPriority(PXP_IRQn, 5);
    NVIC_EnableIRQ(PXP_IRQn);
    PXP_EnableInterrupts(PXP, kPXP_CompleteInterruptEnable);
    PXP_SetProcessSurfaceBackGroundColor(PXP, 0xF04A00FF);
    PXP_SetAlphaSurfacePosition(PXP, 0xFFFF, 0xFFFF, 0, 0);
    PXP_EnableCsc1(PXP, false);

    // Streaming
    status = CSI_TransferStart(CSI, &csi_handle_);
    SetMode(mode);
    resp.success = (status == kStatus_Success);
    return resp;
}

void CameraTask::HandleDisableRequest() {
    SetMode(Mode::STANDBY);
    CSI_TransferStop(CSI, &csi_handle_);
    NVIC_DisableIRQ(PXP_IRQn);
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

}  // namespace valiant
