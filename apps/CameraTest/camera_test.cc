#include "libs/base/tasks_m7.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"

#include <cstdio>

constexpr const int kPosenetWidth = 481;
constexpr const int kPosenetHeight = 353;
static uint8_t camera_grayscale[valiant::CameraTask::kWidth * valiant::CameraTask::kHeight] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_grayscale_small[96 * 96] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_rgb[valiant::CameraTask::kWidth * valiant::CameraTask::kHeight * 3] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));
static uint8_t camera_rgb_posenet[kPosenetWidth * kPosenetHeight * 3] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

extern "C" int fs_open_custom(struct fs_file *file, const char *name) {
    if (strncmp(name, "/camera.rgb", strlen(name)) == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data = reinterpret_cast<const char*>(camera_rgb);
        file->len = sizeof(camera_rgb);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    if (strncmp(name, "/camera-posenet.rgb", strlen(name)) == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data = reinterpret_cast<const char*>(camera_rgb_posenet);
        file->len = sizeof(camera_rgb_posenet);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    if (strncmp(name, "/camera.gray", strlen(name)) == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data = reinterpret_cast<const char*>(camera_grayscale);
        file->len = sizeof(camera_grayscale);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    if (strncmp(name, "/camera-small.gray", strlen(name)) == 0) {
        memset(file, 0, sizeof(struct fs_file));
        file->data = reinterpret_cast<const char*>(camera_grayscale_small);
        file->len = sizeof(camera_grayscale_small);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
        return 1;
    }
    return 0;
}

extern "C" void fs_close_custom(struct fs_file *file) {
    (void)file;
}

extern "C" void app_main(void *param) {
    printf("Camera test\r\n");

    // Enable Power, Streaming, and enable test pattern.
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable();
    valiant::CameraTask::GetSingleton()->SetTestPattern(
            valiant::camera::TestPattern::WALKING_ONES);

    // Get and discard some frames to let the sensor stabilize.
    uint8_t *buffer = nullptr;
    int index = -1;
    for (int i = 0; i < 100; ++i) {
        index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
        valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    }

    // Get a frame -- compare the result to the expected 'walking ones' test pattern
    index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
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
    for (int i = 0; i < 100; ++i) {
        index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
        valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    }
    valiant::camera::FrameFormat fmt;
    fmt.fmt = valiant::camera::Format::RGB;
    fmt.width = valiant::CameraTask::kWidth;
    fmt.height = valiant::CameraTask::kHeight;
    fmt.preserve_ratio = false;
    valiant::CameraTask::GetFrame(fmt, camera_rgb);

    fmt.fmt = valiant::camera::Format::RGB;
    fmt.width = kPosenetWidth;
    fmt.height = kPosenetHeight;
    fmt.preserve_ratio = false;
    valiant::CameraTask::GetFrame(fmt, camera_rgb_posenet);

    fmt.fmt = valiant::camera::Format::Y8;
    fmt.width = valiant::CameraTask::kWidth;
    fmt.height = valiant::CameraTask::kHeight;
    fmt.preserve_ratio = false;
    valiant::CameraTask::GetFrame(fmt, camera_grayscale);

    fmt.fmt = valiant::camera::Format::Y8;
    fmt.width = 96;
    fmt.height = 96;
    fmt.preserve_ratio = false;
    valiant::CameraTask::GetFrame(fmt, camera_grayscale_small);

    // Disable streaming, and turn the power off.
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(NULL);
}
