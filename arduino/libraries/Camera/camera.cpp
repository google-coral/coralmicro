#include "camera.h"
#include <cstdio>

namespace valiant {
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

int CameraClass::begin(int32_t width, int32_t height, camera::Format fmt, bool preserve_ratio) {
    initialized_ = true;
    width_ = width;
    height_ = height;
    format_ = fmt;
    preserve_ratio_ = preserve_ratio;
    camera_->SetPower(true);
    camera_->Enable(valiant::camera::Mode::STREAMING);
    return CameraStatus::SUCCESS;
}

int CameraClass::grab(uint8_t* buffer) {
    if (!initialized_) {
        printf("%s Camera is not initialized...\n", __func__);
        return CameraStatus::NOT_INITIALIZED;
    }
    std::list<valiant::camera::FrameFormat> fmts;
    if (test_pattern_ != camera::TestPattern::NONE) {
        fmts.push_back({camera::Format::RAW,
                        CameraTask::kWidth,
                        CameraTask::kHeight,
                        preserve_ratio_,
                        buffer});
    } else {
        fmts.push_back({format_,
                        width_,
                        height_,
                        preserve_ratio_,
                        buffer});
    }

    auto success = valiant::CameraTask::GetFrame(fmts);
    if (!success) {
        printf("Failed to get frame from camera\r\n");
        return CameraStatus::FAILURE;
    }
    return CameraStatus::SUCCESS;
}

int CameraClass::testPattern(bool walking) {
    auto test_pattern = walking ? valiant::camera::TestPattern::WALKING_ONES
                                : valiant::camera::TestPattern::NONE;
    test_pattern_ = test_pattern;
    camera_->SetTestPattern(test_pattern_);
    return CameraStatus::SUCCESS;
}

int CameraClass::testPattern(valiant::camera::TestPattern pattern) {
    test_pattern_ = pattern;
    camera_->SetTestPattern(test_pattern_);
    return CameraStatus::SUCCESS;
}

int CameraClass::standby(bool enable) {
    if (enable) {
        camera_->Enable(valiant::camera::Mode::STREAMING);
    } else {
        camera_->Enable(valiant::camera::Mode::STANDBY);
    }
    return CameraStatus::SUCCESS;
}

int CameraClass::preserveRatio(bool preserve_ratio) {
    preserve_ratio_ = preserve_ratio;
    return CameraStatus::SUCCESS;
}

int CameraClass::format(valiant::camera::Format fmt) {
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

int CameraClass::motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
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

} // namespace arduino
} // namespace valiant
