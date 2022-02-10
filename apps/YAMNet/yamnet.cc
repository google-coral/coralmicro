#include "libs/base/filesystem.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tensorflow/classification.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/YAMNet/yamnet.h"

extern "C" [[noreturn]] void app_main(void *param) {
    std::shared_ptr<std::vector<valiant::tensorflow::Class>> output = nullptr;
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);

    size_t yamnet_test_input_bin_len;
    auto yamnet_test_input_bin = valiant::filesystem::ReadToMemory(
        "/models/yamnet_test_input.bin", &yamnet_test_input_bin_len);
    if (!valiant::yamnet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }
    valiant::yamnet::loop(output);

    while (true) {
        auto tensor = tflite::GetTensorData<float>(valiant::yamnet::input().get());
        memcpy(tensor, yamnet_test_input_bin.get(), yamnet_test_input_bin_len);
        valiant::yamnet::loop(output);
        vTaskDelay(1000);
    }
    valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
    vTaskSuspend(nullptr);
}
