#include "libs/base/filesystem.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tensorflow/classification.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/YAMNet/yamnet.h"

extern "C" [[noreturn]] void app_main(void *param) {
    std::shared_ptr<std::vector<coral::micro::tensorflow::Class>> output = nullptr;
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice(coral::micro::PerformanceMode::kMax);

    std::vector<uint8_t> yamnet_test_input_bin;
    if (!coral::micro::filesystem::ReadFile("/models/yamnet_test_input.bin",
                                       &yamnet_test_input_bin)) {
        printf("Failed to load input\r\n");
        vTaskSuspend(nullptr);
    }

    if (!coral::micro::yamnet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(nullptr);
    }
    coral::micro::yamnet::loop(output);

    while (true) {
        auto tensor = tflite::GetTensorData<float>(coral::micro::yamnet::input().get());
        std::memcpy(tensor, yamnet_test_input_bin.data(), yamnet_test_input_bin.size());
        coral::micro::yamnet::loop(output);
        vTaskDelay(1000);
    }
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(false);
    vTaskSuspend(nullptr);
}
