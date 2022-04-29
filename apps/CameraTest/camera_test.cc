#include "libs/base/gpio.h"
#include "libs/base/http_server.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#include <cstdio>
#include <cstring>

static uint8_t camera_grayscale[coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_grayscale_small[96 * 96] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_rgb[coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight * 3] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_rgb_posenet[coral::micro::posenet::kPosenetWidth * coral::micro::posenet::kPosenetHeight * 3] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_raw[coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

static coral::micro::HttpServer::Content UriHandler(const char *name) {
    if (std::strcmp(name, "/camera.rgb") == 0)
        return coral::micro::HttpServer::StaticBuffer{camera_rgb, sizeof(camera_rgb)};

    if (std::strcmp(name, "/camera-posenet.rgb") == 0)
        return coral::micro::HttpServer::StaticBuffer{camera_rgb_posenet, sizeof(camera_rgb_posenet)};

    if (std::strcmp(name, "/camera.gray") == 0)
        return coral::micro::HttpServer::StaticBuffer{camera_grayscale, sizeof(camera_grayscale)};

    if (std::strcmp(name, "/camera-small.gray") == 0)
        return coral::micro::HttpServer::StaticBuffer{camera_grayscale_small, sizeof(camera_grayscale_small)};

    if (std::strcmp(name, "/camera.raw") == 0)
        return coral::micro::HttpServer::StaticBuffer{camera_raw, sizeof(camera_raw)};

    return {};
}

void GetFrame() {
    coral::micro::camera::FrameFormat fmt_rgb, fmt_rgb_posenet, fmt_grayscale, fmt_grayscale_small, fmt_raw;
    fmt_rgb.fmt = coral::micro::camera::Format::RGB;
    fmt_rgb.filter = coral::micro::camera::FilterMethod::BILINEAR;
    fmt_rgb.width = coral::micro::CameraTask::kWidth;
    fmt_rgb.height = coral::micro::CameraTask::kHeight;
    fmt_rgb.preserve_ratio = false;
    fmt_rgb.buffer = camera_rgb;

    fmt_rgb_posenet.fmt = coral::micro::camera::Format::RGB;
    fmt_rgb_posenet.filter = coral::micro::camera::FilterMethod::BILINEAR;
    fmt_rgb_posenet.width = coral::micro::posenet::kPosenetWidth;
    fmt_rgb_posenet.height = coral::micro::posenet::kPosenetHeight;
    fmt_rgb_posenet.preserve_ratio = false;
    fmt_rgb_posenet.buffer = camera_rgb_posenet;

    fmt_grayscale.fmt = coral::micro::camera::Format::Y8;
    fmt_grayscale.filter = coral::micro::camera::FilterMethod::BILINEAR;
    fmt_grayscale.width = coral::micro::CameraTask::kWidth;
    fmt_grayscale.height = coral::micro::CameraTask::kHeight;
    fmt_grayscale.preserve_ratio = false;
    fmt_grayscale.buffer = camera_grayscale;

    fmt_grayscale_small.fmt = coral::micro::camera::Format::Y8;
    fmt_grayscale_small.filter = coral::micro::camera::FilterMethod::BILINEAR;
    fmt_grayscale_small.width = 96;
    fmt_grayscale_small.height = 96;
    fmt_grayscale_small.preserve_ratio = false;
    fmt_grayscale_small.buffer = camera_grayscale_small;

    fmt_raw.fmt = coral::micro::camera::Format::RAW;
    fmt_raw.width = coral::micro::CameraTask::kWidth;
    fmt_raw.height = coral::micro::CameraTask::kHeight;
    fmt_raw.preserve_ratio = true;
    fmt_raw.buffer = camera_raw;

    coral::micro::CameraTask::GetFrame({fmt_rgb, fmt_rgb_posenet, fmt_grayscale, fmt_grayscale_small, fmt_raw});
}

extern "C" void app_main(void *param) {
    printf("Camera test\r\n");

    coral::micro::HttpServer http_server;
    http_server.AddUriHandler(UriHandler);
    coral::micro::UseHttpServer(&http_server);

    // Enable Power, Streaming, and enable test pattern.
    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(coral::micro::camera::Mode::STREAMING);
    coral::micro::CameraTask::GetSingleton()->SetTestPattern(
            coral::micro::camera::TestPattern::WALKING_ONES);

    // Get and discard some frames to let the sensor stabilize.
    coral::micro::CameraTask::GetSingleton()->DiscardFrames(100);

    // Get a frame -- compare the result to the expected 'walking ones' test pattern
    uint8_t *buffer = nullptr;
    int index = coral::micro::CameraTask::GetSingleton()->GetFrame(&buffer, true);
    uint8_t expected = 0;
    bool success = true;
    for (unsigned int i = 0; i < coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight; ++i) {
        if (buffer[i] != expected) {
            printf("Test pattern mismatch at index %d! 0x%x != 0x%x\r\n", i, buffer[i], expected);
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
    coral::micro::CameraTask::GetSingleton()->ReturnFrame(index);

    coral::micro::CameraTask::GetSingleton()->SetTestPattern(
            coral::micro::camera::TestPattern::NONE);
    coral::micro::CameraTask::GetSingleton()->DiscardFrames(100);
    GetFrame();

    // Switch to triggered mode, the button press will capture and convert a new frame.
    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->Enable(coral::micro::camera::Mode::TRIGGER);

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    coral::micro::gpio::RegisterIRQHandler(coral::micro::gpio::Gpio::kUserButton, [current_task]() {
        static TickType_t last_wake = 0;
        TickType_t now = xTaskGetTickCountFromISR();
        if ((now - last_wake) < pdMS_TO_TICKS(200)) {
            return;
        }
        last_wake = now;
        xTaskResumeFromISR(current_task);
    });
    while (true) {
        coral::micro::CameraTask::GetSingleton()->Trigger();
        GetFrame();
        vTaskSuspend(NULL);
    }

    // Disable streaming, and turn the power off.
    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(NULL);
}
