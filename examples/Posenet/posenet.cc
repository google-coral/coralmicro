#include "libs/posenet/posenet.h"

#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"

// Runs pose estimation using PoseNet, running on the Edge TPU.
// Scores and keypoint data is printed to the serial console.

extern "C" [[noreturn]] void app_main(void* param) {
    coral::micro::posenet::Output output{};
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    vTaskDelay(pdMS_TO_TICKS(500));
    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(
        coral::micro::camera::Mode::STREAMING);
    auto tpu_context = coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice(
        coral::micro::PerformanceMode::kMax);
    if (!coral::micro::posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }
    coral::micro::posenet::loop(&output);
    printf("Posenet static datatest finished.\r\n");

    coral::micro::CameraTask::GetSingleton()->DiscardFrames(100);

    while (true) {
        auto* input = coral::micro::posenet::input();
        coral::micro::camera::FrameFormat fmt{};
        fmt.width = input->dims->data[2];
        fmt.height = input->dims->data[1];
        fmt.fmt = coral::micro::camera::Format::RGB;
        fmt.filter = coral::micro::camera::FilterMethod::BILINEAR;
        fmt.preserve_ratio = false;
        fmt.buffer = tflite::GetTensorData<uint8_t>(input);
        coral::micro::CameraTask::GetFrame({fmt});
        coral::micro::posenet::loop(&output);
    }
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
    vTaskSuspend(nullptr);
}
