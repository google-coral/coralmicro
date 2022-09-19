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

#ifndef CORAL_MICRO_CAMERACLASS_H
#define CORAL_MICRO_CAMERACLASS_H

#include <cstdint>
#include <memory>

#include "libs/camera/camera.h"

namespace coralmicro {
namespace arduino {

// Supported camera resolutions.
//
enum CameraResolution : uint32_t {
  // QQVGA Resolution
  CAMERA_R160x120 = 0x00,
  // QVGA Resolution
  CAMERA_R320x240 = 0x01,
  // 320x320 Resolution
  CAMERA_R320x320 = 0x02,
  // 324x324 Default camera resolution.
  CAMERA_R324x324 = 0x03,
  CAMERA_RMAX
};

// Return statuses from camera API functions.
//
enum CameraStatus : int32_t {
  FAILURE = -1,
  SUCCESS = 0,
  NOT_INITIALIZED = 1,
  UNIMPLEMENTED = 2,
};

// Motion detection callback.
using md_callback_t = void (*)(void* param);

// Represents an image's frame buffer.
class FrameBuffer {
 public:
  // Constructs a new frame buffer object without allocating framebuffer
  // pointer.
  FrameBuffer();

  // Destroys and delete the frame buffer if it is allocated.
  // Note: The buffer must be allocated with `new []` and not `malloc()`.
  ~FrameBuffer();

  // Gets the frame buffer size.
  // @return The buffer's size.
  uint32_t getBufferSize();

  // Gets the frame buffer's data.
  // @return The pointer to the frame buffer's data.
  uint8_t* getBuffer();

  // Set the frame buffer's data.
  // @param buffer The frame buffer pointer to set.
  // @param frame_size The frame buffer size to set.
  void setBuffer(uint8_t* buffer, uint32_t frame_size);

  // Checks if the frame buffer has a fixed size.
  // @return true if the frame buffer has been allocated and the size is known.
  bool hasFixedSize();

  // Checks if the frame buffer is allocated.
  // @return true if the framebuffer is allocated, else false.
  bool isAllocated();

 private:
  uint32_t fb_size_;
  uint8_t* fb_;
};

// Exposes the Coral Micro device's native camera.
//
// You should not initialize this object yourself; instead include
// `coralmicro_camera.h` and then use the global `Camera` instance.
class CameraClass {
 public:
  // @cond Do not generate docs.
  // Externally, use `begin()` and `end()`.
  CameraClass();

  ~CameraClass() {
    camera_->Disable();
    camera_->SetPower(false);
  };
  // @endcond

  // Starts the camera.
  //
  // @param resolution A `CameraResolution` value representing the camera's
  // resolution.
  // @returns A `CameraStatus` value such as `SUCCESS` once initialization has
  // completed.
  int begin(CameraResolution resolution = CAMERA_R324x324);

  // Starts the camera.
  //
  // @param width The resolution width in pixels.
  // @param height The resolution height in pixels.
  // @param fmt The image format.
  // @param filter The bayer filtering method.
  // @param rotation The image rotation amount.
  // @param auto_white_balance Applies auto white balance if true.
  // @returns `SUCCESS` once initialization has completed.
  int begin(int32_t width = 320, int32_t height = 320,
            CameraFormat fmt = CameraFormat::kRgb,
            CameraFilterMethod filter = CameraFilterMethod::kBilinear,
            CameraRotation rotation = CameraRotation::k270,
            bool preserve_ratio = false, bool auto_white_balance = true);

  // Grabs a camera frame.
  //
  // @param buffer The buffer where the frame will be stored.
  // @returns A `CameraStatus` value such as `SUCCESS` if a frame was captured
  // from the camera, `NOT_INITIALIZED` if the camera was not initialized, or
  // `FAILURE` if a frame was not captured.
  int grab(uint8_t* buffer);

  // Grabs a camera frame.
  //
  // @param buffer The buffer where the frame will be stored.
  // @returns A `CameraStatus` value such as `SUCCESS` if a frame was captured
  // from the camera, `NOT_INITIALIZED` if the camera was not initialized, or
  // `FAILURE` if a frame was not captured.
  // Note: buffer does not have to be allocated, this function will
  // automatically allocate the memory needed to store the image data.
  int grab(FrameBuffer& buffer);

  // Changes the camera mode.
  //
  // @param enable Sets the camera to standby mode if true, and sets the camera
  // to streaming mode if false.
  // @returns A `CameraStatus` value such as `SUCCESS` once the camera mode is
  // set.
  int setStandby(bool enable);

  // Changes the camera's test pattern.  Test pattern data is fake data that
  // replaces camera sensor data when the value is not `NONE`.
  //
  // @param walking Sets the test pattern to `WALKING_ONES` if true,
  // and to `NONE` if false.
  // @param enable Starts test pattern if true, else only set the test pattern.
  // @returns A `CameraStatus` value such as `SUCCESS` once the pattern has been
  // set.
  int setTestPattern(bool enable, bool walking);

  // Changes the camera's test pattern.  Test pattern data is fake data that
  // replaces camera sensor data when the value is not `NONE`.
  //
  // @param pattern The desired new test pattern.
  // @param enable Starts test pattern if true, else only set the test pattern.
  // @returns A `CameraStatus` value such as `SUCCESS` once the pattern has been
  // set.
  int setTestPattern(bool enable, coralmicro::CameraTestPattern pattern);

  // Sets whether the image's aspect ratio is preserved.
  //
  // @param preserve_ratio Images will be scaled to preserve aspect ratio if
  // true.
  // @returns A `CameraStatus` value such as `SUCCESS` once the configuration
  // has been set.
  int setPreserveRatio(bool preserve_ratio);

  // Sets the image format.
  //
  // @param fmt The image format.
  // @returns A `CameraSttus` value such as `SUCCESS` once the format has been
  // set.
  int setPixelFormat(coralmicro::CameraFormat fmt);

  // Discards a set amount of frames captured by the camera.
  //
  // @param num_frames The amount of frames to discard.
  // @returns A `CameraStatus` value such as `SUCCESS` once the camera has
  // started discarding frames.
  int discardFrames(int num_frames);

  // Enables or disables HW motion detection.
  //
  // @param callback The function to call when the camera detected motion.
  // @param cb_param The callback parameter.
  int enableMotionDetection(md_callback_t callback = nullptr,
                            void* cb_param = nullptr);

  // Disables motion detection.
  int disableMotionDetection();

  // Sets the motion detection windows.
  //
  // @param x The top left x coordinate to monitor motion detection.
  // @param y the top left y coordinate to monitor motion detection.
  // @param w The width of the window to monitor for motion.
  // @param h the height of the window to monitor for motion.
  int setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

  // @cond Do not generate docs.
  // Unimplemented APIs left over from Portenta.
  int setMotionDetectionThreshold(uint32_t threshold);
  int setMotionDetected();
  int setFramerate(uint32_t framerate);
  // @endcond

 private:
  std::unique_ptr<CameraTask> camera_;
  int32_t width_;
  int32_t height_;
  coralmicro::CameraFormat format_;
  coralmicro::CameraFilterMethod filter_;
  coralmicro::CameraRotation rotation_;
  coralmicro::CameraTestPattern test_pattern_;
  bool preserve_ratio_;
  bool auto_white_balance_;
  bool initialized_;
  coralmicro::CameraMotionDetectionConfig md_config_;
};
}  // namespace arduino
}  // namespace coralmicro

// This is the global `CameraClass` instance you should use instead of
// creating your own instance.
extern coralmicro::arduino::CameraClass Camera;

#endif  // CORAL_MICRO_CAMERACLASS_H
