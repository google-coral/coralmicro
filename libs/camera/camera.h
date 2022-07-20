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
#include <list>

#include "libs/base/queue_task.h"
#include "libs/base/tasks.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_csi.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coralmicro {

namespace camera {

// The camera operating mode for `CameraTask::Enable()`.
enum class Mode : uint8_t {
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

enum class RequestType : uint8_t {
  kEnable,
  kDisable,
  kFrame,
  kPower,
  kTestPattern,
  kDiscard,
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
  kNone = 0x00,
  kColorBar = 0x01,
  kWalkingOnes = 0x11,
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

// Image format options, used with `FrameFormat`.
enum class Format {
  // Currently not supported.
  kRgba,
  // RGB image.
  kRgb,
  // Y8 (grayscale) image.
  kY8,
  // Raw bayer image.
  kRaw,
};

// Image resampling method (when resizing the image).
enum class FilterMethod {
  kBilinear = 0,
  kNearestNeighbor,
};

// Clockwise image rotations.
enum class Rotation {
  k0,
  k90,
  k180,
  k270,
};

// Specifies your image buffer location and any image processing you want to
// perform when fetching images with `CameraTask::GetFrame()`.
struct FrameFormat {
  // Image format such as RGB or raw.
  Format fmt;
  // Filter method such as bilinear (default) or nearest-neighbor.
  FilterMethod filter = FilterMethod::kBilinear;
  // Image rotation in 90-degree increments.
  Rotation rotation = Rotation::k0;
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

}  // namespace camera

inline constexpr char kCameraTaskName[] = "camera_task";

// Provides access to the Dev Board Micro camera.
//
// All camera control is handled through the `CameraTask` singleton, which you
// can get with `CameraTask::GetSingleton()`.
// Then you must power on the camera with `SetPower()` and specify the
// camera mode (continuous capture or single image capture) with `Enable()`.
//
// To get and process each frame, then call `GetFrame()` and
// specify the image format with `camera::FrameFormat`.
//
// **Example** (from `examples/image_server/`):
//
// \snippet image_server/image_server.cc camera-stream
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
  bool Enable(camera::Mode mode);

  // Sets the camera into a low-power state, using appoximately 200 μW
  // (compared to approximately 4 mW when streaming). The camera configuration
  // is sustained so can quickly start again with `Enable()`.
  void Disable();

  // Gets one frame from the camera buffer and processes it into one or
  // more formats.
  // @param fmts A list of image formats you want to receive.
  // @return True if image processing succeeds, false otherwise.
  bool GetFrame(const std::list<camera::FrameFormat>& fmts);

  // Turns the camera power on and off. You must call this before `Enable()`.
  // @param enable True to turn the camera on, false to turn it off.
  // @return True if the action was successful, false otherwise.
  bool SetPower(bool enable);

  // Enables a camera test pattern instead of using actual sensor data.
  // @param pattern The test pattern to use.
  void SetTestPattern(camera::TestPattern pattern);

  // Triggers image capture when the camera is enabled with
  // `camera::Mode::kTrigger`.
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

  // Native image pixel width.
  static constexpr size_t kWidth = 324;

  // Native image pixel height.
  static constexpr size_t kHeight = 324;

  // Gets the bytes-per-pixel (the number of color channels) used by the
  // given image format.
  // @param The image format (from `camera::FrameFormat`).
  // @return The number of bytes per pixel.
  static int FormatToBPP(camera::Format fmt);

 private:
  int GetFrame(uint8_t** buffer, bool block);
  void ReturnFrame(int index);
  void TaskInit() override;
  void RequestHandler(camera::Request* req) override;
  camera::EnableResponse HandleEnableRequest(const camera::Mode& mode);
  void HandleDisableRequest();
  camera::PowerResponse HandlePowerRequest(const camera::PowerRequest& power);
  camera::FrameResponse HandleFrameRequest(const camera::FrameRequest& frame);
  void HandleTestPatternRequest(const camera::TestPatternRequest& test_pattern);
  void HandleDiscardRequest(const camera::DiscardRequest& discard);
  void SetMode(const camera::Mode& mode);
  bool Read(uint16_t reg, uint8_t* val);
  bool Write(uint16_t reg, uint8_t val);
  void SetDefaultRegisters();
  static void BayerToRGB(const uint8_t* camera_raw, uint8_t* camera_rgb,
                         int width, int height, camera::FilterMethod filter,
                         camera::Rotation rotation);
  static void BayerToRGBA(const uint8_t* camera_raw, uint8_t* camera_rgb,
                          int width, int height, camera::FilterMethod filter,
                          camera::Rotation rotation);
  static void BayerToGrayscale(const uint8_t* camera_raw,
                               uint8_t* camera_grayscale, int width, int height,
                               camera::FilterMethod filter,
                               camera::Rotation rotation);
  static void RGBToGrayscale(const uint8_t* camera_rgb,
                             uint8_t* camera_grayscale, int width, int height);
  static void AutoWhiteBalance(uint8_t* camera_rgb, int width, int height);
  static void ResizeNearestNeighbor(const uint8_t* src, int src_width,
                                    int src_height, uint8_t* dst, int dst_width,
                                    int dst_height, int components,
                                    bool preserve_aspect);

  static constexpr uint8_t kModelIdHExpected = 0x01;
  static constexpr uint8_t kModelIdLExpected = 0xB0;

  // CSI driver wants width to be divisible by 8, and 324 is not.
  // 324 * 324 == 13122 * 8 -- this makes the CSI driver happy!
  static constexpr size_t kCsiWidth = 8;
  static constexpr size_t kCsiHeight = 13122;

  lpi2c_rtos_handle_t* i2c_handle_;
  csi_handle_t csi_handle_;
  csi_config_t csi_config_;
  camera::Mode mode_;
  camera::TestPattern test_pattern_;
};

}  // namespace coralmicro

#endif  // LIBS_CAMERA_CAMERA_H_
