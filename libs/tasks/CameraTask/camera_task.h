#ifndef _LIBS_TASKS_CAMERA_TASK_H_
#define _LIBS_TASKS_CAMERA_TASK_H_

#include "libs/base/tasks_m7.h"
#include "libs/base/queue_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

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
    TEST_PATTERN_MODE = 0x0601,
    BLC_CFG = 0x1000,
    DPC_CTRL = 0x1008,
    VSYNC_HSYNC_PIXEL_SHIFT_EN = 0x1012,
    AE_CTRL = 0x2100,
    OSC_CLK_DIV = 0x3060,
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
    int GetFrame(uint8_t **buffer, bool block);
    void ReturnFrame(int index);
    void SetPower(bool enable);
    void SetTestPattern(camera::TestPattern pattern);

    // CSI driver wants width to be divisible by 8, and 324 is not.
    // 324 * 324 == 13122 * 8 -- this makes the CSI driver happy!
    static constexpr size_t kCsiWidth = 8;
    static constexpr size_t kCsiHeight = 13122;
    static constexpr size_t kWidth = 324;
    static constexpr size_t kHeight = 324;
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

    static constexpr uint8_t kModelIdHExpected = 0x01;
    static constexpr uint8_t kModelIdLExpected = 0xB0;
    lpi2c_rtos_handle_t* i2c_handle_;
    csi_handle_t csi_handle_;
    csi_config_t csi_config_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_CAMERA_TASK_H_
