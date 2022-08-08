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

#include "coralmicro_camera.h"

#include <cstdio>
#include <vector>

namespace coralmicro {
namespace arduino {

CameraClass::CameraClass()
    : camera_{coralmicro::CameraTask::GetSingleton()},
      width_{0},
      height_{0},
      format_{CameraFormat::kRgb},
      filter_{CameraFilterMethod::kBilinear},
      test_pattern_{CameraTestPattern::kNone},
      preserve_ratio_{false},
      auto_white_balance_{false},
      initialized_{false} {
  coralmicro::CameraTask::GetSingleton()->GetMotionDetectionConfigDefault(
      md_config_);
}

int CameraClass::begin(CameraResolution resolution) {
  switch (resolution) {
    case CAMERA_R160x120:
      return begin(160, 120);
    case CAMERA_R320x240:
      return begin(320, 240);
    case CAMERA_R320x320:
    case CAMERA_RMAX:
    default:
      return begin(320, 320);
  }
}

int CameraClass::begin(int32_t width, int32_t height, CameraFormat fmt,
                       CameraFilterMethod filter, CameraRotation rotation,
                       bool preserve_ratio, bool auto_white_balance) {
  initialized_ = true;
  width_ = width;
  height_ = height;
  format_ = fmt;
  filter_ = filter;
  rotation_ = rotation;
  preserve_ratio_ = preserve_ratio;
  auto_white_balance_ = auto_white_balance;
  camera_->SetPower(true);
  camera_->Enable(coralmicro::CameraMode::kStreaming);
  return CameraStatus::SUCCESS;
}

int CameraClass::end() {
  camera_->Disable();
  camera_->SetPower(false);
  initialized_ = false;
}

int CameraClass::grab(uint8_t* buffer) {
  if (!initialized_) {
    printf("%s Camera is not initialized...\n", __func__);
    return CameraStatus::NOT_INITIALIZED;
  }
  std::vector<coralmicro::CameraFrameFormat> fmts;
  if (test_pattern_ != CameraTestPattern::kNone) {
    fmts.push_back({CameraFormat::kRaw, CameraFilterMethod::kBilinear,
                    CameraRotation::k0, CameraTask::kWidth, CameraTask::kHeight,
                    preserve_ratio_, buffer, auto_white_balance_});
  } else {
    fmts.push_back({format_, filter_, rotation_, width_, height_,
                    preserve_ratio_, buffer, auto_white_balance_});
  }

  auto success = coralmicro::CameraTask::GetSingleton()->GetFrame(fmts);
  if (!success) {
    printf("Failed to get frame from camera\r\n");
    return CameraStatus::FAILURE;
  }
  return CameraStatus::SUCCESS;
}

int CameraClass::testPattern(bool walking) {
  test_pattern_ = walking ? coralmicro::CameraTestPattern::kWalkingOnes
                          : coralmicro::CameraTestPattern::kNone;
  camera_->SetTestPattern(test_pattern_);
  return CameraStatus::SUCCESS;
}

int CameraClass::testPattern(coralmicro::CameraTestPattern pattern) {
  test_pattern_ = pattern;
  camera_->SetTestPattern(test_pattern_);
  return CameraStatus::SUCCESS;
}

int CameraClass::standby(bool enable) {
  if (enable) {
    camera_->Enable(coralmicro::CameraMode::kStandBy);
  } else {
    camera_->Enable(coralmicro::CameraMode::kStreaming);
  }
  return CameraStatus::SUCCESS;
}

int CameraClass::preserveRatio(bool preserve_ratio) {
  preserve_ratio_ = preserve_ratio;
  return CameraStatus::SUCCESS;
}

int CameraClass::format(coralmicro::CameraFormat fmt) {
  format_ = fmt;
  return CameraStatus::SUCCESS;
}

int CameraClass::discardFrames(int num_frames) {
  camera_->DiscardFrames(num_frames);
  return CameraStatus::SUCCESS;
}

int CameraClass::motionDetection(bool enable, md_callback_t callback,
                                 void* cb_param) {
  if (!initialized_) {
    printf("%s Camera is not initialized...\n", __func__);
    return CameraStatus::NOT_INITIALIZED;
  }
  if (!callback) {
    printf("%s motion detection callback cannot be null\n", __func__);
    return CameraStatus::FAILURE;
  }
  md_config_.enable = enable;
  md_config_.cb = callback;
  if (cb_param) md_config_.cb_param = cb_param;
  CameraTask::GetSingleton()->SetMotionDetectionConfig(md_config_);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);
  return CameraStatus::SUCCESS;
}

int CameraClass::motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w,
                                       uint32_t h) {
  if (x > width_ || (x + y) > width_ || y > height_ || (y + h) > height_) {
    printf("Motion Detection window is out of bound\r\n");
    return CameraStatus::FAILURE;
  }
  md_config_.x0 = x;
  md_config_.x1 = x + w;
  md_config_.y0 = y;
  md_config_.y1 = y + h;
  return CameraStatus::SUCCESS;
}

int CameraClass::motionDetected() { return CameraStatus::SUCCESS; }

int CameraClass::motionDetectionThreshold(uint32_t threshold) {
  printf("%s not implemented\n", __func__);
  return CameraStatus::UNIMPLEMENTED;
}

int CameraClass::framerate(uint32_t framerate) {
  printf("%s not implemented\n", __func__);
  return CameraStatus::UNIMPLEMENTED;
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::CameraClass Camera = coralmicro::arduino::CameraClass();
