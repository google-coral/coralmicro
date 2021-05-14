#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"

extern "C" void app_main(void *param) {
    valiant::posenet::Output output;
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(500));
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable();
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    if (!valiant::posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }
    valiant::posenet::loop(&output);
    printf("Posenet static datatest finished.\r\n");

    uint8_t *buffer = nullptr;
    int index = -1;
    for (int i = 0; i < 100; ++i) {
        index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
        valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    }

    while (true) {
        TfLiteTensor *input = valiant::posenet::input();
        valiant::camera::FrameFormat fmt;
        fmt.width = input->dims->data[2];
        fmt.height = input->dims->data[1];
        fmt.fmt = valiant::camera::Format::RGB;
        fmt.preserve_ratio = false;
        valiant::CameraTask::GetFrame(fmt, tflite::GetTensorData<uint8_t>(input));
        valiant::posenet::loop(&output);
    }
    valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
    valiant::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(NULL);
}
