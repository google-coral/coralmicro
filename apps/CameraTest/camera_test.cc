#include "libs/base/gpio.h"
#include "libs/base/httpd.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

#include <cstdio>
#include <cstring>

static uint8_t camera_grayscale[valiant::CameraTask::kWidth * valiant::CameraTask::kHeight] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_grayscale_small[96 * 96] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_rgb[valiant::CameraTask::kWidth * valiant::CameraTask::kHeight * 3] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_rgb_posenet[valiant::posenet::kPosenetWidth * valiant::posenet::kPosenetHeight * 3] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_raw[valiant::CameraTask::kWidth * valiant::CameraTask::kHeight] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

static valiant::HttpServer::Content UriHandler(const char *name) {
    if (std::strcmp(name, "/camera.rgb") == 0)
        return valiant::HttpServer::StaticBuffer{camera_rgb, sizeof(camera_rgb)};

    if (std::strcmp(name, "/camera-posenet.rgb") == 0)
        return valiant::HttpServer::StaticBuffer{camera_rgb_posenet, sizeof(camera_rgb_posenet)};

    if (std::strcmp(name, "/camera.gray") == 0)
        return valiant::HttpServer::StaticBuffer{camera_grayscale, sizeof(camera_grayscale)};

    if (std::strcmp(name, "/camera-small.gray") == 0)
        return valiant::HttpServer::StaticBuffer{camera_grayscale_small, sizeof(camera_grayscale_small)};

    if (std::strcmp(name, "/camera.raw") == 0)
        return valiant::HttpServer::StaticBuffer{camera_raw, sizeof(camera_raw)};

    return {};
}

void GetFrame() {
    valiant::camera::FrameFormat fmt_rgb, fmt_rgb_posenet, fmt_grayscale, fmt_grayscale_small, fmt_raw;
    fmt_rgb.fmt = valiant::camera::Format::RGB;
    fmt_rgb.filter = valiant::camera::FilterMethod::BILINEAR;
    fmt_rgb.width = valiant::CameraTask::kWidth;
    fmt_rgb.height = valiant::CameraTask::kHeight;
    fmt_rgb.preserve_ratio = false;
    fmt_rgb.buffer = camera_rgb;

    fmt_rgb_posenet.fmt = valiant::camera::Format::RGB;
    fmt_rgb_posenet.filter = valiant::camera::FilterMethod::BILINEAR;
    fmt_rgb_posenet.width = valiant::posenet::kPosenetWidth;
    fmt_rgb_posenet.height = valiant::posenet::kPosenetHeight;
    fmt_rgb_posenet.preserve_ratio = false;
    fmt_rgb_posenet.buffer = camera_rgb_posenet;

    fmt_grayscale.fmt = valiant::camera::Format::Y8;
    fmt_grayscale.filter = valiant::camera::FilterMethod::BILINEAR;
    fmt_grayscale.width = valiant::CameraTask::kWidth;
    fmt_grayscale.height = valiant::CameraTask::kHeight;
    fmt_grayscale.preserve_ratio = false;
    fmt_grayscale.buffer = camera_grayscale;

    fmt_grayscale_small.fmt = valiant::camera::Format::Y8;
    fmt_grayscale_small.filter = valiant::camera::FilterMethod::BILINEAR;
    fmt_grayscale_small.width = 96;
    fmt_grayscale_small.height = 96;
    fmt_grayscale_small.preserve_ratio = false;
    fmt_grayscale_small.buffer = camera_grayscale_small;

    fmt_raw.fmt = valiant::camera::Format::RAW;
    fmt_raw.width = valiant::CameraTask::kWidth;
    fmt_raw.height = valiant::CameraTask::kHeight;
    fmt_raw.preserve_ratio = true;
    fmt_raw.buffer = camera_raw;

    valiant::CameraTask::GetFrame({fmt_rgb, fmt_rgb_posenet, fmt_grayscale, fmt_grayscale_small, fmt_raw});
}

extern "C" void app_main(void *param) {
    printf("Camera test\r\n");

    valiant::HttpServer http_server;
    http_server.AddUriHandler(UriHandler);
    valiant::UseHttpServer(&http_server);

    // Enable Power, Streaming, and enable test pattern.
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);
    valiant::CameraTask::GetSingleton()->SetTestPattern(
            valiant::camera::TestPattern::WALKING_ONES);

    // Get and discard some frames to let the sensor stabilize.
    valiant::CameraTask::GetSingleton()->DiscardFrames(100);

    // Get a frame -- compare the result to the expected 'walking ones' test pattern
    uint8_t *buffer = nullptr;
    int index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
    uint8_t expected = 0;
    bool success = true;
    for (unsigned int i = 0; i < valiant::CameraTask::kWidth * valiant::CameraTask::kHeight; ++i) {
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
    valiant::CameraTask::GetSingleton()->ReturnFrame(index);

    valiant::CameraTask::GetSingleton()->SetTestPattern(
            valiant::camera::TestPattern::NONE);
    valiant::CameraTask::GetSingleton()->DiscardFrames(100);
    GetFrame();

    // Switch to triggered mode, the button press will capture and convert a new frame.
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::TRIGGER);

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
    valiant::gpio::RegisterIRQHandler(valiant::gpio::Gpio::kUserButton, [current_task]() {
        static TickType_t last_wake = 0;
        TickType_t now = xTaskGetTickCountFromISR();
        if ((now - last_wake) < pdMS_TO_TICKS(200)) {
            return;
        }
        last_wake = now;
        xTaskResumeFromISR(current_task);
    });
    while (true) {
        valiant::CameraTask::GetSingleton()->Trigger();
        GetFrame();
        vTaskSuspend(NULL);
    }

    // Disable streaming, and turn the power off.
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(NULL);
}
