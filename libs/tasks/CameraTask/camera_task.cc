#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"

namespace valiant {

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
}

int CameraTask::GetFrame(uint8_t** buffer, bool block) {
    CameraRequest req;
    req.type = CameraRequestType::Frame;
    req.request.frame.index = -1;
    CameraResponse resp;
    do {
        resp = SendRequest(req);
    } while (block && resp.response.frame.index == -1);
    *buffer = IndexToFramebufferPtr(resp.response.frame.index);
    return resp.response.frame.index;
}

void CameraTask::ReturnFrame(int index) {
    CameraRequest req;
    req.type = CameraRequestType::Frame;
    req.request.frame.index = index;
    SendRequest(req);
}

void CameraTask::Enable() {
    CameraRequest req;
    req.type = CameraRequestType::Enable;
    SendRequest(req);
}

void CameraTask::Disable() {
    CameraRequest req;
    req.type = CameraRequestType::Disable;
    SendRequest(req);
}

void CameraTask::SetPower(bool enable) {
    CameraRequest req;
    req.type = CameraRequestType::Power;
    req.request.power.enable = enable;
    SendRequest(req);
}

void CameraTask::SetTestPattern(TestPattern pattern) {
    CameraRequest req;
    req.type = CameraRequestType::TestPattern;
    req.request.test_pattern.pattern = pattern;
    SendRequest(req);
}

CameraResponse CameraTask::SendRequest(CameraRequest& req) {
    CameraResponse resp;
    resp.type = static_cast<CameraRequestType>(0xff);
    SemaphoreHandle_t req_semaphore = xSemaphoreCreateBinary();
    req.callback =
        [req_semaphore, &resp](CameraResponse cb_resp) {
            xSemaphoreGive(req_semaphore);
            resp = cb_resp;
        };
    xQueueSend(message_queue_, &req, pdMS_TO_TICKS(200));
    xSemaphoreTake(req_semaphore, pdMS_TO_TICKS(200));
    vSemaphoreDelete(req_semaphore);

    return resp;
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

    status = CSI_TransferCreateHandle(CSI, &csi_handle_, nullptr, 0);
    if (status != kStatus_Success) {
        return;
    }

    for (int i = 0; i < kFramebufferCount; i++) {
        status = CSI_TransferSubmitEmptyBuffer(CSI, &csi_handle_, reinterpret_cast<uint32_t>(framebuffers[i]));
        if (status != kStatus_Success) {
            return;
        }
    }

    PowerRequest req;
    req.enable = false;
    HandlePowerRequest(req);
}

EnableResponse CameraTask::HandleEnableRequest() {
    EnableResponse resp;
    uint8_t model_id_h = 0xff, model_id_l = 0xff;
    Read(CameraRegisters::MODEL_ID_H, &model_id_h);
    Read(CameraRegisters::MODEL_ID_L, &model_id_l);
    Write(CameraRegisters::SW_RESET, 0x00);
    if (model_id_h != kModelIdHExpected || model_id_l != kModelIdLExpected) {
        printf("Camera model id not as expected: 0x%02x%02x\r\n", model_id_h, model_id_l);
        resp.success = false;
        return resp;
    }

    // Gated clock mode
    uint8_t osc_clk_div;
    Read(CameraRegisters::OSC_CLK_DIV, &osc_clk_div);
    osc_clk_div |= 1 << 5;
    Write(CameraRegisters::OSC_CLK_DIV, osc_clk_div);

    // Quality type registers
    // Taken from Tensorflow's configuration in the person detection sample
    Write(CameraRegisters::AE_CTRL, 0x00);
    Write(CameraRegisters::BLC_CFG, 0x00);
    Write(CameraRegisters::DPC_CTRL, 0x00);
    Write(CameraRegisters::ANALOG_GAIN, 0x00);
    Write(CameraRegisters::DIGITAL_GAIN_H, 0x01);
    Write(CameraRegisters::DIGITAL_GAIN_L, 0x00);

    // Shifting
    Write(CameraRegisters::VSYNC_HSYNC_PIXEL_SHIFT_EN, 0x0);

    // Streaming
    status_t status = CSI_TransferStart(CSI, &csi_handle_);
    Write(CameraRegisters::MODE_SELECT, 1);

    resp.success = (status == kStatus_Success);
    return resp;
}

void CameraTask::HandleDisableRequest() {
    Write(CameraRegisters::MODE_SELECT, 0);
    CSI_TransferStop(CSI, &csi_handle_);
}

void CameraTask::HandlePowerRequest(const PowerRequest& power) {
    valiant::PmicTask::GetSingleton()->SetRailState(
            valiant::Rail::CAM_2V8, power.enable);
    valiant::PmicTask::GetSingleton()->SetRailState(
            valiant::Rail::CAM_1V8, power.enable);
    vTaskDelay(pdMS_TO_TICKS(10));
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
    Write(CameraRegisters::TEST_PATTERN_MODE, static_cast<uint8_t>(test_pattern.pattern));
}

void CameraTask::MessageHandler(CameraRequest *req) {
    CameraResponse resp;
    resp.type = req->type;
    switch (req->type) {
        case CameraRequestType::Enable:
            resp.response.enable = HandleEnableRequest();
            break;
        case CameraRequestType::Disable:
            HandleDisableRequest();
            break;
        case CameraRequestType::Power:
            HandlePowerRequest(req->request.power);
            break;
        case CameraRequestType::Frame:
            resp.response.frame = HandleFrameRequest(req->request.frame);
            break;
        case CameraRequestType::TestPattern:
            HandleTestPatternRequest(req->request.test_pattern);
            break;
    }
    if (req->callback)
        req->callback(resp);
}

}  // namespace valiant
