// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <cstring>

#include "libs/base/gpio.h"
#include "libs/base/http_server.h"
#include "libs/camera/camera.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

namespace coral::micro {
namespace {
uint8_t camera_grayscale[CameraTask::kWidth * CameraTask::kHeight]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_grayscale_small[96 * 96]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb[CameraTask::kWidth * CameraTask::kHeight * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb_90[coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb_180[coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb_270[coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb_posenet[324 * 324 * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_raw[CameraTask::kWidth * CameraTask::kHeight]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

HttpServer::Content UriHandler(const char* name) {
    if (std::strcmp(name, "/camera.rgb") == 0)
        return HttpServer::StaticBuffer{camera_rgb, sizeof(camera_rgb)};

    if (std::strcmp(name, "/camera-90.rgb") == 0)
        return HttpServer::StaticBuffer{camera_rgb_90, sizeof(camera_rgb_90)};

    if (std::strcmp(name, "/camera-180.rgb") == 0)
        return HttpServer::StaticBuffer{camera_rgb_180, sizeof(camera_rgb_180)};

    if (std::strcmp(name, "/camera-270.rgb") == 0)
        return HttpServer::StaticBuffer{camera_rgb_270, sizeof(camera_rgb_270)};

    if (std::strcmp(name, "/camera-posenet.rgb") == 0)
        return HttpServer::StaticBuffer{camera_rgb_posenet,
                                        sizeof(camera_rgb_posenet)};

    if (std::strcmp(name, "/camera.gray") == 0)
        return HttpServer::StaticBuffer{camera_grayscale,
                                        sizeof(camera_grayscale)};

    if (std::strcmp(name, "/camera-small.gray") == 0)
        return HttpServer::StaticBuffer{camera_grayscale_small,
                                        sizeof(camera_grayscale_small)};

    if (std::strcmp(name, "/camera.raw") == 0)
        return HttpServer::StaticBuffer{camera_raw, sizeof(camera_raw)};

    return {};
}

void GetFrame() {
    camera::FrameFormat fmt_rgb, fmt_rgb_90, fmt_rgb_180, fmt_rgb_270,
        fmt_rgb_posenet, fmt_grayscale,
        fmt_grayscale_small, fmt_raw;
    fmt_rgb.fmt = camera::Format::RGB;
    fmt_rgb.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb.rotation = camera::Rotation::k0;
    fmt_rgb.width = CameraTask::kWidth;
    fmt_rgb.height = CameraTask::kHeight;
    fmt_rgb.preserve_ratio = false;
    fmt_rgb.buffer = camera_rgb;

    fmt_rgb_90.fmt = camera::Format::RGB;
    fmt_rgb_90.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb_90.rotation = camera::Rotation::k90;
    fmt_rgb_90.width = CameraTask::kWidth;
    fmt_rgb_90.height = CameraTask::kHeight;
    fmt_rgb_90.preserve_ratio = false;
    fmt_rgb_90.buffer = camera_rgb_90;

    fmt_rgb_180.fmt = camera::Format::RGB;
    fmt_rgb_180.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb_180.rotation = camera::Rotation::k180;
    fmt_rgb_180.width = CameraTask::kWidth;
    fmt_rgb_180.height = CameraTask::kHeight;
    fmt_rgb_180.preserve_ratio = false;
    fmt_rgb_180.buffer = camera_rgb_180;

    fmt_rgb_270.fmt = camera::Format::RGB;
    fmt_rgb_270.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb_270.rotation = camera::Rotation::k270;
    fmt_rgb_270.width = CameraTask::kWidth;
    fmt_rgb_270.height = CameraTask::kHeight;
    fmt_rgb_270.preserve_ratio = false;
    fmt_rgb_270.buffer = camera_rgb_270;

    fmt_rgb_posenet.fmt = camera::Format::RGB;
    fmt_rgb_posenet.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb_posenet.width = 324;
    fmt_rgb_posenet.height = 324;
    fmt_rgb_posenet.preserve_ratio = false;
    fmt_rgb_posenet.buffer = camera_rgb_posenet;

    fmt_grayscale.fmt = camera::Format::Y8;
    fmt_grayscale.filter = camera::FilterMethod::BILINEAR;
    fmt_grayscale.width = CameraTask::kWidth;
    fmt_grayscale.height = CameraTask::kHeight;
    fmt_grayscale.preserve_ratio = false;
    fmt_grayscale.buffer = camera_grayscale;

    fmt_grayscale_small.fmt = camera::Format::Y8;
    fmt_grayscale_small.filter = camera::FilterMethod::BILINEAR;
    fmt_grayscale_small.width = 96;
    fmt_grayscale_small.height = 96;
    fmt_grayscale_small.preserve_ratio = false;
    fmt_grayscale_small.buffer = camera_grayscale_small;

    fmt_raw.fmt = camera::Format::RAW;
    fmt_raw.width = CameraTask::kWidth;
    fmt_raw.height = CameraTask::kHeight;
    fmt_raw.preserve_ratio = true;
    fmt_raw.buffer = camera_raw;

    CameraTask::GetFrame({fmt_rgb, fmt_rgb_90, fmt_rgb_180, fmt_rgb_270,
                          fmt_rgb_posenet, fmt_grayscale,
                          fmt_grayscale_small, fmt_raw});
}

void Main() {
    printf("Camera test\r\n");

    HttpServer http_server;
    http_server.AddUriHandler(UriHandler);
    UseHttpServer(&http_server);

    // Enable Power, Streaming, and enable test pattern.
    CameraTask::GetSingleton()->SetPower(true);
    CameraTask::GetSingleton()->Enable(camera::Mode::STREAMING);
    CameraTask::GetSingleton()->SetTestPattern(
        camera::TestPattern::WALKING_ONES);

    // Get and discard some frames to let the sensor stabilize.
    CameraTask::GetSingleton()->DiscardFrames(100);

    // Get a frame -- compare the result to the expected 'walking ones' test
    // pattern
    uint8_t* buffer = nullptr;
    int index = CameraTask::GetSingleton()->GetFrame(&buffer, true);
    uint8_t expected = 0;
    bool success = true;
    for (unsigned int i = 0; i < CameraTask::kWidth * CameraTask::kHeight;
         ++i) {
        if (buffer[i] != expected) {
            printf("Test pattern mismatch at index %d! 0x%x != 0x%x\r\n", i,
                   buffer[i], expected);
            success = false;
            break;
        }
        if (expected == 0) {
            expected = 1;
        } else {
            expected = expected << 1;
        }
    }
    if (success) {
        printf("Camera test pattern verified successfully!\r\n");
    }
    CameraTask::GetSingleton()->ReturnFrame(index);

    CameraTask::GetSingleton()->SetTestPattern(camera::TestPattern::NONE);
    CameraTask::GetSingleton()->DiscardFrames(100);
    GetFrame();

    // Switch to triggered mode, the button press will capture and convert a new
    // frame.
    CameraTask::GetSingleton()->Disable();
    CameraTask::GetSingleton()->Enable(camera::Mode::TRIGGER);

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    gpio::RegisterIRQHandler(gpio::Gpio::kUserButton, [current_task]() {
        static TickType_t last_wake = 0;
        TickType_t now = xTaskGetTickCountFromISR();
        if ((now - last_wake) < pdMS_TO_TICKS(200)) {
            return;
        }
        last_wake = now;
        xTaskResumeFromISR(current_task);
    });
    while (true) {
        CameraTask::GetSingleton()->Trigger();
        GetFrame();
        vTaskSuspend(nullptr);
    }

    // Disable streaming, and turn the power off.
    CameraTask::GetSingleton()->Disable();
    CameraTask::GetSingleton()->SetPower(false);
}
}  // namespace
}  // namespace coral::micro

extern "C" void app_main(void* param) {
    (void)param;
    coral::micro::Main();
    vTaskSuspend(nullptr);
}
