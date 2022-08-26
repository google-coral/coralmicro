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

FrameBuffer::FrameBuffer() : fb_size_{0}, fb_{nullptr} {}

FrameBuffer::~FrameBuffer() { delete[] fb_; }

uint32_t FrameBuffer::getBufferSize() { return fb_size_; }

uint8_t* FrameBuffer::getBuffer() { return fb_; }

bool FrameBuffer::hasFixedSize() {
  if (fb_size_) return true;
  return false;
}

void FrameBuffer::setBuffer(uint8_t* buffer, uint32_t frame_size) {
  fb_ = buffer;
  fb_size_ = frame_size;
}

bool FrameBuffer::isAllocated() { return fb_ != nullptr; }

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
      return begin(320, 320);
    case CAMERA_R324x324:
    case CAMERA_RMAX:
    default:
      return begin(324, 324);
  }
}

int CameraClass::begin(int32_t width, int32_t height, CameraFormat fmt,
                       CameraFilterMethod filter, CameraRotation rotation,
                       bool preserve_ratio, bool auto_white_balance) {
  width_ = width;
  height_ = height;
  format_ = fmt;
  filter_ = filter;
  rotation_ = rotation;
  preserve_ratio_ = preserve_ratio;
  auto_white_balance_ = auto_white_balance;
  if (!initialized_) {
    camera_->SetPower(true);
    camera_->Enable(coralmicro::CameraMode::kStreaming);
  }
  initialized_ = true;
  return CameraStatus::SUCCESS;
}

int CameraClass::grab(FrameBuffer& buffer) {
  if (!initialized_) {
    printf("%s Camera is not initialized...\n", __func__);
    return CameraStatus::NOT_INITIALIZED;
  }
  auto frame_size = width_ * height_;
  if (test_pattern_ == CameraTestPattern::kNone)
    frame_size *= CameraFormatBpp(format_);
  if (buffer.isAllocated()) {
    if (buffer.getBufferSize() < frame_size) {
      printf("%s Buffer size not big enough to hold frame...\r\n", __func__);
      return CameraStatus::FAILURE;
    }
  } else {  // Allocate new buffer.
    auto* buf = new uint8_t[frame_size];
    buffer.setBuffer(buf, frame_size);
  }
  std::vector<coralmicro::CameraFrameFormat> fmts;
  if (test_pattern_ != CameraTestPattern::kNone) {
    fmts.push_back({CameraFormat::kRaw, CameraFilterMethod::kBilinear,
                    CameraRotation::k0, CameraTask::kWidth, CameraTask::kHeight,
                    preserve_ratio_, buffer.getBuffer(), auto_white_balance_});
  } else {
    fmts.push_back({format_, filter_, rotation_, width_, height_,
                    preserve_ratio_, buffer.getBuffer(), auto_white_balance_});
  }

  auto success = coralmicro::CameraTask::GetSingleton()->GetFrame(fmts);
  if (!success) {
    printf("Failed to get frame from camera\r\n");
    return CameraStatus::FAILURE;
  }
  return CameraStatus::SUCCESS;
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

int CameraClass::setTestPattern(bool enable, bool walking) {
  test_pattern_ = walking ? coralmicro::CameraTestPattern::kWalkingOnes
                          : coralmicro::CameraTestPattern::kNone;
  if (enable) {
    camera_->SetTestPattern(test_pattern_);
  }
  return CameraStatus::SUCCESS;
}

int CameraClass::setTestPattern(bool enable,
                                coralmicro::CameraTestPattern pattern) {
  test_pattern_ = pattern;
  if (enable) {
    camera_->SetTestPattern(test_pattern_);
  }
  return CameraStatus::SUCCESS;
}

int CameraClass::setStandby(bool enable) {
  if (enable) {
    camera_->Disable();
  } else {
    camera_->Enable(coralmicro::CameraMode::kStreaming);
  }
  return CameraStatus::SUCCESS;
}

int CameraClass::setPreserveRatio(bool preserve_ratio) {
  preserve_ratio_ = preserve_ratio;
  return CameraStatus::SUCCESS;
}

int CameraClass::setPixelFormat(coralmicro::CameraFormat fmt) {
  format_ = fmt;
  return CameraStatus::SUCCESS;
}

int CameraClass::discardFrames(int num_frames) {
  camera_->DiscardFrames(num_frames);
  return CameraStatus::SUCCESS;
}

int CameraClass::enableMotionDetection(md_callback_t callback, void* cb_param) {
  if (!initialized_) {
    printf("%s Camera is not initialized...\n", __func__);
    return CameraStatus::NOT_INITIALIZED;
  }
  if (!callback) {
    printf("%s motion detection callback cannot be null\n", __func__);
    return CameraStatus::FAILURE;
  }
  md_config_.enable = true;
  md_config_.cb = callback;
  if (cb_param) md_config_.cb_param = cb_param;
  CameraTask::GetSingleton()->SetMotionDetectionConfig(md_config_);
  CameraTask::GetSingleton()->Enable(CameraMode::kStreaming);
  return CameraStatus::SUCCESS;
}

int CameraClass::disableMotionDetection() {
  md_config_.enable = false;
  CameraTask::GetSingleton()->SetMotionDetectionConfig(md_config_);
  return CameraStatus::SUCCESS;
}

int CameraClass::setMotionDetectionWindow(uint32_t x, uint32_t y, uint32_t w,
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

int CameraClass::setMotionDetected() { return CameraStatus::SUCCESS; }

int CameraClass::setMotionDetectionThreshold(uint32_t threshold) {
  (void)threshold;
  printf("%s not implemented\n", __func__);
  return CameraStatus::UNIMPLEMENTED;
}

int CameraClass::setFramerate(uint32_t framerate) {
  (void)framerate;
  printf("%s not implemented\n", __func__);
  return CameraStatus::UNIMPLEMENTED;
}

}  // namespace arduino
}  // namespace coralmicro

coralmicro::arduino::CameraClass Camera = coralmicro::arduino::CameraClass();
