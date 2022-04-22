#ifndef _LIBS_TASKS_CAMERA_TASK_H_
#define _LIBS_TASKS_CAMERA_TASK_H_

#include "libs/base/tasks.h"
#include "libs/base/queue_task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#include <functional>
#include <list>

namespace valiant {

namespace camera {

enum class Mode : uint8_t {
    STANDBY = 0,
    STREAMING = 1,
    TRIGGER = 5,
};

enum class RequestType : uint8_t {
    Enable,
    Disable,
    Frame,
    Power,
    TestPattern,
    Discard,
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

struct DiscardRequest {
    int count;
};

struct EnableResponse {
    bool success;
};

struct PowerResponse {
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
      PowerResponse power;
  } response;
};

struct Request {
    RequestType type;
    union {
        FrameRequest frame;
        PowerRequest power;
        TestPatternRequest test_pattern;
        Mode mode;
        DiscardRequest discard;
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
    RAW,
};

enum class FilterMethod {
    BILINEAR = 0,
    NEAREST_NEIGHBOR,
};

struct FrameFormat {
    Format fmt;
    FilterMethod filter;
    int width;
    int height;
    bool preserve_ratio;
    uint8_t *buffer;
};

}  // namespace camera

static constexpr size_t kCameraTaskStackDepth = configMINIMAL_STACK_SIZE * 10;
static constexpr UBaseType_t kCameraTaskQueueLength = 4;
extern const char kCameraTaskName[];

class CameraTask : public QueueTask<camera::Request, camera::Response, kCameraTaskName, kCameraTaskStackDepth, CAMERA_TASK_PRIORITY, kCameraTaskQueueLength> {
  public:
    void Init(lpi2c_rtos_handle_t *i2c_handle);
    static CameraTask *GetSingleton() {
        static CameraTask camera;
        return &camera;
    }
    void Enable(camera::Mode mode);
    void Disable();
    // TODO(atv): Convert this to return a class that cleans up?
    int GetFrame(uint8_t **buffer, bool block);
    static bool GetFrame(const std::list<camera::FrameFormat>& fmts);
    void ReturnFrame(int index);
    bool SetPower(bool enable);
    void SetTestPattern(camera::TestPattern pattern);
    void Trigger();
    void DiscardFrames(int count);

    // CSI driver wants width to be divisible by 8, and 324 is not.
    // 324 * 324 == 13122 * 8 -- this makes the CSI driver happy!
    static constexpr size_t kCsiWidth = 8;
    static constexpr size_t kCsiHeight = 13122;
    static constexpr size_t kWidth = 324;
    static constexpr size_t kHeight = 324;

    static int FormatToBPP(camera::Format fmt);
  private:
    void TaskInit() override;
    void RequestHandler(camera::Request *req) override;
    camera::EnableResponse HandleEnableRequest(const camera::Mode& mode);
    void HandleDisableRequest();
    camera::PowerResponse HandlePowerRequest(const camera::PowerRequest& power);
    camera::FrameResponse HandleFrameRequest(const camera::FrameRequest& frame);
    void HandleTestPatternRequest(const camera::TestPatternRequest& test_pattern);
    void HandleDiscardRequest(const camera::DiscardRequest& discard);
    void SetMode(const camera::Mode& mode);
    bool Read(camera::CameraRegisters reg, uint8_t *val);
    bool Write(camera::CameraRegisters reg, uint8_t val);
    void SetDefaultRegisters();
    static void BayerToRGB(const uint8_t *camera_raw, uint8_t *camera_rgb, int width, int height, camera::FilterMethod filter);
    static void BayerToRGBA(const uint8_t *camera_raw, uint8_t *camera_rgb, int width, int height, camera::FilterMethod filter);
    static void BayerToGrayscale(const uint8_t *camera_raw, uint8_t *camera_grayscale, int width, int height, camera::FilterMethod filter);
    static void RGBToGrayscale(const uint8_t *camera_rgb, uint8_t *camera_grayscale, int width, int height);
    static void ResizeNearestNeighbor(const uint8_t *src, int src_width, int src_height,
                                      uint8_t *dst, int dst_width, int dst_height,
                                      int components, bool preserve_aspect);

    static constexpr uint8_t kModelIdHExpected = 0x01;
    static constexpr uint8_t kModelIdLExpected = 0xB0;
    lpi2c_rtos_handle_t* i2c_handle_;
    csi_handle_t csi_handle_;
    csi_config_t csi_config_;
    camera::Mode mode_;
};

}  // namespace valiant

#endif  // _LIBS_TASKS_CAMERA_TASK_H_
