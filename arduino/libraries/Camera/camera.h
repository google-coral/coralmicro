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

// Exposes the Coral Micro device's native camera.  
// You should not initialize this object yourself; instead include `camera.h`
// and then use the global `Camera` instance.  You can start the camera with `Camera.begin()`
// and then access frames with `Camera.grab()`.  Example code can be found in `sketches/Camera`.
class CameraClass {
   public:
    // @cond Internal only, do not generate docs.
    // Externally, use `begin()` and `end()`.
    CameraClass()
        : camera_{coralmicro::CameraTask::GetSingleton()},
          width_{0},
          height_{0},
          format_{camera::Format::kRgb},
          filter_{camera::FilterMethod::kBilinear},
          test_pattern_{camera::TestPattern::kNone},
          preserve_ratio_{false},
          initialized_{false} {}

    ~CameraClass() {
        camera_->Disable();
        camera_->SetPower(false);
    };
    // @endcond

    // Starts the camera.
    //
    // @param resolution A `CameraResolution` value representing the camera's resolution.
    // @returns A `CameraStatus` value such as `SUCCESS` once initialization has completed.
    int begin(CameraResolution resolution = CAMERA_R320x320);

    // Starts the camera.
    //
    // @param width The resolution width in pixels.
    // @param height The resolution height in pixels.
    // @param fmt The image format.
    // @param filter The bayer filtering method. 
    // @param rotation The image rotation amount.
    // @returns `SUCCESS` once initialization has completed.
    int begin(int32_t width = 320, int32_t height = 320,
              camera::Format fmt = camera::Format::kRgb,
              camera::FilterMethod filter = camera::FilterMethod::kBilinear,
              camera::Rotation rotation = camera::Rotation::k0,
              bool preserve_ratio = false);

    // Turns the camera off.
    // 
    int end();

    // Grabs a camera frame.
    // 
    // @param buffer The buffer where the frame will be stored.
    // @returns A `CameraStatus` value such as `SUCCESS` if a frame was captured from the camera,
    // `NOT_INITIALIZED` if the camera was not initialized, or `FAILURE` if a frame was not captured.
    int grab(uint8_t* buffer);

    // Changes the camera mode.
    //
    // @param enable Sets the camera to streaming mode if true,
    // and sets the camera to standby mode if false.
    // @returns A `CameraStatus` value such as `SUCCESS` once the camera mode is set.
    int standby(bool enable);

    // Changes the camera's test pattern.  Test pattern data is fake data that
    // replaces camera sensor data when the value is not `NONE`.
    // 
    // @param walking Sets the test pattern to `WALKING_ONES` if true, 
    // and to `NONE` if false.
    // @returns A `CameraStatus` value such as `SUCCESS` once the pattern has been set.
    int testPattern(bool walking);

    // Changes the camera's test pattern.  Test pattern data is fake data that
    // replaces camera sensor data when the value is not `NONE`.
    // 
    // @param pattern The desired new test pattern.
    // @returns A `CameraStatus` value such as `SUCCESS` once the pattern has been set.
    int testPattern(coralmicro::camera::TestPattern pattern);

    // Sets whether the image's aspect ratio is preserved.
    //
    // @param preserve_ratio Images will be scaled to preserve aspect ratio if true.
    // @returns A `CameraStatus` value such as `SUCCESS` once the configuration has been set.
    int preserveRatio(bool preserve_ratio);

    // Sets the image format.
    //
    // @param fmt The image format.
    // @returns A `CameraSttus` value such as `SUCCESS` once the format has been set.
    int format(coralmicro::camera::Format fmt);

    // Discards a set amount of frames captured by the camera.
    //
    // @param num_frames The amount of frames to discard.
    // @returns A `CameraStatus` value such as `SUCCESS` once the camera has started discarding frames.
    int discardFrames(int num_frames);

    // @cond Internal only, do not generate docs.
    // Unimplemented APIs left over from Portenta.
    int motionDetection(bool enable);
    int motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    int motionDetectionThreshold(uint32_t threshold);
    int motionDetected();
    int framerate(uint32_t framerate);
    // @endcond

   private:
    std::unique_ptr<CameraTask> camera_;
    int32_t width_;
    int32_t height_;
    coralmicro::camera::Format format_;
    coralmicro::camera::FilterMethod filter_;
    coralmicro::camera::Rotation rotation_;
    coralmicro::camera::TestPattern test_pattern_;
    bool preserve_ratio_;
    bool initialized_;
};
}  // namespace arduino
}  // namespace coralmicro

// This is the global `CameraClass` instance you should use instead of 
// creating your own instance of `CameraClass`.
extern coralmicro::arduino::CameraClass Camera;

#endif  // CORAL_MICRO_CAMERACLASS_H
