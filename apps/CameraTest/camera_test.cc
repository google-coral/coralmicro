#include "libs/base/tasks_m7.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_lpi2c_freertos.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/cm7/fsl_cache.h"

#include <cstdio>

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
    for (int i = 0; i < 2; ++i) {
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

    // Disable streaming, and turn the power off.
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);

    while (true) {
        taskYIELD();
    }
}
