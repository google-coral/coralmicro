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

#ifndef LIBS_CAMERA_CAMERA_H_
#define LIBS_CAMERA_CAMERA_H_

#include <cstdint>
#include <functional>
#include <vector>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {

// The camera operating mode for `CameraTask::Enable()`.
enum class CameraMode : uint8_t {
  // Streaming mode. The camera continuously captures and pushes raw images to
  // an internal image buffer. You can then fetch images one at a time in your
  // preferred format with `CameraTask::GetFrame()`.
  kStreaming = 1,
  // Trigger mode. The camera captures one image at a time when you call
  // `CameraTask::Trigger()`. You can then fetch each image and process it
  // into your preferred format with `CameraTask::GetFrame()`.
  kTrigger = 5,
};

// Test patterns to use with `CameraTask::SetTestPattern()`
enum class CameraTestPattern : uint8_t {
  kNone = 0x00,
  kColorBar = 0x01,
  kWalkingOnes = 0x11,
};

// The function type required by `CameraMotionDetectionConfig`.
using CameraMotionDetectionCallback = void (*)(void* param);

// Specifies the configuration for motion detection (performed in camera).
//
// You can pre-fill this configuration with defaults by passing an instance to
// `CameraTask::GetMotionDetectionConfigDefault()`. Then specify
// your callback function (the `cb` variable) and pass this config to
// `CameraTask::SetMotionDetectionConfig()`.
// You must also enable camera streaming mode
// with `CameraTask::Enable()`.
//
// The default configuration detects any movement in the image frame, but you
// can specify a smaller detection region by defining a bounding box with the
// `x0`, `y0`, `x1`, and `y1` variables.
struct CameraMotionDetectionConfig {
  // The callback function to call when the camera detects motion.
  // The default config is `nullptr`.
  CameraMotionDetectionCallback cb;
  // Optional parameters to pass to the callback function.
  // The default config is `nullptr`.
  void* cb_param;
  // Set true to enable motion detection; false to disable it.
  // The default config is true.
  bool enable;
  // The detection zone's left-most pixel (index position).
  // The default config is `0`.
  size_t x0;
  // The detection zone's top-most pixel (index position).
  // The default config is `0`.
  size_t y0;
  // The detection zone's right-most pixel (index position).
  // The default config is `CameraTask::kWidth - 1` (`323`).
  size_t x1;
  // The detection zone's left-most pixel (index position).
  // The default config is `CameraTask::kHeight - 1` (`323`).
  size_t y1;
};

// @cond Do not generate docs
inline constexpr char kCameraTaskName[] = "camera_task";

namespace camera {

enum class RequestType : uint8_t {
  kEnable,
  kDisable,
  kFrame,
  kPower,
  kTestPattern,
  kDiscard,
  kMotionDetectionInterrupt,
  kMotionDetectionConfig,
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

struct TestPatternRequest {
  CameraTestPattern pattern;
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
    CameraMode mode;
    DiscardRequest discard;
    CameraMotionDetectionConfig motion_detection_config;
  } request;
  std::function<void(Response)> callback;
};

}  // namespace camera
// @endcond

// Image format options, used with `CameraFrameFormat`.
enum class CameraFormat {
  // RGB image.
  kRgb,
  // Y8 (grayscale) image.
  kY8,
  // Raw bayer image.
  kRaw,
};

// Gets the bytes-per-pixel (the number of color channels) used by the
// given image format.
// @param The image format (from `CameraFormat`).
// @return The number of bytes per pixel.
int CameraFormatBpp(CameraFormat fmt);

// Image resampling method (when resizing the image).
enum class CameraFilterMethod {
  kBilinear,
  kNearestNeighbor,
};

// Clockwise image rotations.
enum class CameraRotation {
  // The natural orientation for the camera module
  k0,
  // Rotated 90-degrees clockwise.
  // Upside down, relative to the board's "Coral" label.
  k90,
  // Rotated 180-degrees clockwise.
  k180,
  // Rotated 270-degrees clockwise.
  // Right-side up, relative to the board's "Coral" label.
  k270,
};

// Specifies your image buffer location and any image processing you want to
// perform when fetching images with `CameraTask::GetFrame()`.
struct CameraFrameFormat {
  // Image format such as RGB or raw.
  CameraFormat fmt;
  // Filter method such as bilinear (default) or nearest-neighbor.
  CameraFilterMethod filter = CameraFilterMethod::kBilinear;
  // Image rotation in 90-degree increments. Default is 270 degree which
  // corresponds to the device held vertically with USB port facing down.
  CameraRotation rotation = CameraRotation::k270;
  // Image width. (Native size is `CameraTask::kWidth`.)
  int width;
  // Image height. (Native size is `CameraTask::kHeight`.)
  int height;
  // If using non-native width/height, set this true to maintain the native
  // aspect ratio, false to crop the image.
  bool preserve_ratio;
  // Location to store the image.
  uint8_t* buffer;
  // Set true to perform auto whitebalancing (default), false to disable it.
  bool white_balance = true;
};

// Provides access to the Dev Board Micro camera.
//
// You can access the shared camera object with `CameraTask::GetSingleton()`.
class CameraTask
    : public QueueTask<camera::Request, camera::Response, kCameraTaskName,
                       configMINIMAL_STACK_SIZE * 10, kCameraTaskPriority,
                       /*QueueLength=*/4> {
 public:
  // Initializes the camera.
  //
  // Programs on the M7 do not need to call this because it is automatically
  // called internally. M7 programs can immediately turn on the camera with
  // `SetPower()`.
  //
  // Programs on the M4 must call this to intialize the camera before they can
  // turn on the camera. For example:
  //
  // ```
  // CameraTask::GetSingleton()->Init(I2C5Handle());
  // CameraTask::GetSingleton()->SetPower(true);
  // ```
  //
  // @param i2c_handle The camera I2C handle: `I2C5Handle()`.
  void Init(lpi2c_rtos_handle_t* i2c_handle);

  // Gets the `CameraTask` singleton.
  //
  // You must use this to acquire the shared `CameraTask` object.
  static CameraTask* GetSingleton() {
    static CameraTask camera;
    return &camera;
  }

  // Enables the camera to begin capture. You must call `SetPower()` before
  // this.
  // @param mode The operating mode (either `kStreaming` or `kTrigger`).
  // @return True if camera is enabled, false otherwise.
  bool Enable(CameraMode mode);

  // Sets the camera into a low-power state, using appoximately 200 μW
  // (compared to approximately 4 mW when streaming). The camera configuration
  // is sustained so it can quickly start again with `Enable()`.
  void Disable();

  // Gets one frame from the camera buffer and processes it into one or
  // more formats.
  //
  // @note This blocks until a new frame is available from the camera. However,
  // if trigger mode, it returns false if the camera has not been trigged (via
  // `CameraTask::Trigger`) since the last time `CameraTask::GetFrame` was
  // called.
  //
  // @param fmts A list of image formats you want to receive.
  // @return True if image processing succeeds, false otherwise.
  bool GetFrame(const std::vector<CameraFrameFormat>& fmts);

  // Turns the camera power on and off. You must call this before `Enable()`.
  // @param enable True to turn the camera on, false to turn it off.
  // @return True if the action was successful, false otherwise.
  bool SetPower(bool enable);

  // Enables a camera test pattern instead of using actual sensor data.
  // @param pattern The test pattern to use.
  void SetTestPattern(CameraTestPattern pattern);

  // Triggers image capture when the camera is enabled with
  // `CameraMode::kTrigger`.
  //
  // The raw image is held in the camera module memory and you must then
  // fetch it with `GetFrame()`.
  void Trigger();

  // Purges the image sensor data one frame at a time.
  //
  // This essentially captures images without saving any of the data,
  // which allows the sensor to calibrate exposure and rid the sensor of any
  // image artifacts that sometimes occur upon initialization.
  //
  // @param The number of frames to capture and immediately discard. To
  // allow auto exposure to calibrate, try discarding 100 frames before you
  // begin using images with `GetFrame()`.
  void DiscardFrames(int count);

  // Gets the default configuration for motion detection.
  //
  // @param config The `CameraMotionDetectionConfig` struct to fill with default
  // values.
  void GetMotionDetectionConfigDefault(CameraMotionDetectionConfig& config);

  // Sets the configuration for hardware motion detection.
  //
  // Note: You must enable camera streaming mode with `CameraTask::Enable()`.
  //
  // @param config `CameraMotionDetectionConfig` to apply to the camera.
  void SetMotionDetectionConfig(const CameraMotionDetectionConfig& config);

  // Native image pixel width.
  static constexpr size_t kWidth = 324;

  // Native image pixel height.
  static constexpr size_t kHeight = 324;

 private:
  int GetFrame(uint8_t** buffer, bool block);
  void ReturnFrame(int index);
  void TaskInit() override;
  void RequestHandler(camera::Request* req) override;
  camera::EnableResponse HandleEnableRequest(const CameraMode& mode);
  void HandleDisableRequest();
  camera::PowerResponse HandlePowerRequest(const camera::PowerRequest& power);
  camera::FrameResponse HandleFrameRequest(const camera::FrameRequest& frame);
  void HandleTestPatternRequest(const camera::TestPatternRequest& test_pattern);
  void HandleDiscardRequest(const camera::DiscardRequest& discard);
  void HandleMotionDetectionInterrupt();
  void HandleMotionDetectionConfig(const CameraMotionDetectionConfig& config);
  void SetMode(const CameraMode& mode);
  bool Read(uint16_t reg, uint8_t* val);
  bool Write(uint16_t reg, uint8_t val);
  void SetDefaultRegisters();
  void SetMotionDetectionRegisters();

  lpi2c_rtos_handle_t* i2c_handle_;
  csi_handle_t csi_handle_;
  csi_config_t csi_config_;
  CameraMode mode_;
  CameraTestPattern test_pattern_;
  CameraMotionDetectionConfig md_config_;
  bool enabled_{false};
};

}  // namespace coralmicro

#endif  // LIBS_CAMERA_CAMERA_H_
