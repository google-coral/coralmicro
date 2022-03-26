#include "libs/base/filesystem.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tensorflow/classification.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/YAMNet/yamnet.h"

extern "C" [[noreturn]] void app_main(void *param) {
    std::shared_ptr<std::vector<valiant::tensorflow::Class>> output = nullptr;
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);

    std::vector<uint8_t> yamnet_test_input_bin;
    if (!valiant::filesystem::ReadFile("/models/yamnet_test_input.bin",
                                       &yamnet_test_input_bin)) {
        printf("Failed to load input\r\n");
        vTaskSuspend(nullptr);
    }

    if (!valiant::yamnet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }
    valiant::yamnet::loop(output);

    while (true) {
        auto tensor = tflite::GetTensorData<float>(valiant::yamnet::input().get());
        std::memcpy(tensor, yamnet_test_input_bin.data(), yamnet_test_input_bin.size());
        valiant::yamnet::loop(output);
        vTaskDelay(1000);
    }
    valiant::EdgeTpuTask::GetSingleton()->SetPower(false);
    vTaskSuspend(nullptr);
}
