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
  // Do not use this. If you want to conserve power when not using the camera,
  // then use `CameraTask::Disable()`.
  kStandBy = 0,
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

typedef void (*MotionDetectionCallback)(void* param);
struct MotionDetectionConfig {
  MotionDetectionCallback cb;
  void* cb_param;
  bool enable;
  size_t x0;
  size_t y0;
  size_t x1;
  size_t y1;
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
    MotionDetectionConfig motion_detection_config;
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
  k0,
  k90,
  k180,
  k270,
};

// Specifies your image buffer location and any image processing you want to
// perform when fetching images with `CameraTask::GetFrame()`.
struct CameraFrameFormat {
  // Image format such as RGB or raw.
  CameraFormat fmt;
  // Filter method such as bilinear (default) or nearest-neighbor.
  CameraFilterMethod filter = CameraFilterMethod::kBilinear;
  // Image rotation in 90-degree increments.
  CameraRotation rotation = CameraRotation::k0;
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
  // @cond Do not generate docs
  void Init(lpi2c_rtos_handle_t* i2c_handle);
  // @endcond

  // Gets the `CameraTask` singleton.
  //
  // You must use this to acquire the shared `CameraTask` object.
  static CameraTask* GetSingleton() {
    static CameraTask camera;
    return &camera;
  }

  // Enables the camera to begin capture. You must call `SetPower()` before
  // this.
  // @param mode The operating mode.
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

  // Populates the `config` parameter with a default configuration for motion
  // detection.
  //
  // @param config The `MotionDetectionConfig` struct to fill with default
  // values.
  void GetMotionDetectionConfigDefault(camera::MotionDetectionConfig& config);

  // Sets the configuration for hardware motion detection.
  //
  // @param config `MotionDetectionConfig` to apply to the camera.
  void SetMotionDetectionConfig(const camera::MotionDetectionConfig& config);

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
  void HandleMotionDetectionConfig(const camera::MotionDetectionConfig& config);
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
  camera::MotionDetectionConfig md_config_;
};

}  // namespace coralmicro

#endif  // LIBS_CAMERA_CAMERA_H_
