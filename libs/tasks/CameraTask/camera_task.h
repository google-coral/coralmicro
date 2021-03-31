#ifndef _LIBS_TASKS_CAMERA_TASK_H_
#define _LIBS_TASKS_CAMERA_TASK_H_

#include "libs/base/tasks_m7.h"
#include "libs/base/queue_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pxp.h"

#include <functional>

namespace valiant {

namespace camera {

enum class RequestType : uint8_t {
    Enable,
    Disable,
    Frame,
    Power,
    TestPattern,
};

struct FrameRequest {
    int index;
};

struct FrameResponse {
    int index;
};

struct PowerRequest {
    bool enable;
};

struct EnableResponse {
    bool success;
};

enum class TestPattern : uint8_t {
  NONE = 0x00,
  COLOR_BAR = 0x01,
  WALKING_ONES = 0x11,
};

struct TestPatternRequest {
    TestPattern pattern;
};

struct Response {
  RequestType type;
  union {
      FrameResponse frame;
      EnableResponse enable;
  } response;
};

struct Request {
    RequestType type;
    union {
        FrameRequest frame;
        PowerRequest power;
        TestPatternRequest test_pattern;
    } request;
    std::function<void(Response)> callback;
};

enum class CameraRegisters : uint16_t {
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

enum class Format {
    RGBA,
    RGB,
    Y8,
};

struct FrameFormat {
    Format fmt;
    int width;
    int height;
    bool preserve_ratio;
};

}  // namespace camera

static constexpr size_t kCameraTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kCameraTaskQueueLength = 4;
extern const char kCameraTaskName[];

class CameraTask : public QueueTask<camera::Request, camera::Response, kCameraTaskName, kCameraTaskStackDepth, CAMERA_TASK_PRIORITY, kCameraTaskQueueLength> {
  public:
    void Init(lpi2c_rtos_handle_t *i2c_handle);
    static CameraTask *GetSingleton() {
        static CameraTask pmic;
        return &pmic;
    }
    void Enable();
    void Disable();
    // TODO(atv): Convert this to return a class that cleans up?
    int GetFrame(uint8_t **buffer, bool block);
    static bool GetFrame(const camera::FrameFormat& fmt, uint8_t *buffer);
    void ReturnFrame(int index);
    void SetPower(bool enable);
    void SetTestPattern(camera::TestPattern pattern);

    // CSI driver wants width to be divisible by 8, and 324 is not.
    // 324 * 324 == 13122 * 8 -- this makes the CSI driver happy!
    static constexpr size_t kCsiWidth = 8;
    static constexpr size_t kCsiHeight = 13122;
    static constexpr size_t kWidth = 324;
    static constexpr size_t kHeight = 324;

    void PXP_IRQHandler();
  private:
    void TaskInit() override;
    void RequestHandler(camera::Request *req) override;
    camera::EnableResponse HandleEnableRequest();
    void HandleDisableRequest();
    void HandlePowerRequest(const camera::PowerRequest& power);
    camera::FrameResponse HandleFrameRequest(const camera::FrameRequest& frame);
    void HandleTestPatternRequest(const camera::TestPatternRequest& test_pattern);
    bool Read(camera::CameraRegisters reg, uint8_t *val);
    bool Write(camera::CameraRegisters reg, uint8_t val);
    void SetDefaultRegisters();
    struct PXPConfiguration {
        const uint8_t *data;
        camera::Format fmt;
        int width;
        int height;
    };
    static pxp_ps_pixel_format_t FormatToPXPPSFormat(camera::Format fmt);
    static pxp_output_pixel_format_t FormatToPXPOutputFormat(camera::Format fmt);
    static int FormatToBPP(camera::Format fmt);
    static void BayerToRGB(const uint8_t *camera_raw, uint8_t *camera_rgb, int width, int height);
    static void BayerToRGBA(const uint8_t *camera_raw, uint8_t *camera_rgb, int width, int height);
    bool PXPOperation(const PXPConfiguration& input, const PXPConfiguration& output, bool preserve_ratio);

    static constexpr uint8_t kModelIdHExpected = 0x01;
    static constexpr uint8_t kModelIdLExpected = 0xB0;
    lpi2c_rtos_handle_t* i2c_handle_;
    csi_handle_t csi_handle_;
    csi_config_t csi_config_;
    SemaphoreHandle_t pxp_semaphore_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_CAMERA_TASK_H_
