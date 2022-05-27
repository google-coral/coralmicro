#include <cstdio>
#include <cstring>

#include "libs/base/gpio.h"
#include "libs/base/http_server.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

namespace coral::micro {
namespace {
uint8_t camera_grayscale[CameraTask::kWidth * CameraTask::kHeight]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_grayscale_small[96 * 96]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb[CameraTask::kWidth * CameraTask::kHeight * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_rgb_posenet[posenet::kPosenetWidth * posenet::kPosenetHeight * 3]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
uint8_t camera_raw[CameraTask::kWidth * CameraTask::kHeight]
    __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

HttpServer::Content UriHandler(const char* name) {
    if (std::strcmp(name, "/camera.rgb") == 0)
        return HttpServer::StaticBuffer{camera_rgb, sizeof(camera_rgb)};

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
    camera::FrameFormat fmt_rgb, fmt_rgb_posenet, fmt_grayscale,
        fmt_grayscale_small, fmt_raw;
    fmt_rgb.fmt = camera::Format::RGB;
    fmt_rgb.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb.width = CameraTask::kWidth;
    fmt_rgb.height = CameraTask::kHeight;
    fmt_rgb.preserve_ratio = false;
    fmt_rgb.buffer = camera_rgb;

    fmt_rgb_posenet.fmt = camera::Format::RGB;
    fmt_rgb_posenet.filter = camera::FilterMethod::BILINEAR;
    fmt_rgb_posenet.width = posenet::kPosenetWidth;
    fmt_rgb_posenet.height = posenet::kPosenetHeight;
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

    CameraTask::GetFrame({fmt_rgb, fmt_rgb_posenet, fmt_grayscale,
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