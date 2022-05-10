#include "camera.h"

#include <cstdio>

namespace coral::micro {
namespace arduino {

int CameraClass::begin(uint32_t resolution) {
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

int CameraClass::begin(int32_t width, int32_t height, camera::Format fmt,
                       camera::FilterMethod filter, camera::Rotation rotation,
                       bool preserve_ratio) {
    initialized_ = true;
    width_ = width;
    height_ = height;
    format_ = fmt;
    filter_ = filter;
    rotation_ = rotation;
    preserve_ratio_ = preserve_ratio;
    camera_->SetPower(true);
    camera_->Enable(coral::micro::camera::Mode::STREAMING);
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
    std::list<coral::micro::camera::FrameFormat> fmts;
    if (test_pattern_ != camera::TestPattern::NONE) {
        fmts.push_back({camera::Format::RAW, camera::FilterMethod::BILINEAR,
                        camera::Rotation::k0,
                        CameraTask::kWidth, CameraTask::kHeight,
                        preserve_ratio_, buffer});
    } else {
        fmts.push_back(
            {format_, filter_, rotation_, width_, height_, preserve_ratio_, buffer});
    }

    auto success = coral::micro::CameraTask::GetFrame(fmts);
    if (!success) {
        printf("Failed to get frame from camera\r\n");
        return CameraStatus::FAILURE;
    }
    return CameraStatus::SUCCESS;
}

int CameraClass::testPattern(bool walking) {
    auto test_pattern = walking ? coral::micro::camera::TestPattern::WALKING_ONES
                                : coral::micro::camera::TestPattern::NONE;
    test_pattern_ = test_pattern;
    camera_->SetTestPattern(test_pattern_);
    return CameraStatus::SUCCESS;
}

int CameraClass::testPattern(coral::micro::camera::TestPattern pattern) {
    test_pattern_ = pattern;
    camera_->SetTestPattern(test_pattern_);
    return CameraStatus::SUCCESS;
}

int CameraClass::standby(bool enable) {
    if (enable) {
        camera_->Enable(coral::micro::camera::Mode::STREAMING);
    } else {
        camera_->Enable(coral::micro::camera::Mode::STANDBY);
    }
    return CameraStatus::SUCCESS;
}

int CameraClass::preserveRatio(bool preserve_ratio) {
    preserve_ratio_ = preserve_ratio;
    return CameraStatus::SUCCESS;
}

int CameraClass::format(coral::micro::camera::Format fmt) {
    format_ = fmt;
    return CameraStatus::SUCCESS;
}

int CameraClass::discardFrames(int num_frames) {
    camera_->DiscardFrames(num_frames);
    return CameraStatus::SUCCESS;
}

int CameraClass::motionDetection(bool enable) {
    printf("%s not implemented\n", __func__);
    return CameraStatus::UNIMPLEMENTED;
}

int CameraClass::motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w,
                                       uint32_t h) {
    printf("%s not implemented\n", __func__);
    return CameraStatus::UNIMPLEMENTED;
}

int CameraClass::motionDetectionThreshold(uint32_t threshold) {
    printf("%s not implemented\n", __func__);
    return CameraStatus::UNIMPLEMENTED;
}

int CameraClass::motionDetected() {
    printf("%s not implemented\n", __func__);
    return CameraStatus::UNIMPLEMENTED;
}

int CameraClass::framerate(uint32_t framerate) {
    printf("%s not implemented\n", __func__);
    return CameraStatus::UNIMPLEMENTED;
}

}  // namespace arduino
}  // namespace coral::micro

coral::micro::arduino::CameraClass Camera = coral::micro::arduino::CameraClass();
