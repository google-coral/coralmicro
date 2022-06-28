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

namespace coral::micro {
namespace arduino {

enum {
    CAMERA_R160x120 = 0x00, /* QQVGA Resolution   */
    CAMERA_R320x240 = 0x01, /* QVGA Resolution    */
    CAMERA_R320x320 = 0x02, /* 320x320 Resolution */
    CAMERA_RMAX
};

enum CameraStatus : int32_t {
    FAILURE = -1,
    SUCCESS = 0,
    NOT_INITIALIZED = 1,
    UNIMPLEMENTED = 2,
};

class CameraClass {
   public:
    // Note: based on Portenta's API, we do requires that user calls "begin" to
    // set the resolution.
    CameraClass()
        : camera_{coral::micro::CameraTask::GetSingleton()},
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

    // These APIs are intentionally made to be similar to Portenta with slight
    // modifications.
    int begin(uint32_t resolution = CAMERA_R320x320);
    int begin(int32_t width = 320, int32_t height = 320,
              camera::Format fmt = camera::Format::kRgb,
              camera::FilterMethod filter = camera::FilterMethod::kBilinear,
              camera::Rotation rotation = camera::Rotation::k0,
              bool preserve_ratio = false);
    int end();
    int grab(uint8_t* buffer);
    int standby(bool enable);
    int testPattern(bool walking);
    int testPattern(coral::micro::camera::TestPattern pattern);

    // Followings APIs are adds on to extends features that we have and Portenta
    // doesn't.
    int preserveRatio(bool preserve_ratio);
    int format(coral::micro::camera::Format fmt);
    int discardFrames(int num_frames);

    // Unimplemented APIs left over from Portenta.
    int motionDetection(bool enable);
    int motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    int motionDetectionThreshold(uint32_t threshold);
    int motionDetected();
    int framerate(uint32_t framerate);

   private:
    std::unique_ptr<CameraTask> camera_;
    int32_t width_;
    int32_t height_;
    coral::micro::camera::Format format_;
    coral::micro::camera::FilterMethod filter_;
    coral::micro::camera::Rotation rotation_;
    coral::micro::camera::TestPattern test_pattern_;
    bool preserve_ratio_;
    bool initialized_;
};
}  // namespace arduino
}  // namespace coral::micro

extern coral::micro::arduino::CameraClass Camera;

#endif  // CORAL_MICRO_CAMERACLASS_H
